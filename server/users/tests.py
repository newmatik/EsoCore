from django.contrib.auth.models import User
from django.test import TestCase
from rest_framework import status
from rest_framework.authtoken.models import Token
from rest_framework.test import APIClient

from devices.models import Device, Site

from .models import Dashboard, UserDeviceRole, UserProfile


# ---------------------------------------------------------------------------
# Model tests
# ---------------------------------------------------------------------------


class UserProfileModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")

    def test_create_profile(self):
        profile = UserProfile.objects.create(
            user=self.user, company="Newmatik", phone="+49123456789"
        )
        self.assertIn("testuser", str(profile))
        self.assertEqual(profile.timezone, "UTC")
        self.assertTrue(profile.is_active)


class UserDeviceRoleModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.admin = User.objects.create_user(username="admin", password="pass1234")
        self.site = Site.objects.create(name="Site 1", customer=self.admin)

    def test_create_role(self):
        role = UserDeviceRole.objects.create(
            user=self.user,
            site=self.site,
            role="operator",
            granted_by=self.admin,
        )
        self.assertIn("operator", str(role))
        self.assertTrue(role.is_active)
        self.assertEqual(role.role, "operator")


class DashboardModelTests(TestCase):
    def setUp(self):
        self.user = User.objects.create_user(username="testuser", password="pass1234")

    def test_create_dashboard(self):
        dashboard = Dashboard.objects.create(
            name="Main Dashboard",
            user=self.user,
            layout={"columns": 2},
            widgets=[{"type": "chart", "metric": "temperature"}],
        )
        self.assertIn("testuser", str(dashboard))
        self.assertFalse(dashboard.is_default)
        self.assertFalse(dashboard.is_public)


# ---------------------------------------------------------------------------
# API endpoint tests
# ---------------------------------------------------------------------------


class UserProfileViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.profile = UserProfile.objects.create(
            user=self.user, company="Newmatik"
        )

    def test_list_own_profile(self):
        response = self.client.get("/api/users/profiles/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        results = response.data["results"]
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["company"], "Newmatik")

    def test_cannot_see_other_profiles(self):
        other = User.objects.create_user(username="other", password="pass1234")
        UserProfile.objects.create(user=other, company="Other Corp")
        response = self.client.get("/api/users/profiles/")
        self.assertEqual(len(response.data["results"]), 1)

    def test_update_profile(self):
        response = self.client.patch(
            f"/api/users/profiles/{self.profile.pk}/",
            {"company": "Updated Corp", "phone": "+491111"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.profile.refresh_from_db()
        self.assertEqual(self.profile.company, "Updated Corp")


class UserDeviceRoleViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.admin = User.objects.create_user(username="admin", password="pass1234")
        self.token = Token.objects.create(user=self.admin)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")
        self.site = Site.objects.create(name="Site 1", customer=self.admin)

    def test_create_role(self):
        response = self.client.post(
            "/api/users/roles/",
            {
                "user": self.user.pk,
                "site": self.site.pk,
                "role": "operator",
                "granted_by": self.admin.pk,
            },
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_201_CREATED)
        self.assertEqual(UserDeviceRole.objects.count(), 1)

    def test_list_granted_roles(self):
        UserDeviceRole.objects.create(
            user=self.user,
            site=self.site,
            role="viewer",
            granted_by=self.admin,
        )
        response = self.client.get("/api/users/roles/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(len(response.data["results"]), 1)


class DashboardViewSetTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(username="testuser", password="pass1234")
        self.token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {self.token.key}")

    def test_create_dashboard(self):
        response = self.client.post(
            "/api/users/dashboards/",
            {
                "name": "My Dashboard",
                "layout": {"columns": 2},
                "widgets": [{"type": "chart"}],
            },
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_201_CREATED)
        dashboard = Dashboard.objects.get()
        self.assertEqual(dashboard.user, self.user)

    def test_list_own_dashboards(self):
        Dashboard.objects.create(
            name="Dash 1", user=self.user, layout={}, widgets=[]
        )
        other = User.objects.create_user(username="other", password="pass1234")
        Dashboard.objects.create(
            name="Other Dash", user=other, layout={}, widgets=[]
        )
        response = self.client.get("/api/users/dashboards/")
        results = response.data["results"]
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]["name"], "Dash 1")

    def test_update_dashboard(self):
        dash = Dashboard.objects.create(
            name="Dash", user=self.user, layout={}, widgets=[]
        )
        response = self.client.patch(
            f"/api/users/dashboards/{dash.pk}/",
            {"name": "Updated"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        dash.refresh_from_db()
        self.assertEqual(dash.name, "Updated")

    def test_delete_dashboard(self):
        dash = Dashboard.objects.create(
            name="Dash", user=self.user, layout={}, widgets=[]
        )
        response = self.client.delete(f"/api/users/dashboards/{dash.pk}/")
        self.assertEqual(response.status_code, status.HTTP_204_NO_CONTENT)
        self.assertEqual(Dashboard.objects.count(), 0)


# ---------------------------------------------------------------------------
# Auth endpoint tests
# ---------------------------------------------------------------------------


class AuthViewTests(TestCase):
    def setUp(self):
        self.client = APIClient()
        self.user = User.objects.create_user(
            username="testuser",
            email="test@example.com",
            password="pass1234",
            first_name="Test",
            last_name="User",
        )

    def test_login_with_username(self):
        response = self.client.post(
            "/api/auth/login/",
            {"username": "testuser", "password": "pass1234"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertIn("token", response.data)
        self.assertEqual(response.data["user"]["username"], "testuser")

    def test_login_with_email(self):
        response = self.client.post(
            "/api/auth/login/",
            {"email": "test@example.com", "password": "pass1234"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertIn("token", response.data)

    def test_login_invalid_credentials(self):
        response = self.client.post(
            "/api/auth/login/",
            {"username": "testuser", "password": "wrong"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_401_UNAUTHORIZED)

    def test_login_missing_password(self):
        response = self.client.post(
            "/api/auth/login/",
            {"username": "testuser"},
            format="json",
        )
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)

    def test_me_authenticated(self):
        token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {token.key}")
        response = self.client.get("/api/auth/me/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["username"], "testuser")
        self.assertTrue("is_staff" in response.data)

    def test_me_unauthenticated(self):
        response = self.client.get("/api/auth/me/")
        self.assertIn(response.status_code, (status.HTTP_401_UNAUTHORIZED, status.HTTP_403_FORBIDDEN))

    def test_logout(self):
        token = Token.objects.create(user=self.user)
        self.client.credentials(HTTP_AUTHORIZATION=f"Token {token.key}")
        response = self.client.post("/api/auth/logout/")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        # Token should be deleted
        self.assertFalse(Token.objects.filter(user=self.user).exists())
