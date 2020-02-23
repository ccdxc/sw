//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines init and teardown API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_INIT_HPP__
#define __INCLUDE_API_PDS_INIT_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_event.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"

/// \defgroup PDS_INIT Initialization and teardown API
/// @{

/// \brief Initialization modes
typedef enum pds_init_mode_e {
    PDS_INIT_MODE_NONE,            ///< invalid mode
    PDS_INIT_MODE_COLD_START,      ///< initialize from scratch
                                   ///< ignore any state if preserved previously
    PDS_INIT_MODE_POST_UPGRADE,    ///< initialize using state preserved, if any
} pds_init_mode_t;

/// \brief Initialization profiles
typedef enum pds_scale_profile_e {
    PDS_SCALE_PROFILE_DEFAULT,
    ///< P1 = 128 VPCs, 128 subnets, 1K vnics, 1M mappings,
    ///<      128 IPv4 route tables, with max. of 256K routes per table
    ///<      128 IPv6 route tables, with max. of 64k routes per table
    ///<      8M flows
    PDS_SCALE_PROFILE_P1,
    ///< P2 = 64 VPCs, 64 subnets, 64 vnics, 1M mappings,
    ///       64 IPv4 route tables, with max. of 1K routes per table
    ///       64 IPv6 route tables, with max. of 1K routes per table
    ///       8M IPv4 flows + 8M IPv6 flows, 64 IPv4 meter LPMs with max. of 1K
    ///       prefixes, 64 IPv6 meter LPMs wth max. of 1K prefixes, 1 IPv4
    ///       service tag LPM with max. of 256k prefixes, 1 IPv6 service tag
    ///       LPM with max. of 16k prefixes, 128 IPv4 security rule tables with
    ///       max. of 1K rules each, 128 IPv6 security rule tables with max. of
    ///       1K rules each
    PDS_SCALE_PROFILE_P2,
} pds_scale_profile_t;

/// \brief Initialization parameters
typedef struct pds_init_params_s {
    pds_init_mode_t           init_mode;        ///< mode of initialization
    sdk_logger::trace_cb_t    trace_cb;         ///< callback for trace msgs
    std::string               pipeline;         ///< P4 program pipeline name
                                                ///< only "apollo" supported now
    pds_scale_profile_t       scale_profile;    ///< scale profile for pipeline
    std::string               cfg_file;         ///< config files directory path
                                                ///< all config files are present,
                                                ///< files under \<cfg_path\>/pipeline/
                                                ///< are picked by the software
    pds_event_cb_t            event_cb;         ///< callback registered for events
} pds_init_params_t;

/// \brief initialize routine
/// \param[in] params init time parameters
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_init(pds_init_params_t *params);

/// \brief uninitialize routine
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_teardown(void);

/// @}

#endif    // __INCLUDE_API_PDS_INIT_HPP__
