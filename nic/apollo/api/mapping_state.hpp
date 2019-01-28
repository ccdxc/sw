/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    mapping_state.hpp
 *
 * @brief   mapping database maintenance
 */

#if !defined (__MAPPING_STATE_HPP__)
#define __MAPPING_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/api/mapping.hpp"

namespace api {

/**
 * @defgroup OCI_MAPPING_STATE - mapping state functionality
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief    state maintained for MAPPINGs
 */
class mapping_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    mapping_state();

    /**
     * @brief    destructor
     */
    ~mapping_state();

    /**
     * @brief    allocate memory required for a mapping
     * @return pointer to the allocated mapping, NULL if no memory
     */
    mapping_entry *mapping_alloc(void);

    /**
     * @brief      free mapping instance back to slab
     * @param[in]  mapping   pointer to the allocated mapping
     */
    void mapping_free(mapping_entry *mapping);

#if 0
    /**
     * @brief     lookup a mapping in database given the key
     * @param[in] mapping_key mapping key
     */
    mapping_entry *mapping_find(oci_mapping_key_t *mapping_key) const;
#endif
    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    slab *mapping_slab(void) { return mapping_slab_; }

private:
    slab    *mapping_slab_;    /**< slab for allocating mapping entry */
};

/** @} */    // end of OCI_MAPPING_STATE

}    // namespace api

using api::mapping_state;

#endif    /** __MAPPING_STATE_HPP__ */
