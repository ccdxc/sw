---
id: networkinterfaces
---
## Network Interfaces

Network Interface describes the logical interface that is exposed to the operating system. 

<load-table group:network obj:ApiObjectMeta
            include:name >
<load-table group:network obj:NetworkNetworkInterfaceSpec
            include:mac-address omitHeader:true>
<load-table group:network obj:ApiObjectMeta
            include:creation-time omitHeader:true>
<load-table group:network obj:NetworkNetworkInterfaceSpec
            include:admin-status omitHeader:true>
<load-table group:network obj:NetworkNetworkInterfaceStatus
            include:type omitHeader:true>
<load-table group:network obj:ApiObjectMeta
            include:labels omitHeader:true>
<load-table group:network obj:NetworkNetworkInterfaceStatus
            include:oper-status omitHeader:true>
	
	
A search criteria provides filtering based upone one or more of the above columns to narrow down the view of the Network Interfaces. In the right hand top corner is the gear icon which allows to export the table view of Network Interfaces into a CSV file or a JSON file.