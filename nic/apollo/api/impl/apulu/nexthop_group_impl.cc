//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of nexthop group
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/nexthop_group.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

#define ecmp_info    action_u.ecmp_ecmp_info

/// \defgroup PDS_NEXTHOP_GROUP_IMPL - nexthop group datapath implementation
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_group_impl *
nexthop_group_impl::factory(pds_nexthop_group_spec_t *spec) {
    nexthop_group_impl *impl;

    impl = nexthop_group_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) nexthop_group_impl();
    return impl;
}

void
nexthop_group_impl::destroy(nexthop_group_impl *impl) {
    impl->~nexthop_group_impl();
     nexthop_group_impl_db()->free(impl);
}

impl_base *
nexthop_group_impl::clone(void) {
    nexthop_group_impl *cloned_impl;

    cloned_impl = nexthop_group_impl_db()->alloc();
    new (cloned_impl) nexthop_group_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
nexthop_group_impl::free(nexthop_group_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::reserve_resources(api_base *api_obj,
                                      api_obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    pds_nexthop_group_spec_t *spec;

    spec = &obj_ctxt->api_params->nexthop_group_spec;
    // reserve an entry in NEXTHOP_GROUP table
    ret = nexthop_group_impl_db()->nhgroup_idxr()->alloc(&idx);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve an entry in ECMP table, ",
                      "for nexthop group %u, err %u", spec->key.id, ret);
        return ret;
    }
    hw_id_ = idx;
    if (spec->type == PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        if (spec->num_nexthops) {
            ret = nexthop_impl_db()->nh_idxr()->alloc_block(&idx, spec->num_nexthops);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to reserve %u entries in "
                              "NEXTHOP table for nexthop group %u, "
                              "err %u", spec->num_nexthops,
                              spec->key.id, ret);
                goto error;
            }
            nh_base_hw_id_ = idx;
        }
    }
    return SDK_RET_OK;

error:

    if (hw_id_ != 0xFFFF) {
        nexthop_group_impl_db()->nhgroup_idxr()->free(hw_id_);
        hw_id_ = 0xFFFF;
    }
    return ret;
}

sdk_ret_t
nexthop_group_impl::release_resources(api_base *api_obj) {
    nexthop_group *nhgroup = (nexthop_group *)api_obj;

    if (hw_id_ != 0xFFFF) {
        nexthop_group_impl_db()->nhgroup_idxr()->free(hw_id_);
    }
    if (nh_base_hw_id_ != 0xFFFF) {
        nexthop_impl_db()->nh_idxr()->free(nh_base_hw_id_,
                                           nhgroup->num_nexthops());
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::nuke_resources(api_base *api_obj) {
    // for indexers, release_resources() does the job
    return this->release_resources(api_obj);
}

sdk_ret_t
nexthop_group_impl::populate_ecmp_tep_info_(pds_nexthop_group_spec_t *spec,
                                            ecmp_actiondata_t *ecmp_data)
{
    tep_impl *tep;
    for (uint8_t i = 0; i < spec->num_nexthops; i++) {
        tep = (tep_impl *)tep_db()->find(&spec->nexthops[i].tep)->impl();
        switch (i) {
        case 0:
            ecmp_data->ecmp_info.tunnel_id1 = tep->hw_id();
            break;
        case 1:
            ecmp_data->ecmp_info.tunnel_id2 = tep->hw_id();
            break;
        case 2:
            ecmp_data->ecmp_info.tunnel_id3 = tep->hw_id();
            break;
        case 3:
            ecmp_data->ecmp_info.tunnel_id4 = tep->hw_id();
            break;
        default:
            PDS_TRACE_ERR("Unexpected nexthop count %u in nexthop group %u",
                          spec->num_nexthops, spec->key.id);
            return SDK_RET_ERR;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::activate_create_(pds_epoch_t epoch,
                                     nexthop_group *nh_group,
                                     pds_nexthop_group_spec_t *spec) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    nexthop_actiondata_t nh_data;
    ecmp_actiondata_t ecmp_data = { 0 };

    ecmp_data.action_id = ECMP_ECMP_INFO_ID;
    ecmp_data.ecmp_info.num_nexthops = spec->num_nexthops;
    if (spec->type == PDS_NHGROUP_TYPE_OVERLAY_ECMP) {
        // populate ECMP table entry data
        ecmp_data.ecmp_info.nexthop_type = NEXTHOP_TYPE_TUNNEL;
        ret = populate_ecmp_tep_info_(spec, &ecmp_data);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    } else if (spec->type == PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        // program the nexthops first in NEXTHOP table
        for (uint8_t i = 0; i < spec->num_nexthops; i++) {
            ret = populate_underlay_nh_info_(&spec->nexthops[i], &nh_data);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Programming of nexthop %u in NEXTHOP table for "
                              "nexthop group %u failed, err %u",
                              i, spec->key.id, ret);
                return ret;
            }
            p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP,
                                               nh_base_hw_id_ + i,
                                               NULL, NULL, &nh_data);
            if (p4pd_ret != P4PD_SUCCESS) {
                PDS_TRACE_ERR("Failed to program nexthop %u at idx %u",
                              spec->key.id, nh_base_hw_id_ + i);
                return sdk::SDK_RET_HW_PROGRAM_ERR;
            }
        }
        // populate ECMP table entry data
        ecmp_data.ecmp_info.nexthop_type = NEXTHOP_TYPE_NEXTHOP;
        ecmp_data.ecmp_info.nexthop_base = nh_base_hw_id_;
    }

    // program the ECMP table now
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_ECMP, hw_id_,
                                       NULL, NULL, &ecmp_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program nexthop group %u at idx %u in "
                      "ECMP table", spec->key.id, hw_id_);
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::activate_delete_(pds_epoch_t epoch,
                                     nexthop_group *nh_group) {
    uint8_t num_nexthops;
    pds_nexthop_group_key_t key;
    pds_nexthop_group_type_t type;
    p4pd_error_t p4pd_ret;
    nexthop_actiondata_t nh_data = { 0 };
    ecmp_actiondata_t ecmp_data = { 0 };

    key = nh_group->key();
    type = nh_group->type();
    num_nexthops = nh_group->num_nexthops();
    if (type == PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        for (uint8_t i = 0; i < num_nexthops; i++) {
            p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NEXTHOP,
                                               nh_base_hw_id_ + i,
                                               NULL, NULL, &nh_data);
            if (p4pd_ret != P4PD_SUCCESS) {
                PDS_TRACE_ERR("Failed to program nexthop %u at idx %u",
                              key.id, nh_base_hw_id_ + i);
                return sdk::SDK_RET_HW_PROGRAM_ERR;
            }
        }
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_ECMP, hw_id_,
                                       NULL, NULL, &ecmp_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to program nexthop %u at idx %u",
                      key.id, hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                                pds_epoch_t epoch, api_op_t api_op,
                                api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_nexthop_group_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->nexthop_group_spec;
        ret = activate_create_(epoch, (nexthop_group *)api_obj, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (nexthop_group *)api_obj);
        break;

    case API_OP_UPDATE:
    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
nexthop_group_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop_group_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                              api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop_group_impl::reactivate_hw(api_base *api_obj, pds_epoch_t epoch,
                                  api_op_t api_op) {
    return SDK_RET_ERR;
}

void
nexthop_group_impl::fill_status_(pds_nexthop_group_status_t *status) {
    status->hw_id = hw_id_;
}

sdk_ret_t
nexthop_group_impl::fill_spec_(pds_nexthop_group_spec_t *spec) {
    uint16_t nh_base_hw_id;
    p4pd_error_t p4pd_ret;
    ecmp_actiondata_t ecmp_data;

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_ECMP, hw_id_,
                                      NULL, NULL, &ecmp_data);
    if (unlikely(p4pd_ret != P4PD_SUCCESS)) {
        PDS_TRACE_ERR("Failed to read nexthop group table at index %u ret %u",
                      hw_id_, p4pd_ret);
        return sdk::SDK_RET_HW_READ_ERR;
    }
    if (spec->num_nexthops != ecmp_data.ecmp_info.num_nexthops) {
        spec->num_nexthops = ecmp_data.ecmp_info.num_nexthops;
        PDS_TRACE_ERR("Mismatch in number of nexthops in sw(%u) and hw(%u)"
                      " at index %u key %u", spec->num_nexthops,
                      ecmp_data.ecmp_info.num_nexthops, hw_id_, spec->key.id);
    }
    if (ecmp_data.ecmp_info.nexthop_type == NEXTHOP_TYPE_TUNNEL) {
        spec->type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
        // TODO - fetch nh spec from ecmp_data tunnel ids
    } else if (ecmp_data.ecmp_info.nexthop_type == NEXTHOP_TYPE_NEXTHOP) {
        spec->type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
        nh_base_hw_id = ecmp_data.ecmp_info.nexthop_base;
        for (uint8_t i = 0; i < spec->num_nexthops; i++) {
            fill_nh_spec_(&spec->nexthops[i], nh_base_hw_id + i);
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::read_hw(api_base *api_obj, obj_key_t *key,
                            obj_info_t *info) {
    sdk_ret_t ret;
    pds_nexthop_group_info_t *dinfo = (pds_nexthop_group_info_t *)info;

    ret = fill_spec_(&dinfo->spec);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    fill_status_(&dinfo->status);
    return SDK_RET_OK;
}

/// \@}    // end of PDS_NEXTHOP_GROUP_IMPL

}    // namespace impl
}    // namespace api
