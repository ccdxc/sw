/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    vnic_state.hpp
 *
 * @brief   vnic database handling
 */

#if !defined (__VNIC_STATE_HPP__)
#define __VNIC_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/api/vnic.hpp"

namespace api {

/**
 * @defgroup OCI_VNIC_STATE - vnic state functionality
 * @ingroup OCI_VNIC
 * @{
 */

/**
 * @brief    state maintained for VNICs
 */
class vnic_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    vnic_state();

    /**
     * @brief    destructor
     */
    ~vnic_state();

    /**
     * @brief    allocate memory required for a vnic
     * @return pointer to the allocated vnic, NULL if no memory
     */
    vnic_entry *vnic_alloc(void);

    /**
     * @brief      free vnic instance back to slab
     * @param[in]  vnic   pointer to the allocated vnic
     */
    void vnic_free(vnic_entry *vnic);

    /**
     * @brief     lookup a vnic in database given the key
     * @param[in] vnic_key vnic key
     */
    vnic_entry *vnic_find(oci_vnic_key_t *vnic_key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *vnic_ht(void) { return vnic_ht_; }
    slab *vnic_slab(void) { return vnic_slab_; }
    friend class vnic_entry;   /**< vnic_entry class is friend of vnic_state */

private:
    ht              *vnic_ht_;      /**< vnic database
                                         NOTE: even though VNIC scale is 1K, ids
                                               can be in the range [0, 4095], so
                                               to save memory, instead of 4k
                                               index table, we use hash table */
    slab            *vnic_slab_;    /**< slab to allocate vnic entry */
};

/** @} */    // end of OCI_VNIC_STATE

}    // namespace api

using api::vnic_state;

#endif    /** __VNIC_STATE_HPP__ */
