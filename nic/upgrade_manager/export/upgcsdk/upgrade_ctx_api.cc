// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_ctx_api.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

delphi::error UpgCtxApi::UpgCtxGetTableVersion(string name, int &version, unordered_map<string, TableMeta>& table) {
    if (name.empty()) {
        return delphi::error("Table name is not set");
    }
    auto elem = table.find(name);
    if (elem == table.end()) {
        return delphi::error("Table not found");
    }
    version = elem->second.version;
    return delphi::error::OK();
}

delphi::error UpgCtxApi::UpgCtxGetPreUpgTableVersion (UpgCtx &ctx, string name, int &version) {
    return UpgCtxGetTableVersion(name, version, ctx.preUpgTables);
}

delphi::error UpgCtxApi::UpgCtxGetPostUpgTableVersion (UpgCtx &ctx, string name, int &version) {
    return UpgCtxGetTableVersion(name, version, ctx.postUpgTables);
}

UpgType UpgCtxApi::UpgCtxGetUpgType(UpgCtx &ctx) {
    return ctx.upgType;
}

}
