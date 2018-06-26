// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_STATE_MACHINE_H__
#define __UPGRADE_STATE_MACHINE_H__

#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"
#include "upgrade_pre_state_handlers.hpp"

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

typedef void (UpgPreStateHandler::*UpgStateFunc)(void);

typedef struct UpgStateMachine_ {
    UpgReqStateType             state;
    UpgReqStateType             stateNext;
    UpgRespStateType            statePassResp;
    UpgRespStateType            stateFailResp;
    string                      upgAppRespValToStrPass;
    string                      upgAppRespValToStrFail;
    string                      upgReqStateTypeToStr;
    string                      upgRespStateTypeToStrPass;
    string                      upgRespStateTypeToStrFail;
    UpgStateFunc                preStateFunc;
} UpgStateMachine;

extern UpgStateMachine StateMachine[UpgStateTerminal]; 
extern UpgPreStateHandler* preStateHandlers;
void InitStateMachineVector(void);
}

#endif //__UPGRADE_STATE_MACHINE_H__
