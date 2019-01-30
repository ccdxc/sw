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

    HdlrResp CompatCheckHandler(UpgCtx& upgCtx) {
        //HdlrResp resp = {.resp=INPROGRESS, .errStr=""};
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        UPG_LOG_DEBUG("UpgHandler ProcessQuiesce called for the SVC!");

        UPG_LOG_DEBUG("UpgType {}", upgCtx.upgType);
        string version;
        delphi::error err = UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx, NICMGRVER, version);
        if (err != delphi::error::OK()) {
            UPG_LOG_DEBUG("Error from API {}", err.Error());
        }
        UPG_LOG_DEBUG("Version for NICMGR table is {}", version);


        err = UpgCtxApi::UpgCtxGetPostUpgTableVersion(upgCtx, NICMGRVER, version);
        if (err != delphi::error::OK()) {
            UPG_LOG_DEBUG("Error from API {}", err.Error());
        }
        UPG_LOG_DEBUG("Version for Post NICMGR table is {}", version);

        err = UpgCtxApi::UpgCtxGetPreUpgTableVersion(upgCtx, KERNELVER, version);
        if (err != delphi::error::OK()) {
            UPG_LOG_DEBUG("Error from API {}", err.Error());
        }
        UPG_LOG_DEBUG("Version for kernel is {}", version);

        return resp;
    }

    HdlrResp PostRestartHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        //HdlrResp resp = {.resp=FAIL, .errStr="LALALALA: Example could not do DataplaneDowntimePhase1"};
        UPG_LOG_DEBUG("UpgHandler PostRestartHandler called for the SVC!");
        return resp;
    }

    HdlrResp CleanupHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        UPG_LOG_DEBUG("UpgHandler Cleanup called for the SVC!");
        return resp;
    }

    void SuccessHandler(UpgCtx& upgCtx) {
        UPG_LOG_DEBUG("UpgHandler Success called for the SVC!");
        return;
    }

    void FailedHandler(UpgCtx& upgCtx) {
        UPG_LOG_DEBUG("UpgHandler Failed called for the SVC!");
        return;
    }
};

} // namespace example

#endif // __EXAMPLE_H__
