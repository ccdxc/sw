/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <cinttypes>

#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "platform/src/app/pciemgrd/src/pciemgrd_impl.hpp"

#include "delphic.h"

HdlrResp
PciemgrSvcHandler::CompatCheckHandler(UpgCtx& upgCtx)
{
    return HdlrResp(SUCCESS, "");
}

HdlrResp
PciemgrSvcHandler::SaveStateHandler(UpgCtx& upgCtx)
{
    if (upgrade_state_save() < 0) {
        return HdlrResp(FAIL, "pciemgr upgrade_state_save failed");
    } else {
        return HdlrResp(SUCCESS, "");
    }
}
