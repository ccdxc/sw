//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines shared routines for all objects
///
//----------------------------------------------------------------------------

#ifndef __API_OBJ_API_HPP__
#define __API_OBJ_API_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/framework/api.hpp"

static inline sdk::sdk_ret_t
pds_obj_api_validate (api_op_t op, void *key, void *spec)
{
    if (op == API_OP_DELETE && key) {
        return sdk::SDK_RET_OK;
    }

    if ((op == API_OP_CREATE || op == API_OP_UPDATE) && spec) {
        return sdk::SDK_RET_OK;
    }

    return sdk::SDK_RET_INVALID_ARG;
}

#endif    // __API_OBJ_API_HPP__
