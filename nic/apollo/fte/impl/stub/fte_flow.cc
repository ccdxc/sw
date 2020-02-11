//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains stub APIs for FTE flow functionality
///
//----------------------------------------------------------------------------
#include <stdint.h>
#include <rte_mbuf.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/fte/fte.hpp"

using sdk::sdk_ret_t;

namespace fte {

void
fte_flow_prog (struct rte_mbuf *m)
{
    return;    
}

sdk_ret_t
fte_ftl_init (void)
{
    return SDK_RET_OK;
}

void
fte_ftl_set_core_id (unsigned int)
{
    return;
}

} // namespace fte
