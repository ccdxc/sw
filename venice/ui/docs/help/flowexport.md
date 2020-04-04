---
id: flowexport
---
## Flow Export

PSM allows to send specific flows to an external destination flow collector supporting IPFIX format.  Users can send the flows to multiple destination flow collectors.  To configure the flow export, the following are the configuration parameters.

<load-table group:monitoring obj:ApiObjectMeta
            include:name >
<load-table group:monitoring obj:MonitoringFlowExportPolicySpec
            include:interval,template-interval omitHeader:true >

###### Export Packet If
This is the filters for the flow export. Packets will only be exported if they match these rules.

<load-table group:monitoring obj:MonitoringMatchRule
            >


###### Syslog Export Configuration

<load-table group:monitoring obj:MonitoringFlowExportPolicySpec
            include:format>
<load-table group:monitoring obj:MonitoringSyslogExportConfig omitHeader:true
            >

###### Target
Servers to send the data to. Additional Targets are allowed by clicking on "+Add". 

<load-table group:monitoring obj:MonitoringExportConfig 
            omit:credentials>

## Example:

**Name:** Splunk

**Interval:** 10s

**Template Refresh:** 5m

**Export Packet If:**
	
	- Source
		- IP Address: 10.29.30.0/24
		- Mac Address:
	
	- Destination
		- IP Address: 10.29.31.0/24
		- Mac Address:
		
	- App/Ports
		- Apps:
		- Protocol/Ports: tcp/80
		
**Export Configuration**

	- Format: ipfix
	
	- Target
		- Destination: 10.29.30.22
		- Transport: udp/4739