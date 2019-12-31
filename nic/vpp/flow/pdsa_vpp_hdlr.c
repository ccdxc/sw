//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include <arpa/inet.h>
#include "node.h"

void
pds_flow_idle_timeout_get (uint32_t *flow_idle_timeout, size_t sz)
{
    pds_flow_main_t *fm = &pds_flow_main;

    clib_memcpy(flow_idle_timeout, fm->flow_idle_timeout, sz);
}

void
pds_flow_idle_timeout_set (const uint32_t *flow_idle_timeout, size_t sz)
{
    pds_flow_main_t *fm = &pds_flow_main;

    clib_memcpy(fm->flow_idle_timeout, flow_idle_timeout, sz);
}

void
pds_flow_cfg_init (void)
{
    uint32_t flow_idle_timeouts[IPPROTO_MAX];

    memset(flow_idle_timeouts, 0, sizeof(flow_idle_timeouts));

    // initialize flow config to not age
    pds_flow_idle_timeout_set(flow_idle_timeouts, sizeof(flow_idle_timeouts));
}
