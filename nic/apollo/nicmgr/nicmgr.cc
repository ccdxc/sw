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
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"

/// \defgroup PDS_NICMGR
/// @{

namespace nicmgr {
void *
nicmgr_thread_start (void *ctxt)
{
    PDS_TRACE_INFO("nicmgr thread running ...");
    while (true) {
        pthread_yield();
    }
}

}    // namespace nicmgr

/// \@}

