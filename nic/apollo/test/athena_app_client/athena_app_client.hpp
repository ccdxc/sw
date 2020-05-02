//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __ATHENA_APP_CLIENT_HPP__
#define __ATHENA_APP_CLIENT_HPP__

#include "nic/apollo/test/athena_app/app_test_utils.hpp"
#include "nic/apollo/test/athena_app/athena_app_server.hpp"
#include "nic/third-party/gflags/include/gflags/gflags.h"
#include <zmq.h>

using namespace test::athena_app;
using namespace std;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#endif

#define CLIENT_LOG_INFO(args...)                            \
    printf(args)

#define CLIENT_LOG_ERR(args...)                             \
    printf("ERROR: " args)

/*
 * d must be a char array with a finite size
 */
#define SERVER_REQ_STRCPY(d, s)         \
    strncpy(d, s, sizeof(d) - 1)

typedef int (*client_resp_fn_t)(zmq_msg_t *rx_msg);

int client_req_rsp(zmq_msg_t *tx_msg,
                   client_resp_fn_t rsp_handler,
                   int tx_flags = 0);

int cmd_script_exec_req_check(void);
int cmd_flow_stats_dump_req_check(void);
int cmd_flow_cache_dump_req_check(void);
int cmd_session_info_dump_req_check(void);

#define CMD_STATUS_CHECK(status, cmd_fn)                    \
    if ((status) == 0) {                                    \
        (status) = cmd_fn;                                  \
    }                                                       \

#endif   // __ATHENA_APP_TEST_UTILS_HPP__

