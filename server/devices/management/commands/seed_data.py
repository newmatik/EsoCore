"""
Seed the database with realistic industrial IoT sample data.

Usage:
    python manage.py seed_data
    python manage.py seed_data --clear  # Clear existing data first

Creates:
    - 1 admin user (admin@esocore.local / admin)
    - 2 sites with addresses
    - 8 devices across sites (mixed statuses)
    - 6 assets (CNC, conveyor, pump, robot, motor, door)
    - ~200 telemetry points over the past 24h
    - 10 telemetry packets
    - 12 system events at various severities
    - 4 alert rules
    - Device configurations with realistic thresholds
"""

import hashlib
import random
import uuid
from datetime import timedelta

from django.contrib.auth.models import User
from django.core.management.base import BaseCommand
from django.utils import timezone

from assets.models import Asset, AssetCycle
from devices.models import Device, DeviceConfiguration, FirmwareBundle, Site
from events.models import AlertRule, NotificationQueue, SystemEvent
from telemetry.models import TelemetryPacket, TelemetryPoint, TelemetryWindow
from users.models import Dashboard, UserDeviceRole, UserProfile


class Command(BaseCommand):
    help = "Seed the database with realistic industrial IoT sample data"

    def add_arguments(self, parser):
        parser.add_argument(
            "--clear",
            action="store_true",
            default=True,
            help="Clear existing seed data before seeding (default: True)",
        )

    def handle(self, *args, **options):
        self.stdout.write("Seeding database with sample data...")

        if options["clear"]:
            self._clear_data()

        now = timezone.now()

        admin = self._create_users()
        sites = self._create_sites(admin)
        devices = self._create_devices(sites, now)
        self._create_device_configs(devices)
        assets = self._create_assets(sites)
        self._create_asset_cycles(assets, now)
        self._create_firmware_bundles()
        self._create_telemetry_packets(devices, now)
        self._create_telemetry_points(devices, now)
        self._create_telemetry_windows(devices, now)
        events = self._create_system_events(devices, now)
        rules = self._create_alert_rules(devices, assets)
        self._create_notifications(rules, events)
        self._create_user_roles(admin, sites, devices)
        self._create_dashboards(admin, sites)
        self._create_user_profile(admin)

        self.stdout.write(
            self.style.SUCCESS("Successfully seeded database with sample data!")
        )
        self.stdout.write(
            self.style.SUCCESS("  Login: admin@esocore.local / admin")
        )

    def _clear_data(self):
        """Clear all seeded data."""
        self.stdout.write("  Clearing existing data...")
        NotificationQueue.objects.all().delete()
        AlertRule.objects.all().delete()
        SystemEvent.objects.all().delete()
        TelemetryWindow.objects.all().delete()
        TelemetryPoint.objects.all().delete()
        TelemetryPacket.objects.all().delete()
        AssetCycle.objects.all().delete()
        Asset.objects.all().delete()
        DeviceConfiguration.objects.all().delete()
        Device.objects.all().delete()
        FirmwareBundle.objects.all().delete()
        Dashboard.objects.all().delete()
        UserDeviceRole.objects.all().delete()
        UserProfile.objects.all().delete()
        Site.objects.all().delete()
        User.objects.filter(username="admin").delete()

    def _create_users(self):
        """Create admin user."""
        self.stdout.write("  Creating users...")
        admin, created = User.objects.get_or_create(
            username="admin",
            defaults={
                "email": "admin@esocore.local",
                "first_name": "Admin",
                "last_name": "User",
                "is_staff": True,
                "is_superuser": True,
            },
        )
        if created:
            admin.set_password("admin")
            admin.save()
        return admin

    def _create_sites(self, admin):
        """Create factory sites."""
        self.stdout.write("  Creating sites...")
        site_data = [
            {
                "name": "Warehouse Alpha",
                "address": "Industriestrasse 12, 80939 Munich, Germany",
            },
            {
                "name": "Production Floor Beta",
                "address": "Werksweg 7, 70565 Stuttgart, Germany",
            },
        ]
        sites = []
        for data in site_data:
            site, _ = Site.objects.get_or_create(
                name=data["name"],
                customer=admin,
                defaults={"address": data["address"]},
            )
            sites.append(site)
        return sites

    def _create_devices(self, sites, now):
        """Create IoT edge devices across sites."""
        self.stdout.write("  Creating devices...")
        device_data = [
            # Site 0: Warehouse Alpha
            {
                "serial_number": "ESO-EDGE-2025-001",
                "model": "EsoCore Edge v1.3",
                "firmware_version": "1.3.2",
                "site": sites[0],
                "status": "active",
                "last_seen": now - timedelta(minutes=2),
                "tags": {"zone": "A1", "line": "assembly-1"},
            },
            {
                "serial_number": "ESO-EDGE-2025-002",
                "model": "EsoCore Edge v1.3",
                "firmware_version": "1.3.2",
                "site": sites[0],
                "status": "active",
                "last_seen": now - timedelta(minutes=5),
                "tags": {"zone": "A2", "line": "assembly-2"},
            },
            {
                "serial_number": "ESO-EDGE-2025-003",
                "model": "EsoCore Edge v1.3",
                "firmware_version": "1.3.1",
                "site": sites[0],
                "status": "offline",
                "last_seen": now - timedelta(hours=6),
                "tags": {"zone": "B1", "line": "packaging"},
            },
            {
                "serial_number": "ESO-EDGE-2025-004",
                "model": "EsoCore Edge v1.2",
                "firmware_version": "1.2.8",
                "site": sites[0],
                "status": "maintenance",
                "last_seen": now - timedelta(hours=1),
                "tags": {"zone": "B2", "line": "loading-dock"},
            },
            # Site 1: Production Floor Beta
            {
                "serial_number": "ESO-EDGE-2025-005",
                "model": "EsoCore Edge v1.3",
                "firmware_version": "1.3.2",
                "site": sites[1],
                "status": "active",
                "last_seen": now - timedelta(minutes=1),
                "tags": {"zone": "C1", "line": "cnc-bay"},
            },
            {
                "serial_number": "ESO-EDGE-2025-006",
                "model": "EsoCore Edge v1.3",
                "firmware_version": "1.3.2",
                "site": sites[1],
                "status": "active",
                "last_seen": now - timedelta(minutes=3),
                "tags": {"zone": "C2", "line": "welding"},
            },
            {
                "serial_number": "ESO-EDGE-2025-007",
                "model": "EsoCore Edge v1.3",
                "firmware_version": "1.3.0",
                "site": sites[1],
                "status": "active",
                "last_seen": now - timedelta(minutes=8),
                "tags": {"zone": "D1", "line": "paint-shop"},
            },
            {
                "serial_number": "ESO-EDGE-2025-008",
                "model": "EsoCore Edge v1.2",
                "firmware_version": "1.2.8",
                "site": sites[1],
                "status": "inactive",
                "last_seen": now - timedelta(days=2),
                "tags": {"zone": "D2", "line": "storage"},
            },
        ]

        devices = []
        for i, data in enumerate(device_data):
            api_key = f"eso-key-{i + 1:04d}-{uuid.uuid4().hex[:12]}"
            api_secret = uuid.uuid4().hex
            device, _ = Device.objects.get_or_create(
                serial_number=data["serial_number"],
                defaults={
                    "model": data["model"],
                    "firmware_version": data["firmware_version"],
                    "api_key": api_key,
                    "api_secret": api_secret,
                    "site": data["site"],
                    "status": data["status"],
                    "last_seen": data["last_seen"],
                    "tags": data["tags"],
                },
            )
            devices.append(device)
        return devices

    def _create_device_configs(self, devices):
        """Create device configurations."""
        self.stdout.write("  Creating device configurations...")
        for device in devices:
            DeviceConfiguration.objects.get_or_create(
                device=device,
                defaults={
                    "sampling_rates": {
                        "temperature": 1.0,
                        "vibration": 1000.0,
                        "current": 10.0,
                        "pressure": 1.0,
                    },
                    "thresholds": {
                        "temperature_high": 85.0,
                        "temperature_critical": 95.0,
                        "vibration_warning": 7.5,
                        "vibration_critical": 12.0,
                        "current_max": 25.0,
                        "pressure_low": 2.0,
                        "pressure_high": 8.0,
                    },
                    "ntp_servers": [
                        "0.pool.ntp.org",
                        "1.pool.ntp.org",
                    ],
                    "endpoints": {
                        "telemetry": "/api/iot/v1/telemetry/batch",
                        "config": "/api/iot/v1/config",
                        "ota": "/api/iot/v1/ota/check",
                    },
                },
            )

    def _create_assets(self, sites):
        """Create monitored equipment assets."""
        self.stdout.write("  Creating assets...")
        asset_data = [
            # Warehouse Alpha
            {
                "asset_id": "CNC-001",
                "site": sites[0],
                "asset_type": "cnc",
                "make": "Haas",
                "model": "VF-2SS",
                "serial_number": "HAAS-2023-44871",
                "location": "Zone A1, Bay 3",
                "specifications": {
                    "spindle_speed_max": 12000,
                    "axes": 3,
                    "power_kw": 22.4,
                },
            },
            {
                "asset_id": "CONV-001",
                "site": sites[0],
                "asset_type": "conveyor",
                "make": "Dorner",
                "model": "3200 Series",
                "serial_number": "DOR-2024-00512",
                "location": "Zone B1, Main Line",
                "specifications": {
                    "belt_width_mm": 600,
                    "length_m": 12.5,
                    "speed_m_min": 30,
                },
            },
            {
                "asset_id": "DOOR-001",
                "site": sites[0],
                "asset_type": "door",
                "make": "Hormann",
                "model": "SRT-S",
                "serial_number": "HOR-2022-10234",
                "location": "Loading Dock B2",
                "specifications": {
                    "width_mm": 4000,
                    "height_mm": 4500,
                    "speed_m_s": 2.0,
                },
            },
            # Production Floor Beta
            {
                "asset_id": "CNC-002",
                "site": sites[1],
                "asset_type": "cnc",
                "make": "DMG Mori",
                "model": "CMX 600 V",
                "serial_number": "DMG-2024-78213",
                "location": "Zone C1, Bay 1",
                "specifications": {
                    "spindle_speed_max": 15000,
                    "axes": 5,
                    "power_kw": 35.0,
                },
            },
            {
                "asset_id": "ROBOT-001",
                "site": sites[1],
                "asset_type": "robot",
                "make": "KUKA",
                "model": "KR 16 R2010-2",
                "serial_number": "KUKA-2024-91003",
                "location": "Zone C2, Welding Cell",
                "specifications": {
                    "payload_kg": 16,
                    "reach_mm": 2010,
                    "axes": 6,
                },
            },
            {
                "asset_id": "PUMP-001",
                "site": sites[1],
                "asset_type": "pump",
                "make": "Grundfos",
                "model": "CRE 15-3",
                "serial_number": "GF-2023-55401",
                "location": "Zone D1, Coolant System",
                "specifications": {
                    "flow_m3_h": 15,
                    "head_m": 37,
                    "power_kw": 5.5,
                },
            },
        ]

        assets = []
        for data in asset_data:
            asset, _ = Asset.objects.get_or_create(
                asset_id=data["asset_id"],
                site=data["site"],
                defaults={
                    "asset_type": data["asset_type"],
                    "make": data["make"],
                    "model": data["model"],
                    "serial_number": data["serial_number"],
                    "location": data["location"],
                    "specifications": data["specifications"],
                    "is_active": True,
                },
            )
            assets.append(asset)
        return assets

    def _create_asset_cycles(self, assets, now):
        """Create asset cycle records."""
        self.stdout.write("  Creating asset cycles...")
        # Door cycles
        door = next((a for a in assets if a.asset_type == "door"), None)
        if door:
            for i in range(15):
                start = now - timedelta(hours=24 - i * 1.5)
                duration = timedelta(seconds=random.randint(15, 120))
                AssetCycle.objects.create(
                    asset=door,
                    cycle_type="open_close",
                    start_time=start,
                    end_time=start + duration,
                    duration=duration,
                    metadata={"trigger": "sensor", "direction": "inbound"},
                )

        # CNC spindle cycles
        for cnc in [a for a in assets if a.asset_type == "cnc"]:
            for i in range(8):
                start = now - timedelta(hours=24 - i * 3)
                duration = timedelta(minutes=random.randint(15, 90))
                AssetCycle.objects.create(
                    asset=cnc,
                    cycle_type="machining_cycle",
                    start_time=start,
                    end_time=start + duration,
                    duration=duration,
                    metadata={
                        "program": f"OP{random.randint(100, 999)}",
                        "part_count": random.randint(1, 50),
                    },
                )

    def _create_firmware_bundles(self):
        """Create firmware bundle records."""
        self.stdout.write("  Creating firmware bundles...")
        bundles = [
            {
                "version": "1.2.8",
                "channel": "stable",
                "rollout_policy": "immediate",
                "supported_models": ["EsoCore Edge v1.2"],
                "release_notes": "Bug fixes for RS-485 communication timing.",
            },
            {
                "version": "1.3.0",
                "channel": "stable",
                "rollout_policy": "staged",
                "supported_models": ["EsoCore Edge v1.3"],
                "release_notes": "Added IEPE analog input support and TinyML inference.",
            },
            {
                "version": "1.3.1",
                "channel": "stable",
                "rollout_policy": "staged",
                "supported_models": ["EsoCore Edge v1.3"],
                "release_notes": "Fixed SD card write issue under high vibration.",
            },
            {
                "version": "1.3.2",
                "channel": "stable",
                "rollout_policy": "staged",
                "supported_models": ["EsoCore Edge v1.3", "EsoCore Edge v1.2"],
                "release_notes": "Improved telemetry batch compression. "
                "Added supercap safe-shutdown support.",
            },
            {
                "version": "1.4.0-beta.1",
                "channel": "beta",
                "rollout_policy": "manual",
                "supported_models": ["EsoCore Edge v1.3"],
                "release_notes": "PROFINET I/O device support (experimental).",
            },
        ]
        for b in bundles:
            FirmwareBundle.objects.get_or_create(
                version=b["version"],
                defaults={
                    "hash": hashlib.sha256(
                        b["version"].encode()
                    ).hexdigest(),
                    "channel": b["channel"],
                    "rollout_policy": b["rollout_policy"],
                    "supported_models": b["supported_models"],
                    "release_notes": b["release_notes"],
                },
            )

    def _create_telemetry_packets(self, devices, now):
        """Create telemetry packet (batch upload) records."""
        self.stdout.write("  Creating telemetry packets...")
        active_devices = [d for d in devices if d.status == "active"]
        for device in active_devices:
            for i in range(2):
                upload_id = uuid.uuid4()
                TelemetryPacket.objects.get_or_create(
                    device=device,
                    upload_id=upload_id,
                    defaults={
                        "checksum": hashlib.sha256(
                            str(upload_id).encode()
                        ).hexdigest()[:64],
                        "status": "processed",
                        "record_count": random.randint(50, 200),
                        "processed_at": now - timedelta(hours=i * 6),
                    },
                )

    def _create_telemetry_points(self, devices, now):
        """Create ~200 telemetry data points over the past 24h."""
        self.stdout.write("  Creating telemetry points...")
        metrics = [
            ("temp_ambient", "C", 18.0, 32.0),
            ("temp_motor", "C", 40.0, 88.0),
            ("vibration_rms", "mm/s", 0.5, 10.0),
            ("current_draw", "A", 2.0, 22.0),
            ("pressure_coolant", "bar", 3.0, 7.5),
        ]

        points = []
        active_devices = [d for d in devices if d.status in ("active", "maintenance")]

        for device in active_devices:
            device_metrics = random.sample(metrics, k=min(3, len(metrics)))
            for metric_name, unit, low, high in device_metrics:
                # ~8 points per metric per device over 24h
                for hour_offset in range(0, 24, 3):
                    ts = now - timedelta(
                        hours=hour_offset,
                        minutes=random.randint(0, 59),
                    )
                    value = round(random.uniform(low, high), 2)
                    points.append(
                        TelemetryPoint(
                            device=device,
                            timestamp=ts,
                            metric=metric_name,
                            value=value,
                            unit=unit,
                            meta={
                                "sensor_id": f"{metric_name.split('_')[0]}_01",
                                "sample_rate": 1000 if "vibration" in metric_name else 1,
                            },
                        )
                    )

        TelemetryPoint.objects.bulk_create(points)
        self.stdout.write(f"    Created {len(points)} telemetry points")

    def _create_telemetry_windows(self, devices, now):
        """Create aggregated telemetry windows."""
        self.stdout.write("  Creating telemetry windows...")
        windows = []
        active_devices = [d for d in devices if d.status == "active"]

        for device in active_devices[:4]:
            for hour_offset in range(0, 24, 6):
                ts = now - timedelta(hours=hour_offset)
                for metric in ["vibration_rms", "temp_motor"]:
                    mean_val = round(random.uniform(2.0, 8.0), 2)
                    windows.append(
                        TelemetryWindow(
                            device=device,
                            timestamp=ts,
                            metric=metric,
                            window_size_seconds=3600,
                            statistics={
                                "mean": mean_val,
                                "rms": round(mean_val * 1.1, 2),
                                "max": round(mean_val * 1.5, 2),
                                "min": round(mean_val * 0.5, 2),
                                "std": round(mean_val * 0.2, 2),
                            },
                            sample_count=random.randint(3000, 3600),
                        )
                    )
        TelemetryWindow.objects.bulk_create(windows)

    def _create_system_events(self, devices, now):
        """Create system events with various severities."""
        self.stdout.write("  Creating system events...")
        event_data = [
            # Active events
            {
                "device": devices[2],  # offline device
                "event_type": "connectivity_lost",
                "severity": "high",
                "description": f"Device {devices[2].serial_number} lost connectivity. "
                "Last seen 6 hours ago.",
                "status": "active",
                "created_offset": timedelta(hours=6),
            },
            {
                "device": devices[0],
                "event_type": "vibration_threshold_exceeded",
                "severity": "medium",
                "description": "Vibration RMS exceeded warning threshold (7.5 mm/s) "
                "on sensor vibration_01. Current: 8.2 mm/s.",
                "status": "active",
                "created_offset": timedelta(hours=2),
            },
            {
                "device": devices[4],
                "event_type": "temperature_high",
                "severity": "high",
                "description": "Motor temperature exceeded threshold (85 C). "
                "Current: 87.3 C on CNC-002.",
                "status": "active",
                "created_offset": timedelta(minutes=45),
            },
            {
                "device": devices[5],
                "event_type": "sensor_fault",
                "severity": "medium",
                "description": "Pressure sensor coolant_01 returning invalid readings. "
                "Possible wiring fault.",
                "status": "active",
                "created_offset": timedelta(hours=3),
            },
            # Acknowledged events
            {
                "device": devices[3],  # maintenance device
                "event_type": "safety_input_triggered",
                "severity": "critical",
                "description": "Emergency stop triggered on loading dock B2. "
                "Safety relay opened.",
                "status": "acknowledged",
                "created_offset": timedelta(hours=8),
            },
            {
                "device": devices[1],
                "event_type": "supercap_low",
                "severity": "medium",
                "description": "Supercapacitor voltage below threshold (2.1V). "
                "Safe shutdown may be limited.",
                "status": "acknowledged",
                "created_offset": timedelta(hours=12),
            },
            # Resolved events
            {
                "device": devices[0],
                "event_type": "connectivity_lost",
                "severity": "high",
                "description": f"Device {devices[0].serial_number} lost connectivity "
                "briefly due to network switch restart.",
                "status": "resolved",
                "created_offset": timedelta(hours=18),
            },
            {
                "device": devices[4],
                "event_type": "ota_completed",
                "severity": "low",
                "description": "Firmware update to v1.3.2 completed successfully.",
                "status": "resolved",
                "created_offset": timedelta(days=1),
            },
            {
                "device": devices[5],
                "event_type": "device_boot",
                "severity": "low",
                "description": "Device rebooted after scheduled maintenance window.",
                "status": "resolved",
                "created_offset": timedelta(hours=14),
            },
            {
                "device": devices[6],
                "event_type": "sd_error",
                "severity": "medium",
                "description": "SD card write error detected. Retried successfully "
                "after 3 attempts.",
                "status": "resolved",
                "created_offset": timedelta(hours=20),
            },
            {
                "device": devices[1],
                "event_type": "power_outage",
                "severity": "critical",
                "description": "Mains power lost. Device running on supercap backup. "
                "Duration: 4.2 seconds.",
                "status": "resolved",
                "created_offset": timedelta(hours=16),
            },
            {
                "device": devices[6],
                "event_type": "auth_failed",
                "severity": "medium",
                "description": "Authentication failure from unknown source. "
                "IP: 192.168.1.254. Blocked.",
                "status": "resolved",
                "created_offset": timedelta(hours=22),
            },
        ]

        events = []
        for data in event_data:
            event = SystemEvent.objects.create(
                device=data["device"],
                event_type=data["event_type"],
                severity=data["severity"],
                description=data["description"],
                status=data["status"],
                metadata={
                    "source": "edge",
                    "firmware_version": data["device"].firmware_version,
                },
            )
            events.append(event)

        return events

    def _create_alert_rules(self, devices, assets):
        """Create alert rules for monitoring."""
        self.stdout.write("  Creating alert rules...")
        rules_data = [
            {
                "name": "High Vibration Alert",
                "description": "Triggers when vibration RMS exceeds 7.5 mm/s on any device.",
                "device": None,
                "asset": None,
                "event_type": "vibration_threshold_exceeded",
                "conditions": {
                    "metric": "vibration_rms",
                    "operator": "gt",
                    "value": 7.5,
                    "duration_seconds": 30,
                },
                "severity": "high",
                "notification_channels": ["email", "slack"],
                "cooldown_minutes": 30,
            },
            {
                "name": "Temperature Critical",
                "description": "Triggers when motor temperature exceeds 90 C.",
                "device": None,
                "asset": None,
                "event_type": "temperature_high",
                "conditions": {
                    "metric": "temp_motor",
                    "operator": "gt",
                    "value": 90.0,
                    "duration_seconds": 60,
                },
                "severity": "critical",
                "notification_channels": ["email", "sms", "slack"],
                "cooldown_minutes": 15,
            },
            {
                "name": "Device Offline",
                "description": "Triggers when a device hasn't been seen for 30 minutes.",
                "device": None,
                "asset": None,
                "event_type": "connectivity_lost",
                "conditions": {
                    "metric": "last_seen",
                    "operator": "older_than",
                    "value": 1800,
                },
                "severity": "high",
                "notification_channels": ["email"],
                "cooldown_minutes": 60,
            },
            {
                "name": "CNC Spindle Overload",
                "description": "Triggers when current draw exceeds 20A on CNC machines.",
                "device": devices[0],
                "asset": assets[0],  # CNC-001
                "event_type": "current_overload",
                "conditions": {
                    "metric": "current_draw",
                    "operator": "gt",
                    "value": 20.0,
                    "duration_seconds": 10,
                },
                "severity": "high",
                "notification_channels": ["email", "slack"],
                "cooldown_minutes": 15,
            },
        ]

        rules = []
        for data in rules_data:
            rule, _ = AlertRule.objects.get_or_create(
                name=data["name"],
                device=data["device"],
                asset=data["asset"],
                defaults={
                    "description": data["description"],
                    "event_type": data["event_type"],
                    "conditions": data["conditions"],
                    "severity": data["severity"],
                    "notification_channels": data["notification_channels"],
                    "cooldown_minutes": data["cooldown_minutes"],
                    "is_active": True,
                },
            )
            rules.append(rule)
        return rules

    def _create_notifications(self, rules, events):
        """Create sample notifications in the queue."""
        self.stdout.write("  Creating notification queue entries...")
        active_events = [e for e in events if e.status == "active"]
        for event in active_events[:2]:
            rule = rules[0] if rules else None
            if rule:
                NotificationQueue.objects.create(
                    alert_rule=rule,
                    event=event,
                    recipients=[{"email": "admin@esocore.local", "name": "Admin"}],
                    channel="email",
                    status="sent",
                )

    def _create_user_roles(self, admin, sites, devices):
        """Create user-device role assignments."""
        self.stdout.write("  Creating user roles...")
        for site in sites:
            UserDeviceRole.objects.get_or_create(
                user=admin,
                device=None,
                site=site,
                defaults={
                    "role": "owner",
                    "granted_by": admin,
                },
            )

    def _create_dashboards(self, admin, sites):
        """Create default dashboards."""
        self.stdout.write("  Creating dashboards...")
        Dashboard.objects.get_or_create(
            user=admin,
            name="Main Overview",
            defaults={
                "site": sites[0],
                "layout": {"columns": 2, "rows": 3},
                "widgets": [
                    {"type": "stats_cards", "position": {"row": 0, "col": 0, "span": 2}},
                    {"type": "device_status_chart", "position": {"row": 1, "col": 0}},
                    {"type": "recent_alerts", "position": {"row": 1, "col": 1}},
                    {"type": "telemetry_chart", "position": {"row": 2, "col": 0, "span": 2}},
                ],
                "is_default": True,
            },
        )

    def _create_user_profile(self, admin):
        """Create user profile."""
        self.stdout.write("  Creating user profile...")
        UserProfile.objects.get_or_create(
            user=admin,
            defaults={
                "company": "Newmatik GmbH",
                "phone": "+49 89 123 4567",
                "timezone": "Europe/Berlin",
                "notification_preferences": {
                    "email": True,
                    "sms": False,
                    "slack": True,
                    "push": True,
                },
            },
        )
