from django.urls import include, path
from rest_framework.routers import DefaultRouter

from . import views

router = DefaultRouter()
router.register(r'sites', views.SiteViewSet)
router.register(r'devices', views.DeviceViewSet)
router.register(r'firmware', views.FirmwareBundleViewSet)

urlpatterns = [
    path('', include(router.urls)),
]
