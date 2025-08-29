from django.contrib.auth.models import User
from django.db import models

from devices.models import Device, Site


class UserProfile(models.Model):
    """Extended user profile with additional information"""

    user = models.OneToOneField(User, on_delete=models.CASCADE, related_name="profile")
    company = models.CharField(max_length=255, blank=True)
    phone = models.CharField(max_length=20, blank=True)
    timezone = models.CharField(max_length=50, default="UTC")
    notification_preferences = models.JSONField(default=dict, blank=True)
    is_active = models.BooleanField(default=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    def __str__(self):
        return f"{self.user.username} - {self.company}"


class UserDeviceRole(models.Model):
    """Role-based access control for users on specific devices/sites"""

    ROLE_CHOICES = [
        ("owner", "Owner"),
        ("admin", "Administrator"),
        ("operator", "Operator"),
        ("viewer", "Viewer"),
        ("technician", "Technician"),
    ]

    user = models.ForeignKey(
        User, on_delete=models.CASCADE, related_name="device_roles"
    )
    device = models.ForeignKey(
        Device,
        on_delete=models.CASCADE,
        related_name="user_roles",
        null=True,
        blank=True,
    )
    site = models.ForeignKey(
        Site, on_delete=models.CASCADE, related_name="user_roles", null=True, blank=True
    )
    role = models.CharField(max_length=20, choices=ROLE_CHOICES, default="viewer")
    permissions = models.JSONField(
        default=dict, blank=True, help_text="Specific permissions override"
    )
    granted_by = models.ForeignKey(
        User,
        on_delete=models.SET_NULL,
        null=True,
        blank=True,
        related_name="granted_roles",
    )
    granted_at = models.DateTimeField(auto_now_add=True)
    expires_at = models.DateTimeField(null=True, blank=True)
    is_active = models.BooleanField(default=True)

    class Meta:
        unique_together = ["user", "device", "site"]

    def __str__(self):
        target = (
            self.device.serial_number
            if self.device
            else (self.site.name if self.site else "Global")
        )
        return f"{self.user.username} - {self.role} on {target}"


class Dashboard(models.Model):
    """User-customizable dashboards"""

    name = models.CharField(max_length=255)
    user = models.ForeignKey(User, on_delete=models.CASCADE, related_name="dashboards")
    site = models.ForeignKey(
        Site, on_delete=models.CASCADE, related_name="dashboards", null=True, blank=True
    )
    layout = models.JSONField(default=dict, help_text="Dashboard layout configuration")
    widgets = models.JSONField(default=list, help_text="Widget configurations")
    is_default = models.BooleanField(default=False)
    is_public = models.BooleanField(default=False)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    class Meta:
        unique_together = ["user", "name"]

    def __str__(self):
        return f"{self.user.username} - {self.name}"
