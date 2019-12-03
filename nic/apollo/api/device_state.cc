//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// \file
/// device database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/device_state.hpp"

namespace api {

device_entry *
device_state::alloc(void) {
    return (device_entry *)SDK_CALLOC(PDS_MEM_ALLOC_DEVICE,
                                      sizeof(device_entry));
}

void
device_state::free(device_entry *device) {
    SDK_FREE(PDS_MEM_ALLOC_DEVICE, device);
}

sdk_ret_t
device_state::insert(device_entry *device) {
    if (device_cfg_) {
        PDS_TRACE_ERR("Attempt to insert a device entry before deleting");
        return SDK_RET_ENTRY_EXISTS;
    }
    device_cfg_ = device;
    return SDK_RET_OK;
}

device_entry *
device_state::remove(void) {
    device_entry *device;

    device = device_cfg_;
    device_cfg_ = NULL;
    return device;
}

sdk_ret_t
device_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    if (device_cfg_) {
        walk_cb(device_cfg_, ctxt);
    }
    return SDK_RET_OK;
}

}    // namespace api
