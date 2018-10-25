// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_SERVICE_H__
#define __UPGRADE_SERVICE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/upgrade.delphi.hpp"
#include "upgrade_req_react.hpp"
#include "upgrade_app_resp_reactor.hpp"
#include "upgrade_app_reg_reactor.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

namespace upgrade {

using namespace std;

// UpgradeService is the service object for upgrade manager 
class UpgradeService : public delphi::Service, public enable_shared_from_this<UpgradeService> {
private:
    UpgReqReactPtr          upgMgr_;
    UpgAppRespReactPtr      upgAppRespHdlr_;
    UpgAppRegReactPtr       upgAppRegHdlr_;
    delphi::SdkPtr          sdk_;
    string                  svcName_;
    sysmgr::Client          sysMgr_;
public:
    // UpgradeService constructor
    UpgradeService(delphi::SdkPtr sk);
    UpgradeService(delphi::SdkPtr sk, string name);

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete();

    // createUpgReqSpec creates a dummy Upgrade Request
    void createUpgReqSpec();

    // override service name method
    virtual string Name() { return svcName_; }

    // timer for creating a dummy object
    ev::timer          createTimer;
    void createTimerHandler(ev::timer &watcher, int revents);
};
typedef std::shared_ptr<UpgradeService> UpgradeServicePtr;

} // namespace upgrade

#endif // __UPGRADE_SERVICE_H__
