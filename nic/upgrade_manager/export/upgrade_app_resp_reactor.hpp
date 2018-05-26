// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_APP_RESP_REACTOR_H__
#define __UPGRADE_APP_RESP_REACTOR_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"
#include "upgrade_agent_handler.hpp"
#include "upgrade_handler.hpp"

namespace upgrade {

using namespace std;

//UpgAppRespReact class is used to create the object for 
class UpgAppRespReact : public delphi::objects::UpgAppRespReactor {
    UpgAgentHandlerPtr upgAgentHandler_;
public:
    UpgAppRespReact() {}

    UpgAppRespReact(UpgAgentHandlerPtr ptr) {
        upgAgentHandler_ = ptr;
    }

    // OnUpgAppRespCreate gets called when UpgAppResp object is created
    virtual delphi::error OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp);

    // OnUpgAppRespVal gets called when UpgAppRespVal attribute changes
    virtual delphi::error OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp);

    string GetAppRespStr(delphi::objects::UpgAppRespPtr resp);
    void InvokeAgentHandler(delphi::objects::UpgAppRespPtr resp);

    void SetAppRespSuccess(HdlrResp &resp);
    void SetAppRespFail(HdlrResp &resp, string str);

};
typedef std::shared_ptr<UpgAppRespReact> UpgAppRespReactPtr;

} // namespace upgrade

#endif // __UPGRADE_APP_RESP_REACTOR_H__
