from rest_framework.pagination import PageNumberPagination


class StandardPagination(PageNumberPagination):
    """Page-number pagination whose page size the client may set.

    The portal and the public integration guide pass ``page_size``. DRF's
    ``PageNumberPagination`` ignores that parameter unless
    ``page_size_query_param`` is set, so every list returned the default
    page regardless of what was asked for.
    """

    page_size = 100
    page_size_query_param = "page_size"
    max_page_size = 500
