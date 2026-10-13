import os
from unittest import mock

from django.conf import settings
from django.test import SimpleTestCase

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
