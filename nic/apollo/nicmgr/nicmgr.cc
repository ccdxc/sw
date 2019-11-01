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
//#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/core.hpp"

/// \defgroup PDS_NICMGR
/// @{

pciemgr *pciemgr;
DeviceManager *devmgr;
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
    fwd_mode_t fwd_mode = sdk::platform::FWD_MODE_CLASSIC;

    // opting for graceful termination as fd wait used by evpoll
    // is part of pthread_cancel list
    SDK_THREAD_DFRD_TERM_INIT(ctxt);

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

    // initialize pciemgr
    if (platform_is_hw(state->platform_type())) {
        PDS_TRACE_INFO("initializing pciemgr");
        pciemgr = new class pciemgr("nicmgrd",
                                    curr_thread->ev_loop());
        pciemgr->initialize();
    }


    PDS_TRACE_INFO("Initializing device manager ...");
    devmgr = new DeviceManager(config_file, fwd_mode, state->platform_type(),
                               curr_thread->ev_loop());
    devmgr->LoadConfig(config_file);

    if (pciemgr) {
        pciemgr->finalize();
    }

    // creating mnets
    PDS_TRACE_INFO("Creating mnets ...");
    devmgr->HalEventHandler(true);

    sdk::event_thread::subscribe(EVENT_ID_PORT, port_event_handler_);
    sdk::event_thread::subscribe(EVENT_ID_XCVR, xcvr_event_handler_);
    sdk::event_thread::prepare_init(&g_ev_prepare, prepare_callback, NULL);
    sdk::event_thread::prepare_start(&g_ev_prepare);

    PDS_TRACE_INFO("Listening to events ...");
}

//------------------------------------------------------------------------------
// nicmgr thread cleanup
//------------------------------------------------------------------------------
void
nicmgrapi::nicmgr_thread_exit(void *ctxt) {
    delete devmgr;
    if (pciemgr) {
        delete pciemgr;
    }
    sdk::event_thread::prepare_stop(&g_ev_prepare);
}

void
nicmgrapi::nicmgr_event_handler(void *msg, void *ctxt) {
}

void
nicmgrapi::port_event_handler_(void *data, size_t data_len, void *ctxt) {
    port_status_t st = { 0 };
    core::event_t *event = (core::event_t *)data;

    st.id = event->port.ifindex;
    st.status =
        (event->port.event == port_event_t::PORT_EVENT_LINK_UP) ? 1 : 0;
    st.speed = sdk::lib::port_speed_enum_to_mbps(event->port.speed);
    PDS_TRACE_DEBUG("Rcvd port event for ifidx 0x%x, speed %u, status %u",
                    st.id, st.speed, st.status);
    devmgr->LinkEventHandler(&st);
}

void
nicmgrapi::xcvr_event_handler_(void *data, size_t data_len, void *ctxt) {
    port_status_t st = { 0 };
    core::event_t *event = (core::event_t *)data;

    st.id = event->port.ifindex;
    st.xcvr.state = event->xcvr.state;
    st.xcvr.pid = event->xcvr.pid;
    st.xcvr.phy = event->xcvr.cable_type;
    memcpy(st.xcvr.sprom, event->xcvr.sprom, XCVR_SPROM_SIZE);
    devmgr->XcvrEventHandler(&st);
    PDS_TRACE_DEBUG("Rcvd xcvr event for ifidx 0x%x, state %u, cable type %u"
                    "pid %u", st.id, st.xcvr.state, st.xcvr.phy, st.xcvr.pid);
    devmgr->LinkEventHandler(&st);
}

}    // namespace nicmgr

/// \@}

