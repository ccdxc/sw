//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains FTE core functionality
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/fte/fte.hpp"

namespace fte {

void *
fte_thread_start (void *ctxt)
{
    SDK_THREAD_INIT(ctxt);
    PDS_TRACE_DEBUG("FTE entering forever loop ...");
    while (1);
    return NULL;
}

}    // namespace fte
