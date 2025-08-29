from django_filters.rest_framework import DjangoFilterBackend
from rest_framework import viewsets
from rest_framework.filters import OrderingFilter

from .models import TelemetryPacket, TelemetryPoint, TelemetryWindow
from .serializers import (
    TelemetryPacketSerializer,
    TelemetryPointSerializer,
    TelemetryWindowSerializer,
)


class TelemetryPacketViewSet(viewsets.ReadOnlyModelViewSet):
    """Read-only viewset for telemetry packets"""

    queryset = TelemetryPacket.objects.all()
    serializer_class = TelemetryPacketSerializer
    filter_backends = [DjangoFilterBackend, OrderingFilter]
    filterset_fields = ["device", "status", "created_at"]
    ordering_fields = ["created_at", "processed_at"]

    def get_queryset(self):
        queryset = TelemetryPacket.objects.select_related("device")
        if self.request.user.is_authenticated:
            # Filter by user's accessible devices
            user_sites = self.request.user.sites.all()
            return queryset.filter(device__site__in=user_sites)
        return queryset.none()


class TelemetryPointViewSet(viewsets.ReadOnlyModelViewSet):
    """Read-only viewset for telemetry points"""

    queryset = TelemetryPoint.objects.all()
    serializer_class = TelemetryPointSerializer
    filter_backends = [DjangoFilterBackend, OrderingFilter]
    filterset_fields = ["device", "metric", "timestamp"]
    ordering_fields = ["timestamp", "value"]

    def get_queryset(self):
        queryset = TelemetryPoint.objects.select_related("device")
        if self.request.user.is_authenticated:
            # Filter by user's accessible devices
            user_sites = self.request.user.sites.all()
            return queryset.filter(device__site__in=user_sites)
        return queryset.none()


class TelemetryWindowViewSet(viewsets.ReadOnlyModelViewSet):
    """Read-only viewset for telemetry windows"""

    queryset = TelemetryWindow.objects.all()
    serializer_class = TelemetryWindowSerializer
    filter_backends = [DjangoFilterBackend, OrderingFilter]
    filterset_fields = ["device", "metric", "timestamp"]
    ordering_fields = ["timestamp", "sample_count"]

    def get_queryset(self):
        queryset = TelemetryWindow.objects.select_related("device")
        if self.request.user.is_authenticated:
            # Filter by user's accessible devices
            user_sites = self.request.user.sites.all()
            return queryset.filter(device__site__in=user_sites)
        return queryset.none()
