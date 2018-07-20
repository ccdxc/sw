// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_CTX_H__
#define __UPGRADE_CTX_H__

#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

typedef struct TableMeta_ {
    int       version;
    string    name;
} TableMeta;

typedef struct UpgCtx_ {
    unordered_map<string, TableMeta>    preUpgTables;
    unordered_map<string, TableMeta>    postUpgTables;
    UpgType                             upgType;
} UpgCtx;

}

#endif //__UPGRADE_CTX_H__
