//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of VNIC
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apulu/route_impl.hpp"
#include "nic/apollo/api/impl/apulu/security_policy_impl.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VNIC_IMPL - VNIC entry datapath implementation
/// \ingroup PDS_VNIC
/// @{

vnic_impl *
vnic_impl::factory(pds_vnic_spec_t *spec) {
    vnic_impl *impl;

    // TODO: move to slab later
    impl = (vnic_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VNIC_IMPL,
                                  sizeof(vnic_impl));
    new (impl) vnic_impl();
    return impl;
}

void
vnic_impl::destroy(vnic_impl *impl) {
    impl->~vnic_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_VNIC_IMPL, impl);
}

sdk_ret_t
vnic_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    sdk_table_api_params_t tparams = { 0 };
    local_mapping_sw_key_t local_mapping_key;
    pds_vnic_spec_t *spec = &obj_ctxt->api_params->vnic_spec;

    // allocate hw id for this vnic
    if (vnic_impl_db()->vnic_idxr()->alloc(&idx) !=
            sdk::lib::indexer::SUCCESS) {
        PDS_TRACE_ERR("Failed to allocate hw id for vnic %u", spec->key.id);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    hw_id_ = idx;

    // TODO: reserve an entry in LOCAL_MAPPING table
     PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key,
                                    NULL, NULL, 0,
                                    sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_MAPPING"
                      "table for vnic %u, err %u", spec->key.id, ret);
        return ret;
    }
    local_mapping_handle_ = tparams.handle;

    // TODO: reserve an entry in MAPPING table
    ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING "
                      "table for vnic %u, err %u", spec->key.id, ret);
        return ret;
    }
    mapping_handle_ = tparams.handle;
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams;

    if (hw_id_ != 0xFFFF) {
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
    }

    if (local_mapping_handle_.valid()) {
        memset(&tparams, 0, sizeof(tparams));
        tparams.handle = local_mapping_handle_;
        mapping_impl_db()->local_mapping_tbl()->release(&tparams);
    }

    if (mapping_handle_.valid()) {
        memset(&tparams, 0, sizeof(tparams));
        tparams.handle = mapping_handle_;
        mapping_impl_db()->local_mapping_tbl()->release(&tparams);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams;

    if (hw_id_ != 0xFFFF) {
        vnic_impl_db()->vnic_idxr()->free(hw_id_);
    }
    memset(&tparams, 0, sizeof(tparams));
    // TODO:
    return SDK_RET_OK;
}

// TODO: undo stuff if something goes wrong here !!
sdk_ret_t
vnic_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t                  ret;
    pds_vnic_spec_t            *spec;
    p4pd_error_t               p4pd_ret;
    subnet_entry               *subnet;
    //local_mapping_actiondata_t local_mapping_data = { 0 };
    //mapping_actiondata_t       mapping_data = { 0 };
    //vnic_rx_stats_actiondata_t vnic_rx_stats_data = { 0 };
    //vnic_tx_stats_actiondata_t vnic_tx_stats_data = { 0 };

    spec = &obj_ctxt->api_params->vnic_spec;
    subnet = subnet_db()->find(&spec->subnet);
    if (subnet == NULL) {
        PDS_TRACE_ERR("Unable to find subnet : %u, vpc : %u",
                      spec->subnet.id, spec->vpc.id);
        return sdk::SDK_RET_INVALID_ARG;
    }

    // TODO: program tables

#if 0
    // initialize rx stats tables for this vnic
    vnic_rx_stats_data.action_id = VNIC_RX_STATS_VNIC_RX_STATS_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VNIC_RX_STATS,
                                       hw_id_, NULL, NULL,
                                       &vnic_rx_stats_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
#endif
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    sdk_ret_t ret;
    pds_subnet_key_t subnet_key;
    vnic_entry *vnic = (vnic_entry *)api_obj;
    subnet_entry *subnet;

    subnet_key = vnic->subnet();
    subnet = (subnet_entry *)api_base::find_obj(OBJ_ID_SUBNET, &subnet_key);

    // TODO
    return ret;
}

sdk_ret_t
vnic_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vnic_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vnic_impl::activate_vnic_create_(pds_epoch_t epoch, vnic_entry *vnic,
                                 pds_vnic_spec_t *spec) {
    sdk_ret_t                ret;
    pds_vpc_key_t            vpc_key;
    vpc_entry                *vpc;
    pds_subnet_key_t         subnet_key;
    subnet_entry             *subnet;
    pds_route_table_key_t    route_table_key;
    route_table              *v4_route_table, *v6_route_table;
    pds_policy_key_t         policy_key;
    policy                   *ing_v4_policy, *ing_v6_policy;
    policy                   *egr_v4_policy, *egr_v6_policy;

    subnet_key = vnic->subnet();
    subnet = subnet_db()->find(&subnet_key);
    if (unlikely(subnet == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    vpc_key = subnet->vpc();
    vpc = vpc_db()->find(&vpc_key);
    if (unlikely(vpc == NULL)) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    route_table_key = subnet->v4_route_table();
    v4_route_table = route_table_db()->find(&route_table_key);
    route_table_key = subnet->v6_route_table();
    if (route_table_key.id != PDS_ROUTE_TABLE_ID_INVALID) {
        v6_route_table =
            route_table_db()->find(&route_table_key);
    } else {
        v6_route_table = NULL;
    }

    policy_key = subnet->ing_v4_policy();
    ing_v4_policy = policy_db()->policy_find(&policy_key);
    policy_key = subnet->ing_v6_policy();
    ing_v6_policy = policy_db()->policy_find(&policy_key);
    policy_key = subnet->egr_v4_policy();
    egr_v4_policy = policy_db()->policy_find(&policy_key);
    policy_key = subnet->egr_v6_policy();
    egr_v6_policy = policy_db()->policy_find(&policy_key);

#if 0
    // TODO: program MAPPING table entry
    ret = activate_mapping_create_(epoch, vpc, spec, vnic,
                                   v4_route_table, v6_route_table,
                                   egr_v4_policy, egr_v6_policy);
    if (ret == SDK_RET_OK) {
        // program local_vnic_by_slot_rx table entry
        ret = activate_local_mapping_create_(epoch, vpc, spec, vnic,
                                             ing_v4_policy,
                                             ing_v6_policy);
    }
#endif

    return ret;
}

sdk_ret_t
vnic_impl::activate_vnic_delete_(pds_epoch_t epoch, vnic_entry *vnic) {
    sdk_ret_t              ret;
    sdk_table_api_params_t tparams = { 0 };

    // TODO: read LOCAL_MAPPING table entry and cleanup the entry

    // TODO: read MAPPING table entry and cleanup the entry

    return ret;
}

sdk_ret_t
vnic_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                       api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t          ret;
    pds_vnic_spec_t    *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
        spec = &obj_ctxt->api_params->vnic_spec;
        ret = activate_vnic_create_(epoch, (vnic_entry *)api_obj, spec);
        break;

    case api::API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_vnic_delete_(epoch, (vnic_entry *)api_obj);
        break;

    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
vnic_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                         api_op_t api_op) {
    return SDK_RET_INVALID_OP;
}

void
vnic_impl::fill_status_(pds_vnic_status_t *status) {
    status->hw_id = hw_id_;
}

sdk_ret_t
vnic_impl::fill_stats_(pds_vnic_stats_t *stats) {
#if 0
    p4pd_error_t p4pd_ret;
    vnic_tx_stats_actiondata_t tx_stats = { 0 };
    vnic_rx_stats_actiondata_t rx_stats = { 0 };

    // read P4TBL_ID_VNIC_TX_STATS table
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_TX_STATS, hw_id_, NULL,
                                      NULL, &tx_stats);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VNIC_TX_STATS table; hw_id:%u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    stats->tx_pkts  = *(uint64_t *)tx_stats.vnic_tx_stats_action.out_packets;
    stats->tx_bytes = *(uint64_t *)tx_stats.vnic_tx_stats_action.out_bytes;

    // read P4TBL_ID_VNIC_RX_STATS table
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VNIC_RX_STATS, hw_id_, NULL,
                                      NULL, &rx_stats);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VNIC_RX_STATS table hw_id:%u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    stats->rx_pkts  = *(uint64_t *)rx_stats.vnic_rx_stats_action.in_packets;
    stats->rx_bytes = *(uint64_t *)rx_stats.vnic_rx_stats_action.in_bytes;
#endif
    return SDK_RET_OK;
}

sdk_ret_t
vnic_impl::fill_spec_(pds_vnic_spec_t *spec) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
vnic_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t rv;
    pds_vnic_info_t *vnic_info = (pds_vnic_info_t *)info;

    rv = fill_spec_(&vnic_info->spec);
    if (unlikely(rv != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read hardware spec tables for vnic %s",
                      api_obj->key2str().c_str());
        return rv;
    }

    rv = fill_stats_(&vnic_info->stats);
    if (unlikely(rv != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to read hardware stats tables for vnic %s",
                      api_obj->key2str().c_str());
        return rv;
    }
    fill_status_(&vnic_info->status);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
