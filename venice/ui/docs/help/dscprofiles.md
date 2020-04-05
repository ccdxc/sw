---
id: dscprofiles
---

# DSC Profiles
The "DSC Profile" refers to various modes and policies that can be applied to the DSC.

XXX Pls note this will change shortly as we are still in discussion on what to call these modes.

<load-table group:cluster obj:ApiObjectMeta
            include:name >
<load-table group:cluster obj:ClusterDSCProfileSpec
            include:fwd-mode omitHeader:true>


Click on "Create DSC Profile" to complete the operation.

DSC Profiles Overview displays all the current DSC profiles configured. There are 9 available columns that can be shown and are selectable.

<load-table group:cluster obj:ApiObjectMeta
            include:name >
| Associated DSCs | One or more DSCs associated with the profile |
| Utilization | One or more DSCs associated with the profile |
<load-table group:cluster obj:ClusterDSCProfileSpec
            include:fwd-mode omitHeader:true>
<load-table group:cluster obj:ClusterDSCProfileSpec
            include:policy-mode omitHeader:true>
| Update DSC | "0" indicate the DSC profile is updated |
| Pending DSC | This field indicates the association of DSC profile to DSC is in progress |
<load-table group:cluster obj:ApiObjectMeta
            include:mod-time omitHeader:true>
<load-table group:cluster obj:ApiObjectMeta
            include:creation-time omitHeader:true>

Top right side is the gear icon which allows to export the table view of the DSC Profiles into a CSV file or a JSON file.
