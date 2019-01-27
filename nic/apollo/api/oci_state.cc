/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_state.cc
 *
 * @brief   This file contains implementation of oci state class
 */

#include "nic/apollo/api/oci_state.hpp"

namespace api {

/**< (singleton) instance of all OCI state in one place */
oci_state g_oci_state;

/**
 * @defgroup OCI_STATE - Internal state
 * @{
 */

/**< @brief    constructor */
oci_state::oci_state() {
}

/**< @brief    destructor */
oci_state::~oci_state() {
}

/** * @} */    // end of OCI_STATE

}    // namespace api

