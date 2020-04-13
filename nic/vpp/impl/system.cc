//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Capri system related info interfaces

#include <math.h>
#include "system.h"
#include <asic/pd/pd.hpp>

using namespace sdk::asic::pd;

#define TIME_NSECS_PER_SEC  1000000000ULL

static double g_clock_adjustment = 0;

uint64_t
pds_system_get_ticks (uint64_t sec)
{
    uint32_t freq;

    if (!g_clock_adjustment) {
        // freq in MHz
        freq = asicpd_clock_freq_get();
        g_clock_adjustment = ((double)(((double)1)/(freq * 1000000)));
    }

    return ((uint64_t) (((double)sec)/g_clock_adjustment));
}

double
pds_system_get_secs (uint64_t tick)
{
    return ((double) (((double)tick) * g_clock_adjustment));
}

uint64_t
pds_system_get_current_tick (void)
{
    uint64_t tick;
    sdk_ret_t ret;

    ret = asicpd_tm_get_clock_tick(&tick);
    SDK_ASSERT(ret == sdk::SDK_RET_OK);
    return tick;
}
