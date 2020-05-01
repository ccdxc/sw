//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains athena_app server utility API
///
//----------------------------------------------------------------------------

#ifndef __ATHENA_APP_SERVER_HPP__
#define __ATHENA_APP_SERVER_HPP__

#include "app_test_utils.hpp"

namespace test {
namespace athena_app {

#define ATHENA_APP_CMD_STR_BYTES    64
#define ATHENA_APP_SCRIPT_STR_BYTES 128
#define ATHENA_APP_FNAME_STR_BYTES  128

/*
 * Ensure termination of a string char array
 */
#define ATHENA_APP_MSG_STR_TERM(str)                        \
    str[sizeof(str) - 1] = 0

/*
 * Prototype request message
 */
typedef struct {
    char            cmd_str[ATHENA_APP_CMD_STR_BYTES];
    char            cmd_body[0];
} server_req_t;

/*
 * Initialize a server request message.
 */
static inline server_req_t *
server_req_get(zmq_msg_t *rx_msg)
{
    server_req_t *req = (server_req_t *)zmq_msg_data(rx_msg);
    ATHENA_APP_MSG_STR_TERM(req->cmd_str);
    return req;
}

/*
 * Initialize a server request message (for client use).
 */
#define SERVER_REQ_INIT(msg, req, req_type_t)           \
    req_type_t *req;                                    \
    zmq_msg_init_size(msg, sizeof(req_type_t));         \
    req = (req_type_t *)zmq_msg_data(msg);              \
    *req = {0}                                          \

/*
 * Prototype response message usable by responders of the majority of requests.
 * Other responses with different needs may define their own structs in this
 * file as applicable.
 */
typedef struct {
    int             status;
} server_rsp_t;

/*
 * Initialize a server response message.
 */
#define SERVER_RSP_GET(msg, rsp, rsp_type_t)            \
    rsp_type_t *rsp;                                    \
    zmq_msg_init_size(msg, sizeof(rsp_type_t));         \
    rsp = (rsp_type_t *)zmq_msg_data(msg)               \
    
#define SERVER_RSP_INIT(msg, rsp, rsp_type_t)           \
    SERVER_RSP_GET(msg, rsp, rsp_type_t);               \
    *rsp = {0}                                          \
    
/*
 * Server script_exec request message
 */
typedef struct {
    server_req_t    cmd;
    char            dir[ATHENA_APP_SCRIPT_STR_BYTES];
    char            name[ATHENA_APP_SCRIPT_STR_BYTES];
} server_script_exec_t;

/*
 * Server flow cache dump request message
 */
typedef struct {
    server_req_t    cmd;
    char            fname[ATHENA_APP_FNAME_STR_BYTES];
    uint32_t        append;
} flow_cache_dump_t;

/*
 * Server flow stats dump request message
 */
typedef flow_cache_dump_t   flow_stats_dump_t;

int  server_init(void); 
void server_fini(void);
void server_poll(void);
void server_poll_stop(void);

/*
 * Size validator for small messages.
 */
static inline pds_ret_t
server_msg_size_check(zmq_msg_t *rx_msg,
                      size_t exp_size)
{
    size_t  rx_size;

    rx_size = zmq_msg_size(rx_msg);
    if (rx_size < exp_size) {
        TEST_LOG_ERR("Rx bytes %d less than expected(%d)\n",
                     (int)rx_size, (int)exp_size);
        return PDS_RET_INVALID_ARG;
    }
    return PDS_RET_OK;
}

static inline const char *
athena_app_server_sock_tcp(void)
{
    return "tcp";
}

static inline const char *
athena_app_server_sock_ipc(void)
{
    return "ipc";
}

static inline const char *
athena_app_server_sock_dir(const std::string& sock_type)
{
    if (sock_type == "ipc") {
#ifdef __x86_64__
        return "/sw/nic";
#else
        return "/nic/etc";
#endif
    }

    // Server will listen on any addresses
    return "*";
}

static inline const char *
athena_app_server_sock_name(const std::string& sock_type)
{
    if (sock_type == "ipc") {
        return "athena_app_sock";
    }
    return "52376";
}

static inline const char *
athena_app_client_sock_type(void)
{
#ifdef __x86_64__
    return "tcp";
#else
    return "ipc";
#endif
}

static inline const char *
athena_app_client_sock_dir(const std::string& sock_type)
{
    // When client runs on x86, we really can't tell whether it is
    // in SIM or running on an actual server host. For now, we don't
    // anticipate any use for server/client in SIM mode so we
    // can just assume real HW.
    
    if (sock_type == "ipc") {
        return "/nic/etc";
    }

    // TCP client will try to connect to the address below
    // which is a tun address.
    return "1.0.0.2";
}

static inline const char *
athena_app_client_sock_name(const std::string& sock_type)
{
    return athena_app_server_sock_name(sock_type);
}

static inline std::string
athena_app_zmqsockstr(const std::string& sock_type,
                      const std::string& sock_dir,
                      const std::string& sock_name)
{
    std::string sock_str;

    if (sock_type == "ipc") {
        sock_str.assign(sock_type + ":///" + sock_dir + "/" + sock_name);
    } else {
        sock_str.assign(sock_type + "://" + sock_dir + ":" + sock_name);
    }
    return sock_str;
}

}    // namespace athena_app
}    // namespace test

#endif  // __ATHENA_APP_SERVER_HPP__
