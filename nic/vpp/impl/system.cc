//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Capri system related info interfaces

#include <math.h>
#include "system.h"
#include <platform/capri/capri_tm_rw.hpp>
#include <asic/pd/pd.hpp>

#define TIME_NSECS_PER_SEC  1000000000ULL

static double g_clock_adjustment = 0;

uint64_t
pds_system_get_ticks (uint64_t sec)
{
    uint32_t freq;

    if (!g_clock_adjustment) {
        // freq in MHz
        freq = sdk::asic::pd::asic_pd_clock_freq_get();
        g_clock_adjustment =
                 ((double)(((double)1)/(freq * 1000000)));
    }
    return ((uint64_t) (((double)sec)/g_clock_adjustment));
}

uint64_t
pds_system_get_current_tick (void)
{
    uint64_t tick;
    sdk_ret_t ret;

    ret = sdk::platform::capri::capri_tm_get_clock_tick(&tick);
    SDK_ASSERT(ret == sdk::SDK_RET_OK);
    return tick;
}
