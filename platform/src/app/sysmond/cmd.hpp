//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CMD_HPP__
#define __CMD_HPP__

#define FD_INVALID (-1)

namespace sysmon {

typedef enum cli_cmd_e {
    CLI_CMD_START = 0,
    CLI_CMD_DUMP_INTR,
    CLI_CMD_CLEAR_INTR,
    CLI_CMD_MAX = 255
} cli_cmd_t;

typedef struct cmd_ctxt_s {
    int fd;
    cli_cmd_t cmd;
} __attribute__((packed)) cmd_ctxt_t;

void cmd_server_thread_init(void *ctxt);
void cmd_server_thread_exit(void *ctxt);

}    // namespace sysmon

using sysmon::cmd_ctxt_t;

#endif    // __CMD_HPP__
