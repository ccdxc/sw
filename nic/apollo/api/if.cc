//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// interface entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

if_entry::if_entry() {
    // SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
    memset(&if_info_, 0, sizeof(if_info_));
    ifindex_ = IFINDEX_INVALID;
    ifindex_ht_ctxt_.reset();
}

// TODO: this method should go away !!!
if_entry *
if_entry::factory(pds_ifindex_t ifindex) {
    if_entry *intf;

    // create interface entry with defaults, if any
    intf = if_db()->alloc();
    if (intf) {
        new (intf) if_entry();
    }
    intf->ifindex_ = ifindex;
    return intf;
}

if_entry *
if_entry::factory(pds_if_spec_t *spec) {
    if_entry *intf;

    // create vpc entry with defaults, if any
    intf = if_db()->alloc();
    if (intf) {
        new (intf) if_entry();
        intf->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_IF, spec);
    }
    return intf;
}

if_entry::~if_entry() {
    // SDK_SPINLOCK_DESTROY(&slock_);
}

void
if_entry::destroy(if_entry *intf) {
    intf->nuke_resources_();
    if (intf->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_IF, intf->impl_);
    }
    intf->~if_entry();
    if_db()->free(intf);
}

sdk_ret_t
if_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_if_spec_t *spec = &api_ctxt->api_params->if_spec;

    PDS_TRACE_DEBUG("Initializing interface 0x%x, type %u",
                    spec->key.id, spec->type);
    memcpy(&key_, &spec->key, sizeof(pds_if_key_t));
    type_ = spec->type;
    ifindex_ = spec->key.id;
    admin_state_ = spec->admin_state;
    switch (type_) {
     case PDS_IF_TYPE_UPLINK:
         if_info_.uplink_.port_ = spec->uplink_info.port_num;
         break;
     case PDS_IF_TYPE_L3:
         if_info_.l3_.vpc_ = spec->l3_if_info.vpc;
         if_info_.l3_.ip_pfx_ = spec->l3_if_info.ip_prefix;
         if_info_.l3_.port_ = spec->l3_if_info.port_num;
         if_info_.l3_.encap_ = spec->l3_if_info.encap;
         memcpy(if_info_.l3_.mac_, spec->l3_if_info.mac_addr,
                ETH_ADDR_LEN);
         break;
     default:
         return sdk::SDK_RET_INVALID_ARG;
         break;
     }
     return SDK_RET_OK;
}

sdk_ret_t
if_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->reserve_resources(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::release_resources(void) {
    if (impl_) {
        impl_->release_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::nuke_resources_(void) {
    if (impl_) {
        return impl_->nuke_resources(this);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::program_create(obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->program_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::reprogram_config(api_op_t api_op) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->cleanup_hw(this, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                          api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (impl_) {
        return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
    }
    return SDK_RET_OK;
}

sdk_ret_t
if_entry::add_to_db(void) {
    return if_db()->insert(this);
}

sdk_ret_t
if_entry::del_from_db(void) {
    if (if_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
if_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::delay_delete(void) {
    PDS_TRACE_VERBOSE("Delay delete interface 0x%x", key_);
    return delay_delete_to_slab(PDS_SLAB_ID_IF, this);
}

sdk_ret_t
if_entry::read(pds_if_key_t *key, pds_if_info_t *info) {
    if (impl_) {
        return impl_->read_hw(this, (impl::obj_key_t *)key,
                              (impl::obj_info_t *)info);
    }
    return SDK_RET_OK;
}

}    // namespace api
