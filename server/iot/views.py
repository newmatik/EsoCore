import hashlib
import logging
import uuid
from datetime import datetime

from django.utils import timezone
from django.utils.dateparse import parse_datetime
from drf_spectacular.utils import OpenApiResponse, extend_schema, inline_serializer
from rest_framework import serializers, status
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import AllowAny
from rest_framework.response import Response

from devices.models import Device, FirmwareBundle
from events.models import SystemEvent
from telemetry.models import TelemetryPacket, TelemetryPoint

logger = logging.getLogger(__name__)


def _authenticate_device(request):
    """
    Authenticate a device from request headers (X-Device-Id + X-Auth-Key / X-API-Key).

    Returns (Device, None) on success or (None, Response) on failure.
    """
    device_id = request.META.get("HTTP_X_DEVICE_ID")
    api_key = request.META.get("HTTP_X_AUTH_KEY") or request.META.get("HTTP_X_API_KEY")

    if not device_id or not api_key:
        return None, Response(
            {"error": "Missing required authentication headers"},
            status=status.HTTP_401_UNAUTHORIZED,
        )

    try:
        device = Device.objects.get(id=device_id, api_key=api_key)
    except Device.DoesNotExist:
        return None, Response(
            {"error": "Device not found or invalid API key"},
            status=status.HTTP_401_UNAUTHORIZED,
        )

    return device, None


@extend_schema(
    request=None,
    responses=inline_serializer(
        name="AuthHandshakeResponse",
        fields={
            "nonce": serializers.CharField(),
            "server_time": serializers.CharField(),
            "version": serializers.CharField(),
        },
    ),
    summary="Device authentication handshake",
    tags=["iot"],
)
@api_view(["POST"])
@permission_classes([AllowAny])
def auth_handshake(request):
    """Device authentication handshake endpoint"""
    # Return server time and nonce for device authentication
    nonce = str(uuid.uuid4())
    server_time = datetime.utcnow().isoformat() + "Z"

    return Response({"nonce": nonce, "server_time": server_time, "version": "1.0.0"})


@extend_schema(
    request=serializers.ListSerializer(
        child=inline_serializer(
            name="TelemetryItem",
            fields={
                "timestamp": serializers.CharField(),
                "metric": serializers.CharField(),
                "value": serializers.FloatField(),
                "unit": serializers.CharField(required=False),
                "meta": serializers.DictField(required=False),
            },
        )
    ),
    responses={
        200: inline_serializer(
            name="TelemetryBatchResponse",
            fields={
                "accepted": serializers.IntegerField(),
                "duplicates": serializers.IntegerField(),
                "rejected": serializers.IntegerField(),
            },
        ),
        409: OpenApiResponse(description="Duplicate upload"),
    },
    summary="Batch telemetry ingestion",
    tags=["iot"],
)
@api_view(["POST"])
@permission_classes([AllowAny])
def telemetry_batch(request):
    """Batch telemetry data ingestion"""
    device, err_response = _authenticate_device(request)
    if err_response:
        return err_response

    try:
        idempotency_key = request.META.get("HTTP_IDEMPOTENCY_KEY")
        content_sha256 = request.META.get("HTTP_CONTENT_SHA256")

        if not idempotency_key:
            return Response(
                {"error": "Missing required Idempotency-Key header"},
                status=status.HTTP_400_BAD_REQUEST,
            )

        # Check for duplicate upload
        if TelemetryPacket.objects.filter(
            device=device, upload_id=idempotency_key
        ).exists():
            return Response(
                {"accepted": 0, "duplicates": 1, "rejected": 0},
                status=status.HTTP_409_CONFLICT,
            )

        # Process batch data
        batch_data = request.data
        if not isinstance(batch_data, list):
            return Response(
                {"error": "Batch data must be a list"},
                status=status.HTTP_400_BAD_REQUEST,
            )

        # Optional: verify content hash if provided
        if content_sha256:
            try:
                raw_body = request.body or b""
                calculated = hashlib.sha256(raw_body).hexdigest()
                if calculated != content_sha256:
                    return Response(
                        {"error": "Content-SHA256 mismatch"},
                        status=status.HTTP_400_BAD_REQUEST,
                    )
            except Exception:
                pass

        # Create telemetry packet record
        packet = TelemetryPacket.objects.create(
            device=device,
            upload_id=idempotency_key,
            checksum=content_sha256 or "",
            record_count=len(batch_data),
            status="processing",
        )

        # Process individual telemetry points
        accepted = 0
        rejected = 0

        for item in batch_data:
            try:
                # Parse timestamp (expect ISO-8601 string)
                ts = item.get("timestamp")
                ts_parsed = parse_datetime(ts) if isinstance(ts, str) else ts
                if ts_parsed is None:
                    ts_parsed = timezone.now()
                TelemetryPoint.objects.create(
                    device=device,
                    timestamp=ts_parsed,
                    metric=item.get("metric"),
                    value=item.get("value"),
                    unit=item.get("unit", ""),
                    meta=item.get("meta", {}),
                )
                accepted += 1
            except Exception:
                rejected += 1

        # Update packet status
        packet.status = "processed"
        packet.record_count = accepted
        packet.processed_at = timezone.now()
        packet.save()

        # Update device last_seen
        device.last_seen = timezone.now()
        device.save()

        return Response({"accepted": accepted, "duplicates": 0, "rejected": rejected})

    except Exception:
        logger.exception("Unexpected error in telemetry_batch")
        return Response(
            {"error": "Internal server error"},
            status=status.HTTP_500_INTERNAL_SERVER_ERROR,
        )


@extend_schema(
    responses=inline_serializer(
        name="DeviceConfigResponse",
        fields={
            "sampling_rates": serializers.DictField(),
            "thresholds": serializers.DictField(),
            "ntp_servers": serializers.ListField(child=serializers.CharField()),
            "endpoints": serializers.DictField(),
        },
    ),
    summary="Get device configuration",
    tags=["iot"],
)
@api_view(["GET"])
@permission_classes([AllowAny])
def get_config(request):
    """Get device configuration"""
    device, err_response = _authenticate_device(request)
    if err_response:
        return err_response

    if hasattr(device, "configuration"):
        config = device.configuration
        return Response(
            {
                "sampling_rates": config.sampling_rates,
                "thresholds": config.thresholds,
                "ntp_servers": config.ntp_servers,
                "endpoints": config.endpoints,
            }
        )
    return Response(
        {
            "sampling_rates": {},
            "thresholds": {},
            "ntp_servers": [],
            "endpoints": {},
        }
    )


@extend_schema(
    responses=inline_serializer(
        name="OTACheckResponse",
        fields={
            "available": serializers.BooleanField(),
            "version": serializers.CharField(required=False),
            "hash": serializers.CharField(required=False),
            "download_url": serializers.CharField(required=False),
            "release_notes": serializers.CharField(required=False),
            "current_version": serializers.CharField(required=False),
        },
    ),
    summary="Check for OTA updates",
    tags=["iot"],
)
@api_view(["GET"])
@permission_classes([AllowAny])
def ota_check(request):
    """Check for OTA updates"""
    device, err_response = _authenticate_device(request)
    if err_response:
        return err_response

    # Find latest firmware for device's model
    firmware = (
        FirmwareBundle.objects.filter(
            supported_models__contains=[device.model],
            channel__in=[
                "stable",
                device.firmware_version.split(".")[0],
            ],  # Match major version
        )
        .order_by("-created_at")
        .first()
    )

    if firmware and firmware.version != device.firmware_version:
        return Response(
            {
                "available": True,
                "version": firmware.version,
                "hash": firmware.hash,
                "download_url": f"/api/firmware/{firmware.version}/download/",
                "release_notes": firmware.release_notes,
            }
        )
    return Response({"available": False, "current_version": device.firmware_version})


@extend_schema(
    request=inline_serializer(
        name="OTAReportRequest",
        fields={
            "status": serializers.CharField(),
            "version": serializers.CharField(),
            "error": serializers.CharField(required=False),
        },
    ),
    responses=inline_serializer(
        name="OTAReportResponse",
        fields={"status": serializers.CharField()},
    ),
    summary="Report OTA update status",
    tags=["iot"],
)
@api_view(["POST"])
@permission_classes([AllowAny])
def ota_report(request):
    """Report OTA update status"""
    device, err_response = _authenticate_device(request)
    if err_response:
        return err_response

    status_update = request.data.get("status")
    version = request.data.get("version")
    error_message = request.data.get("error", "")

    # Create system event for OTA status
    event_type = f"ota_{status_update}"
    severity = "low"
    if status_update in ["failed", "rollback"]:
        severity = "high"

    SystemEvent.objects.create(
        device=device,
        event_type=event_type,
        severity=severity,
        description=f"OTA update {status_update}: {version}",
        metadata={"ota_version": version, "error": error_message},
    )

    # Update device firmware version if successful
    if status_update == "completed" and version:
        device.firmware_version = version
        device.save()

    return Response({"status": "reported"})
