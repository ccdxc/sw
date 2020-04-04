---
id: auditevents
---
## Audit Events

The Audit Events section tracks all events for the PSM cluster that provides the number of events seen.  It provides a selectable list to be shown of the following information about the events, which are:

	- Who:  Which user did the action
	- Time:  Timestamp of the event
	- Action: What type of action was executed.  Examples are read, login, create, etc.
	- Act on (kind): What kind of object the action was executed on. Examples are workload, host, network security policy, etc. 
	- Act on (name): What is the name of the object that the action was executed on
	- Outcome: Result of the event
	- Client: Source IP where the action was executed from
	- Service Node: Which cluster node the event processed
	- Service Name: Which PSM service was used 

The audited events can be searched by any of the criteria above to narrow the list of events.  The columns shown can be selectable to show all 9 columns or a sub section of them.  The events can be exported into a CSV or JSON format by clicking on the gear icon.
