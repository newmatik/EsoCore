from rest_framework import serializers

from .models import TelemetryPacket, TelemetryPoint, TelemetryWindow


class TelemetryPacketSerializer(serializers.ModelSerializer):
    device_serial = serializers.CharField(source='device.serial_number', read_only=True)

    class Meta:
        model = TelemetryPacket
        fields = [
            'id', 'device', 'device_serial', 'upload_id', 'checksum',
            'status', 'record_count', 'processed_at', 'error_message',
            'created_at'
        ]
        read_only_fields = ['id', 'created_at']


class TelemetryPointSerializer(serializers.ModelSerializer):
    device_serial = serializers.CharField(source='device.serial_number', read_only=True)

    class Meta:
        model = TelemetryPoint
        fields = [
            'id', 'device', 'device_serial', 'timestamp', 'metric',
            'value', 'unit', 'meta'
        ]
        read_only_fields = ['id']


class TelemetryWindowSerializer(serializers.ModelSerializer):
    device_serial = serializers.CharField(source='device.serial_number', read_only=True)

    class Meta:
        model = TelemetryWindow
        fields = [
            'id', 'device', 'device_serial', 'timestamp', 'metric',
            'window_size_seconds', 'statistics', 'sample_count'
        ]
        read_only_fields = ['id']
