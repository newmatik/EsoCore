"""Test data for the portal's browser smoke test (portal/e2e/).

Run by e2e/start-api.mjs through `manage.py shell` after `seed_data`, on the
smoke test's own SQLite database. The API shows a user only the sites they own,
so the smoke-test user gets a site of its own, with more alerts than the API's
default page of 100 rows (the case of #129).
"""

import os

from django.contrib.auth import get_user_model

from assets.models import Asset
from devices.models import Device, Site
from events.models import SystemEvent

User = get_user_model()

username = os.environ["E2E_USERNAME"]
user, _ = User.objects.get_or_create(
    username=username,
    defaults={
        "email": os.environ["E2E_EMAIL"],
        "first_name": "Smoke",
        "last_name": "Test",
    },
)
user.set_password(os.environ["E2E_PASSWORD"])
user.save()

site, _ = Site.objects.get_or_create(
    name="Smoke Test Plant", customer=user, defaults={"address": "Teststrasse 1"}
)

devices = []
for n in range(1, 4):
    device, _ = Device.objects.get_or_create(
        serial_number=f"E2E-EDGE-{n:03d}",
        defaults={
            "model": "EsoCore Edge",
            "api_key": f"e2e-key-{n}",
            "api_secret": f"e2e-secret-{n}",
            "site": site,
            "status": "active",
        },
    )
    devices.append(device)

for n in range(1, 3):
    Asset.objects.get_or_create(
        asset_id=f"E2E-PUMP-{n:02d}",
        defaults={
            "site": site,
            "asset_type": "pump",
            "make": "Testco",
            "model": f"P{n}",
        },
    )

alerts = int(os.environ.get("E2E_ALERTS", "130"))
missing = alerts - SystemEvent.objects.filter(device__site=site).count()
SystemEvent.objects.bulk_create(
    SystemEvent(
        device=devices[n % len(devices)],
        event_type="smoke_test",
        severity="low",
        description=f"Smoke test event {n}",
    )
    for n in range(max(missing, 0))
)
