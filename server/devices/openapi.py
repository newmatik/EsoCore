from drf_spectacular.extensions import OpenApiAuthenticationExtension


class APIKeyAuthenticationExtension(OpenApiAuthenticationExtension):
    target_class = "devices.auth.APIKeyAuthentication"
    name = "apiKeyAuth"

    def get_security_definition(self, auto_schema):
        return {
            "type": "apiKey",
            "in": "header",
            "name": "X-API-Key",
            "description": "Device API key passed via X-API-Key header or Authorization: Api-Key <key>",
        }
