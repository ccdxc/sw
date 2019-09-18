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
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/nexthop_group.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_NEXTHOP_GROUP_IMPL - nexthop group datapath implementation
/// \ingroup PDS_NEXTHOP
/// \@{

nexthop_group_impl *
nexthop_group_impl::factory(pds_nexthop_group_spec_t *spec) {
    nexthop_group_impl *impl;

    // TODO: move to slab later
    impl = (nexthop_group_impl *)
               SDK_CALLOC(SDK_MEM_ALLOC_PDS_NEXTHOP_GROUP_IMPL,
                          sizeof(nexthop_group_impl));
    new (impl) nexthop_group_impl();
    return impl;
}

void
nexthop_group_impl::destroy(nexthop_group_impl *impl) {
    impl->~nexthop_group_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_NEXTHOP_GROUP_IMPL, impl);
}

sdk_ret_t
nexthop_group_impl::reserve_resources(api_base *orig_obj,
                                      obj_ctxt_t *obj_ctxt) {
    uint32_t idx;
    sdk_ret_t ret;
    pds_nexthop_group_spec_t *spec;

    spec = &obj_ctxt->api_params->nexthop_group_spec;
    if (spec->type == PDS_NHGROUP_TYPE_OVERLAY_ECMP) {
        // reserve an entry in NEXTHOP_GROUP table
        ret = nexthop_group_impl_db()->overlay_nhgroup_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve an entry in OVERLAY_ECMP table, ",
                          "for nexthop group %u, err %u", spec->key.id, ret);
            goto error;
        }
        hw_id_ = idx;
        // reserve entries in the subsequent table, if needed
        if (spec->num_entries) {
            if (spec->entry_type == PDS_NHGROUP_ENTRY_TYPE_NHGROUP) {
                ret = nexthop_group_impl_db()->underlay_nhgroup_idxr()->alloc(&idx,
                                                                              spec->num_entries);
                if (ret != SDK_RET_OK) {
                    PDS_TRACE_ERR("Failed to reserve %u entries in "
                                  "UNDERLAY_ECMP table for nexthop group %u, "
                                  "err %u", spec->num_entries,
                                  spec->key.id, ret);
                    goto error;
                }
                underlay_nhgroup_base_hw_id_ = idx;
            } else if (spec->entry_type == PDS_NHGROUP_ENTRY_TYPE_NEXTHOP) {
                ret = nexthop_impl_db()->nh_idxr()->alloc(&idx,
                                                          spec->num_entries);
                if (ret != SDK_RET_OK) {
                    PDS_TRACE_ERR("Failed to reserve %u entries in "
                                  "NEXTHOP table for nexthop group %u, "
                                  "err %u", spec->num_entries,
                                  spec->key.id, ret);
                    goto error;
                }
                nh_base_hw_id_ = idx;
            }
        }
    } else {
        // reserve an entry in NEXTHOP_GROUP table
        ret = nexthop_group_impl_db()->underlay_nhgroup_idxr()->alloc(&idx);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve an entry in UNDERLAY_ECMP table, ",
                          "for nexthop group %u, err %u", spec->key.id, ret);
            goto error;
        }
        hw_id_ = idx;
        // reserve entries in the subsequent table, if needed
        if (spec->num_entries) {
            ret = nexthop_impl_db()->nh_idxr()->alloc(&idx, spec->num_entries);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to reserve %u entries in "
                              "NEXTHOP table for nexthop group %u, "
                              "err %u", spec->num_entries,
                              spec->key.id, ret);
                goto error;
            }
            nh_base_hw_id_ = idx;
        }
    }
    return SDK_RET_OK;

error:
    return ret;
}

sdk_ret_t
nexthop_group_impl::release_resources(api_base *api_obj) {
    nexthop_group *nhgroup = (nexthop_group *)api_obj;

    if (hw_id_ != 0xFFFF) {
        if (nhgroup->type() == PDS_NHGROUP_TYPE_OVERLAY_ECMP) {
            nexthop_group_impl_db()->overlay_nhgroup_idxr()->free(hw_id_);
            if (underlay_nhgroup_base_hw_id_ != 0xFFFF) {
                nexthop_group_impl_db()->underlay_nhgroup_idxr()->free(underlay_nhgroup_base_hw_id_,
                                                                       nhgroup->num_entries());
            }
        } else {
            nexthop_group_impl_db()->underlay_nhgroup_idxr()->free(hw_id_);
        }
        if (nh_base_hw_id_ != 0xFFFF) {
            nexthop_impl_db()->nh_idxr()->free(nh_base_hw_id_,
                                               nhgroup->num_entries());
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::nuke_resources(api_base *api_obj) {
    // for indexers, release_resources() does the job
    return this->release_resources(api_obj);
}

sdk_ret_t
nexthop_group_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_nexthop_group_spec_t *spec;

    return SDK_RET_ERR;
}

sdk_ret_t
nexthop_group_impl::reprogram_hw(api_base *api_obj, api_op_t api_op) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop_group_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                              obj_ctxt_t *obj_ctxt) {
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
    return SDK_RET_ERR;
}

sdk_ret_t
nexthop_group_impl::read_hw(api_base *api_obj, obj_key_t *key,
                            obj_info_t *info) {
    return SDK_RET_ERR;
}

/// \@}    // end of PDS_NEXTHOP_GROUP_IMPL

}    // namespace impl
}    // namespace api
