//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include "uds_internal.h"
#include <nic/apollo/api/include/pds_debug.hpp>
#include <nic/apollo/agent/svc/specs.hpp>

static vpp_uds_cb vpp_uds_callbacks[VPP_UDS_MAX] = {0};

void
vpp_uds_register_cb(vpp_uds_op_t op, vpp_uds_cb cb)
{
    vpp_uds_callbacks[op] = cb;
}

void
handle_pds_cmd (cmd_ctxt_t *ctxt, int fd)
{
    vpp_uds_cb flow_cb = NULL;
    switch (ctxt->cmd) 
    {
    case CLI_CMD_FLOW_DUMP:
        flow_cb = vpp_uds_callbacks[VPP_UDS_FLOW_DUMP];
        if (flow_cb) {
            flow_cb(fd);
        }
        break;
    case CLI_CMD_NAT_PB_DUMP:
        flow_cb = vpp_uds_callbacks[VPP_UDS_NAT_PB_DUMP];
        if (flow_cb) {
            flow_cb(fd);
        }
        break;
    default:
        // Nothing to do
        break;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

void
udswrap_process_input(int fd, char *buf, int n)
{
    pds::CommandCtxt proto_cmd_ctxt;
    cmd_ctxt_t cmd_ctxt = { 0 };

    // convert to proto msg
    proto_cmd_ctxt.ParseFromArray(buf, n);
    // parse cmd ctxt
    pds_cmd_proto_to_cmd_ctxt(&cmd_ctxt, &proto_cmd_ctxt, fd);
    // handle command
    handle_pds_cmd(&cmd_ctxt, fd);
}

#ifdef __cplusplus
}
#endif
