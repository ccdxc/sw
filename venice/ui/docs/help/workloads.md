---
id: workload
---

# Workload
The "Workload" refers to the virtual machine that will be managed by Venice.  The workload is defined by the following attributes.

<load-table group:workload obj:ApiObjectMeta
            include:name >
<load-table group:workload obj:WorkloadWorkloadSpec
            include:host-name omitHeader: true >
				
### Interfaces 
List of the interfaces for the workload. Click on "+Interface" to create a new interface for the workload
<load-table group:workload obj:WorkloadWorkloadIntfSpec
            >

Click on "Create Workload" to complete the task.

## Examples:

**Name:**  Prod-Web

**Host Name:** Prod-Server1

**Interfaces:**

	Interface 1:
	Mac-Address: 0000.2a2d.9af1
	Micro-seg vlan: 10
	External vlan: 100
	IP Addresses: 10.29.50.21

**Name:**  Prod-DB

**Host Name:** Prod-Server1

**Interfaces:**

	Interface 1:
	Mac-Address: 0000.2a2d.9af1
	Micro-seg vlan: 11
	External vlan: 100
	IP Addresses: 10.29.50.22