// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "gtest/gtest.h"
#include "nic/upgrade_manager/lib/upgrade.hpp"
#include "gen/proto/upgrade.delphi_utest.hpp"

namespace {

using namespace std;
using namespace upgrade;


// --------------------- Reactor test upgrade ------------------------

UPGREQ_REACTOR_TEST(UpgradeReactorTest, UpgReqReact);

TEST_F(UpgradeReactorTest, BasicTest) {
    // create an upgrade request spec object
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);

    // verify spec object is in db
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("UpgStateReq").size(), 0) << "Upgrade request status object was not created";
    upgmetptr->Delete();
}


// --------------------- Service test upgrade ------------------------
// init the delphi utest
DELPHI_SERVICE_TEST(UpgradeTest, UpgradeService);

TEST_F(UpgradeTest, UpgStateReqCreateTest) {
    usleep(1000);

    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);

    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object has wrong oper state";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, UpgTypeNonDisruptiveTest) {
    usleep(1000);

    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);

    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object has wrong oper state";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqtype(),
                        UpgTypeNonDisruptive) << "Upgrade Request status object does not have UpgTypeNonDisruptive";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, UpgTypeDisruptiveTest) {
    usleep(1000);

    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeDisruptive);
    sdk_->QueueUpdate(req);

    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object has wrong oper state";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqtype(),
                        UpgTypeDisruptive) << "Upgrade Request status object does not have UpgTypeDisruptive";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineMoveToCompatCheckTest) {
    usleep(1000);

    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, CheckStateMachineWithNoAppRegistering) {
    usleep(1000);

    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 0) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgStateReq status object was created";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespval(),
                        UpgRespFail) << "Upgrade response not set to fail";
    string testStr = "No app registered for upgrade";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespfailstr(0),
                        testStr) << "Upgrade response str not set to No app registered for upgrade";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, UpgradeNonDisruptiveStateMachineTest) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    appresp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    appresp->set_upgapprespval(UpgStatePostRestartRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase1) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase1 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase1RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase2) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase2 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase2RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase3) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase3 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase3RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase4) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase4 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase4RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 0) << "UpgResp object was created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";

    appresp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, UpgradeDisruptiveStateMachineTest) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkDown) << "Upgrade Request status object does not have UpgStateLinkDown state";

    appresp->set_upgapprespval(UpgStateLinkDownRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateHostDown) << "Upgrade Request status object does not have UpgStateHostDown state";

    appresp->set_upgapprespval(UpgStateHostDownRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostHostDown) << "Upgrade Request status object does not have UpgStatePostHostDown state";

    appresp->set_upgapprespval(UpgStatePostHostDownRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSaveState) << "Upgrade Request status object does not have UpgStateSaveState state";

    appresp->set_upgapprespval(UpgStateSaveStateRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    appresp->set_upgapprespval(UpgStatePostRestartRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateHostUp) << "Upgrade Request status object does not have UpgStateHostUp state";

    appresp->set_upgapprespval(UpgStateHostUpRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkUp) << "Upgrade Request status object does not have UpgStateLinkUp state";

    appresp->set_upgapprespval(UpgStateLinkUpRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostLinkUp) << "Upgrade Request status object does not have UpgStatePostLinkUp state";

    appresp->set_upgapprespval(UpgStatePostLinkUpRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 0) << "UpgResp object was created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";

    appresp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, UpgradePossibleStateMachineTest) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(IsUpgPossible);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateUpgPossible) << "Upgrade Request status object does not have UpgStateUpgPossible state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateUpgPossibleRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 1) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, UpgradePossibleFailStateMachineTest) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(IsUpgPossible);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 1) << "invalid default value for counter";
    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateUpgPossible) << "Upgrade Request status object does not have UpgStateUpgPossible state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateUpgPossibleRespFail);
    appresp->set_upgapprespstr("BABA");
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 1) << "invalid default value for counter";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";
    string testStr = "App app1 returned failure: BABA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespfailstr(0),
                        testStr) << "Upgrade response str not set to BABA";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineTestWithTwoApps) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app1 = make_shared<delphi::objects::UpgApp>();
    app1->set_key("app1");
    sdk_->QueueUpdate(app1);
    delphi::objects::UpgAppPtr app2 = make_shared<delphi::objects::UpgApp>();
    app2->set_key("app2");
    sdk_->QueueUpdate(app2);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 2) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 2) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr app1resp = make_shared<delphi::objects::UpgAppResp>();
    app1resp->set_key("app1");
    app1resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    delphi::objects::UpgAppRespPtr app2resp = make_shared<delphi::objects::UpgAppResp>();
    app2resp->set_key("app2");
    app2resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    app1resp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    app2resp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    app1resp->set_upgapprespval(UpgStatePostRestartRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    app2resp->set_upgapprespval(UpgStatePostRestartRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase1) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase1 state";

    app1resp->set_upgapprespval(UpgStateDataplaneDowntimePhase1RespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase1) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase1 state";

    app2resp->set_upgapprespval(UpgStateDataplaneDowntimePhase1RespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase2) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase2 state";

    app1resp->set_upgapprespval(UpgStateDataplaneDowntimePhase2RespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase2) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase2 state";

    app2resp->set_upgapprespval(UpgStateDataplaneDowntimePhase2RespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase3) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase3 state";

    app1resp->set_upgapprespval(UpgStateDataplaneDowntimePhase3RespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase3) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase3 state";

    app2resp->set_upgapprespval(UpgStateDataplaneDowntimePhase3RespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase4) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase4 state";

    app1resp->set_upgapprespval(UpgStateDataplaneDowntimePhase4RespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase4) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase4 state";

    app2resp->set_upgapprespval(UpgStateDataplaneDowntimePhase4RespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 0) << "UpgResp object was created";

    app1resp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";
    app2resp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineTestWithTwoAppsDisruptive) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app1 = make_shared<delphi::objects::UpgApp>();
    app1->set_key("app1");
    sdk_->QueueUpdate(app1);
    delphi::objects::UpgAppPtr app2 = make_shared<delphi::objects::UpgApp>();
    app2->set_key("app2");
    sdk_->QueueUpdate(app2);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 2) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 2) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr app1resp = make_shared<delphi::objects::UpgAppResp>();
    app1resp->set_key("app1");
    app1resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    delphi::objects::UpgAppRespPtr app2resp = make_shared<delphi::objects::UpgAppResp>();
    app2resp->set_key("app2");
    app2resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkDown) << "Upgrade Request status object does not have UpgStateLinkDown state";

    app1resp->set_upgapprespval(UpgStateLinkDownRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkDown) << "Upgrade Request status object does not have UpgStateLinkDown state";

    app2resp->set_upgapprespval(UpgStateLinkDownRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateHostDown) << "Upgrade Request status object does not have UpgStateHostDown state";

    app2resp->set_upgapprespval(UpgStateHostDownRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateHostDown) << "Upgrade Request status object does not have UpgStateHostDown state";

    app1resp->set_upgapprespval(UpgStateHostDownRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostHostDown) << "Upgrade Request status object does not have UpgStatePostHostDown state";

    app1resp->set_upgapprespval(UpgStatePostHostDownRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostHostDown) << "Upgrade Request status object does not have UpgStatePostHostDown state";

    app2resp->set_upgapprespval(UpgStatePostHostDownRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSaveState) << "Upgrade Request status object does not have UpgStateSaveState state";

    app1resp->set_upgapprespval(UpgStateSaveStateRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSaveState) << "Upgrade Request status object does not have UpgStateSaveState state";

    app2resp->set_upgapprespval(UpgStateSaveStateRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    app1resp->set_upgapprespval(UpgStatePostRestartRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    app2resp->set_upgapprespval(UpgStatePostRestartRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateHostUp) << "Upgrade Request status object does not have UpgStateHostUp state";

    app1resp->set_upgapprespval(UpgStateHostUpRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateHostUp) << "Upgrade Request status object does not have UpgStateHostUp state";

    app2resp->set_upgapprespval(UpgStateHostUpRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 0) << "UpgResp object was created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkUp) << "Upgrade Request status object does not have UpgStateLinkUp state";

    app1resp->set_upgapprespval(UpgStateLinkUpRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkUp) << "Upgrade Request status object does not have UpgStateLinkUp state";
    app2resp->set_upgapprespval(UpgStateLinkUpRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostLinkUp) << "Upgrade Request status object does not have UpgStatePostLinkUp state";

    app1resp->set_upgapprespval(UpgStatePostLinkUpRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostLinkUp) << "Upgrade Request status object does not have UpgStatePostLinkUp state";
    app2resp->set_upgapprespval(UpgStatePostLinkUpRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";
    app1resp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";

    app2resp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, CleanupPostStateMachineTest) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    appresp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    appresp->set_upgapprespval(UpgStatePostRestartRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase1) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase1 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase1RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase2) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase2 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase2RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase3) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase3 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase3RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateDataplaneDowntimePhase4) << "Upgrade Request status object does not have UpgStateDataplaneDowntimePhase4 state";

    appresp->set_upgapprespval(UpgStateDataplaneDowntimePhase4RespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 0) << "UpgResp object was created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateSuccess) << "Upgrade Request status object does not have UpgStateSuccess state";

    appresp->set_upgapprespval(UpgStateSuccessRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgStateReq status object was not deleted";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineAbortTest) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";


    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    appresp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    req->set_upgreqcmd(UpgAbort);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateAbort) << "Upgrade Request status object does not have UpgStateAbort state";

    appresp->set_upgapprespval(UpgStateAbortRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineAbortTestDisruptive) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkDown) << "Upgrade Request status object does not have UpgStateLinkDown state";

    req->set_upgreqcmd(UpgAbort);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateAbort) << "Upgrade Request status object does not have UpgStateAbort state";

    appresp->set_upgapprespval(UpgStateAbortRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineAppFailTest) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    appresp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    appresp->set_upgapprespval(UpgStatePostRestartRespFail);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    appresp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineAppFailTestDisruptive) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkDown) << "Upgrade Request status object does not have UpgStateLinkDown state";

    appresp->set_upgapprespval(UpgStateLinkDownRespFail);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    appresp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineAppFailStringTest) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app = make_shared<delphi::objects::UpgApp>();
    app->set_key("app1");
    sdk_->QueueUpdate(app);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 1) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 1) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr appresp = make_shared<delphi::objects::UpgAppResp>();
    appresp->set_key("app1");
    appresp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    appresp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    appresp->set_upgapprespval(UpgStatePostRestartRespFail);
    appresp->set_upgapprespstr("BABA");
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    appresp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(appresp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespval(),
                        UpgRespFail) << "Upgrade response not set to fail";
    string testStr = "App app1 returned failure: BABA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespfailstr(0),
                        testStr) << "Upgrade response str not set to BABA";
    

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineAppFailStringTestWithTwoApps) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app1 = make_shared<delphi::objects::UpgApp>();
    app1->set_key("app1");
    sdk_->QueueUpdate(app1);
    usleep(1000 * 100);
    delphi::objects::UpgAppPtr app2 = make_shared<delphi::objects::UpgApp>();
    app2->set_key("app2");
    sdk_->QueueUpdate(app2);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 2) << "invalid default value for counter";

    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeNonDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 2) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr app1resp = make_shared<delphi::objects::UpgAppResp>();
    app1resp->set_key("app1");
    app1resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    delphi::objects::UpgAppRespPtr app2resp = make_shared<delphi::objects::UpgAppResp>();
    app2resp->set_key("app2");
    app2resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    app1resp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateProcessQuiesce) << "Upgrade Request status object does not have UpgStateProcessQuiesce state";

    app2resp->set_upgapprespval(UpgStateProcessQuiesceRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    app1resp->set_upgapprespval(UpgStatePostRestartRespFail);
    app1resp->set_upgapprespstr("BABA");
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStatePostRestart) << "Upgrade Request status object does not have UpgStatePostRestart state";

    app2resp->set_upgapprespval(UpgStatePostRestartRespFail);
    app2resp->set_upgapprespstr("LALA");
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    app1resp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    app2resp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespval(),
                        UpgRespFail) << "Upgrade response not set to fail";
    string testStrApp1 = "App app1 returned failure: BABA";
    string testStrApp2 = "App app2 returned failure: LALA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespfailstr(1),
                        testStrApp1) << "Upgrade response str not set to BABA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespfailstr(0),
                        testStrApp2) << "Upgrade response str not set to LALA";
    

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

TEST_F(UpgradeTest, StateMachineAppFailStringTestWithTwoAppsDisruptive) {
    usleep(1000);
    delphi::objects::UpgradeMetricsPtr upgmetptr = delphi::objects::UpgradeMetrics::NewUpgradeMetrics(1);
    ASSERT_TRUE(upgmetptr != NULL) << "Failed to create upgmetptr";
    ASSERT_EQ(upgmetptr->IsUpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NonDisruptiveUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->SuccessfulUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->AbortedUpg()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgPossible()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(upgmetptr->UpgNotPossible()->Get(), 0) << "invalid default value for counter";

    delphi::objects::UpgAppPtr app1 = make_shared<delphi::objects::UpgApp>();
    app1->set_key("app1");
    sdk_->QueueUpdate(app1);
    usleep(1000 * 100);
    delphi::objects::UpgAppPtr app2 = make_shared<delphi::objects::UpgApp>();
    app2->set_key("app2");
    sdk_->QueueUpdate(app2);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->NumRegApps()->Get(), 2) << "invalid default value for counter";


    // create an upgrade request spec object
    delphi::objects::UpgReqPtr req = make_shared<delphi::objects::UpgReq>();
    req->set_upgreqcmd(UpgStart);
    req->set_upgreqtype(UpgTypeDisruptive);
    sdk_->QueueUpdate(req);
    usleep(1000 * 100);
    ASSERT_EQ(upgmetptr->DisruptiveUpg()->Get(), 1) << "invalid default value for counter";

    // verify app obj 
    ASSERT_EQ(sdk_->ListKind("UpgApp").size(), 2) << "UpgApp object was not created";

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 1) << "Upgrade Request spec object was not created";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 1) << "UpgReq status object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    // Create application response
    delphi::objects::UpgAppRespPtr app1resp = make_shared<delphi::objects::UpgAppResp>();
    app1resp->set_key("app1");
    app1resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateCompatCheck) << "Upgrade Request status object does not have UpgStateCompatCheck state";

    delphi::objects::UpgAppRespPtr app2resp = make_shared<delphi::objects::UpgAppResp>();
    app2resp->set_key("app2");
    app2resp->set_upgapprespval(UpgStateCompatCheckRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkDown) << "Upgrade Request status object does not have UpgStateLinkDown state";

    app1resp->set_upgapprespval(UpgStateLinkDownRespFail);
    app1resp->set_upgapprespstr("BABA");
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateLinkDown) << "Upgrade Request status object does not have UpgStateLinkDown state";

    app2resp->set_upgapprespval(UpgStateLinkDownRespFail);
    app2resp->set_upgapprespstr("LALA");
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    app1resp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(app1resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateFailed) << "Upgrade Request status object does not have UpgStateTerminal state";

    app2resp->set_upgapprespval(UpgStateFailedRespPass);
    sdk_->QueueUpdate(app2resp);
    usleep(1000 * 100);
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgStateReq::FindObject(sdk_)->upgreqstate(),
                        UpgStateTerminal) << "Upgrade Request status object does not have UpgStateTerminal state";

    ASSERT_EQ(sdk_->ListKind("UpgResp").size(), 1) << "UpgResp object was not created";

    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespval(),
                        UpgRespFail) << "Upgrade response not set to fail";
    string testStrApp1 = "App app1 returned failure: BABA";
    string testStrApp2 = "App app2 returned failure: LALA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespfailstr(1),
                        testStrApp1) << "Upgrade response str not set to BABA";
    ASSERT_EQ_EVENTUALLY(delphi::objects::UpgResp::FindObject(sdk_)->upgrespfailstr(0),
                        testStrApp2) << "Upgrade response str not set to LALA";
    

    sdk_->QueueDelete(req);
    usleep(1000 * 100);

    // verify spec object is in the db
    ASSERT_EQ(sdk_->ListKind("UpgReq").size(), 0) << "Upgrade Request spec object was not deleted";

    // verify corresponding status object got created
    ASSERT_EQ(sdk_->ListKind("UpgStateReq").size(), 0) << "UpgReq status object was not deleted";
    ASSERT_EQ(upgmetptr->FailedUpg()->Get(), 1) << "invalid default value for counter";
    upgmetptr->Delete();
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
