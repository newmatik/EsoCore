"""Opt-in, disposable PostgreSQL backend for CI and local protocol tests."""

import os

from .settings import *  # noqa: F403

DATABASES = {
    "default": {
        "ENGINE": "django.db.backends.postgresql",
        "NAME": os.environ.get("TEST_POSTGRES_DB", "esocore_telemetry_fixture"),
        "USER": os.environ.get("TEST_POSTGRES_USER", "postgres"),
        "PASSWORD": os.environ.get("TEST_POSTGRES_PASSWORD", "local-test-only"),
        "HOST": os.environ.get("TEST_POSTGRES_HOST", "127.0.0.1"),
        "PORT": os.environ.get("TEST_POSTGRES_PORT", "5432"),
    }
}
