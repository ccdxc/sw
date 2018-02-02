// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <sdk/lock.hpp>
#include <sdk/thread.hpp>
#include <sdk/timerfd.hpp>
#include <sdk/linkmgr.hpp>
#include "linkmgr_internal.hpp"
#include "linkmgr_state.hpp"
#include "linkmgr_periodic.hpp"
#include "port.hpp"

namespace sdk {
namespace linkmgr {

linkmgr_state     *g_linkmgr_state;
linkmgr_cfg_t     g_linkmgr_cfg;
sdk::lib::thread  *g_linkmgr_threads[LINKMGR_THREAD_ID_MAX];

// per producer request queues
linkmgr_queue_t   g_linkmgr_workq[LINKMGR_THREAD_ID_MAX];

sdk::lib::thread *
current_thread (void)
{
    return sdk::lib::thread::current_thread() ?
                       sdk::lib::thread::current_thread() :
                       g_linkmgr_threads[LINKMGR_THREAD_ID_CFG];
}

bool
is_linkmgr_ctrl_thread()
{
    sdk::lib::thread *curr_thread = current_thread();
    sdk::lib::thread *ctrl_thread = g_linkmgr_threads[LINKMGR_THREAD_ID_CTRL];

    // if curr_thread/ctrl_thread is NULL, then init has failed or not invoked
    if (curr_thread == NULL || ctrl_thread == NULL) {
        assert(0);
    }

    // if ctrl_thread is not running, then linkmgr_event_wait hasn't been invoked
    // and no one is waiting to handle incoming msgs
    if (!ctrl_thread->is_running()) {
        return true;
    }

    if (curr_thread->thread_id() == ctrl_thread->thread_id()) {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// linkmgr thread notification by other threads
//------------------------------------------------------------------------------
sdk_ret_t
linkmgr_notify (uint8_t operation, void *ctxt)
{
    uint16_t            pindx;
    sdk::lib::thread    *curr_thread = current_thread();
    uint32_t            curr_tid = curr_thread->thread_id();
    linkmgr_entry_t     *rw_entry;

    if (g_linkmgr_workq[curr_tid].nentries >= LINKMGR_CONTROL_Q_SIZE) {
        SDK_TRACE_ERR("Error: operation {} for thread {}, tid {} full",
                      operation, curr_thread->name(), curr_tid);
        return SDK_RET_ERR;
    }
    pindx = g_linkmgr_workq[curr_tid].pindx;

    rw_entry = &g_linkmgr_workq[curr_tid].entries[pindx];
    rw_entry->opn = operation;
    rw_entry->status = SDK_RET_ERR;
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

void
linkmgr_event_wait (void)
{
    int thread_id = LINKMGR_THREAD_ID_CTRL;
    g_linkmgr_threads[thread_id]->start(g_linkmgr_threads[thread_id]);
}

//------------------------------------------------------------------------------
// linkmgr's forever loop
//------------------------------------------------------------------------------
void*
linkmgr_event_loop (void* ctxt)
{
    uint32_t         qid         = 0;
    uint16_t         cindx       = 0;
    bool             work_done   = false;
    bool             rv          = true;
    linkmgr_entry_t  *rw_entry   = NULL;

    SDK_THREAD_INIT(ctxt);

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
                SDK_TRACE_ERR("Invalid operation {}", rw_entry->opn);
                rv = false;
                break;
            }

            SDK_TRACE_ERR("%s: invoked control thread. opn %d\n",
                            __FUNCTION__, rw_entry->opn);

            // populate the results
            rw_entry->status =  rv ? SDK_RET_OK : SDK_RET_ERR;
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

static sdk_ret_t
thread_init (void)
{
    int    thread_prio = 0, thread_id = 0;

    thread_prio = sched_get_priority_max(SCHED_OTHER);
    if (thread_prio < 0) {
        return SDK_RET_ERR;
    }

    if (linkmgr_timer_init() != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to init timer");
        return SDK_RET_ERR;
    }

    // spawn periodic thread that does background tasks
    thread_id = LINKMGR_THREAD_ID_PERIODIC;
    g_linkmgr_threads[thread_id] =
        sdk::lib::thread::factory(
                        std::string("linkmgr-periodic").c_str(),
                        thread_id,
                        CONTROL_CORE_ID,
                        linkmgr_periodic_thread_start,
                        thread_prio - 1,
                        SCHED_OTHER,
                        true);
    if (g_linkmgr_threads[thread_id] == NULL) {
        SDK_TRACE_ERR("%s: Failed to create linkmgr periodic thread",
                      __FUNCTION__);
        return SDK_RET_ERR;
    }

    // start the periodic thread
    g_linkmgr_threads[thread_id]->start(g_linkmgr_threads[thread_id]);

    // init the control thread
    thread_id = LINKMGR_THREAD_ID_CTRL;
    g_linkmgr_threads[thread_id] =
        sdk::lib::thread::factory(std::string("linkmgr-ctrl").c_str(),
                                  thread_id,
                                  CONTROL_CORE_ID,
                                  linkmgr_event_loop,
                                  thread_prio -1,
                                  SCHED_OTHER,
                                  true);

    // create a thread object for CFG thread
    thread_id = LINKMGR_THREAD_ID_CFG;
    g_linkmgr_threads[thread_id] =
        sdk::lib::thread::factory(std::string("linkmgr-cfg").c_str(),
                                  thread_id,
                                  CONTROL_CORE_ID,
                                  sdk::lib::thread::dummy_entry_func,
                                  thread_prio -1,
                                  SCHED_OTHER,
                                  true);
    g_linkmgr_threads[thread_id]->set_data(g_linkmgr_threads[thread_id]);
    g_linkmgr_threads[thread_id]->set_pthread_id(pthread_self());
    g_linkmgr_threads[thread_id]->set_running(true);

    return SDK_RET_OK;
}

static void
linkmgr_workq_init(void)
{
    uint32_t qid = 0;
    for (qid = 0; qid < LINKMGR_THREAD_ID_MAX; qid++) {
        g_linkmgr_workq[qid].nentries = 0;
    }
}

sdk_ret_t
linkmgr_init (linkmgr_cfg_t *cfg)
{
    sdk_ret_t    ret = SDK_RET_OK;

    linkmgr_workq_init();

    if ((ret = thread_init()) != SDK_RET_OK) {
        SDK_TRACE_ERR("%s: linkmgr thread init failed", __FUNCTION__);
        return ret;
    }

    g_linkmgr_state = linkmgr_state::factory();
    if (NULL == g_linkmgr_state) {
        SDK_TRACE_ERR("%s: linkmgr init failed", __FUNCTION__);
        return SDK_RET_ERR;
    }

    // initialize the port mac and serdes functions
    port::port_init(cfg);

    g_linkmgr_cfg = *cfg;

    return SDK_RET_OK;
}

sdk_ret_t
port_event_timer (void *ctxt)
{
    port *port_p = (port *)ctxt;
    return port_p->port_link_sm_process();
}

sdk_ret_t
port_event_enable (void *ctxt)
{
    port *port_p = (port *)ctxt;
    return port_p->port_enable();
}

sdk_ret_t
port_event_disable (void *ctxt)
{
    port *port_p = (port *)ctxt;
    return port_p->port_disable();
}

//-----------------------------------------------------------------------------
// PD If Create
//-----------------------------------------------------------------------------
void *
port_create (port_args_t *args)
{
    sdk_ret_t    ret = SDK_RET_OK;
    port         *port_p = NULL;

    port_p = (port *)g_linkmgr_state->port_slab()->alloc();
    port_p->set_port_type(args->port_type);
    port_p->set_port_speed(args->port_speed);
    port_p->set_mac_id(args->mac_id);
    port_p->set_mac_ch(args->mac_ch);
    port_p->set_num_lanes(args->num_lanes);

    for (uint32_t i = 0; i < args->num_lanes; ++i) {
        port_p->sbus_addr_set(i, args->sbus_addr[i]);
    }

    // if admin up is set, enable the port, else disable the port
    if (args->admin_state == port_admin_state_t::PORT_ADMIN_STATE_UP) {
        ret = port::port_enable(port_p);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("%s: port enable failed", __FUNCTION__);
        }
    } else {
        ret = port::port_disable(port_p);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("%s: port disable failed", __FUNCTION__);
        }
    }

    return port_p;
}

//-----------------------------------------------------------------------------
// update given port
//-----------------------------------------------------------------------------
sdk_ret_t
port_update (void *pd_p, port_args_t *args)
{
    sdk_ret_t          ret           = SDK_RET_OK;
    bool               configured    = false;
    port               *port_p       = (port *)pd_p;
    port_admin_state_t prev_admin_st = port_p->admin_state();

    SDK_TRACE_DEBUG("%s: port update", __FUNCTION__);

    if (args->port_speed != port_speed_t::PORT_SPEED_NONE) {
        port_p->set_port_speed(args->port_speed);
        configured = true;
    }

    // Disable the port if any config has changed
    if (configured == true) {
        ret = port::port_disable(port_p);
    }

    // Enable the port if -
    //      admin-up state is set in request msg OR
    //      admin state is not set in request msg, but port was admin up
    switch(args->admin_state) {
    case port_admin_state_t::PORT_ADMIN_STATE_NONE:
        if (prev_admin_st == port_admin_state_t::PORT_ADMIN_STATE_UP) {
            ret = port::port_enable(port_p);
        }
        break;

    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        ret = port::port_disable(port_p);
        break;

    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        ret = port::port_enable(port_p);
        break;

    default:
        break;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// delete given port by disabling the port and then deleting the port instance
//-----------------------------------------------------------------------------
sdk_ret_t
port_delete (void *pd_p)
{
    sdk_ret_t    ret = SDK_RET_OK;
    port         *port_p = (port *)pd_p;

    SDK_TRACE_DEBUG("%s: port delete", __FUNCTION__);
    ret = port::port_disable(port_p);
    g_linkmgr_state->port_slab()->free(port_p);

    return ret;
}

//-----------------------------------------------------------------------------
// PD Port get
//-----------------------------------------------------------------------------
sdk_ret_t
port_get (void *pd_p, port_args_t *args)
{
    sdk_ret_t            ret = SDK_RET_OK;
    port    *port_p = (port *)pd_p;

    SDK_TRACE_DEBUG("%s: port get", __FUNCTION__);
    args->port_type   = port_p->port_type();
    args->port_speed  = port_p->port_speed();
    args->admin_state = port_p->admin_state();
    args->mac_id      = port_p->mac_id();
    args->mac_ch      = port_p->mac_ch();
    args->num_lanes   = port_p->num_lanes();
    args->oper_status = port_p->oper_status();

    return ret;
}

#if 0
void *
port_make_clone (void *pd_orig_p)
{
    port  *port_p;
    port  *pd_new_clone_p;

    SDK_TRACE_DEBUG("%s: port clone", __FUNCTION__);
    port_p = (port *)pd_orig_p;
    pd_new_clone_p =
        (port *)g_linkmgr_state->port_slab()->alloc();
    // populate cloned pd instance from existing pd instance
    pd_new_clone_p->set_oper_status(port_p->oper_status());
    pd_new_clone_p->set_port_speed(port_p->port_speed());
    pd_new_clone_p->set_port_type(port_p->port_type());
    pd_new_clone_p->set_admin_state(port_p->admin_state());

    pd_new_clone_p->set_port_link_sm(port_p->port_link_sm());
    pd_new_clone_p->set_link_bring_up_timer(port_p->link_bring_up_timer());

    pd_new_clone_p->set_mac_id(port_p->mac_id());
    pd_new_clone_p->set_mac_ch(port_p->mac_ch());
    pd_new_clone_p->set_num_lanes(port_p->num_lanes());

    return pd_new_clone_p;
}
#endif

//-----------------------------------------------------------------------------
// PD Port mem free
//-----------------------------------------------------------------------------
sdk_ret_t
port_mem_free (port_args_t *args)
{
    sdk_ret_t    ret = SDK_RET_OK;
    port         *port_p = (port *)args->port_p;

    SDK_TRACE_DEBUG("%s: port mem_free", __FUNCTION__);
    g_linkmgr_state->port_slab()->free(port_p);

    return ret;
}

bool
port_has_speed_changed (port_args_t *args)
{
    port *port_p = (port *)args->port_p;
    return (args->port_speed != port_p->port_speed());
}

bool
port_has_admin_state_changed (port_args_t *args)
{
    port *port_p = (port *)args->port_p;
    return (args->admin_state != port_p->admin_state());
}

}    // namespace linkmgr
}    // namespace sdk
