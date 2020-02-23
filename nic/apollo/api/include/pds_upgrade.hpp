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
#include "nic/apollo/upgrade/event_cb.hpp"

/// \defgroup PDS_UPGRADE PDS Upgrade API
/// @{

/// \brief upgrade specification
typedef struct pds_upg_spec_s {
    upg_stage_t stage;      ///< stage to be advanced
} pds_upg_spec_t;

/// \brief upgrade request
/// this implements a stateless upgrade sequences.
/// it is the responsibilty of the upg manager to invoke the stages in proper order.
/// \param[in] spec specification
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_upgrade(pds_upg_spec_t *spec);


/// @}

#endif   // __INCLUDE_API_PDS_UPGRADE_HPP__
