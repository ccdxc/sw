/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    mapping_state.cc
 *
 * @brief   mapping databse handling
 */

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/mapping_state.hpp"

namespace api {

/**
 * @defgroup PDS_MAPPING_STATE - mapping database functionality
 * @ingroup PDS_MAPPING
 * @{
 */

/**
 * @brief    constructor
 */
mapping_state::mapping_state() {
    mapping_slab_ = slab::factory("mapping", PDS_SLAB_ID_MAPPING,
                                  sizeof(mapping_entry),
                                  8192, true, true, NULL);
    SDK_ASSERT(mapping_slab() != NULL);
}

/**
 * @brief    destructor
 */
mapping_state::~mapping_state() {
    slab::destroy(mapping_slab());
}

/**
 * @brief     allocate mapping instance
 * @return    pointer to the allocated mapping , NULL if no memory
 */
mapping_entry *
mapping_state::alloc(void) {
    return ((mapping_entry *)mapping_slab()->alloc());
}

/**
 * @brief      free mapping instance back to slab
 * @param[in]  mapping   pointer to the allocated mapping
 */
void
mapping_state::free(mapping_entry *mapping) {
    mapping_slab()->free(mapping);
}

/** @} */    // end of PDS_MAPPING_STATE

}    // namespace api
