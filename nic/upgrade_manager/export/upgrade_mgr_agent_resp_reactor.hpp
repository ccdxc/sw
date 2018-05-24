// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_MGR_AGENT_RESP_REACTOR_H__
#define __UPGRAGE_MGR_AGENT_RESP_REACTOR_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"
#include "upgrade_app_resp_reactor.hpp"

namespace upgrade {

using namespace std;

class UpgMgrAgentRespReact : public delphi::objects::UpgRespReactor {
    UpgAgentHandlerPtr upgAgentHandler_;
public:
    UpgMgrAgentRespReact() {}

    UpgMgrAgentRespReact(UpgAgentHandlerPtr ptr) {
        upgAgentHandler_ = ptr;
    }

    // OnUpgRespCreate gets called when UpgResp object is created
    virtual delphi::error OnUpgRespCreate(delphi::objects::UpgRespPtr resp);

    // OnUpgRespVal gets called when UpgRespVal attribute changes
    virtual delphi::error OnUpgRespVal(delphi::objects::UpgRespPtr resp);

    string GetRespStr(delphi::objects::UpgRespPtr resp);
    void InvokeAgentHandler(delphi::objects::UpgRespPtr resp);
};
typedef std::shared_ptr<UpgMgrAgentRespReact> UpgMgrAgentRespReactPtr;

} //namespace upgrade
#endif // __UPGRAGE_MGR_AGENT_RESP_REACTOR_H__
