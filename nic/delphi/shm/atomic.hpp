// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SHM_ATOMIC_H_
#define _DELPHI_SHM_ATOMIC_H_

namespace delphi {
namespace shm {

static inline void pause() {
// Not sure how to detect if pause is available on the platform.
#if defined(_X86_)
  __asm__ ( "pause;" );
#else
  std::this_thread::sleep_for(std::chrono::seconds(0));
#endif
}

// atomic_increment increments a 32bit counter atomically in a shared memory
static inline void atomic_increment(volatile int32_t *ptr) {
    __sync_fetch_and_add(ptr, 1);
}

// atomic_decrement decrements a 32bit counter atomically in a shared memory
static inline void atomic_decrement(volatile int32_t *ptr) {
    __sync_fetch_and_sub(ptr, 1);
}

// atomic_compare_and_swap does atomic compare and swap on shared memory
static inline bool atomic_compare_and_swap(volatile int32_t *ptr, int32_t oldval, int32_t newval) {
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

// spin_lock_init initializes a spin lock
static inline void spin_lock_init(volatile int32_t *ptr) {
    *ptr = 0;
}

// spin_lock acquires a lock
static inline void spin_lock(volatile int32_t *ptr) {
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
}

// spin_unlock releases a lock
static inline void spin_unlock(volatile int32_t *ptr) {
    assert(*ptr == 1);
    bool ok = atomic_compare_and_swap(ptr, 1, 0);
    assert(ok);
}

} // namespace shm
} // namespace delphi

#endif // _DELPHI_SHM_ATOMIC_H_
