/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "lib/thread/thread.hpp"
#include "lib/periodic/periodic.hpp"
#include "lib/event_thread/event_thread.hpp"
#include "platform/sysmon/sysmon.hpp"
#include "platform/asicerror/interrupts.hpp"
#include "platform/capri/csrint/csr_init.hpp"
#include "platform/evutils/include/evutils.h"
#include "logger.h"
#include "delphi/sysmond_delphi.hpp"
#include "sysmond_cb.hpp"
#include "cmd.hpp"

#define SYSMOND_TIMER_ID_POLL 1
#define ASICERROR_TIMER_ID_POLL 2
#define SYSMOND_POLL_TIME     10000 // 10 secs = 10 * 1000 msecs
#define ASICERROR_POLL_TIME   1000  // 1 sec   = 1  * 1000 msecs
#define THREAD_ID_AGENT_CMD_SERVER 1

static sdk::event_thread::event_thread *g_cmd_server_thread;

void
handle_cmd (cmd_ctxt_t *cmd_ctxt) {
    int fd = cmd_ctxt->fd;

    switch (cmd_ctxt->cmd) {
    case sysmon::CLI_CMD_CLEAR_INTR:
        clear_interrupts();
        break;

    default:
        dprintf(fd, "%-50s%-10s%-9s%-s\n",
                "Name", "Count", "Severity", "Description");
        walk_interrupts(intr_dump_cb, &fd);
        break;
    }
}

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

static sdk_ret_t
asicerror_timer_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    traverse_interrupts();
    sysmond_flush_logger();
    return SDK_RET_OK;
}

static void
sig_handler (int sig, siginfo_t *info, void *ptr)
{
}

static void
siginit (void)
{
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGPIPE, &act, NULL);
    return;
}

sdk_ret_t
spawn_cmd_server_thread (void)
{
    // spawn periodic thread that does background tasks
    g_cmd_server_thread =
        sdk::event_thread::event_thread::factory(
            "cfg", THREAD_ID_AGENT_CMD_SERVER, sdk::lib::THREAD_ROLE_CONTROL,
            0x0, sysmon::cmd_server_thread_init, sysmon::cmd_server_thread_exit,
            NULL, // message
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);

    SDK_ASSERT_TRACE_RETURN((g_cmd_server_thread != NULL), SDK_RET_ERR,
                            "Command server thread create failure");
    g_cmd_server_thread->start(g_cmd_server_thread);

    return SDK_RET_OK;
}

int
main(int argc, char *argv[])
{
    void *res = NULL;
    int rv = 0;
    int thread_id = 0;
    sysmon_cfg_t sysmon_cfg;
    intr_cfg_t intr_cfg;
    sdk_ret_t ret;

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
    sysmon_cfg.panic_event_cb = panic_event_cb;
    sysmon_cfg.catalog = catalog;

    // init the lib
    sysmon_init(&sysmon_cfg);

    intr_cfg.intr_event_cb = intr_event_cb;
    intr_init(&intr_cfg);

    // set the CPU cores for control threads
    sdk::lib::thread::control_cores_mask_set(0xf);

    // spawn command server thread
    ret = spawn_cmd_server_thread();
    if (ret != SDK_RET_OK) {
    }

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

    // schedule the sysmon timer cb
    sdk::lib::timer_schedule(
                    SYSMOND_TIMER_ID_POLL, SYSMOND_POLL_TIME, NULL,
                    (sdk::lib::twheel_cb_t)sysmond_timer_cb,
                    true);

    // schedule the asicerror timer cb
    sdk::lib::timer_schedule(
                    ASICERROR_TIMER_ID_POLL, ASICERROR_POLL_TIME, NULL,
                    (sdk::lib::twheel_cb_t)asicerror_timer_cb,
                    true);

    siginit();

    // event loop needed for callbacks
    evutil_run(EV_DEFAULT);
    return 0;
}
