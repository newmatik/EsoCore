from django.contrib.auth.models import User
from rest_framework import viewsets

from .models import Dashboard, UserDeviceRole, UserProfile
from .serializers import (
    DashboardSerializer,
    UserDeviceRoleSerializer,
    UserProfileSerializer,
)


class UserProfileViewSet(viewsets.ModelViewSet):
    queryset = UserProfile.objects.all()
    serializer_class = UserProfileSerializer

    def get_queryset(self):
        # Users can only see their own profile
        return UserProfile.objects.filter(user=self.request.user)


class UserDeviceRoleViewSet(viewsets.ModelViewSet):
    queryset = UserDeviceRole.objects.all()
    serializer_class = UserDeviceRoleSerializer

    def get_queryset(self):
        queryset = UserDeviceRole.objects.select_related('user', 'device', 'site')
        if self.request.user.is_authenticated:
            # Users can see roles they granted or roles for themselves
            return queryset.filter(
                granted_by=self.request.user
            ) | queryset.filter(
                user=self.request.user
            )
        return queryset.none()


class DashboardViewSet(viewsets.ModelViewSet):
    queryset = Dashboard.objects.all()
    serializer_class = DashboardSerializer

    def get_queryset(self):
        # Users can only see their own dashboards
        return Dashboard.objects.filter(user=self.request.user)

    def perform_create(self, serializer):
        serializer.save(user=self.request.user)