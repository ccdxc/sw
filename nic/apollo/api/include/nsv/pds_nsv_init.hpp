//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines NVME Init and teardown APIs
///
//----------------------------------------------------------------------------

#ifndef __PDS_NSV_INIT_HPP__
#define __PDS_NSV_INIT_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

/// \defgroup PDS_NSV_INIT - initialization and teardown API
/// @{

/// \brief Max Limits
#define PDS_NSV_NS_MAX                4096 ///< Max Namespace
#define PDS_NSV_NS_SESSION_MAX        1024 ///< Max Session per Namespace
#define PDS_NSV_CNTRLR_NS_MAX         64   ///< Max Namespace per controller
#define PDS_NSV_CNTRLR_QUEUED_CMD_MAX 4096 ///< Max Outstanding cmds/controller

/// \brief PDS_NSV library initialization modes
typedef enum pds_nsv_init_mode_e {
    PDS_NSV_INIT_MODE_NONE,         ///< invalid mode
    PDS_NSV_INIT_MODE_COLD_START,   ///< initialize from scratch, ignore any state if preserved previously
    PDS_NSV_INIT_MODE_RECOVER,	   	///< Attempt to recover sw state 
    PDS_NSV_INIT_MODE_POST_UPGRADE, ///< initialize using state preserved, if any
} pds_nsv_init_mode_t;

/// \brief PDS_NSV library initialization time parameters
typedef struct pds_nsv_init_params_s {
    uint32_t max_ns;             ///< max number of namespace across all subsys Default - 4096
    uint32_t max_session_per_ns; ///< max number of sessions per ns. Default - 1024
    uint32_t max_ns_per_cntrlr;  ///< max number of namespace per cntrlr. Default - 64
} __PACK__ pds_nsv_init_params_t;

/// \brief Initialize PDS_NSV HAL
/// \param[in] params init time parameters
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_init(pds_nsv_init_params_t *params);

/// \brief Teardown PDS_NSV HAL
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nsv_teardown(void);

/// @}

#endif    /// __PDS_NSV_INIT_HPP__ */
