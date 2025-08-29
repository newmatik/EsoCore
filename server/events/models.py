import uuid

from assets.models import Asset
from devices.models import Device
from django.contrib.auth.models import User
from django.db import models


class SystemEvent(models.Model):
    """System events from devices"""
    SEVERITY_CHOICES = [
        ('low', 'Low'),
        ('medium', 'Medium'),
        ('high', 'High'),
        ('critical', 'Critical'),
    ]

    STATUS_CHOICES = [
        ('active', 'Active'),
        ('acknowledged', 'Acknowledged'),
        ('resolved', 'Resolved'),
        ('suppressed', 'Suppressed'),
    ]

    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    device = models.ForeignKey(Device, on_delete=models.CASCADE, related_name='system_events')
    event_type = models.CharField(max_length=255)
    severity = models.CharField(max_length=20, choices=SEVERITY_CHOICES, default='medium')
    description = models.TextField()
    status = models.CharField(max_length=20, choices=STATUS_CHOICES, default='active')
    duration = models.DurationField(null=True, blank=True)
    resolved_at = models.DateTimeField(null=True, blank=True)
    resolved_by = models.ForeignKey(User, on_delete=models.SET_NULL, null=True, blank=True, related_name='resolved_events')
    metadata = models.JSONField(default=dict, blank=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    class Meta:
        indexes = [
            models.Index(fields=['device', 'created_at']),
            models.Index(fields=['event_type', 'created_at']),
            models.Index(fields=['severity', 'created_at']),
            models.Index(fields=['status', 'created_at']),
        ]

    def __str__(self):
        return f"{self.device.serial_number} - {self.event_type} ({self.severity})"


class EventLog(models.Model):
    """Comprehensive audit trail with full context"""
    id = models.BigAutoField(primary_key=True)
    event = models.ForeignKey(SystemEvent, on_delete=models.CASCADE, related_name='logs')
    action = models.CharField(max_length=255, help_text="Action taken or event detail")
    details = models.JSONField(default=dict, blank=True, help_text="Full context and metadata")
    user = models.ForeignKey(User, on_delete=models.SET_NULL, null=True, blank=True)
    timestamp = models.DateTimeField(auto_now_add=True)

    class Meta:
        indexes = [
            models.Index(fields=['event', 'timestamp']),
            models.Index(fields=['timestamp']),
        ]

    def __str__(self):
        return f"{self.event} - {self.action} at {self.timestamp}"


class AlertRule(models.Model):
    """Alert rules and notification configuration"""
    name = models.CharField(max_length=255)
    description = models.TextField(blank=True)
    device = models.ForeignKey(Device, on_delete=models.CASCADE, related_name='alert_rules', null=True, blank=True)
    asset = models.ForeignKey(Asset, on_delete=models.CASCADE, related_name='alert_rules', null=True, blank=True)
    event_type = models.CharField(max_length=255)
    conditions = models.JSONField(help_text="Alert trigger conditions")
    severity = models.CharField(max_length=20, choices=SystemEvent.SEVERITY_CHOICES, default='medium')
    is_active = models.BooleanField(default=True)
    notification_channels = models.JSONField(default=list, help_text="Email, SMS, Slack, webhook URLs")
    escalation_rules = models.JSONField(default=dict, blank=True, help_text="Escalation configuration")
    cooldown_minutes = models.PositiveIntegerField(default=60, help_text="Minimum time between alerts")
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    class Meta:
        unique_together = ['name', 'device', 'asset']

    def __str__(self):
        target = self.device.serial_number if self.device else (self.asset.asset_id if self.asset else 'Global')
        return f"{self.name} - {target}"


class NotificationQueue(models.Model):
    """Pending alerts with delivery status and retries"""
    STATUS_CHOICES = [
        ('pending', 'Pending'),
        ('sent', 'Sent'),
        ('failed', 'Failed'),
        ('retry', 'Retry'),
    ]

    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    alert_rule = models.ForeignKey(AlertRule, on_delete=models.CASCADE, related_name='notifications')
    event = models.ForeignKey(SystemEvent, on_delete=models.CASCADE, related_name='notifications')
    recipients = models.JSONField(help_text="List of recipients with contact info")
    channel = models.CharField(max_length=50, help_text="Email, SMS, Slack, webhook")
    status = models.CharField(max_length=20, choices=STATUS_CHOICES, default='pending')
    retry_count = models.PositiveIntegerField(default=0)
    max_retries = models.PositiveIntegerField(default=3)
    next_retry_at = models.DateTimeField(null=True, blank=True)
    sent_at = models.DateTimeField(null=True, blank=True)
    error_message = models.TextField(blank=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    class Meta:
        indexes = [
            models.Index(fields=['status', 'created_at']),
            models.Index(fields=['next_retry_at']),
        ]

    def __str__(self):
        return f"{self.alert_rule.name} - {self.channel} ({self.status})"
