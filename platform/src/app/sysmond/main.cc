/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "lib/thread/thread.hpp"
#include "lib/periodic/periodic.hpp"
#include "platform/sysmon/sysmon.hpp"
#include "platform/capri/csrint/csr_init.hpp"
#include "platform/evutils/include/evutils.h"
#include "logger.h"
#include "delphi/sysmond_delphi.hpp"
#include "delphi/sysmond_cb.hpp"

#define SYSMOND_TIMER_ID_POLL 1
#define SYSMOND_POLL_TIME     10000 // 10 secs = 10 * 1000 msecs

//------------------------------------------------------------------------------
// starting point for the periodic thread loop
//------------------------------------------------------------------------------
static void *
periodic_thread_start (void *ctxt)
{
    // initialize timer wheel
    sdk::lib::periodic_thread_init(ctxt);

    // run main loop
    sdk::lib::periodic_thread_run(ctxt);
    return NULL;
}

static sdk_ret_t
sysmond_timer_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    sysmon_monitor();
    sysmond_flush_logger();
    return SDK_RET_OK;
}

int
main(int argc, char *argv[])
{
    void *res = NULL;
    int rv = 0;
    int thread_id = 0;
    sysmon_cfg_t sysmon_cfg;

    //initialize the logger
    initializeLogger();

    // register for SDK logger
    sdk::lib::logger::init(sysmond_logger, sysmond_obfl_logger);

    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW) == sdk::lib::PAL_RET_OK);
#endif

    // event cb init
    event_cb_init();

    sdk::platform::capri::csr_init();

    sdk::lib::catalog *catalog = sdk::lib::catalog::factory();

    sysmon_cfg.frequency_change_event_cb = frequency_change_event_cb;
    sysmon_cfg.cattrip_event_cb = cattrip_event_cb;
    sysmon_cfg.power_event_cb = power_event_cb;
    sysmon_cfg.temp_event_cb = temp_event_cb;
    sysmon_cfg.memory_event_cb = memory_event_cb;
    sysmon_cfg.catalog = catalog;

    // init the lib
    sysmon_init(&sysmon_cfg);

    // set the CPU cores for control threads
    sdk::lib::thread::control_cores_mask_set(0xf);

    // create the periodic timer
    sdk::lib::thread *thread = sdk::lib::thread::factory(
                              "sysmon-timer",
                              thread_id,
                              sdk::lib::THREAD_ROLE_CONTROL,
                              0x0 /* use all control cores */,
                              periodic_thread_start,
                              sdk::lib::thread::priority_by_role(
                                                sdk::lib::THREAD_ROLE_CONTROL),
                              sdk::lib::thread::sched_policy_by_role(
                                                sdk::lib::THREAD_ROLE_CONTROL),
                              true);
    SDK_ASSERT_TRACE_RETURN((thread != NULL), SDK_RET_ERR,
                             "periodic thread create failure");

    // start the periodic thread
    thread->start(thread);

    // wait until the periodic thread is ready
    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }

    // schedule the periodic timer cb
    sdk::lib::timer_schedule(
                    SYSMOND_TIMER_ID_POLL, SYSMOND_POLL_TIME, NULL,
                    (sdk::lib::twheel_cb_t)sysmond_timer_cb,
                    true);

    // event loop needed for callbacks
    evutil_run(EV_DEFAULT);
    return 0;
}
