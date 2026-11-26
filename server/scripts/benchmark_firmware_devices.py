"""Local endpoint benchmark using only a fresh in-memory SQLite database."""

import json
import os
import statistics
import sys
from pathlib import Path
from time import perf_counter

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
os.environ["DJANGO_SETTINGS_MODULE"] = "esocore_api.settings"

import django
from django.conf import settings

settings.DATABASES = {
    "default": {"ENGINE": "django.db.backends.sqlite3", "NAME": ":memory:"}
}
django.setup()

from django.contrib.auth.models import User  # noqa: E402
from django.core.management import call_command  # noqa: E402
from django.db import connection  # noqa: E402
from django.test.utils import CaptureQueriesContext  # noqa: E402
from rest_framework.test import APIRequestFactory, force_authenticate  # noqa: E402

from devices.models import (  # noqa: E402
    Device,
    DeviceConfiguration,
    FirmwareBundle,
    Site,
)
from devices.views import FirmwareBundleViewSet  # noqa: E402

call_command("migrate", verbosity=0)
user = User.objects.create(username="benchmark")
site = Site.objects.create(name="Benchmark", customer=user)
firmware = FirmwareBundle.objects.create(
    version="2.0", hash="benchmark", supported_models=["Edge"]
)
factory = APIRequestFactory()
view = FirmwareBundleViewSet.as_view({"get": "devices"})

for count in (1, 25, 100):
    Device.objects.all().delete()
    for index in range(count):
        device = Device.objects.create(
            serial_number=f"bench-{index}",
            api_key=f"bench-{index}",
            api_secret="unused",
            site=site,
            model="Edge",
            firmware_version="1.0",
        )
        if index % 2 == 0:
            DeviceConfiguration.objects.create(
                device=device, sampling_rates={"test": 1}
            )
    timings = []
    queries = []
    for run in range(21):
        request = factory.get("/api/devices/firmware/1/devices/")
        force_authenticate(request, user=user)
        with CaptureQueriesContext(connection) as captured:
            start = perf_counter()
            response = view(request, pk=firmware.pk)
            response.render()
            elapsed = (perf_counter() - start) * 1000
        assert response.status_code == 200
        assert len(response.data) == count
        if run:  # Exclude the first request from warm-run statistics.
            timings.append(elapsed)
            queries.append(len(captured))
        else:
            first_ms = elapsed
    ordered = sorted(timings)
    print(
        json.dumps(
            {
                "devices": count,
                "runs": len(timings),
                "first_request_ms": first_ms,
                "p50_ms": statistics.median(timings),
                "p95_ms": ordered[18],
                "p99_ms": ordered[19],
                "query_counts": sorted(set(queries)),
                "payload_bytes": len(response.content),
            }
        )
    )
