from django import forms
from django.contrib import admin
from django.db import models
from unfold.admin import ModelAdmin

from .models import TelemetryPacket, TelemetryPoint, TelemetryWindow


@admin.register(TelemetryPacket)
class TelemetryPacketAdmin(ModelAdmin):
    list_display = ("device", "upload_id", "status", "record_count", "created_at")
    list_filter = ("status", "created_at")
    search_fields = ("upload_id", "device__serial_number")


@admin.register(TelemetryPoint)
class TelemetryPointAdmin(ModelAdmin):
    list_display = ("device", "metric", "timestamp", "value", "unit")
    list_filter = ("metric", "timestamp")
    search_fields = ("metric", "device__serial_number")
    formfield_overrides = {
        models.JSONField: {
            'widget': forms.Textarea(attrs={'rows': 8, 'style': 'font-family: monospace'})
        }
    }


@admin.register(TelemetryWindow)
class TelemetryWindowAdmin(ModelAdmin):
    list_display = ("device", "metric", "timestamp", "window_size_seconds", "sample_count")
    list_filter = ("metric", "timestamp")
    formfield_overrides = {
        models.JSONField: {
            'widget': forms.Textarea(attrs={'rows': 10, 'style': 'font-family: monospace'})
        }
    }

# Register your models here.
