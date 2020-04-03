// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_REQ_REACT_H__
#define __UPGRADE_REQ_REACT_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/upgrade.delphi.hpp"
#include "upgrade_resp.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

namespace upgrade {

using namespace std;

// UpgReqReact is the reactor for the UpgReq object
class UpgReqReact : public delphi::objects::UpgReqReactor {
    delphi::SdkPtr                         sdk_;
    UpgMgrRespPtr                          upgMgrResp_;
    sysmgr::ClientPtr                      sysMgr_;

    vector<string>                         appRespFailStrList_;
    unordered_map<string, bool>            appRegMap_;
    bool                                   appRespFail_;
    bool                                   upgAborted_;
    bool                                   upgPassed_;
    UpgReqType                             upgReqType_;
    delphi::objects::UpgradeMetricsPtr     upgMetric_;

    bool InvokePostStateHandler(UpgReqStateType reqType);
    bool InvokePreStateHandler(UpgReqStateType reqType);
public:
    UpgReqReact(delphi::SdkPtr sk, sysmgr::ClientPtr sysMgr) {
	upgReqType_ = InvalidCmd;
        sdk_ = sk;
        appRespFail_ = false;
        upgAborted_  = false;
        upgPassed_   = false;
        upgMgrResp_  = make_shared<UpgMgrResp>(sk);
        appRegMap_.clear();
        upgMetric_ = NULL;
	sysMgr_ = sysMgr;
    }

    // OnUpgReqCreate gets called when UpgReq object is created
    virtual delphi::error OnUpgReqCreate(delphi::objects::UpgReqPtr upgreq);

    // OnUpgReqDelete gets called when UpgReq object is deleted
    virtual delphi::error OnUpgReqDelete(delphi::objects::UpgReqPtr upgreq);

    // OnUpgReqCmd gets called when UpgReqCmd attribute changes
    virtual delphi::error OnUpgReqCmd(delphi::objects::UpgReqPtr upgreq);

    // createUpgStateReq creates an upgrade request status object
    delphi::error createUpgStateReq(UpgReqStateType status, UpgType type, string pkgName);

    // findUpgStateReq finds the upgrade request status object
    delphi::objects::UpgStateReqPtr findUpgStateReq();

    // findUpgReq finds the upgrade request object
    delphi::objects::UpgReqPtr findUpgReq();

    void SetStateMachine(delphi::objects::UpgReqPtr req);

    UpgReqStateType GetNextState(void);
    UpgReqStateType GetState(void);
    bool IsRespTypeFail(UpgStateRespType type);

    string UpgReqStateTypeToStr(UpgReqStateType type);

    bool CanMoveStateMachine(bool isOnMountReq);

    delphi::error MoveStateMachine(UpgReqStateType type);

    bool InvokePrePostStateHandlers(UpgReqStateType reqType);

    UpgStateRespType GetFailRespType(UpgReqStateType);
    UpgStateRespType GetPassRespType(UpgReqStateType);

    delphi::error DeleteUpgMgrResp (void);

    void AppendAppRespFailStr (string str);
    bool GetAppRespFail(void);
    void SetAppRespFail(void);
    void ResetAppResp(void);

    delphi::error StartUpgrade(delphi::objects::UpgReqPtr req);
    delphi::error AbortUpgrade();
    delphi::error IsUpgradePossible(delphi::objects::UpgReqPtr req);
    void RegNewApp(string name);
    void UnRegNewApp(string name);
    void CreateUpgradeMetrics(void);
    void DumpFirmwareVersion(void);
};
typedef std::shared_ptr<UpgReqReact> UpgReqReactPtr;

} // namespace upgrade

#endif // __UPGRADE_REQ_REACT_H__
