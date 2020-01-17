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
#include "nic/apollo/fte/fte.hpp"

namespace fte {

void
fte_flow_prog (struct rte_mbuf *m)
{
    return;    
}

void
fte_ftl_init (unsigned int lcore_id)
{
    return;
}

} // namespace fte
