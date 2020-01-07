//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_UPG_HPP__
#define __NICMGR_UPG_HPP__

#include "nic/sdk/lib/shmmgr/shmmgr.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade_ctx_api.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade_handler.hpp"
#include "nic/hal/iris/upgrade/upg_ipc.hpp"

using namespace upgrade;

using upgrade::HdlrResp;
using upgrade::UpgCtx;
using upgrade::UpgHandler;

namespace nicmgr {


class nicmgr_upg_hndlr : public UpgHandler {
public:
    nicmgr_upg_hndlr();
    HdlrResp CompatCheckHandler(UpgCtx& upgCtx);
    HdlrResp ProcessQuiesceHandler(UpgCtx& upgCtx);
    HdlrResp LinkDownHandler(UpgCtx& upgCtx);
    HdlrResp PostRestartHandler(UpgCtx& upgCtx);
    HdlrResp HostDownHandler(UpgCtx& upgCtx);
    HdlrResp PostHostDownHandler(UpgCtx& upgCtx);
    HdlrResp SaveStateHandler(UpgCtx& upgCtx);
    HdlrResp HostUpHandler(UpgCtx& upgCtx);
    HdlrResp LinkUpHandler(UpgCtx& upgCtx);
    HdlrResp PostLinkUpHandler(UpgCtx& upgCtx);
    void ResetState(UpgCtx& upgCtx);
    void SuccessHandler(UpgCtx& upgCtx);
    HdlrResp FailedHandler(UpgCtx& upgCtx);
    void AbortHandler(UpgCtx& upgCtx);
    static void upg_timer_func(void *obj);
    static void upg_ipc_handler_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctxt);
    sdk_ret_t upg_shm_alloc(const char *name, uint32_t size, bool create);
    void upg_restore_states(void);
private:
    void upg_ipc_handler_(sdk::ipc::ipc_msg_ptr msg, const void *ctxt);
    shmmgr     *shm_mmgr_;
    char       *mem_;              ///< mapped memory start
    uint32_t   obj_mem_offset_;    ///< object memory offset
    uint32_t   obj_mem_size_;      ///< max object memory size
};

void nicmgr_upg_init(void);

} // namespace nicmgr
#endif    // __NICMGR_UPG_HPP__
