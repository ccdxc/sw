// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "gtest/gtest.h"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/upgrade_manager/upgrade/upgrade.delphi_utest.hpp"

namespace {

using namespace std;
using namespace upgrade;


// --------------------- Reactor test upgrade ------------------------

UPGSTATEREQ_REACTOR_TEST(UpgradeReactorTest, UpgReqReactor);

TEST_F(UpgradeReactorTest, UpgAppResponseTest) {
    // create an upgrade request spec object
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_key(10);
    req->set_upgreqstate(UpgStateCompatCheck);
    sdk_->QueueUpdate(req);

    usleep(1000 * 100);

    // verify corresponding status object got created
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("UpgStateReq").size(), 1) << "Upgrade request status object was not created";

    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("UpgAppResp").size(), 1) << "Upgrade app resp object was not created";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("test");

    // verify spec object in db has changed
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateCompatCheckRespPass) << "UpgAppResp object has wrong oper state";
}


} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
