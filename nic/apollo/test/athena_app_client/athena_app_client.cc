//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Athena app client code
///
//----------------------------------------------------------------------------
// 
#include "athena_app_client.hpp"

DEFINE_string(server_type, athena_app_client_sock_type(),
              "server type such as 'tcp' or 'ipc'");
DEFINE_string(server_addr,
              athena_app_client_sock_dir(athena_app_client_sock_type()),
              "server address");
DEFINE_string(server_port,
              athena_app_client_sock_name(athena_app_client_sock_type()),
              "server port");
DEFINE_int32(rx_timeout, 600,
             "Socket receive timeout in seconds");
DEFINE_int32(tx_timeout, 300,
             "Socket transmit timeout in seconds");

/*
 * ZMQ transport address for monitoring socket state change
 */
#define ZMQ_SOCK_STATE_CHANGE_ADDR      "inproc://monitor.sock"

static void         *client_ctx;
static void         *client_sock;

static int
client_connect(string& client_sock_str)
{
    zmq_msg_t   mon_msg;
    void        *mon_sock = nullptr;
    bool        connected;
    int         rx_bytes;
    int         ret;

    /*
     * Monitor the socket for connection to server
     */
    ret = zmq_socket_monitor(client_sock, ZMQ_SOCK_STATE_CHANGE_ADDR,
                             ZMQ_EVENT_ALL);
    if (ret == 0) {
        mon_sock = zmq_socket(client_ctx, ZMQ_PAIR);
        ret = zmq_connect(mon_sock, ZMQ_SOCK_STATE_CHANGE_ADDR);
    }

    if (ret == 0) {
        CLIENT_LOG_INFO("Connecting to %s...\n", client_sock_str.c_str());
        ret = zmq_connect(client_sock, client_sock_str.c_str());
    }

    connected = false;
    while ((ret == 0) && !connected) {
        zmq_msg_init(&mon_msg);
        rx_bytes = zmq_msg_recv(&mon_msg, mon_sock, 0);
        if (rx_bytes < 0) {
            if ((zmq_errno() != EAGAIN) && (zmq_errno() != EINTR)) {
                ret = PDS_RET_ERR;
            }

        } else if (rx_bytes >= (int)sizeof(uint16_t)) {
            switch (*((uint16_t *)zmq_msg_data(&mon_msg))) {

            case ZMQ_EVENT_CONNECTED:
                CLIENT_LOG_INFO("Connected!\n");
                connected = true;
                break;

            case ZMQ_EVENT_CLOSED:
                CLIENT_LOG_ERR("failed connect - socket prematurely closed\n");
                ret = PDS_RET_ERR;
                break;

            default:
                break;
            }
        }
        zmq_msg_close(&mon_msg);
    }

    if (mon_sock) {
        zmq_close(mon_sock);
    }
    return ret;
}

static int
client_init(void)
{
    string  client_sock_str;
    int     enable = 1;
    int     rx_tmo_ms = FLAGS_rx_timeout * MSEC_PER_SEC;
    int     tx_tmo_ms = FLAGS_tx_timeout * MSEC_PER_SEC;

    /*
     *Socket to use ZMQ_REQ/ZMQ_REP request/reply mode
     */
    client_ctx = zmq_ctx_new();
    client_sock = zmq_socket(client_ctx, ZMQ_REQ);

    zmq_setsockopt(client_sock, ZMQ_RCVTIMEO, &rx_tmo_ms, sizeof(rx_tmo_ms));
    zmq_setsockopt(client_sock, ZMQ_SNDTIMEO, &tx_tmo_ms, sizeof(tx_tmo_ms));
    zmq_setsockopt(client_sock, ZMQ_REQ_CORRELATE, &enable, sizeof(enable));

    client_sock_str = athena_app_zmqsockstr(FLAGS_server_type, FLAGS_server_addr,
                                            FLAGS_server_port);
    return client_connect(client_sock_str);
}

static void
client_fini(void)
{
    if (client_sock) {
        zmq_close(client_sock);
        client_sock = nullptr;
    }
    if (client_ctx) {
        zmq_ctx_destroy(client_ctx);
        client_ctx = nullptr;
    }
}

int
client_req_rsp(zmq_msg_t *tx_msg,
               client_resp_fn_t rsp_handler,
               int tx_flags)
{
    zmq_msg_t       rx_msg;
    int             status = 0;

    /*
     * If there's no tx_msg to send, the caller most likely is handling
     * a multi-part response.
     */
    if (tx_msg) {
        if (zmq_msg_send(tx_msg, client_sock, tx_flags) == -1) {
            zmq_msg_close(tx_msg);
            CLIENT_LOG_ERR("failed to send message to server: %s\n",
                           zmq_strerror(zmq_errno()));
            return PDS_RET_ERR;
        }
        zmq_msg_close(tx_msg);
    }

    /*
     * If there's no rsp_handler, the caller most likely is doing
     * a multi-part send and will ask for a response on the last send.
     */
    if (rsp_handler) {
        zmq_msg_init(&rx_msg);
        if (zmq_msg_recv(&rx_msg, client_sock, 0) == -1) {
            zmq_msg_close(&rx_msg);
            CLIENT_LOG_ERR("failed to receive message from server: %s\n",
                           zmq_strerror(zmq_errno()));
            return PDS_RET_ERR;
        }
        status = (*rsp_handler)(&rx_msg);
        zmq_msg_close(&rx_msg);
    }
    return status;
}

int
main(int argc,
     char **argv)
{
    int         status;

    /*
     * Note that we use gflags (Reference https://gflags.github.io/gflags, or
     * nic/third-party/gflags/include/gflags/gflags.h) instead of getopt_long
     * for the following reasons:
     *
     * 1) getopt_long requires long option names to map to short
     *    which will eventually become unscalable as the number of
     *    options become large.
     * 2) gflag options don't all have to be defined in main.cc;
     *    they can be defined anywhere in other modules.
     * 3) gflags automatically consolidate all usage help text strings for
     *    --help usage show.
     */
    gflags::SetUsageMessage("Options:\n\n");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    status = client_init();
    CMD_STATUS_CHECK(status, cmd_script_exec_req_check());
    CMD_STATUS_CHECK(status, cmd_flow_stats_dump_req_check());
    CMD_STATUS_CHECK(status, cmd_flow_cache_dump_req_check());

    client_fini();
    return status == 0 ? 0 : 1;
}

