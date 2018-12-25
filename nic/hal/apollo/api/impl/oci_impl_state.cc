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

/**< @brief    constructor */
oci_impl_state::oci_impl_state() {
}

/**< @brief    destructor */
oci_impl_state::~oci_impl_state() {
}

/** * @} */    // end of OCI_IMPL_STATE

}    // namespace impl

