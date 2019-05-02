//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "gen/proto/nicmgr/nicmgr.pb.h"

#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

#include "upgrade.hpp"

using namespace upgrade;
using namespace nicmgr;

extern DeviceManager *devmgr;
extern char* nicmgr_upgrade_state_file;

evutil_timer dev_reset_timer;
evutil_timer dev_quiesce_timer;

namespace nicmgr {

static int
writefile(const char *file, const void *buf, const size_t bufsz)
{
    FILE *fp = fopen(file, "w");
    if (fp == NULL) {
        goto error_out;
    }
    if (fwrite(buf, bufsz, 1, fp) != 1) {
        goto error_out;
    }
    if (fclose(fp) == EOF) {
        fp = NULL;
        goto error_out;
    }
    return bufsz;

 error_out:
    if (fp) (void)fclose(fp);
    (void)unlink(file);
    return -1;
}


// Constructor method
nicmgr_upg_hndlr::nicmgr_upg_hndlr()
{
}

// Perform compat check
HdlrResp
nicmgr_upg_hndlr::CompatCheckHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::ProcessQuiesceHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    return resp;
}

void
nicmgr_upg_hndlr::check_device_quiesced(void *obj)
{
    //Check whether all device are in reset or not
    if (devmgr->GetUpgradeState() == DEVICES_QUIESCED_STATE) {
        NIC_FUNC_DEBUG("All devices are in Quiesced state");
        g_nicmgr_svc->upgsdk()->SendAppRespSuccess();
        evutil_timer_stop(&dev_quiesce_timer);
    }

    return;
}

// Bring link-down
HdlrResp
nicmgr_upg_hndlr::LinkDownHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=INPROGRESS, .errStr=""};

    if (devmgr->HandleUpgradeEvent(UPG_EVENT_QUIESCE)) {
        NIC_FUNC_DEBUG("UPG_EVENT_QUIESCE event Failed! Cannot continue upgrade FSM");
        resp.resp = FAIL;
        return resp;
    }

    NIC_FUNC_DEBUG("Starting timer thread to check whether all devices are quiesced or not ...");
    evutil_timer_start(&dev_quiesce_timer, nicmgr_upg_hndlr::check_device_quiesced, this, 0.0, 0.1);

    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::PostHostDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    return resp;
}

// Post-binary restart handling
HdlrResp
nicmgr_upg_hndlr::PostRestartHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    //TODO: Restore state from Delphi
    return resp;
}

// Bring link-up
HdlrResp
nicmgr_upg_hndlr::LinkUpHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    return resp;
}

// Handle upgrade success
void
nicmgr_upg_hndlr::SuccessHandler(UpgCtx& upgCtx)
{
}

// Handle upgrade failure
HdlrResp
nicmgr_upg_hndlr::FailedHandler(UpgCtx& upgCtx)
{
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};
    return resp;
}

// Handle upgrade abort
void
nicmgr_upg_hndlr::AbortHandler(UpgCtx& upgCtx)
{
}

HdlrResp 
nicmgr_upg_hndlr::SaveStateHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    //TODO: Save state to Delphi

    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::PostLinkUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    return resp;
}

void
nicmgr_upg_hndlr::check_device_reset(void *obj)
{
    //Check whether all device are in reset or not
    if (devmgr->GetUpgradeState() == DEVICES_RESET_STATE) {
        NIC_FUNC_DEBUG("All devices are in reset state");
        writefile(nicmgr_upgrade_state_file, "in progress", 12);
        g_nicmgr_svc->upgsdk()->SendAppRespSuccess();
        evutil_timer_stop(&dev_reset_timer);
    }

    return;
}

HdlrResp 
nicmgr_upg_hndlr::HostDownHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=INPROGRESS, .errStr=""};

    if (devmgr->HandleUpgradeEvent(UPG_EVENT_DEVICE_RESET)) {
        NIC_FUNC_DEBUG("UPG_EVENT_DEVICE_RESET event Failed! Cannot continue upgrade FSM");
        resp.resp = FAIL;
        return resp;
    }

    NIC_FUNC_DEBUG("Starting timer thread to check whether all devices are in reset or not ...");
    evutil_timer_start(&dev_reset_timer, nicmgr_upg_hndlr::check_device_reset, this, 0.0, 0.1);

    return resp;
}

HdlrResp 
nicmgr_upg_hndlr::HostUpHandler(UpgCtx& upgCtx) {
    HdlrResp resp = {.resp=SUCCESS, .errStr=""};

    return resp;
}


} // namespace nicmgr
