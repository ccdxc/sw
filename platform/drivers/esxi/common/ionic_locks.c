/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
 * ionic_lock.c --
 *
 * Implement all lock related functions
 */

#include "ionic_log.h"
#include "ionic_locks.h"

/*
 ******************************************************************************
 *
 * ionic_spinlock_create --
 *
 *    Initialize a spinlock
 *
 *  Parameters:
 *     name      - IN (name for the spinlock)
 *     module_id - IN (module ID that associates with the spinlock)
 *     heap_id   - IN (heap ID the spinlock will be allocated from)
 *     domain    - IN (domain of spinlock, VMK_LOCKDOMAIN_INVALID if unranked)
 *     type      - IN (type of spinlock)
 *     rank      - IN (lock ranking)
 *     lock      - OUT (lock to be created)
 *
 *  Results:
 *     VMK_OK  - Success, returns lock pointer.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_spinlock_create(const char *const name,                     // IN
                      vmk_ModuleID module_id,                     // IN
                      vmk_HeapID heap_id,                         // IN
                      vmk_LockDomainID domain,                    // IN
                      vmk_SpinlockType type,                      // IN
                      ionic_lock_rank rank,                       // IN
                      vmk_Lock *const lock)                       // OUT
{
        VMK_ReturnStatus status;
        vmk_SpinlockCreateProps spinlock_props;

        VMK_ASSERT(name && lock);

        vmk_Memset(&spinlock_props, 0, sizeof(vmk_SpinlockCreateProps));
        spinlock_props.moduleID   = module_id;
        spinlock_props.heapID     = heap_id;
        spinlock_props.type       = type;
        spinlock_props.domain     = (rank == VMK_SPINLOCK_UNRANKED) ?
                                    VMK_LOCKDOMAIN_INVALID : domain;
        spinlock_props.rank       = rank;

        status = vmk_NameInitialize(&spinlock_props.name, name);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_NameInitialize() failed: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_SpinlockCreate(&spinlock_props, lock);
        if (status != VMK_OK) {
                ionic_err("vmk_SpinlockCreate() failed: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_spinlock_destroy --
 *
 *    Destroy the given spinlock
 *
 *  Parameters:
 *     lock    - IN (lock to be destroyed)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_spinlock_destroy(vmk_Lock lock)                             // IN
{
        VMK_ASSERT(lock);
        vmk_SpinlockDestroy(lock);
}


/*
 ******************************************************************************
 *
 * ionic_mutex_create --
 *
 *    Initialize a mutex
 *
 *  Parameters:
 *     name      - IN (name for the mutex)
 *     module_id - IN (module ID that associates with the mutex)
 *     heap_id   - IN (heap ID the mutex will be allocated from)
 *     domain    - IN (domain of mutex, VMK_LOCKDOMAIN_INVALID if unranked)
 *     type      - IN (type of mutex)
 *     rank      - IN (lock ranking)
 *     lock      - OUT (lock to be created)
 *
 *  Results:
 *     VMK_OK  - Success, returns lock pointer.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_mutex_create(const char *const name,                        // IN
                   vmk_ModuleID module_id,                        // IN
                   vmk_HeapID heap_id,                            // IN
                   vmk_LockDomainID domain,                       // IN
                   vmk_SpinlockType type,                         // IN
                   ionic_lock_rank rank,                          // IN
                   vmk_Mutex *const mutex)                        // OUT
{
        VMK_ReturnStatus status;
        vmk_MutexCreateProps mutex_props;

        VMK_ASSERT(name && mutex);

        vmk_Memset(&mutex_props, 0, sizeof(vmk_MutexCreateProps));
        mutex_props.moduleID   = module_id;
        mutex_props.heapID     = heap_id;
        mutex_props.type       = type;
        mutex_props.domain     = domain;
        mutex_props.rank       = rank;

        status = vmk_NameInitialize(&mutex_props.className, name);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_NameInitialize() failed: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_MutexCreate(&mutex_props, mutex);
        if (status != VMK_OK) {
                ionic_err("vmk_MutexCreate() failed: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_mutex_destroy --
 *
 *    Destroy the given mutex
 *
 *  Parameters:
 *     lock    - IN (mutex to be destroyed)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_mutex_destroy(vmk_Mutex mutex)                              // IN
{
        VMK_ASSERT(mutex);
        vmk_MutexDestroy(mutex);
}


/*
 ******************************************************************************
 *
 * ionic_binary_sema_create --
 *
 *    Create and initialize a binary semaphore
 *
 *  Parameters:
 *     heap_id   - IN (heap id where the binary semaphore allocating from)
 *     name      - IN (name of the binary semaphore)
 *     lock      - OUT (lock pointer to be created)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_binary_sema_create(vmk_HeapID heap_id,                      // IN
                         const char *name,                        // IN
                         vmk_Semaphore *lock)                     // OUT
{
        VMK_ReturnStatus status;

        VMK_ASSERT(lock);

        status = vmk_BinarySemaCreate(lock,
                                      heap_id,
                                      name);
        if (status != VMK_OK) {
                ionic_err("vmk_BinarySemaCreate() failed, status: %s",
                          vmk_StatusToString(status));
        }   

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_sema_destroy --
 *
 *    Destroy a semaphore
 *
 *  Parameters:
 *     lock    - IN (lock pointer to be destroyed)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_sema_destroy(vmk_Semaphore *lock)                           // IN
{
        VMK_ASSERT(lock);
        vmk_SemaDestroy(lock);
}

