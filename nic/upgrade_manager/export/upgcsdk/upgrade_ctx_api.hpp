// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_CTX_API_H__
#define __UPGRAGE_CTX_API_H__

#include "nic/upgrade_manager/include/c/upgrade_ctx.hpp"

namespace upgrade {

using namespace std;

class UpgCtxApi {
public:
    UpgCtxApi() {}
    static delphi::error UpgCtxGetPreUpgTableVersion (UpgCtx &ctx, UpgMeta meta, string &version);
    static delphi::error UpgCtxGetPostUpgTableVersion (UpgCtx &ctx, UpgMeta meta, string &version);
    static bool UpgCtxIsUpgTypeDisruptive(UpgCtx &ctx);
    static bool UpgCtxIsUpgTypeNonDisruptive(UpgCtx &ctx);
private:
    static delphi::error UpgCtxGetUpgTableVersion (ImageInfo& imgInfo, UpgMeta meta, string &version);
};
typedef std::shared_ptr<UpgCtxApi> UpgCtxApiPtr;

} // namespace upgrade

#endif // __UPGRAGE_CTX_API_H__
