//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn thread implementation
///
//----------------------------------------------------------------------------

#ifndef __LEARN_LEARN_THREAD_HPP__
#define __LEARN_LEARN_THREAD_HPP__

#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/lib/dpdk/dpdk.hpp"

typedef enum learn_msg_id_e {
    LEARN_MSG_ID_NONE,
    LEARN_MSG_ID_MAPPING_API,
} learn_msg_id_t;

namespace event = sdk::event_thread;

namespace learn {

bool learning_enabled(void);

// callback function invoked during learn thread initialization
void learn_thread_init_fn(void *ctxt);

// callback function invoked during learn thread exit
void learn_thread_exit_fn(void *ctxt);

// callback function invoked to process events received
void learn_thread_event_cb(void *msg, void *ctxt);

// callback functions invoked to process mapping CRUD apis called by control
// plane
void learn_thread_ipc_cp_cb(sdk::ipc::ipc_msg_ptr msg, const void *ctxt);

// callback to poll for packets on learn thread
void learn_thread_pkt_poll_timer_cb(event::timer_t *timer);

}    // namespace learn

#endif    // __LEARN_LEARN_THREAD_HPP__
