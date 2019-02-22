//
//// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
////
////----------------------------------------------------------------------------
/////
///// \file
///// This file implements TEP CRUD API
/////
////----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/api/oci_state.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/tep_impl.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "gen/p4gen/apollo/include/p4pd.h"

static inline sdk_ret_t
oci_tep_api_handle (api_op_t api_op, oci_tep_key_t *key, oci_tep_spec_t *spec)
{
    api_ctxt_t api_ctxt;

    api_ctxt.api_params = api::api_params_alloc(api::OBJ_ID_TEP, api_op);
    if (likely(api_ctxt.api_params != NULL)) {
        api_ctxt.api_op = api_op;
        api_ctxt.obj_id = api::OBJ_ID_TEP;
        if (api_op == api::API_OP_DELETE) {
            api_ctxt.api_params->tep_key = *key;
        } else {
            api_ctxt.api_params->tep_spec = *spec;
        }

        return (api::g_api_engine.process_api(&api_ctxt));
    }

    return sdk::SDK_RET_OOM;
}

sdk_ret_t
oci_tep_create (oci_tep_spec_t *spec)
{
    return (oci_tep_api_handle(api::API_OP_CREATE, NULL, spec));
}

static inline sdk_ret_t
oci_tep_stats_fill (tep_entry *entry, oci_tep_stats_t *stats)
{
    // TODO - TEP does NOT have stats.
    return sdk::SDK_RET_OK;
}

static inline sdk_ret_t
oci_tep_status_fill (tep_entry *entry, tep_tx_actiondata_t *tep_tx_data,
                     oci_tep_status_t *status)
{
    status->hw_id = ((api::impl::tep_impl *)(entry->impl()))->hw_id();
    status->nh_id = ((api::impl::tep_impl *)(entry->impl()))->nh_id();

    switch (tep_tx_data->action_id) {
    case TEP_TX_GRE_TEP_TX_ID:
        memcpy(status->dmac, tep_tx_data->action_u.tep_tx_gre_tep_tx.dmac,
               ETH_ADDR_LEN);
        break;
    case TEP_TX_UDP_TEP_TX_ID:
        memcpy(status->dmac, tep_tx_data->action_u.tep_tx_udp_tep_tx.dmac,
               ETH_ADDR_LEN);
        break;
    default:
        return sdk::SDK_RET_ERR;
        break;
    }

    return sdk::SDK_RET_OK;
}

static inline sdk_ret_t
oci_tep_spec_fill (nexthop_tx_actiondata_t *nh_tx_data,
                   tep_tx_actiondata_t *tep_tx_data, oci_tep_spec_t *spec)
{
    switch (nh_tx_data->action_u.nexthop_tx_nexthop_info.encap_type) {
    case GW_ENCAP:
        spec->type = OCI_ENCAP_TYPE_GW_ENCAP;
        spec->key.ip_addr = tep_tx_data->action_u.tep_tx_udp_tep_tx.dipo;
        break;
    case VNIC_ENCAP:
        spec->type = OCI_ENCAP_TYPE_VNIC;
        spec->key.ip_addr = tep_tx_data->action_u.tep_tx_udp_tep_tx.dipo;
        break;
    default:
        return sdk::SDK_RET_ERR;
        break;
    }

    return sdk::SDK_RET_OK;
}

static inline sdk_ret_t
oci_tep_hw_read (tep_entry *entry, nexthop_tx_actiondata_t *nh_tx_data,
                 tep_tx_actiondata_t *tep_tx_data)
{
    uint16_t nh_id, hw_id;

    // retrieve next_hop id and tep hw_id from sw
    nh_id = ((api::impl::tep_impl *)(entry->impl()))->nh_id();
    hw_id = ((api::impl::tep_impl *)(entry->impl()))->hw_id();

    memset(nh_tx_data, 0, sizeof(nexthop_tx_actiondata_t));
    memset(tep_tx_data, 0, sizeof(tep_tx_actiondata_t));

    // TODO p4pd_entry_read does NOT seem to return proper data?
    if (p4pd_entry_read(P4TBL_ID_NEXTHOP_TX, nh_id, NULL, NULL, nh_tx_data) !=
        P4PD_SUCCESS) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    if (p4pd_entry_read(P4TBL_ID_TEP_TX, hw_id, NULL, NULL, tep_tx_data) !=
        P4PD_SUCCESS) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    return sdk::SDK_RET_OK;
}

sdk_ret_t
oci_tep_read (oci_tep_key_t *key, oci_tep_info_t *info)
{
    sdk_ret_t rv;
    tep_entry *entry;
    nexthop_tx_actiondata_t nh_tx_data;
    tep_tx_actiondata_t tep_tx_data;

    if ((key == NULL) || (info == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    entry = tep_db()->tep_find(key);
    if (entry == NULL) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((rv = oci_tep_hw_read(entry, &nh_tx_data, &tep_tx_data)) !=
        sdk::SDK_RET_OK) {
        return rv;
    }

    if ((rv = oci_tep_spec_fill(&nh_tx_data, &tep_tx_data, &(info->spec))) !=
        sdk::SDK_RET_OK) {
        return rv;
    }

    if ((rv = oci_tep_status_fill(entry, &tep_tx_data, &(info->status))) !=
        sdk::SDK_RET_OK) {
        return rv;
    }

    if ((rv = oci_tep_stats_fill(entry, &(info->stats))) != sdk::SDK_RET_OK) {
        return rv;
    }

    return rv;
}

sdk_ret_t
oci_tep_update (oci_tep_spec_t *spec)
{
    return (oci_tep_api_handle(api::API_OP_UPDATE, NULL, spec));
}

sdk_ret_t
oci_tep_delete (oci_tep_key_t *key)
{
    return (oci_tep_api_handle(api::API_OP_DELETE, key, NULL));
}
