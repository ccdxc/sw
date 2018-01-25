//------------------------------------------------------------------------------
// periodic thread manages few timer wheels and can carry our periodic tasks
// as background activities
//------------------------------------------------------------------------------

#ifndef __SDK_LINKMGR_PERIODIC_HPP__
#define __SDK_LINKMGR_PERIODIC_HPP__

#include <sdk/twheel.hpp>

namespace sdk {
namespace linkmgr {

sdk_ret_t
linkmgr_timer_init();

void *
linkmgr_periodic_thread_start(void *ctxt);

//------------------------------------------------------------------------------
// API invoked by other threads to trigger cb after timeout
// Returns the timer entry used to update/delete the timer
//------------------------------------------------------------------------------
void *
linkmgr_timer_schedule(uint32_t timer_id, uint64_t timeout, void *ctxt,
                       sdk::lib::twheel_cb_t cb, bool periodic);

//------------------------------------------------------------------------------
// API invoked by other threads to delete the scheduled timer
//------------------------------------------------------------------------------
void *
linkmgr_timer_delete(void *timer);

//------------------------------------------------------------------------------
// API invoked by other threads to update the scheduled timer
//------------------------------------------------------------------------------
void *
linkmgr_timer_update(void *timer, uint64_t timeout, bool periodic, void *ctxt);

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_PERIODIC_HPP__
