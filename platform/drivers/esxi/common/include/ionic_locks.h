/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_locks.h --
 *
 * Implement all lock related functions
 */

#ifndef _IONIC_LOCK_H_
#define _IONIC_LOCK_H_

#include <vmkapi.h>

typedef enum ionic_lock_rank {
        IONIC_LOCK_RANK_NORMAL    = 0x1,
        IONIC_LOCK_RANK_HIGH      = 0x2,
} ionic_lock_rank;

VMK_ReturnStatus
ionic_spinlock_create(const char *const name,
                      vmk_ModuleID module_id,
                      vmk_HeapID heap_id,
                      vmk_LockDomainID domain,
                      vmk_SpinlockType type,
                      ionic_lock_rank rank,
                      vmk_Lock *const lock);

void
ionic_spinlock_destroy(vmk_Lock lock);

VMK_ReturnStatus
ionic_mutex_create(const char *const name,
                   vmk_ModuleID module_id,
                   vmk_HeapID heap_id,
                   vmk_LockDomainID domain,
                   vmk_SpinlockType type,
                   ionic_lock_rank rank,
                   vmk_Mutex *const mutex);

void
ionic_mutex_destroy(vmk_Mutex mutex);

VMK_ReturnStatus
ionic_binary_sema_create(vmk_HeapID heap_id,
                         const char *name,
                         vmk_Semaphore *lock);

void
ionic_sema_destroy(vmk_Semaphore *lock);

#endif /* End of _IONIC_LOCK_H_ */
