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
#include "nic/upgrade_manager/include/c/upgrade_state_machine.hpp"
 
namespace upgrade {

typedef enum {
    AGENT,
    NON_AGENT,
} SvcRole;

using namespace std;

class UpgSdk {
    delphi::SdkPtr           sdk_;
    string                   svcName_;
    SvcRole                  svcRole_;
    UpgReqReactorPtr         upgReqReactPtr_;
    UpgAppRespHdlrPtr        upgAppRespPtr_;
    UpgAppRespReactPtr       upgAppRespReactPtr_;
    UpgAgentHandlerPtr       upgAgentHandlerPtr_;
    UpgMgrAgentRespReactPtr  upgMgrAgentRespPtr_;

    delphi::error IsRoleAgent (SvcRole role, const char* errStr);
    delphi::objects::UpgReqPtr FindUpgReqSpec(void);
    delphi::objects::UpgReqPtr CreateUpgReqSpec(void);
    delphi::error UpdateUpgReqSpec(delphi::objects::UpgReqPtr req, UpgReqType type);

public:
    UpgSdk(delphi::SdkPtr sk, string name, SvcRole isRoleAgent);
    UpgSdk(delphi::SdkPtr sk, string name, SvcRole isRoleAgent, UpgAgentHandlerPtr uah);
    UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, SvcRole isRoleAgent);
    UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, SvcRole isRoleAgent, UpgAgentHandlerPtr uah);
    void SendAppRespSuccess(void);
    void SendAppRespFail(string str);

    delphi::error StartUpgrade(void);
    delphi::error AbortUpgrade(void);
    delphi::error GetUpgradeStatus(vector<string>& retStr);

    bool IsUpgradeInProgress(void);
};
typedef std::shared_ptr<UpgSdk> UpgSdkPtr;

} // namespace upgrade

#endif // __UPGRAGE_H__
