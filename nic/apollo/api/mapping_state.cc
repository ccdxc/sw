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
    SDK_ASSERT(mapping_slab_ != NULL);
}

/**
 * @brief    destructor
 */
mapping_state::~mapping_state() {
    slab::destroy(mapping_slab_);
}

/**
 * @brief     allocate mapping instance
 * @return    pointer to the allocated mapping , NULL if no memory
 */
mapping_entry *
mapping_state::mapping_alloc(void) {
    return ((mapping_entry *)mapping_slab_->alloc());
}

/**
 * @brief      free mapping instance back to slab
 * @param[in]  mapping   pointer to the allocated mapping
 */
void
mapping_state::mapping_free(mapping_entry *mapping) {
    mapping_slab_->free(mapping);
}

#if 0
/**
 * @brief        lookup mapping in database with given key
 * @param[in]    mapping_key mapping key
 * @return       pointer to the mapping instance found or NULL
 */
mapping_entry *
mapping_state::mapping_find(pds_mapping_key_t *mapping_key) const {
    // TODO: if we really need to support this, do
    //       1. impl->find(key)
    //       2. decode to pds_mapping_key_t
    //       3. return the info
    return NULL;
}
#endif

/** @} */    // end of PDS_MAPPING_STATE

}    // namespace api
