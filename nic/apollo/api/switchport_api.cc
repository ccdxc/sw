//
//// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
////
////----------------------------------------------------------------------------
/////
///// \file
///// This file implements TEP CRUD API
/////
////----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/api/switchport.hpp"
#include "nic/apollo/api/impl/oci_impl_state.hpp"
#include "gen/p4gen/apollo/include/p4pd.h"

sdk_ret_t
oci_switchport_create (oci_switchport_spec_t *switchport)
{
    api_ctxt_t api_ctxt;

    api_ctxt.api_params =
        api::api_params_alloc(api::OBJ_ID_SWITCHPORT, api::API_OP_CREATE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_CREATE;
        api_ctxt.obj_id = api::OBJ_ID_SWITCHPORT;
        api_ctxt.api_params->switchport_spec = *switchport;
        return api::g_api_engine.process_api(&api_ctxt);
    }
    return sdk::SDK_RET_OOM;
}

static inline sdk_ret_t
oci_switchport_spec_fill (switchport_entry *entry, oci_switchport_spec_t *spec)
{
    spec->switch_ip_addr = entry->ip_addr();
    memcpy(spec->switch_mac_addr, entry->mac_addr(), ETH_ADDR_LEN);
    spec->gateway_ip_addr = entry->gw_ip_addr();

    return sdk::SDK_RET_OK;
}

static inline sdk_ret_t
oci_switchport_status_fill (switchport_entry *entry,
                            oci_switchport_status_t *status)
{
    // TODO No status yet
    return sdk::SDK_RET_OK;
}

static void
oci_get_egress_drop_stats (oci_switchport_edrop_stats_t *egress_stats)
{
    // TODO No egress drop stats yet
    return;
}

static void
oci_get_ingress_drop_stats (oci_switchport_idrop_stats_t *ingress_stats)
{
    p4pd_error_t pd_err = P4PD_SUCCESS;
    uint64_t pkts = 0;
    p4i_drop_stats_swkey_t key = {0};
    p4i_drop_stats_swkey_mask_t key_mask = {0};
    p4i_drop_stats_actiondata_t data = {0};

    for (uint32_t i = P4I_DROP_REASON_MIN; i <= P4I_DROP_REASON_MAX; ++i) {
        pd_err =
            p4pd_entry_read(P4TBL_ID_P4I_DROP_STATS, i, &key, &key_mask, &data);
        if (pd_err == P4PD_SUCCESS) {
            memcpy(&pkts,
                   data.action_u.p4i_drop_stats_p4i_drop_stats.drop_stats_pkts,
                   sizeof(data.action_u.p4i_drop_stats_p4i_drop_stats
                              .drop_stats_pkts));
            ingress_stats
                ->drop_stats_pkts[key.control_metadata_p4i_drop_reason] = pkts;
        }
    }

    return;
}

static inline sdk_ret_t
oci_switchport_stats_fill (switchport_entry *entry,
                           oci_switchport_stats_t *stats)
{
    memset(stats, 0, sizeof(oci_switchport_stats_t));
    oci_get_ingress_drop_stats(&(stats->ingress_drop_stats));
    oci_get_egress_drop_stats(&(stats->egress_drop_stats));
    return sdk::SDK_RET_OK;
}

sdk_ret_t
oci_switchport_read (oci_switchport_info_t *info)
{
    sdk_ret_t rv;
    switchport_entry *entry;

    if (info == NULL) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    entry = switchport_db()->switchport_find();
    if (entry == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((rv = oci_switchport_spec_fill(entry, &(info->spec))) !=
        sdk::SDK_RET_OK) {
        return rv;
    }

    if ((rv = oci_switchport_status_fill(entry, &(info->status))) !=
        sdk::SDK_RET_OK) {
        return rv;
    }

    if ((rv = oci_switchport_stats_fill(entry, &(info->stats))) !=
        sdk::SDK_RET_OK) {
        return rv;
    }

    return sdk::SDK_RET_OK;
}

sdk_ret_t
oci_switchport_delete ()
{
    api_ctxt_t api_ctxt;

    api_ctxt.api_params =
        api::api_params_alloc(api::OBJ_ID_SWITCHPORT, api::API_OP_DELETE);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api::API_OP_DELETE;
        api_ctxt.obj_id = api::OBJ_ID_SWITCHPORT;
        return api::g_api_engine.process_api(&api_ctxt);
    }
    return sdk::SDK_RET_OOM;
}
