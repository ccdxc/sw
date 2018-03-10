//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __TWHEEL_HPP__
#define __TWHEEL_HPP__

#include <time.h>
#include "sdk/base.hpp"
#include "sdk/lock.hpp"
#include "sdk/slab.hpp"
#include "sdk/timestamp.hpp"

using sdk::lib::slab;

namespace sdk {
namespace lib {

#define TWHEEL_DEFAULT_SLICE_DURATION    250      // in msecs
#define TWHEEL_DEFAULT_DURATION          (2 * 60 * TIME_MSECS_PER_MIN) // 2 hr

typedef void (*twheel_cb_t)(void *timer, uint32_t timer_id, void *ctxt);
typedef struct twentry_s twentry_t;
struct twentry_s {
    uint32_t       timer_id_;    // unique (in app's space) timer id
    uint32_t       timeout_;     // timeout (in ms) of this timer
    bool           periodic_;    // periodic timer
    void           *ctxt_;       // user provided context
    twheel_cb_t    cb_;          // callback to invoke at timeout
    uint16_t       nspins_;      // age this out when nspins == 0
    uint32_t       slice_;       // slice this entry is sitting in
    twentry_t      *next_;       // next entry in the list
    twentry_t      *prev_;       // previous entry in the list
} __PACK__;

// one slice of the timer wheel
typedef struct tw_slice_s {
    sdk_spinlock_t    slock_;          // lock for thread safety
    twentry_t         *slice_head_;    // slice head
}  __PACK__ tw_slice_t;

//------------------------------------------------------------------------------
// NOTE: all intervals are expressed in milli seconds
//------------------------------------------------------------------------------
class twheel {
public:
    static twheel *factory(uint64_t slice_intvl=TWHEEL_DEFAULT_SLICE_DURATION,
                           uint32_t wheel_duration=TWHEEL_DEFAULT_DURATION,
                           bool thread_safe=false);
    static void destroy(twheel *twh);
    void tick(uint32_t msecs_elapsed);
    void *add_timer(uint32_t timer_id, uint64_t timeout, void *ctxt,
                    twheel_cb_t cb, bool periodic=false);
    void *del_timer(void *timer);
    void *upd_timer(void *timer, uint64_t timeout, bool periodic, void *ctxt);
    uint64_t get_timeout_remaining(void *timer);
    uint32_t num_entries(void) const { return num_entries_; }

private:

    slab          *twentry_slab_;    // slab memory for timer wheel entry
    uint64_t      slice_intvl_;      // per slice interval in msecs
    bool          thread_safe_;      // TRUE if this is thread_safe instance
    timespec_t    prev_tick_tp_;     // point in time previous tick was processed
    uint32_t      nslices_;          // # of slices in this wheel
    tw_slice_t    *twheel_;          // timer wheel itself
    uint32_t      curr_slice_;       // current slice we are processing
    uint32_t      num_entries_;      // no. of entries in the wheel

private:
    twheel() {};
    ~twheel();
    sdk_ret_t init(uint64_t slice_intvl, uint32_t wheel_duration, bool thread_safe);
    void init_twentry_(twentry_t *twentry, uint32_t timer_id, uint64_t timeout,
                       bool periodic, void *ctxt, twheel_cb_t cb);

    inline void insert_timer_(twentry_t *twentry) {
        twentry->next_ = twheel_[twentry->slice_].slice_head_;
        twheel_[twentry->slice_].slice_head_ = twentry;
        num_entries_++;
    }

    inline void remove_timer_(twentry_t *twentry) {
        if (twentry->next_) {
            // removing last entry in the list
            twentry->next_->prev_ = twentry->prev_;
        }
        if (twentry->prev_ == NULL) {
            // removing the head of the list
            twheel_[twentry->slice_].slice_head_ = twentry->next_;
        } else {
            twentry->prev_->next_ = twentry->next_;
        }
        num_entries_--;
    }

    inline void upd_timer_(twentry_t *twentry, uint64_t timeout, bool periodic) {
        remove_timer_(twentry);
        init_twentry_(twentry, twentry->timer_id_, timeout,
                      periodic, twentry->ctxt_, twentry->cb_);
        insert_timer_(twentry);
    }

};

}    // namespace lib
}    // namespace sdk

#endif    // __TWHEEL_HPP__

