// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_H__
#define __UPGRADE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"
#include "upgrade_handler.hpp"
#include "upgrade_app_resp_hdlr.hpp"
#include "upgrade_req_reactor.hpp"
 
namespace upgrade {

using namespace std;

class UpgSdk : public delphi::Service {
    delphi::SdkPtr sdk_;
    UpgReqReactorPtr upgReqReactPtr_;
    UpgAppRespHdlrPtr upgAppRespPtr_;
public:
    UpgSdk(delphi::SdkPtr sk, string name) {
        sdk_ = sk;
        upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, name, upgAppRespPtr_);
        delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
        delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadWriteMode);
    }

    UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name) {
        sdk_ = sk;
        upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, uh, name, upgAppRespPtr_);
        delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
        delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadWriteMode);
    }

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete(void);

    void SendAppRespSuccess(void);
    void SendAppRespFail(void);
};
typedef std::shared_ptr<UpgSdk> UpgSdkPtr;

} // namespace upgrade

#endif // __UPGRAGE_H__
