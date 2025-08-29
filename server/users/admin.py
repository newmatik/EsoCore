from django import forms
from django.contrib import admin
from django.db import models
from unfold.admin import ModelAdmin

from .models import Dashboard, UserDeviceRole, UserProfile


@admin.register(UserProfile)
class UserProfileAdmin(ModelAdmin):
    list_display = ("user", "company", "phone", "timezone", "is_active")
    list_filter = ("is_active",)
    search_fields = ("user__username", "company")


@admin.register(UserDeviceRole)
class UserDeviceRoleAdmin(ModelAdmin):
    list_display = ("user", "device", "site", "role", "is_active", "granted_at")
    list_filter = ("role", "is_active")
    search_fields = ("user__username", "device__serial_number", "site__name")


@admin.register(Dashboard)
class DashboardAdmin(ModelAdmin):
    list_display = ("name", "user", "site", "is_default", "is_public")
    list_filter = ("is_default", "is_public")
    formfield_overrides = {
        models.JSONField: {
            "widget": forms.Textarea(
                attrs={"rows": 10, "style": "font-family: monospace"}
            )
        }
    }


# Register your models here.
