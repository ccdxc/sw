#include <assert.h>
#include <unistd.h>
#include <twheel.hpp>

namespace hal {
namespace utils {

//------------------------------------------------------------------------------
// init function for the timer wheel 
//------------------------------------------------------------------------------
hal_ret_t
twheel::init(uint64_t slice_intvl, uint32_t wheel_duration, bool thread_safe)
{
    uint32_t    i;

    twentry_slab_ = slab::factory("twheel", HAL_SLAB_RSVD,
                                  sizeof(twentry_t), 256,
                                  thread_safe, true, false, false);
    if (twentry_slab_ == NULL) {
        return HAL_RET_OOM;
    }
    slice_intvl_ = slice_intvl;
    thread_safe_ = thread_safe;
    clock_gettime(CLOCK_MONOTONIC, &prev_tick_tp_);
    nslices_ = wheel_duration/slice_intvl;
    twheel_ = (tw_slice_t *)HAL_CALLOC(HAL_MEM_ALLOC_LIB_TWHEEL,
                                       nslices_ * sizeof(tw_slice_t));
    if (twheel_ == NULL) {
        delete twentry_slab_;
        return HAL_RET_OOM;
    }
    if (thread_safe_) {
        for (i = 0; i < nslices_; i++) {
            HAL_SPINLOCK_INIT(&twheel_[i].slock_, PTHREAD_PROCESS_PRIVATE);
        }
    }
    curr_slice_ = 0;
    num_entries_ = 0;
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
twheel *
twheel::factory(uint64_t slice_intvl, uint32_t wheel_duration,
                bool thread_safe)
{
    twheel    *new_twheel = NULL;

    if ((slice_intvl == 0) || (wheel_duration == 0) ||
        (wheel_duration <= slice_intvl)) {
        return NULL;
    }
    new_twheel = new twheel();
    if (new_twheel->init(slice_intvl, wheel_duration, thread_safe) !=
            HAL_RET_OK) {
        delete new_twheel;
        return NULL;
    }
    return new_twheel;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
twheel::~twheel()
{
    uint32_t    i;

    delete twentry_slab_;
    if (thread_safe_) {
        for (i = 0; i < nslices_; i++) {
            HAL_SPINLOCK_DESTROY(&twheel_[i].slock_);
        }
    }
    HAL_FREE(HAL_MEM_ALLOC_LIB_TWHEEL, twheel_);
}

//------------------------------------------------------------------------------
// initialize a given timer wheel entry
//------------------------------------------------------------------------------
void
twheel::init_twentry_(twentry_t *twentry, uint32_t timer_id, uint64_t timeout,
                      bool periodic, void *ctxt, twheel_cb_t cb)
{
    twentry->timer_id_ = timer_id;
    twentry->timeout_ = timeout;
    twentry->periodic_ = periodic;
    twentry->ctxt_ = ctxt;
    twentry->cb_ = cb;
    twentry->nspins_ = timeout/(nslices_ * slice_intvl_);
    twentry->slice_ = (curr_slice_ + (timeout % slice_intvl_)) % nslices_;
    twentry->next_ = twentry->prev_ = NULL;
}

//------------------------------------------------------------------------------
// add a timer entry to the wheel
//------------------------------------------------------------------------------
void *
twheel::add_timer(uint32_t timer_id, uint64_t timeout, void *ctxt,
                  twheel_cb_t cb, bool periodic)
{
    twentry_t    *twentry;

    twentry = static_cast<twentry_t *>(this->twentry_slab_->alloc());
    if (twentry == NULL) {
        return NULL;
    }
    init_twentry_(twentry, timer_id, timeout, periodic, ctxt, cb);

    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&twheel_[twentry->slice_].slock_);
    }
    insert_timer_(twentry);
    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&twheel_[twentry->slice_].slock_);
    }

    return twentry;
}


//------------------------------------------------------------------------------
// delete timer entry from the timer wheel and return the application context
//------------------------------------------------------------------------------
void *
twheel::del_timer(void *timer)
{
    twentry_t    *twentry;
    void         *ctxt;

    if (timer == NULL) {
        return NULL;
    }
    twentry = static_cast<twentry_t *>(timer);
    ctxt = twentry->ctxt_;

    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&twheel_[twentry->slice_].slock_);
    }
    remove_timer_(twentry);
    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&twheel_[twentry->slice_].slock_);
    }
    twentry_slab_->free(twentry);

    return ctxt;
}

//------------------------------------------------------------------------------
// update a given timer wheel entry
//------------------------------------------------------------------------------
void *
twheel::upd_timer(void *timer, uint64_t timeout, bool periodic)
{
    twentry_t        *twentry;

    if (timer == NULL) {
        return NULL;
    }
    twentry = static_cast<twentry_t *>(timer);

    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&twheel_[twentry->slice_].slock_);
    }
    // remove this entry from current slice
    remove_timer_(twentry);
    // re-init with updated params
    init_twentry_(twentry, twentry->timer_id_, timeout,
                  periodic, twentry->ctxt_, twentry->cb_);
    // re-insert in the right slice
    insert_timer_(twentry);
    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&twheel_[twentry->slice_].slock_);
    }
    return twentry;
}

//------------------------------------------------------------------------------
// timer wheel tick routine that drives the wheel, expected to be called by user
// of the timer wheel instance (ideally once every tick), tick is assumed  to be
// same as timer wheel slice interval
//
// TODO: don't use clock_gettime() etc. instead get tick count from the tick
//       driver
//------------------------------------------------------------------------------
void
twheel::tick(uint32_t msecs_elapsed)
{
    uint32_t                  nslices = 0;
    twentry_t                 *twentry, *next_entry;

    // check if full slice interval has elapsed since last invokation
    if (msecs_elapsed < slice_intvl_) {
        return;
    }

    // compute the number of slices to walk from current slice
    nslices = msecs_elapsed/slice_intvl_;
    HAL_ASSERT(nslices >= 1);

    // process all the timer events from current slice
    do {
        if (thread_safe_) {
            HAL_SPINLOCK_LOCK(&twheel_[curr_slice_].slock_);
        }
        twentry = twheel_[curr_slice_].slice_head_;
        while (twentry) {
            if (twentry->nspins_) {
                // revisit this after one more full spin
                twentry->nspins_ -= 1;
                twentry = twentry->next_;
            } else {
            sleep(1);
                twentry->cb_(twentry->timer_id_, twentry->ctxt_);
                if (twentry->periodic_) {
                    next_entry = twentry->next_;
                    // re-insert this timer
                    upd_timer_(twentry, twentry->timeout_, true);
                    twentry = next_entry;
                } else {
                    // delete this timer
                    remove_timer_(twentry);
                    twentry_slab_->free(twentry);
                    // pick the next one
                    twentry = twheel_[curr_slice_].slice_head_;
                }
            }
        }
        if (thread_safe_) {
            HAL_SPINLOCK_UNLOCK(&twheel_[curr_slice_].slock_);
        }
        curr_slice_ = (curr_slice_ + 1)%nslices_;
    } while (--nslices);
}

}    // namespace hal
}    // namespace utils

