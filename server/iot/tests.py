import hashlib
import json
import unittest
import uuid
from concurrent.futures import ThreadPoolExecutor
from threading import Barrier
from unittest.mock import MagicMock, PropertyMock, patch

from django.conf import settings
from django.contrib.auth.models import User
from django.db import DatabaseError, IntegrityError, close_old_connections
from django.http.request import RawPostDataException
from django.test import TestCase, TransactionTestCase
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

    def test_invalid_timestamps_are_rejected_without_fabricating_server_time(self):
        payload = [
            {"timestamp": "not-a-date", "metric": "temperature", "value": 1},
            {"metric": "temperature", "value": 2},
            {"timestamp": "2026-02-12T10:00:00Z", "metric": "temperature", "value": 3},
        ]
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps(payload),
            content_type="application/json",
            **self._headers(),
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data, {"accepted": 1, "duplicates": 0, "rejected": 2})
        point = TelemetryPoint.objects.get()
        self.assertEqual(point.timestamp.isoformat(), "2026-02-12T10:00:00+00:00")

    def test_nonfinite_and_oversized_readings_are_rejected_before_storage(self):
        payload = [
            {"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": "Infinity"},
            {"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": "-Infinity"},
            {"timestamp": "2026-02-12T10:00:00Z", "metric": "x" * 256, "value": 1},
            {"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1},
        ]
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps(payload),
            content_type="application/json",
            **self._headers(),
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data, {"accepted": 1, "duplicates": 0, "rejected": 3})
        self.assertEqual(TelemetryPoint.objects.count(), 1)

    def test_storage_failure_rolls_back_upload_so_same_key_can_retry(self):
        payload = [
            {"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1},
            {"timestamp": "2026-02-12T10:00:01Z", "metric": "t", "value": 2},
        ]
        real_create = TelemetryPoint.objects.create
        calls = 0

        def fail_second(**kwargs):
            nonlocal calls
            calls += 1
            if calls == 2:
                raise DatabaseError("synthetic storage outage")
            return real_create(**kwargs)

        with patch("iot.views.TelemetryPoint.objects.create", side_effect=fail_second):
            response = self.client.post(
                "/api/iot/v1/telemetry/batch",
                data=json.dumps(payload),
                content_type="application/json",
                **self._headers(),
            )
        self.assertEqual(response.status_code, status.HTTP_500_INTERNAL_SERVER_ERROR)
        self.assertEqual(TelemetryPacket.objects.count(), 0)
        self.assertEqual(TelemetryPoint.objects.count(), 0)
        self.device.refresh_from_db()
        self.assertIsNone(self.device.last_seen)
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=json.dumps(payload),
            content_type="application/json",
            **self._headers(),
        )
        self.assertEqual(response.data["accepted"], 2)
        self.assertEqual(TelemetryPoint.objects.count(), 2)

    def test_last_seen_failure_does_not_leave_a_committed_receipt(self):
        payload = [{"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1}]
        with patch(
            "iot.views.Device.save", side_effect=DatabaseError("synthetic outage")
        ):
            response = self.client.post(
                "/api/iot/v1/telemetry/batch",
                data=json.dumps(payload),
                content_type="application/json",
                **self._headers(),
            )
        self.assertEqual(response.status_code, status.HTTP_500_INTERNAL_SERVER_ERROR)
        self.assertEqual(TelemetryPacket.objects.count(), 0)
        self.assertEqual(TelemetryPoint.objects.count(), 0)

    def test_malformed_upload_id_returns_400(self):
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data="[]",
            content_type="application/json",
            **self._headers(HTTP_IDEMPOTENCY_KEY="not-a-uuid"),
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertEqual(TelemetryPacket.objects.count(), 0)

    def test_malformed_json_returns_400_without_a_receipt(self):
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data="[invalid-json",
            content_type="application/json",
            **self._headers(),
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertEqual(TelemetryPacket.objects.count(), 0)

    def test_unavailable_hash_body_is_rejected_instead_of_skipping_integrity(self):
        with patch(
            "django.http.HttpRequest.body",
            new_callable=PropertyMock,
            side_effect=RawPostDataException("synthetic consumed stream"),
        ):
            response = self.client.post(
                "/api/iot/v1/telemetry/batch",
                data="[]",
                content_type="application/json",
                **self._headers(HTTP_CONTENT_SHA256="0" * 64),
            )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertEqual(response.data["error"], "Content-SHA256 cannot be verified")
        self.assertEqual(TelemetryPacket.objects.count(), 0)

    def test_unrelated_integrity_failure_is_retryable_without_a_receipt(self):
        with patch(
            "iot.views.TelemetryPoint.objects.create",
            side_effect=IntegrityError("synthetic constraint failure"),
        ):
            response = self.client.post(
                "/api/iot/v1/telemetry/batch",
                data=json.dumps(
                    [{"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1}]
                ),
                content_type="application/json",
                **self._headers(),
            )
        self.assertEqual(response.status_code, status.HTTP_500_INTERNAL_SERVER_ERROR)
        self.assertEqual(TelemetryPacket.objects.count(), 0)
        self.assertEqual(TelemetryPoint.objects.count(), 0)

    def test_duplicate_recovery_lookup_outage_remains_a_retryable_json_error(self):
        empty = MagicMock()
        empty.exists.return_value = False
        with (
            patch(
                "iot.views.TelemetryPacket.objects.filter",
                side_effect=[empty, DatabaseError("synthetic lookup outage")],
            ),
            patch(
                "iot.views.TelemetryPoint.objects.create",
                side_effect=IntegrityError("synthetic constraint failure"),
            ),
        ):
            response = self.client.post(
                "/api/iot/v1/telemetry/batch",
                data=json.dumps(
                    [{"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1}]
                ),
                content_type="application/json",
                **self._headers(),
            )
        self.assertEqual(response.status_code, status.HTTP_500_INTERNAL_SERVER_ERROR)
        self.assertEqual(response.data, {"error": "Internal server error"})
        self.assertEqual(TelemetryPacket.objects.count(), 0)

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
        self.assertEqual(response.status_code, status.HTTP_401_UNAUTHORIZED)

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

    def test_content_sha256_match_is_accepted(self):
        body = json.dumps(
            [{"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1}]
        )
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=body,
            content_type="application/json",
            **self._headers(
                HTTP_CONTENT_SHA256=hashlib.sha256(body.encode()).hexdigest()
            ),
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["accepted"], 1)

    def test_content_sha256_mismatch_is_rejected(self):
        body = json.dumps(
            [{"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1}]
        )
        response = self.client.post(
            "/api/iot/v1/telemetry/batch",
            data=body,
            content_type="application/json",
            **self._headers(
                HTTP_CONTENT_SHA256=hashlib.sha256(b"tampered").hexdigest()
            ),
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertEqual(response.data["error"], "Content-SHA256 mismatch")
        self.assertEqual(TelemetryPacket.objects.count(), 0)
        self.assertEqual(TelemetryPoint.objects.count(), 0)


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
        self.assertEqual(response.status_code, status.HTTP_401_UNAUTHORIZED)


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


@unittest.skipIf(_using_sqlite, "Concurrent writer semantics require PostgreSQL")
class ConcurrentTelemetryBatchTests(TransactionTestCase):
    def test_simultaneous_uploads_commit_one_packet_and_one_set_of_points(self):
        user = User.objects.create_user(
            username="parallel", password="synthetic-password"
        )
        site = Site.objects.create(name="Synthetic site", customer=user)
        device = Device.objects.create(
            serial_number="PARALLEL-001",
            model="Synthetic edge",
            site=site,
            api_key="synthetic-parallel-key",
            api_secret="synthetic-secret",
        )
        upload = str(uuid.uuid4())
        barrier = Barrier(2)
        payload = json.dumps(
            [
                {"timestamp": "2026-02-12T10:00:00Z", "metric": "t", "value": 1},
                {"timestamp": "2026-02-12T10:00:01Z", "metric": "t", "value": 2},
            ]
        )

        def send():
            close_old_connections()
            try:
                barrier.wait(timeout=5)
                response = APIClient().post(
                    "/api/iot/v1/telemetry/batch",
                    data=payload,
                    content_type="application/json",
                    HTTP_X_DEVICE_ID=str(device.id),
                    HTTP_X_AUTH_KEY=device.api_key,
                    HTTP_IDEMPOTENCY_KEY=upload,
                )
                return response.status_code
            finally:
                close_old_connections()

        with ThreadPoolExecutor(max_workers=2) as workers:
            results = list(workers.map(lambda _: send(), range(2)))
        self.assertEqual(sorted(results), [200, 409])
        self.assertEqual(TelemetryPacket.objects.count(), 1)
        self.assertEqual(TelemetryPoint.objects.count(), 2)
        self.assertEqual(TelemetryPacket.objects.get().status, "processed")
