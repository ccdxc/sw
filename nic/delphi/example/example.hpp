// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_EXAMPLE_H_
#define _DELPHI_EXAMPLE_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/example.delphi.hpp"
#include "nic/delphi/example/example_stats.hpp"

namespace example {

using namespace std;

// ExampleReactor is the reactor for the ExampleSpec object
class ExampleReactor : public delphi::objects::ExampleSpecReactor {
    delphi::SdkPtr sdk_;
public:
    ExampleReactor(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnExampleSpecCreate gets called when ExampleSpec object is created
    virtual delphi::error OnExampleSpecCreate(delphi::objects::ExampleSpecPtr intf);

    // OnExampleSpecDelete gets called when ExampleSpec object is deleted
    virtual delphi::error OnExampleSpecDelete(delphi::objects::ExampleSpecPtr intf);

    // OnAdminState gets called when AdminState attribute changes
    virtual delphi::error OnAdminState(delphi::objects::ExampleSpecPtr intf);

    // createIntfStatus creates an interface status object
    delphi::error createIntfStatus(uint32_t ifidx, example::IntfState status);

    // findIntfStatus finds the interface status object
    delphi::objects::ExampleStatusPtr findIntfStatus(uint32_t ifidx);
};
typedef std::shared_ptr<ExampleReactor> ExampleReactorPtr;

// ExampleService is the service object for this example
class ExampleService : public delphi::Service, public enable_shared_from_this<ExampleService> {
private:
    ExampleReactorPtr        intf_mgr_;
    ExampleStatsMgrPtr   intf_stats_mgr_;
    delphi::SdkPtr         sdk_;
    string                 svcName_;
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
