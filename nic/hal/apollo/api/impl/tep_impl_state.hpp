/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    tep_impl_state.hpp
 *
 * @brief   TEP implementation state
 */
#if !defined (__TEP_IMPL_STATE_HPP__)
#define __TEP_IMPL_STATEHPP__

#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/core/oci_state.hpp"

namespace impl {

/**
 * @defgroup OCI_TEP_IMPL_STATE - tep state functionality
 * @ingroup OCI_TEP
 * @{
 */

 /**< forward declaration */
class tep_impl;

/**
 * @brief    state maintained for teps
 */
class tep_impl_state : public obj_base {
public:
    /**< @brief    constructor */
    tep_impl_state(oci_state *state);

    /**< @brief    destructor */
    ~tep_impl_state();

private:
    indexer *tep_idxr(void) { return tep_idxr_; }
    directmap *tep_tx_tbl(void) { return tep_tx_tbl_; }
    directmap *nh_tx_tbl(void) { return nh_tx_tbl_; }
    friend class tep_impl;   /**< tep_impl class is friend of tep_impl_state */

private:
    indexer      *tep_idxr_;    /**< indexer to allocate hw ids for TEPs */
    directmap    *tep_tx_tbl_;  /**< directmap table for TEP_TX */
    directmap    *nh_tx_tbl_;   /**< directmap table for NH_TX */
};

/** * @} */    // end of OCI_TEP_IMPL_STATE

}    // namespace impl

#endif    /** __TEP_IMPL_STATE_HPP__ */
