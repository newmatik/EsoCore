from django.urls import include, path
from rest_framework.routers import DefaultRouter

from . import views

router = DefaultRouter()
router.register(r'assets', views.AssetViewSet)
router.register(r'cycles', views.AssetCycleViewSet)

urlpatterns = [
    path('', include(router.urls)),
]
