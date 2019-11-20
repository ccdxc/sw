---
id: hosts
---

# Hosts
The "Host" refers to the physical VMware ESXi server that is hosting the DSC.  The host is created with the following attributes:

- Name: Unique name for the host
- Add DSC by: 

	Association of the DSC to the host can be defined by two (2) methods by choosing the radio button:
	- id: This will be the name (ID) of the DSC
	- mac-address: This will be the MAC Address of the DSC

Click on "Create Hosts" to complete the creation.


## Examples:

**Name:** Prod-Server1

**Add DSC by:**

	id: ---> DSC Name: prod-server1-dsc

or

**Name: Prod-Server1**

**Add DSC by:**

	mac-address:  ---> MAC-ADDRESS: 00ae.cd00.4848
