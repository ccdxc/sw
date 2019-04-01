/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    pds_impl_state.cc
 *
 * @brief   This file contains implementation of pds impl state class
 */

#include "nic/apollo/api/impl/pds_impl_state.hpp"

namespace api {
namespace impl {

/**< (singleton) instance of all PDS state in one place */
pds_impl_state g_pds_impl_state;

/**
 * @defgroup PDS_IMPL_STATE - Internal state
 * @{
 */
sdk_ret_t
pds_impl_state::init(pds_state *state) {
    apollo_impl_db_ = new apollo_impl_state(state);
    tep_impl_db_ = new tep_impl_state(state);
    vnic_impl_db_ = new vnic_impl_state(state);
    mapping_impl_db_ = new mapping_impl_state(state);
    route_table_impl_db_ = new route_table_impl_state(state);
    security_policy_impl_db_ = new security_policy_impl_state(state);

    return SDK_RET_OK;
}

/**< @brief    constructor */
pds_impl_state::pds_impl_state() {
    apollo_impl_db_ = NULL;
    tep_impl_db_ = NULL;
    vnic_impl_db_ = NULL;
    mapping_impl_db_ = NULL;
    route_table_impl_db_ = NULL;
    security_policy_impl_db_ = NULL;
}

/**< @brief    destructor */
pds_impl_state::~pds_impl_state() {
    delete apollo_impl_db_;
    delete tep_impl_db_;
    delete vnic_impl_db_;
    delete mapping_impl_db_;
    delete route_table_impl_db_;
    delete security_policy_impl_db_;
}

/** * @} */    // end of PDS_IMPL_STATE

}    // namespace impl
}    // namespace api
