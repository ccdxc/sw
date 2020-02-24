/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include "sysmon_internal.hpp"
#include "lib/ipc/ipc.hpp"

static void
sysmon_xcvr_event_handler(sdk::ipc::ipc_msg_ptr msg, const void *ctxt) {

    SDK_TRACE_INFO("xcvr event received");
}

static void
sysmon_port_event_handler(sdk::ipc::ipc_msg_ptr msg, const void *ctxt) {

    SDK_TRACE_INFO("Port event received");
}

void
sysmon_ipc_init(void) {
    sdk::ipc::subscribe(EVENT_ID_XCVR_STATUS,
                        sysmon_xcvr_event_handler, NULL);
    sdk::ipc::subscribe(EVENT_ID_PORT_STATUS,
                        sysmon_port_event_handler, NULL);
}
