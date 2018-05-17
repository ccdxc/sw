// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __CODE_UPGRADE_H__
#define __CODE_UPGRADE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

// UpgradeMgr is the reactor for the UpgReq object
class UpgradeMgr : public delphi::objects::UpgReqReactor {
    delphi::SdkPtr sdk_;
public:
    UpgradeMgr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnUpgReqCreate gets called when UpgReq object is created
    virtual delphi::error OnUpgReqCreate(delphi::objects::UpgReqPtr upgreq);

    // OnUpgReqDelete gets called when UpgReq object is deleted
    virtual delphi::error OnUpgReqDelete(delphi::objects::UpgReqPtr upgreq);

    // OnUpgReqCmd gets called when UpgReqCmd attribute changes
    virtual delphi::error OnUpgReqCmd(delphi::objects::UpgReqPtr upgreq);

    // createUpgReqStatus creates an upgrade request status object
    delphi::error createUpgReqStatus(uint32_t id, upgrade::UpgReqStateType status);

    // findUpgReqStatus finds the upgrade request status object
    delphi::objects::UpgReqStatusPtr findUpgReqStatus(uint32_t id);

    UpgReqStateType GetNextState(void);

    bool CanMoveStateMachine(void);

    delphi::error MoveStateMachine(UpgReqStateType type);

    UpgRespStateType GetFailRespType(UpgReqStateType);
    UpgRespStateType GetPassRespType(UpgReqStateType);
};
typedef std::shared_ptr<UpgradeMgr> UpgradeMgrPtr;

class UpgAppRespHdlr : public delphi::objects::UpgAppRespReactor {
    delphi::SdkPtr     sdk_;
    UpgradeMgrPtr      upgMgr_;
public:
    UpgAppRespHdlr(delphi::SdkPtr sk, UpgradeMgrPtr upgmgr) {
        this->sdk_ = sk;
        this->upgMgr_ = upgmgr;
    }

    // OnUpgAppRespCreate gets called when UpgAppResp object is created
    virtual delphi::error OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp);

    // OnUpgAppRespVal gets called when UpgAppRespVal attribute changes
    virtual delphi::error OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp);
};
typedef std::shared_ptr<UpgAppRespHdlr> UpgAppRespHdlrPtr;

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

#endif // __CODE_UPGRADE_H__
