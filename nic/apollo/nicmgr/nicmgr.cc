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
#include "include/sdk/base.hpp"
#include "nic/sdk/lib/utils/port_utils.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/core.hpp"

/// \defgroup PDS_NICMGR
/// @{

DeviceManager *devmgr;
pciemgr *pciemgr;

namespace nicmgr {

static void
log_flush(void *arg)
{
    fflush(stdout);
    fflush(stderr);
    if (utils::logger::logger()) {
        utils::logger::logger()->flush();
    }
}

void *
nicmgrapi::nicmgr_thread_start(void *ctxt) {
    evutil_check log_check, port_status_check;
    EV_P;
    fwd_mode_t fwd_mode = sdk::platform::FWD_MODE_CLASSIC;
    pds_state *state;
    string config_file;

    // opting for graceful termination as fd wait used by evpoll
    // is part of pthread_cancel list
    SDK_THREAD_DFRD_TERM_INIT(ctxt);
    EV_A = evutil_create_loop();

    // get pds state
    state = (pds_state *)sdk::lib::thread::current_thread()->data();

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
        pciemgr = new class pciemgr("nicmgrd", EV_A);
        pciemgr->initialize();
    }


    PDS_TRACE_INFO("Initializing device manager ...");
    devmgr = new DeviceManager(config_file, fwd_mode, state->platform_type(), EV_A);
    devmgr->LoadConfig(config_file);

    if (pciemgr) {
        pciemgr->finalize();
    }

    pthread_cleanup_push(nicmgr::nicmgrapi::nicmgr_thread_cleanup, NULL);
    // creating mnets
    PDS_TRACE_INFO("Creating mnets ...");
    devmgr->HalEventHandler(true);

    evutil_add_check(EV_A_ &log_check, &log_flush, NULL);

    // port status event handler
    evutil_add_check(EV_A_ &port_status_check,
                     &nicmgrapi::port_status_handler_, NULL);

    PDS_TRACE_INFO("Listening to events ...");
    evutil_run(EV_A);
    pthread_cleanup_pop(1);

    return NULL;
}

//------------------------------------------------------------------------------
// nicmgr thread cleanup
//------------------------------------------------------------------------------
void
nicmgrapi::nicmgr_thread_cleanup (void *arg) {
    delete devmgr;
    if (pciemgr) {
        delete pciemgr;
    }
}


void
nicmgrapi::port_status_handler_(void *ctxt) {
    port_status_t st = {0};
    core::event_t *event;

    event = core::event_dequeue(core::THREAD_ID_NICMGR);
    if (event == NULL) {
        return;
    }
    switch (event->event_id) {
    case EVENT_ID_PORT:
        st.speed = sdk::lib::port_speed_enum_to_mbps(event->port.speed);
        st.id = event->port.port_id;
        st.status =
            (event->port.event == port_event_t::PORT_EVENT_LINK_UP) ? 1 : 0;
        // st.xcvr =
        PDS_TRACE_DEBUG("Rcvd port event for ifidx 0x%x, speed %u, status %u",
                        st.id, st.speed, st.status);
        devmgr->LinkEventHandler(&st);
        break;

    default:
        PDS_TRACE_DEBUG("Unknown event %u", event->event_id);
        break;
    }
}


}    // namespace nicmgr

/// \@}

