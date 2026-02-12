import uuid

from django.contrib.auth.models import User
from django.test import TestCase
from rest_framework import status
from rest_framework.authtoken.models import Token
from rest_framework.test import APIClient

from .models import Device, DeviceConfiguration, FirmwareBundle, Site
from .serializers import DeviceSerializer, FirmwareBundleSerializer, SiteSerializer


# ---------------------------------------------------------------------------
# Model tests
# ---------------------------------------------------------------------------


class SiteModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")

    def test_create_site(self):
        site = Site.objects.create(
            name="Factory A", customer=self.user, address="123 Industrial Rd"
        )
        self.assertEqual(str(site), "testuser - Factory A")
        self.assertEqual(site.customer, self.user)

    def test_unique_together(self):
        Site.objects.create(name="Factory A", customer=self.user)
        with self.assertRaises(Exception):
            Site.objects.create(name="Factory A", customer=self.user)


class DeviceModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)

    def test_create_device(self):
        device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )
        self.assertEqual(str(device), "SN-001 (Edge v1)")
        self.assertEqual(device.status, "active")
        self.assertIsInstance(device.id, uuid.UUID)

    def test_unique_serial_number(self):
        Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )
        with self.assertRaises(Exception):
            Device.objects.create(
                serial_number="SN-001",
                model="Edge v2",
                api_key="key-002",
                api_secret="secret-002",
                site=self.site,
            )


class DeviceConfigurationModelTests(TestCase):
    def setUp(self):
        user = User.objects.create_user(username="testuser", password="pass1234")
        site = Site.objects.create(name="Site 1", customer=user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=site,
        )

    def test_create_configuration(self):
        config = DeviceConfiguration.objects.create(
            device=self.device,
            sampling_rates={"vibration": 1000},
            thresholds={"temp_high": 80},
        )
        self.assertEqual(str(config), "Config for SN-001")


class FirmwareBundleModelTests(TestCase):
    def test_create_firmware_bundle(self):
        fw = FirmwareBundle.objects.create(
            version="1.5.0",
            hash="abc123",
            channel="stable",
            rollout_policy="staged",
            supported_models=["Edge v1"],
            release_notes="Bug fixes",
        )
        self.assertEqual(str(fw), "1.5.0 (stable)")


# ---------------------------------------------------------------------------
# Serializer tests
# ---------------------------------------------------------------------------


class SiteSerializerTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)

    def test_serializer_fields(self):
        serializer = SiteSerializer(self.site)
        data = serializer.data
        self.assertIn("device_count", data)
        self.assertEqual(data["device_count"], 0)
        self.assertEqual(data["name"], "Site 1")

    def test_device_count(self):
        Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )
        serializer = SiteSerializer(self.site)
        self.assertEqual(serializer.data["device_count"], 1)


class DeviceSerializerTests(TestCase):
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

    def test_serializer_includes_site_name(self):
        serializer = DeviceSerializer(self.device)
        self.assertEqual(serializer.data["site_name"], "Site 1")


# ---------------------------------------------------------------------------
# API endpoint tests
# ---------------------------------------------------------------------------


class SiteViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="My Site", customer=self.user)

    def test_list_sites(self):
        response = self.client.get("/api/devices/sites/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        results = response.data["results"]
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["name"], "My Site")

    def test_list_sites_filters_by_user(self):
        other_user = User.objects.create_user(username="other", password="pass1234")
        Site.objects.create(name="Other Site", customer=other_user)
        response = self.client.get("/api/devices/sites/")
        self.assertEqual(len(response.data["results"]), 1)

    def test_retrieve_site(self):
        response = self.client.get(f"/api/devices/sites/{self.site.pk}/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["name"], "My Site")

    def test_unauthenticated_returns_empty(self):
        self.client.credentials()
        response = self.client.get("/api/devices/sites/")
        # DRF defaults to allowing unauthenticated access but queryset returns none()
        self.assertEqual(len(response.data["results"]), 0)


class DeviceViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="My Site", customer=self.user)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )

    def test_list_devices(self):
        response = self.client.get("/api/devices/devices/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 1)

    def test_retrieve_device(self):
        response = self.client.get(f"/api/devices/devices/{self.device.id}/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["serial_number"], "SN-001")

    def test_update_status_action(self):
        response = self.client.post(
            f"/api/devices/devices/{self.device.id}/update_status/",
            {"status": "maintenance"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.device.refresh_from_db()
        self.assertEqual(self.device.status, "maintenance")

    def test_update_status_invalid(self):
        response = self.client.post(
            f"/api/devices/devices/{self.device.id}/update_status/",
            {"status": "invalid"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)

    def test_configuration_action(self):
        DeviceConfiguration.objects.create(
            device=self.device, sampling_rates={"vibration": 1000}
        )
        response = self.client.get(
            f"/api/devices/devices/{self.device.id}/configuration/"
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["sampling_rates"]["vibration"], 1000)

    def test_configuration_not_found(self):
        response = self.client.get(
            f"/api/devices/devices/{self.device.id}/configuration/"
        )
        self.assertEqual(response.status_code, status.HTTP_404_NOT_FOUND)


class FirmwareBundleViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.fw = FirmwareBundle.objects.create(
            version="1.5.0",
            hash="abc123",
            channel="stable",
            supported_models=["Edge v1"],
        )

    def test_list_firmware(self):
        response = self.client.get("/api/devices/firmware/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 1)

    def test_devices_action(self):
        Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            firmware_version="1.4.0",
            api_key="key-001",
            api_secret="secret-001",
            site=self.site,
        )
        response = self.client.get(f"/api/devices/firmware/{self.fw.pk}/devices/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data), 1)
