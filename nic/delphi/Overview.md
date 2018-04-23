# Delphi Framework Overview

Delphi is a reactive framework for writing Naples services(like Agents, HAL, NicMgr, LinkMgr etc). Delphi supports writing code in multiple languages like C++, Go and Python.

![Delphi System Overview](https://drive.google.com/uc?id=1Uq25PJqaxOoKLS9hxpXvo-C4vyoT_kfm)

Delphi framework follows the 'inversion of control' approach similar to many popular frameworks. Unlike in traditional libraries, where user code invokes library code, in this approach framework code invokes user code as needed. Main control flow is determined by the framework rather than the user code. Since delphi is a reactive framework, callbacks to the user code is done based on events in the system. Delphi stores all state in the system in a central database. It manages state as tree of objects. Each delphi service(i.e a process) should manage a subtree of system state it specializes in. For example, LinkMgr might manage physical port related objects, NicMgr might manage PCIe device related objects, HAL would manage forwarding objects etc.

Delphi is made up of three components:

- **Delphi Hub**: is the central database that stores all state in the system. Delphi hub stores the entire object tree. Delphi services mount the part of the tree they are interested in.
- **Delphi Compiler**: compiles protobuf object model into delphi objects. Delphi compiler generates all the language bindings needed by the delphi framework.
- **Delphi Framework**: runs the main event loop of each process and makes event callbacks to event reactors.




## What is reactive programming?

According to [wikipedia](https://en.wikipedia.org/wiki/Reactive_programming):

> Reactive programming is a [declarative programming paradigm](https://en.wikipedia.org/wiki/Declarative_programming) oriented around [data flows](https://en.wikipedia.org/wiki/Dataflow_programming) and the propagation of change. This means that it should be possible to express static or dynamic data flows with ease in the programming languages used, and that the underlying execution model will automatically propagate changes through the data flow.

In reactive programming, programs are written as event reactors on a data flow. Reactive framework will propagate data flows emitted by one component to other components that have registered to receive those changes.

Lets compare this to typical RPC style programs. Picture below shows an example of RPC style program.

<img src="https://drive.google.com/uc?id=14sSvZlgV3UDK8Ssh0Ti4FZv_ECEjkyfZ" width="500">

Typical RPC style program is written as number of message handlers which process incoming message, create some internal state and respond back with another message.

In comparison, a reactive program is written as an event handler on an event. Event Reactor is similar to an AWS Lambda function that knows how to react to a single event. It contains just enough logic to know how to change the derived objects based on the event and nothing more. This lets you write highly modular code.

<img src="https://drive.google.com/uc?id=1_9hn6bsBIkEozBcqwb2wt9PtKHCel85A" width="500">

In this example, the event reactor just needs to know that it is reacting to an event on `Spec` object and in response, it needs to create the `State` object and `Status` object. it needs to have no further logic. This is really the power of reactive programming, it lets developer focus just on the part of the data flow graph he's interested in without having to worry about the rest of the picture.

Lets take a more concrete example of hypothetical HAL process that programs the HW in a reactive model.

<img src="https://drive.google.com/uc?id=1x26bGoO9N3nE7NZF4LF2DqJmsJlOOwW8" width="500">

In this example, the agent process publishes an `InterfaceSpec` object, the HAL PI module might have a reactor registered for `InterfaceSpec` object and create a `LifStatus` object and a `PD-Lif` object in response. The HAL PD modules might register multiple event reactors on `PD-Lif` object, one to program the HW and FTE state and other to program the QOS parameters. Again, this improves the modularity of the code by allowing the QOS developer to focus on his module while interface developer to focus on his part.


## Developer Workflow

Writing a delphi service is three step process.

1. Define the object model using protobuf and compile it using delphi compiler.
2. Define the event reactors.
3. Define a service object that mounts all the objects from delphi hub and instantiates all the event reactors.

<img src="https://drive.google.com/uc?id=1RQo4pJSutsd3u86TbPzyUunWZKYg8Id0" width="700">

**Object Model:** Delphi object model is defined using protobuf syntax. Protobuf objects need to follow certain convention (see Developer guide for details). Once the protobuf object is defined, it is compiled using delphi compiler. Delphi compiler generates the protobuf objects using `protoc` compiler and wraps the protobuf object in a delphi object. Delphi object contains all the hooks required by delphi framework. Delphi compiler also generates a reactor base class for each delphi object. When developers want to write the event reactor for an object, they need to inherit from this base reactor class.

Delphi subscribes to attributed based interface design similar to REST. All APIs of a delphi service can be consumed only by setting attributes on objects. There are no method calls, no RPCs, no messages. Setting specific attributes on an object triggers specific events. Event reactors can register for event on specific attributes.

Every delphi object needs to have an object 'kind', this uniquely identifies the class of the object. Every object is also required to have a 'key' which uniquely identifies an object instance within a 'kind'. The key attribute can be a scalar field like <`vlan-id`> or it can be a multi-field nested structure like <`vlan-id`, `mac-address`>. A key can not be a repeated field(i.e it can not be a vector or map). Even though Delphi stores object in a tree, it contains only two levels today('kind' and 'key').

Every delphi object has a unique 64bit identifier called 'handle'. Any delphi object can be looked up by it's key or handle. The handle can also be used for referring to other objects. For example, an interface might want to list of vlans, a route might want to refer to a list of next-hops. In those cases, an object might store a vector of object handles. Delphi framework provides convenient utility functions to get or iterate over a list of object handles. There is no guarantee that an object referenced by a handle will always exists. Developers should be careful to handle the cases where finding the object by it's handle might fail.

Note that Delphi object model is independent of Venice object model. Delphi object model if used by Naples processes for inter process communication.

**Event Reactor:** Almost all code in delphi is written as event reactors on an object. Event reactor need to inherit from an object's base reactor and override the event handlers for the events they are interested in. A delphi object might have tens of attributes and trigger a unique event type when each of those attributes are changed. An event reactor class might choose to implement the event handlers for only handful of those events.

**Service:** Each process needs to define a `Service` class that manages the process's life cycle. Service object needs to inherit from the base delphi service object that implements some of the common functionalities all services need to implement. The service object is responsible for telling the delphi framework what part of the object tree it is interested in mounting, instantiating the event reactors and registering them for event notifications.

Delphi Framework runs the main event loop of all delphi services. To provide simple programming model, all delphi services run single threaded event loops. This eliminates the need for worrying about threading, locking etc. A delphi service can have multiple threads, but delphi framework's event loop runs only on one of those threads. For example, HAL can have multiple threads running FTE, but it's config thread can only be single thread. Similarly NicMgr can have multiple threads servicing PCIe admin queues, it can only have one config thread.

## Mounting and Watching Objects

Each delphi service needs to mount part of the object sub tree they are interested in. A service can mount a subtree as read-only or as read-write. When it is mounted as read-only, the object can only watch the subtree, it can not modify it. When it is mounted as read-write, it can modify the objects in that subtree. Only one service can mount a subtree as read-write, all others have to mount it read only.

<img src="https://drive.google.com/uc?id=19JmCdB1uFVfOMMcEBeLKai8WZvNKU-Bu" width="500">

Once a subtree is mounted into a service, all objects in that subtree are available in the local memory of the process as C++/Go/Python objects. They can be accessed any time at memory speeds. If the subtree is mounted as read-write, these objects can be modified by the service and they will be reflected back to delphi hub and all the other services mounting the subtree.

A watch can be established for any object in the mounted subtree. Whenever the object being watched changes, corresponding events will be triggered on the reactors. All objects in the mounted subtree are not required to have reactors for them. For example, Linkmgr might mount interface objects and vlan objects. It might establish event reactors for interface object but not vlan object. It might be interested in looking at specific vlan object attributes whenever an interface object's vlan membership changes, it may not be interested in reacting to changes in vlan objects.

## Object Sync and Events

If a delphi service modifies an object, changed state will be synced back to delphi hub and all the other services mounting the object. This sync operation is done asynchronously when the delphi framework finishes handling the events and goes back to the main event loop. Typically, these object syncs are done in large batches by accumulating the object changes over a small period of time.

<img src="https://drive.google.com/uc?id=1jQg4aafO3qBhykXBRrmE11NxndDWRm0_" width="500">

Delphi hub maintains a list of services mounting a subtree of objects. When delphi hub receives a change event on an object it re-distributes the change to all services mounting the subtree. This is also done in large batches of objects to achieve efficiency.

Delphi framework does not provide any guarantees on ordering events across objects. If `service1` changes `object1` and `object2`, `service2` which is watching both the objects might not see the events in exactly same order. Similarly, if a service changes `field1` and `field2` on `object1`, event handlers on `reactor1` which is watching the object might be triggered in any order. Developers need to be careful to make sure event handlers always converge to correct state no matter which order they see the events. This requires a mind set of thinking event reactors as always trying to reconcile the state. Though, no ordering guarantee is provided for events, delphi framework triggers events only when it finishes handling current event and goes back to the main loop. So, event handlers dont need to worry about seeing partially changed objects. Since object sync is done in the main event loop, multiple changes done in one event handler will be synced together. Any service watching these changes will have all the changes in it's memory, before it's event handlers are called. This makes it easy to reconcile the state by looking at all the objects in it's memory.

## Service Lifecycle

Delphi framework manages the internal lifecycle of services. This doesn't mean, delphi framework manages the process starting/stopping etc. systemd or launcher will manages starting of processes. Delphi framework manages the internal event loop lifecycle, connecting to delphi hub, mounting all the objects, providing callbacks etc. Picture below shows the typical service lifecycle events.

<img src="https://drive.google.com/uc?id=1VK16_GP7DWsCXdgA5WqSFK7_L4wzCqP8" width="800">

When the service starts, first it initializes the delphi SDK and instantiates it's service object(as described above, this class is derived from `delphi::Service` class). As part of service's constructor, it needs to request mounting the objects it is interested in and watch the objects it is interested in. After the service object is instantiated, process can enter the main event loop. Once the delphi main loop is started, it connects to the delphi hub and downloads all the mounted objects. Once all the objects are instantiated in process's local memory, it calls the `OnMountComplete()` callback. The service is required to reconcile it's state during this callback. After the reconciliation is complete, delphi enters the event callback loop. Any changes to the object after this point will trigger event callback on reactors.

## Service Heartbeat

<img src="https://drive.google.com/uc?id=1xbZYPIi5DJumtrwQvkXpwtqWzKwOcMqJ" width="500">

Delphi framework maintains a periodic heartbeat with delphi hub. This heartbeat is performed periodically when event callbacks are done and it goes back to the event loop. So, if any event callback is stuck, heartbeat will not be done. A health monitor process can watch the service heartbeat objects in dlephi hub and see if any process is not performing the heartbeat. It can restart the process thats stuck for too long.

Its also worth mentioning that dlephi Hub maintains a list of all services that are connected to it. Any interested service can simply mount the service object subtree and get a list of all other services in the system. This is useful for things like code upgrade module. it can determine all the services in the system and send them a message to save and restore their state during upgrade.

## Delphi Unit Test

Being able to unit test the code is important part of any framework. Since delphi is an event based framework, it provides a way to inject specific events to unit test the code. This allows developers to test their event reactors without requiring the delphi hub and rest of the system. Delphi unit test works by simulating a mounted object database and triggering events on it. Delphi unit test is integrated with google's gtest suite.

<img src="https://drive.google.com/uc?id=1JRiikfphFKb5-kGBNGZr_7GNqa3yRWuq" width="500">

Delphi unit test can be done at two levels:
1. **Reactor level tests:** Reactor level test can be done by instantiating a single reactor object and triggering events on it. These should be isolated tests for a single reactor class without requiring other reactor classes. This is true unit test.
2. **Service level tests:** These are done by instantiating the entire service object for a process and triggering events for the whole service. This tests multiple reactors of a process. Its process level integration test.


## Delphi Metrics

Delphi Metrics is a shared memory object database where all delphi services can publish statistics. Metrics published into this shared memory database are also in protobuf serialized objects. Multiple processes or threads can read and write into the shared memory database concurrently. Details of the shared memory implementation are still to be worked out.

<img src="https://drive.google.com/uc?id=14tiixm8zmkr9ePy49D2RdGWS_P_dQzCK" width="500">


## Delphi framework features

### Memory Management and Smart Pointers

Delphi Framework manages the lifecycle of all objects. This takes the burden of memory management away from the developers. Delphi framework will make use of smart pointers extensively. Smart pointers maintain a ref count and the object will be freed automatically when count goes to zero. This is similar to modern garbage collected languages. Developers are strongly encouraged to use smart pointers everywhere in their code. When referring to a Delphi object, storing a handle to the object is preferred over storing a smart pointer. Also prevents circular reference problem of garbage collection.

### Transactional write to HW

Often, programming HW tables, publishing data structures to other threads require a transactional all-or-nothing model. This especially simplifies the error handling in event handlers where it can return errors without having to worry about rolling back partial changes. In delphi, a service can implement `TxnBegin()`, `TxnCommit()` and `TxnAbort()` callbacks. Framework will call these functions before/after each event handlers. This is especially useful for publishing data structures from HAL to FTE threads.

### Error Handling

Robust error handling will make the code more reliable. Some frameworks use C++ exceptions and some use extensive error code mechanism. Delphi framework does not use C++ exceptions. It takes the approach of returning an error object and aborting current transaction. Delphi framework defines an abstract error class. All return values are a derived class of this base error class. This approach is similar to Golang error Interface. Derived error classes can be used to encode additional context or data required during logging error messages. Aborting the transaction should clear all partially updated data. This simplifies the error handling significantly.

### Attribute Validation

In delphi, all attribute validations are done by setters of the object. So, reactors of an object generally don’t need to do any attribute validation. Valid ranges for the attributes can be specified in the protobuf definition as annotations. Current plan is to use https://github.com/lyft/protoc-gen-validate for validating the attributes. See [here](https://github.com/lyft/protoc-gen-validate/blob/master/README.md) for validator syntax.

### Timers, Sockets and File Events

Delphi event loop is based on ‘libev’ (which is just a wrapper on select/epoll loop). Any socket or file descriptor can be added to ‘libev’ event loop and registered for event callbacks. Any kernel or driver event that can be modelled as a file descriptor event can be handled using this mechanism. Timers are also supported using ‘libev’. Timers would be common way of retrying failed events. E.g. If an interface failed to come up due to TCAM resource exhaustion, we might put it in “Error disabled” state and retry bringing it up every five minutes, using a timer. Signal handlers are also supported using ‘libev’

# Developer Guide

See [here](./Readme.md) for developer guide on how to write delphi services.
