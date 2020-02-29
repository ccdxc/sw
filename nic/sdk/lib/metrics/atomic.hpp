// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#ifndef _SDK_METRICS_SHM_ATOMIC_H_
#define _SDK_METRICS_SHM_ATOMIC_H_

#include <thread>

#define USE_PTHREAD 1

#ifdef USE_PTHREAD
#include <pthread.h>
#endif // USE_PTHREAD

namespace sdk {
namespace metrics {

static inline void pause() {
// Not sure how to detect if pause is available on the platform.
#if defined(_X86_)
  __asm__ ( "pause;" );
#else
  std::this_thread::sleep_for(std::chrono::seconds(0));
#endif
}

// atomic_increment increments a 32bit counter atomically in a shared memory
static inline int atomic_increment(volatile int *ptr) {
    return __atomic_fetch_add(ptr, 1, __ATOMIC_SEQ_CST);
}

// atomic_decrement decrements a 32bit counter atomically in a shared memory
static inline int atomic_decrement(volatile int *ptr) {
    return __atomic_fetch_add(ptr, -1, __ATOMIC_SEQ_CST);
}

// atomic_compare_and_swap does atomic compare and swap on shared memory
static inline bool atomic_compare_and_swap(volatile int *ptr, int oldval, int newval) {
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

// spin_lock_init initializes a spin lock
static inline void spin_lock_init(volatile int *ptr) {
#ifdef USE_PTHREAD
    pthread_spin_init(ptr, PTHREAD_PROCESS_SHARED);
#else // USE_PTHREAD
    *ptr = 0;
#endif // USE_PTHREAD
}

// spin_lock acquires a lock
static inline void spin_lock(volatile int *ptr) {
#ifdef USE_PTHREAD
    pthread_spin_lock(ptr);
#else // USE_PTHREAD
    if (*ptr != 0) {
        while (*ptr != 0) {
            pause();
        }
    }

    bool ok = false;
    while (!ok) {
        ok = atomic_compare_and_swap(ptr, 0, 1);
        if (!ok) {
            pause();
        }
    }
#endif // USE_PTHREAD
}

// spin_unlock releases a lock
static inline void spin_unlock(volatile int *ptr) {
#ifdef USE_PTHREAD
    pthread_spin_unlock(ptr);
#else // USE_PTHREAD
    assert(*ptr == 1);
    bool ok = atomic_compare_and_swap(ptr, 1, 0);
    assert(ok);
#endif // USE_PTHREAD
}

} // namespace metrics
} // namespace sdk

#endif // _SDK_METRICS_SHM_ATOMIC_H_
