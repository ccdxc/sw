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

static bool
lif_reset_walk_cb (void *entry, void *ctxt)
{
    sdk_ret_t *ret = (sdk_ret_t *)ctxt;
    api::impl::lif_impl *lif = (api::impl::lif_impl *)entry;
    asicpd_scheduler_lif_params_t lif_params;
    uint32_t offset, num_entries;

    PDS_TRACE_DEBUG("Lif walk lif %u", lif->id());
    if (lif->tx_sched_info(&offset, &num_entries)) {
        lif_params.tx_sched_table_offset = offset;
        lif_params.tx_sched_num_table_entries = num_entries;
        PDS_TRACE_DEBUG("TX scheduler map free for lif %u, offset %u, entries %u",
                        lif->id(), offset, num_entries);
        *ret = asicpd_tx_scheduler_map_free(&lif_params);
        if (*ret != SDK_RET_OK) {
            PDS_TRACE_ERR("TX schedule map free failed for lif %u", lif->id());
            return true;
        }
    }
    return false;
}

static sdk_ret_t
lif_all_reset (void)
{
    sdk_ret_t status = SDK_RET_OK;
    lif_db()->walk(lif_reset_walk_cb, &status);
    return status;
}

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
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_ready (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_link_down (upg_ev_params_t *params)
{
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Shutting down all uplink ports");
    ret = port_shutdown_all();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade Port shutdown failed, err %u", ret);
    }
    return ret;
}

static sdk_ret_t
upg_ev_quiesce (upg_ev_params_t *params)
{
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Resetting all lifs");
    // disable TX scheduler for all LIFs
    ret = lif_all_reset();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Lifs reset failed, err %u", ret);
    }
    return ret;
}

static sdk_ret_t
upg_ev_hostdev_reset (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_switchover (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_repeal (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_exit (upg_ev_params_t *params)
{
    return SDK_RET_OK;
}

sdk_ret_t
upg_graceful_init (pds_init_params_t *params)
{
    upg_ev_graceful_t ev_hdlr;

    // fill upgrade events for graceful
    memset(&ev_hdlr, 0, sizeof(ev_hdlr));
    strncpy(ev_hdlr.thread_name, "hal", sizeof(ev_hdlr.thread_name));
    ev_hdlr.compat_check_hdlr = upg_ev_compat_check;
    ev_hdlr.start_hdlr = upg_ev_start;
    ev_hdlr.linkdown_hdlr = upg_ev_link_down;
    ev_hdlr.hostdev_reset_hdlr = upg_ev_hostdev_reset;
    ev_hdlr.backup_hdlr = upg_ev_backup;
    ev_hdlr.ready_hdlr = upg_ev_ready;
    ev_hdlr.quiesce_hdlr = upg_ev_quiesce;
    ev_hdlr.switchover_hdlr = upg_ev_switchover;
    ev_hdlr.repeal_hdlr = upg_ev_repeal;
    ev_hdlr.exit_hdlr = upg_ev_exit;

    // register for upgrade events
    upg_ev_thread_hdlr_register(ev_hdlr);

    return SDK_RET_OK;
}

}    // namespace api
