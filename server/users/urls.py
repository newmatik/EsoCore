from django.urls import include, path
from rest_framework.routers import DefaultRouter

from . import views

router = DefaultRouter()
router.register(r'profiles', views.UserProfileViewSet)
router.register(r'roles', views.UserDeviceRoleViewSet)
router.register(r'dashboards', views.DashboardViewSet)

urlpatterns = [
    path('', include(router.urls)),
]
