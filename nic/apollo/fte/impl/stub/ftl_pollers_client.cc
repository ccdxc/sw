//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains stub APIs for Flow Table Library software pollers client
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/athena/ftl_pollers_client.hpp"

namespace ftl_pollers_client {

volatile uint8_t    user_will_poll_;

sdk_ret_t
init(void)
{
    return SDK_RET_OK;
}

uint32_t
qcount_get(void)
{
    return 0;
}

sdk_ret_t
expiry_fn_dflt(pds_flow_expiry_fn_t *ret_fn_dflt)
{
    *ret_fn_dflt = NULL;
    return SDK_RET_OK;
}

sdk_ret_t
poll_control(bool user_will_poll,
             pds_flow_expiry_fn_t expiry_fn)
{
    return SDK_RET_OK;
}

sdk_ret_t
force_session_expired_ts_set(bool force_expired_ts)
{
    return SDK_RET_OK;
}

sdk_ret_t
force_conntrack_expired_ts_set(bool force_expired_ts)
{
    return SDK_RET_OK;
}

sdk_ret_t
poll(uint32_t qid,
     bool debug_log)
{
    return SDK_RET_OK;
}

} // namespace ftl_pollers_client

