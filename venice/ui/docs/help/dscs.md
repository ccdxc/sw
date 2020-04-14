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

Each entry is selectable and on selection there are two supported operations.

	1. Add lablels to Selected DSC(s) - Labels are key-value pair that allows admins to add friendly names. 
	This is useful for tagging the DSC and aid in searching. Some of the labels are used internally for vCenter 
	Integration. For Ex. io.pensando.orch-name vCenter-10.10.10.200
	2. Assign DSC Profile to selected DSC(s) - Option to set profile to DSC. The DSC profile(s) are configured 
	in the DSC profile section.

A search criteria provides filtering based upone one or more of the above columns to narrow down the view of the DSCs.  Further details can be shown for items in the table view that are in blue color, which are "Name/Spec id", "Host" or "Workloads".  In the middle right side is the gear icon which allows to export the table view of the DSCs into a CSV file or a JSON file.
