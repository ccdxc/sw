/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_impl_state.cc
 *
 * @brief   This file contains implementation of oci impl state class
 */

#include "nic/hal/apollo/api/impl/oci_impl_state.hpp"

namespace impl {

/**< (singleton) instance of all OCI state in one place */
oci_impl_state g_oci_impl_state;

/**
 * @defgroup OCI_IMPL_STATE - Internal state
 * @{
 */
sdk_ret_t
oci_impl_state::init(void) {
    tep_impl_db_ = new tep_impl_state();
    vnic_impl_db_ = new vnic_impl_state();
    mapping_impl_db_ = new mapping_impl_state();

    return SDK_RET_OK;
}

/**< @brief    constructor */
oci_impl_state::oci_impl_state() {
    tep_impl_db_ = NULL;
    vnic_impl_db_ = NULL;
    mapping_impl_db_ = NULL;
}

/**< @brief    destructor */
oci_impl_state::~oci_impl_state() {
}

/** * @} */    // end of OCI_IMPL_STATE

}    // namespace impl

