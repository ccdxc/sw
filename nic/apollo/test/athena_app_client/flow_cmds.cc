//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file parses and processes the commands related to flow
///
//----------------------------------------------------------------------------
// 
#include "athena_app_client.hpp"

DEFINE_bool(flow_stats_dump, false,
            "Dump flow cache and related stats on server");

static int
cmd_flow_stats_dump_rsp_check(zmq_msg_t *rx_msg)
{
    if (server_msg_size_check(rx_msg, sizeof(server_rsp_t)) == 0) {
        SERVER_RSP_GET(rx_msg, rsp, server_rsp_t);
        CLIENT_LOG_INFO("Completed flow stats dump, status: %s\n",
                        rsp->status == 0 ? "SUCCESS" : "FAILURE");
        return rsp->status;
    }
    return SDK_RET_ERR;
}

int
cmd_flow_stats_dump_req_check(void)
{
    int     status = 0;

    if (FLAGS_flow_stats_dump) {
        zmq_msg_t   tx_msg;

        SERVER_REQ_INIT(&tx_msg, req, server_req_t);
        SERVER_REQ_STRCPY(req->cmd_str, "flow_stats_dump");
        status = client_req_rsp(&tx_msg, cmd_flow_stats_dump_rsp_check);
    }
    return status;
}

