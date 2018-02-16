// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "hal_control.hpp"
#include "nic/hal/hal.hpp"
#include "sdk/thread.hpp"
#include "nic/include/asic_pd.hpp"

namespace hal {

extern bool gl_super_user;

namespace pd {

//------------------------------------------------------------------------------
// hal-control thread's entry point
//------------------------------------------------------------------------------
void*
hal_control_start (void *ctxt)
{
    SDK_THREAD_INIT(ctxt);

    uint64_t cores_mask = 0x0;

    hal_cfg_t *hal_cfg =
                (hal_cfg_t *)hal::hal_get_current_thread()->data();
    if (hal_cfg == NULL) {
        return NULL;
    }

    // start the asic-rw thread
    HAL_TRACE_DEBUG("Starting asic-rw thread ...");
    g_hal_threads[HAL_THREAD_ID_ASIC_RW] =
        thread::factory(std::string("asic-rw").c_str(),
                HAL_THREAD_ID_ASIC_RW,
                sdk::lib::THREAD_ROLE_CONTROL,
                cores_mask,
                hal::pd::asic_rw_start,
                sched_get_priority_max(SCHED_RR),
                gl_super_user ? SCHED_RR : SCHED_OTHER,
                true);

    // set custom data
    g_hal_threads[HAL_THREAD_ID_ASIC_RW]->set_data(hal_cfg);

    // invoke with thread instance reference
    g_hal_threads[HAL_THREAD_ID_ASIC_RW]->start(
                            g_hal_threads[HAL_THREAD_ID_ASIC_RW]);

    HAL_TRACE_DEBUG("Waiting for asic-rw thread to be ready ...");
    // wait for ASIC RW thread to be ready
    while (!is_asic_rw_ready()) {
        pthread_yield();
    }

    HAL_TRACE_DEBUG("hal-control thread done");

    return NULL;
}

}    // namespace pd
}    // namespace hal
