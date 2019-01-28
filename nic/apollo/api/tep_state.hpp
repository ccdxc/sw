/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    tep_state.hpp
 *
 * @brief   Tunnel EndPoint (TEP) database maintenance
 */

#if !defined (__TEP_STATE_HPP__)
#define __TEP_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/tep.hpp"

namespace api {

/**
 * @defgroup OCI_TEP_STATE - tep state functionality
 * @ingroup OCI_TEP
 * @{
 */

/**
 * @brief    state maintained for teps
 */
class tep_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    tep_state();

    /**
     * @brief    destructor
     */
    ~tep_state();

    /**
     * @brief    allocate memory required for a tep
     * @return pointer to the allocated tep, NULL if no memory
     */
    tep_entry *tep_alloc(void);

    /**
     * @brief      free tep instance back to slab
     * @param[in]  tep   pointer to the allocated tep
     */
    void tep_free(tep_entry *tep);

    /**
     * @brief     lookup a tep in database given the key
     * @param[in] tep_key tep key
     */
    tep_entry *tep_find(oci_tep_key_t *tep_key) const;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *tep_ht(void) { return tep_ht_; }
    slab *tep_slab(void) { return tep_slab_; }
    friend class tep_entry;   /**< tep_entry class is friend of tep_state */

private:
    ht      *tep_ht_;      /**< Hash table root */
    slab    *tep_slab_;    /**< slab for allocating tep entry */
};

/** * @} */    // end of OCI_TEP_STATE

}  /** end namespace api */

using api::tep_state;

#endif    /** __TEP_STATE_HPP__ */
