// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_MGR_H__
#define __UPGRADE_MGR_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"
#include "upgrade_resp.hpp"

namespace upgrade {

using namespace std;

// UpgradeMgr is the reactor for the UpgReq object
class UpgradeMgr : public delphi::objects::UpgReqReactor {
    delphi::SdkPtr                 sdk_;
    UpgMgrRespPtr                  upgMgrResp_;

    vector<string>                 appRespFailStrList_;
    unordered_map<string, bool>    appRegMap_;
    bool                           appRespFail_;
    bool                           upgAborted_;
    bool                           upgPassed_;

public:
    UpgradeMgr(delphi::SdkPtr sk) {
        sdk_ = sk;
        appRespFail_ = false;
        upgAborted_  = false;
        upgPassed_   = false;
        upgMgrResp_  = make_shared<UpgMgrResp>(sk);
        appRegMap_.clear();
    }

    // OnUpgReqCreate gets called when UpgReq object is created
    virtual delphi::error OnUpgReqCreate(delphi::objects::UpgReqPtr upgreq);

    // OnUpgReqDelete gets called when UpgReq object is deleted
    virtual delphi::error OnUpgReqDelete(delphi::objects::UpgReqPtr upgreq);

    // OnUpgReqCmd gets called when UpgReqCmd attribute changes
    virtual delphi::error OnUpgReqCmd(delphi::objects::UpgReqPtr upgreq);

    // createUpgStateReq creates an upgrade request status object
    delphi::error createUpgStateReq(uint32_t id, UpgReqStateType status);

    // findUpgStateReq finds the upgrade request status object
    delphi::objects::UpgStateReqPtr findUpgStateReq(uint32_t id);

    // findUpgReq finds the upgrade request object
    delphi::objects::UpgReqPtr findUpgReq(uint32_t id);

    UpgReqStateType GetNextState(void);
    bool IsRespTypeFail(UpgStateRespType type);

    string UpgReqStateTypeToStr(UpgReqStateType type);

    bool CanMoveStateMachine(void);

    delphi::error MoveStateMachine(UpgReqStateType type);

    UpgStateRespType GetFailRespType(UpgReqStateType);
    UpgStateRespType GetPassRespType(UpgReqStateType);

    delphi::error DeleteUpgMgrResp (void);

    void AppendAppRespFailStr (string str);
    bool GetAppRespFail(void);
    void SetAppRespFail(void);
    void ResetAppResp(void);

    delphi::error StartUpgrade(uint32_t key);
    delphi::error AbortUpgrade(uint32_t key);
    void RegNewApp(string name);
};
typedef std::shared_ptr<UpgradeMgr> UpgradeMgrPtr;

} // namespace upgrade

#endif // __UPGRADE_MGR_H__
