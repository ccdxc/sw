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
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
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
    string config_file = "/nic/conf/device.json";
    fwd_mode_t fwd_mode = sdk::platform::FWD_MODE_CLASSIC;
    platform_t platform = PLATFORM_HW;

    // instantiate the logger
    utils::logger::init();

    PDS_TRACE_INFO("Initializing device manager ...");
    devmgr = new DeviceManager(config_file, fwd_mode, platform);
    devmgr->LoadConfig(config_file);

    pthread_cleanup_push(nicmgr::nicmgrapi::nicmgr_thread_cleanup, NULL);
    // creating mnets
    PDS_TRACE_INFO("Creating mnets ...");
    devmgr->HalEventHandler(true);

    evutil_add_check(&log_check, &log_flush, NULL);

    // port status event handler
    evutil_add_check(&port_status_check,
                     &nicmgrapi::port_status_handler_, NULL);

    PDS_TRACE_INFO("Listening to events ...");
    evutil_run();
    pthread_cleanup_pop(1);

    return NULL;
}

//------------------------------------------------------------------------------
// nicmgr thread cleanup
//------------------------------------------------------------------------------
void
nicmgrapi::nicmgr_thread_cleanup (void *arg) {
    delete devmgr;
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

