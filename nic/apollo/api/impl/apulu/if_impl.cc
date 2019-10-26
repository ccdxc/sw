//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of interface
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/impl/apulu/if_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"

namespace api {
namespace impl {

/// \defgroup IF_IMPL_IMPL - interface datapath implementation
/// \ingroup IF_IMPL
/// \@{

if_impl *
if_impl::factory(pds_if_spec_t *spec) {
    if_impl *impl;

    impl = (if_impl *)SDK_CALLOC(SDK_MEM_ALLOC_IF_IMPL_IMPL, sizeof(if_impl));
    new (impl) if_impl();
    return impl;
}

void
if_impl::destroy(if_impl *impl) {
    impl->~if_impl();
    SDK_FREE(SDK_MEM_ALLOC_IF_IMPL_IMPL, impl);
}

sdk_ret_t
if_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    pds_if_spec_t *spec;

    spec = &obj_ctxt->api_params->if_spec;
    switch (spec->type) {
    case PDS_IF_TYPE_UPLINK:
        ret = if_impl_db()->lif_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to alloc lif hw id for uplink 0x%x, err %u",
                          spec->key.id, ret);
            return ret;
        }
        hw_id_ = idx;
        break;

    case PDS_IF_TYPE_L3:
        // TODO: what are we using this for in case of L3 if ??
        ret = if_impl_db()->l3if_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to alloc hw id for L3 if 0x%x, err %u",
                          spec->key.id, ret);
            return ret;
        }
        hw_id_ = idx;
        break;

    default:
        PDS_TRACE_ERR("Unknown interface type %u for interface %u",
                      spec->type, spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_impl::release_resources(api_base *api_obj) {
    if_entry *intf = (if_entry *)api_obj;

    if (hw_id_ != 0xFFFF) {
        switch (intf->type()) {
        case PDS_IF_TYPE_UPLINK:
            if_impl_db()->lif_idxr()->free(hw_id_);
            break;

        case PDS_IF_TYPE_L3:
            if_impl_db()->l3if_idxr()->free(hw_id_);
            break;

        default:
            break;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };
    return SDK_RET_INVALID_OP;
}

#define lif_action              action_u.lif_lif_info
sdk_ret_t
if_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t tm_port;
    pds_if_spec_t *spec;
    p4pd_error_t p4pd_ret;
    lif_actiondata_t lif_data = { 0 };

    spec = &obj_ctxt->api_params->if_spec;
    if (spec->type == PDS_IF_TYPE_UPLINK) {
        // program the lif id in the TM
        tm_port =
            g_pds_state.catalogue()->logical_port_to_tm_port(spec->uplink_info.port_num);
        PDS_TRACE_DEBUG("Creating uplink if 0x%x, port %u, hw_id_ %u, "
                        "tm_port %u", spec->key.id, spec->uplink_info.port_num,
                        hw_id_, tm_port);
        ret = sdk::platform::capri::capri_tm_uplink_lif_set(tm_port, hw_id_);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to program uplink 0x%s's lif %u in TM "
                          "register", spec->key.id, hw_id_);
        }
        // program LIF table
        lif_data.action_id = LIF_LIF_INFO_ID;
        lif_data.lif_action.lif_type = P4_LIF_TYPE_UPLINK;
        lif_data.lif_action.vnic_id = PDS_IMPL_RSVD_VNIC_HW_ID;
        lif_data.lif_action.bd_id = PDS_IMPL_RSVD_BD_HW_ID;
        lif_data.lif_action.vpc_id = PDS_IMPL_RSVD_VPC_HW_ID;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_LIF, hw_id_,
                                           NULL, NULL, &lif_data);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to program LIF table for uplink lif %u",
                          hw_id_);
            return sdk::SDK_RET_HW_PROGRAM_ERR;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
if_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_impl::activate_create_(pds_epoch_t epoch, if_entry *intf,
                          pds_if_spec_t *spec) {
    PDS_TRACE_DEBUG("Activating if 0x%x, type %u, admin state %u",
                    spec->key.id, spec->type, spec->admin_state);
    return SDK_RET_OK;
}

sdk_ret_t
if_impl::activate_delete_(pds_epoch_t epoch, if_entry *intf) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
if_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                     api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_if_spec_t *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
        spec = &obj_ctxt->api_params->if_spec;
        ret = activate_create_(epoch, (if_entry *)api_obj, spec);
        break;

    case api::API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (if_entry *)api_obj);
        break;

    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
if_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                       api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
if_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of IF_IMPL_IMPL

}    // namespace impl
}    // namespace api
