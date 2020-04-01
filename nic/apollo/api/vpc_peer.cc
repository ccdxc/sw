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

api_base *
vpc_peer_entry::clone(api_ctxt_t *api_ctxt) {
    vpc_peer_entry *cloned_vpc_peer;

    cloned_vpc_peer = vpc_peer_db()->alloc();
    if (cloned_vpc_peer) {
        new (cloned_vpc_peer) vpc_peer_entry();
        if (cloned_vpc_peer->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
        cloned_vpc_peer->impl_ = impl_->clone();
        if (unlikely(cloned_vpc_peer->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone vpc peer obj %s impl",
                           key2str().c_str());
            goto error;
        }
    }
    return cloned_vpc_peer;

error:

    cloned_vpc_peer->~vpc_peer_entry();
    vpc_peer_db()->free(cloned_vpc_peer);
    return NULL;
}

sdk_ret_t
vpc_peer_entry::free(vpc_peer_entry *vpc_peer) {
    if (vpc_peer->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_VPC_PEER, vpc_peer->impl_);
    }
    vpc_peer->~vpc_peer_entry();
    vpc_peer_db()->free(vpc_peer);
    return SDK_RET_OK;
}

vpc_peer_entry *
vpc_peer_entry::build(pds_obj_key_t *key) {
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

    PDS_TRACE_DEBUG("Intializing vpc peering vpc1 %s, vpc2 %s",
                    spec->vpc1.str(), spec->vpc2.str());
    memcpy(&key_, &spec->key, sizeof(pds_obj_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
vpc_peer_entry::reserve_resources(api_base *orig_obj,
                                  api_obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, orig_obj, obj_ctxt);
}

sdk_ret_t
vpc_peer_entry::program_create(api_obj_ctxt_t *obj_ctxt) {
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
vpc_peer_entry::cleanup_config(api_obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
vpc_peer_entry::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_peer_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                                api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    pds_vpc_peer_spec_t *spec = &obj_ctxt->api_params->vpc_peer_spec;
    PDS_TRACE_DEBUG("Activating vpc peering (%s, %s) config",
                    spec->vpc1.str(), spec->vpc2.str());
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

sdk_ret_t
vpc_peer_entry::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
vpc_peer_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_VPC_PEER, this);
}

void
vpc_peer_entry::fill_spec_(pds_vpc_peer_spec_t *spec) {
    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
}

sdk_ret_t
vpc_peer_entry::read(pds_vpc_peer_info_t *info) {
    fill_spec_(&info->spec);
    return SDK_RET_OK;
    // TODO: implement vpc peer read
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}

}    // namespace api
