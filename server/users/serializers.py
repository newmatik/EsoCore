from rest_framework import serializers

from .models import Dashboard, UserDeviceRole, UserProfile


class UserProfileSerializer(serializers.ModelSerializer):
    username = serializers.CharField(source="user.username", read_only=True)
    email = serializers.EmailField(source="user.email", read_only=True)

    class Meta:
        model = UserProfile
        fields = [
            "user",
            "username",
            "email",
            "company",
            "phone",
            "timezone",
            "notification_preferences",
            "is_active",
            "created_at",
            "updated_at",
        ]
        read_only_fields = ["user", "created_at", "updated_at"]


class UserDeviceRoleSerializer(serializers.ModelSerializer):
    user_name = serializers.CharField(source="user.username", read_only=True)
    device_serial = serializers.CharField(
        source="device.serial_number", read_only=True, allow_null=True
    )
    site_name = serializers.CharField(
        source="site.name", read_only=True, allow_null=True
    )
    granted_by_name = serializers.CharField(
        source="granted_by.username", read_only=True, allow_null=True
    )

    class Meta:
        model = UserDeviceRole
        fields = [
            "id",
            "user",
            "user_name",
            "device",
            "device_serial",
            "site",
            "site_name",
            "role",
            "permissions",
            "granted_by",
            "granted_by_name",
            "granted_at",
            "expires_at",
            "is_active",
        ]
        read_only_fields = ["id", "granted_at"]


class DashboardSerializer(serializers.ModelSerializer):
    class Meta:
        model = Dashboard
        fields = [
            "id",
            "name",
            "site",
            "layout",
            "widgets",
            "is_default",
            "is_public",
            "created_at",
            "updated_at",
        ]
        read_only_fields = ["id", "created_at", "updated_at"]
