//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of subnet
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"

#define vni_info    action_u.vni_vni_info
#define bd_info     action_u.bd_bd_info

namespace api {
namespace impl {

/// \defgroup PDS_SUBNET_IMPL - subnet entry datapath implementation
/// \ingroup PDS_SUBNET
/// \@{

subnet_impl *
subnet_impl::factory(pds_subnet_spec_t *spec) {
    subnet_impl *impl;

    // TODO: move to slab later
    if (spec->fabric_encap.type != PDS_ENCAP_TYPE_VXLAN) {
        PDS_TRACE_ERR("Unknown fabric encap type %u, value %u - only VxLAN "
                      "fabric encap is supported", spec->fabric_encap.type,
                      spec->fabric_encap.val);
        return NULL;
    }
    impl = (subnet_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_SUBNET_IMPL,
                                     sizeof(subnet_impl));
    new (impl) subnet_impl();
    return impl;
}

void
subnet_impl::destroy(subnet_impl *impl) {
    impl->~subnet_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_SUBNET_IMPL, impl);
}

sdk_ret_t
subnet_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    vni_swkey_t vni_key =  { 0 };
    sdk_table_api_params_t tparams;
    pds_subnet_spec_t *spec = &obj_ctxt->api_params->subnet_spec;

    // reserve an entry in VNI table
    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, NULL,
                                   VNI_VNI_INFO_ID,
                                   handle_t::null());
    ret = vpc_impl_db()->vni_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in VNI table for subnet %u, "
                      "err %u", spec->key.id, ret);
        return ret;
    }
    vni_hdl_ = tparams.handle;
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };

    if (vni_hdl_.valid()) {
        tparams.handle = vni_hdl_;
        vpc_impl_db()->vni_tbl()->release(&tparams);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::nuke_resources(api_base *api_obj) {
    sdk_ret_t ret;
    vni_swkey_t vni_key =  { 0 };
    sdk_table_api_params_t tparams = { 0 };
    subnet_entry *subnet = (subnet_entry *)api_obj;

    vni_key.vxlan_1_vni = subnet->fabric_encap().val.vnid;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, NULL,
                                   VNI_VNI_INFO_ID,
                                   vni_hdl_);
    return vpc_impl_db()->vni_tbl()->remove(&tparams);
}

sdk_ret_t
subnet_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data { 0 };
    subnet_entry *subnet = (subnet_entry *)api_obj;
    pds_subnet_spec_t *spec = &obj_ctxt->api_params->subnet_spec;

    // program BD table in the egress pipe
    bd_data.action_id = BD_BD_INFO_ID;
    bd_data.bd_info.vni = spec->fabric_encap.val.vnid;
    memcpy(bd_data.bd_info.vrmac, spec->vr_mac, ETH_ADDR_LEN);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_BD, subnet->hw_id(),
                                       NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program BD table at index %u",
                      subnet->hw_id());
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
subnet_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data { 0 };
    subnet_entry *subnet = (subnet_entry *)api_obj;

    // program BD table in the egress pipe
    bd_data.action_id = BD_BD_INFO_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_BD, subnet->hw_id(),
                                       NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to cleanup BD table at index %u",
                      subnet->hw_id());
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                       obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
subnet_impl::activate_create_(pds_epoch_t epoch, subnet_entry *subnet,
                              pds_subnet_spec_t *spec) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    vni_swkey_t vni_key = { 0 };
    sdk_table_api_params_t tparams;
    vni_actiondata_t vni_data = { 0 };

    PDS_TRACE_DEBUG("Activating subnet %u, vpc %u, fabric encap (%u, %u)",
                    spec->key.id, spec->vpc.id, spec->fabric_encap.type,
                    spec->fabric_encap.val.vnid);
    vpc = vpc_db()->find(&spec->vpc);
    if (vpc == NULL) {
        PDS_TRACE_ERR("No vpc %u found to program subnet %u",
                      spec->vpc.id, spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // fill the key
    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    // fill the data
    vni_data.vni_info.bd_id = subnet->hw_id();
    vni_data.vni_info.vpc_id = vpc->hw_id();
    memcpy(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID, vni_hdl_);
    // program the VNI table
    ret = vpc_impl_db()->vni_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of VNI table failed for subnet %u, err %u",
                      spec->key.id, ret);
    }
    return ret;
}

sdk_ret_t
subnet_impl::activate_delete_(pds_epoch_t epoch, subnet_entry *subnet) {
    sdk_ret_t ret;
    vni_swkey_t vni_key = { 0 };
    vni_actiondata_t vni_data = { 0 };
    sdk_table_api_params_t tparams = { 0 };

    PDS_TRACE_DEBUG("Activating subnet %u delete, fabric encap (%u, %u)",
                    subnet->key().id, subnet->fabric_encap().type,
                    subnet->fabric_encap().val.vnid);
    // fill the key
    vni_key.vxlan_1_vni = subnet->fabric_encap().val.vnid;
    // fill the data
    vni_data.vni_info.bd_id = PDS_IMPL_RSVD_BD_HW_ID;
    vni_data.vni_info.vpc_id = PDS_IMPL_RSVD_VPC_HW_ID;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID,
                                   sdk::table::handle_t::null());
    // program the VNI table
    ret = vpc_impl_db()->vni_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of VNI table failed for subnet %u, err %u",
                      subnet->key().id, ret);
    }
    return ret;
}

sdk_ret_t
subnet_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                         api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_subnet_spec_t *spec;

    switch (api_op) {
    case api::API_OP_CREATE:
        spec = &obj_ctxt->api_params->subnet_spec;
        ret = activate_create_(epoch, (subnet_entry *)api_obj, spec);
        break;

    case api::API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (subnet_entry *)api_obj);
        break;

    case api::API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
subnet_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                           api_op_t api_op) {
    return SDK_RET_ERR;
}

sdk_ret_t
subnet_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    p4pd_error_t p4pd_ret;
    sdk_ret_t ret;
    bd_actiondata_t bd_data { 0 };
    subnet_entry *subnet = (subnet_entry *)api_obj;
    pds_subnet_info_t *sinfo = (pds_subnet_info_t *) info;
    pds_subnet_spec_t *spec = &sinfo->spec;
    vni_swkey_t vni_key = { 0 };
    sdk_table_api_params_t tparams;
    vni_actiondata_t vni_data = { 0 };

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_BD, subnet->hw_id(),
                                       NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read BD table at index %u",
                      subnet->hw_id());
        return sdk::SDK_RET_HW_READ_ERR;
    }

    spec->fabric_encap.val.vnid = bd_data.bd_info.vni;
    spec->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    memcpy(spec->vr_mac, bd_data.bd_info.vrmac, ETH_ADDR_LEN);

    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID, handle_t::null());
    // read the VNI table
    ret = vpc_impl_db()->vni_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read VNI table for vnid %u, err %u",
                      bd_data.bd_info.vni, ret);
        return sdk::SDK_RET_HW_READ_ERR;
    }

    // validate values read from hw table with sw state
    SDK_ASSERT(vni_data.vni_info.bd_id == subnet->hw_id());
    SDK_ASSERT(!memcmp(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN)); 
    return SDK_RET_OK;
}

/// \@}    // end of PDS_SUBNET_IMPL

}    // namespace impl
}    // namespace api
