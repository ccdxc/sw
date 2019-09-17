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
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

if_entry::if_entry() {
    // SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
    if_info_ = NULL;
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
    }
    return intf;
}

if_entry::~if_entry() {
    // SDK_SPINLOCK_DESTROY(&slock_);
}

void
if_entry::destroy(if_entry *intf) {
    intf->~if_entry();
    if_db()->free(intf);
}

sdk_ret_t
if_entry::init_config(api_ctxt_t *api_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::release_resources(void) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::program_config(obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::reprogram_config(api_op_t api_op) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
if_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                          obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
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

}    // namespace api
