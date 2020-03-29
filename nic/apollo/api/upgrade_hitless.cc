//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/pd/scheduler.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/api/impl/lif_impl.hpp"

namespace api {

static sdk_ret_t
upg_ev_compat_check (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_start (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_backup (upg_ev_params_t *params)
{
    sdk_ret_t ret = impl_base::pipeline_impl()->upgrade_backup();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade pipeline backup failed, err %u", ret);
    }
    return ret;
}

static sdk_ret_t
upg_ev_ready (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_quiesce (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_switchover (upg_ev_params_t *params)
{
    sdk_ret_t ret = impl_base::pipeline_impl()->upgrade_switchover();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade pipeline switchover failed, err %u", ret);
    }
    return ret;
}

static sdk_ret_t
upg_ev_repeal (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_exit (upg_ev_params_t *params)
{
    return pds_teardown();
}

sdk_ret_t
upg_hitless_init (pds_init_params_t *params)
{
    upg_ev_hitless_t ev_hdlr;

    // fill upgrade events for graceful
    memset(&ev_hdlr, 0, sizeof(ev_hdlr));
    // thread id is used for just identification here
    // no ipc to the api thread from caller thread as grpc is already
    // disabled, not expecting any conflicts in configs
    ev_hdlr.thread_id = core::PDS_THREAD_ID_API;
    ev_hdlr.compat_check = upg_ev_compat_check;
    ev_hdlr.start = upg_ev_start;
    ev_hdlr.backup = upg_ev_backup;
    ev_hdlr.ready = upg_ev_ready;
    ev_hdlr.quiesce = upg_ev_quiesce;
    ev_hdlr.switchover = upg_ev_switchover;
    ev_hdlr.repeal = upg_ev_repeal;
    ev_hdlr.exit  = upg_ev_exit;

    // register for upgrade events
    upg_ev_thread_hdlr_register(ev_hdlr);

    return SDK_RET_OK;
}

}    // namespace api
