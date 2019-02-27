/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    pds_state.cc
 *
 * @brief   This file contains implementation of pds state class
 */

#include "nic/apollo/api/pds_state.hpp"

namespace api {

/**< (singleton) instance of all PDS state in one place */
pds_state g_pds_state;

/**
 * @defgroup PDS_STATE - Internal state
 * @{
 */

/**< @brief    constructor */
pds_state::pds_state() {
}

/**< @brief    destructor */
pds_state::~pds_state() {
}

/** * @} */    // end of PDS_STATE

}    // namespace api

