//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pciemgr functionality
///
//----------------------------------------------------------------------------

#include <pthread.h>
#include "include/sdk/base.hpp"
#include "nic/sdk/lib/utils/port_utils.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciemgrd/pciemgrd.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/pciemgr/pciemgr.hpp"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/core.hpp"

/// \defgroup PDS_pciemgr
/// @{

namespace pdspciemgr {

void *
pciemgrapi::pciemgr_thread_start(void *ctxt) {

    PDS_TRACE_INFO("Initializing PCIe manager ...");
    pciemgrd_start();

#if 0
    evutil_check log_check, port_status_check;
    string config_file = "/nic/conf/device.json";
    fwd_mode_t fwd_mode = sdk::platform::FWD_MODE_CLASSIC;
    platform_t platform = PLATFORM_HW;

    // instantiate the logger
    utils::logger::init();

    // initialize pciemgr
    if (platform_is_hw(platform)) {
        PDS_TRACE_INFO("initializing pciemgr");
        pciemgr = new class pciemgr("pciemgrd");
        pciemgr->initialize();
    }

    PDS_TRACE_INFO("Initializing device manager ...");
    devmgr = new DeviceManager(config_file, fwd_mode, platform);
    devmgr->LoadConfig(config_file);

    // creating mnets
    PDS_TRACE_INFO("Creating mnets ...");
    devmgr->HalEventHandler(true);

    evutil_add_check(&log_check, &log_flush, NULL);

    // port status event handler
    evutil_add_check(&port_status_check,
                     &pciemgrapi::port_status_handler_, NULL);

    PDS_TRACE_INFO("Listening to events ...");
    evutil_run();

#endif
    return NULL;
}

}    // namespace pciemgr

/// \@}

