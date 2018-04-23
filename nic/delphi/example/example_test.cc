// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "gtest/gtest.h"
#include "example.hpp"
#include "nic/delphi/example/example.delphi_utest.hpp"

namespace {

using namespace std;
using namespace example;


// --------------------- Reactor test example ------------------------

INTERFACESPEC_REACTOR_TEST(ExampleReactorTest, InterfaceMgr);

TEST_F(ExampleReactorTest, BasicTest) {
    // create an interface spec object
    delphi::objects::InterfaceSpecPtr intf = make_shared<delphi::objects::InterfaceSpec>();
    intf->mutable_key()->set_ifidx(1);
    intf->set_adminstate(example::IntfStateDown);
    sdk_->QueueUpdate(intf);

    // verify spec object is in db
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("InterfaceSpec").size(), 1) << "Interface spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("InterfaceStatus").size(), 1) << "Interface status object was not created";

    // change the admin status on spec
    intf->set_adminstate(example::IntfStateUp);
    sdk_->QueueUpdate(intf);

    // verify spec object in db has changed
    ASSERT_EQ_EVENTUALLY(delphi::objects::InterfaceSpec::FindObject(sdk_, intf)->adminstate(),
                        example::IntfStateUp) << "Interface spec object has wrong oper state";

    // verify status object has correct operstate too
    delphi::objects::InterfaceStatusPtr intfStatusKey = make_shared<delphi::objects::InterfaceStatus>();
    intfStatusKey->set_key(1);
    ASSERT_EQ_EVENTUALLY(delphi::objects::InterfaceStatus::FindObject(sdk_, intfStatusKey)->operstate(),
                        example::IntfStateUp) << "Interface status object has wrong oper state";
}


// --------------------- Service test example ------------------------
// init the delphi utest
DELPHI_SERVICE_TEST(ExampleTest, ExampleService);

TEST_F(ExampleTest, BasicTest) {
    usleep(1000);

    // create an interface spec object
    delphi::objects::InterfaceSpecPtr intf = make_shared<delphi::objects::InterfaceSpec>();
    intf->mutable_key()->set_ifidx(1);
    intf->set_adminstate(example::IntfStateDown);
    sdk_->QueueUpdate(intf);

    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("InterfaceSpec").size(), 1) << "Interface spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("InterfaceStatus").size(), 1) << "Interface status object was not created";

    // change the admin status on spec and verify status gets changed too
    auto intfSpecList = sdk_->ListKind("InterfaceSpec");
    for (vector<delphi::BaseObjectPtr>::iterator iter=intfSpecList.begin(); iter!=intfSpecList.end(); ++iter) {
        delphi::objects::InterfaceSpecPtr intfSpec = static_pointer_cast<delphi::objects::InterfaceSpec>(*iter);
        intfSpec->set_adminstate(example::IntfStateUp);
        sdk_->QueueUpdate(intfSpec);
    }

    usleep(1000 * 100);

    auto intfStatusList = sdk_->ListKind("InterfaceStatus");
    for (vector<delphi::BaseObjectPtr>::iterator iter=intfStatusList.begin(); iter!=intfStatusList.end(); ++iter) {
        delphi::objects::InterfaceStatusPtr intfStatus = static_pointer_cast<delphi::objects::InterfaceStatus>(*iter);
        ASSERT_EQ(intfStatus->operstate(), example::IntfStateUp) << "Interface status object has wrong oper state";
    }
}
} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
