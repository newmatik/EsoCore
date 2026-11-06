"""Site checks for management writes.

List and retrieve already follow the caller's sites. Create and update have
to apply the same limit to the site, device, and asset values they store.
"""

from rest_framework import serializers


def _caller_sites(serializer):
    request = serializer.context.get("request")
    user = getattr(request, "user", None)
    if user is None or not user.is_authenticated:
        return None
    return getattr(user, "sites", None)


def require_caller_site(serializer, site):
    """Return site when the authenticated caller can already read it.

    None is left unchanged so an optional relation can stay empty. A missing
    request, an anonymous caller, and a site owned by someone else are rejected.
    """
    if site is None:
        return site
    sites = _caller_sites(serializer)
    if sites is None or not sites.filter(pk=site.pk).exists():
        raise serializers.ValidationError("Not permitted")
    return site


def require_caller_device(serializer, device):
    """Return device when its site belongs to the caller."""
    if device is None:
        return device
    require_caller_site(serializer, device.site)
    return device


def require_caller_asset(serializer, asset):
    """Return asset when its site belongs to the caller."""
    if asset is None:
        return asset
    require_caller_site(serializer, asset.site)
    return asset
