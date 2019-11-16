//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc peer entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/vpc_peer.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

vpc_peer_entry::vpc_peer_entry() {
    // SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    stateless_ = true;
}

vpc_peer_entry *
vpc_peer_entry::factory(pds_vpc_peer_spec_t *spec) {
    vpc_peer_entry *vpc_peer;

    // create vpc peer entry with defaults, if any
    vpc_peer = vpc_peer_db()->alloc();
    if (vpc_peer) {
        new (vpc_peer) vpc_peer_entry();
        vpc_peer->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_VPC_PEER, spec);
        if (vpc_peer->impl_ == NULL) {
            vpc_peer_entry::destroy(vpc_peer);
            return NULL;
        }
    }
    return vpc_peer;
}

vpc_peer_entry::~vpc_peer_entry() {
    // SDK_SPINLOCK_DESTROY(&slock_);
}

void
vpc_peer_entry::destroy(vpc_peer_entry *vpc_peer) {
    vpc_peer->nuke_resources_();
    if (vpc_peer->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_VPC_PEER, vpc_peer->impl_);
    }
    vpc_peer->~vpc_peer_entry();
    vpc_peer_db()->free(vpc_peer);
}

vpc_peer_entry *
vpc_peer_entry::build(pds_vpc_peer_key_t *key) {
    return NULL;
}

void
vpc_peer_entry::soft_delete(vpc_peer_entry *vpc_peer) {
    if (vpc_peer->impl_) {
        impl_base::soft_delete(impl::IMPL_OBJ_ID_VPC_PEER, vpc_peer->impl_);
    }
    vpc_peer->~vpc_peer_entry();
    vpc_peer_db()->free(vpc_peer);
}

sdk_ret_t
vpc_peer_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_vpc_peer_spec_t *spec = &api_ctxt->api_params->vpc_peer_spec;

    PDS_TRACE_DEBUG("Intializing vpc peering vpc1 %u, vpc2 %u",
                    spec->vpc1.id, spec->vpc2.id);
    memcpy(&key_, &spec->key, sizeof(pds_vpc_peer_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
vpc_peer_entry::program_config(obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
vpc_peer_entry::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
vpc_peer_entry::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
vpc_peer_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
vpc_peer_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_peer_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                                obj_ctxt_t *obj_ctxt) {
    pds_vpc_peer_spec_t *spec = &obj_ctxt->api_params->vpc_peer_spec;
    PDS_TRACE_DEBUG("Activating vpc peering (%u, %u) config",
                    spec->vpc1.id, spec->vpc2.id);
    return impl_->activate_hw(this, epoch, api_op, obj_ctxt);
}

sdk_ret_t
vpc_peer_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_peer_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_VPC_PEER, this);
}

}    // namespace api
