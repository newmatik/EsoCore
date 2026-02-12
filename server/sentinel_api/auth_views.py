from django.contrib.auth import authenticate
from drf_spectacular.utils import OpenApiResponse, extend_schema, inline_serializer
from rest_framework import serializers, status
from rest_framework.authtoken.models import Token
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import AllowAny, IsAuthenticated
from rest_framework.response import Response


@extend_schema(
    request=inline_serializer(
        name="LoginRequest",
        fields={
            "username": serializers.CharField(required=False),
            "email": serializers.EmailField(required=False),
            "password": serializers.CharField(),
        },
    ),
    responses={
        200: inline_serializer(
            name="LoginResponse",
            fields={
                "token": serializers.CharField(),
                "user": inline_serializer(
                    name="LoginUser",
                    fields={
                        "id": serializers.IntegerField(),
                        "username": serializers.CharField(),
                        "email": serializers.EmailField(),
                        "first_name": serializers.CharField(),
                        "last_name": serializers.CharField(),
                    },
                ),
            },
        ),
        401: OpenApiResponse(description="Invalid credentials"),
    },
    summary="Login",
)
@api_view(["POST"])
@permission_classes([AllowAny])
def login_view(request):
    """
    Authenticate a user and return a token.

    Accepts username or email + password.
    Returns: { token, user: { id, username, email, first_name, last_name } }
    """
    username = request.data.get("username", "").strip()
    email = request.data.get("email", "").strip()
    password = request.data.get("password", "")

    if not password:
        return Response(
            {"detail": "Password is required."},
            status=status.HTTP_400_BAD_REQUEST,
        )

    # Allow login by email or username
    if email and not username:
        from django.contrib.auth.models import User

        try:
            user_obj = User.objects.get(email=email)
            username = user_obj.username
        except User.DoesNotExist:
            return Response(
                {"detail": "Invalid credentials."},
                status=status.HTTP_401_UNAUTHORIZED,
            )

    if not username:
        return Response(
            {"detail": "Username or email is required."},
            status=status.HTTP_400_BAD_REQUEST,
        )

    user = authenticate(username=username, password=password)
    if user is None:
        return Response(
            {"detail": "Invalid credentials."},
            status=status.HTTP_401_UNAUTHORIZED,
        )

    token, _created = Token.objects.get_or_create(user=user)

    return Response(
        {
            "token": token.key,
            "user": {
                "id": user.pk,
                "username": user.username,
                "email": user.email,
                "first_name": user.first_name,
                "last_name": user.last_name,
            },
        }
    )


@extend_schema(
    request=None,
    responses={200: inline_serializer(name="LogoutResponse", fields={"detail": serializers.CharField()})},
    summary="Logout",
)
@api_view(["POST"])
@permission_classes([IsAuthenticated])
def logout_view(request):
    """Delete the current user's auth token."""
    try:
        request.user.auth_token.delete()
    except Token.DoesNotExist:
        pass
    return Response({"detail": "Logged out."}, status=status.HTTP_200_OK)


@extend_schema(
    responses=inline_serializer(
        name="MeResponse",
        fields={
            "id": serializers.IntegerField(),
            "username": serializers.CharField(),
            "email": serializers.EmailField(),
            "first_name": serializers.CharField(),
            "last_name": serializers.CharField(),
            "is_staff": serializers.BooleanField(),
        },
    ),
    summary="Current user profile",
)
@api_view(["GET"])
@permission_classes([IsAuthenticated])
def me_view(request):
    """Return the currently authenticated user's profile."""
    user = request.user
    return Response(
        {
            "id": user.pk,
            "username": user.username,
            "email": user.email,
            "first_name": user.first_name,
            "last_name": user.last_name,
            "is_staff": user.is_staff,
        }
    )
