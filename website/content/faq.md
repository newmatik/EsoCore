# Frequently Asked Questions

Common questions about EsoCore, the open-source industrial IoT platform for predictive maintenance and equipment monitoring.

---

## General Questions

### What is EsoCore?

EsoCore is an open-source industrial IoT platform designed for comprehensive equipment monitoring and predictive maintenance in manufacturing environments.
The platform combines edge computing with machine learning to deliver real-time insights into machine health, preventing costly downtime and optimizing
maintenance schedules. EsoCore provides everything needed for modern factory automation: modular sensor modules, edge intelligence, cloud infrastructure,
and seamless integration with existing industrial control systems.

### Is EsoCore really free?

Yes. EsoCore is fully open source under the Apache 2.0 License, which means you can use it commercially, modify it, distribute it, and integrate it into
proprietary systems without paying licensing fees. All hardware designs, firmware source code, and cloud platform components are freely available. While the
core platform is free, optional commercial services like managed cloud hosting, enterprise support, and custom development are available from Newmatik and
partners.

### How does EsoCore differ from proprietary industrial IoT platforms?

EsoCore differentiates itself through complete transparency, vendor independence, and customization freedom. Unlike proprietary platforms, you own the entire
technology stack and can modify it for your specific needs. There are no per-device fees, no forced cloud subscriptions, and no vendor lock-in. You can deploy
on-premises, integrate with any cloud provider, or use the optional managed service. The open-source model also means continuous community-driven improvements
and the ability to fork the project if your needs diverge from the main development path.

### Who develops and maintains EsoCore?

EsoCore is developed and maintained by Newmatik, a European electronics and software company with extensive experience in industrial automation. Development
follows a collaborative partnership model where strategic partners contribute to the platform while maintaining independence. The entire platform is open
source, enabling community contributions and ensuring long-term sustainability beyond any single company.

### What industries and applications is EsoCore designed for?

EsoCore serves diverse industrial applications including CNC machine monitoring, industrial door systems, injection molding equipment, conveyor systems,
packaging lines, pumps, compressors, motors, and general manufacturing equipment. Any industrial machinery that requires condition monitoring, predictive
maintenance, or safety compliance can benefit from EsoCore. The modular sensor architecture enables customization for industry-specific requirements.

---

## Technical Questions

### What hardware does EsoCore require?

EsoCore consists of an Edge device (STM32H747-based industrial controller) and modular RS-485 sensor modules. The Edge device handles data processing, cloud
connectivity, and fieldbus integration. Sensor modules connect via industrial RS-485 bus and include vibration sensors, temperature sensors, acoustic sensors,
current sensors, pressure sensors, and more. Reference hardware designs are open source, allowing partners to manufacture devices or customize for specific
applications.

### Can EsoCore work offline without cloud connectivity?

Yes. Edge intelligence is a core design principle. The Edge device processes sensor data locally using TinyML models for anomaly detection, stores data on
industrial SD cards, and continues monitoring even during network outages. Local buffering supports 30+ days of offline operation with automatic sync when
connectivity returns. Critical alerts can be delivered via local fieldbus integration or on-device displays without requiring cloud connectivity.

### What programming languages and frameworks does EsoCore use?

The firmware is built on Zephyr RTOS using C for embedded systems programming. The cloud platform uses Django REST Framework (Python) for the backend API and
Nuxt.js (Vue.js/TypeScript) for the frontend portal. All components follow modern development practices with comprehensive documentation, making it accessible
to developers familiar with these widely-used technologies.

### Does EsoCore support existing industrial protocols?

Yes. EsoCore natively supports Modbus RTU/TCP, PROFIBUS DP, and PROFINET Device protocols, enabling seamless integration with PLCs, SCADA systems, and existing
factory automation infrastructure. The platform also provides REST APIs for modern web-based integrations and can be extended to support additional protocols as
needed.

### How does edge AI and TinyML work in EsoCore?

EsoCore runs lightweight machine learning models directly on the STM32 microcontroller using TensorFlow Lite Micro. Models are optimized to fit in <16KB of
flash memory while providing real-time anomaly detection with <100ms inference latency. This enables immediate response to equipment issues without cloud
round-trip delays. Models can be updated over-the-air as equipment behavior changes or new failure modes are identified.

### What sensors are supported?

EsoCore supports a comprehensive sensor suite including 3-axis vibration sensors, acoustic sensors (audible and ultrasonic), temperature sensors (NTC, RTD,
thermocouple, IR), pressure sensors, current sensors, air quality sensors, light sensors, oil quality sensors, and proximity/position sensors. The modular
RS-485 architecture allows new sensor types to be added without redesigning the Edge hardware. Partners can develop proprietary sensor modules for specific
applications.

### Can I add custom sensors to EsoCore?

Absolutely. The RS-485 sensor bus architecture is specifically designed for extensibility. You can develop custom sensor modules with local MCUs that communicate
via the standardized protocol. These can be proprietary sensors for competitive advantages, industry-specific sensors, or experimental sensor types. The Edge
device auto-discovers new sensors and their capabilities at runtime without firmware changes.

---

## Deployment and Integration Questions

### How long does it take to deploy EsoCore?

Initial pilot deployments typically take 1-2 days for hardware installation and basic configuration. The modular design enables rapid expansion—adding monitoring
to additional machines can be accomplished in hours rather than weeks. Cloud platform deployment takes 2-4 hours for self-hosted installations or is immediate
with managed hosting. Most organizations achieve full production deployment within 2-4 weeks including integration with existing systems.

### Can EsoCore integrate with my existing MES, SCADA, or ERP systems?

Yes. EsoCore provides multiple integration pathways: industrial fieldbus protocols (Modbus, PROFIBUS, PROFINET) for SCADA integration, comprehensive REST APIs
for MES/ERP integration, and webhook support for event-driven workflows. The open-source nature enables custom integrations for proprietary systems. Documentation
includes integration examples and best practices for common industrial software platforms.

### Does EsoCore require changes to my existing machinery?

No. EsoCore is designed for non-invasive deployment. Sensors mount externally or connect to existing measurement points without modifying machine internals.
Safety I/O integrates with existing emergency stop circuits and safety systems without replacing certified components. The system monitors and augments existing
control systems rather than replacing them.

### What network infrastructure does EsoCore need?

EsoCore supports both Ethernet (with PoE option) and Wi-Fi connectivity. Recommended deployment uses industrial Ethernet for reliability, but Wi-Fi works well for
retrofit installations. The Edge device requires one IP address and operates on standard factory networks. Firewall configuration is minimal—outbound HTTPS for
cloud sync and optional inbound HTTPS for web UI access. The platform is designed for industrial network environments with intermittent connectivity.

### Can I run EsoCore entirely on-premises without cloud services?

Yes. The Django backend can be deployed on-premises providing all data management, analytics, and API functionality without external cloud dependencies. Local
deployment ensures complete data sovereignty and eliminates internet connectivity requirements. The platform architecture separates edge, backend, and frontend
components allowing flexible deployment topologies including air-gapped installations for sensitive environments.

---

## Predictive Maintenance and ROI Questions

### How does predictive maintenance reduce costs?

Predictive maintenance shifts from reactive (repair after failure) or time-based (scheduled regardless of condition) to condition-based maintenance. This reduces
unplanned downtime by 30-50%, extends equipment lifespan by 20-40%, and reduces maintenance costs by 20-30% according to industry studies. EsoCore enables this
by continuously monitoring equipment health and alerting you to developing issues before they cause failures, allowing planned maintenance during scheduled
downtime rather than emergency repairs during production hours.

### What is the typical ROI for industrial IoT monitoring?

ROI varies by application but typically ranges from 200-400% within the first year. A single avoided unplanned outage on critical production equipment often pays
for the monitoring system. Additional value comes from extended equipment life, reduced spare parts inventory, optimized maintenance schedules, and improved
operational efficiency. The open-source nature of EsoCore reduces upfront costs compared to proprietary platforms, improving ROI further.

### How accurate is the anomaly detection?

EsoCore's TinyML models achieve >95% accuracy in identifying bearing faults, motor imbalances, and mechanical anomalies on test datasets. False positive rates are
typically <5% for maintenance alerts and <1% for safety-critical events. Models improve over time through federated learning as more data is collected across the
fleet. The system provides confidence scores with each prediction allowing you to tune sensitivity for your specific risk tolerance.

### Can EsoCore replace my existing maintenance program?

EsoCore augments rather than replaces traditional maintenance programs. It enables the transition from time-based to condition-based maintenance while maintaining
required regulatory inspections and OEM-recommended service intervals. The system helps prioritize maintenance activities, optimize spare parts inventory, and
provide data-driven justification for maintenance decisions. Most organizations use EsoCore to reduce unnecessary preventive maintenance while preventing
unexpected failures.

---

## Security and Compliance Questions

### Is open-source software secure for industrial applications?

Yes. Open-source software often provides superior security because the code is transparently auditable by security researchers and the community. EsoCore follows
security best practices including TLS encryption, signed firmware updates, device authentication, and secure boot. The Apache 2.0 License includes patent
protection. Regular security updates are provided by Newmatik and the open-source community can contribute security improvements. Many critical industrial and
government systems rely on open-source components.

### How does EsoCore handle data privacy?

EsoCore provides complete data ownership and control. Edge processing keeps sensitive data on-device with only aggregated metrics transmitted to the cloud.
You control what data leaves your premises. On-premises deployment options provide complete data sovereignty. The platform is designed to support GDPR, HIPAA,
and other data protection regulations. No data is shared with third parties without your explicit configuration and consent.

### What security certifications does EsoCore have?

The hardware designs follow EN ISO 13849 for functional safety and EN 61000 for EMC compliance. The platform architecture supports security best practices aligned
with IEC 62443 for industrial automation security. As an open-source platform, individual deployments can pursue necessary certifications for their specific
regulatory requirements. Newmatik provides certification support services for partners requiring specific compliance documentation.

### Can EsoCore be used in safety-critical applications?

EsoCore's safety I/O subsystem is designed for EN ISO 13849 compliance with <10ms response times for emergency stops, light curtains, and safety edges. However,
EsoCore should be used as a monitoring and predictive system alongside, not replacing, certified safety systems. The platform detects safety events and logs them
for compliance but does not replace safety-rated PLCs or control systems in safety-critical paths. Always consult with safety engineers for your specific
application requirements.

---

## Cost and Licensing Questions

### How much does EsoCore cost?

The core EsoCore platform (software, firmware, and hardware designs) is free under the Apache 2.0 License. Hardware costs depend on your deployment size and
sensor requirements—typical installations range from $500-2000 per monitored machine for hardware. Optional commercial services include managed cloud hosting
($10-50/device/month), enterprise support contracts, custom development, and certification assistance. Unlike proprietary platforms, there are no per-device
licensing fees or forced subscription models.

### Are there ongoing costs after initial deployment?

Ongoing costs depend on your deployment model. Self-hosted installations have minimal operating costs (server hosting and maintenance). Managed cloud hosting has
per-device fees but eliminates infrastructure management overhead. There are no mandatory licensing fees, but you may choose optional support contracts or
software updates services. The open-source nature means you are never locked into ongoing payments—you can switch from managed to self-hosted at any time.

### Can I sell products based on EsoCore?

Yes. The Apache 2.0 License explicitly permits commercial use, modification, and distribution. You can build products on EsoCore, sell them under your own brand,
and add proprietary features without sharing your modifications. Many partners use EsoCore as the foundation for industry-specific solutions. The license only
requires attribution—you must include the Apache 2.0 License notice in your product documentation.

### What happens if Newmatik stops supporting EsoCore?

The open-source nature provides protection against vendor abandonment. All source code, hardware designs, and documentation are publicly available, enabling the
community or any organization to continue development. The Apache 2.0 License allows anyone to fork the project and maintain it independently. Partners can establish
their own support and development capabilities. This is fundamentally different from proprietary platforms where vendor decisions directly impact your ability to
maintain your systems.

---

## Getting Started Questions

### How do I get started with EsoCore?

Start by exploring the comprehensive documentation at [www.esocore.com/docs](/docs). The [Quick Start Guide](/docs/getting-started/quick-start) walks through initial
setup and your first sensor deployment. For pilot projects, we recommend starting with 1-3 monitored machines to validate the approach before facility-wide deployment.
Newmatik and partners offer pilot program support including hardware, installation assistance, and training. The GitHub repository contains all source code and
reference designs.

### Do I need specialized skills to deploy EsoCore?

Basic industrial automation knowledge is helpful but not required. The system is designed for deployment by maintenance technicians with guidance from the documentation.
For custom development, firmware modifications require embedded systems programming skills (C/Zephyr RTOS), while cloud customization uses widely-known technologies
(Python/Django, Vue.js). Most organizations successfully deploy standard configurations without custom development. Newmatik offers training and professional services
for complex deployments.

### Where can I get hardware and sensors?

Reference hardware designs are open source, enabling manufacturing by partners or your preferred contract manufacturer. Newmatik offers pre-built hardware for evaluation
and production deployments. Several partners provide certified hardware with regional support. Because the designs are open, you are never dependent on a single source
for hardware—you can manufacture it yourself, use multiple suppliers, or switch vendors without platform lock-in.

### Is there a demo or trial available?

Yes. Virtual demos are available showing the cloud platform interface, dashboard analytics, and alert workflows. Newmatik and partners offer evaluation hardware for
pilot deployments, typically 30-90 day trials with 1-3 edge devices and sensor kits. The open-source nature means you can also set up your own evaluation environment
using the source code and off-the-shelf development boards before committing to production hardware.

### What support is available?

Community support is available through GitHub discussions and issue tracking. The comprehensive documentation covers most common deployment scenarios. Newmatik offers
commercial support contracts with guaranteed response times, priority bug fixes, and direct engineering consultation. Partners provide regional support services tailored
to specific industries. Because the platform is open source, you can also hire independent consultants or build internal expertise without vendor dependency.

---

## Comparison Questions

### How does EsoCore compare to MachineMetrics?

MachineMetrics is a mature proprietary platform focused on production monitoring and OEE optimization for CNC machines. EsoCore differentiates through open-source
licensing (no vendor lock-in), edge-first TinyML architecture (lower latency, offline operation), safety system integration (EN ISO 13849 compliance), and cross-domain
capability (doors, CNC, and general equipment on one platform). MachineMetrics offers more polished UI and established customer base, while EsoCore provides customization
freedom and lower total cost of ownership.

### How does EsoCore compare to Senseye?

Senseye (Siemens) is an enterprise-focused predictive maintenance platform emphasizing advanced AI/ML analytics for mixed equipment fleets. EsoCore differentiates through
embedded edge intelligence (vs cloud-centric), open-source flexibility (vs proprietary), and integrated hardware-software solution (vs software-only). Senseye excels at
large enterprise deployments with existing IoT infrastructure, while EsoCore provides complete edge-to-cloud solution with more accessible pricing for mid-market manufacturers.

### How does EsoCore compare to proprietary OEM monitoring solutions?

OEM-specific solutions (like ASSA ABLOY Insight for doors or Fanuc FIELD for CNC machines) provide tight integration with specific equipment but lack cross-platform
capability. EsoCore enables monitoring diverse equipment types on a single platform, avoiding the complexity and cost of managing multiple vendor solutions. The open-source
nature provides insurance against vendor decisions, pricing changes, or product discontinuation. OEM solutions excel at equipment-specific optimization, while EsoCore
provides facility-wide visibility and vendor independence.

### Can I migrate from another IoT platform to EsoCore?

Yes. The open architecture and standard protocols facilitate migration from proprietary platforms. The REST API enables data import from existing systems. The modular
design allows gradual migration—new equipment deployed with EsoCore while legacy systems remain operational. Complete documentation of data formats and protocols simplifies
integration projects. Many organizations run EsoCore alongside existing systems during transition periods.

---

## Advanced Technical Questions

### Can I modify the firmware or hardware designs?

Absolutely. The Apache 2.0 License explicitly grants modification rights. All firmware source code, hardware schematics, PCB layouts, and component bills of materials are
publicly available. You can modify for your specific needs, fix bugs, add features, or adapt for novel applications. Modifications can remain proprietary or be contributed
back to the community. The modular architecture makes targeted modifications feasible without understanding the entire codebase.

### Does EsoCore support federated learning?

Yes. The platform architecture supports federated learning where models improve across the fleet without sharing raw sensor data. Each edge device can participate in
distributed training while keeping sensitive data on-premises. Updated models are validated and distributed via OTA updates. This approach provides the benefits of
fleet-wide intelligence while maintaining data privacy and regulatory compliance.

### Can EsoCore handle high-frequency data acquisition?

Yes. The vibration sensors support sampling rates up to 3kHz for detailed mechanical analysis. The Edge device includes dedicated signal processing for FFT analysis,
bandpass filtering, and statistical computations. High-frequency data is processed locally with only aggregated metrics transmitted to the cloud, preventing bandwidth
saturation. The firmware architecture prioritizes real-time data acquisition over other tasks to ensure deterministic latency.

### How does the OTA update mechanism work?

Firmware updates use A/B partition scheme with signed images (Ed25519 or ECDSA signatures). The Edge device downloads, verifies, and installs updates to the inactive
partition, then reboots to test new firmware. Automatic rollback occurs if the new firmware fails health checks. Staged rollouts (5%/25%/100%) prevent fleet-wide issues.
The entire update process is designed for industrial reliability with fail-safe mechanisms and minimal downtime.

---

## Contact and Community

### How do I contribute to EsoCore?

Contributions are welcome through the GitHub repository. Start by reviewing the contribution guidelines, opening issues for bugs or feature requests, and submitting pull
requests for improvements. The project follows standard open-source governance with code review and testing before merging. Documentation improvements, test cases, and
use case examples are especially valuable. Join the community discussions to connect with other developers and users.

### How do I report security vulnerabilities?

Security issues should be reported privately to security@newmatik.com rather than public GitHub issues. We follow responsible disclosure practices with acknowledgment
within 48 hours and patches typically available within 7-14 days depending on severity. Security researchers who report valid vulnerabilities are credited in release
notes (with permission).

### Are there partnerships or collaboration opportunities?

Yes. Newmatik actively seeks strategic partners for industry-specific customizations, regional distribution, and application development. The collaborative partnership
model enables shared platform development costs while maintaining partner independence. Contact partnerships@newmatik.com for opportunities. The open-source license also
permits independent commercial activities without formal partnership agreements.

### Where can I get professional services and training?

Newmatik offers professional services including custom development, system integration, certification support, and operator training. Regional partners provide localized
support and services. Because the platform is open source, you can also engage independent consultants or system integrators familiar with the technologies. Training
materials and documentation are publicly available for self-service learning.

---

## Additional Resources

For more information about EsoCore:

- **Documentation**: [Complete technical documentation](/docs)
- **GitHub Repository**: [Source code and hardware designs](https://github.com/newmatik/EsoCore)
- **Getting Started**: [Quick start guide](/docs/getting-started/quick-start)
- **Hardware Specifications**: [Edge device](/docs/esocore-edge) and [Sensor modules](/docs/esocore-sensors)
- **API Documentation**: [REST API specification](/docs/api-specification)
- **Business Information**: [Partnership model](/docs/business-model-and-partnerships)

Have a question not answered here? Contact us or open a discussion on GitHub.
