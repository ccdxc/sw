// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __TEST_H__
#define __TEST_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace test {

using namespace std;
using namespace upgrade;

extern bool CompatCheckFail;
extern bool PostRestartFail;
extern bool ProcessQuiesceFail;
extern bool LinkDownFail;
extern bool HostDownFail;
extern bool PostHostDownFail;
extern bool SaveStateFail;
extern bool HostUpFail;
extern bool LinkUpFail;
extern bool PostLinkUpFail;

// TestUpgSvc is the service object for test upgrade service 
class TestUpgSvc : public delphi::Service, public enable_shared_from_this<TestUpgSvc> {
private:
    UpgSdkPtr          upgsdk_;
    delphi::SdkPtr     sdk_;
    string             svcName_;
public:
    // TestUpgSvc constructor
    TestUpgSvc(delphi::SdkPtr sk);
    TestUpgSvc(delphi::SdkPtr sk, string name);

    // override service name method
    virtual string Name() { return svcName_; }

    // timer for creating a dummy object
    ev::timer          createTimer;
    void createTimerHandler(ev::timer &watcher, int revents);

    void OnMountComplete(void) {
        UPG_LOG_DEBUG("TestUpgSvc OnMountComplete called! Nothing to be done.");
    }
};
typedef std::shared_ptr<TestUpgSvc> TestUpgSvcPtr;

class ExSvcHandler : public UpgHandler {
public:
    ExSvcHandler(){}

    HdlrResp CompatCheckHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (CompatCheckFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp PostRestartHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (PostRestartFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp ProcessQuiesceHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (ProcessQuiesceFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp LinkDownHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (LinkDownFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp HostDownHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (HostDownFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp PostHostDownHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (PostHostDownFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp SaveStateHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (SaveStateFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp HostUpHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (HostUpFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp LinkUpHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (LinkUpFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    HdlrResp PostLinkUpHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        if (PostLinkUpFail) {
            resp = {.resp=FAIL, .errStr=""};
        }
        return resp;
    }

    void AbortHandler(UpgCtx& upgCtx) {
        return;
    }

    void SuccessHandler(UpgCtx& upgCtx) {
        return;
    }

    void FailedHandler(UpgCtx& upgCtx) {
        return;
    }
};

} // namespace test

#endif // __TEST_H__
