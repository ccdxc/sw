---
id: auditevents
---
## Audit Events

The Audit Events section tracks all events for the Venice cluster that provides the number of events seen.  It provides a selectable list to be shown of the following information about the events, which are:

	- Who:  Which user did the action
	- Time:  Timestamp of the event
	- Action: What type of action was executed.  Examples are read, login, create, etc.
	- Act on (kind): What kind of object the action was executed on. Examples are workload, host, network security policy, etc. 
	- Act on (name): What is the name of the object that the action was executed on
	- Outcome: Result of the event
	- Client: Source IP where the action was executed from
	- Service Node: Which cluster node the event processed
	- Service Name: Which Venice service was used 

The audited events can be searched by any of the criteria above to narrow the list of events.  The columns shown can be selectable to show all 9 columns or a sub section of them.  The events can be exported into a CSV or JSON format by clicking on the gear icon.

### Export Audit Events
Audit Events section allow users to export Audit Events log to a named file for archival. This file is downloadable from the Audit Events section itself after "Create archive" is successful. The archived Audit Event log file is also available for download in the "Monitoring" section under "Archive log".

Steps for creating a archive request -

1. Click on "Export Audit Events" on the right hand top corner of the Audit Events panel
2. Specify a Archive request name - Unique name 
1. Specify timerange - "Start time" and "End time" or select one of the avaiable time range from the following options:
	Past hour/ Past day/ Previous day/ Past 6 hours/ Past week/ Previous week/ Past 12 hours/ Past month/ Previous month.  
	The range is automatically set if one of the preset range is choosen.
1. Select "Create Archive" to complete the archive request.

If the input file name already exist, an error message - "Create failed" / "Already exist in Cache" is displayed in a pop up.

If successful,  "Create Successful - Created archive "*request archive-request-file*" pop up is displayed along with - Archive Request "*archive-request-file*" is completed. Click [_Download_]() to download the file! 

The [Download]() link can be used to download the archive log. The archive log is gzip'ed for saving space.

The archive request file is available for download in the "Monitoring" section under "Archive log".
Note: Advanced search query in the Audit Event records (the "V" arrow next to the search icon) can be used to select only those records that meets your criteria to be archived. The "View Search Query" button will show the Archive Query during the "Create Archive" operation.

For ex. 
Using the advanced search, filter the Audit records based on "Action" column with "Action" Equal to "Delete", now click "Export Audit Events" and follow the steps for creating an archive request.
"View Search Query" will display the "Archive Query" -

	{
	 "fields": {
	  "_ui": {},
	  "requirements": [
	   {
	    "_ui": {},
	    "key": "action",
	    "operator": "in",
	    "values": [
	     "Delete"
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


