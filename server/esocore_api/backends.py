from django.contrib.auth import get_user_model
from django.contrib.auth.backends import ModelBackend

User = get_user_model()


class EmailOrUsernameBackend(ModelBackend):
    """
    Authentication backend that allows login with either username or email.

    Tries username first, then falls back to email lookup.
    All permission checks are inherited from ModelBackend.
    """

    def authenticate(self, request, username=None, password=None, **kwargs):
        if username is None or password is None:
            return None

        # Try username first (default behavior)
        try:
            user = User.objects.get(username=username)
        except User.DoesNotExist:
            # Fall back to email lookup
            try:
                user = User.objects.get(email=username)
            except (User.DoesNotExist, User.MultipleObjectsReturned):
                return None

        if user.check_password(password) and self.user_can_authenticate(user):
            return user
        return None
