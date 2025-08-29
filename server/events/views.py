from django_filters.rest_framework import DjangoFilterBackend
from rest_framework import viewsets
from rest_framework.decorators import action
from rest_framework.filters import OrderingFilter
from rest_framework.response import Response

from .models import AlertRule, NotificationQueue, SystemEvent
from .serializers import (
    AlertRuleSerializer,
    NotificationQueueSerializer,
    SystemEventSerializer,
)


class SystemEventViewSet(viewsets.ModelViewSet):
    queryset = SystemEvent.objects.all()
    serializer_class = SystemEventSerializer
    filter_backends = [DjangoFilterBackend, OrderingFilter]
    filterset_fields = ["device", "event_type", "severity", "status", "created_at"]
    ordering_fields = ["created_at", "severity"]

    def get_queryset(self):
        queryset = SystemEvent.objects.select_related("device", "resolved_by")
        if self.request.user.is_authenticated:
            # Filter by user's accessible devices
            user_sites = self.request.user.sites.all()
            return queryset.filter(device__site__in=user_sites)
        return queryset.none()

    @action(detail=True, methods=["post"])
    def acknowledge(self, request, pk=None):
        """Acknowledge an event"""
        event = self.get_object()
        event.status = "acknowledged"
        event.save()
        serializer = self.get_serializer(event)
        return Response(serializer.data)

    @action(detail=True, methods=["post"])
    def resolve(self, request, pk=None):
        """Resolve an event"""
        event = self.get_object()
        event.status = "resolved"
        event.resolved_at = request.data.get("resolved_at")
        event.resolved_by = request.user
        event.save()
        serializer = self.get_serializer(event)
        return Response(serializer.data)


class AlertRuleViewSet(viewsets.ModelViewSet):
    queryset = AlertRule.objects.all()
    serializer_class = AlertRuleSerializer
    filter_backends = [DjangoFilterBackend, OrderingFilter]
    filterset_fields = ["device", "asset", "event_type", "severity", "is_active"]
    ordering_fields = ["created_at", "name"]

    def get_queryset(self):
        queryset = AlertRule.objects.select_related("device", "asset")
        if self.request.user.is_authenticated:
            # Filter by user's accessible devices/assets
            user_sites = self.request.user.sites.all()
            return (
                queryset.filter(device__site__in=user_sites)
                | queryset.filter(asset__site__in=user_sites)
                | queryset.filter(device__isnull=True, asset__isnull=True)
            )
        return queryset.none()


class NotificationQueueViewSet(viewsets.ReadOnlyModelViewSet):
    queryset = NotificationQueue.objects.all()
    serializer_class = NotificationQueueSerializer
    filter_backends = [DjangoFilterBackend, OrderingFilter]
    filterset_fields = ["alert_rule", "status", "channel", "created_at"]
    ordering_fields = ["created_at", "next_retry_at"]

    def get_queryset(self):
        queryset = NotificationQueue.objects.select_related("alert_rule", "event")
        if self.request.user.is_authenticated:
            # Filter by user's accessible alert rules
            user_sites = self.request.user.sites.all()
            return queryset.filter(
                alert_rule__device__site__in=user_sites
            ) | queryset.filter(alert_rule__asset__site__in=user_sites)
        return queryset.none()
