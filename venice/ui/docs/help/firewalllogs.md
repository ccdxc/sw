---
id: fwlogs
---

# Firewall Logs

The firewall logs provide information of the security policy and events of the security rules.  It shows policies that are allowed, denied, rejected and other pertiennt information for the various flows within the environment.	 

## Create Firewall Log Policy
A firewall log policy can be created to send all of the logs or specific logs to an external server. To do so, provide the following information:

<load-table group:monitoring obj:ApiObjectMeta
            include:name >
<load-table group:monitoring obj:MonitoringFwlogPolicySpec
            include:filter rename:filter|export omitHeader:true >

###### Syslog Export Configuration

<load-table group:monitoring obj:MonitoringSyslogExport
            include:format>
<load-table group:monitoring obj:MonitoringSyslogExportConfig omitHeader:true
            >

###### Target
Servers to send the data to. Additional Targets are allowed by clicking on "+Add". 

<load-table group:monitoring obj:MonitoringExportConfig 
            omit:credentials>

