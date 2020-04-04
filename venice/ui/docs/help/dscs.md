---
id: dscs
---

## Distributed Services Card (DSC)

The DSC section provides details and health of the DSCs within the PSM cluster.  The overview shows the total number of DSCs along with which DSC has the highest CPU utilization, highest memory utilization and highest storage utilization.  There are 12 available columns that can be shown and are selectable

	- Name/Spec id: Name of the DSC
	- MAC Address: MAC address of the DSC
	- Version: What software version is running
	- Management IP Address: The management IP Address
	- Admit: True or false if the DSC is admitted or not
	- Phase: What phase is the DSC in
	- Condition: If the DSC is healthy or not
	- Host:  What host the DSC is associated with
	- Labels:  Any labels associated with the DSC
	- Workloads: All associated workloads that resides behind the DSC
	- Modification Time: Last timestamp when the DSC has been modified
	- Creation Time: When the DSC was created to the PSM cluster

A search criteria provides filtering based upon one or more of the above columns to narrow down the view of the DSCs.  Further details can be shown for items in the table view that are in blue color, which are "Name/Spec id", "Host" or "Workloads".  In the middle right side is the gear icon which allows to export the table view of the DSCs into a CSV file.

<load-table group:cluster obj:ApiObjectMeta
            include:name >
<load-table group:cluster obj:ClusterDistributedServiceCardSpec
            include:filter omitHeader:true >
