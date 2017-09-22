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
#include <base.h>

typedef pthread_spinlock_t    hal_spinlock_t;

#define HAL_SPINLOCK_INIT(slock, mtype)       pthread_spin_init((slock), mtype)
#define HAL_SPINLOCK_DESTROY(slock)           pthread_spin_destroy((slock))
#define HAL_SPINLOCK_LOCK(slock)              pthread_spin_lock((slock))
#define HAL_SPINLOCK_UNLOCK(slock)            pthread_spin_unlock((slock))

//------------------------------------------------------------------------------
// writer preferred read-write lock implemented using spinlocks ... this lock is
// safe for use by real-time (FTE threads) and control threads (like cfg
// thread). if there is a writer waiting (on active readers) for the lock, no
// new reader coming after the writer will acquire the lock, this avoid starving
// of the writer forever
//------------------------------------------------------------------------------
class wp_rwlock {
public:
    wp_rwlock() {
        HAL_SPINLOCK_INIT(&mutex_, PTHREAD_PROCESS_PRIVATE);
        nwriters_ = nreaders_ = 0;
    }

    ~wp_rwlock() { HAL_SPINLOCK_DESTROY(&mutex_); }

    void rlock(void) {
        uint32_t wcount;

        do {
            HAL_SPINLOCK_LOCK(&mutex_);
            HAL_ATOMIC_LOAD_UINT32(&nwriters_, &wcount);
            if (wcount) {
                HAL_SPINLOCK_UNLOCK(&mutex_);
            } else {
                break;
            }
        } while (true);
        HAL_ATOMIC_INC_UINT32(&nreaders_, 1);
        HAL_SPINLOCK_UNLOCK(&mutex_);
    }

    void runlock(void) {
        HAL_SPINLOCK_LOCK(&mutex_);
        HAL_ATOMIC_DEC_UINT32(&nreaders_, 1);
        HAL_SPINLOCK_UNLOCK(&mutex_);
    }

    void wlock(void) {
		uint32_t rcount;

		HAL_ATOMIC_INC_UINT32(&nwriters_, 1);
		do {
		    HAL_SPINLOCK_LOCK(&mutex_);
		    HAL_ATOMIC_LOAD_UINT32(&nreaders_, &rcount);
		    if (rcount) {
		        HAL_SPINLOCK_UNLOCK(&mutex_);
		    } else {
		        break;
		    }
		} while (true);
    }

    void wunlock(void) {
        HAL_ATOMIC_DEC_UINT32(&nwriters_, 1);
        HAL_SPINLOCK_UNLOCK(&mutex_);
    }

private:
    hal_spinlock_t    mutex_;    // used for mutual exclusion between writers, readers
    uint32_t          nwriters_; // no. of writers that are active or waiting
    uint32_t          nreaders_; // no. of active readers
};

#endif    // __HAL_LOCK_HPP__

