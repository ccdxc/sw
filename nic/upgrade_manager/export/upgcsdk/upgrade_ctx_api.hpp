// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_CTX_API_H__
#define __UPGRAGE_CTX_API_H__

#include "nic/upgrade_manager/include/c/upgrade_ctx.hpp"

namespace upgrade {

using namespace std;

class UpgCtxApi {
public:
    UpgCtxApi() {}
    static delphi::error UpgCtxGetPreUpgTableVersion (UpgCtx &ctx, string name, int &version);
    static delphi::error UpgCtxGetPostUpgTableVersion (UpgCtx &ctx, string name, int &version);
private:
    static delphi::error UpgCtxGetTableVersion(string name, int &version, unordered_map<string, TableMeta> &table);
};
typedef std::shared_ptr<UpgCtxApi> UpgCtxApiPtr;

} // namespace upgrade

#endif // __UPGRAGE_CTX_API_H__
