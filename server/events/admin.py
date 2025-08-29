from django import forms
from django.contrib import admin
from django.db import models
from unfold.admin import ModelAdmin

from .models import AlertRule, EventLog, NotificationQueue, SystemEvent


@admin.register(SystemEvent)
class SystemEventAdmin(ModelAdmin):
    list_display = ("device", "event_type", "severity", "status", "created_at")
    list_filter = ("severity", "status", "event_type")
    search_fields = ("event_type", "device__serial_number")
    formfield_overrides = {
        models.JSONField: {
            'widget': forms.Textarea(attrs={'rows': 10, 'style': 'font-family: monospace'})
        }
    }


@admin.register(EventLog)
class EventLogAdmin(ModelAdmin):
    list_display = ("event", "action", "timestamp", "user")
    list_filter = ("timestamp",)


@admin.register(AlertRule)
class AlertRuleAdmin(ModelAdmin):
    list_display = ("name", "event_type", "severity", "is_active", "created_at")
    list_filter = ("severity", "is_active")
    search_fields = ("name", "event_type")
    formfield_overrides = {
        models.JSONField: {
            'widget': forms.Textarea(attrs={'rows': 12, 'style': 'font-family: monospace'})
        }
    }


@admin.register(NotificationQueue)
class NotificationQueueAdmin(ModelAdmin):
    list_display = ("alert_rule", "event", "channel", "status", "retry_count", "created_at")
    list_filter = ("status", "channel")

# Register your models here.
