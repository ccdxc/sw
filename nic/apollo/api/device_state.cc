/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    device_state.cc
 *
 * @brief   device database handling
 */

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/device_state.hpp"

namespace api {

/**
 * @defgroup PDS_DEVICE_STATE - device database functionality
 * @ingroup PDS_DEVICE
 * @{
 */

/**
 * @brief     allocate device instance
 * @return    pointer to the allocated device , NULL if no memory
 */
device_entry *
device_state::alloc(void) {
    return (device_entry *)SDK_CALLOC(PDS_MEM_ALLOC_DEVICE,
                                      sizeof(device_entry));
}

/**
 * @brief      free device instance back
 * @param[in]  device   pointer to the allocated device
 */
void
device_state::free(device_entry *device) {
    SDK_FREE(PDS_MEM_ALLOC_DEVICE, device);
}

/** @} */    // end of PDS_DEVICE_STATE

}    // namespace api
