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

delphi::error UpgCtxApi::UpgCtxGetComponentVersion(string name, int &version, unordered_map<string, ComponentMeta>& comp) {
    if (name.empty()) {
        return delphi::error("Component name is not set");
    }
    auto elem = comp.find(name);
    if (elem == comp.end()) {
        return delphi::error("Component not found");
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

delphi::error UpgCtxApi::UpgCtxGetPreUpgComponentVersion (UpgCtx &ctx, string name, int &version) {
    return UpgCtxGetComponentVersion(name, version, ctx.preUpgComps);
}

delphi::error UpgCtxApi::UpgCtxGetPostUpgComponentVersion (UpgCtx &ctx, string name, int &version) {
    return UpgCtxGetComponentVersion(name, version, ctx.postUpgComps);
}

bool UpgCtxApi::UpgCtxIsUpgTypeDisruptive(UpgCtx &ctx) {
    return ctx.upgType == UpgTypeDisruptive;
}

bool UpgCtxApi::UpgCtxIsUpgTypeNonDisruptive(UpgCtx &ctx) {
    return ctx.upgType == UpgTypeNonDisruptive;
}

}
