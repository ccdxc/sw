//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// SDK lock primitives
//------------------------------------------------------------------------------

#ifndef __SDK_LOCK_HPP__
#define __SDK_LOCK_HPP__

#include <pthread.h>
#include "sdk/base.hpp"

namespace sdk {

typedef pthread_spinlock_t    sdk_spinlock_t;

#define SDK_SPINLOCK_INIT(slock, mtype)       pthread_spin_init((slock), mtype)
#define SDK_SPINLOCK_DESTROY(slock)           pthread_spin_destroy((slock))
#define SDK_SPINLOCK_LOCK(slock)              pthread_spin_lock((slock))
#define SDK_SPINLOCK_UNLOCK(slock)            pthread_spin_unlock((slock))

}    // namespace sdk

#endif    // __SDK_LOCK_HPP__

