from django.urls import path

from . import views

urlpatterns = [
    path('auth/handshake', views.auth_handshake, name='auth_handshake'),
    path('telemetry/batch', views.telemetry_batch, name='telemetry_batch'),
    path('config', views.get_config, name='get_config'),
    path('ota/check', views.ota_check, name='ota_check'),
    path('ota/report', views.ota_report, name='ota_report'),
]
