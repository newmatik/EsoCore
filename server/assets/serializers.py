from rest_framework import serializers

from .models import Asset, AssetCycle


class AssetSerializer(serializers.ModelSerializer):
    site_name = serializers.CharField(source="site.name", read_only=True)
    cycle_count = serializers.SerializerMethodField()

    class Meta:
        model = Asset
        fields = [
            "id",
            "asset_id",
            "site",
            "site_name",
            "asset_type",
            "make",
            "model",
            "serial_number",
            "installation_date",
            "location",
            "specifications",
            "tags",
            "is_active",
            "cycle_count",
            "created_at",
            "updated_at",
        ]
        read_only_fields = ["id", "created_at", "updated_at"]

    def get_cycle_count(self, obj):
        return obj.cycles.count()


class AssetCycleSerializer(serializers.ModelSerializer):
    asset_name = serializers.CharField(source="asset.asset_id", read_only=True)

    class Meta:
        model = AssetCycle
        fields = [
            "id",
            "asset",
            "asset_name",
            "cycle_type",
            "start_time",
            "end_time",
            "duration",
            "metadata",
            "created_at",
        ]
        read_only_fields = ["id", "created_at"]
