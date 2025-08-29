import uuid

from django.contrib.auth.models import User
from django.db import models


class Site(models.Model):
    """Represents a customer site/location"""

    name = models.CharField(max_length=255)
    customer = models.ForeignKey(User, on_delete=models.CASCADE, related_name="sites")
    address = models.TextField(blank=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    class Meta:
        unique_together = ["name", "customer"]

    def __str__(self):
        return f"{self.customer.username} - {self.name}"


class Device(models.Model):
    """Device model representing IoT devices"""

    STATUS_CHOICES = [
        ("active", "Active"),
        ("inactive", "Inactive"),
        ("maintenance", "Maintenance"),
        ("offline", "Offline"),
    ]

    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    serial_number = models.CharField(max_length=255, unique=True)
    model = models.CharField(max_length=255)
    firmware_version = models.CharField(max_length=100, blank=True)
    api_key = models.CharField(max_length=255, unique=True)
    api_secret = models.CharField(max_length=255)
    site = models.ForeignKey(Site, on_delete=models.CASCADE, related_name="devices")
    tags = models.JSONField(default=dict, blank=True)
    status = models.CharField(max_length=20, choices=STATUS_CHOICES, default="active")
    last_seen = models.DateTimeField(null=True, blank=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    def __str__(self):
        return f"{self.serial_number} ({self.model})"


class DeviceConfiguration(models.Model):
    """Device-specific configuration"""

    device = models.OneToOneField(
        Device, on_delete=models.CASCADE, related_name="configuration"
    )
    sampling_rates = models.JSONField(
        default=dict, help_text="Sampling rates for different sensors"
    )
    thresholds = models.JSONField(default=dict, help_text="Alert thresholds")
    ntp_servers = models.JSONField(default=list, help_text="NTP server endpoints")
    endpoints = models.JSONField(default=dict, help_text="API endpoints configuration")
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    def __str__(self):
        return f"Config for {self.device.serial_number}"


class FirmwareBundle(models.Model):
    """Firmware bundle for OTA updates"""

    CHANNEL_CHOICES = [
        ("stable", "Stable"),
        ("beta", "Beta"),
        ("dev", "Development"),
    ]

    ROLLOUT_CHOICES = [
        ("immediate", "Immediate"),
        ("staged", "Staged"),
        ("manual", "Manual"),
    ]

    version = models.CharField(max_length=100, unique=True)
    hash = models.CharField(max_length=128)
    channel = models.CharField(max_length=20, choices=CHANNEL_CHOICES, default="stable")
    rollout_policy = models.CharField(
        max_length=20, choices=ROLLOUT_CHOICES, default="staged"
    )
    supported_models = models.JSONField(
        default=list, help_text="List of supported device models"
    )
    release_notes = models.TextField(blank=True)
    created_at = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f"{self.version} ({self.channel})"
