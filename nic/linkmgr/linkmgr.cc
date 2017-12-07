// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "linkmgr.hpp"
#include "linkmgr_src.hpp"

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

namespace linkmgr {

// per producer request queues
linkmgr_queue_t g_linkmgr_workq[LINKMGR_THREAD_ID_MAX];

//------------------------------------------------------------------------------
// linkmgr's forever loop
//------------------------------------------------------------------------------
static void
linkmgr_loop (void)
{
    uint32_t           qid;
    uint16_t           cindx;
    bool               work_done = false;
    bool               rv = true;
    linkmgr_entry_t   *rw_entry;

    while (TRUE) {
        work_done = false;
        for (qid = 0; qid < LINKMGR_THREAD_ID_MAX; qid++) {
            if (!g_linkmgr_workq[qid].nentries) {
                // no read/write requests
                continue;
            }

            // found a read/write request to serve
            cindx = g_linkmgr_workq[qid].cindx;
            rw_entry = &g_linkmgr_workq[qid].entries[cindx];
            switch (rw_entry->opn) {
                case LINKMGR_OPERATION_PORT_TIMER:
                    port_event_timer(rw_entry->data);
                    break;

                case LINKMGR_OPERATION_PORT_ENABLE:
                    port_event_enable(rw_entry->data);
                    break;

                case LINKMGR_OPERATION_PORT_DISABLE:
                    port_event_disable(rw_entry->data);
                    break;

                default:
                    HAL_TRACE_ERR("Invalid operation {}", rw_entry->opn);
                    rv = false;
                    break;
            }

            // populate the results
            rw_entry->status =  rv ? HAL_RET_OK : HAL_RET_ERR;
            rw_entry->done.store(true);

            // advance to next entry in the queue
            g_linkmgr_workq[qid].cindx++;
            if (g_linkmgr_workq[qid].cindx >= LINKMGR_CONTROL_Q_SIZE) {
                g_linkmgr_workq[qid].cindx = 0;
            }
            g_linkmgr_workq[qid].nentries--;
            work_done = true;
        }

        // all queues scanned once, check if any work was found
        if (!work_done) {
            // didn't find any work, yield and give chance to other threads
            pthread_yield();
        }
    }
}

//------------------------------------------------------------------------------
// linkmgr thread notification by other threads
//------------------------------------------------------------------------------
hal_ret_t
linkmgr_notify (uint8_t operation, void *ctxt)
{
    uint16_t           pindx;
    hal::utils::thread *curr_thread = current_thread();
    uint32_t           curr_tid = curr_thread->thread_id();
    linkmgr_entry_t   *rw_entry;

    if (g_linkmgr_workq[curr_tid].nentries >= LINKMGR_CONTROL_Q_SIZE) {
        HAL_TRACE_ERR("Error: operation {} for thread {}, tid {} full",
                      operation, curr_thread->name(), curr_tid);
        return HAL_RET_HW_PROG_ERR;
    }
    pindx = g_linkmgr_workq[curr_tid].pindx;

    rw_entry = &g_linkmgr_workq[curr_tid].entries[pindx];
    rw_entry->opn = operation;
    rw_entry->status = HAL_RET_ERR;
    rw_entry->data = ctxt;
    rw_entry->done.store(false);

    g_linkmgr_workq[curr_tid].nentries++;

    while (rw_entry->done.load() == false) {
        if (curr_thread->can_yield()) {
            pthread_yield();
        }
    }

    // move the producer index to next slot.
    // consumer is unaware of the blocking/non-blocking call and always
    // moves to the next slot.
    g_linkmgr_workq[curr_tid].pindx++;
    if (g_linkmgr_workq[curr_tid].pindx >= LINKMGR_CONTROL_Q_SIZE) {
        g_linkmgr_workq[curr_tid].pindx = 0;
    }

    return rw_entry->status;
}

}    // namespace linkmgr

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
static void
linkmgr_sig_handler (int sig, siginfo_t *info, void *ptr)
{
    HAL_TRACE_DEBUG("HAL received signal {}", sig);

    switch (sig) {
    case SIGINT:
        HAL_GCOV_FLUSH();
        hal::utils::hal_logger().flush();
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        hal::utils::hal_logger().flush();
        break;

    case SIGHUP:
    case SIGQUIT:
    case SIGCHLD:
    case SIGURG:
    case SIGTERM:
    default:
        hal::utils::hal_logger().flush();
        break;
    }
}

//------------------------------------------------------------------------------
// initialize all the signal handlers
// TODO: save old handlers and restore when signal happened
//------------------------------------------------------------------------------
static hal_ret_t
linkmgr_sig_init (void)
{
    struct sigaction    act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = linkmgr_sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGURG, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// linkmgr's entry point
//------------------------------------------------------------------------------
int
main (int argc, char **argv)
{
    linkmgr_sig_init();

    // initialize the port mac and serdes functions
    linkmgr::linkmgr_init();

    // keep polling the queue and serve requests
    linkmgr::linkmgr_loop();

    HAL_TRACE_DEBUG("linkmgr done");

    return 0;
}

