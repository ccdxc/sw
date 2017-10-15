#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "nic/include/asic_rw.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/utils/thread/thread.hpp"
#include "nic/hal/hal.hpp"
#include "hal_control.hpp"
#include "port.hpp"

namespace hal {

extern thread_local hal::utils::thread *t_curr_thread;

namespace pd {

// per producer request queues
hal_ctrl_queue_t g_hal_ctrl_workq[HAL_THREAD_ID_MAX];

//------------------------------------------------------------------------------
// hal-control thread's forever loop
//------------------------------------------------------------------------------
static void
hal_control_loop (void)
{
    uint32_t           qid;
    uint16_t           cindx;
    bool               work_done, rv;
    hal_ctrl_entry_t   *rw_entry;

    while (TRUE) {
        work_done = false;
        for (qid = 0; qid < HAL_THREAD_ID_MAX; qid++) {
            if (!g_hal_ctrl_workq[qid].nentries) {
                // no read/write requests
                continue;
            }

            // found a read/write request to serve
            cindx = g_hal_ctrl_workq[qid].cindx;
            rw_entry = &g_hal_ctrl_workq[qid].entries[cindx];
            switch (rw_entry->opn) {
                case HAL_CONTROL_OPERATION_PORT:
                    port::port_event_notify(rw_entry->data);
                    break;

                default:
                    HAL_TRACE_ERR("Invalid operation {}", rw_entry->opn);
                    rv = false;
                    break;
            }

            // populate the results
            rw_entry->status =  rv ? HAL_RET_OK : HAL_RET_ERR;
            rw_entry->done = TRUE;

            // advance to next entry in the queue
            g_hal_ctrl_workq[qid].cindx++;
            if (g_hal_ctrl_workq[qid].cindx >= HAL_CONTROL_Q_SIZE) {
                g_hal_ctrl_workq[qid].cindx = 0;
            }
            g_hal_ctrl_workq[qid].nentries--;
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
// attempt to connect to ASIC model in sim mode
//------------------------------------------------------------------------------
static hal_ret_t
asic_sim_connect (hal_cfg_t *hal_cfg)
{
    int    rc;

    HAL_TRACE_DEBUG("Connecting to ASIC SIM");
    if ((rc = lib_model_connect()) == -1) {
        HAL_TRACE_ERR("Failed to connect to ASIC. Return code: {}", rc);
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

void
hal_control_asic_init(hal_cfg_t *hal_cfg)
{
    asic_cfg_t asic_cfg;
    hal_ret_t ret;

    if (hal_cfg->sim) {
        do {
            ret = asic_sim_connect(hal_cfg);
            if (ret == HAL_RET_OK) {
                HAL_TRACE_DEBUG("Connected to the ASIC model...");
                break;
            }
            HAL_TRACE_WARN("Failed to connect to asic, retrying in 1 sec ...");
            sleep(1);
        } while (ret != HAL_RET_OK);
    }

    // do asic initialization
    asic_cfg.loader_info_file = hal_cfg->loader_info_file;
    HAL_ABORT(asic_init(&asic_cfg) == HAL_RET_OK);
    return;
}

//------------------------------------------------------------------------------
// hal-control thread's entry point
//------------------------------------------------------------------------------
void*
hal_control_start (void *ctxt)
{
    HAL_TRACE_DEBUG("Thread {} initializing ...", t_curr_thread->name());

    hal_cfg_t    *hal_cfg = (hal_cfg_t *)ctxt;
    if (hal_cfg == NULL) {
        return NULL;
    }

    HAL_THREAD_INIT();

    // asic init code
    hal_control_asic_init(hal_cfg);

    // start the asic-rw thread
    HAL_TRACE_DEBUG("Starting asic-rw thread ...");
    g_hal_threads[HAL_THREAD_ID_ASIC_RW] =
        thread::factory(std::string("asic-rw").c_str(),
                HAL_THREAD_ID_ASIC_RW, HAL_CONTROL_CORE_ID,
                hal::pd::asic_rw_start,
                sched_get_priority_max(SCHED_RR), SCHED_RR, true);
    HAL_ABORT(g_hal_threads[HAL_THREAD_ID_ASIC_RW] != NULL);
    g_hal_threads[HAL_THREAD_ID_ASIC_RW]->start(hal_cfg);

    // keep polling the queue and serve requests
    hal_control_loop();

    HAL_TRACE_DEBUG("hal-control thread done");

    return NULL;
}

}    // namespace pd
}    // namespace hal
