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

DEFINE_string(flow_stats_dump, "",
              "Dump flow cache statistics to a file on Naples");
DEFINE_string(flow_cache_dump, "",
              "Dump flow cache to a file on Naples");
DEFINE_bool(append, false,
            "Append to the end of file rather than overwrite");

static int
cmd_flow_stats_dump_rsp_check(zmq_msg_t *rx_msg)
{
    if (server_msg_size_check(rx_msg, sizeof(server_rsp_t)) == 0) {
        SERVER_RSP_GET(rx_msg, rsp, server_rsp_t);
        CLIENT_LOG_INFO("Completed flow stats dump to %s, status: %s\n",
                        FLAGS_flow_stats_dump.c_str(),
                        rsp->status == 0 ? "SUCCESS" : "FAILURE");
        return rsp->status;
    }
    return PDS_RET_ERR;
}

int
cmd_flow_stats_dump_req_check(void)
{
    int     status = 0;

    if (!FLAGS_flow_stats_dump.empty()) {
        zmq_msg_t   tx_msg;

        SERVER_REQ_INIT(&tx_msg, req, flow_stats_dump_t);
        SERVER_REQ_STRCPY(req->cmd.cmd_str, "flow_stats_dump");
        SERVER_REQ_STRCPY(req->fname, FLAGS_flow_stats_dump.c_str());
        req->append = FLAGS_append;
        status = client_req_rsp(&tx_msg, cmd_flow_stats_dump_rsp_check);
    }
    return status;
}

static int
cmd_flow_cache_dump_rsp_check(zmq_msg_t *rx_msg)
{
    if (server_msg_size_check(rx_msg, sizeof(server_rsp_t)) == 0) {
        SERVER_RSP_GET(rx_msg, rsp, server_rsp_t);
        CLIENT_LOG_INFO("Completed flow cache dump to %s, status: %s\n",
                        FLAGS_flow_cache_dump.c_str(),
                        rsp->status == 0 ? "SUCCESS" : "FAILURE");
        return rsp->status;
    }
    return PDS_RET_ERR;
}

int
cmd_flow_cache_dump_req_check(void)
{
    int     status = 0;

    if (!FLAGS_flow_cache_dump.empty()) {
        zmq_msg_t   tx_msg;

        SERVER_REQ_INIT(&tx_msg, req, flow_cache_dump_t);
        SERVER_REQ_STRCPY(req->cmd.cmd_str, "flow_cache_dump");
        SERVER_REQ_STRCPY(req->fname, FLAGS_flow_cache_dump.c_str());
        req->append = FLAGS_append;
        status = client_req_rsp(&tx_msg, cmd_flow_cache_dump_rsp_check);
    }
    return status;
}

