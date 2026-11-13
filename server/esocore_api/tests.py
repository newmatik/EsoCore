import os
from unittest import mock

from django.conf import settings
from django.contrib.auth.models import User
from django.test import SimpleTestCase, TestCase
from rest_framework.authtoken.models import Token
from rest_framework.test import APIClient

from devices.models import Device, Site
from esocore_api.pagination import with_stable_ordering
from esocore_api.settings import DEFAULT_CORS_ALLOWED_ORIGINS, env_list


class CorsAllowedOriginsSettingTests(SimpleTestCase):
    def test_default_origins_when_env_unset(self):
        env = {k: v for k, v in os.environ.items() if k != "CORS_ALLOWED_ORIGINS"}
        with mock.patch.dict(os.environ, env, clear=True):
            origins = env_list("CORS_ALLOWED_ORIGINS", DEFAULT_CORS_ALLOWED_ORIGINS)
        self.assertEqual(origins, DEFAULT_CORS_ALLOWED_ORIGINS)
        self.assertIn("http://localhost:3080", origins)
        self.assertIn("http://127.0.0.1:3080", origins)

    def test_parses_comma_separated_env_value(self):
        value = " https://portal.example.com, ,https://admin.example.com ,"
        with mock.patch.dict(os.environ, {"CORS_ALLOWED_ORIGINS": value}):
            origins = env_list("CORS_ALLOWED_ORIGINS", DEFAULT_CORS_ALLOWED_ORIGINS)
        self.assertEqual(
            origins, ["https://portal.example.com", "https://admin.example.com"]
        )

    def test_loaded_setting_matches_environment(self):
        expected = env_list("CORS_ALLOWED_ORIGINS", DEFAULT_CORS_ALLOWED_ORIGINS)
        self.assertEqual(settings.CORS_ALLOWED_ORIGINS, expected)


class StableOrderingTests(TestCase):
    """Pages are cut with LIMIT/OFFSET, so every list needs a total order."""

    def test_unordered_queryset_is_ordered_by_primary_key(self):
        queryset = with_stable_ordering(Device.objects.all())
        self.assertEqual(queryset.query.order_by, ("pk",))

    def test_primary_key_breaks_ties_of_the_requested_ordering(self):
        queryset = with_stable_ordering(Device.objects.order_by("-last_seen", "status"))
        self.assertEqual(queryset.query.order_by, ("-last_seen", "status", "pk"))

    def test_ordering_that_ends_in_the_primary_key_is_kept(self):
        for field in ("pk", "-pk", "id", "-id"):
            queryset = with_stable_ordering(Device.objects.order_by("status", field))
            self.assertEqual(queryset.query.order_by, ("status", field))

    def test_sliced_queryset_and_lists_are_left_alone(self):
        sliced = Device.objects.all()[:5]
        self.assertIs(with_stable_ordering(sliced), sliced)
        rows = [1, 2, 3]
        self.assertIs(with_stable_ordering(rows), rows)

    def test_pages_of_a_list_cover_every_row_once(self):
        user = User.objects.create_user(username="pager", password="pass1234")
        token = Token.objects.create(user=user)
        client = APIClient()
        client.credentials(HTTP_AUTHORIZATION=f"Token {token.key}")
        site = Site.objects.create(name="Site 1", customer=user)
        Device.objects.bulk_create(
            Device(
                serial_number=f"SN-{index:03d}",
                model="Edge v1",
                api_key=f"key-{index:03d}",
                api_secret=f"secret-{index:03d}",
                site=site,
            )
            for index in range(7)
        )

        seen = []
        for page in (1, 2, 3):
            response = client.get(
                "/api/devices/devices/", {"page": page, "page_size": 3}
            )
            self.assertEqual(response.status_code, 200)
            seen.extend(row["id"] for row in response.data["results"])

        self.assertEqual(len(seen), 7)
        self.assertEqual(len(set(seen)), 7)
        self.assertEqual(seen, sorted(seen))
