from rest_framework import viewsets

from .models import Asset, AssetCycle
from .serializers import AssetCycleSerializer, AssetSerializer


class AssetViewSet(viewsets.ModelViewSet):
    queryset = Asset.objects.all()
    serializer_class = AssetSerializer

    def get_queryset(self):
        queryset = Asset.objects.select_related('site')
        if self.request.user.is_authenticated:
            # Filter assets by user's site access
            user_sites = self.request.user.sites.all()
            return queryset.filter(site__in=user_sites)
        return queryset.none()


class AssetCycleViewSet(viewsets.ModelViewSet):
    queryset = AssetCycle.objects.all()
    serializer_class = AssetCycleSerializer

    def get_queryset(self):
        queryset = AssetCycle.objects.select_related('asset')
        if self.request.user.is_authenticated:
            # Filter by user's accessible assets
            user_sites = self.request.user.sites.all()
            return queryset.filter(asset__site__in=user_sites)
        return queryset.none()