//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop handling
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

/// \defgroup PDS_NEXTHOP_ENTRY - nexthop functionality
/// \ingroup PDS_NEXTHOP
/// @{

nexthop::nexthop() {
    ht_ctxt_.reset();
    impl_ = NULL;
}

nexthop *
nexthop::factory(pds_nexthop_spec_t *spec) {
    nexthop *nh;

    ///< create nexthop with defaults, if any
    nh = nexthop_db()->alloc();
    if (nh) {
        new (nh) nexthop();
        nh->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_NEXTHOP, spec);
        if (nh->impl_ == NULL) {
            nexthop::destroy(nh);
            return NULL;
        }
    }
    return nh;
}

nexthop::~nexthop() {
}

void
nexthop::destroy(nexthop *nh) {
    nh->nuke_resources_();
    if (nh->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_NEXTHOP, nh->impl_);
    }
    nh->~nexthop();
    nexthop_db()->free(nh);
}

api_base *
nexthop::clone(api_ctxt_t *api_ctxt) {
    nexthop *cloned_nh;

    cloned_nh = nexthop_db()->alloc();
    if (cloned_nh) {
        new (cloned_nh) nexthop();
        cloned_nh->impl_ = impl_->clone();
        if (unlikely(cloned_nh->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone nexthop %s impl", key_.str());
            goto error;
        }
        cloned_nh->init_config(api_ctxt);
    }
    return cloned_nh;

error:

    cloned_nh->~nexthop();
    nexthop_db()->free(cloned_nh);
    return NULL;
}

sdk_ret_t
nexthop::free(nexthop *nh) {
    if (nh->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_NEXTHOP, nh->impl_);
    }
    nh->~nexthop();
    nexthop_db()->free(nh);
    return SDK_RET_OK;
}

sdk_ret_t
nexthop::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
nexthop::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
nexthop::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
nexthop::init_config(api_ctxt_t *api_ctxt) {
    pds_nexthop_spec_t *spec = &api_ctxt->api_params->nexthop_spec;

    memcpy(&this->key_, &spec->key, sizeof(key_));
    type_ = spec->type;
    return SDK_RET_OK;
}

sdk_ret_t
nexthop::program_create(api_obj_ctxt_t *obj_ctxt) {
    pds_nexthop_spec_t *spec = &obj_ctxt->api_params->nexthop_spec;

    PDS_TRACE_VERBOSE("Programming nexthop %s, type %u", key_.str(), type_);
    if (type_ == PDS_NH_TYPE_IP) {
        PDS_TRACE_DEBUG("nexthop vpc %s, ip %s, vlan %u, mac %s",
                        spec->vpc.str(), ipaddr2str(&spec->ip),
                        spec->vlan, macaddr2str(spec->mac));
    }
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
nexthop::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
nexthop::add_deps(api_obj_ctxt_t *obj_ctxt) {
    // objects pointing to the nexthop need not be update because of nexthop
    // update as the h/w id is not changing
    return SDK_RET_OK;
}

sdk_ret_t
nexthop::reprogram_config(api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_ERR;
}

sdk_ret_t
nexthop::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_nexthop_spec_t *spec = &obj_ctxt->api_params->nexthop_spec;

    if (type_ != spec->type) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"type\" from %u to %s",
                      " on nexthop %s", type_, spec->type, spec->key.str());
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
nexthop::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->update_hw(orig_obj, this, obj_ctxt);
}

sdk_ret_t
nexthop::activate_config(pds_epoch_t epoch, api_op_t api_op,
                         api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_VERBOSE("Activating nexthop %s config", key_.str());
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

sdk_ret_t
nexthop::reactivate_config(pds_epoch_t epoch, api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

void
nexthop::fill_spec_(pds_nexthop_spec_t *spec) {
    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
}

sdk_ret_t
nexthop::read(pds_nexthop_info_t *info) {
    fill_spec_(&info->spec);
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}

sdk_ret_t
nexthop::add_to_db(void) {
    PDS_TRACE_VERBOSE("Adding nexthop %s to db", key_.str());
    return nexthop_db()->insert(this);
}

sdk_ret_t
nexthop::del_from_db(void) {
    if (nexthop_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
nexthop::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (nexthop_db()->remove((nexthop *)orig_obj)) {
        return nexthop_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
nexthop::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_NEXTHOP, this);
}

/// @}     // end of PDS_NEXTHOP_ENTRY

}    // namespace api
