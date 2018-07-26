// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "gtest/gtest.h"
#include "nic/upgrade_manager/lib/upgrade.hpp"
#include "nic/upgrade_manager/upgrade/upgrade.delphi_utest.hpp"

namespace {

using namespace std;
using namespace upgrade;


// --------------------- Reactor test upgrade ------------------------

UPGREQ_REACTOR_TEST(UpgradeReactorTest, UpgReqReact);

TEST_F(UpgradeReactorTest, BasicTest) {
    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);

    // verify spec object is in db
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("UpgStateReq").size(), 0) << "Upgrade request status object was not created";
}


// --------------------- Service test upgrade ------------------------
// init the delphi utest
DELPHI_SERVICE_TEST(UpgradeTest, UpgradeService);

TEST_F(UpgradeTest, UpgStateReqCreateTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

TEST_F(UpgradeTest, UpgTypeNonDisruptiveTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
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
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqtype(),
                        UpgTypeNonDisruptive) << "Upgrade Request status object does not have UpgTypeNonDisruptive";
}

TEST_F(UpgradeTest, UpgTypeDisruptiveTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeDisruptive);
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
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqtype(),
                        UpgTypeDisruptive) << "Upgrade Request status object does not have UpgTypeDisruptive";
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
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

TEST_F(UpgradeTest, CheckStateMachineWithNoAppRegistering) {
    usleep(1000);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 0) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgStateReq status object was created";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    delphi::objects::UpgRespPtr upgRespPtr = make_shared<delphi::objects::UpgResp>();
    upgRespPtr->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_, upgRespPtr)->upgrespval(),
                        UpgRespFail) << "Upgrade response not set to fail";
    string testStr = "No app registered for upgrade";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_, upgRespPtr)->upgrespfailstr(0),
                        testStr) << "Upgrade response str not set to No app registered for upgrade";
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
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

TEST_F(UpgradeTest, StateMachineTestWithTwoApps) {
    usleep(1000);

    delphi::objects::UpgAppPtr app1 = make_shared<delphi::objects::UpgApp>();
    app1->set_key("app1");
    sdk_->QueueUpdate(app1);
    delphi::objects::UpgAppPtr app2 = make_shared<delphi::objects::UpgApp>();
    app2->set_key("app2");
    sdk_->QueueUpdate(app2);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 2) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    delphi::objects::UpgStateReqPtr upgReqStatusKey = make_shared<delphi::objects::UpgStateReq>();
    upgReqStatusKey->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr app1resp = make_shared<delphi::objects::UpgAppResp>();
    app1resp->set_key("app1");
    app1resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    delphi::objects::UpgAppRespPtr app2resp = make_shared<delphi::objects::UpgAppResp>();
    app2resp->set_key("app2");
    app2resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    app1resp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    app2resp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStatePostBinRestart) << "Upgrade Request status object does not have UpgStatePostBinRestart state";

    app1resp->set_upgapprespval(UpgStatePostBinRestartRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStatePostBinRestart) << "Upgrade Request status object does not have UpgStatePostBinRestart state";

    app2resp->set_upgapprespval(UpgStatePostBinRestartRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase1) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase1 state";

    app1resp->set_upgapprespval(UpgStateDataplaneDowntimePhase1RespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase1) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase1 state";

    app2resp->set_upgapprespval(UpgStateDataplaneDowntimePhase1RespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase2) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase2 state";

    app1resp->set_upgapprespval(UpgStateDataplaneDowntimePhase2RespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase2) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase2 state";

    app2resp->set_upgapprespval(UpgStateDataplaneDowntimePhase2RespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase3) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase3 state";

    app1resp->set_upgapprespval(UpgStateDataplaneDowntimePhase3RespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase3) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase3 state";

    app2resp->set_upgapprespval(UpgStateDataplaneDowntimePhase3RespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase4) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase4 state";

    app1resp->set_upgapprespval(UpgStateDataplaneDowntimePhase4RespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase4) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase4 state";

    app2resp->set_upgapprespval(UpgStateDataplaneDowntimePhase4RespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 0) << "UpgResp object was created";

    app1resp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";
    app2resp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCleanup) << "Upgrade Request status object does not have UpgStateCleanup state";

    app1resp->set_upgapprespval(UpgStateCleanupRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCleanup) << "Upgrade Request status object does not have UpgStateCleanup state";

    app2resp->set_upgapprespval(UpgStateCleanupRespPass);
    sdk_->QueueUpdate(app2resp);
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
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgStateReq status object was not deleted";
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
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

TEST_F(UpgradeTest, StateMachineAppFailTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

    appresp->set_upgapprespval(UpgStatePostBinRestartRespFail);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    appresp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCleanup) << "Upgrade Request status object does not have UpgStateTerminal state";

    appresp->set_upgapprespval(UpgStateCleanupRespPass);
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

TEST_F(UpgradeTest, StateMachineAppFailStringTest) {
    usleep(1000);

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
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

    appresp->set_upgapprespval(UpgStatePostBinRestartRespFail);
    appresp->set_upgapprespstr("BABA");
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    appresp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCleanup) << "Upgrade Request status object does not have UpgStateTerminal state";

    appresp->set_upgapprespval(UpgStateCleanupRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    delphi::objects::UpgRespPtr upgRespPtr = make_shared<delphi::objects::UpgResp>();
    upgRespPtr->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_, upgRespPtr)->upgrespval(),
                        UpgRespFail) << "Upgrade response not set to fail";
    string testStr = "App app1 returned failure: BABA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_, upgRespPtr)->upgrespfailstr(0),
                        testStr) << "Upgrade response str not set to BABA";
    

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
}

TEST_F(UpgradeTest, StateMachineAppFailStringTestWithTwoApps) {
    usleep(1000);

    delphi::objects::UpgAppPtr app1 = make_shared<delphi::objects::UpgApp>();
    app1->set_key("app1");
    sdk_->QueueUpdate(app1);
    usleep(1000 * 100);
    delphi::objects::UpgAppPtr app2 = make_shared<delphi::objects::UpgApp>();
    app2->set_key("app2");
    sdk_->QueueUpdate(app2);
    usleep(1000 * 100);

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_key(10);
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 2) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    delphi::objects::UpgStateReqPtr upgReqStatusKey = make_shared<delphi::objects::UpgStateReq>();
    upgReqStatusKey->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr app1resp = make_shared<delphi::objects::UpgAppResp>();
    app1resp->set_key("app1");
    app1resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    delphi::objects::UpgAppRespPtr app2resp = make_shared<delphi::objects::UpgAppResp>();
    app2resp->set_key("app2");
    app2resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    app1resp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    app2resp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStatePostBinRestart) << "Upgrade Request status object does not have UpgStatePostBinRestart state";

    app1resp->set_upgapprespval(UpgStatePostBinRestartRespFail);
    app1resp->set_upgapprespstr("BABA");
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStatePostBinRestart) << "Upgrade Request status object does not have UpgStatePostBinRestart state";

    app2resp->set_upgapprespval(UpgStatePostBinRestartRespFail);
    app2resp->set_upgapprespstr("LALA");
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    app1resp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    app2resp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCleanup) << "Upgrade Request status object does not have UpgStateTerminal state";

    app1resp->set_upgapprespval(UpgStateCleanupRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateCleanup) << "Upgrade Request status object does not have UpgStateTerminal state";

    app2resp->set_upgapprespval(UpgStateCleanupRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_, upgReqStatusKey)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    delphi::objects::UpgRespPtr upgRespPtr = make_shared<delphi::objects::UpgResp>();
    upgRespPtr->set_key(10);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_, upgRespPtr)->upgrespval(),
                        UpgRespFail) << "Upgrade response not set to fail";
    string testStrApp1 = "App app1 returned failure: BABA";
    string testStrApp2 = "App app2 returned failure: LALA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_, upgRespPtr)->upgrespfailstr(1),
                        testStrApp1) << "Upgrade response str not set to BABA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_, upgRespPtr)->upgrespfailstr(0),
                        testStrApp2) << "Upgrade response str not set to LALA";
    

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
