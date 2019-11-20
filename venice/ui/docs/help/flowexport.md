---
id: flowexport
---
## Flow Export

Venice allows to send specific flows to an external destination flow collector supporting IPFIX format. Users can send the flows to multiple destination flow collectors.  To configure the flow export, the following are the configuration parameters.

<load-table group:monitoring obj:ApiObjectMeta
            include:name >
<load-table group:monitoring obj:MonitoringFlowExportPolicySpec
            include:interval,template-interval omitHeader:true >

###### Match Rules
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

- Name: Unique name for Flow Export policy
- Interval: This is the value in seconds that the flows will be sent
- Template Refresh Rate: <Need clarification>
- Export Packet If:  This is the filters for the flow export
	- Source:  Can be based upon
		- IP Address
		- Mac Address
	- Destination:  Can be based upon
		- IP Address
		- Mac Address
	- App/Ports:  Can be based upon App or Protocol/Ports
		- App
		- Protocol/Port
	- Format:  IPFIX only
	- Target:  This will be the destination flow export collector
		- Destination: IP Address
		- Transport: UDP/port

		Additional Targets is allowed by clicking on "+Add"   