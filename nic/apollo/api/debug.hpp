/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    debug.hpp
 *
 * @brief   APIs for troubleshooting
 */

#include <unordered_map>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/table.hpp"

#if !defined (__DEBUG_HPP__)
#define __DEBUG_HPP__

using std::unordered_map;

#define FD_INVALID (-1)

namespace debug {

typedef unordered_map<int64_t, int> cmd_fd_db_t;

extern cmd_fd_db_t g_cmd_fd_map;

typedef struct mapping_dump_args_s {
    pds_mapping_key_t key;
} mapping_dump_args_t;

typedef struct cmd_args_s {
    union {
        mapping_dump_args_t *mapping_dump;
    };
} cmd_args_t;

typedef enum cli_cmd_e {
    CLI_CMD_START        = 0,
    CLI_CMD_MAPPING_DUMP = CLI_CMD_START,
    CLI_CMD_MAX          = 255,
} cli_cmd_t;

typedef struct cmd_ctxt_s {
    int        fd;    // File descriptor
    cli_cmd_t  cmd;   // CLI command
    cmd_args_t *args; // Command arguments
} cmd_ctxt_t;

int fd_get_from_cid(int64_t cid);
void cid_remove_from_fd_map(int64_t cid);
void *fd_recv_thread_start(void *ctxt);

}    // namespace debug

#endif    /** __DEBUG_HPP__ */
