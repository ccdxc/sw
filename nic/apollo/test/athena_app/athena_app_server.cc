//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Athena app server code
///
//----------------------------------------------------------------------------
// 
#include "nic/apollo/agent/athena/trace.hpp"
#include "app_test_utils.hpp"
#include "athena_app_server.hpp"
#include "fte_athena.hpp"
#include "zmq_wait.h"

namespace test {
namespace athena_app {

static void     *server_ctx;
static void     *server_sock;

const static map<string,server_msg_proc_fn_t>  cmd2fn_map =
{
    {"script_exec",             script_exec_msg_process},
    {"flow_stats_dump",         fte_ath::fte_dump_flow_stats},
    {"flow_cache_dump",         fte_ath::fte_dump_flows},
    {"session_info_dump",       fte_ath::fte_dump_sessions},
};

static inline server_msg_proc_fn_t
cmd2fn_find(const string &cmd)
{
    auto iter = cmd2fn_map.find(cmd);
    if (iter != cmd2fn_map.end()) {
        return iter->second;
    }
    return nullptr;
}

static int
server_msg_process(zmq_msg_t *rx_msg,
                   zmq_msg_t *tx_msg)
{
    std::string             cmd;
    server_req_t            *req;
    server_msg_proc_fn_t    msg_proc_fn;

    if (server_msg_size_check(rx_msg, sizeof(server_req_t)) != PDS_RET_OK) {
        SERVER_RSP_INIT(tx_msg, rsp, server_rsp_t);
        return (rsp->status = PDS_RET_INVALID_ARG);
    }

    /*
     * Distribute to corresponding request processor. Note that in the case
     * of very long (i.e., multi-part) request messages, the request processor
     * would be responsible for issuing more calls zmq_msg_recv() to obtain
     * and handle the subsequent parts.
     */
    req = server_req_get(rx_msg);
    cmd.assign(req->cmd_str);
    msg_proc_fn = cmd2fn_find(cmd);
    if (msg_proc_fn) {
        return (*msg_proc_fn)(rx_msg, tx_msg);
    }

    TEST_LOG_ERR("unknown command: %s\n", req->cmd_str);
    SERVER_RSP_INIT(tx_msg, rsp, server_rsp_t);
    return (rsp->status = PDS_RET_INVALID_OP);
}

static void
server_req_rsp(void *server_sock,
               void *arg)
{
    zmq_msg_t       rx_msg;
    zmq_msg_t       tx_msg;
    int             rx_bytes;

    zmq_msg_init(&rx_msg);
    rx_bytes = zmq_msg_recv(&rx_msg, server_sock, 0);
    if (rx_bytes == -1) {
        TEST_LOG_ERR("failed server message receive: %s\n",
                     zmq_strerror(zmq_errno()));
    } else {
        server_msg_process(&rx_msg, &tx_msg);
        zmq_msg_send(&tx_msg, server_sock, 0);
        zmq_msg_close(&tx_msg);
    }
    zmq_msg_close(&rx_msg);

}

int
server_init(void)
{
    std::string sock_type;
    std::string sock_str;
    int         no_linger = 0;
    int         ret;

    /*
     * Unlike regular sockets, one ZMQ socket can bind to multiple transports
     * and serve multiple client connections simultaneously. Once a socket is
     * bound, it automatically starts accepting connections.
     * Note: ZMQ_REP is reply mode
     */
    server_ctx = zmq_ctx_new();
    server_sock = zmq_socket(server_ctx, ZMQ_REP);

    /*
     * Bind to TCP to serve clients on server host.
     */
    sock_type = athena_app_server_sock_tcp();
    sock_str = athena_app_zmqsockstr(sock_type,
                                     athena_app_server_sock_dir(sock_type),
                                     athena_app_server_sock_name(sock_type));
    TEST_LOG_INFO("App server binding to ZMQ Socket: %s\n",
                  sock_str.c_str());
    ret = zmq_bind(server_sock, sock_str.c_str());
    if (ret) {
        TEST_LOG_ERR("failed app server bind to %s: %s\n",
                     sock_str.c_str(), zmq_strerror(zmq_errno()));
        return ret;
    }

    /*
     * Bind to IPC to serve clients running on Naples itself.
     */
    sock_type = athena_app_server_sock_ipc();
    sock_str = athena_app_zmqsockstr(sock_type,
                                     athena_app_server_sock_dir(sock_type),
                                     athena_app_server_sock_name(sock_type));
    TEST_LOG_INFO("App server binding to ZMQ Socket: %s\n",
                  sock_str.c_str());
    ret = zmq_bind(server_sock, sock_str.c_str());
    if (ret) {
        TEST_LOG_ERR("failed app server bind to %s: %s\n",
                     sock_str.c_str(), zmq_strerror(zmq_errno()));
        return ret;
    }
    zmq_setsockopt(server_sock, ZMQ_LINGER, &no_linger, sizeof(no_linger));

    TEST_LOG_INFO("App server initialized, waiting for commands...\n");
    ret = zmq_wait_add(server_sock, server_req_rsp, NULL);
    if (ret) {
        TEST_LOG_ERR("failed app server command wait\n");
    }
    return ret;
}

void
server_fini(void)
{
    if (server_sock) {
        zmq_close(server_sock);
        server_sock = nullptr;
    }
    if (server_ctx) {
        zmq_ctx_destroy(server_ctx);
        server_ctx = nullptr;
    }
}

void
server_poll(void)
{
    zmq_wait_loop();
}

void
server_poll_stop(void)
{
    zmq_wait_loop_exit();
}

}    // namespace athena_app
}    // namespace test
