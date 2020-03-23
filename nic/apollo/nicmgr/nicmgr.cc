//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nicmgr functionality
///
//----------------------------------------------------------------------------

#include <pthread.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/utils/port_utils.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/core.hpp"

/// \defgroup PDS_NICMGR
/// @{

DeviceManager *g_devmgr;
sdk::event_thread::prepare_t g_ev_prepare;

namespace nicmgr {

static void
prepare_callback (sdk::event_thread::prepare_t *prepare, void *ctx)
{
    fflush(stdout);
    fflush(stderr);
    if (utils::logger::logger()) {
        utils::logger::logger()->flush();
    }
}

void
nicmgrapi::nicmgr_thread_init(void *ctxt) {
    pds_state *state;
    string config_file;
    sdk::event_thread::event_thread *curr_thread;
    devicemgr_cfg_t cfg;

    // get pds state
    state = (pds_state *)sdk::lib::thread::current_thread()->data();
    curr_thread = (sdk::event_thread::event_thread *)ctxt;

#ifdef __x86_64__
    config_file = state->cfg_path() + "/" + state->pipeline() + "/device.json";
#else
    config_file = state->cfg_path() + "/device.json";
#endif

    // instantiate the logger
    utils::logger::init();

    // initialize device manager
    PDS_TRACE_INFO("Initializing device manager ...");
    cfg.platform_type = state->platform_type();
    cfg.cfg_path = state->cfg_path();
    cfg.device_conf_file = "";
    cfg.fwd_mode = sdk::lib::FORWARDING_MODE_NONE;
    cfg.micro_seg_en = false;
    cfg.shm_mgr = NULL;
    cfg.EV_A = curr_thread->ev_loop();

    g_devmgr = new DeviceManager(&cfg);
    SDK_ASSERT(g_devmgr);
    g_devmgr->LoadProfile(config_file, true);

    sdk::ipc::subscribe(EVENT_ID_PORT_STATUS, port_event_handler_, NULL);
    sdk::ipc::subscribe(EVENT_ID_XCVR_STATUS, xcvr_event_handler_, NULL);
    sdk::ipc::subscribe(EVENT_ID_PDS_HAL_UP, hal_up_event_handler_, NULL);
    sdk::event_thread::prepare_init(&g_ev_prepare, prepare_callback, NULL);
    sdk::event_thread::prepare_start(&g_ev_prepare);

    PDS_TRACE_INFO("Listening to events ...");
}

//------------------------------------------------------------------------------
// nicmgr thread cleanup
//------------------------------------------------------------------------------
void
nicmgrapi::nicmgr_thread_exit(void *ctxt) {
    delete g_devmgr;
    sdk::event_thread::prepare_stop(&g_ev_prepare);
}

void
nicmgrapi::nicmgr_event_handler(void *msg, void *ctxt) {
}

void
nicmgrapi::hal_up_event_handler_(sdk::ipc::ipc_msg_ptr msg, const void *ctxt) {
    // create mnets
    PDS_TRACE_INFO("Creating mnets ...");
    g_devmgr->HalEventHandler(true);
}

void
nicmgrapi::port_event_handler_(sdk::ipc::ipc_msg_ptr msg, const void *ctxt) {
    port_status_t st = { 0 };
    core::event_t *event = (core::event_t *)msg->data();

    st.id = event->port.ifindex;
    st.status =
        (event->port.event == port_event_t::PORT_EVENT_LINK_UP) ? 1 : 0;
    st.speed = sdk::lib::port_speed_enum_to_mbps(event->port.speed);
    st.fec_type = (uint8_t)event->port.fec_type;
    PDS_TRACE_DEBUG("Rcvd port event for ifidx 0x%x, speed %u, status %u "
                    " fec_type %u", st.id, st.speed, st.status, st.fec_type);
    g_devmgr->LinkEventHandler(&st);
}

void
nicmgrapi::xcvr_event_handler_(sdk::ipc::ipc_msg_ptr msg, const void *ctxt) {
    port_status_t st = { 0 };
    core::event_t *event = (core::event_t *)msg->data();

    st.id = event->port.ifindex;
    st.xcvr.state = event->xcvr.state;
    st.xcvr.pid = event->xcvr.pid;
    st.xcvr.phy = event->xcvr.cable_type;
    memcpy(st.xcvr.sprom, event->xcvr.sprom, XCVR_SPROM_SIZE);
    g_devmgr->XcvrEventHandler(&st);
    PDS_TRACE_DEBUG("Rcvd xcvr event for ifidx 0x%x, state %u, cable type %u"
                    "pid %u", st.id, st.xcvr.state, st.xcvr.phy, st.xcvr.pid);
    g_devmgr->LinkEventHandler(&st);
}

DeviceManager *
nicmgrapi::devmgr_if(void) {
    return g_devmgr;
}

}    // namespace nicmgr

/// \@}

