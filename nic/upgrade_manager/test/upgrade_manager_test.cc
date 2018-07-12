// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "gtest/gtest.h"
#include "nic/upgrade_manager/lib/upgrade.hpp"
#include "nic/upgrade_manager/upgrade/upgrade.delphi_utest.hpp"

namespace {

using namespace std;
using namespace upgrade;


// --------------------- Reactor test upgrade ------------------------

UPGREQ_REACTOR_TEST(UpgradeReactorTest, UpgradeMgr);

TEST_F(UpgradeReactorTest, BasicTest) {
    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    sdk_->QueueUpdate(req);

    // verify spec object is in db
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("UpgStateReq").size(), 1) << "Upgrade request status object was not created";

    // verify spec object in db has changed
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgReq::FindObject(sdk_, req)->upgreqcmd(),
                        UpgStart) << "Upgrade request spec object has wrong oper state";

    // verify status object has correct operstate too
    delphi::objects::UpgStateReqPtr upgReqStatusKey = make_shared<delphi::objects::UpgStateReq>();
    upgReqStatusKey->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object has wrong oper state";
}


// --------------------- Service test upgrade ------------------------
// init the delphi utest
DELPHI_SERVICE_TEST(UpgradeTest, UpgradeService);

TEST_F(UpgradeTest, UpgStateReqCreateTest) {
    usleep(1000);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    sdk_->QueueUpdate(req);

    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    delphi::objects::UpgStateReqPtr upgReqStatusKey = make_shared<delphi::objects::UpgStateReq>();
    upgReqStatusKey->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object has wrong oper state";
}

TEST_F(UpgradeTest, StateMachineMoveToCompatCheckTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    delphi::objects::UpgStateReqPtr upgReqStatusKey = make_shared<delphi::objects::UpgStateReq>();
    upgReqStatusKey->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

}

TEST_F(UpgradeTest, StateMachineTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    delphi::objects::UpgStateReqPtr upgReqStatusKey = make_shared<delphi::objects::UpgStateReq>();
    upgReqStatusKey->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    appresp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStatePostBinRestart) << "Upgrade Request status object does not have UpgStatePostBinRestart state";

    appresp->set_upgapprespval(UpgStatePostBinRestartRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase1) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase1 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase1RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase2) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase2 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase2RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase3) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase3 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase3RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase4) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase4 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase4RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 0) << "UpgResp object was created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";

    appresp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCleanup) << "Upgrade Request status object does not have UpgStateCleanup state";

    appresp->set_upgapprespval(UpgStateCleanupRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";
}

TEST_F(UpgradeTest, CleanupPostStateMachineTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    delphi::objects::UpgStateReqPtr upgReqStatusKey = make_shared<delphi::objects::UpgStateReq>();
    upgReqStatusKey->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    appresp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStatePostBinRestart) << "Upgrade Request status object does not have UpgStatePostBinRestart state";

    appresp->set_upgapprespval(UpgStatePostBinRestartRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase1) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase1 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase1RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase2) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase2 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase2RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase3) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase3 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase3RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase4) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase4 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase4RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 0) << "UpgResp object was created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";

    appresp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCleanup) << "Upgrade Request status object does not have UpgStateCleanup state";

    appresp->set_upgapprespval(UpgStateCleanupRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";
}

TEST_F(UpgradeTest, StateMachineAbortTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    delphi::objects::UpgStateReqPtr upgReqStatusKey = make_shared<delphi::objects::UpgStateReq>();
    upgReqStatusKey->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    appresp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStatePostBinRestart) << "Upgrade Request status object does not have UpgStatePostBinRestart state";

    req->set_upgreqcmd(UpgAbort);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateAbort) << "Upgrade Request status object does not have UpgStateAbort state";

    appresp->set_upgapprespval(UpgStateAbortRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
