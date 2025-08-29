from django.urls import include, path
from rest_framework.routers import DefaultRouter

from . import views

router = DefaultRouter()
router.register(r'events', views.SystemEventViewSet)
router.register(r'rules', views.AlertRuleViewSet)
router.register(r'notifications', views.NotificationQueueViewSet)

urlpatterns = [
    path('', include(router.urls)),
]
