// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "gtest/gtest.h"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/upgrade_manager/upgrade/upgrade.delphi_utest.hpp"

namespace {

using namespace std;
using namespace upgrade;


// --------------------- Reactor test upgrade ------------------------

UPGSTATEREQ_REACTOR_TEST(UpgradeReactorTest, UpgStateReqReact);

TEST_F(UpgradeReactorTest, UpgAppResponseTest) {
    // create an upgrade request spec object
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
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

TEST_F(UpgradeReactorTest, UpgStateMachineTestNonDisruptive) {
    // create an upgrade request spec object
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_upgreqstate(UpgStateCompatCheck);
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

    req->set_upgreqstate(UpgStateProcessQuiesce);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateProcessQuiesceRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStatePostBinRestart);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStatePostBinRestartRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateDataplaneDowntimePhase1);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateDataplaneDowntimePhase1RespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateDataplaneDowntimePhase2);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateDataplaneDowntimePhase2RespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateDataplaneDowntimePhase3);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateDataplaneDowntimePhase3RespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateDataplaneDowntimePhase4);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateDataplaneDowntimePhase4RespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateSuccess);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateSuccessRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateCleanup);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateCleanupRespPass) << "UpgAppResp object has wrong oper state";

}

TEST_F(UpgradeReactorTest, UpgStateMachineTestDisruptive) {
    // create an upgrade request spec object
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_upgreqstate(UpgStateCompatCheck);
    req->set_upgreqtype(UpgTypeDisruptive);
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

    req->set_upgreqstate(UpgStateProcessQuiesce);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateProcessQuiesceRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateLinkDown);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateLinkDownRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateDataplaneDowntimePhase1);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateDataplaneDowntimePhase1RespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStatePostBinRestart);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStatePostBinRestartRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateSuccess);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateSuccessRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateCleanup);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateCleanupRespPass) << "UpgAppResp object has wrong oper state";

}

TEST_F(UpgradeReactorTest, UpgStateMachineAbortTestNonDisruptive) {
    // create an upgrade request spec object
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_upgreqstate(UpgStateCompatCheck);
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

    req->set_upgreqstate(UpgStateProcessQuiesce);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateProcessQuiesceRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateAbort);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateAbortRespPass) << "UpgAppResp object has wrong oper state";

}

TEST_F(UpgradeReactorTest, UpgStateMachineAbortTestDisruptive) {
    // create an upgrade request spec object
    delphi::objects::UpgStateReqPtr req = make_shared<delphi::objects::UpgStateReq>();
    req->set_upgreqstate(UpgStateCompatCheck);
    req->set_upgreqtype(UpgTypeDisruptive);
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

    req->set_upgreqstate(UpgStateProcessQuiesce);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateProcessQuiesceRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateLinkDown);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateLinkDownRespPass) << "UpgAppResp object has wrong oper state";

    req->set_upgreqstate(UpgStateAbort);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgAppResp::FindObject(sdk_, appresp)->upgapprespval(),
                        UpgStateAbortRespPass) << "UpgAppResp object has wrong oper state";

}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
