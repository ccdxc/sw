// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <sdk/lock.hpp>
#include <sdk/thread.hpp>
#include <sdk/timerfd.hpp>
#include <sdk/linkmgr.hpp>
#include "linkmgr_state.hpp"
#include "port.hpp"

#define LINKMGR_THREAD_ID_MAX        4

namespace sdk {
namespace linkmgr {

#define CONTROL_CORE_ID    0

enum {
    LINKMGR_THREAD_ID_PERIODIC,
}

linkmgr_state       *g_linkmgr_state;
linkmgr_cfg_t       g_linkmgr_cfg;
sdk::lib::thread    *g_linkmgr_threads[LINKMGR_THREAD_ID_MAX];

static void *
linkmgr_periodic_thread_start (void *ctxt)
{
    uint64_t            missed;
    sdk::lib::twheel    *twheel;
    timerfd_info_t      timerfd_info;

    SDK_THREAD_INIT(ctxt);

    // create a timer wheel
    twheel = sdk::lib::twheel::factory(TWHEEL_DEFAULT_SLICE_DURATION,
                                        TWHEEL_DEFAULT_DURATION, true);
    if (twheel == NULL) {
        SDK_TRACE_ERR("Periodic thread failed to create timer wheel");
        return NULL;
    }

    // prepare the timer fd(s)
    sdk::lib::timerfd_init(&timerfd_info);
    timerfd_info.usecs = TWHEEL_DEFAULT_SLICE_DURATION * TIME_USECS_PER_MSEC;
    if (sdk::lib::timerfd_prepare(&timerfd_info) < 0) {
        SDK_TRACE_ERR("Periodic thread failed to intiialize timerfd");
        return NULL;
    }


    // start the forever loop
    while (TRUE) {
        // wait for timer to fire
        if (sdk::lib::timerfd_wait(&timerfd_info, &missed) < 0) {
            SDK_TRACE_ERR("Periodic thread failed to wait on timer");
            break;
        }

        // drive the timer wheel if enough time elapsed
        twheel->tick(missed * TWHEEL_DEFAULT_SLICE_DURATION);
    }

    return NULL;
}

static sdk_ret_t
thread_init (void)
{
    int    rv, thread_prio, thread_id;

    // spawn periodic thread
    thread_prio = sched_get_priority_max(SCHED_FIFO);
    if (thread_prio < 0) {
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
                        thread_prio - 1, SCHED_RR, true);
    if (g_linkmgr_threads[thread_id] == NULL) {
        SDK_TRACE_ERR("Failed to create linkmgr periodic thread");
        return SDK_RET_ERR;
    }

    // start the thread
    g_linkmgr_threads[thread_id]->start(g_linkmgr_threads[thread_id]);

    return SDK_RET_OK;
}

sdk_ret_t
linkmgr_init (linkmgr_cfg_t *cfg)
{
    sdk_ret_t    ret;
    //int          rc  = 0;

    if ((ret = thread_init()) != SDK_RET_OK) {
        return ret;
    }

    g_linkmgr_state = linkmgr_state::factory();
    if (NULL == g_linkmgr_state) {
        SDK_TRACE_ERR("linkmgr init failed", __FUNCTION__);
        return SDK_RET_ERR;
    }

    // initialize the port mac and serdes functions
    port::port_init(cfg);

#if 0
    if (cfg->platform_type == platform_type_t::PLATFORM_TYPE_SIM) {
        do {
            rc = lib_model_connect();
            if (rc == -1) {
                SDK_TRACE_ERR("Failed to connect to asic, retrying in 1 sec ...");
                sleep(1);
            }
        } while (rc == -1);
    }
#endif
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
    port         *port_p;

    port_p = (port *)g_linkmgr_state->port_slab()->alloc();
    port_p->set_port_type(args->port_type);
    port_p->set_port_speed(args->port_speed);
    port_p->set_mac_id(args->mac_id);
    port_p->set_mac_ch(args->mac_ch);
    port_p->set_num_lanes(args->num_lanes);

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
port_update (port_args_t *args)
{
    sdk_ret_t    ret = SDK_RET_OK;
    port         *port_p = (port *)args->port_p;

    SDK_TRACE_DEBUG("%s: port update", __FUNCTION__);
    if (args->port_speed != port_speed_t::PORT_SPEED_NONE) {
        port_p->set_port_speed(args->port_speed);
    }

    if (args->admin_state == port_admin_state_t::PORT_ADMIN_STATE_UP) {
        ret = port::port_enable(port_p);
    } else if (args->admin_state == port_admin_state_t::PORT_ADMIN_STATE_DOWN) {
        ret = port::port_disable(port_p);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// delete given port by disabling the port and then deleting the port instance
//-----------------------------------------------------------------------------
sdk_ret_t
port_delete (port_args_t *args)
{
    sdk_ret_t    ret = SDK_RET_OK;
    port         *port_p = (port *)args->port_p;

    SDK_TRACE_DEBUG("%s: port delete", __FUNCTION__);
    ret = port::port_disable(port_p);
    g_linkmgr_state->port_slab()->free(port_p);

    return ret;
}

//-----------------------------------------------------------------------------
// PD Port get
//-----------------------------------------------------------------------------
sdk_ret_t
port_get (port_args_t *args)
{
    sdk_ret_t            ret = SDK_RET_OK;
    port    *port_p = (port *)args->port_p;

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
