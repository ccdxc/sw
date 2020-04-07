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
#include "nic/apollo/api/include/pds_device.hpp"
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

/// \brief Initialization parameters
typedef struct pds_init_params_s {
    pds_init_mode_t           init_mode;        ///< mode of initialization
    sdk_logger::trace_cb_t    trace_cb;         ///< callback for trace msgs
    std::string               pipeline;         ///< P4 program pipeline name
                                                ///< only "apollo" supported now
    pds_memory_profile_t      memory_profile;   ///< memory profile for pipeline
                                                ///< NOTE: memory carving config
                                                ///<       is picked based on
                                                ///<        this profile
    pds_device_profile_t      device_profile;   ///< PF/VF device profile
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
