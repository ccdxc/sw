/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <cinttypes>

#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"

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

void
PciemgrSvcHandler::SuccessHandler(UpgCtx& upgCtx)
{
    upgrade_complete();
}

HdlrResp
PciemgrSvcHandler::FailedHandler(UpgCtx& upgCtx)
{
    //
    // If we've gone passed the compat check stage, then we
    // need to roll back to previous saved state.  A rollback
    // is very much like an upgrade to saved state, but we
    // need to mark that we are in rollback so we can remove
    // the saved state after we restart.  We will not get any
    // upgrade events after the rollback so the rollback mark
    // tells us we can remove the saved state once we've restarted
    // and used the restored state.
    //
    // If we failed *during* the compat check stage then we
    // abort the upgrade and remove any saved state because it
    // is no longer needed.  Our current process will just continue
    // to run with the current running state.
    //
    if (UpgCtxApi::UpgCtxGetUpgState(upgCtx) != CompatCheck) {
        upgrade_state_save();
        upgrade_rollback_begin();
    } else {
        upgrade_failed();
    }
    return HdlrResp(SUCCESS, "");
}
