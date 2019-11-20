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
