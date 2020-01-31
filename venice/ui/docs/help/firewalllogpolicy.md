---
id: firewalllogpolicy
---

# Firewall Log Policy

The firewall logs provide information of the security policy and events of the security rules.  This section will define firewall events that will be sent to a logging destination.  To create this policy, click on Create Firewall Log Policy

## Create Firewall Log Policy
A firewall log policy can be created to send all of the logs or specific logs to an external server. To do so, provide the following information:

<load-table group:monitoring obj:ApiObjectMeta
            include:name >
<load-table group:monitoring obj:MonitoringFwlogPolicySpec
            include:filter rename:filter|export omitHeader:true >
			
##### SYSLOG EXPORT CONFIGURATION

<load-table group:monitoring obj:MonitoringSyslogExportConfig omit:prefix >

##### TARGET

<load-table group:monitoring obj:MonitoringExportConfig omit:credentials>
