// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_STATE_MACHINE_H__
#define __UPGRADE_STATE_MACHINE_H__

#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"
#include "upgrade_pre_state_handlers.hpp"
#include "upgrade_post_state_handlers.hpp"

namespace upgrade {

using namespace std;

typedef enum {
    SUCCESS,
    FAIL,
    INPROGRESS
} HdlrRespCode;

typedef struct HdlrResp_ {
    HdlrRespCode     resp;
    string           errStr;
} HdlrResp;

typedef struct UpgCtx_ {
    string                fromVer;
    string                toVer;
    UpgType               upgType;
} UpgCtx;

typedef bool (UpgPreStateHandler::*UpgPreStateFunc)(void);
typedef bool (UpgPostStateHandler::*UpgPostStateFunc)(void);

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

extern UpgStateMachine StateMachine[UpgStateTerminal]; 
extern UpgPreStateHandler* preStateHandlers;
extern UpgPostStateHandler* postStateHandlers;

void InitStateMachineVector(void);
string GetAppRespStrUtil(UpgStateRespType type);
string GetUpgAppRespValToStr(UpgStateRespType type);
bool UpgRespStatePassType(UpgStateRespType type);
}

#endif //__UPGRADE_STATE_MACHINE_H__
