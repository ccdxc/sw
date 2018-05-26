// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/export/upgrade.hpp"

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
        LogInfo("ExUpgSvc OnMountComplete called!!");
        this->upgsdk_->OnMountComplete();
    }
};
typedef std::shared_ptr<ExUpgSvc> ExUpgSvcPtr;

class ExSvcHandler : public upgrade::UpgHandler {
public:
    ExSvcHandler(){}
    HdlrResp UpgStateReqCreate(delphi::objects::UpgStateReqPtr req) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler UpgStateReqCreate called for the SVC!!");
        return resp;
    }

    HdlrResp UpgStateReqDelete(delphi::objects::UpgStateReqPtr req) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler UpgStateReqDelete called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateUpgReqRcvd(delphi::objects::UpgStateReqPtr req) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler HandleStateUpgReqRcvd called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateProcessesQuiesced(delphi::objects::UpgStateReqPtr req) {
        HdlrResp resp = {.resp=INPROGRESS, .errStr=""};
        LogInfo("UpgHandler HandleStateProcessesQuiesced called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateDataplaneDowntimeStart(delphi::objects::UpgStateReqPtr req) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        //HdlrResp resp = {.resp=FAIL, .errStr="Example could not do HandleStateDataplaneDowntimeStart"};
        LogInfo("UpgHandler HandleStateDataplaneDowntimeStart called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateCleanup(delphi::objects::UpgStateReqPtr req) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler HandleStateCleanup called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateUpgSuccess(delphi::objects::UpgStateReqPtr req) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler HandleStateUpgSuccess called for the SVC!!");
        return resp;
    }

    HdlrResp HandleStateUpgFailed(delphi::objects::UpgStateReqPtr req) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        LogInfo("UpgHandler HandleStateUpgFailed called for the SVC!!");
        return resp;
    }
};

} // namespace example

#endif // __EXAMPLE_H__
