---
id: alertsevents
---

# Alerts and Events

The alert and event logs discovers events of interest, that provide general information or information that are triggered by some critical event.  Venice allows the creation of customized alert and event policies to facilitate these specific events in the environment, that can be sent to an external syslog server.

## Alert Policies

Within the alert section, a customizable alert policy can be created to meet one or more specific criteria.   This will allow monitoring of your environment within this view.  To build this alert policy, you will need to provide the following from the "Add Alert Policy".  It is required to have at least one destination for the "Alert Policy".  

### Add Destination

The "Add Destination" is referring to the external syslog server that the alerts will be sent to.  One destination is required before any alert policy can be created.  The destination needs the following information:

- Unique Destination Name: Provide a unique destination name
- Syslog Export Configuration:
	- Facility Override: A drop-down box that provides the list of overrides
	- Prefix: <?>
	- Format: Two supported format (BSD or RFC5424)
- Target:
	- Destination: IP Address and path (URL) of the syslog server
	- Transport: TCP or UDP and port number

### Alert Policy

The alert policy allows for specific customized alerts to be sent to the syslog server.  

#### Add Alert Policy
To build this alert policy, the following information is needed:

- Policy Name: Provide an alert policy name
- If Event: A drop down list of possible event criteria is provided. 
- Operand: Equals or Not Equals
- Value: Enter in the value for the criteria
- Add:  Additional criteria can be added (optional)
- Then Alert:  This will define what type of alert it is categorized as (info, warn, critical)
- Destination: This will be the external syslog server that this alert will be sent to.  A drop down list provided the configured external syslog server(s).

#### Add Destination
A definition for the syslog destination can be configured in this section.  Here are the following attributes.

- Name:  Unique name for syslog destination
- Syslog Export Configuration:
	- Facility Override: Options are
		- Auth
		- Private Auth
		- CRON
		- Daemon
		- FTP
		- Kernel
		- local 0 - 7
		- LPR
		- Mail
		- News
		- Syslog
		- User
		- UUCP
	- Prefix: gathers the specific prefix 
	- Format: Options are BSD or RFC5424
- Target:  This is the information for the syslog server
	- Destination: IP Address of the server
	- Transport:  Protocol/Port 

## Events Policy

Within the events section, an event policy can be created to send specific categorized events to an external syslog server. To build this event policy, you will need to provide the following from the "Add Event Policy".  It is required to have at least one destination for the "Alert Policy", please create the "Destination" prior to creating the Event Policy. 

- Unique Policy Name: Provide a unique event policy name
	- If Event:  The events are based upon the following options
		- name
		- tenant
		- namespace
		- generation-id
		- resource-version
		- uuid
		- creation-time
		- mod-time
	- Operand:  Options are equals or not equals 
	- Value: Enter in the value for the event filter
	- Then Alert:  Once the event occurs, provide the following information to the syslog server
		- info
		- warn
		- critical
	- Destinations:  Choose from the drop-down box the list of destinations that has been configured 
	
### Export Events
Events section allow users to export Events log to a named file for archival. This file is downloadable from the Events section itself after "Create archive" is successful. The archived Events log file is also available for download in the "Monitoring" section under "Archive log".

Steps for creating a archive request -

1. Click on "Export Events" found next to "Event Policies"
2. Specify a Archive request name - Unique name 
1. Specify timerange - "Start time" and "End time" or select one of the avaiable time range from the following options:  
Past hour/ Past day/ Previous day/ Past 6 hours/ Past week/ Previous week/ Past 12 hours/ Past month/ Previous month.  
The range is automatically set if one of the preset range is choosen.

1. Select "Create Archive" to complete the archive request.  
If the input file name already exist, an error message - "Create failed" / Already exist in Cache is displayed in a pop up.

	If successful, a "Create Successful - Created archive "*request archive-request-file*" pop up is displayed along with - Archive Request "*archive-request-file*" is completed. Click [_Download_]() to download the file!
	
	The [Download]() link can be used to download the archive log. The archive log file is gzip'ed for saving space. 

The Event logs archive file is available for download in the "Monitoring" section under "Archive log".

Note: Advanced search query in the Events records (the "V" arrow next to the search icon) can be used to select only those records that meets your criteria to be archived. The "View Search Query" button will show the Archive Query during the "Create Archive" operation.

For ex. 
Using Advanced search, filter the Events records based on "Action" column with "Severity" Equal to "critical", now click "Export Events" and follow the steps for creating an archive request.
"View Search Query" will display the "Archive Query"-

	{
	 "fields": {
	  "_ui": {},
	  "requirements": [
	   {
	    "_ui": {},
	    "key": "severity",
	    "operator": "in",
	    "values": [
	     "critical"
	    ]
	   }
	  ]
	 },
	 "labels": {
	  "_ui": {},
	  "requirements": []
	 },
	 "texts": [
	  {
	   "_ui": {},
	   "text": []
	  }
	 ]
	}

