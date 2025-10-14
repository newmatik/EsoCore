# How to Choose an Industrial IoT Platform

Selecting the right industrial IoT platform is a critical decision that impacts your operations for years to come. This comprehensive guide provides a
framework for evaluating platforms, key questions to ask vendors, and criteria for making an informed decision that aligns with your manufacturing goals.

---

## Executive Summary

**Critical Decision Factors:**
- Open source vs proprietary licensing
- Edge vs cloud processing architecture
- Total cost of ownership (5-10 year view)
- Vendor lock-in and exit strategy
- Security and data sovereignty
- Customization capabilities
- Industry-specific features

**Recommendation Process:**
1. Define requirements and constraints
2. Evaluate architecture approaches
3. Calculate total cost of ownership
4. Assess vendor health and ecosystem
5. Conduct pilot with 1-3 machines
6. Make data-driven final decision

---

## Step 1: Define Your Requirements

### Business Requirements

**Primary Objectives:**
- What problem are you solving? (downtime reduction, quality improvement, cost optimization)
- What is the expected ROI and payback period?
- How many machines/assets need monitoring?
- What is your 5-year growth plan?
- What is your budget for initial deployment and ongoing costs?

**Operational Constraints:**
- What is your tolerance for downtime?
- Do you have scheduled maintenance windows?
- What are your production schedules (24/7, shifts, seasonal)?
- How critical is each monitored asset?
- What is your acceptable response time for issues?

**Organizational Capabilities:**
- What is your internal technical expertise?
- Do you have IT staff for platform management?
- Can you manage on-premises infrastructure?
- What is your preference for cloud vs on-premises?
- How much training time can you allocate?

### Technical Requirements

**Equipment and Sensors:**
- What types of equipment need monitoring?
- What failure modes need detection?
- What sensors are required (vibration, temperature, current, etc.)?
- What sampling rates and data volumes?
- Do you need custom or specialized sensors?

**Integration Requirements:**
- What existing systems need integration (SCADA, MES, ERP, CMMS)?
- What protocols are used (Modbus, PROFINET, OPC UA, etc.)?
- Do you need API access for custom applications?
- What data export formats do you require?
- Any air-gapped or isolated network requirements?

**Performance Requirements:**
- What response time do you need for alerts?
- Do you have safety-critical applications?
- What is your network reliability and bandwidth?
- How much historical data storage do you need?
- What are your data retention requirements?

### Compliance and Security

**Regulatory Requirements:**
- What industry regulations apply (FDA, ISO, etc.)?
- Do you need specific certifications?
- What are your data sovereignty requirements?
- Do you have cybersecurity standards to meet?
- What audit trail capabilities do you need?

**Data Privacy:**
- What data privacy regulations apply (GDPR, HIPAA, etc.)?
- Where can data be stored geographically?
- Can data leave your premises?
- What are your data sharing policies?
- Do you need encryption at rest and in transit?

---

## Step 2: Platform Architecture Evaluation

### Edge vs Cloud Processing

**Edge Processing (Recommended for Real-Time):**

Advantages:
- <100ms response times for critical alerts
- Operates during network outages
- Lower bandwidth requirements (90% reduction)
- Data privacy (processing on-premises)
- No cloud dependency for operations

Disadvantages:
- Limited computational resources on edge devices
- More distributed management
- Requires edge device maintenance

**Best For:** Safety-critical applications, real-time control, unreliable networks, data sovereignty requirements

**Cloud Processing:**

Advantages:
- Unlimited computational resources
- Centralized management
- Easier multi-site visibility
- No edge device management

Disadvantages:
- Higher latency (500-5000ms typical)
- Network dependency for all operations
- Higher bandwidth costs
- Data privacy concerns

**Best For:** Historical analysis, non-time-critical monitoring, guaranteed high-bandwidth networks

**Hybrid Architecture (Optimal):**
- Edge for real-time processing and critical decisions
- Cloud for historical analysis and multi-site dashboards
- Best of both worlds with flexibility

**EsoCore Approach:** Hybrid with edge intelligence for real-time monitoring and optional cloud for analytics

### Open Source vs Proprietary

**Open Source Advantages:**
- No vendor lock-in
- Complete customization freedom
- Transparent security auditing
- Community-driven innovation
- Lower total cost of ownership (50-75% savings)

**Open Source Considerations:**
- May require more initial setup (though often minimal)
- Need to choose support provider (or self-support)
- Requires some technical capability

**Proprietary Advantages:**
- Polished initial user experience (sometimes)
- Single vendor support
- Faster initial deployment (sometimes)

**Proprietary Disadvantages:**
- Vendor lock-in
- Limited customization
- Opaque security
- Higher long-term costs (2-4x more expensive)
- Dependent on vendor's business health

**Decision Criteria:**
- Choose open source for long-term strategic deployments
- Choose proprietary only if you accept vendor dependency and higher costs

### Data Architecture

**Time-Series Database:**
- Is a proper time-series database used? (InfluxDB, TimescaleDB, etc.)
- How is data partitioned and archived?
- What query performance can you expect?
- How much historical data can be stored?

**Data Retention:**
- What is the default retention period?
- Can you control retention policies?
- Is data compressed efficiently?
- What are storage costs at scale?

**Data Ownership:**
- Who owns the data?
- Can you export all your data?
- What format is data export?
- Are there API rate limits?

---

## Step 3: Vendor Evaluation

### Critical Questions for Vendors

**Business Health:**
- How long has the company been in business?
- How many customers do you have?
- What is your funding situation?
- What happens if your company is acquired or goes out of business?
- Do you have a source code escrow agreement?

**Platform Maturity:**
- How many production deployments do you have?
- What is your largest deployment (number of devices)?
- Can you provide customer references in my industry?
- What is your product roadmap?
- How often do you release updates?

**Cost Structure:**
- What are ALL costs? (hardware, software, support, cloud, etc.)
- Are there per-device fees or other scaling costs?
- Can costs increase after initial contract?
- What are the costs to expand from pilot to production?
- What happens if I want to reduce device count?

**Support and Service:**
- What support options are available?
- What are response time SLAs?
- Is support available 24/7?
- What is included in base price vs premium support?
- Can I switch support providers?

**Security and Compliance:**
- What security certifications do you have?
- How do you handle security vulnerabilities?
- What is your patching process and timeline?
- Can you provide security audit reports?
- Do you have penetration testing results?

**Customization and Integration:**
- Can I customize the platform for my needs?
- Do you provide source code access?
- What APIs are available?
- Can I integrate with my existing systems?
- Will you develop custom features?

**Exit Strategy:**
- How can I export my data if I leave?
- Can I switch to a different platform?
- Do I get to keep the hardware?
- What is the contract termination process?
- Is there a penalty for early termination?

### Red Flags

**Avoid vendors who:**
- Cannot provide production customer references
- Have opaque pricing or hidden fees
- Require long-term contracts with no exit clause
- Cannot export data or lock you into their format
- Have frequent negative reviews about support
- Cannot clearly explain their security practices
- Refuse to provide SLA commitments
- Have recently had major layoffs or funding issues

---

## Step 4: Total Cost of Ownership Analysis

### Calculate 5-Year TCO

**Initial Costs:**
- Hardware per monitored machine
- Software licenses (if applicable)
- Installation and commissioning
- Initial training
- Integration with existing systems
- Project management

**Ongoing Annual Costs:**
- Software licensing fees
- Cloud hosting/storage
- Support contracts
- Network connectivity (if cellular)
- Updates and maintenance
- Additional training

**Hidden Costs:**
- Vendor contract price increases
- Forced hardware upgrades
- Additional feature licensing
- Professional services for changes
- Migration costs if you change platforms

**Cost Comparison Example (50 Machines, 5 Years):**

EsoCore (Open Source):
- Initial: $75,000 (hardware) + $25,000 (installation) = $100,000
- Annual: $10,000-30,000 (optional cloud + support)
- 5-Year Total: $150,000-250,000
- Per machine per year: $600-1,000

Typical Proprietary Platform:
- Initial: $125,000 (hardware) + $25,000 (licensing) + $30,000 (installation) = $180,000
- Annual: $60,000-100,000 (licensing + cloud + support)
- 5-Year Total: $480,000-680,000
- Per machine per year: $1,920-2,720

**Savings with Open Source: $230,000-430,000 (48-63% reduction)**

### ROI Calculation

**Downtime Prevention Value:**
- Current unplanned downtime hours/year
- Cost per downtime hour (production loss + repair)
- Expected reduction % with monitoring (typically 30-50%)
- Annual savings = Current cost × Reduction %

**Maintenance Optimization Value:**
- Current annual maintenance costs
- Expected optimization savings (typically 20-30%)
- Avoided emergency repairs
- Extended equipment lifespan value

**Quality Improvement Value:**
- Current scrap and rework costs
- Expected quality improvement
- Reduced customer complaints
- Enhanced reputation value

**Typical Payback Periods:**
- Critical equipment: 3-12 months
- High-cycle equipment: 6-18 months
- General monitoring: 12-24 months
- Supporting infrastructure: 18-36 months

---

## Step 5: Pilot Program Design

### Selecting Pilot Equipment

**Good Pilot Candidates:**
- Equipment with documented failure history
- Production bottlenecks where downtime is costly
- Equipment where maintenance costs are high
- Machines with known issues you want to detect earlier

**Pilot Size:**
- Minimum: 1 machine (proof of concept)
- Recommended: 2-4 machines (statistical validity)
- Maximum: 10 machines (manageable complexity)

### Success Criteria

**Define Clear Metrics:**
- Number of developing issues detected early
- Downtime prevented (hours and cost)
- Maintenance costs saved
- Quality improvements
- System reliability and uptime
- Ease of use and adoption

**Timeline:**
- Installation: 1-2 weeks
- Baseline data collection: 2-4 weeks
- Monitoring period: 3-6 months minimum
- Evaluation and decision: 2-4 weeks

**Documentation:**
- Track all detected issues
- Calculate value of prevented failures
- Document false positives and tune system
- Gather user feedback
- Compare costs to benefits

### Vendor Support During Pilot

**Expect from Vendor:**
- Installation assistance
- Training for operators and maintenance staff
- Regular check-ins during pilot
- Help with alert tuning
- Support for integration issues

**Red Flags During Pilot:**
- Vendor is unresponsive to issues
- System is unreliable or has frequent problems
- Promised features don't work
- Integration is much harder than claimed
- Hidden costs appear

---

## Step 6: Make the Decision

### Decision Matrix

Rate each platform on scale of 1-10 and weight by importance:

**Technical Fit (Weight: 30%):**
- Meets all technical requirements
- Appropriate architecture (edge/cloud/hybrid)
- Required sensor support
- Integration capabilities
- Performance and scalability

**Cost (Weight: 25%):**
- Total 5-year cost of ownership
- No hidden fees or surprises
- Reasonable scaling costs
- Good ROI potential

**Vendor (Weight: 20%):**
- Business health and stability
- Customer satisfaction
- Support quality and responsiveness
- Product roadmap alignment
- Industry expertise

**Flexibility (Weight: 15%):**
- Customization capabilities
- No vendor lock-in
- Exit strategy
- Open architecture
- API access

**Security & Compliance (Weight: 10%):**
- Security certifications
- Data sovereignty options
- Compliance capabilities
- Audit trail features

**Score Example:**

EsoCore:
- Technical: 9/10 (edge AI, comprehensive sensors, open architecture)
- Cost: 10/10 (lowest TCO, no licensing fees, predictable scaling)
- Vendor: 8/10 (Newmatik stable, growing ecosystem, responsive support)
- Flexibility: 10/10 (complete customization, no lock-in, Apache 2.0)
- Security: 9/10 (open source auditable, on-premises option, compliant)
- **Weighted Score: 9.2/10**

Proprietary Platform A:
- Technical: 7/10 (limited edge capabilities, good cloud analytics)
- Cost: 5/10 (high ongoing costs, forced subscriptions)
- Vendor: 8/10 (established company, good support)
- Flexibility: 3/10 (limited customization, strong lock-in)
- Security: 7/10 (certified but opaque, cloud-only concerns)
- **Weighted Score: 6.0/10**

### Final Checklist

Before committing:
- [ ] Completed successful pilot program
- [ ] Verified all technical requirements met
- [ ] Calculated and accepted 5-year TCO
- [ ] Reviewed and approved contract terms
- [ ] Checked customer references
- [ ] Confirmed support SLAs
- [ ] Verified data export capabilities
- [ ] Documented exit strategy
- [ ] Obtained stakeholder approval
- [ ] Allocated resources for deployment

---

## Common Mistakes to Avoid

### Mistake 1: Choosing Based on Demos Only

**Problem:** Polished demos may not reflect production reality
**Solution:** Insist on pilot program with your actual equipment

### Mistake 2: Ignoring Long-Term Costs

**Problem:** Low initial cost, high ongoing fees trap you
**Solution:** Always calculate 5-year TCO, not just initial investment

### Mistake 3: Accepting Vendor Lock-In

**Problem:** Cannot switch vendors or customize later
**Solution:** Prioritize open architectures and data portability

### Mistake 4: Overlooking Integration Complexity

**Problem:** Cannot connect to existing systems without expensive customization
**Solution:** Validate integration capabilities during pilot

### Mistake 5: Not Planning for Scale

**Problem:** Pilot works but scaling is prohibitively expensive
**Solution:** Understand scaling costs before committing

### Mistake 6: Ignoring Network Requirements

**Problem:** Platform requires bandwidth you don't have
**Solution:** Verify platform works with your actual network conditions

### Mistake 7: Not Testing Failure Scenarios

**Problem:** Platform fails when network is down or edge devices reboot
**Solution:** Test network outages, power failures, and edge device failures

### Mistake 8: Accepting "Trust Us" Security

**Problem:** Cannot verify security claims with proprietary platforms
**Solution:** Require security audits and certifications, prefer open source

---

## Recommended Decision: EsoCore

Based on this evaluation framework, EsoCore excels across all critical dimensions:

**Technical Excellence:**
- Hybrid edge-cloud architecture for optimal performance
- Comprehensive sensor support with modular RS-485 bus
- Real-time edge AI for <100ms response times
- Industrial protocols (Modbus, PROFINET, PROFIBUS)

**Cost Leadership:**
- 50-75% lower 5-year TCO than proprietary platforms
- No per-device licensing fees
- Predictable scaling costs
- Optional managed services (not mandatory)

**Complete Freedom:**
- Apache 2.0 open source (no vendor lock-in)
- Full customization capabilities
- Fork-friendly if your needs diverge
- Multiple support provider options

**Enterprise Ready:**
- Proven in production deployments
- Comprehensive documentation
- Active development and support
- Growing partner ecosystem

**Security First:**
- Auditable open source code
- On-premises deployment option
- Data sovereignty by default
- EN ISO 13849 safety compliance

---

## Next Steps

1. **Review EsoCore Documentation:** [www.esocore.com/docs](/docs)
2. **Calculate Your TCO:** [Cost Calculator](/docs/roi/cost-savings-calculator)
3. **Request Pilot Program:** [Contact for pilot details](#)
4. **Join Community:** [GitHub Discussions](https://github.com/newmatik/EsoCore)

---

## Related Resources

- [Open Source vs Proprietary Comparison](/docs/comparisons/open-source-vs-proprietary)
- [Edge AI vs Cloud Monitoring](/docs/comparisons/edge-ai-vs-cloud-monitoring)
- [Total Cost of Ownership Analysis](/docs/roi/total-cost-ownership)
- [Industrial IoT Best Practices](/docs/guides/industrial-iot-best-practices)
- [Getting Started Guide](/docs/getting-started/quick-start)

---

Make an informed decision based on data, not marketing. Choose a platform that provides freedom, flexibility, and long-term value.

[Explore EsoCore](/docs) | [Calculate TCO](/docs/roi/total-cost-ownership) | [Start pilot program](#)

