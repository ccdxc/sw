// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_SERVICE_H__
#define __UPGRADE_SERVICE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"
#include "upgrade_mgr.hpp"
#include "upgrade_resp_handlers.hpp"

namespace upgrade {

using namespace std;

// UpgradeService is the service object for upgrade manager 
class UpgradeService : public delphi::Service, public enable_shared_from_this<UpgradeService> {
private:
    UpgradeMgrPtr      upgMgr_;
    UpgAppRespHdlrPtr  upgAppRespHdlr_;
    delphi::SdkPtr     sdk_;
    string             svcName_;
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

} // namespace example

#endif // __UPGRADE_SERVICE_H__
