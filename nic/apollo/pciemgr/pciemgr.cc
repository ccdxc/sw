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
#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/core.hpp"

/// \defgroup PDS_pciemgr
/// @{

namespace pdspciemgr {

void *
pciemgrapi::pciemgr_thread_start(void *ctxt) {
    SDK_THREAD_DFRD_TERM_INIT(ctxt);
    logger_init();
    PDS_TRACE_INFO("Initializing PCIe manager ...");
    pciemgrd_start();
    return NULL;
}

}    // namespace pciemgr

/// \@}

