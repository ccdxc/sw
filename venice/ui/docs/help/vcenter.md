---
id: vcenter
---

# vCenter
The vCenter sub-section under the controller section refers to the VMware vCenter that we want to manage. Assumption is that the virtual infrastructure admin would have created a cluster of DSC hosts under a DataCenter, and PSM admin will register the vCenter as part of the VMware vCenter integration. 

## Add vCenter
vCenter is created with the following attributes:

<load-table group:orchestration obj:ApiObjectMeta
            include:name >
<load-table group:orchestration obj:OrchestrationOrchestratorSpec
            include:uri omitHeader:true>
| DataCenter(s) to Manage | DataCenter(s) selection by either</br></br>"Manage All Datacenters" - To manage all DataCenters within the vCenter</br></br>"Manage Individual Datacenters" - To manage one or more DataCenter (space separation) |


- Credentials: Credential can be

| Option | Description |
| ------ | ------ |
| None | No Credentials |
| Username/Password | Username and Password of the vCenter |
| Token | Token for authentication and authorization |
| Certs | Certs comprise of Private key, Cert, and CA Bundle |

vCenter Integration Overview displays the following five columns for all the registered vCenters.

| Option | Description |
| ------ | ------ |
| Name | vCenter name |
<load-table group:orchestration obj:ApiObjectMeta
            include:creation-time omitHeader:true>
<load-table group:orchestration obj:OrchestrationOrchestratorSpec
            include:uri omitHeader:true>
<load-table group:orchestration obj:OrchestrationOrchestratorStatus
            include:connection-status omitHeader:true>
<load-table group:orchestration obj:OrchestrationOrchestratorStatus
            include:last-connected omitHeader:true>

### Example Using Username and Password

*Name: SanJose-vCenter.acme.com*

*URI: 10.10.10.200*  

*"Managed Individual Datacenters" Radio button Selected*  

*Datacenter names: SJ-BizCriticalApp-DC SJ-Empl-VDI-DC*  

*Credentials : "Username-Password" Radio button Selected*  

*User Name: administrator@acme.com*  

*Password:*  \*\*\*\*\*\*\*\*\*\*\*\*\*  
*Confirm Password:*  \*\*\*\*\*\*\*\*\*\*\*\*\* 