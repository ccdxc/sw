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
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/if.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

if_entry::if_entry() {
    // SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
    if_info_ = NULL;
}

if_entry *
if_entry::factory(pds_ifindex_t key) {
    if_entry *intf;

    // create vcn entry with defaults, if any
    intf = if_db()->alloc();
    if (intf) {
        new (intf) if_entry();
    }
    intf->key_ = key;
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
if_entry::delay_delete(void) {
    PDS_TRACE_VERBOSE("Delay delete interface 0x%x", key_);
    return delay_delete_to_slab(PDS_SLAB_ID_IF, this);
}

}    // namespace api
