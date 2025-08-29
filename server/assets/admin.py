from django import forms
from django.contrib import admin
from django.db import models
from unfold.admin import ModelAdmin

from .models import Asset, AssetCycle


@admin.register(Asset)
class AssetAdmin(ModelAdmin):
    list_display = ("asset_id", "site", "asset_type", "make", "model", "is_active")
    list_filter = ("asset_type", "is_active", "site")
    search_fields = ("asset_id", "make", "model")
    formfield_overrides = {
        models.JSONField: {
            "widget": forms.Textarea(
                attrs={"rows": 10, "style": "font-family: monospace"}
            )
        }
    }


@admin.register(AssetCycle)
class AssetCycleAdmin(ModelAdmin):
    list_display = ("asset", "cycle_type", "start_time", "end_time", "duration")
    list_filter = ("cycle_type", "start_time")


# Register your models here.
