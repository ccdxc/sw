// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "upgrade_ctx_api.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using namespace std;

delphi::error UpgCtxApi::UpgCtxGetUpgTableVersion (ImageInfo& imgInfo, UpgMeta meta, string &version) {
    switch (meta) {
        case NICMGRVER:
            version = imgInfo.nicmgrVersion;
            return delphi::error::OK();
        case KERNELVER:
            version = imgInfo.kernelVersion;
            return delphi::error::OK();
    }
    return delphi::error("Meta not found");
}

delphi::error UpgCtxApi::UpgCtxGetPreUpgTableVersion (UpgCtx &ctx, UpgMeta meta, string &version) {
    return UpgCtxGetUpgTableVersion(ctx.preUpgMeta, meta, version);
}

delphi::error UpgCtxApi::UpgCtxGetPostUpgTableVersion (UpgCtx &ctx, UpgMeta meta, string &version) {
    return UpgCtxGetUpgTableVersion(ctx.postUpgMeta, meta, version);
}

bool UpgCtxApi::UpgCtxIsUpgTypeDisruptive(UpgCtx &ctx) {
    return ctx.upgType == UpgTypeDisruptive;
}

bool UpgCtxApi::UpgCtxIsUpgTypeNonDisruptive(UpgCtx &ctx) {
    return ctx.upgType == UpgTypeNonDisruptive;
}

}
