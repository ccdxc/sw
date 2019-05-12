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
#include "nic/apollo/api/device_state.hpp"

namespace api {

device_entry *
device_state::alloc(void) {
    device_cfg_ = (device_entry *)SDK_CALLOC(PDS_MEM_ALLOC_DEVICE,
                                             sizeof(device_entry));
    return device_cfg_;
}

void
device_state::free(device_entry *device) {
    SDK_FREE(PDS_MEM_ALLOC_DEVICE, device);
    device_cfg_ = NULL;
}

sdk_ret_t
device_state::walk(state_walk_cb_t walk_cb, void *ctxt) {
    if (device_cfg_) {
        walk_cb(device_cfg_, ctxt);
    }
    return SDK_RET_OK;
}

}    // namespace api
