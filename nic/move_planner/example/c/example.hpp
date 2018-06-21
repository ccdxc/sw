// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/export/c/upgrade.hpp"
#include "nic/move_planner/lib/planner.h"
#include "fte_example.hpp"

namespace example {

using namespace std;
using namespace upgrade;

// ExUpgSvc is the service object for example upgrade service 
class ExUpgSvc : public delphi::Service, public enable_shared_from_this<ExUpgSvc> {
private:
    UpgSdkPtr          upgsdk_;
    delphi::SdkPtr     sdk_;
    string             svcName_;

public:
    // ExUpgSvc constructor
    ExUpgSvc(delphi::SdkPtr sk);
    ExUpgSvc(delphi::SdkPtr sk, string name);

    // override service name method
    virtual string Name() { return svcName_; }

    // timer for creating a dummy object
    ev::timer          createTimer;
    void createTimerHandler(ev::timer &watcher, int revents);

    void OnMountComplete(void) {
        LogInfo("ExUpgSvc OnMountComplete called! Nothing to be done.");
    }
};
typedef std::shared_ptr<ExUpgSvc> ExUpgSvcPtr;

class ExSvcHandler : public UpgHandler {
    planner::planner_t planner;

public:
    ExSvcHandler(){}
    HdlrResp UpgStateReqCreate(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler UpgStateReqCreate called for the SVC!!");
        return resp;
    }

    HdlrResp UpgStateReqDelete(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler UpgStateReqDelete called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateUpgReqRcvd(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        planner::planner_t plan;

        LogInfo("UpgHandler HandleStateUpgReqRcvd called for the SVC!!");

        if(planner::setup_plan("/sw/nic/move_planner/example/c/hal_mem_example.json",
                               "/sw/nic/move_planner/example/c/hal_mem_example_after.json",
                               65536,
                               plan, false) != planner::PLAN_SUCCESS) {
            resp.resp = FAIL;
            resp.errStr = "Could not setup plan. Metadata files are not valid.";
	    return resp;
        }

        if(planner::check_upgrade(plan) != planner::PLAN_SUCCESS) {
	    resp.resp = FAIL;
	    resp.errStr = "Upgrade cannot be handled";
	}

        return resp;
    }

    HdlrResp HandleStateProcessesQuiesced(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler HandleStateProcessesQuiesced called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateDataplaneDowntimePhase1Start(UpgCtx& upgCtx) {
        HdlrResp resp;
        char ch;
	planner::planner_t plan;

        LogInfo("UpgHandler HandleStateDataplaneDowntimePhase1Start called for the SVC!! - Press any button to continue move...");
 	scanf("%c", &ch);

        if(planner::plan_and_move()) {
            resp = {.resp=SUCCESS, .errStr=""};
        } else {
            resp = {.resp=FAIL, .errStr="LALALALA: Example could not do HandleStateDataplaneDowntimePhase1Start"};
        }
        return resp;
    }

    HdlrResp HandleStateCleanup(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler HandleStateCleanup called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateUpgSuccess(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler HandleStateUpgSuccess called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateUpgFailed(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler HandleStateUpgFailed called for the SVC!!");
        return resp;
    }
};

} // namespace example

#endif // __EXAMPLE_H__
