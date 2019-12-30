//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// periodic thread manages few timer wheels and can carry our periodic tasks
// as background activities. Few examples include:
// - delay deleting memory resources to slabs or heap
// - flow table scan to age out sessions or detect dead flows
// - periodic stats collection and/or aggregation
//------------------------------------------------------------------------------

#ifndef __SDK_LINKMGR_PERIODIC_HPP__
#define __SDK_LINKMGR_PERIODIC_HPP__

#include "include/sdk/base.hpp"
#include "lib/twheel/twheel.hpp"

namespace sdk {
namespace linkmgr {

//------------------------------------------------------------------------------
// return true if periodic thread is up and running
//------------------------------------------------------------------------------
bool periodic_thread_is_running(void);

//------------------------------------------------------------------------------
// return true if periodic thread is ready and fully initialized
//------------------------------------------------------------------------------
bool periodic_thread_is_ready(void);

//------------------------------------------------------------------------------
// one time initialization for periodic thread
//------------------------------------------------------------------------------
void *periodic_thread_init(void *ctxt);

//------------------------------------------------------------------------------
// core logic of periodic thread
//------------------------------------------------------------------------------
void *periodic_thread_run(void *ctxt);

//------------------------------------------------------------------------------
// cleanup for periodic thread
//------------------------------------------------------------------------------
void periodic_thread_cleanup(void *arg=NULL);

//------------------------------------------------------------------------------
// API invoked by other threads to trigger cb after timeout
// Returns the timer entry used to update/delete the timer
//------------------------------------------------------------------------------
void *timer_schedule(uint32_t timer_id, uint64_t timeout, void *ctxt,
                     sdk::lib::twheel_cb_t cb, bool periodic);

//------------------------------------------------------------------------------
// API invoked by other threads to get timeout remaining in msecs for the timer.
//------------------------------------------------------------------------------
uint64_t get_timeout_remaining(void *timer);

//------------------------------------------------------------------------------
// API invoked by other threads to delete the scheduled timer
//------------------------------------------------------------------------------
void *timer_delete(void *timer);

//------------------------------------------------------------------------------
// API invoked by other threads to update the scheduled timer context
//------------------------------------------------------------------------------
void *timer_update_ctxt(void *timer, void *ctxt);

//------------------------------------------------------------------------------
// API invoked by other threads to update the scheduled timer
//------------------------------------------------------------------------------
void *timer_update (void *timer, uint64_t timeout, bool periodic, void *ctxt);

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_PERIODIC_HPP__

