//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"

namespace nicmgr {

// below functions are called from nicmgr thread context
sdk_ret_t
nicmgr_upg_graceful_init (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
nicmgr_upg_hitless_init (void)
{
    return SDK_RET_OK;
}

}   // namespace nicmgr

// below functions are called from api thread context
namespace api {

sdk_ret_t
nicmgr_upg_graceful_init (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
nicmgr_upg_hitless_init (void)
{
    return SDK_RET_OK;
}

}    // namespace api
