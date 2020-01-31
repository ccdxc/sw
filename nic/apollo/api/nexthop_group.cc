//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop group handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/nexthop.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/// \defgroup PDS_NEXTHOP_GROUP - nexthop group functionality
/// \ingroup PDS_NEXTHOP
/// @{

nexthop_group::nexthop_group() {
    type_ = PDS_NHGROUP_TYPE_NONE;
    num_nexthops_ = 0;
    ht_ctxt_.reset();
    impl_ = NULL;
}

nexthop_group *
nexthop_group::factory(pds_nexthop_group_spec_t *spec) {
    nexthop_group *nh_group;

    ///< create nexthop group with defaults, if any
    nh_group = nexthop_group_db()->alloc();
    if (nh_group) {
        new (nh_group) nexthop_group();
        nh_group->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_NEXTHOP_GROUP,
                                             spec);
        if (nh_group->impl_ == NULL) {
            nexthop_group::destroy(nh_group);
            return NULL;
        }
    }
    return nh_group;
}

nexthop_group::~nexthop_group() {
}

void
nexthop_group::destroy(nexthop_group *nh_group) {
    nh_group->nuke_resources_();
    impl_base::destroy(impl::IMPL_OBJ_ID_NEXTHOP_GROUP, nh_group->impl_);
    nh_group->~nexthop_group();
    nexthop_group_db()->free(nh_group);
}

api_base *
nexthop_group::clone(api_ctxt_t *api_ctxt) {
    nexthop_group *cloned_nh_group;

    cloned_nh_group = nexthop_group_db()->alloc();
    if (cloned_nh_group) {
        new (cloned_nh_group) nexthop_group();
        cloned_nh_group->impl_ = impl_->clone();
        if (unlikely(cloned_nh_group->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone nexthop group %s impl", key_.str());
            goto error;
        }
        cloned_nh_group->init_config(api_ctxt);
    }
    return cloned_nh_group;

error:

    cloned_nh_group->~nexthop_group();
    nexthop_group_db()->free(cloned_nh_group);
    return NULL;
}

sdk_ret_t
nexthop_group::free(nexthop_group *nh_group) {
    if (nh_group->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_NEXTHOP_GROUP, nh_group->impl_);
    }
    nh_group->~nexthop_group();
    nexthop_group_db()->free(nh_group);
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
nexthop_group::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
nexthop_group::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
nexthop_group::init_config(api_ctxt_t *api_ctxt) {
    pds_nexthop_group_spec_t *spec = &api_ctxt->api_params->nexthop_group_spec;

    memcpy(&this->key_, &spec->key, sizeof(key_));
    type_ = spec->type;
    num_nexthops_ = spec->num_nexthops;
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group::program_create(api_obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
nexthop_group::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
nexthop_group::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_nexthop_group_spec_t *spec = &obj_ctxt->api_params->nexthop_group_spec;

    if (type_ != spec->type) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"type\" from %u to %u "
                      "on nexthop group %s", type_, spec->type, key_.str());
        return SDK_RET_INVALID_ARG;
    }
    if (type_ != PDS_NHGROUP_TYPE_UNDERLAY_ECMP) {
        PDS_TRACE_ERR("Unsupported update of %u type ECMP group %s",
                      spec->type, key_.str());
        return SDK_RET_INVALID_ARG;
    }
    if (num_nexthops_ != spec->num_nexthops) {
        PDS_TRACE_ERR("Change of number of nexthops from %u to %u in "
                      "nexthop group %s not supported", num_nexthops_,
                      spec->num_nexthops, key_.str());
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop_group::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->update_hw(orig_obj, this, obj_ctxt);
}

sdk_ret_t
nexthop_group::activate_config(pds_epoch_t epoch, api_op_t api_op,
                               api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Activating nexthop group %s", key_.str());
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

void
nexthop_group::fill_spec_(pds_nexthop_group_spec_t *spec) {
    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
    spec->type = type_;
    spec->num_nexthops = num_nexthops_;
}

sdk_ret_t
nexthop_group::read(pds_nexthop_group_info_t *info) {
    fill_spec_(&info->spec);
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}

sdk_ret_t
nexthop_group::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (nexthop_group_db()->remove((nexthop_group *)orig_obj)) {
        return nexthop_group_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
nexthop_group::add_to_db(void) {
    return nexthop_group_db()->insert(this);
}

sdk_ret_t
nexthop_group::del_from_db(void) {
    if (nexthop_group_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
nexthop_group::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_NEXTHOP_GROUP, this);
}

/// @}     // end of PDS_NEXTHOP

}    // namespace api
