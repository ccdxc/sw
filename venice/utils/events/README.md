# Events Management System
This document captures how different events management components are designed and how to use it to record events.

Table of Contents
-----------------
* [Events Management System](#events-management-system)
  * [Design](#design)
  * [How to add/update/delete event types?](#how-to-addupdatedelete-event-types)
  * [How to recored events from venice components?](#how-to-recored-events-from-venice-components)
  * [How to record events from HAL/FTE?](#how-to-record-events-from-halfte)
  * [How to record events from NAPLES agents?](#how-to-record-events-from-naples-agents)
  * [How to record events from NAPLES platform components?](#how-to-record-events-from-naples-platform-components)

## Design
![](https://drive.google.com/uc?id=13XX4KYNDeryxA6P6agUKz_YQkfF41Tso)

#### Event Source
Any component in the system that generates an event. For example:

* *Controller components* - CMD, NPM, etc.
* *Agent components* - NMD, NetAgent, Nicmgr, Linkmgmr, etc.

#### Event Recorder
Recorder creates the event with given information (severity, event type, message and reference object) and sends it to the events proxy server for further processing. Events that could not be sent to the proxy due to connection failure will be preserved in a file and replayed once the proxy is up.

#### Event Proxy
Proxy server receives events from all different sources and persists it. The call from the recorder terminates here. Further processing and delivery of the events are asynchronous. Events proxy applies deduplication and distributes the received events using the dispatcher library. Events proxy encapsulates the dispatcher and writers.

#### Event Dispatcher
Events dispatcher is a library used by the proxy for performing deduplication and distribution.

Events are deduped for a given dedup interval. Any event that reoccurs after the interval will be consisdered a new event.

Events are distributed to all registered writers in intervals (batch interval). Venice will be the default writer in MANAGED mode.

#### Writers
Writers (venice, syslog, etc.) will get the events from dispatcher. And it is responsible for writing it to their respective destination (venice, syslog).

Now, let us see how to use this service to record events. To record an event, the respective event type must exist in the system.

## How to add/update/delete event types?

Event types are defined and managed in the service. Service proto (api/protos) needs to be updated to add/update/delete any event type.

e.g. `EventTypesV1` enum needs to be updated to reflect the new list of events.

***svc_cluster.proto***

```go
// list of V1 event types supported by the cluster service
enum EventTypesV1 {
    ElectionStarted   = 0; // Leader election started in the cluster
    ElectionCancelled = 1; // Leader election cancelled
    ElectionStopped   = 2; // Leader election stopped
    LeaderElected     = 3; // Leader elected for the cluster
    LeaderLost        = 4; // Node lost leadership during the election
    LeaderChanged     = 5; // Leader changed in the election
}

```

Once the proto is compiled, these event types will be available in `api/generated/{service}/svc_{service}_events.go`

e.g: `svc_cluster.proto` events will be in `/api/generated/cluster/svc_cluster_events.go`

## How to recored events from venice components?
1. All the event sources needs to provide source information (node-name and component) and a list of supported events to create the recorder. This information will be used when the event is recorded.

	***venice/cmd/cmd.go***

	```go
	import "github.com/pensando/sw/api/generated/cluster"
	...
	if _, err := recorder.NewRecorder(
			&monitoring.EventSource{NodeName: utils.GetHostname(), Component: globals.Cmd},
			cluster.GetEventTypes(), "", ""); err!=nil {
			log.Fatalf("failed to create events recorder, err: %v", err)
	}
	```

2. Start recording events - Once the recorder is created/initialized (singleton), users can use the library function `recorder.Event(...)` anywhere in the process to record events.

	***venice/cmd/services/leader.go***

	```go
	import "github.com/pensando/sw/api/generated/cluster"
	import "github.com/pensando/sw/venice/utils/events/recorder"
	...
	recorder.Event(cluster.ElectionStarted,
		monitoring.SeverityLevel_INFO,
		"Leader election started",
		nil)
	```

	- Event type will be validated against the list provided while creating the recorder.
	- Event source will be inherited from the underlying recorder.

## How to record events from HAL/FTE?
The agents managing HAL/FTE will record events on their behalf. Upon recieving the status update from HAL/FTE, the agents are responsible for recording the appropriate events. HAL/FTE components are expected to only update the status with the agents and the agents will handle the events for them.

## How to record events from NAPLES agents?

Same as the venice component.

1. Create events recorder.

	***nic/agent/cmd/nmd/main.go***

	```go
	import "github.com/pensando/sw/api/generated/cluster"
	...
	if _, err := recorder.NewRecorder(
		&monitoring.EventSource{NodeName: utils.GetHostname(), Component: globals.Nmd},
		cluster.GetEventTypes(), "", ""); err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}

	```

2. Start recording events.

	***nic/agent/nmd/state/smartnic.go***

	```go
	import "github.com/pensando/sw/api/generated/cluster"
	import "github.com/pensando/sw/venice/utils/events/recorder"
	...
	recorder.Event(cluster.NICAdmitted,
		monitoring.SeverityLevel_INFO,
		fmt.Sprintf("Smart NIC %s addmitted to the cluster",
		nic.GetName()),
		nic)
	```


## How to record events from NAPLES platform components?
// TODO