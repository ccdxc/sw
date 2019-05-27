//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of vpc
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/impl/artemis/vpc_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL - vpc entry datapath implementation
/// \ingroup PDS_VPC
/// \@{

vpc_impl *
vpc_impl::factory(pds_vpc_spec_t *spec) {
    vpc_impl *impl;

    // TODO: move to slab later
    impl = (vpc_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VPC_IMPL,
                                   sizeof(vpc_impl));
    new (impl) vpc_impl();
    return impl;
}

void
vpc_impl::destroy(vpc_impl *impl) {
    impl->~vpc_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_VPC_IMPL, impl);
}

sdk_ret_t
vpc_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    tep1_rx_swkey_t tep1_rx_key = { 0 };
    tep1_rx_swkey_mask_t tep1_rx_mask = { 0 };
    sdk_table_api_params_t api_params = { 0 };
    pds_vpc_spec_t *spec = &obj_ctxt->api_params->vpc_spec;

    tep1_rx_key.vxlan_1_vni = spec->fabric_encap.val.value;
    tep1_rx_mask.vxlan_1_vni_mask = 0xFFFFFFFF;
    api_params.key = &tep1_rx_key;
    api_params.mask = &tep1_rx_mask;
    api_params.handle = sdk::table::handle_t::null();
    ret = vpc_impl_db()->tep1_rx_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in TEP1_RX "
                      "table for vpc %u, err %u", spec->key.id, ret);
        return ret;
    }
    tep1_rx_handle_ = api_params.handle;
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };

    if (tep1_rx_handle_.valid()) {
        api_params.handle = tep1_rx_handle_;
        vpc_impl_db()->tep1_rx_tbl()->release(&api_params);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t api_params = { 0 };

    if (tep1_rx_handle_.valid()) {
        api_params.handle = tep1_rx_handle_;
        vpc_impl_db()->tep1_rx_tbl()->remove(&api_params);
    }
    return SDK_RET_OK;
}

// TODO: when epoch support is added to these tables, we should pick
//       old epoch contents and override them !!!
sdk_ret_t
vpc_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
vpc_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                     obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

#define tep1_rx_info    action_u.tep1_rx_tep1_rx_info
sdk_ret_t
vpc_impl::activate_vpc_create_(pds_epoch_t epoch, vpc_entry *vpc,
                               pds_vpc_spec_t *spec) {
    sdk_ret_t ret;
    tep1_rx_swkey_t tep1_rx_key = { 0 };
    tep1_rx_swkey_mask_t tep1_rx_mask = { 0 };
    tep1_rx_actiondata_t tep1_rx_data = { 0 };
    sdk_table_api_params_t api_params = { 0 };

    tep1_rx_key.vxlan_1_vni = spec->fabric_encap.val.value;
    tep1_rx_mask.vxlan_1_vni_mask = 0xFFFFFFFF;
    tep1_rx_data.tep1_rx_info.decap_next = 0;
    tep1_rx_data.tep1_rx_info.src_vpc_id = vpc->hw_id();

    api_params.key = &tep1_rx_key;
    api_params.mask = &tep1_rx_mask;
    api_params.appdata = &tep1_rx_data;
    api_params.action_id = TEP1_RX_TEP1_RX_INFO_ID;
    api_params.handle = tep1_rx_handle_;
    ret = vpc_impl_db()->tep1_rx_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of TEP1_RX table failed for vpc %u, err %u",
                      spec->key.id, ret);
    }
    return ret;
}

sdk_ret_t
vpc_impl::activate_vpc_delete_(pds_epoch_t epoch, vpc_entry *vpc) {
    // invalidate entry in tcam (but not release resources) ??
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                       api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_vpc_spec_t *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
        spec = &obj_ctxt->api_params->vpc_spec;
        ret = activate_vpc_create_(epoch, (vpc_entry *)api_obj, spec);
        break;

    case api::API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_vpc_delete_(epoch, (vpc_entry *)api_obj);
        break;

    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
vpc_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                        api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
vpc_impl::read_hw(obj_key_t *key, obj_info_t *info, void *arg) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_VPC_IMPL

}    // namespace impl
}    // namespace api
