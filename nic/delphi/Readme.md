# Delphi Framework

Delphi is a reactive framework for writing Naples services(like Agents, HAL, NicMgr, LinkMgr etc). Delphi supports wiriting code in multiple languages like C++, Go and Python.

![Delphi System Overview](https://drive.google.com/uc?id=1Uq25PJqaxOoKLS9hxpXvo-C4vyoT_kfm)

Delphi is made up of three components:

- Delphi Hub: is the central database that stores all state in the system.
- Delphi Compiler: compiles protobuf object model into delphi objects.
- Delphi Framework: runs the main event loop of each process and makes event callbacks to event reactors.



See [Overview](./Overview.md) for more details on Delphi Framework.

See [here](./Build.md) for instructions on how to build delphi and the examples.

# Delphi Developer Guide

Delphi is a reactive framework. That means almost all code in delphi is written as event reactors on objects. Idea is to make the developers more productive by allowing them to focus on the core logic and eliminate the boilerplate as much as possible.

Writing a Delphi service is three step process:

1. Define the object model using protobuf
2. Create event reactors for the object
3. Create a service that mounts the objects and instantiates event reactors

<br>
Lets go over each step in detail.
<br> <br>

### 1. Define the object model

```
syntax = "proto3";
package example;

import "nic/delphi/proto/delphi/delphi.proto";

// Interface id
message IntfIndex {
    uint32    Ifidx = 1;
}

// Interface
enum IntfState {
    InvalidIntfState   = 0;
    IntfStateUp        = 1;
    IntfStateDown      = 2;
}

// spec part of the object
message InterfaceSpec {
    option (delphi.update_event) = true;
    delphi.ObjectMeta      Meta         = 1;
    IntfIndex              Key          = 2;
    IntfState              AdminState   = 3 [(delphi.event) = true];
    string                 MacAddress   = 4;
}

// status part of the object
message InterfaceStatus {
    option (delphi.update_event) = false;
    delphi.ObjectMeta      Meta              = 1;
    uint32                 Key               = 2;
    IntfState              OperState         = 3 [(delphi.event) = true];
    string                 DummyStatusField  = 4 [(delphi.event) = false];
}
```

The example above defines two objects `InterfaceSpec` and `InterfaceStatus`. Couple of things to note about the proto file.
1. Notice that all proto files need to import `nic/delphi/proto/delphi/delphi.proto`. This imports all delphi extensions to protobuf.
2. All Delphi objects are required to have an `delphi.ObjectMeta` field. This contains all the metadata about the object that framework needs. one important attribute of the metadata is object kind.  All delphi object classes needs to have a unique object kind.
3. All Delphi objects are required to have a field named `Key`. This field needs to contain the unique key for the object. This could be a scalar field like `uint32`, `uint64` or a `string`. Or this could be another structure, like in the example of `InterfaceSpec` which uses `IntfIndex` as the key.

In delphi, there can be event reactors for object create or delete event, or event reactor for individual field changes. For every object, delphi framework triggers `OnCreate()` and `OnDelete()` events. For each attribute that developer is interested in receiving update event, he should add an option `(delphi.event) = true` to the attribute. See `AdminState` attribute of `InterfaceSpec` object for an example.  Event reactors can be registered on a scalar value types or on nested structs. Notice that not all attributes of an object are required to generate events. Some attributes like 'description' is not required to generate any event.

In certain cases, its more beneficial to have an object level "update" event instead of an individual fields. In those cases `option (delphi.update_event) = true;` option can be specified on the object. In this case framework will trigger the update event on the object itself in addition to attribute level events.

Once the object model is defined, it can be compiled using delphi compiler `delphic` (pronounced 'delphi-c'). Delphi compiler generates the delphi object code in C++, Go and python. This generated code has all the hooks required by delphi framework for triggering events.

### 2. Define a reactor

```
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/example/example.delphi.hpp"

// InterfaceMgr is the reactor for the InterfaceSpec object
class InterfaceMgr : public delphi::objects::InterfaceSpecReactor {
    delphi::SdkPtr sdk_;
public:
    InterfaceMgr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnInterfaceSpecCreate gets called when InterfaceSpec object is created
    virtual delphi::error OnInterfaceSpecCreate(delphi::objects::InterfaceSpecPtr intf) {
        LogInfo("InterfaceSpec got created for {}/{}", intf, intf->meta().ShortDebugString());

        // find the corresponding status object
        auto intfStatus = this->findIntfStatus(intf->key().ifidx());
        if (intfStatus == NULL) {
            // create the status object since it doesnt exist
            RETURN_IF_FAILED(this->createIntfStatus(intf->key().ifidx(), intf->adminstate()));
        }

        return delphi::error::OK();
    }

    // OnInterfaceSpecDelete gets called when InterfaceSpec object is deleted
    virtual delphi::error OnInterfaceSpecDelete(delphi::objects::InterfaceSpecPtr intf) {
        LogInfo("InterfaceSpec got deleted");
        return delphi::error::OK();
    }

    // OnAdminState gets called when AdminState attribute changes
    virtual delphi::error OnAdminState(delphi::objects::InterfaceSpecPtr intf) {
        // up or down?
        if (intf->adminstate() == example::IntfStateUp) {
            LogInfo("Interface is up");
        } else {
            LogInfo("Interface is down");
        }

        // set the oper state on status object
        auto intfStatus = this->findIntfStatus(intf->key().ifidx());
        if (intfStatus != NULL) {
            intfStatus->set_operstate(intf->adminstate());
            sdk_->SetObject(intfStatus);
        }

        return delphi::error::OK();
    }
};
typedef std::shared_ptr<InterfaceMgr> InterfaceMgrPtr;
```

Example above defines a reactor for `InterfaceSpec` object. Couple of things to notice about the reactors.

1. All delphi source or header files should `#include` `nic/delphi/sdk/delphi_sdk.hpp` file. This contains all the objects and functions required by the delphi framework.
2. Note that the file includes `nic/delphi/example/example.delphi.hpp` This is a file generated by the delphi compiler. This contains the c++ protobuf object definition for the `InterfaceSpec` object and the delphi extensions to it.
3. The reactor object `InterfaceMgr` inherits from `delphi::objects::InterfaceSpecReactor`. This is an object thats generated by the delphi compiler. For each delphi object, compiler generates a `xxxReactor` object. This object contains all the event handlers for the object as virtual functions. The derived object written by the developer can selectively override the events he is interested in handling.
4. All event reactors get a pointer to the object that changed. For example `OnInterfaceSpecCreate` event handler gets a pointer to `delphi::objects::InterfaceSpecPtr`. Delphi object for `InterfaceSpec` gets defined in `delphi::objects` namespace. `InterfaceSpecPtr` is a smart pointer to the object. In delphi, all objects are managed using smart pointers. Memory for these objects get freed when there is no one refering to the object.

### 3. Define a Service

```
// ExampleService is the service object for this example
class ExampleService : public delphi::Service, public enable_shared_from_this<ExampleService> {
private:
    InterfaceMgrPtr    intfMgr_;
    delphi::SdkPtr     sdk_;
public:
    // ExampleService constructor
    ExampleService(delphi::SdkPtr sk) {
        // save a pointer to sdk
        this->sdk_ = sk;

        // mount objects
        delphi::objects::InterfaceSpec::Mount(sdk_, delphi::ReadWriteMode);

        // create interface event handler
        intfMgr_ = make_shared<InterfaceMgr>(sdk_);

        // Register interface reactor
        delphi::objects::InterfaceSpec::Watch(sdk_, intfMgr_);
    }

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete() {
        LogInfo("ExampleService OnMountComplete got called\n");

        // walk all interface objects and reconcile them
        vector<delphi::objects::InterfaceSpecPtr> iflist = delphi::objects::InterfaceSpec::List(sdk_);
        for (vector<delphi::objects::InterfaceSpecPtr>::iterator intf=iflist.begin(); intf!=iflist.end(); ++intf) {
            intfMgr_->OnInterfaceSpecCreate(*intf);
        }
    }
};
typedef std::shared_ptr<ExampleService> ExampleServicePtr;
```

This example shows a typical service.
1. All services should inherit from `delphi::Service` class.
2. Service constructor should take a pointer to `delphi::Sdk` object and store it for later use. All access to delphi objects are done thru the sdk object.
3. Service should mount the subtree of objects its interested in. In this example `delphi::objects::InterfaceSpec::Mount(sdk_, delphi::ReadWriteMode);` mounts the `InterfaceSpec` objects.
4. Service should instantiate the reactors for each object kind and establish watch for object kind.
5. Service needs to implement `OnMountComplete()` callback. This will get called after delphi framework has connected with delphi hub, mounted all the objects and they are available to the service in local memory. Service should reconcile all state during this callback. If service programs HW, it needs to make sure HW state is in sync with delphi object state. If service publishes any oper state, it needs to make sure its in sync with the config state in delphi DB.

### 4. Main loop

Finally, main loop of the service looks relatively simple:

```
int main(int argc, char **argv) {
    // Create delphi SDK instance
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());

    // Create a service instance
    shared_ptr<ExampleService> exsvc = make_shared<ExampleService>(sdk);
    assert(exsvc != NULL);
    sdk->RegisterService(exsvc);

    // run the main loop
    return sdk->MainLoop();
}
```

Main function does three things:
1. It instantiates the Delphi SDK. This in turn connects to the delphi hub.
2. It instantiates a service object and registers it with the delphi SDK.
3. It runs the main eventloop. Delphi event loop watches for changes from delphi hub and triggers event callbacks.

### 5. Create a bazel build file for the service

Delphi provides convenient bazel macros for compiling protobuf object model into source files. Below shows an example of bazel `BUILD` file.

```
load("//nic/delphi/compiler:delphi.bzl", "delphi_compile")

# compile the proto file
delphi_compile(
    name = "example_gen",
    srcs = [ 'example.proto' ],
)

# compile the reactors and service
cc_library(
    name = "example_lib",
    srcs = [
        "example.cc",
        "example.hpp",
    ],
    hdrs = [
        "example.hpp",
    ],
    deps = [
        "//nic/delphi/sdk",
        ":example_gen",
    ],
)
```

1. first thing to note here is the build file loads delphi compiler macros from `load("//nic/delphi/compiler:delphi.bzl", "delphi_compile")`. This file contains a macro called `delphi_compile()` that can be used for compiling one or more proto files.
2. call `delphi_compile()` macro with a rule name and list of `.proto` files. This macro will compile the proto files into `.pb` files and `.delphi` files and then compiles them into a library.
3. Rest of the source files can be compiled using standard bazel rules and link to the delphi compiled library by adding it as a dependency.

## Writing unit tests

Delphi framework provides convenient wrappers for writing unit tests. Idea is to make it easy to write test code by emulating delphi database and events and objects. Delphi provides two ways to test your code:

1. Test single event reactor
2. Test an entire service

Here is an example of a simple test code that tests the `InterfaceSpec` reactor we wrote in previous section.

```
#include "gtest/gtest.h"
#include "example.hpp"
#include "nic/delphi/example/example.delphi_utest.hpp"

INTERFACESPEC_REACTOR_TEST(ExampleReactorTest, InterfaceMgr);

TEST_F(ExampleReactorTest, BasicTest) {
    // create an interface spec object
    delphi::objects::InterfaceSpecPtr intf = make_shared<delphi::objects::InterfaceSpec>();
    intf->mutable_key()->set_ifidx(1);
    intf->set_adminstate(example::IntfStateDown);
    sdk_->QueueUpdate(intf);

    // verify spec object is in db
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("InterfaceSpec").size(), 1) << "Interface spec object was not created";

    // verify corresponding status object was created by the reactor
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("InterfaceStatus").size(), 1) << "Interface status object was not created";
}
```

1. First you need to include the `example.delphi_utest.hpp` that was generated by the delphi compiler. This contains the wrappers required by the test code.
2. Next, you need to instantiate a mock service and run a mock event loop. This is achieved by calling the macro `INTERFACESPEC_REACTOR_TEST(ExampleReactorTest, InterfaceMgr);` This creates a dummy service that instantiates a reactor object of type `InterfaceMgr`. It mounts and watches all `InterfaceSPec` objects. It creates a gtest fixture called `ExampleReactorTest` and runs a delphi event loop in a seperate pthread.
3. After this, you can just start writing test code. In the above example `BasicTest` test case triggers a create event on an interface spec object and verifies that reactor has created the interface status object.
4. delphi SDK provides a `ASSERT_EQ_EVENTUALLY()` macro which is an asynchronous wrapper to `ASSERT_EQ()` macro with the same syntax. It periodically checks for the condition till it becomes true.


## Coding Style Guide

Delphi follows google c++ style guide https://google.github.io/styleguide/cppguide.html as much as possible. Please follow the same convention.
