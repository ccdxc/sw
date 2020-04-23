//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __SVC_THREAD_HPP__
#define __SVC_THREAD_HPP__

#define FD_INVALID (-1)

namespace core {

void svc_server_thread_init(void *ctxt);
void svc_server_thread_exit(void *ctxt);

}    // namespace core

#endif    // __SVC_THREAD_HPP__
