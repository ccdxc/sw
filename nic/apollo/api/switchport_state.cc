/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    switchport_state.cc
 *
 * @brief   switchport database handling
 */

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/switchport_state.hpp"

namespace api {

/**
 * @defgroup PDS_SWITCHPORT_STATE - switchport database functionality
 * @ingroup PDS_SWITCHPORT
 * @{
 */

/**
 * @brief     allocate switchport instance
 * @return    pointer to the allocated switchport , NULL if no memory
 */
switchport_entry *
switchport_state::switchport_alloc(void) {
    return (switchport_entry *)SDK_CALLOC(PDS_MEM_ALLOC_SWITCHPORT,
                                          sizeof(switchport_entry));
}

/**
 * @brief      free switchport instance back to slab
 * @param[in]  switchport   pointer to the allocated switchport
 */
void
switchport_state::switchport_free(switchport_entry *switchport) {
    SDK_FREE(PDS_MEM_ALLOC_SWITCHPORT, switchport);
}

/** @} */    // end of PDS_SWITCHPORT_STATE

}    // namespace api
