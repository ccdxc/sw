//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file parses and processes the script_exec command
///
//----------------------------------------------------------------------------
// 
#include "athena_app_client.hpp"

DEFINE_string(script_dir, "",
              "server script directory");
DEFINE_string(script_name, "",
              "server script file name");
DEFINE_bool(script_exec, false,
            "execute the specified script_dir/script_name on server");

static int
cmd_script_exec_rsp_check(zmq_msg_t *rx_msg)
{
    if (server_msg_size_check(rx_msg, sizeof(server_rsp_t)) == 0) {
        SERVER_RSP_GET(rx_msg, rsp, server_rsp_t);
        CLIENT_LOG_INFO("Completed server script: %s, status: %s\n",
                        FLAGS_script_name.c_str(),
                        rsp->status == 0 ? "SUCCESS" : "FAILURE");
        return rsp->status;
    }
    return PDS_RET_ERR;
}

int
cmd_script_exec_req_check(void)
{
    int     status = 0;

    if (FLAGS_script_exec) {
        zmq_msg_t   tx_msg;

        SERVER_REQ_INIT(&tx_msg, req, server_script_exec_t);
        SERVER_REQ_STRCPY(req->cmd.cmd_str, "script_exec");
        SERVER_REQ_STRCPY(req->dir, FLAGS_script_dir.c_str());
        SERVER_REQ_STRCPY(req->name, FLAGS_script_name.c_str());
        status = client_req_rsp(&tx_msg, cmd_script_exec_rsp_check);
    }
    return status;
}

