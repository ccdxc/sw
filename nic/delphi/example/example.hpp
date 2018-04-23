// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_EXAMPLE_H_
#define _DELPHI_EXAMPLE_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/example/example.delphi.hpp"

namespace example {

using namespace std;

// InterfaceMgr is the reactor for the InterfaceSpec object
class InterfaceMgr : public delphi::objects::InterfaceSpecReactor {
    delphi::SdkPtr sdk_;
public:
    InterfaceMgr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnInterfaceSpecCreate gets called when InterfaceSpec object is created
    virtual delphi::error OnInterfaceSpecCreate(delphi::objects::InterfaceSpecPtr intf);

    // OnInterfaceSpecDelete gets called when InterfaceSpec object is deleted
    virtual delphi::error OnInterfaceSpecDelete(delphi::objects::InterfaceSpecPtr intf);

    // OnAdminState gets called when AdminState attribute changes
    virtual delphi::error OnAdminState(delphi::objects::InterfaceSpecPtr intf);

    // createIntfStatus creates an interface status object
    delphi::error createIntfStatus(uint32_t ifidx, example::IntfState status);

    // findIntfStatus finds the interface status object
    delphi::objects::InterfaceStatusPtr findIntfStatus(uint32_t ifidx);
};
typedef std::shared_ptr<InterfaceMgr> InterfaceMgrPtr;

// ExampleService is the service object for this example
class ExampleService : public delphi::Service, public enable_shared_from_this<ExampleService> {
private:
    InterfaceMgrPtr    intfMgr_;
    delphi::SdkPtr     sdk_;
    string             svcName_;
public:
    // ExampleService constructor
    ExampleService(delphi::SdkPtr sk);
    ExampleService(delphi::SdkPtr sk, string name);

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete();

    // createIntfSpec creates a dummy interface
    void createIntfSpec();

    // override service name method
    virtual string Name() { return svcName_; }

    // timer for creating a dummy object
    ev::timer          createTimer;
    void createTimerHandler(ev::timer &watcher, int revents);
};
typedef std::shared_ptr<ExampleService> ExampleServicePtr;

} // namespace example

#endif // _DELPHI_EXAMPLE_H_
