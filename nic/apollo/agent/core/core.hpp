//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CORE_HPP__
#define __CORE_HPP__

#define FD_INVALID (-1)

namespace core {

void *fd_recv_thread_start(void *ctxt);

#define CALLOC(id, size) calloc(size, size);
#define FREE(id, mem)    free(mem)

}    // namespace core

#endif    // __CORE_HPP__
