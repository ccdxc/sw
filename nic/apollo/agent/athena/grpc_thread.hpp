//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __GRPC_THREAD_HPP__
#define __GRPC_THREAD_HPP__

#define FD_INVALID (-1)

namespace core {

void grpc_reg_thread_init(void *ctxt);
void grpc_reg_thread_exit(void *ctxt);

}    // namespace core

#endif    // __GRPC_THREAD_HPP__
