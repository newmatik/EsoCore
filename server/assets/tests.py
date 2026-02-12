from datetime import timedelta

from django.contrib.auth.models import User
from django.test import TestCase
from django.utils import timezone
from rest_framework import status
from rest_framework.authtoken.models import Token
from rest_framework.test import APIClient

from devices.models import Site

from .models import Asset, AssetCycle
from .serializers import AssetSerializer


# ---------------------------------------------------------------------------
# Model tests
# ---------------------------------------------------------------------------


class AssetModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)

    def test_create_asset(self):
        asset = Asset.objects.create(
            asset_id="DOOR-001",
            site=self.site,
            asset_type="door",
            make="Kuka",
            model="KR 10",
        )
        self.assertIn("DOOR-001", str(asset))
        self.assertTrue(asset.is_active)

    def test_unique_together(self):
        Asset.objects.create(
            asset_id="DOOR-001",
            site=self.site,
            asset_type="door",
            make="Kuka",
            model="KR 10",
        )
        with self.assertRaises(Exception):
            Asset.objects.create(
                asset_id="DOOR-001",
                site=self.site,
                asset_type="motor",
                make="Siemens",
                model="1LE1",
            )


class AssetCycleModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.asset = Asset.objects.create(
            asset_id="DOOR-001",
            site=self.site,
            asset_type="door",
            make="Kuka",
            model="KR 10",
        )

    def test_create_cycle(self):
        now = timezone.now()
        cycle = AssetCycle.objects.create(
            asset=self.asset,
            cycle_type="open/close",
            start_time=now,
            end_time=now + timedelta(seconds=5),
            duration=timedelta(seconds=5),
        )
        self.assertIn("DOOR-001", str(cycle))
        self.assertEqual(cycle.cycle_type, "open/close")


# ---------------------------------------------------------------------------
# Serializer tests
# ---------------------------------------------------------------------------


class AssetSerializerTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.asset = Asset.objects.create(
            asset_id="MOTOR-001",
            site=self.site,
            asset_type="motor",
            make="Siemens",
            model="1LE1",
        )

    def test_cycle_count_field(self):
        serializer = AssetSerializer(self.asset)
        self.assertEqual(serializer.data["cycle_count"], 0)

    def test_cycle_count_after_adding_cycles(self):
        now = timezone.now()
        AssetCycle.objects.create(
            asset=self.asset, cycle_type="start/stop", start_time=now
        )
        # Use annotated queryset (as the viewset does) for the count field
        from django.db.models import Count

        asset = Asset.objects.annotate(cycle_count=Count("cycles")).get(pk=self.asset.pk)
        serializer = AssetSerializer(asset)
        self.assertEqual(serializer.data["cycle_count"], 1)

    def test_site_name_field(self):
        serializer = AssetSerializer(self.asset)
        self.assertEqual(serializer.data["site_name"], "Site 1")


# ---------------------------------------------------------------------------
# API endpoint tests
# ---------------------------------------------------------------------------


class AssetViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.asset = Asset.objects.create(
            asset_id="PUMP-001",
            site=self.site,
            asset_type="pump",
            make="Grundfos",
            model="CR 32",
        )

    def test_list_assets(self):
        response = self.client.get("/api/assets/assets/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        results = response.data["results"]
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["asset_id"], "PUMP-001")

    def test_list_filters_by_user_sites(self):
        other_user = User.objects.create_user(username="other", password="pass1234")
        other_site = Site.objects.create(name="Other Site", customer=other_user)
        Asset.objects.create(
            asset_id="PUMP-002",
            site=other_site,
            asset_type="pump",
            make="Grundfos",
            model="CR 16",
        )
        response = self.client.get("/api/assets/assets/")
        self.assertEqual(len(response.data["results"]), 1)

    def test_create_asset(self):
        response = self.client.post(
            "/api/assets/assets/",
            {
                "asset_id": "MOTOR-001",
                "site": self.site.pk,
                "asset_type": "motor",
                "make": "Siemens",
                "model": "1LE1",
            },
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_201_CREATED)
        self.assertEqual(Asset.objects.count(), 2)

    def test_retrieve_asset(self):
        response = self.client.get(f"/api/assets/assets/{self.asset.id}/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["asset_id"], "PUMP-001")


class AssetCycleViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="Site 1", customer=self.user)
        self.asset = Asset.objects.create(
            asset_id="DOOR-001",
            site=self.site,
            asset_type="door",
            make="Kuka",
            model="KR 10",
        )

    def test_list_cycles(self):
        now = timezone.now()
        AssetCycle.objects.create(
            asset=self.asset, cycle_type="open/close", start_time=now
        )
        response = self.client.get("/api/assets/cycles/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 1)

    def test_create_cycle(self):
        response = self.client.post(
            "/api/assets/cycles/",
            {
                "asset": str(self.asset.id),
                "cycle_type": "open/close",
                "start_time": "2026-02-12T10:00:00Z",
            },
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_201_CREATED)
