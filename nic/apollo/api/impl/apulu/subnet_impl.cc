//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of subnet
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"

#define vni_info    action_u.vni_vni_info

namespace api {
namespace impl {

/// \defgroup PDS_SUBNET_IMPL - subnet entry datapath implementation
/// \ingroup PDS_SUBNET
/// \@{

subnet_impl *
subnet_impl::factory(pds_subnet_spec_t *spec) {
    subnet_impl *impl;

    if (spec->fabric_encap.type != PDS_ENCAP_TYPE_VXLAN) {
        PDS_TRACE_ERR("Unknown fabric encap type %u, value %u - only VxLAN "
                      "fabric encap is supported", spec->fabric_encap.type,
                      spec->fabric_encap.val);
        return NULL;
    }
    impl = subnet_impl_db()->alloc();
    new (impl) subnet_impl();
    return impl;
}

void
subnet_impl::destroy(subnet_impl *impl) {
    impl->~subnet_impl();
    subnet_impl_db()->free(impl);
}

impl_base *
subnet_impl::clone(void) {
    subnet_impl *cloned_impl;

    cloned_impl = subnet_impl_db()->alloc();
    new (cloned_impl) subnet_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
subnet_impl::free(subnet_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::reserve_resources(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    vni_swkey_t vni_key =  { 0 };
    sdk_table_api_params_t tparams;
    pds_subnet_spec_t *spec = &obj_ctxt->api_params->subnet_spec;

    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        // reserve a hw id for this subnet
        ret = subnet_impl_db()->subnet_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to allocate hw id for subnet %u, err %u",
                          spec->key.id, ret);
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
            PDS_TRACE_ERR("Failed to reserve entry in VNI table for subnet %u, "
                          "err %u", spec->key.id, ret);
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
subnet_impl::release_resources(api_base *api_obj) {
    sdk_table_api_params_t tparams = { 0 };

    if (hw_id_ != 0xFFFF) {
        subnet_impl_db()->subnet_idxr()->free(hw_id_);
    }

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

    if (hw_id_ != 0xFFFF) {
        subnet_impl_db()->subnet_idxr()->free(hw_id_);
    }

    if (vni_hdl_.valid()) {
        vni_key.vxlan_1_vni = subnet->fabric_encap().val.vnid;
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, NULL,
                                       VNI_VNI_INFO_ID, vni_hdl_);
        return vpc_impl_db()->vni_tbl()->remove(&tparams);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::populate_msg(pds_msg_t *msg, api_base *api_obj,
                          api_obj_ctxt_t *obj_ctxt) {
    msg->cfg_msg.subnet.status.hw_id = hw_id_;
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data { 0 };
    pds_subnet_spec_t *spec = &obj_ctxt->api_params->subnet_spec;

    // program BD table in the egress pipe
    bd_data.action_id = BD_BD_INFO_ID;
    bd_data.bd_info.vni = spec->fabric_encap.val.vnid;
    bd_data.bd_info.tos = spec->tos;
    memcpy(bd_data.bd_info.vrmac, spec->vr_mac, ETH_ADDR_LEN);
    PDS_TRACE_DEBUG("Programming BD table at %u with vni %u",
                    hw_id_, bd_data.bd_info.vni);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_BD, hw_id_,
                                       NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program BD table at index %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data = { 0 };

    // program BD table in the egress pipe
    bd_data.action_id = BD_BD_INFO_ID;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_BD, hw_id_,
                                       NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to cleanup BD table at index %u", hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                       api_obj_ctxt_t *obj_ctxt) {
    return program_hw(curr_obj, obj_ctxt);
}

sdk_ret_t
subnet_impl::activate_create_(pds_epoch_t epoch, subnet_entry *subnet,
                              pds_subnet_spec_t *spec) {
    sdk_ret_t ret;
    lif_impl *lif;
    vpc_entry *vpc;
    pds_lif_key_t lif_key;
    vni_swkey_t vni_key = { 0 };
    sdk_table_api_params_t tparams;
    vni_actiondata_t vni_data = { 0 };

    PDS_TRACE_DEBUG("Activating subnet %u, hw id %u, create host if 0x%x",
                    spec->key.id, hw_id_, spec->host_ifindex);
    vpc = vpc_find(&spec->vpc);
    if (vpc == NULL) {
        PDS_TRACE_ERR("No vpc %u found to program subnet %u",
                      spec->vpc.id, spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // fill the key
    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    // fill the data
    vni_data.action_id = VNI_VNI_INFO_ID;
    vni_data.vni_info.bd_id = hw_id_;
    vni_data.vni_info.vpc_id = vpc->hw_id();
    memcpy(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID, vni_hdl_);
    // program the VNI table
    ret = vpc_impl_db()->vni_tbl()->insert(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Programming of VNI table failed for subnet %u, err %u",
                      spec->key.id, ret);
        return ret;
    }

#if 0
    PDS_TRACE_ERR("Programmed VNI table subnet %u, vnid %u, vni_hdl_ 0x%llx",
                  spec->key.id, vni_key.vxlan_1_vni, vni_hdl_);
    // read the VNI table
    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    memset(&vni_data, 0, sizeof(vni_data));
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, /*&vni_key,*/ NULL, NULL,
                                   &vni_data,
                                   VNI_VNI_INFO_ID, vni_hdl_);
    ret = vpc_impl_db()->vni_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read VNI table for vnid %u, err %u",
                      vni_key.vxlan_1_vni, vni_hdl_, ret);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    PDS_TRACE_DEBUG("vnid data read back, bd id %u, vpc id %u",
                    vni_data.vni_info.bd_id, vni_data.vni_info.vpc_id);
#endif

    // if the subnet is enabled on host interface, update the lif table with
    // subnet and vpc ids appropriately
    if (spec->host_ifindex != IFINDEX_INVALID) {
        lif_key = LIF_IFINDEX_TO_LIF_ID(spec->host_ifindex);
        lif = lif_impl_db()->find(&lif_key);
        ret = program_lif_table(lif_key, P4_LIF_TYPE_HOST, vpc->hw_id(), hw_id_,
                                lif->vnic_hw_id());
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update lif 0x%x on subnet %u create, "
                          "err %u", spec->host_ifindex, spec->key.id, ret);
        }
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
subnet_impl::activate_update_(pds_epoch_t epoch, subnet_entry *subnet,
                              subnet_entry *orig_subnet,
                              api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    lif_impl *lif;
    vpc_entry *vpc;
    pds_lif_key_t lif_key;
    pds_subnet_spec_t *spec;
    vni_swkey_t vni_key = { 0 };
    sdk_table_api_params_t tparams;
    vni_actiondata_t vni_data = { 0 };

    PDS_TRACE_DEBUG("Activating subnet %u, hw id %u, update host if 0x%x",
                    spec->key.id, hw_id_, spec->host_ifindex);
    spec = &obj_ctxt->api_params->subnet_spec;
    vpc = vpc_find(&spec->vpc);
    if (vpc == NULL) {
        PDS_TRACE_ERR("No vpc %u found to program subnet %u",
                      spec->vpc.id, spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // fill the key
    vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
    // fill the data
    vni_data.action_id = VNI_VNI_INFO_ID;
    vni_data.vni_info.bd_id = hw_id_;
    vni_data.vni_info.vpc_id = vpc->hw_id();
    memcpy(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                   VNI_VNI_INFO_ID, vni_hdl_);
    // update the VNI table
    ret = vpc_impl_db()->vni_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Updating VNI table failed for subnet %u, err %u",
                      spec->key.id, ret);
        return ret;
    }

    if (obj_ctxt->upd_bmap & PDS_SUBNET_UPD_HOST_IFINDEX) {
        // host ifindex has changed
        if (spec->host_ifindex != IFINDEX_INVALID) {
            lif_key = LIF_IFINDEX_TO_LIF_ID(spec->host_ifindex);
            lif = lif_impl_db()->find(&lif_key);
            ret = program_lif_table(lif_key, P4_LIF_TYPE_HOST, vpc->hw_id(),
                                    hw_id_, lif->vnic_hw_id());
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to update lif 0x%x on subnet %u update, "
                              "err %u", spec->host_ifindex, spec->key.id, ret);
                return ret;
            }
        }
        if (orig_subnet->host_ifindex() != IFINDEX_INVALID) {
            // we need to reset the previous lif table entry
            lif_key = LIF_IFINDEX_TO_LIF_ID(orig_subnet->host_ifindex());
            lif = lif_impl_db()->find(&lif_key);
            ret = program_lif_table(lif_key, P4_LIF_TYPE_HOST,
                                    PDS_IMPL_RSVD_VPC_HW_ID,
                                    PDS_IMPL_RSVD_BD_HW_ID, lif->vnic_hw_id());
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to reset lif 0x%x on subnet %u update, "
                              "err %u", orig_subnet->host_ifindex(),
                              spec->key.id, ret);
                return ret;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                         pds_epoch_t epoch, api_op_t api_op,
                         api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_subnet_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->subnet_spec;
        ret = activate_create_(epoch, (subnet_entry *)api_obj, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (subnet_entry *)api_obj);
        break;

    case API_OP_UPDATE:
        ret = activate_update_(epoch, (subnet_entry *)api_obj,
                               (subnet_entry *)orig_obj, obj_ctxt);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

void
subnet_impl::fill_status_(pds_subnet_status_t *status) {
    status->hw_id = hw_id_;
}

sdk_ret_t
subnet_impl::fill_spec_(pds_subnet_spec_t *spec) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data;
    vni_actiondata_t vni_data;
    vni_swkey_t vni_key = { 0 };
    sdk_table_api_params_t tparams;

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_BD, hw_id_,
                                       NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read BD table at index %u", hw_id_);
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
    // TODO: this is disabled as the data being written and read back aren't
    // same
    //SDK_ASSERT(vni_data.vni_info.bd_id == hw_id_);
    //SDK_ASSERT(!memcmp(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN));

    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t ret;
    pds_subnet_info_t *subnet_info = (pds_subnet_info_t *)info;

    if ((ret = fill_spec_(&subnet_info->spec)) != SDK_RET_OK) {
        return ret;
    }

    fill_status_(&subnet_info->status);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_SUBNET_IMPL

}    // namespace impl
}    // namespace api
