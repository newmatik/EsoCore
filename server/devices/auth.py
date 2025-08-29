from django.conf import settings
from rest_framework import authentication, exceptions
from rest_framework_api_key.models import APIKey


class APIKeyAuthentication(authentication.BaseAuthentication):
    """
    Custom authentication class for API key authentication
    """

    def authenticate(self, request):
        # Get API key from header
        api_key = self.get_api_key(request)
        if not api_key:
            return None

        # Validate API key
        try:
            key_obj = APIKey.objects.get_from_key(api_key)
            if not key_obj.is_active:
                raise exceptions.AuthenticationFailed("API key is inactive")

            # Return user and key (DRF expects tuple of (user, auth))
            return (key_obj, api_key)
        except APIKey.DoesNotExist:
            raise exceptions.AuthenticationFailed("Invalid API key")

    def get_api_key(self, request):
        """Extract API key from request headers"""
        custom_header = getattr(settings, "API_KEY_CUSTOM_HEADER", "HTTP_X_API_KEY")

        # Try custom header first
        api_key = request.META.get(custom_header)
        if api_key:
            return api_key

        # Try Authorization header with Api-Key prefix
        auth = request.META.get("HTTP_AUTHORIZATION", "")
        if auth.startswith("Api-Key "):
            return auth[8:]  # Remove 'Api-Key ' prefix

        return None
