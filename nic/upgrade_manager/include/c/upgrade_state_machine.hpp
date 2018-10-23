// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_STATE_MACHINE_H__
#define __UPGRADE_STATE_MACHINE_H__

#include "gen/proto/upgrade.delphi.hpp"
#include "upgrade_ctx.hpp"
#include "upgrade_pre_state_handlers.hpp"
#include "upgrade_post_state_handlers.hpp"

namespace upgrade {

using namespace std;

typedef bool (UpgPreStateHandler::*UpgPreStateFunc)(UpgCtx &ctx);
typedef bool (UpgPostStateHandler::*UpgPostStateFunc)(UpgCtx &ctx);

typedef struct UpgStateMachine_ {
    UpgReqStateType             state;
    UpgReqStateType             stateNext;
    UpgStateRespType            statePassResp;
    UpgStateRespType            stateFailResp;
    string                      upgAppRespValToStrPass;
    string                      upgAppRespValToStrFail;
    string                      upgReqStateTypeToStr;
    string                      upgRespStateTypeToStrPass;
    string                      upgRespStateTypeToStrFail;
    UpgPreStateFunc             preStateFunc;
    UpgPostStateFunc            postStateFunc;
} UpgStateMachine;

extern UpgStateMachine NonDisruptiveUpgradeStateMachine[UpgStateTerminal];
extern UpgStateMachine DisruptiveUpgradeStateMachine[UpgStateTerminal];
extern UpgStateMachine CanUpgradeStateMachine[UpgStateTerminal]; 
extern UpgPreStateHandler* preStateHandlers;
extern UpgPostStateHandler* postStateHandlers;

void InitStateMachineVector(void);
string GetAppRespStrUtil(UpgStateRespType type, UpgType upgType);
string GetUpgAppRespValToStr(UpgStateRespType type, UpgType upgType);
bool UpgRespStatePassType(UpgStateRespType type);
}

#endif //__UPGRADE_STATE_MACHINE_H__
