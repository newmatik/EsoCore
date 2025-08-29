from rest_framework import serializers

from .models import AlertRule, EventLog, NotificationQueue, SystemEvent


class SystemEventSerializer(serializers.ModelSerializer):
    device_serial = serializers.CharField(source='device.serial_number', read_only=True)
    resolved_by_name = serializers.CharField(source='resolved_by.username', read_only=True)

    class Meta:
        model = SystemEvent
        fields = [
            'id', 'device', 'device_serial', 'event_type', 'severity',
            'description', 'status', 'duration', 'resolved_at',
            'resolved_by', 'resolved_by_name', 'metadata', 'created_at', 'updated_at'
        ]
        read_only_fields = ['id', 'created_at', 'updated_at']


class AlertRuleSerializer(serializers.ModelSerializer):
    device_serial = serializers.CharField(source='device.serial_number', read_only=True, allow_null=True)
    asset_name = serializers.CharField(source='asset.asset_id', read_only=True, allow_null=True)

    class Meta:
        model = AlertRule
        fields = [
            'id', 'name', 'description', 'device', 'device_serial', 'asset', 'asset_name',
            'event_type', 'conditions', 'severity', 'is_active', 'notification_channels',
            'escalation_rules', 'cooldown_minutes', 'created_at', 'updated_at'
        ]
        read_only_fields = ['id', 'created_at', 'updated_at']


class NotificationQueueSerializer(serializers.ModelSerializer):
    alert_rule_name = serializers.CharField(source='alert_rule.name', read_only=True)
    event_description = serializers.CharField(source='event.description', read_only=True)

    class Meta:
        model = NotificationQueue
        fields = [
            'id', 'alert_rule', 'alert_rule_name', 'event', 'event_description',
            'recipients', 'channel', 'status', 'retry_count', 'max_retries',
            'next_retry_at', 'sent_at', 'error_message', 'created_at', 'updated_at'
        ]
        read_only_fields = ['id', 'created_at', 'updated_at']


class EventLogSerializer(serializers.ModelSerializer):
    event_description = serializers.CharField(source='event.description', read_only=True)
    user_name = serializers.CharField(source='user.username', read_only=True, allow_null=True)

    class Meta:
        model = EventLog
        fields = [
            'id', 'event', 'event_description', 'action', 'details',
            'user', 'user_name', 'timestamp'
        ]
        read_only_fields = ['id', 'timestamp']
