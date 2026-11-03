import uuid

from django.contrib.auth.models import User
from django.db import IntegrityError
from django.test import TestCase
from rest_framework import status
from rest_framework.authtoken.models import Token
from rest_framework.test import APIClient, APIRequestFactory

from .auth import APIKeyAuthentication, DevicePrincipal
from .models import Device, DeviceConfiguration, FirmwareBundle, Site
from .serializers import DeviceSerializer, SiteSerializer

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
        with self.assertRaises(IntegrityError):
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
        with self.assertRaises(IntegrityError):
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
        # Use annotated queryset (as the viewset does) for the count field
        from django.db.models import Count

        site = Site.objects.annotate(device_count=Count("devices")).get(pk=self.site.pk)
        serializer = SiteSerializer(site)
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


# ---------------------------------------------------------------------------
# Device API key authentication (devices.auth)
# ---------------------------------------------------------------------------


class DeviceAPIKeyAuthenticationTests(TestCase):
    """Regression: a valid key used to raise AttributeError (500) because the
    authenticator read `is_active` from rest_framework_api_key's APIKey model
    and returned that key object as request.user."""

    def setUp(self):
        self.client = APIClient()
        self.owner = User.objects.create_user(username="owner", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.owner)
        self.device = Device.objects.create(
            serial_number="SN-001",
            model="Edge v1",
            api_key="device-key-001",
            api_secret="secret-001",
            site=self.site,
        )
        self.factory = APIRequestFactory()

    def _authenticate(self, **headers):
        return APIKeyAuthentication().authenticate(
            self.factory.get("/api/devices/devices/", **headers)
        )

    def test_device_credentials_authenticate_as_the_device(self):
        for key_header in (
            {"HTTP_X_AUTH_KEY": "device-key-001"},
            {"HTTP_X_API_KEY": "device-key-001"},
            {"HTTP_AUTHORIZATION": "Api-Key device-key-001"},
        ):
            user, auth = self._authenticate(
                HTTP_X_DEVICE_ID=str(self.device.id), **key_header
            )
            self.assertIsInstance(user, DevicePrincipal)
            self.assertEqual(user.device, self.device)
            self.assertEqual(auth, self.device)

    def test_device_principal_has_no_user_rights(self):
        user, _ = self._authenticate(
            HTTP_X_DEVICE_ID=str(self.device.id), HTTP_X_API_KEY="device-key-001"
        )
        self.assertFalse(user.is_authenticated)
        self.assertFalse(user.is_staff)
        self.assertFalse(user.has_perm("devices.change_device"))

    def test_valid_device_key_on_user_api_is_not_a_server_error(self):
        headers = {
            "HTTP_X_DEVICE_ID": str(self.device.id),
            "HTTP_X_API_KEY": "device-key-001",
        }
        response = self.client.get("/api/devices/devices/", **headers)
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        # A device sees none of its owner's data and cannot write.
        self.assertEqual(response.data["count"], 0)
        response = self.client.post(
            "/api/devices/sites/", {"name": "Injected"}, format="json", **headers
        )
        self.assertIn(
            response.status_code,
            (status.HTTP_401_UNAUTHORIZED, status.HTTP_403_FORBIDDEN),
        )
        self.assertFalse(Site.objects.filter(name="Injected").exists())

    def test_wrong_key_or_malformed_device_id_is_rejected(self):
        for headers in (
            {"HTTP_X_DEVICE_ID": str(self.device.id), "HTTP_X_API_KEY": "wrong"},
            {"HTTP_X_DEVICE_ID": str(uuid.uuid4()), "HTTP_X_API_KEY": "device-key-001"},
            {"HTTP_X_DEVICE_ID": "not-a-uuid", "HTTP_X_API_KEY": "device-key-001"},
        ):
            response = self.client.get("/api/devices/devices/", **headers)
            # DRF answers 403 rather than 401 because SessionAuthentication,
            # listed first, sends no WWW-Authenticate challenge.
            self.assertIn(
                response.status_code,
                (status.HTTP_401_UNAUTHORIZED, status.HTTP_403_FORBIDDEN),
            )
            self.assertEqual(
                response.data["detail"], "Device not found or invalid API key"
            )

    def test_key_without_device_id_falls_back_to_other_authentication(self):
        self.assertIsNone(self._authenticate(HTTP_X_API_KEY="device-key-001"))
        token = Token.objects.create(user=self.owner)
        response = self.client.get(
            "/api/devices/devices/",
            HTTP_AUTHORIZATION=f"Token {token.key}",
            HTTP_X_API_KEY="stray-key",
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["count"], 1)
