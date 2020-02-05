//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines upgrade API for PDS
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_UPGRADE_HPP__
#define __INCLUDE_API_PDS_UPGRADE_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/upgrade/upgrade.hpp"

/// \defgroup PDS_UPGRADE PDS Upgrade API
/// @{

/// \brief upgrade specification
typedef struct pds_upg_spec_s {
    upg_stage_t stage;      ///< stage to be advanced
    upg_mode_t mode;        ///< mode of upgrade
} pds_upg_spec_t;

/// \brief Upgrade request
/// This implements a stateless upgrade sequences.
/// It is the responsibilty of the upg manager to invoke the stages in proper order.
/// \param[in] spec Specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_upgrade(pds_upg_spec_t *spec);

// callback function for upgrade specific events
typedef sdk_ret_t (*pds_upg_event_cb_t)(const pds_upg_spec_t *spec);

/// @}

#endif   // __INCLUDE_API_PDS_UPGRADE_HPP__
