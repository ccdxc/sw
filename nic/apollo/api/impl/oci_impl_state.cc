/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_impl_state.cc
 *
 * @brief   This file contains implementation of oci impl state class
 */

#include "nic/apollo/api/impl/oci_impl_state.hpp"

namespace impl {

/**< (singleton) instance of all OCI state in one place */
oci_impl_state g_oci_impl_state;

/**
 * @defgroup OCI_IMPL_STATE - Internal state
 * @{
 */
sdk_ret_t
oci_impl_state::init(oci_state *state) {
    apollo_impl_db_ = new apollo_impl_state(state);
    tep_impl_db_ = new tep_impl_state(state);
    vnic_impl_db_ = new vnic_impl_state(state);
    mapping_impl_db_ = new mapping_impl_state(state);
    route_table_impl_db_ = new route_table_impl_state(state);

    return SDK_RET_OK;
}

/**< @brief    constructor */
oci_impl_state::oci_impl_state() {
    apollo_impl_db_ = NULL;
    tep_impl_db_ = NULL;
    vnic_impl_db_ = NULL;
    mapping_impl_db_ = NULL;
    route_table_impl_db_ = NULL;
}

/**< @brief    destructor */
oci_impl_state::~oci_impl_state() {
    delete apollo_impl_db_;
    delete tep_impl_db_;
    delete vnic_impl_db_;
    delete mapping_impl_db_;
    delete route_table_impl_db_;
}

/** * @} */    // end of OCI_IMPL_STATE

}    // namespace impl

