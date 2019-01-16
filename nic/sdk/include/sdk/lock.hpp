//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// SDK lock primitives
//------------------------------------------------------------------------------

#ifndef __SDK_LOCK_HPP__
#define __SDK_LOCK_HPP__

#include <pthread.h>
#include "include/sdk/base.hpp"

namespace sdk {

typedef pthread_spinlock_t    sdk_spinlock_t;

#define SDK_SPINLOCK_INIT(slock, mtype)       pthread_spin_init((slock), mtype)
#define SDK_SPINLOCK_DESTROY(slock)           pthread_spin_destroy((slock))
#define SDK_SPINLOCK_LOCK(slock)              pthread_spin_lock((slock))
#define SDK_SPINLOCK_UNLOCK(slock)            pthread_spin_unlock((slock))

//------------------------------------------------------------------------------
// writer preferred read-write lock implemented using spinlocks ... this lock is
// safe for use by real-time (FTE threads) and control threads (like cfg
// thread). if there is a writer waiting (on active readers) for the lock, no
// new reader coming after the writer will acquire the lock, this avoids
// starving of the writer forever
//------------------------------------------------------------------------------
class wp_rwlock {
public:
    wp_rwlock() {
        SDK_SPINLOCK_INIT(&mutex_, PTHREAD_PROCESS_SHARED);
        nwriters_ = nreaders_ = 0;
    }

    ~wp_rwlock() { SDK_SPINLOCK_DESTROY(&mutex_); }

    void rlock(void) {
        uint32_t wcount;

        do {
            SDK_SPINLOCK_LOCK(&mutex_);
            SDK_ATOMIC_LOAD_UINT32(&nwriters_, &wcount);
            if (wcount) {
                SDK_SPINLOCK_UNLOCK(&mutex_);
            } else {
                break;
            }
        } while (true);
        SDK_ATOMIC_INC_UINT32(&nreaders_, 1);
        SDK_SPINLOCK_UNLOCK(&mutex_);
    }

    void runlock(void) {
        SDK_SPINLOCK_LOCK(&mutex_);
        SDK_ATOMIC_DEC_UINT32(&nreaders_, 1);
        SDK_SPINLOCK_UNLOCK(&mutex_);
    }

    void wlock(void) {
        uint32_t rcount;

        SDK_ATOMIC_INC_UINT32(&nwriters_, 1);
        do {
            SDK_SPINLOCK_LOCK(&mutex_);
            SDK_ATOMIC_LOAD_UINT32(&nreaders_, &rcount);
            if (rcount) {
                SDK_SPINLOCK_UNLOCK(&mutex_);
            } else {
                break;
            }
        } while (true);
    }

    void wunlock(void) {
        SDK_ATOMIC_DEC_UINT32(&nwriters_, 1);
        SDK_SPINLOCK_UNLOCK(&mutex_);
    }

private:
    sdk_spinlock_t    mutex_;    // used for mutual exclusion between writers, readers
    uint32_t          nwriters_; // no. of writers that are active or waiting
    uint32_t          nreaders_; // no. of active readers
};
}    // namespace sdk

using sdk::sdk_spinlock_t;

#endif    // __SDK_LOCK_HPP__

