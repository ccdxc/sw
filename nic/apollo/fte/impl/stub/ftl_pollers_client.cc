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
poll(uint32_t qid,
     expiry_user_cb_t expiry_user_cb,
     bool bringup_log)
{
    return SDK_RET_OK;
}

} // namespace ftl_pollers_client

