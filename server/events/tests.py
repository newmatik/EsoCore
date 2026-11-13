from django.contrib.auth.models import User
from django.test import TestCase
from rest_framework import status
from rest_framework.authtoken.models import Token
from rest_framework.test import APIClient

from devices.models import Device, Site

from .models import AlertRule, SystemEvent

# ---------------------------------------------------------------------------
# Model tests
# ---------------------------------------------------------------------------


class SystemEventModelTests(TestCase):
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

    def test_create_event(self):
        event = SystemEvent.objects.create(
            device=self.device,
            event_type="temperature_high",
            severity="high",
            description="Temperature exceeded threshold",
        )
        self.assertEqual(event.status, "active")
        self.assertIn("SN-001", str(event))

    def test_event_defaults(self):
        event = SystemEvent.objects.create(
            device=self.device,
            event_type="heartbeat",
            description="Regular heartbeat",
        )
        self.assertEqual(event.severity, "medium")
        self.assertEqual(event.status, "active")


class AlertRuleModelTests(TestCase):
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

    def test_create_alert_rule(self):
        rule = AlertRule.objects.create(
            name="High Temp Alert",
            device=self.device,
            event_type="temperature_high",
            conditions={"threshold": 80},
            severity="high",
        )
        self.assertIn("SN-001", str(rule))
        self.assertTrue(rule.is_active)
        self.assertEqual(rule.cooldown_minutes, 60)


# ---------------------------------------------------------------------------
# API endpoint tests
# ---------------------------------------------------------------------------


class SystemEventViewSetTests(TestCase):
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
        self.event = SystemEvent.objects.create(
            device=self.device,
            event_type="vibration_high",
            severity="high",
            description="Vibration exceeded threshold",
        )

    def test_list_events(self):
        response = self.client.get("/api/events/events/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 1)

    def _create_events(self, count):
        SystemEvent.objects.bulk_create(
            SystemEvent(
                device=self.device,
                event_type="vibration_high",
                severity="low",
                description=f"Event {index}",
            )
            for index in range(count)
        )

    def test_list_events_honours_page_size(self):
        """The portal asks for its "recent events" with page_size."""
        self._create_events(11)  # 12 with the one from setUp

        response = self.client.get("/api/events/events/", {"page_size": 8})

        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["count"], 12)
        self.assertEqual(len(response.data["results"]), 8)
        self.assertIn("page=2", response.data["next"])
        self.assertIn("page_size=8", response.data["next"])

        response = self.client.get("/api/events/events/", {"page_size": 8, "page": 2})

        self.assertEqual(len(response.data["results"]), 4)
        self.assertIsNone(response.data["next"])

    def test_list_events_default_and_maximum_page_size(self):
        self._create_events(510)  # 511 with the one from setUp

        response = self.client.get("/api/events/events/")
        self.assertEqual(len(response.data["results"]), 100)

        response = self.client.get("/api/events/events/", {"page_size": 100000})
        self.assertEqual(len(response.data["results"]), 500)

        # Zero and non-numeric values fall back to the default page size
        response = self.client.get("/api/events/events/", {"page_size": 0})
        self.assertEqual(len(response.data["results"]), 100)
        response = self.client.get("/api/events/events/", {"page_size": "all"})
        self.assertEqual(len(response.data["results"]), 100)

    def test_list_events_filters_by_user_sites(self):
        other_user = User.objects.create_user(username="other", password="pass1234")
        other_site = Site.objects.create(name="Other Site", customer=other_user)
        other_device = Device.objects.create(
            serial_number="SN-002",
            model="Edge v1",
            api_key="key-002",
            api_secret="secret-002",
            site=other_site,
        )
        SystemEvent.objects.create(
            device=other_device,
            event_type="test",
            description="Other event",
        )
        response = self.client.get("/api/events/events/")
        self.assertEqual(len(response.data["results"]), 1)

    def test_create_rejects_another_users_device(self):
        other = User.objects.create_user(username="other", password="pass1234")
        other_site = Site.objects.create(name="Other Site", customer=other)
        other_device = Device.objects.create(
            serial_number="SN-009",
            model="Edge v1",
            api_key="key-009",
            api_secret="secret-009",
            site=other_site,
        )
        before = SystemEvent.objects.count()
        response = self.client.post(
            "/api/events/events/",
            {
                "device": str(other_device.id),
                "event_type": "vibration_high",
                "description": "nope",
            },
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertEqual(SystemEvent.objects.count(), before)

    def test_acknowledge_action(self):
        response = self.client.post(f"/api/events/events/{self.event.id}/acknowledge/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.event.refresh_from_db()
        self.assertEqual(self.event.status, "acknowledged")

    def test_resolve_action(self):
        response = self.client.post(f"/api/events/events/{self.event.id}/resolve/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.event.refresh_from_db()
        self.assertEqual(self.event.status, "resolved")
        self.assertEqual(self.event.resolved_by, self.user)


class AlertRuleViewSetTests(TestCase):
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

    def test_create_alert_rule(self):
        response = self.client.post(
            "/api/events/rules/",
            {
                "name": "Temp Alert",
                "device": str(self.device.id),
                "event_type": "temperature_high",
                "conditions": {"threshold": 80},
                "severity": "high",
            },
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_201_CREATED)
        self.assertEqual(AlertRule.objects.count(), 1)

    def test_create_rejects_another_users_device(self):
        other = User.objects.create_user(username="other", password="pass1234")
        other_site = Site.objects.create(name="Other Site", customer=other)
        other_device = Device.objects.create(
            serial_number="SN-009",
            model="Edge v1",
            api_key="key-009",
            api_secret="secret-009",
            site=other_site,
        )
        response = self.client.post(
            "/api/events/rules/",
            {
                "name": "Temp Alert",
                "device": str(other_device.id),
                "event_type": "temperature_high",
                "conditions": {"threshold": 80},
                "severity": "high",
            },
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertEqual(AlertRule.objects.count(), 0)

    def test_list_alert_rules(self):
        AlertRule.objects.create(
            name="Rule 1",
            device=self.device,
            event_type="test",
            conditions={"threshold": 1},
        )
        response = self.client.get("/api/events/rules/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 1)


class NotificationQueueViewSetTests(TestCase):
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

    def test_list_empty(self):
        response = self.client.get("/api/events/notifications/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 0)

    def test_read_only(self):
        """NotificationQueue should be read-only"""
        response = self.client.post(
            "/api/events/notifications/",
            {"channel": "email"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_405_METHOD_NOT_ALLOWED)
