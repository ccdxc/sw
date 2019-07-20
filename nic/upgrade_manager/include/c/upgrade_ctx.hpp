// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_CTX_H__
#define __UPGRADE_CTX_H__

#include "gen/proto/upgrade.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

namespace upgrade {

using namespace std;

typedef struct ImageInfo_ {
   string buildDate; 
   string buildUser; 
   string baseVersion; 
   string softwareVersion; 
   string nicmgrVersion; 
   string kernelVersion; 
   string pcieVersion; 
   string devConfVersion; 
} ImageInfo;

typedef struct UpgCtx_ {
    ImageInfo                  preUpgMeta;
    ImageInfo                  postUpgMeta;
    UpgType                    upgType;
    sysmgr::ClientPtr          sysMgr;
    string                     firmwarePkgName;
    bool                       postRestart;
    bool                       upgFailed;
    bool                       upgPostCompatCheck;
    UpgReqStateType            prevExecState;
    bool                       haltStateMachine;
    string                     compatCheckFailureReason;
} UpgCtx;

typedef enum {
    NICMGRVER,
    KERNELVER,
    PCIEVER,
    DEVCONFVER
} UpgMeta;

}

#endif //__UPGRADE_CTX_H__
