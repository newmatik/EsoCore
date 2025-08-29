"""
URL configuration for sentinel_api project.

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/5.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import include, path
from drf_spectacular.views import (
    SpectacularAPIView,
    SpectacularRedocView,
    SpectacularSwaggerView,
)
from rest_framework import routers
from rest_framework.decorators import api_view
from rest_framework.response import Response
from sentinel_api.branding import ADMIN_TITLE

# API Router for management endpoints
router = routers.DefaultRouter()

# Include app URLs
urlpatterns = [
    path('admin/', admin.site.urls),

    # API Documentation
    path('api/docs/', SpectacularSwaggerView.as_view(url_name='schema'), name='swagger-ui'),
    path('api/schema/', SpectacularAPIView.as_view(), name='schema'),
    path('api/redoc/', SpectacularRedocView.as_view(url_name='schema'), name='redoc'),

    # IoT Device-facing endpoints (API Key auth)
    path('api/iot/v1/', include('iot.urls')),

    # Management API endpoints (Session auth)
    path('api/', include(router.urls)),
    path('api/devices/', include('devices.urls')),
    path('api/assets/', include('assets.urls')),
    path('api/telemetry/', include('telemetry.urls')),
    path('api/events/', include('events.urls')),
    path('api/users/', include('users.urls')),
]


@api_view(["GET"])  # Simple API root listing
def api_root(request):
    return Response({
        "docs": "/api/docs/",
        "schema": "/api/schema/",
        "iot": "/api/iot/v1/",
        "devices": "/api/devices/",
        "assets": "/api/assets/",
        "telemetry": "/api/telemetry/",
        "events": "/api/events/",
        "users": "/api/users/"
    })

urlpatterns += [path("api/", api_root, name="api-root")]

# Customize Django admin titles
admin.site.site_header = ADMIN_TITLE           # left header
admin.site.site_title = ADMIN_TITLE           # browser/tab title
admin.site.index_title = 'Admin'              # right/page title
