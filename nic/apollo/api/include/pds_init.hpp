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

/// \defgroup PDS_INIT Initialization and teardown API
/// @{

/// \brief Initialization modes
typedef enum pds_init_mode_e {
    PDS_INIT_MODE_NONE,            ///< invalid mode
    PDS_INIT_MODE_COLD_START,      ///< initialize from scratch
                                   ///< ignore any state if preserved previously
    PDS_INIT_MODE_POST_UPGRADE,    ///< initialize using state preserved, if any
} pds_init_mode_t;

typedef enum pds_scale_profile_e {
    PDS_SCALE_PROFILE_DEFAULT,
    ///< P1 = 128 VPCs, 128 subnet, 1K vnics, 1M mappings,
    ///<      128 IPv4 route tables, with max. of 256K routers per table
    ///<      128 IPv6 route tables, with max. of 64k routes per table
    ///<      8M flows
    PDS_SCALE_PROFILE_P1,
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
                                                ///< files under <cfg_path>/pipeline/
                                                ///< are picked by the software
    bool                      batching_en;      ///< true if API batching is enabled
                                                ///< or else every API call is
                                                ///< treated as batch of one
} pds_init_params_t;

/// \brief initialize routine
/// \param[in] params init time parameters
/// \return #SDK_RET_OK on success, failure status code on error
sdk::sdk_ret_t pds_init(pds_init_params_t *params);

/// \@}

#endif    // __INCLUDE_API_PDS_INIT_HPP__
