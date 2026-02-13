from drf_spectacular.utils import extend_schema, inline_serializer
from rest_framework import serializers
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import IsAuthenticated
from rest_framework.response import Response

from assets.models import Asset
from devices.models import Device
from events.models import SystemEvent


@extend_schema(
    responses=inline_serializer(
        name="DashboardSummaryResponse",
        fields={
            "total_devices": serializers.IntegerField(),
            "online_devices": serializers.IntegerField(),
            "offline_devices": serializers.IntegerField(),
            "maintenance_devices": serializers.IntegerField(),
            "active_alerts": serializers.IntegerField(),
            "total_assets": serializers.IntegerField(),
            "total_sites": serializers.IntegerField(),
            "device_status_breakdown": serializers.DictField(
                child=serializers.IntegerField()
            ),
            "alert_severity_breakdown": serializers.DictField(
                child=serializers.IntegerField()
            ),
        },
    ),
    summary="Dashboard summary statistics",
)
@api_view(["GET"])
@permission_classes([IsAuthenticated])
def dashboard_summary(request):
    """
    Returns pre-computed dashboard summary stats.

    Response:
    {
        "total_devices": int,
        "online_devices": int,
        "offline_devices": int,
        "maintenance_devices": int,
        "active_alerts": int,
        "total_assets": int,
        "total_sites": int,
        "device_status_breakdown": { "active": int, "inactive": int, ... },
        "alert_severity_breakdown": { "low": int, "medium": int, ... }
    }
    """
    # Scope all queries to the authenticated user's sites
    user_sites = request.user.sites.all()

    devices = Device.objects.filter(site__in=user_sites)
    total_devices = devices.count()

    status_breakdown = {}
    for choice_value, _label in Device.STATUS_CHOICES:
        status_breakdown[choice_value] = devices.filter(status=choice_value).count()

    active_events = SystemEvent.objects.filter(
        status="active", device__site__in=user_sites
    )
    severity_breakdown = {}
    for choice_value, _label in SystemEvent.SEVERITY_CHOICES:
        severity_breakdown[choice_value] = active_events.filter(
            severity=choice_value
        ).count()

    return Response(
        {
            "total_devices": total_devices,
            "online_devices": status_breakdown.get("active", 0),
            "offline_devices": status_breakdown.get("offline", 0),
            "maintenance_devices": status_breakdown.get("maintenance", 0),
            "active_alerts": active_events.count(),
            "total_assets": Asset.objects.filter(site__in=user_sites).count(),
            "total_sites": user_sites.count(),
            "device_status_breakdown": status_breakdown,
            "alert_severity_breakdown": severity_breakdown,
        }
    )
