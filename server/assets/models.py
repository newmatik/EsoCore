import uuid

from devices.models import Site
from django.db import models


class Asset(models.Model):
    """Asset model representing equipment being monitored"""
    ASSET_TYPES = [
        ('door', 'Door'),
        ('cnc', 'CNC Machine'),
        ('robot', 'Industrial Robot'),
        ('pump', 'Pump'),
        ('motor', 'Motor'),
        ('conveyor', 'Conveyor'),
        ('other', 'Other'),
    ]

    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    asset_id = models.CharField(max_length=255, unique=True)
    site = models.ForeignKey(Site, on_delete=models.CASCADE, related_name='assets')
    asset_type = models.CharField(max_length=50, choices=ASSET_TYPES)
    make = models.CharField(max_length=255)
    model = models.CharField(max_length=255)
    serial_number = models.CharField(max_length=255, blank=True)
    installation_date = models.DateField(null=True, blank=True)
    location = models.CharField(max_length=255, blank=True, help_text="Physical location within site")
    specifications = models.JSONField(default=dict, blank=True, help_text="Technical specifications")
    tags = models.JSONField(default=dict, blank=True)
    is_active = models.BooleanField(default=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    class Meta:
        unique_together = ['asset_id', 'site']

    def __str__(self):
        return f"{self.asset_id} - {self.make} {self.model}"


class AssetCycle(models.Model):
    """Asset cycle tracking (e.g., door open/close cycles)"""
    asset = models.ForeignKey(Asset, on_delete=models.CASCADE, related_name='cycles')
    cycle_type = models.CharField(max_length=100, help_text="Type of cycle (open/close, start/stop, etc.)")
    start_time = models.DateTimeField()
    end_time = models.DateTimeField(null=True, blank=True)
    duration = models.DurationField(null=True, blank=True)
    metadata = models.JSONField(default=dict, blank=True)
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        indexes = [
            models.Index(fields=['asset', 'start_time']),
            models.Index(fields=['asset', 'cycle_type', 'start_time']),
        ]

    def __str__(self):
        return f"{self.asset.asset_id} - {self.cycle_type} at {self.start_time}"
