from django.db.models import QuerySet
from rest_framework.pagination import PageNumberPagination


def with_stable_ordering(queryset):
    """Return the queryset ordered so that its pages never overlap.

    Pages are cut with LIMIT/OFFSET. Without a total order the database may
    return rows in a different order for each page, so a client that reads
    all pages can see a row twice and miss another. The primary key is
    appended as a tie-breaker to whatever ordering the view asked for.
    """
    if not isinstance(queryset, QuerySet) or queryset.query.is_sliced:
        return queryset

    meta = queryset.query.get_meta()
    ordering = list(queryset.query.order_by)
    if not ordering and queryset.query.default_ordering:
        ordering = list(meta.ordering or [])

    pk_names = {"pk", meta.pk.name}
    if any(str(field).lstrip("-") in pk_names for field in ordering):
        return queryset

    return queryset.order_by(*ordering, "pk")


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

    def paginate_queryset(self, queryset, request, view=None):
        return super().paginate_queryset(with_stable_ordering(queryset), request, view)
