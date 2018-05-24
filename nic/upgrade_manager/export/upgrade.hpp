// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_H__
#define __UPGRADE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"
#include "upgrade_handler.hpp"
#include "upgrade_agent_handler.hpp"
#include "upgrade_app_resp_hdlr.hpp"
#include "upgrade_app_resp_reactor.hpp"
#include "upgrade_req_reactor.hpp"
#include "upgrade_mgr_agent_resp_reactor.hpp"
 
namespace upgrade {

typedef enum {
    AGENT,
    NON_AGENT,
} SvcRole;

using namespace std;

class UpgSdk : public delphi::Service {
    delphi::SdkPtr sdk_;
    UpgReqReactorPtr upgReqReactPtr_;
    UpgAppRespHdlrPtr upgAppRespPtr_;
    UpgAppRespReactPtr upgAppRespReactPtr_;
    UpgAgentHandlerPtr upgAgentHandlerPtr_;
    UpgMgrAgentRespReactPtr upgMgrAgentRespPtr_;
public:
    UpgSdk(delphi::SdkPtr sk, string name, SvcRole isRoleAgent) {
        sdk_ = sk;
        upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, name, upgAppRespPtr_);
        delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
        delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadWriteMode);
        if (isRoleAgent == AGENT) {
            upgAgentHandlerPtr_ = make_shared<UpgAgentHandler>();
            upgMgrAgentRespPtr_ = make_shared<UpgMgrAgentRespReact>(upgAgentHandlerPtr_);
            delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
            delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
            upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(upgAgentHandlerPtr_);
            delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
        }
    }

    UpgSdk(delphi::SdkPtr sk, string name, SvcRole isRoleAgent, UpgAgentHandlerPtr uah) {
        sdk_ = sk;
        upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, name, upgAppRespPtr_);
        delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
        delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadWriteMode);
        if (isRoleAgent == AGENT) {
            upgMgrAgentRespPtr_ = make_shared<UpgMgrAgentRespReact>(uah);
            delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
            delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
            upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(uah);
            delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
        }
    }

    UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, SvcRole isRoleAgent) {
        sdk_ = sk;
        upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, uh, name, upgAppRespPtr_);
        delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
        delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadWriteMode);
        if (isRoleAgent == AGENT) {
            upgAgentHandlerPtr_ = make_shared<UpgAgentHandler>();
            upgMgrAgentRespPtr_ = make_shared<UpgMgrAgentRespReact>(upgAgentHandlerPtr_);
            delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
            delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
            upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(upgAgentHandlerPtr_);
            delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
        }
    }

    UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, SvcRole isRoleAgent, UpgAgentHandlerPtr uah) {
        sdk_ = sk;
        upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, uh, name, upgAppRespPtr_);
        delphi::objects::UpgStateReq::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgStateReq::Watch(sdk_, upgReqReactPtr_);
        delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadWriteMode);
        if (isRoleAgent == AGENT) {
            upgMgrAgentRespPtr_ = make_shared<UpgMgrAgentRespReact>(uah);
            delphi::objects::UpgResp::Mount(sdk_, delphi::ReadMode);
            delphi::objects::UpgResp::Watch(sdk_, upgMgrAgentRespPtr_);
            upgAppRespReactPtr_ = make_shared<UpgAppRespReact>(uah);
            delphi::objects::UpgAppResp::Watch(sdk_, upgAppRespReactPtr_);
        }
    }

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete(void);

    void SendAppRespSuccess(void);
    void SendAppRespFail(void);
};
typedef std::shared_ptr<UpgSdk> UpgSdkPtr;

} // namespace upgrade

#endif // __UPGRAGE_H__
