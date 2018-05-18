// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_REQ_REACTOR_H__
#define __UPGRADE_REQ_REACTOR_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"
#include "upgrade_handler.hpp"
#include "upgrade_app_resp_hdlr.hpp"

namespace upgrade {

using namespace std;

// UpgReqReactor is the reactor for the UpgReqStatus object
class UpgReqReactor : public delphi::objects::UpgReqStatusReactor {
    delphi::SdkPtr sdk_;
    UpgHandlerPtr upgHdlrPtr_;
    UpgAppRespHdlrPtr upgAppRespPtr_;
    delphi::objects::UpgReqStatusPtr upgReqStatus_;
public:
    UpgReqReactor(delphi::SdkPtr sk, string name, UpgAppRespHdlrPtr ptr) {
        this->sdk_ = sk;
        this->upgHdlrPtr_ = make_shared<UpgHandler>();
        this->upgAppRespPtr_ = ptr;
    }

    UpgReqReactor(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, UpgAppRespHdlrPtr ptr) {
        this->sdk_ = sk;
        this->upgHdlrPtr_ = uh;
        this->upgAppRespPtr_ = ptr;
    }

    // OnUpgReqStatusCreate gets called when UpgReqStatus object is created
    virtual delphi::error OnUpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req);

    // OnUpgReqStatusDelete gets called when UpgReqStatus object is deleted
    virtual delphi::error OnUpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req);

    // OnUpgReqState gets called when UpgReqState attribute changes
    virtual delphi::error OnUpgReqState(delphi::objects::UpgReqStatusPtr req);
};
typedef std::shared_ptr<UpgReqReactor> UpgReqReactorPtr;

} // namespace upgrade

#endif // __UPGRADE_REQ_REACTOR_H__
