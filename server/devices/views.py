from django.shortcuts import get_object_or_404
from rest_framework import status, viewsets
from rest_framework.decorators import action
from rest_framework.response import Response

from .models import Device, DeviceConfiguration, FirmwareBundle, Site
from .serializers import (
    DeviceConfigurationSerializer,
    DeviceSerializer,
    FirmwareBundleSerializer,
    SiteSerializer,
)


class SiteViewSet(viewsets.ModelViewSet):
    queryset = Site.objects.all()
    serializer_class = SiteSerializer

    def get_queryset(self):
        queryset = Site.objects.all()
        if self.request.user.is_authenticated:
            # Filter sites by user's access
            return queryset.filter(customer=self.request.user)
        return queryset.none()


class DeviceViewSet(viewsets.ModelViewSet):
    queryset = Device.objects.all()
    serializer_class = DeviceSerializer

    def get_queryset(self):
        queryset = Device.objects.select_related('site', 'configuration')
        if self.request.user.is_authenticated:
            # Filter devices by user's site access
            user_sites = self.request.user.sites.all()
            return queryset.filter(site__in=user_sites)
        return queryset.none()

    @action(detail=True, methods=['get'])
    def configuration(self, request, pk=None):
        """Get device configuration"""
        device = self.get_object()
        if hasattr(device, 'configuration'):
            serializer = DeviceConfigurationSerializer(device.configuration)
            return Response(serializer.data)
        return Response({'error': 'Configuration not found'}, status=404)

    @action(detail=True, methods=['post'])
    def update_status(self, request, pk=None):
        """Update device status"""
        device = self.get_object()
        new_status = request.data.get('status')
        if new_status in dict(Device.STATUS_CHOICES):
            device.status = new_status
            device.save()
            serializer = self.get_serializer(device)
            return Response(serializer.data)
        return Response({'error': 'Invalid status'}, status=400)


class FirmwareBundleViewSet(viewsets.ModelViewSet):
    queryset = FirmwareBundle.objects.all()
    serializer_class = FirmwareBundleSerializer

    @action(detail=True, methods=['get'])
    def devices(self, request, pk=None):
        """Get devices compatible with this firmware"""
        firmware = self.get_object()
        devices = Device.objects.filter(
            model__in=firmware.supported_models,
            firmware_version__lt=firmware.version
        )
        serializer = DeviceSerializer(devices, many=True)
        return Response(serializer.data)