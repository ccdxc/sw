// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __CODE_UPGRADE_H__
#define __CODE_UPGRADE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

// UpgradeMgr is the reactor for the UpgradeRequestSpec object
class UpgradeMgr : public delphi::objects::UpgradeRequestSpecReactor {
    delphi::SdkPtr sdk_;
public:
    UpgradeMgr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnUpgradeRequestSpecCreate gets called when UpgradeRequestSpec object is created
    virtual delphi::error OnUpgradeRequestSpecCreate(delphi::objects::UpgradeRequestSpecPtr upgreq);

    // OnUpgradeRequestSpecDelete gets called when UpgradeRequestSpec object is deleted
    virtual delphi::error OnUpgradeRequestSpecDelete(delphi::objects::UpgradeRequestSpecPtr upgreq);

    // OnUpgReqAction gets called when UpgReqAction attribute changes
    virtual delphi::error OnUpgReqAction(delphi::objects::UpgradeRequestSpecPtr upgreq);

    // createUpgReqStatus creates an upgrade request status object
    delphi::error createUpgReqStatus(uint32_t id, upgrade::UpgReqState status);

    // findIntfStatus finds the interface status object
    delphi::objects::UpgReqStatusPtr findUpgReqStatus(uint32_t id);
};
typedef std::shared_ptr<UpgradeMgr> UpgradeMgrPtr;

// UpgradeService is the service object for upgrade manager 
class UpgradeService : public delphi::Service, public enable_shared_from_this<UpgradeService> {
private:
    UpgradeMgrPtr      upgMgr_;
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

#endif // __CODE_UPGRADE_H__
