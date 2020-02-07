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
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

#define vni_info    action_u.vni_vni_info
#define vpc_info    action_u.vpc_vpc_info

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL - vpc entry datapath implementation
/// \ingroup PDS_VPC
/// \@{

vpc_impl *
vpc_impl::factory(pds_vpc_spec_t *spec) {
    vpc_impl *impl;

    // TODO: move to slab later
    if (spec->fabric_encap.type != PDS_ENCAP_TYPE_VXLAN) {
        PDS_TRACE_ERR("Unknown fabric encap type %u, value %u - only VxLAN "
                      "fabric encap is supported", spec->fabric_encap.type,
                      spec->fabric_encap.val);
        return NULL;
    }
    impl = vpc_impl_db()->alloc();
    new (impl) vpc_impl(spec);
    return impl;
}

void
vpc_impl::destroy(vpc_impl *impl) {
    impl->~vpc_impl();
     vpc_impl_db()->free(impl);
}

impl_base *
vpc_impl::clone(void) {
    vpc_impl *cloned_impl;

    cloned_impl = (vpc_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VPC_IMPL,
                                         sizeof(vpc_impl));
    new (cloned_impl) vpc_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
vpc_impl::free(vpc_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::reserve_resources(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    vni_swkey_t vni_key =  { 0 };
    sdk_table_api_params_t tparams;
    pds_vpc_spec_t *spec = &obj_ctxt->api_params->vpc_spec;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        // reserve a hw id for this vpc
        ret = vpc_impl_db()->vpc_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to allocate hw id for vpc %s, err %u",
                           spec->key.str(), ret);
            return ret;
        }
        hw_id_ = idx;

        // reserve an entry in VNI table
        vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, NULL,
                                       VNI_VNI_INFO_ID,
                                       handle_t::null());
        ret = vpc_impl_db()->vni_tbl()->reserve(&tparams);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve entry in VNI table for vpc %s, "
                          "err %u", spec->key.str(), ret);
            return ret;
        }
        vni_hdl_ = tparams.handle;
        break;

    case API_OP_UPDATE:
        // vnid update is not supported, hence no need to reserve any resources
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };

    if (hw_id_ != 0xFFFF) {
        vpc_impl_db()->vpc_idxr()->free(hw_id_);
    }

    if (vni_hdl_.valid()) {
        tparams.handle = vni_hdl_;
        vpc_impl_db()->vni_tbl()->release(&tparams);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::nuke_resources(api_base *api_obj) {
    sdk_ret_t ret;
    vni_swkey_t vni_key =  { 0 };
    vpc_entry *vpc = (vpc_entry *)api_obj;
    sdk_table_api_params_t tparams = { 0 };

    if (hw_id_ != 0xFFFF) {
        vpc_impl_db()->vpc_idxr()->free(hw_id_);
    }

    if (vni_hdl_.valid()) {
        vni_key.vxlan_1_vni = vpc->fabric_encap().val.vnid;
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, NULL,
                                       VNI_VNI_INFO_ID,
                                       vni_hdl_);
       return vpc_impl_db()->vni_tbl()->remove(&tparams);
   }
   return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::populate_msg(pds_msg_t *msg, api_base *api_obj,
                        api_obj_ctxt_t *obj_ctxt) {

    msg->cfg_msg.vpc.status.hw_id = hw_id_;
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    vpc_actiondata_t vpc_data { 0 };
    pds_vpc_spec_t *spec = &obj_ctxt->api_params->vpc_spec;

    // program VPC table in the egress pipe
    vpc_data.action_id = VPC_VPC_INFO_ID;
    vpc_data.vpc_info.vni = spec->fabric_encap.val.vnid;
    vpc_data.vpc_info.tos = spec->tos;
    memcpy(vpc_data.vpc_info.vrmac, spec->vr_mac, ETH_ADDR_LEN);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VPC, hw_id_,
                                       NULL, NULL, &vpc_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program VPC table at index %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    vpc_actiondata_t vpc_data { 0 };

    // program VPC table in the egress pipe
    vpc_data.action_id = VPC_VPC_INFO_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VPC, hw_id_,
                                       NULL, NULL, &vpc_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to cleanup VPC table at index %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                    api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    vpc_actiondata_t vpc_data;
    pds_vpc_spec_t *spec = &obj_ctxt->api_params->vpc_spec;

    // read the VPC table in the egress pipe
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VPC, hw_id_,
                                       NULL, NULL, &vpc_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VPC table at index %u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }

    // update the contents of that entry
    vpc_data.vpc_info.vni = spec->fabric_encap.val.vnid;
    vpc_data.vpc_info.tos = spec->tos;
    memcpy(vpc_data.vpc_info.vrmac, spec->vr_mac, ETH_ADDR_LEN);
    PDS_TRACE_DEBUG("Updating VPC table at %u with vni %u",
                    hw_id_, vpc_data.vpc_info.vni);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VPC, hw_id_,
                                       NULL, NULL, &vpc_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to update VPC table at index %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::activate_create_(pds_epoch_t epoch, vpc_entry *vpc,
                           pds_vpc_spec_t *spec) {
    sdk_ret_t ret;
    vni_swkey_t vni_key = { 0 };
    vni_actiondata_t vni_data = { 0 };
    sdk_table_api_params_t tparams = { 0 };

    PDS_TRACE_DEBUG("Activating vpc %s, hw id %u create, type %u, "
                    "fabric encap (%u, %u)", spec->key.str(), hw_id_,
                    spec->type, spec->fabric_encap.type,
                    spec->fabric_encap.val.vnid);
    // fill the key
    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    // fill the data
    vni_data.vni_info.bd_id = hw_id_;    // bd hw id = vpc hw id for a vpc
    vni_data.vni_info.vpc_id = hw_id_;
    memcpy(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID, vni_hdl_);
    // program the VNI table
    ret = vpc_impl_db()->vni_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of VNI table failed for vpc %s, err %u",
                      spec->key.str(), ret);
    }
    vpc_impl_db()->insert(hw_id_, this);
    return ret;
}

sdk_ret_t
vpc_impl::activate_update_(pds_epoch_t epoch, vpc_entry *vpc,
                           api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_vpc_spec_t *spec;
    vni_swkey_t vni_key = { 0 };
    vni_actiondata_t vni_data = { 0 };
    sdk_table_api_params_t tparams = { 0 };

    spec = &obj_ctxt->api_params->vpc_spec;
    PDS_TRACE_DEBUG("Activating vpc %s, hw id %u update",
                    spec->key.str(), hw_id_);

    // fill the key
    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    // fill the data
    vni_data.vni_info.bd_id = hw_id_;    // bd hw id = vpc hw id for a vpc
    vni_data.vni_info.vpc_id = hw_id_;
    memcpy(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID, vni_hdl_);
    // update the VNI table
    ret = vpc_impl_db()->vni_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Updating VNI table failed for vpc %s, err %u",
                      spec->key.str(), ret);
    }

    // delete old impl and insert cloned impl into ht
    vpc_impl_db()->remove(hw_id_);
    vpc_impl_db()->insert(hw_id_, this);
    return ret;
}

sdk_ret_t
vpc_impl::activate_delete_(pds_epoch_t epoch, vpc_entry *vpc) {
    sdk_ret_t ret;
    vni_swkey_t vni_key = { 0 };
    vni_actiondata_t vni_data = { 0 };
    sdk_table_api_params_t tparams = { 0 };

    PDS_TRACE_DEBUG("Activating vpc %s delete, type %u, fabric encap (%u, %u)",
                    vpc->key().str(), vpc->type(), vpc->fabric_encap().type,
                    vpc->fabric_encap().val.vnid);
    // fill the key
    vni_key.vxlan_1_vni = vpc->fabric_encap().val.vnid;
    // fill the data
    vni_data.vni_info.bd_id = PDS_IMPL_RSVD_BD_HW_ID;
    vni_data.vni_info.vpc_id = PDS_IMPL_RSVD_VPC_HW_ID;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID,
                                   sdk::table::handle_t::null());
    // program the VNI table
    ret = vpc_impl_db()->vni_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of VNI table failed for vpc %s, err %u",
                      vpc->key().str(), ret);
    }
    vpc_impl_db()->remove(hw_id_);
    return ret;
}

sdk_ret_t
vpc_impl::activate_hw(api_base *api_obj, api_base *orig_obj, pds_epoch_t epoch,
                      api_op_t api_op, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_vpc_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->vpc_spec;
        ret = activate_create_(epoch, (vpc_entry *)api_obj, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (vpc_entry *)api_obj);
        break;

    case API_OP_UPDATE:
        ret = activate_update_(epoch, (vpc_entry *)api_obj, obj_ctxt);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
vpc_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t ret;
    pds_vpc_spec_t *spec;
    p4pd_error_t p4pd_ret;
    vni_actiondata_t vni_data;
    vpc_actiondata_t vpc_data;
    vni_swkey_t vni_key = { 0 };
    sdk_table_api_params_t tparams = { 0 };
    pds_vpc_info_t *vpcinfo = (pds_vpc_info_t *)info;
    pds_vpc_status_t *status;

    spec = &vpcinfo->spec;
    status = &vpcinfo->status;
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VPC, hw_id_,
                                       NULL, NULL, &vpc_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VPC table at index %u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    spec->fabric_encap.val.vnid = vpc_data.vpc_info.vni;
    memcpy(spec->vr_mac, vpc_data.vpc_info.vrmac, ETH_ADDR_LEN);
    spec->tos = vpc_data.vpc_info.tos;
    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID, handle_t::null());
    // read the VNI table
    ret = vpc_impl_db()->vni_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read VNI table for vpc %s, vnid %u, err %u",
                      spec->key.str(), vni_key.vxlan_1_vni, ret);
        return ret;
    }

    // validate values read from hw table with sw state
    SDK_ASSERT(vni_data.vni_info.bd_id == hw_id_);
    status->hw_id = hw_id_;
    return SDK_RET_OK;
}

/// \@}    // end of PDS_VPC_IMPL

}    // namespace impl
}    // namespace api
