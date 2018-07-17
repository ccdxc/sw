// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_CTX_H__
#define __UPGRADE_CTX_H__

#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

typedef struct UpgCtx_ {
    int                fromVer;
    int                toVer;
    UpgType            upgType;
} UpgCtx;

}

#endif //__UPGRADE_CTX_H__
