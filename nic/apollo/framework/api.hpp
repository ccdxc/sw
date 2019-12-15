//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines basic types for API processing
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_HPP__
#define __FRAMEWORK_API_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/framework/api.h"

namespace api {

// forward declarations
typedef struct api_ctxt_s api_ctxt_t;
typedef struct api_obj_ctxt_s api_obj_ctxt_t;
typedef union api_params_u api_params_t;

/// \brief    return current epoch value
pds_epoch_t get_current_epoch(void);

}    // namespace api

using api::api_ctxt_t;
using api::api_obj_ctxt_t;
using api::api_params_t;

#endif    // __FRAMEWORK_API_HPP__
