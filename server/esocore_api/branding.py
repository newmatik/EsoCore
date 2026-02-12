import os

# Brand and product naming for the admin UI and documentation
# You can override these via environment variables: BRAND_NAME, PRODUCT_NAME, ADMIN_TITLE

BRAND_NAME = os.getenv("BRAND_NAME", "Newmatik")
PRODUCT_NAME = os.getenv("PRODUCT_NAME", "EsoCore")


def _default_admin_title() -> str:
    return f"{PRODUCT_NAME} Server"


ADMIN_TITLE = os.getenv("ADMIN_TITLE", _default_admin_title())
