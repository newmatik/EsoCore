import uuid

from django.db import models

from devices.models import Device


class TelemetryPacket(models.Model):
    """Ingest ledger for telemetry batches"""

    STATUS_CHOICES = [
        ("pending", "Pending"),
        ("processing", "Processing"),
        ("processed", "Processed"),
        ("failed", "Failed"),
    ]

    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    device = models.ForeignKey(
        Device, on_delete=models.CASCADE, related_name="telemetry_packets"
    )
    upload_id = models.UUIDField(help_text="Client-generated upload identifier")
    checksum = models.CharField(max_length=128)
    status = models.CharField(max_length=20, choices=STATUS_CHOICES, default="pending")
    record_count = models.PositiveIntegerField(default=0)
    processed_at = models.DateTimeField(null=True, blank=True)
    error_message = models.TextField(blank=True)
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        unique_together = ["device", "upload_id"]
        indexes = [
            models.Index(fields=["device", "created_at"]),
            models.Index(fields=["status", "created_at"]),
        ]

    def __str__(self):
        return f"{self.device.serial_number} - {self.upload_id}"


class TelemetryPoint(models.Model):
    """Individual telemetry data points"""

    id = models.BigAutoField(primary_key=True)
    device = models.ForeignKey(
        Device, on_delete=models.CASCADE, related_name="telemetry_points"
    )
    timestamp = models.DateTimeField()
    metric = models.CharField(max_length=255)
    value = models.FloatField()
    unit = models.CharField(max_length=50, blank=True)
    meta = models.JSONField(default=dict, blank=True, help_text="Additional metadata")

    class Meta:
        indexes = [
            models.Index(fields=["device", "timestamp"]),
            models.Index(fields=["device", "metric", "timestamp"]),
            models.Index(fields=["metric", "timestamp"]),
        ]

    def __str__(self):
        return f"{self.device.serial_number} - {self.metric}: {self.value} at {self.timestamp}"


class TelemetryWindow(models.Model):
    """Aggregated telemetry windows (RMS, max, statistics)"""

    id = models.BigAutoField(primary_key=True)
    device = models.ForeignKey(
        Device, on_delete=models.CASCADE, related_name="telemetry_windows"
    )
    timestamp = models.DateTimeField()
    metric = models.CharField(max_length=255)
    window_size_seconds = models.PositiveIntegerField(
        help_text="Size of the aggregation window"
    )
    statistics = models.JSONField(
        help_text="Aggregated statistics (rms, max, std, bandpower, etc.)"
    )
    sample_count = models.PositiveIntegerField(default=0)

    class Meta:
        indexes = [
            models.Index(fields=["device", "timestamp"]),
            models.Index(fields=["device", "metric", "timestamp"]),
            models.Index(fields=["metric", "timestamp"]),
        ]

    def __str__(self):
        return f"{self.device.serial_number} - {self.metric} window at {self.timestamp}"
