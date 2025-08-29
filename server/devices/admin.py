from django import forms
from django.contrib import admin
from django.db import models
from unfold.admin import ModelAdmin

from .models import Device, DeviceConfiguration, FirmwareBundle, Site


@admin.register(Site)
class SiteAdmin(ModelAdmin):
    list_display = ("name", "customer", "created_at", "updated_at")
    search_fields = ("name", "customer__username")


@admin.register(Device)
class DeviceAdmin(ModelAdmin):
    list_display = ("serial_number", "model", "site", "status", "last_seen")
    list_filter = ("status", "model", "site")
    search_fields = ("serial_number", "model")
    formfield_overrides = {
        models.JSONField: {
            'widget': forms.Textarea(attrs={'rows': 10, 'style': 'font-family: monospace'})
        }
    }


@admin.register(DeviceConfiguration)
class DeviceConfigurationAdmin(ModelAdmin):
    list_display = ("device", "created_at", "updated_at")
    formfield_overrides = {
        models.JSONField: {
            'widget': forms.Textarea(attrs={'rows': 12, 'style': 'font-family: monospace'})
        }
    }


@admin.register(FirmwareBundle)
class FirmwareBundleAdmin(ModelAdmin):
    list_display = ("version", "channel", "created_at")
    list_filter = ("channel",)

# Register your models here.
