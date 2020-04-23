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
    case CMD_MSG_FLOW_DUMP:
        flow_cb = vpp_uds_callbacks[VPP_UDS_FLOW_DUMP];
        if (flow_cb) {
            flow_cb(fd);
        }
        break;
    case CMD_MSG_NAT_PB_DUMP:
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
    types::ServiceRequestMessage proto_ctxt;
    svc_req_ctxt_t ctxt;

    // convert to proto msg
    proto_ctxt.ParseFromArray(buf, n);
    // parse cmd ctxt
    pds_svc_req_proto_to_svc_req_ctxt(&ctxt, &proto_ctxt, fd);
    // handle command
    handle_pds_cmd(&ctxt.cmd_ctxt, fd);
}

#ifdef __cplusplus
}
#endif
