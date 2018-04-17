## Event Management Components
This document captures how different events management components are designed and how to use it from event sources (producers).

### Design
![](https://drive.google.com/uc?id=13XX4KYNDeryxA6P6agUKz_YQkfF41Tso)

#### Event Source
Any component in the system that generates an event. For example:

* *Controller components* - CMD, NPM, etc.
* *Agent components* - NMD, NetAgent, Nicmgr, Linkmgmr, etc.

#### Event Recorder
Recorder creates the event with given information (severity, event type, message and object reference) and sends it to the events proxy server for further processing. Events that could not be sent to the proxy due to connection failure will be preserved in a file and replayed once the proxy is up.

#### Event Proxy
Proxy server receives the events from all different sources and forwards it to the dispatcher for further processing (dedup and dispatch to writers). The calls from the recorder terminates here. Further processing and delivery of the events are asynchronous.

Events proxy encapsulates the dispatcher and writers.

#### Event Dispatcher
Events dispatcher processes the incoming events for deudplication and distribute it to the writers in intervals.

Upon receiving the event from recorder, it checks for potential deduplication. And, it either sends the event to writers (if this is the first occurrence of an event in the interval) or to the cache.

#### Writers
Writers (venice, syslog, etc.) will get the events from dispatcher. And it is responsible for writing it to their respective destination (venice, syslog).

### How to use the recorder at Venice?
1. All the event sources needs to provide source information (node-name and component) and a list of supported events to create the recorder. This information will be used when each event is recorded using this recorder.

	```go
	recorder, err := NewRecorder(
		&evtsapi.EventSource{NodeName: "test-node", Component: "test-component"},
		[]string{"TestNICDisconnected", "TestNICConnected"},
		proxyServerURL)
	```

2. Start recording events using the recorder instance.

	```go
	err = recorder.Event(TestNICDisconnected, "INFO", "test message", nil)
	```
	* Given event type will be validated against the list provided while creating the recorder.
	* Event source will be inherited from the recoder.

### How dispatcher cache works internally?

![](https://drive.google.com/uc?id=1zEz-gUyGQ279XMA2hnqtlL21Qh9fZX9c)


### How to use the recorder at NAPLES?
// TODO
