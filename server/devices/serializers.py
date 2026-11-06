from rest_framework import serializers

from esocore_api.access import require_caller_site

from .models import Device, DeviceConfiguration, FirmwareBundle, Site


class SiteSerializer(serializers.ModelSerializer):
    device_count = serializers.IntegerField(read_only=True, default=0)

    class Meta:
        model = Site
        fields = ["id", "name", "address", "device_count", "created_at", "updated_at"]
        read_only_fields = ["id", "created_at", "updated_at"]


class DeviceConfigurationSerializer(serializers.ModelSerializer):
    class Meta:
        model = DeviceConfiguration
        fields = [
            "sampling_rates",
            "thresholds",
            "ntp_servers",
            "endpoints",
            "created_at",
            "updated_at",
        ]
        read_only_fields = ["created_at", "updated_at"]


class DeviceSerializer(serializers.ModelSerializer):
    site_name = serializers.CharField(source="site.name", read_only=True)
    configuration = DeviceConfigurationSerializer(read_only=True)

    class Meta:
        model = Device
        fields = [
            "id",
            "serial_number",
            "model",
            "firmware_version",
            "site",
            "site_name",
            "tags",
            "status",
            "last_seen",
            "configuration",
            "created_at",
            "updated_at",
        ]
        read_only_fields = ["id", "created_at", "updated_at"]

    def validate_site(self, site):
        return require_caller_site(self, site)


class FirmwareBundleSerializer(serializers.ModelSerializer):
    class Meta:
        model = FirmwareBundle
        fields = [
            "version",
            "hash",
            "channel",
            "rollout_policy",
            "supported_models",
            "release_notes",
            "created_at",
        ]
        read_only_fields = ["created_at"]
