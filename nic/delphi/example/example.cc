// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "example.hpp"

namespace example {

using namespace std;

// OnExampleSpecCreate gets called when ExampleSpec object is created
delphi::error ExampleReactor::OnExampleSpecCreate(delphi::objects::ExampleSpecPtr intf) {
    LogInfo("ExampleSpec got created for {}/{}", intf, intf->meta().ShortDebugString());

    // find the status object
    auto intfStatus = this->findIntfStatus(intf->key().ifidx());
    if (intfStatus == NULL) {
        // create it since it doesnt exist
        RETURN_IF_FAILED(this->createIntfStatus(intf->key().ifidx(), intf->adminstate()));
    }

    return delphi::error::OK();
}

// OnExampleSpecDelete gets called when ExampleSpec object is deleted
delphi::error ExampleReactor::OnExampleSpecDelete(delphi::objects::ExampleSpecPtr intf) {
    LogInfo("ExampleSpec got deleted");
    return delphi::error::OK();
}

// OnAdminState gets called when AdminState attribute changes
delphi::error ExampleReactor::OnAdminState(delphi::objects::ExampleSpecPtr intf) {
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

// createIntfStatus creates a interface status object
delphi::error ExampleReactor::createIntfStatus(uint32_t ifidx, example::IntfState status) {
    // create an object
    delphi::objects::ExampleStatusPtr intf = make_shared<delphi::objects::ExampleStatus>();
    intf->set_key(ifidx);
    intf->set_operstate(status);

    // add it to database
    sdk_->SetObject(intf);

    LogInfo("Created interface status object for ifidx {} admin state {} intf: {}", ifidx, status, intf);

    return delphi::error::OK();
}

// findIntfStatus finds an interface status object
delphi::objects::ExampleStatusPtr ExampleReactor::findIntfStatus(uint32_t ifidx) {
    delphi::objects::ExampleStatusPtr intf = make_shared<delphi::objects::ExampleStatus>();
    intf->set_key(ifidx);

    // find the object
    delphi::BaseObjectPtr obj = sdk_->FindObject(intf);

    return static_pointer_cast<delphi::objects::ExampleStatus>(obj);
}

// ExampleService constructor
ExampleService::ExampleService(delphi::SdkPtr sk) : ExampleService(sk, "ExampleService") {
}

// ExampleService constructor
ExampleService::ExampleService(delphi::SdkPtr sk, string name) {
    // save a pointer to sdk
    this->sdk_ = sk;
    this->svcName_ = name;

    // mount objects
    delphi::objects::ExampleSpec::Mount(sdk_, delphi::ReadWriteMode);

    // create interface event handler
    intf_mgr_ = make_shared<ExampleReactor>(sdk_);

    // Register interface reactor
    delphi::objects::ExampleSpec::Watch(sdk_, intf_mgr_);

    // create interface stats mgr
    intf_stats_mgr_ = make_shared<ExampleStatsMgr>();

    LogInfo("Example service constructor got called");
}

// OnMountComplete gets called when all the objects are mounted
void ExampleService::OnMountComplete() {
    string out_str;

    LogInfo("ExampleService OnMountComplete got called\n");

    // walk all interface objects and reconcile them
    vector<delphi::objects::ExampleSpecPtr> iflist = delphi::objects::ExampleSpec::List(sdk_);
    for (vector<delphi::objects::ExampleSpecPtr>::iterator intf=iflist.begin(); intf!=iflist.end(); ++intf) {
        intf_mgr_->OnExampleSpecCreate(*intf);
    }

    LogInfo("============== ExampleService Finished Reconciliation ==================\n");
}

// createIntfSpec creates a dummy interface
void ExampleService::createIntfSpec() {
    // create an object
    delphi::objects::ExampleSpecPtr intf = make_shared<delphi::objects::ExampleSpec>();
    srand(time(NULL));
    intf->mutable_key()->set_ifidx(rand());
    intf->set_adminstate(example::IntfStateDown);

    // add it to database
    sdk_->SetObject(intf);
}

// createTimerHandler creates a dummy interface
void ExampleService::createTimerHandler(ev::timer &watcher, int revents) {
    LogInfo("Creating a dummy interface");

    // create a dummy interface
    this->createIntfSpec();
}
} // namespace example
