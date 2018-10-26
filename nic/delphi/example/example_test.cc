// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "gtest/gtest.h"
#include "example.hpp"
#include "gen/proto/example.delphi_utest.hpp"

namespace {

using namespace std;
using namespace example;


// --------------------- Reactor test example ------------------------

EXAMPLESPEC_REACTOR_TEST(ExampleReactorTest, ExampleReactor);

TEST_F(ExampleReactorTest, BasicTest) {
    // create an interface spec object
    delphi::objects::ExampleSpecPtr intf = make_shared<delphi::objects::ExampleSpec>();
    intf->mutable_key()->set_ifidx(1);
    intf->set_adminstate(example::IntfStateDown);
    sdk_->QueueUpdate(intf);

    // verify spec object is in db
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("ExampleSpec").size(), (uint32_t)1) << "Interface spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("ExampleStatus").size(), (uint32_t)1) << "Interface status object was not created";

    // change the admin status on spec
    intf->set_adminstate(example::IntfStateUp);
    sdk_->QueueUpdate(intf);

    // verify spec object in db has changed
    ASSERT_EQ_EVENTUALLY(delphi::objects::ExampleSpec::FindObject(sdk_, intf)->adminstate(),
                        example::IntfStateUp) << "Interface spec object has wrong oper state";

    // verify status object has correct operstate too
    delphi::objects::ExampleStatusPtr intfStatusKey = make_shared<delphi::objects::ExampleStatus>();
    intfStatusKey->set_key(1);
    ASSERT_EQ_EVENTUALLY(delphi::objects::ExampleStatus::FindObject(sdk_, intfStatusKey)->operstate(),
                        example::IntfStateUp) << "Interface status object has wrong oper state";
}


// --------------------- Service test example ------------------------
// init the delphi utest
DELPHI_SERVICE_TEST(ExampleTest, ExampleService);

TEST_F(ExampleTest, BasicTest) {
    usleep(1000);

    // create an interface spec object
    delphi::objects::ExampleSpecPtr intf = make_shared<delphi::objects::ExampleSpec>();
    intf->mutable_key()->set_ifidx(1);
    intf->set_adminstate(example::IntfStateDown);
    sdk_->QueueUpdate(intf);

    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("ExampleSpec").size(), (uint32_t)1) << "Interface spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("ExampleStatus").size(), (uint32_t)1) << "Interface status object was not created";

    // change the admin status on spec and verify status gets changed too
    auto intfSpecList = sdk_->ListKind("ExampleSpec");
    for (vector<delphi::BaseObjectPtr>::iterator iter=intfSpecList.begin(); iter!=intfSpecList.end(); ++iter) {
        delphi::objects::ExampleSpecPtr intfSpec = static_pointer_cast<delphi::objects::ExampleSpec>(*iter);
        intfSpec->set_adminstate(example::IntfStateUp);
        sdk_->QueueUpdate(intfSpec);
    }

    usleep(1000 * 100);

    auto intfStatusList = sdk_->ListKind("ExampleStatus");
    for (vector<delphi::BaseObjectPtr>::iterator iter=intfStatusList.begin(); iter!=intfStatusList.end(); ++iter) {
        delphi::objects::ExampleStatusPtr intfStatus = static_pointer_cast<delphi::objects::ExampleStatus>(*iter);
        ASSERT_EQ(intfStatus->operstate(), example::IntfStateUp) << "Interface status object has wrong oper state";
    }
}

TEST_F(ExampleTest, MetricsTest) {
    usleep(1000);

    // iterate over all interface stats
    int iter_count = 0;
    for (auto iter = delphi::objects::ExampleMetrics::Iterator(); iter.IsNotNil(); iter.Next()) {
        auto tmp = iter.Get();
        ASSERT_TRUE_EVENTUALLY((tmp->RxPkts()->Get() > 1)) << "Interface RxPkts counter is wrong";
        ASSERT_TRUE_EVENTUALLY((tmp->RxPktRate()->Get() > 0)) << "Interface RxPktRate gauge is wrong";
        ASSERT_TRUE_EVENTUALLY((tmp->RxErrors()->Get() > 1)) << "Interface RxErrors counter is wrong";
        iter_count++;
    }
    ASSERT_TRUE((iter_count > 0)) << "Got invalid number of metrics";
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
