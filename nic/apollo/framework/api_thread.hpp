//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// API thread specific helper functions
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_THREAD_HPP__
#define __FRAMEWORK_API_THREAD_HPP__

#include "nic/sdk/lib/ipc/ipc.hpp"

namespace api {

/// \defgroup PDS_API_THREAD API thread helper functions/wrappers
/// @{

/// \brief    callback function invoked during api thread initialization
/// \param[in] ctxt    opaque context
void api_thread_init_fn(void *ctxt);

/// \brief    callback function invoked during api thread exit
/// \param[in] ctxt    opaque context
void api_thread_exit_fn(void *ctxt);

/// \brief    callback function invoked to process events received by API thread
/// \param[in] msg     message carrying the event information
/// \param[in] ctxt    opaque context
void api_thread_event_cb(void *msg, void *ctx);

/// \brief    callback function invoked to process BATCH IPC msgs received by
///           API thread
/// \param[in] msg     IPC msg received
/// \param[in] ctxt    opaque context
void api_thread_ipc_batch_cb(sdk::ipc::ipc_msg_ptr msg, void *ctxt);

/// \brief    return true if API thread is ready to receive events
/// \return true or false based on whether API thread is ready or not
bool is_api_thread_ready(void);

/// \@}

}    // namespace api

#endif    // __FRAMEWORK_API_THREAD_HPP__
