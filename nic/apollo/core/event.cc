//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// event related infra support
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/core.hpp"

namespace core {

event_t *
event_alloc (void)
{
    return (event_t *)SDK_CALLOC(SDK_MEM_ALLOC_EVENT, sizeof(event_t));
}

void
event_free (event_t *event)
{
    if (event) {
        SDK_FREE(SDK_MEM_ALLOC_EVENT, event);
    }
}

event_t *
event_dequeue (uint32_t thread_id)
{
    thread *thr;

    thr = thread_get(thread_id);
    if (thr) {
        return (event_t *)thr->dequeue();
    }
    return NULL;
}

bool
event_enqueue (event_t *event, uint32_t thread_id)
{
    thread *thr;

    thr = thread_get(thread_id);
    if (thr) {
        return thr->enqueue(event);
    }
    return false;
}

}    // namespace core
