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
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
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
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
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

sdk_ret_t
nexthop::init_config(api_ctxt_t *api_ctxt) {
    pds_nexthop_spec_t *spec = &api_ctxt->api_params->nexthop_spec;

    memcpy(&this->key_, &spec->key, sizeof(key_));
    type_ = spec->type;
    return SDK_RET_OK;
}

sdk_ret_t
nexthop::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
nexthop::program_config(obj_ctxt_t *obj_ctxt) {
    pds_nexthop_spec_t *spec = &obj_ctxt->api_params->nexthop_spec;

    PDS_TRACE_DEBUG("Programming nexthop %u, type %u", key_, type_);
    if (type_ == PDS_NH_TYPE_IP) {
        PDS_TRACE_DEBUG("nexthop vpc %u, ip %s, vlan %u, mac %s",
                        spec->vpc.id, ipaddr2str(&spec->ip),
                        spec->vlan, macaddr2str(spec->mac));
    }
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
nexthop::reprogram_config(api_op_t api_op) {
    return SDK_RET_ERR;
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
nexthop::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
nexthop::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    //return impl_->update_hw();
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop::activate_config(pds_epoch_t epoch, api_op_t api_op,
                         obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Activating nexthop %u config", key_);
    return impl_->activate_hw(this, epoch, api_op, obj_ctxt);
}

sdk_ret_t
nexthop::reactivate_config(pds_epoch_t epoch, api_op_t api_op) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop::read(pds_nexthop_key_t *key, pds_nexthop_info_t *info) {
    if (impl_) {
        return impl_->read_hw(this, (impl::obj_key_t *)key,
                              (impl::obj_info_t *)info);
    }
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
nexthop::add_to_db(void) {
    PDS_TRACE_VERBOSE("Adding nexthop %u to db", key_);
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
nexthop::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_NEXTHOP, this);
}

/// @}     // end of PDS_NEXTHOP_ENTRY

}    // namespace api
