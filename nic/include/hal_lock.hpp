//------------------------------------------------------------------------------
// HAL lock related APIs
// TODO: the macros in this file are supposed to hide implementation detail
//       until actual locking primitives are decided, main logic will use these
//       wrappers so it doesn't have to change when a decision is made... for
//       now, pthread spinlocks are used !!
//------------------------------------------------------------------------------

#ifndef __HAL_LOCK_HPP__
#define __HAL_LOCK_HPP__

#include <pthread.h>

typedef pthread_spinlock_t    hal_spinlock_t;

#define HAL_SPINLOCK_INIT(slock, mtype)       pthread_spin_init((slock), mtype)
#define HAL_SPINLOCK_DESTROY(slock)           pthread_spin_destroy((slock))
#define HAL_SPINLOCK_LOCK(slock)              pthread_spin_lock((slock))
#define HAL_SPINLOCK_UNLOCK(slock)            pthread_spin_unlock((slock))

#endif    // __HAL_LOCK_HPP__

