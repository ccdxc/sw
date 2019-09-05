/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "logger.h"
#include "platform/sysmon/sysmon.h"
#include "lib/thread/thread.hpp"
#include "lib/periodic/periodic.hpp"

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
    sysmond_monitor();
    sysmond_flush_logger();
    return SDK_RET_OK;
}

int
main(int argc, char *argv[])
{
    void *res = NULL;
    int rv = 0;
    int thread_id = 0;

    //initialize the logger
    initializeLogger();

    // register for SDK logger
    sdk::lib::logger::init(sysmond_logger, sysmond_obfl_logger);

    // init the lib
    sysmond_init();

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

    // wait for the periodic thread
    thread->wait_until_complete();
    return 0;
}
