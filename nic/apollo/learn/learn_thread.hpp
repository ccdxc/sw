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

#include <unistd.h>
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/lib/dpdk/dpdk.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"

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

// callback to handle endpoint aging
void learn_thread_aging_timer_cb(event::timer_t *timer);

// callback to process async response from API thread for CRUD APIs
// issued when learning local endpoints
void learn_thread_api_async_cb_local(sdk_ret_t ret, const void *cookie);

// callback to process async response from API thread for CRUD APIs
// issued when learning remote endpoints
void learn_thread_api_async_cb_remote(sdk_ret_t ret, const void *cookie);

// callback to process async response from API thread for CRUD APIs
// issued when aging local endpoints
void learn_thread_api_async_cb_aging(sdk_ret_t ret, const void *cookie);


// utility functions
void learn_thread_drop_pkt(void *mbuf, uint8_t reason);
void learn_thread_send_pkt(void *mbuf, learn_nexthop_t nexthop);

}    // namespace learn

#endif    // __LEARN_LEARN_THREAD_HPP__
