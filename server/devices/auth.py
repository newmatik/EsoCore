"""Device authentication.

Devices authenticate with their ``X-Device-Id`` and their ``Device.api_key``,
sent as ``X-Auth-Key`` (preferred), ``X-API-Key`` (compatibility) or
``Authorization: Api-Key <key>``. The IoT endpoints and the DRF authentication
class share :func:`device_from_request`, so both accept exactly the same
credentials.
"""

import hmac
import uuid

from django.contrib.auth.models import AnonymousUser
from rest_framework import authentication, exceptions

from .models import Device


class DeviceCredentialsError(Exception):
    """The request carries device credentials that do not match a device."""


def device_key_from_request(request):
    """Return the device API key from the request headers, or None."""
    meta = request.META
    key = meta.get("HTTP_X_AUTH_KEY") or meta.get("HTTP_X_API_KEY")
    if key:
        return key
    auth = meta.get("HTTP_AUTHORIZATION", "")
    if auth.startswith("Api-Key "):
        return auth[len("Api-Key ") :] or None
    return None


def device_from_request(request):
    """
    Return the Device the request's credentials identify.

    Returns None when the request carries no device credentials (no key, or no
    ``X-Device-Id``). Raises DeviceCredentialsError when it carries credentials
    that don't match a device, including a malformed device id.
    """
    key = device_key_from_request(request)
    device_id = request.META.get("HTTP_X_DEVICE_ID")
    if not key or not device_id:
        return None
    try:
        device = Device.objects.get(id=uuid.UUID(device_id))
    except (ValueError, Device.DoesNotExist) as err:
        raise DeviceCredentialsError from err
    if not hmac.compare_digest(device.api_key.encode(), key.encode()):
        raise DeviceCredentialsError
    return device


class DevicePrincipal(AnonymousUser):
    """
    ``request.user`` for a request authenticated by device credentials.

    It identifies the device (``.device``; ``request.auth`` is the same
    Device) but is not a user: ``is_authenticated`` stays False and it has no
    sites, groups or permissions, so device credentials never grant access to
    the user-facing API.
    """

    is_device = True

    def __init__(self, device):
        super().__init__()
        self.device = device

    def __str__(self):
        return f"device:{self.device.pk}"

    def __eq__(self, other):
        return isinstance(other, DevicePrincipal) and other.device.pk == self.device.pk

    def __hash__(self):
        return hash(("device", self.device.pk))


class APIKeyAuthentication(authentication.BaseAuthentication):
    """Authenticate an edge device by ``X-Device-Id`` and its API key."""

    def authenticate(self, request):
        try:
            device = device_from_request(request)
        except DeviceCredentialsError as err:
            raise exceptions.AuthenticationFailed(
                "Device not found or invalid API key"
            ) from err
        if device is None:
            return None
        return (DevicePrincipal(device), device)

    def authenticate_header(self, request):
        return "Api-Key"
