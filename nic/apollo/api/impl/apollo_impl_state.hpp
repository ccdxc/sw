/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    apollo_impl_state.hpp
 *
 * @brief   pipeline global state implementation
 */
#if !defined (__APOLLO_IMPL_STATE_HPP__)
#define __APOLLO_IMPL_STATEHPP__

#include "nic/sdk/lib/table/tcam/tcam.hpp"
#include "nic/apollo/framework/obj_base.hpp"
#include "nic/sdk/lib/table/tcam/tcam.hpp"
#include "nic/apollo/core/oci_state.hpp"

namespace impl {

/**
 * @defgroup OCI_APOLLO_IMPL_STATE - pipeline global state functionality
 * @ingroup OCI_APOLLO
 * @{
 */

 /**< forward declaration */
class apollo_impl;

/**
 * @brief    pipeline global state
 */
class apollo_impl_state : public obj_base {
public:
    /**< @brief    constructor */
    apollo_impl_state(oci_state *state);

    /**< @brief    destructor */
    ~apollo_impl_state();

private:
    tcam    *key_native_tbl(void) { return key_native_tbl_; };
    tcam    *key_tunneled_tbl(void) { return key_tunneled_tbl_; };
    friend class apollo_impl;   /**< apollo_impl class is friend of apollo_impl_state */

private:
    tcam    *key_native_tbl_;      /**< key table for native packets */
    tcam    *key_tunneled_tbl_;    /**< key table for tunneled packets */
};

/** * @} */    // end of OCI_APOLLO_IMPL_STATE

}    // namespace impl

#endif    /** __APOLLO_IMPL_STATE_HPP__ */
