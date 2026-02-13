import uuid

from django.contrib.auth.models import User
from django.db import IntegrityError
from django.test import TestCase
from django.utils import timezone
from rest_framework import status
from rest_framework.authtoken.models import Token
from rest_framework.test import APIClient

from devices.models import Device, Site

from .models import TelemetryPacket, TelemetryPoint, TelemetryWindow

# ---------------------------------------------------------------------------
# Model tests
# ---------------------------------------------------------------------------


class TelemetryPacketModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )

    def test_create_packet(self):
        packet = TelemetryPacket.objects.create(
            device=self.device,
            upload_id=uuid.uuid4(),
            checksum="abc123",
            record_count=10,
        )
        self.assertEqual(packet.status, "pending")
        self.assertIn("SN-001", str(packet))

    def test_unique_together(self):
        upload_id = uuid.uuid4()
        TelemetryPacket.objects.create(
            device=self.device, upload_id=upload_id, checksum="abc"
        )
        with self.assertRaises(IntegrityError):
            TelemetryPacket.objects.create(
                device=self.device, upload_id=upload_id, checksum="def"
            )


class TelemetryPointModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )

    def test_create_point(self):
        point = TelemetryPoint.objects.create(
            device=self.device,
            timestamp=timezone.now(),
            metric="temperature",
            value=22.5,
            unit="C",
        )
        self.assertIn("temperature", str(point))
        self.assertEqual(point.value, 22.5)


class TelemetryWindowModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )

    def test_create_window(self):
        window = TelemetryWindow.objects.create(
            device=self.device,
            timestamp=timezone.now(),
            metric="vibration_rms",
            window_size_seconds=60,
            statistics={"rms": 0.05, "max": 0.12, "std": 0.01},
            sample_count=1000,
        )
        self.assertIn("vibration_rms", str(window))
        self.assertEqual(window.sample_count, 1000)


# ---------------------------------------------------------------------------
# API endpoint tests (read-only viewsets)
# ---------------------------------------------------------------------------


class TelemetryPacketViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )

    def test_list_packets(self):
        TelemetryPacket.objects.create(
            device=self.device,
            upload_id=uuid.uuid4(),
            checksum="abc",
            status="processed",
            record_count=5,
        )
        response = self.client.get("/api/telemetry/packets/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 1)

    def test_read_only(self):
        """Packets viewset is read-only"""
        response = self.client.post(
            "/api/telemetry/packets/",
            {"checksum": "abc"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_405_METHOD_NOT_ALLOWED)


class TelemetryPointViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )

    def test_list_points(self):
        TelemetryPoint.objects.create(
            device=self.device,
            timestamp=timezone.now(),
            metric="temperature",
            value=22.5,
            unit="C",
        )
        response = self.client.get("/api/telemetry/points/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        results = response.data["results"]
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["metric"], "temperature")

    def test_filters_by_user_sites(self):
        other_user = User.objects.create_user(username="other", password="pass1234")
        other_site = Site.objects.create(name="Other Site", customer=other_user)
        other_device = Device.objects.create(
            serial_number="SN-002",
            model="Edge v1",
            api_key="key-002",
            api_secret="secret-002",
            site=other_site,
        )
        TelemetryPoint.objects.create(
            device=other_device,
            timestamp=timezone.now(),
            metric="temperature",
            value=30.0,
        )
        response = self.client.get("/api/telemetry/points/")
        self.assertEqual(len(response.data["results"]), 0)


class TelemetryWindowViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )

    def test_list_windows(self):
        TelemetryWindow.objects.create(
            device=self.device,
            timestamp=timezone.now(),
            metric="vibration_rms",
            window_size_seconds=60,
            statistics={"rms": 0.05},
            sample_count=100,
        )
        response = self.client.get("/api/telemetry/windows/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 1)

    def test_read_only(self):
        """Windows viewset is read-only"""
        response = self.client.post(
            "/api/telemetry/windows/",
            {"metric": "test"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_405_METHOD_NOT_ALLOWED)
