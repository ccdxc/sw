---
id: alertsevents
---

# Alerts and Events

The alert and event logs discovers events of interest, that provide general information or information that are triggered by some critical event.  Venice allows the creation of customized alert and event policies to facilitate these specific events in the environment, that can be sent to an external syslog server.

## Alert Policies

Within the alert section, a customizable alert policy can be created to meet one or more specific criteria.   This will allow monitoring of your environment within this view.  To build this alert policy, you will need to provide the following from the "Add Alert Policy".  It is required to have at least one destination for the "Alert Policy".  

### Add Destination

The "Add Destination" is referring to the external syslog server that the alerts will be sent to.  One destination is required before any alert policy can be created.  The destination needs the following information:

<load-table group:monitoring obj:ApiObjectMeta
            include:name >

###### Syslog Export Configuration

<load-table group:monitoring obj:MonitoringSyslogExport
            include:format>
<load-table group:monitoring obj:MonitoringSyslogExportConfig omitHeader:true
            >

###### Target
Servers to send the data to. Additional Targets are allowed by clicking on "+Add". 

<load-table group:monitoring obj:MonitoringExportConfig 
            omit:credentials>

### Alert Policy

The alert policy allows for specific customized alerts to be sent to the syslog server.  

#### Add Alert Policy
To build this alert policy, the following information is needed:

<load-table group:monitoring obj:ApiObjectMeta
            include:name >

###### Criteria selector
If the event matches the given criteria, then an alert will be generated. 
All requirements are ANDed together.

| Option | Description |
| ------ | ----------- |
| key   | The event field that the condition applies to.  |
| operator   | Condition checked for the key. |
| values   | Values contains one or more comma separated values corresponding to the key. |


###### Alert level
Then Alert defines the severity of the generated alert
<load-table group:monitoring obj:MonitoringAlertPolicySpec
            include:severity>
				
###### Destination

<load-table group:monitoring obj:MonitoringAlertPolicySpec
            include:destinations >

## Events Policy

Within the events section, an event policy can be created to send events to an external syslog server. To build this event policy, you will need to provide the following from the "Add Event Policy". 

<load-table group:monitoring obj:ApiObjectMeta
            include:name >

###### Syslog Export Configuration

<load-table group:monitoring obj:MonitoringEventPolicySpec
            include:format>
<load-table group:monitoring obj:MonitoringSyslogExportConfig omitHeader:true 
            >

###### Target
Servers to send the data to. Additional Targets are allowed by clicking on "+Add". 

<load-table group:monitoring obj:MonitoringExportConfig 
            omit:credentials>


	
