/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    subnet_state.hpp
 *
 * @brief   subnet database maintenance
 */

#if !defined (__SUBNET_STATE_HPP__)
#define __SUBNET_STATE_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/indexer/indexer.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/subnet.hpp"

namespace api {

/**
 * @defgroup OCI_SUBNET_STATE - subnet state functionality
 * @ingroup OCI_SUBNET
 * @{
 */

/**
 * @brief    state maintained for subnets
 */
class subnet_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    subnet_state();

    /**
     * @brief    destructor
     */
    ~subnet_state();

    /**
     * @brief    allocate memory required for a subnet
     * @return pointer to the allocated subnet, NULL if no memory
     */
    subnet_entry *subnet_alloc(void);

    /**
     * @brief      free subnet instance back to slab
     * @param[in]  subnet   pointer to the allocated subnet
     */
    void subnet_free(subnet_entry *subnet);

    /**
     * @brief     lookup a subnet in database given the key
     * @param[in] subnet_key subnet key
     */
    subnet_entry *subnet_find(oci_subnet_key_t *subnet_key) const;
    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *subnet_ht(void) { return subnet_ht_; }
    indexer *subnet_idxr(void) { return subnet_idxr_; }
    slab *subnet_slab(void) { return subnet_slab_; }
    friend class subnet_entry;   /**< subnet_entry class is friend of subnet_state */

private:
    ht      *subnet_ht_;      /**< Hash table root */
    indexer *subnet_idxr_;    /**< Indexer to allocate hw subnet id */
    slab    *subnet_slab_;    /**< slab for allocating subnet entry */
};

/** * @} */ // end of OCI_SUBNET_STATE

}    // namespace api

using api::subnet_state;

#endif    /** __SUBNET_STATE_HPP__ */
