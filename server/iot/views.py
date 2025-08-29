import hashlib
import json
import uuid
from datetime import datetime

from devices.models import Device, DeviceConfiguration, FirmwareBundle
from django.utils import timezone
from django.utils.dateparse import parse_datetime
from events.models import SystemEvent
from rest_framework import status
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import AllowAny
from rest_framework.response import Response
from telemetry.models import TelemetryPacket, TelemetryPoint


@api_view(['POST'])
@permission_classes([AllowAny])
def auth_handshake(request):
    """Device authentication handshake endpoint"""
    # Return server time and nonce for device authentication
    nonce = str(uuid.uuid4())
    server_time = datetime.utcnow().isoformat() + 'Z'

    return Response({
        'nonce': nonce,
        'server_time': server_time,
        'version': '1.0.0'
    })


@api_view(['POST'])
@permission_classes([AllowAny])
def telemetry_batch(request):
    """Batch telemetry data ingestion"""
    try:
        device_id = request.META.get('HTTP_X_DEVICE_ID')
        # Match spec header: X-Auth-Key (accept X-API-Key for backward-compat)
        api_key = request.META.get('HTTP_X_AUTH_KEY') or request.META.get('HTTP_X_API_KEY')
        idempotency_key = request.META.get('HTTP_IDEMPOTENCY_KEY')
        content_sha256 = request.META.get('HTTP_CONTENT_SHA256')

        if not all([device_id, idempotency_key]):
            return Response(
                {'error': 'Missing required headers'},
                status=status.HTTP_400_BAD_REQUEST
            )

        # Verify device exists and API key matches
        try:
            device = Device.objects.get(id=device_id, api_key=api_key)
        except Device.DoesNotExist:
            return Response(
                {'error': 'Device not found or invalid API key'},
                status=status.HTTP_401_UNAUTHORIZED
            )

        # Check for duplicate upload
        if TelemetryPacket.objects.filter(
            device=device,
            upload_id=idempotency_key
        ).exists():
            return Response({
                'accepted': 0,
                'duplicates': 1,
                'rejected': 0
            }, status=status.HTTP_409_CONFLICT)

        # Process batch data
        batch_data = request.data
        if not isinstance(batch_data, list):
            return Response(
                {'error': 'Batch data must be a list'},
                status=status.HTTP_400_BAD_REQUEST
            )

        # Optional: verify content hash if provided
        if content_sha256:
            try:
                raw_body = request.body or b''
                calculated = hashlib.sha256(raw_body).hexdigest()
                if calculated != content_sha256:
                    return Response(
                        {'error': 'Content-SHA256 mismatch'},
                        status=status.HTTP_400_BAD_REQUEST
                    )
            except Exception:
                pass

        # Create telemetry packet record
        packet = TelemetryPacket.objects.create(
            device=device,
            upload_id=idempotency_key,
            checksum=content_sha256 or '',
            record_count=len(batch_data),
            status='processing'
        )

        # Process individual telemetry points
        accepted = 0
        rejected = 0

        for item in batch_data:
            try:
                # Parse timestamp (expect ISO-8601 string)
                ts = item.get('timestamp')
                if isinstance(ts, str):
                    ts_parsed = parse_datetime(ts)
                else:
                    ts_parsed = ts
                if ts_parsed is None:
                    ts_parsed = timezone.now()
                TelemetryPoint.objects.create(
                    device=device,
                    timestamp=ts_parsed,
                    metric=item.get('metric'),
                    value=item.get('value'),
                    unit=item.get('unit', ''),
                    meta=item.get('meta', {})
                )
                accepted += 1
            except Exception as e:
                rejected += 1

        # Update packet status
        packet.status = 'processed'
        packet.record_count = accepted
        packet.processed_at = timezone.now()
        packet.save()

        # Update device last_seen
        device.last_seen = timezone.now()
        device.save()

        return Response({
            'accepted': accepted,
            'duplicates': 0,
            'rejected': rejected
        })

    except Exception as e:
        return Response(
            {'error': str(e)},
            status=status.HTTP_500_INTERNAL_SERVER_ERROR
        )


@api_view(['GET'])
@permission_classes([AllowAny])
def get_config(request):
    """Get device configuration"""
    try:
        device_id = request.META.get('HTTP_X_DEVICE_ID')
        api_key = request.META.get('HTTP_X_AUTH_KEY') or request.META.get('HTTP_X_API_KEY')

        device = Device.objects.get(id=device_id, api_key=api_key)

        if hasattr(device, 'configuration'):
            config = device.configuration
            return Response({
                'sampling_rates': config.sampling_rates,
                'thresholds': config.thresholds,
                'ntp_servers': config.ntp_servers,
                'endpoints': config.endpoints,
            })
        else:
            return Response({
                'sampling_rates': {},
                'thresholds': {},
                'ntp_servers': [],
                'endpoints': {},
            })

    except Device.DoesNotExist:
        return Response(
            {'error': 'Device not found'},
            status=status.HTTP_404_NOT_FOUND
        )


@api_view(['GET'])
@permission_classes([AllowAny])
def ota_check(request):
    """Check for OTA updates"""
    try:
        device_id = request.META.get('HTTP_X_DEVICE_ID')
        api_key = request.META.get('HTTP_X_AUTH_KEY') or request.META.get('HTTP_X_API_KEY')

        device = Device.objects.get(id=device_id, api_key=api_key)

        # Find latest firmware for device's model
        firmware = FirmwareBundle.objects.filter(
            supported_models__contains=[device.model],
            channel__in=['stable', device.firmware_version.split('.')[0]]  # Match major version
        ).order_by('-created_at').first()

        if firmware and firmware.version != device.firmware_version:
            return Response({
                'available': True,
                'version': firmware.version,
                'hash': firmware.hash,
                'download_url': f'/api/firmware/{firmware.version}/download/',
                'release_notes': firmware.release_notes
            })
        else:
            return Response({
                'available': False,
                'current_version': device.firmware_version
            })

    except Device.DoesNotExist:
        return Response(
            {'error': 'Device not found'},
            status=status.HTTP_404_NOT_FOUND
        )


@api_view(['POST'])
@permission_classes([AllowAny])
def ota_report(request):
    """Report OTA update status"""
    try:
        device_id = request.META.get('HTTP_X_DEVICE_ID')
        api_key = request.META.get('HTTP_X_AUTH_KEY') or request.META.get('HTTP_X_API_KEY')

        device = Device.objects.get(id=device_id, api_key=api_key)

        status_update = request.data.get('status')
        version = request.data.get('version')
        error_message = request.data.get('error', '')

        # Create system event for OTA status
        event_type = f'ota_{status_update}'
        severity = 'low'
        if status_update in ['failed', 'rollback']:
            severity = 'high'

        SystemEvent.objects.create(
            device=device,
            event_type=event_type,
            severity=severity,
            description=f'OTA update {status_update}: {version}',
            metadata={
                'ota_version': version,
                'error': error_message
            }
        )

        # Update device firmware version if successful
        if status_update == 'completed' and version:
            device.firmware_version = version
            device.save()

        return Response({'status': 'reported'})

    except Device.DoesNotExist:
        return Response(
            {'error': 'Device not found'},
            status=status.HTTP_404_NOT_FOUND
        )