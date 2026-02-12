import json
import unittest
import uuid

from django.conf import settings
from django.contrib.auth.models import User
from django.test import TestCase
from rest_framework import status
from rest_framework.test import APIClient

from devices.models import Device, DeviceConfiguration, FirmwareBundle, Site
from events.models import SystemEvent
from telemetry.models import TelemetryPacket, TelemetryPoint

_using_sqlite = "sqlite" in settings.DATABASES["default"]["ENGINE"]


class AuthHandshakeTests(TestCase):
    def setUp(self):
        self.client = APIClient()

    def test_handshake_returns_nonce_and_time(self):
        response = self.client.post("/api/iot/v1/auth/handshake")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertIn("nonce", response.data)
        self.assertIn("server_time", response.data)
        self.assertIn("version", response.data)
        self.assertEqual(response.data["version"], "1.0.0")


class TelemetryBatchTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="test-api-key",
            api_secret="secret-001",
            site=self.site,
        )
        self.idempotency_key = str(uuid.uuid4())

    def _headers(self, **overrides):
        defaults = {
            "HTTP_X_DEVICE_ID": str(self.device.id),
            "HTTP_X_AUTH_KEY": self.device.api_key,
            "HTTP_IDEMPOTENCY_KEY": self.idempotency_key,
        }
        defaults.update(overrides)
        return defaults

    def test_successful_batch(self):
        payload = [
            {
                "timestamp": "2026-02-12T10:00:00Z",
                "metric": "temperature",
                "value": 22.5,
                "unit": "C",
            },
            {
                "timestamp": "2026-02-12T10:00:01Z",
                "metric": "vibration",
                "value": 0.05,
                "unit": "g",
            },
        ]
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps(payload),
            content_type="application/json",
            **self._headers(),
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["accepted"], 2)
        self.assertEqual(response.data["rejected"], 0)
        self.assertEqual(TelemetryPoint.objects.count(), 2)
        self.assertEqual(TelemetryPacket.objects.count(), 1)

        # Device last_seen should be updated
        self.device.refresh_from_db()
        self.assertIsNotNone(self.device.last_seen)

    def test_duplicate_upload_rejected(self):
        payload = [{"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1}]
        self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps(payload),
            content_type="application/json",
            **self._headers(),
        )
        # Same idempotency key should be rejected
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps(payload),
            content_type="application/json",
            **self._headers(),
        )
        self.assertEqual(response.status_code, status.HTTP_409_CONFLICT)
        self.assertEqual(response.data["duplicates"], 1)

    def test_missing_headers(self):
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps([]),
            content_type="application/json",
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)

    def test_invalid_device(self):
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps([]),
            content_type="application/json",
            **self._headers(
                HTTP_X_DEVICE_ID=str(uuid.uuid4()),
                HTTP_X_AUTH_KEY="wrong-key",
            ),
        )
        self.assertEqual(response.status_code, status.HTTP_401_UNAUTHORIZED)

    def test_non_list_payload(self):
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps({"metric": "t", "value": 1}),
            content_type="application/json",
            **self._headers(),
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)


class GetConfigTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="test-api-key",
            api_secret="secret-001",
            site=self.site,
        )

    def _headers(self):
        return {
            "HTTP_X_DEVICE_ID": str(self.device.id),
            "HTTP_X_AUTH_KEY": self.device.api_key,
        }

    def test_get_config_with_configuration(self):
        DeviceConfiguration.objects.create(
            device=self.device,
            sampling_rates={"vibration": 2000},
            thresholds={"temp": 80},
            ntp_servers=["pool.ntp.org"],
            endpoints={"telemetry": "/api/iot/v1/telemetry/batch"},
        )
        response = self.client.get("/api/iot/v1/config", **self._headers())
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["sampling_rates"]["vibration"], 2000)
        self.assertEqual(response.data["ntp_servers"], ["pool.ntp.org"])

    def test_get_config_without_configuration(self):
        response = self.client.get("/api/iot/v1/config", **self._headers())
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["sampling_rates"], {})

    def test_invalid_device(self):
        response = self.client.get(
            "/api/iot/v1/config",
            HTTP_X_DEVICE_ID=str(uuid.uuid4()),
            HTTP_X_AUTH_KEY="wrong",
        )
        self.assertEqual(response.status_code, status.HTTP_404_NOT_FOUND)


class OTACheckTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            firmware_version="1.4.0",
            api_key="test-api-key",
            api_secret="secret-001",
            site=self.site,
        )

    def _headers(self):
        return {
            "HTTP_X_DEVICE_ID": str(self.device.id),
            "HTTP_X_AUTH_KEY": self.device.api_key,
        }

    @unittest.skipIf(_using_sqlite, "JSON contains not supported on SQLite")
    def test_update_available(self):
        FirmwareBundle.objects.create(
            version="1.5.0",
            hash="abc123",
            channel="stable",
            supported_models=["Edge v1"],
            release_notes="Bug fixes",
        )
        response = self.client.get("/api/iot/v1/ota/check", **self._headers())
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertTrue(response.data["available"])
        self.assertEqual(response.data["version"], "1.5.0")

    @unittest.skipIf(_using_sqlite, "JSON contains not supported on SQLite")
    def test_no_update_available(self):
        response = self.client.get("/api/iot/v1/ota/check", **self._headers())
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertFalse(response.data["available"])


class OTAReportTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            firmware_version="1.4.0",
            api_key="test-api-key",
            api_secret="secret-001",
            site=self.site,
        )

    def _headers(self):
        return {
            "HTTP_X_DEVICE_ID": str(self.device.id),
            "HTTP_X_AUTH_KEY": self.device.api_key,
        }

    def test_report_completed(self):
        response = self.client.post(
            "/api/iot/v1/ota/report",
            {"status": "completed", "version": "1.5.0"},
            format="json",
            **self._headers(),
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.device.refresh_from_db()
        self.assertEqual(self.device.firmware_version, "1.5.0")
        self.assertTrue(SystemEvent.objects.filter(event_type="ota_completed").exists())

    def test_report_failed(self):
        response = self.client.post(
            "/api/iot/v1/ota/report",
            {"status": "failed", "version": "1.5.0", "error": "CRC mismatch"},
            format="json",
            **self._headers(),
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.device.refresh_from_db()
        # Firmware version should NOT be updated on failure
        self.assertEqual(self.device.firmware_version, "1.4.0")
        event = SystemEvent.objects.get(event_type="ota_failed")
        self.assertEqual(event.severity, "high")
