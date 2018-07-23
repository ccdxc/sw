// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

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
        UPG_LOG_DEBUG("ExUpgSvc OnMountComplete called! Nothing to be done.");
    }
};
typedef std::shared_ptr<ExUpgSvc> ExUpgSvcPtr;

class ExSvcHandler : public UpgHandler {
public:
    ExSvcHandler(){}
    HdlrResp UpgStateReqCreate(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        UPG_LOG_DEBUG("UpgHandler UpgStateReqCreate called for the SVC!");
        return resp;
    }

    HdlrResp UpgStateReqDelete(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        UPG_LOG_DEBUG("UpgHandler UpgStateReqDelete called for the SVC!");
        return resp;
    }

    HdlrResp HandleUpgStateProcessQuiesce(UpgCtx& upgCtx) {
        //HdlrResp resp = {.resp=INPROGRESS, .errStr=""};
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        UPG_LOG_DEBUG("UpgHandler HandleUpgStateProcessQuiesce called for the SVC!");

        UPG_LOG_DEBUG("UpgType {}", upgCtx.upgType);
        int version;
        delphi::error err = UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx, "TABLE-7", version);
        if (err != delphi::error::OK()) {
            UPG_LOG_DEBUG("Error from API {}", err.Error());
        }
        UPG_LOG_DEBUG("Version for TABLE-7 table is {}", version);
        err = UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx, "TABLE-10", version);
        if (err != delphi::error::OK()) {
            UPG_LOG_DEBUG("Error from API {}", err.Error());
        }
        return resp;
    }

    HdlrResp HandleUpgStateDataplaneDowntimePhase1(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        //HdlrResp resp = {.resp=FAIL, .errStr="LALALALA: Example could not do HandleUpgStateDataplaneDowntimePhase1"};
        UPG_LOG_DEBUG("UpgHandler HandleUpgStateDataplaneDowntimePhase1 called for the SVC!");
        return resp;
    }

    HdlrResp HandleUpgStateCleanup(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        UPG_LOG_DEBUG("UpgHandler HandleUpgStateCleanup called for the SVC!");
        return resp;
    }

    void HandleUpgStateSuccess(UpgCtx& upgCtx) {
        UPG_LOG_DEBUG("UpgHandler HandleUpgStateSuccess called for the SVC!");
        return;
    }

    void HandleUpgStateFailed(UpgCtx& upgCtx) {
        UPG_LOG_DEBUG("UpgHandler HandleUpgStateFailed called for the SVC!");
        return;
    }
};

} // namespace example

#endif // __EXAMPLE_H__
