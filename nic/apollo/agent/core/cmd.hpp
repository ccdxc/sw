//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CMD_HPP__
#define __CMD_HPP__

#define FD_INVALID (-1)

namespace core {

void cmd_server_thread_init(void *ctxt);
void cmd_server_thread_exit(void *ctxt);

}    // namespace core

#endif    // __CMD_HPP__
