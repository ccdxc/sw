---
id: dscprofiles
---

# DSC Profiles
The PSM allows users to turn on features incrementally on one or more hosts at a time. Users can define feature profiles using an object called DSC Profiles, and associate them with DSCs.

Each DSC Profile has two settings: Deployment Target and Feature Set; together they determine the set of features supported by all DSCs assigned to that profile. A DSC can only be assigned to a single profile at any given time.

<load-table group:cluster obj:ApiObjectMeta
            include:name >
<load-table group:cluster obj:ClusterDSCProfileSpec
            include:deployment-target omitHeader:true>
<load-table group:cluster obj:ClusterDSCProfileSpec
            include:feature-set omitHeader:true>

If a profile’s settings are changed, all DSCs associated with that profile will automatically inherit support of the new set of features. Similarly, if a DSC is associated with a different profile, the DSC will inherit support for the new set of features.

## Deployment Targets
Two options are currently available as Deployment Targets: Host and Virtualized.

### Host
The services implemented by the DSC are applied to the traffic entering and exiting the host. When this Deployment Target is selected, two Feature Sets are available for the user to select between: SmartNIC and Flow Aware (see below for an explanation of the features available in each of these Feature Sets).

### Virtualized
When the Virtualized Deployment Target is selected, the services implemented by the DSC installed on an ESXi host are applied to the traffic generated and received by each single VM. When this Deployment Target is selected, only the Flow Aware with Firewall Feature Set is available (see below for an explanation of the features available in this Feature Set).

## Feature Sets
Three Feature Sets are currently supported by the Pensando platform:

### SmartNIC
The SmartNIC Feature Set, currently available with the Host Deployment Target, includes telemetry and visibility functions, such as a rich set of metrics and bi-directional traffic mirroring with ERSPAN. Although other Feature Sets include a wider range of services, the SmartNIC Feature Set does not have any limits in terms of connections per second that can be established through the card and it is therefore recommended when other services are not needed and the traffic is expected to have a large number of new flows per second is needed, such as in HPC (high performance computing) clusters or when conducting host performance testing.

### Flow Aware
The Flow Aware Feature Set, currently available with the Host Deployment Target, includes features that require a DSC to keep track of individual flows. Examples include flow-based bi-directional ERSPAN, flow statistics, NetFlow/IPFIX. These features are key in gaining visibility on the network and possibly learn communication patterns within the enterprise data center, which can be used as a basis to create appropriate firewall policies.

### Flow Aware with Firewall
The Flow Aware with Firewall Feature Set, currently available with the Virtualized Deployment Target, includes, in addition to all the features listed above for the Flow Aware Feature Set, also the capability of enforcing security policies, where a security policy specifies flows whose packets shall be forwarded or dropped by DSCs. This Feature Set selected with the Virtualized Deployment Target operates on traffic among single workloads (e.g, VMs) even if they execute on the same host. Hence, it can be used to provide visibility and enforcement with that level of granularity. A typical use case is an East-West stateful firewall, providing micro-segmentation within a data center.


Click on "Create DSC Profile" to complete the operation.

DSC Profiles Overview displays all the current DSC profiles configured. There are 9 available columns that can be shown and are selectable.

<load-table group:cluster obj:ApiObjectMeta
            include:name >
| Associated DSCs | One or more DSCs associated with the profile |
| Utilization | One or more DSCs associated with the profile |
<load-table group:cluster obj:ClusterDSCProfileSpec
            include:deployment-target omitHeader:true>
<load-table group:cluster obj:ClusterDSCProfileSpec
            include:feature-set omitHeader:true>
| Update DSC | "0" indicate the DSC profile is updated |
| Pending DSC | This field indicates the association of DSC profile to DSC is in progress |
<load-table group:cluster obj:ApiObjectMeta
            include:mod-time omitHeader:true>
<load-table group:cluster obj:ApiObjectMeta
            include:creation-time omitHeader:true>

Top right side is the gear icon which allows to export the table view of the DSC Profiles into a CSV file or a JSON file.