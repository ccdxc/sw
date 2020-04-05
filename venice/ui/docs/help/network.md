---
id: network
---
## Network

The Network section provides two functions -

	1. Add a network. 
	2. Network overview - Provides a detailed table view of various networks created along with 
	the VLAN Id and the Orchestrator information.

### Add Network
The Network refers to the DSC network that PSM creates for network traffic. The network is created with the following attributes: 

<load-table group:network obj:ApiObjectMeta
            include:name >
<load-table group:network obj:NetworkNetworkSpec
            include:vlan-id omitHeader:true>
<load-table group:network obj:NetworkNetworkSpec
            include:orchestrators omitHeader:true>

### Network Overview
The network overview comprise of four columns including

<load-table group:network obj:ApiObjectMeta
            include:name >
<load-table group:network obj:ApiObjectMeta
            include:creation-time omitHeader:true>
<load-table group:network obj:NetworkNetworkSpec
            include:vlan-id omitHeader:true>
<load-table group:network obj:NetworkNetworkSpec
            include:orchestrators omitHeader:true>

Note: Name of the object must be unique within a Namespace for scoped objects. Must start and end with alpha numeric and can have alphanumeric, -, _, . Length of string should be between 2 and 64.
