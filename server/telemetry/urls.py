from django.urls import include, path
from rest_framework.routers import DefaultRouter

from . import views

router = DefaultRouter()
router.register(r"packets", views.TelemetryPacketViewSet)
router.register(r"points", views.TelemetryPointViewSet)
router.register(r"windows", views.TelemetryWindowViewSet)

urlpatterns = [
    path("", include(router.urls)),
]
