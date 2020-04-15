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

// max of 1k classes are suppored for both remote and local mapping
// class id 1023 (PDS_IMPL_RSVD_MAPPING_CLASS_ID) is reserved to indicate
// that class id is not configured, so 0 to (PDS_IMPL_RSVD_MAPPING_CLASS_ID-1)
// class id values are valid
#define PDS_MAX_CLASS_ID_PER_VPC    1024

vpc_impl *
vpc_impl::factory(pds_vpc_spec_t *spec) {
    vpc_impl *impl;

    if (spec->fabric_encap.type != PDS_ENCAP_TYPE_VXLAN) {
        PDS_TRACE_ERR("Unknown fabric encap type %u, value %u - only VxLAN "
                      "fabric encap is supported", spec->fabric_encap.type,
                      spec->fabric_encap.val);
        return NULL;
    }
    impl = vpc_impl_db()->alloc();
    new (impl) vpc_impl(spec);

    // allocate tag/class state for this vpc
    impl->tag_state_ =
        (vpc_impl_tag_state_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_VPC_IMPL_TAG_STATE,
                                           sizeof(vpc_impl_tag_state_t));
    if (impl->tag_state_ == NULL) {
        goto error;
    }

    // create classid indexer for local mappings
    impl->tag_state_->local_mapping_classs_id_idxr_ =
        rte_indexer::factory(PDS_MAX_CLASS_ID_PER_VPC, false, false);
    if (impl->tag_state_->local_mapping_classs_id_idxr_ == NULL) {
        goto error;
    }
    // set the reserved classid aside
    impl->tag_state_->local_mapping_classs_id_idxr_->alloc(PDS_IMPL_RSVD_MAPPING_CLASS_ID);

    // create classid indexer for remote mappings
    impl->tag_state_->remote_mapping_class_id_idxr_ =
        rte_indexer::factory(PDS_MAX_CLASS_ID_PER_VPC, false, false);
    if (impl->tag_state_->remote_mapping_class_id_idxr_ == NULL) {
        goto error;
    }
    // set the reserved classid aside
    impl->tag_state_->remote_mapping_class_id_idxr_->alloc(PDS_IMPL_RSVD_MAPPING_CLASS_ID);
    return impl;

error:

    if (impl) {
        if (impl->tag_state_) {
            if (impl->tag_state_->local_mapping_classs_id_idxr_) {
                rte_indexer::destroy(impl->tag_state_->local_mapping_classs_id_idxr_);
            }
            if (impl->tag_state_->remote_mapping_class_id_idxr_) {
                rte_indexer::destroy(impl->tag_state_->remote_mapping_class_id_idxr_);
            }
            SDK_FREE(PDS_MEM_ALLOC_ID_VPC_IMPL_TAG_STATE, impl->tag_state_);
        }
        impl->~vpc_impl();
        vpc_impl_db()->free(impl);
    }
    return NULL;
}

void
vpc_impl::destroy(vpc_impl *impl) {
    if (impl->tag_state_) {
        if (impl->tag_state_->local_mapping_classs_id_idxr_) {
            rte_indexer::destroy(impl->tag_state_->local_mapping_classs_id_idxr_);
        }
        if (impl->tag_state_->remote_mapping_class_id_idxr_) {
            rte_indexer::destroy(impl->tag_state_->remote_mapping_class_id_idxr_);
        }
        SDK_FREE(PDS_MEM_ALLOC_ID_VPC_IMPL_TAG_STATE, impl->tag_state_);
    }
    impl->~vpc_impl();
    vpc_impl_db()->free(impl);
}

impl_base *
vpc_impl::clone(void) {
    vpc_impl *cloned_impl;

    cloned_impl = (vpc_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VPC_IMPL,
                                         sizeof(vpc_impl));
    new (cloned_impl) vpc_impl();
    // NOTE: we don't need to copy the resources at this time, they will be
    // trasferred during the update process
    return cloned_impl;
}

sdk_ret_t
vpc_impl::free(vpc_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::reserve_vni_entry_(uint32_t vnid) {
    sdk_ret_t ret;
    vni_swkey_t vni_key =  { 0 };
    sdk_table_api_params_t tparams;

    vni_key.vxlan_1_vni = vnid;
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, NULL,
                                   VNI_VNI_INFO_ID,
                                   handle_t::null());
    ret = vpc_impl_db()->vni_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    vni_hdl_ = tparams.handle;
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::reserve_resources(api_base *api_obj, api_base *orig_obj,
                            api_obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
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

        // reserve a bd id for this vpc
        ret = subnet_impl_db()->subnet_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to allocate BD hw id for vpc %s, err %u",
                           spec->key.str(), ret);
            return ret;
        }
        bd_hw_id_ = idx;

        // reserve an entry in VNI table
        ret = reserve_vni_entry_(spec->fabric_encap.val.vnid);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to reserve entry in VNI table for vpc %s, "
                          "err %u", spec->key.str(), ret);
            return ret;
        }
        break;

    case API_OP_UPDATE:
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        // if vnid is updated, reserve a handle for it in VNI table
        if (obj_ctxt->upd_bmap & PDS_VPC_UPD_FABRIC_ENCAP) {
            ret = reserve_vni_entry_(spec->fabric_encap.val.vnid);
            if (unlikely(ret != SDK_RET_OK)) {
                PDS_TRACE_ERR("Failed to reserve entry in VNI table for "
                              "vpc %s, err %u", spec->key.str(), ret);
                return ret;
            }
        }
        break;

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

    if (bd_hw_id_ != 0xFFFF) {
        subnet_impl_db()->subnet_idxr()->free(bd_hw_id_);
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

    if (bd_hw_id_ != 0xFFFF) {
        subnet_impl_db()->subnet_idxr()->free(bd_hw_id_);
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
    msg->cfg_msg.vpc.status.bd_hw_id = bd_hw_id_;
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
    sdk::lib::memrev(vpc_data.vpc_info.vrmac, spec->vr_mac, ETH_ADDR_LEN);
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
    vpc_impl *orig_impl = (vpc_impl *)(((vpc_entry *)orig_obj)->impl());

    // read the VPC table in the egress pipe
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VPC, orig_impl->hw_id_,
                                       NULL, NULL, &vpc_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VPC table at index %u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }

    // update the contents of that entry
    vpc_data.vpc_info.vni = spec->fabric_encap.val.vnid;
    vpc_data.vpc_info.tos = spec->tos;
    sdk::lib::memrev(vpc_data.vpc_info.vrmac, spec->vr_mac, ETH_ADDR_LEN);
    PDS_TRACE_DEBUG("Updating VPC table at %u with vni %u",
                    orig_impl->hw_id_, vpc_data.vpc_info.vni);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_VPC, orig_impl->hw_id_,
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
    vni_data.vni_info.bd_id = bd_hw_id_;
    vni_data.vni_info.vpc_id = hw_id_;
    sdk::lib::memrev(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN);
    vni_data.vni_info.is_l3_vnid = TRUE;
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
vpc_impl::activate_update_(pds_epoch_t epoch, vpc_entry *new_vpc,
                           vpc_entry *orig_vpc, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vni_swkey_t vni_key;
    pds_vpc_spec_t *spec;
    vni_actiondata_t vni_data;
    sdk_table_api_params_t tparams;
    vpc_impl *orig_impl = (vpc_impl *)orig_vpc->impl();

    // xfer resources from original object to the cloned object
    hw_id_ = orig_impl->hw_id_;
    bd_hw_id_ = orig_impl->bd_hw_id_;
    tag_state_ = orig_impl->tag_state_;

    PDS_TRACE_DEBUG("Activating vpc %s, hw id %u update",
                    spec->key.str(), hw_id_);

    if (obj_ctxt->upd_bmap & PDS_VPC_UPD_FABRIC_ENCAP) {
        spec = &obj_ctxt->api_params->vpc_spec;
        // fill the key
        memset(&vni_key, 0, sizeof(vni_key));
        vni_key.vxlan_1_vni = spec->fabric_encap.val.vnid;
        // fill the data
        memset(&vni_data, 0, sizeof(vni_data));
        vni_data.vni_info.bd_id = bd_hw_id_;
        vni_data.vni_info.vpc_id = hw_id_;
        sdk::lib::memrev(vni_data.vni_info.rmac, spec->vr_mac, ETH_ADDR_LEN);
        vni_data.vni_info.is_l3_vnid = TRUE;
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &vni_key, NULL, &vni_data,
                                       VNI_VNI_INFO_ID, vni_hdl_);
        // update the VNI table
        ret = vpc_impl_db()->vni_tbl()->update(&tparams);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Updating VNI table failed for vpc %s, err %u",
                          spec->key.str(), ret);
            return ret;
        }
    } else {
        // take over the existing VNI entry as there is not change
        vni_hdl_ = orig_impl->vni_hdl_;
        // and relinquish vni handle resource from original object
        orig_impl->vni_hdl_ = handle_t::null();
    }
    // update the vpc db
    vpc_impl_db()->update(hw_id_, this);

    // reliquish ownership of rest of the resources from original object so
    // they won't be freed
    orig_impl->hw_id_ = 0xFFFF;
    orig_impl->bd_hw_id_ = 0xFFFF;
    orig_impl->tag_state_ = nullptr;
    return SDK_RET_OK;
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
    vpc_impl *orig_impl;
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
        ret = activate_update_(epoch, (vpc_entry *)api_obj,
                               (vpc_entry *)orig_obj, obj_ctxt);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
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
    pds_vpc_status_t *status;
    vni_actiondata_t vni_data;
    vpc_actiondata_t vpc_data;
    vni_swkey_t vni_key = { 0 };
    sdk_table_api_params_t tparams = { 0 };
    pds_vpc_info_t *vpcinfo = (pds_vpc_info_t *)info;

    spec = &vpcinfo->spec;
    status = &vpcinfo->status;
    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_VPC, hw_id_,
                                       NULL, NULL, &vpc_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read VPC table at index %u", hw_id_);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    spec->fabric_encap.val.vnid = vpc_data.vpc_info.vni;
    sdk::lib::memrev(spec->vr_mac, vpc_data.vpc_info.vrmac, ETH_ADDR_LEN);
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
    status->hw_id = hw_id_;
    status->bd_hw_id = vni_data.vni_info.bd_id;
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::alloc_class_id(uint32_t tag, bool local, uint32_t *class_id) {
    sdk_ret_t ret;
    rte_indexer *class_idxr;
    tag2class_map_t::iterator it;
    tag2class_map_t *tag2class_map;
    class2tag_map_t *class2tag_map;

    if (local) {
        tag2class_map = &tag_state_->local_tag2class_map_;
        class2tag_map = &tag_state_->local_class2tag_map_;
        class_idxr = tag_state_->local_mapping_classs_id_idxr_;
    } else {
        tag2class_map = &tag_state_->remote_tag2class_map_;
        class2tag_map = &tag_state_->remote_class2tag_map_;
        class_idxr = tag_state_->remote_mapping_class_id_idxr_;
    }

    if (tag2class_map->find(tag) != tag2class_map->end()) {
        // tag to class id mapping exists
        (*tag2class_map)[tag].refcount++;
        *class_id = (*tag2class_map)[tag].class_id;
    } else {
        // try to allocate new class id
        ret = class_idxr->alloc(class_id);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to allocate class id for tag %u, err %u",
                          tag, ret);
            return ret;
        }
        // add tag -> class id mapping
        (*tag2class_map)[tag].class_id = *class_id;
        (*tag2class_map)[tag].refcount = 1;
        // add class id -> tag mapping
        (*class2tag_map)[*class_id] = tag;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::release_class_id(uint32_t class_id, bool local) {
    uint32_t tag;
    sdk_ret_t ret;
    rte_indexer *class_idxr;
    tag2class_map_t::iterator it;
    tag2class_map_t *tag2class_map;
    class2tag_map_t *class2tag_map;

    if (local) {
        tag2class_map = &tag_state_->local_tag2class_map_;
        class2tag_map = &tag_state_->local_class2tag_map_;
        class_idxr = tag_state_->local_mapping_classs_id_idxr_;
    } else {
        tag2class_map = &tag_state_->remote_tag2class_map_;
        class2tag_map = &tag_state_->remote_class2tag_map_;
        class_idxr = tag_state_->remote_mapping_class_id_idxr_;
    }

    if (class2tag_map->find(class_id) != class2tag_map->end()) {
        tag = (*class2tag_map)[class_id];
        if (unlikely(tag2class_map->find(tag) == tag2class_map->end())) {
            PDS_TRACE_ERR("tag2class lookup failed tag %u, class id %u",
                          tag, class_id);
            // go ahead and release the classid and class2tag map entry
            class2tag_map->erase(class_id);
            class_idxr->free(class_id);
        } else {
            (*tag2class_map)[tag].refcount--;
            if ((*tag2class_map)[tag].refcount == 0) {
                class2tag_map->erase(class_id);
                tag2class_map->erase(tag);
                class_idxr->free(class_id);
            }
        }
    } else {
        // handle it gracefully
        PDS_TRACE_ERR("class id %u not in use", class_id);
    }

    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl::release_tag(uint32_t tag, bool local) {
    sdk_ret_t ret;
    uint32_t class_id;
    rte_indexer *class_idxr;
    tag2class_map_t *tag2class_map;
    class2tag_map_t *class2tag_map;

    if (local) {
        tag2class_map = &tag_state_->local_tag2class_map_;
        class2tag_map = &tag_state_->local_class2tag_map_;
        class_idxr = tag_state_->local_mapping_classs_id_idxr_;
    } else {
        tag2class_map = &tag_state_->remote_tag2class_map_;
        class2tag_map = &tag_state_->remote_class2tag_map_;
        class_idxr = tag_state_->remote_mapping_class_id_idxr_;
    }

    if (tag2class_map->find(tag) != tag2class_map->end()) {
        (*tag2class_map)[tag].refcount--;
        if ((*tag2class_map)[tag].refcount == 0) {
            // cleanup the tag <-> class id mappings
            class_id = (*tag2class_map)[tag].class_id;
            tag2class_map->erase(tag);
            class2tag_map->erase(class_id);
            // free back the class id
            class_idxr->free(class_id);
        }
    } else {
        // handle it gracefully
        PDS_TRACE_ERR("Tag %u has no class id allocated", tag);
    }
    return SDK_RET_OK;
}

/// \@}    // end of PDS_VPC_IMPL

}    // namespace impl
}    // namespace api
