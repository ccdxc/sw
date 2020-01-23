//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/event.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

sdk_ret_t
handle_event_request (const pds_event_spec_t *spec, void *ctxt)
{
    if (spec->event_op == PDS_EVENT_OP_SUBSCRIBE) {
        core::agent_state::state()->event_mgr()->subscribe(spec->event_id,
                                                           ctxt);
    } else if (spec->event_op == PDS_EVENT_OP_UNSUBSCRIBE) {
        core::agent_state::state()->event_mgr()->subscribe(spec->event_id,
                                                           ctxt);
    } else {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
update_event_listener (void *ctxt)
{
    if (!core::agent_state::state()->event_mgr()->is_listener_active(ctxt)) {
        PDS_TRACE_DEBUG("Listener {} is not active, removing ...", ctxt);
        //core::agent_state::state()->event_mgr()->unsubscribe_listener(ctxt);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

}    // namespace core
