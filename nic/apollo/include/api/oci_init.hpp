//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines init and teardown API
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_OCI_INIT_HPP__
#define __INCLUDE_API_OCI_INIT_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/include/api/oci.hpp"

/// \defgroup OCI_INIT Initialization and teardown API
/// @{

/// \brief Initialization modes
typedef enum oci_init_mode_e {
    OCI_INIT_MODE_NONE,            ///< Invalid mode
    OCI_INIT_MODE_COLD_START,      ///< Initialize from scratch
                                   ///< ignore any state if preserved previously
    OCI_INIT_MODE_POST_UPGRADE,    ///< Initialize using state preserved, if any
} oci_init_mode_t;

/// \brief Initialization parameters
typedef struct oci_init_params_s {
    oci_init_mode_t           init_mode;   ///< Mode of initialization
    sdk_logger::trace_cb_t    trace_cb;    ///< Callback for trace msgs
    std::string               pipeline;    ///< P4 program pipeline name
                                           ///< only "apollo" supported now
    std::string               cfg_file;    ///< Config files directory path
                                           ///< All config files are present,
                                           ///< files under <cfg_path>/pipeline/
                                           ///< are picked by the software
} oci_init_params_t;

/// \brief Initialize routine
///
/// \param[in] params Init time parameters
///
/// \eturn #SDK_RET_OK on success, failure status code on error
sdk::sdk_ret_t oci_init(oci_init_params_t *params);

/// \brief Teardown routine
///
/// \eturn #SDK_RET_OK on success, failure status code on error
sdk::sdk_ret_t oci_init(oci_init_params_t *params);

/// \@}

#endif    // __INCLUDE_API_OCI_INIT_HPP__
