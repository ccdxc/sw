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
 * ionic_work_queue.c --
 *
 * Implement functions that used for work queue
 */

#include "ionic_work_queue.h"
#include "ionic_log.h"
#include "ionic_memory.h"
#include "ionic_locks.h"

/*
 ******************************************************************************
 *
 * ionic_work_init --
 *
 *     Initialize ionic work struct
 *
 *  Parameters:
 *     work       - IN (work item to initialize)
 *     fun        - IN (function to initialize with)
 *     data       - IN (argument for the work function)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void
ionic_work_init(struct ionic_work *work,                          // IN
                vmk_HelperRequestFunc fun,                        // IN
                void *data)                                       // IN
{
        work->queue = NULL;
        work->fun = fun;
        work->fun_data.ptr = data;
}


/*
 ******************************************************************************
 *
 * ionic_work_queue_tag_compare --
 *
 *     Compare tags for helper queue
 *
 *  Parameters:
 *     tag1     - IN (first tag to compare)
 *     tag2     - IN (second tag to compare)
 *
 *  Results:
 *     VMK_TRUE if tags match, VMK_FALSE otherwise
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static vmk_Bool
ionic_work_queue_tag_compare(void *tag1,                          // IN
                             void *tag2)                          // IN
{
        return (tag1 == tag2);
}


/*
 ******************************************************************************
 *
 * ionic_work_queue_create --
 *
 *     Create single work queue
 *
 *  Parameters:
 *     heap_id     - IN (heap the workqueue can use for allocations)
 *     module_id   - IN (module ID associated with this workqueue)
 *     name        - IN (name associated with this workqueue)
 *
 *  Results:
 *     ionic_work_queue or NULL on failure
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

struct ionic_work_queue *
ionic_work_queue_create(vmk_HeapID heap_id,                       // IN
                        vmk_ModuleID module_id,                   // IN
                        const char *name)                         // IN
{
        VMK_ReturnStatus status;
        struct ionic_work_queue *work_queue;
        vmk_HelperProps helper_props;

        work_queue = ionic_heap_zalloc(heap_id,
                                       sizeof(struct ionic_work_queue));
        if (VMK_UNLIKELY(!work_queue)) {
                ionic_err("ionic_heap_zalloc() failed, status: NO MEMORY");
                return NULL;
        }

        work_queue->heap_id = heap_id;

        vmk_Memset(&helper_props, 0, sizeof(helper_props));

        status = vmk_NameInitialize(&helper_props.name, name) |
                 vmk_NameInitialize(&work_queue->name, name);
        if (status != VMK_OK) {
                ionic_err("vmk_NameInitialize() failed, status: %s",
                          vmk_StatusToString(status));
                goto name_init_err;
        }

        status = ionic_spinlock_create("work_queue->flush_lock",
                                       module_id,
                                       heap_id,
                                       VMK_LOCKDOMAIN_INVALID,
                                       VMK_SPINLOCK,
                                       VMK_SPINLOCK_UNRANKED,
                                       &(work_queue->flush_lock));
        if (status != VMK_OK) {
                ionic_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto name_init_err;
        }

        work_queue->is_flush = VMK_FALSE;

        helper_props.heap             = heap_id;
        helper_props.preallocRequests = VMK_TRUE;
        helper_props.blockingSubmit   = VMK_FALSE;
        helper_props.maxRequests      = IONIC_WORK_QUEUE_MAX_REQUESTS;

        helper_props.mutables.minWorlds           = 1;
        helper_props.mutables.maxWorlds           = 1;
        helper_props.mutables.maxIdleTime         = 0;
        helper_props.mutables.maxRequestBlockTime = 0;

        helper_props.tagCompare         = ionic_work_queue_tag_compare;
        helper_props.constructor        = NULL;
        helper_props.constructorArg.ptr = NULL;

        status = vmk_HelperCreate(&helper_props, &work_queue->helper_fun);
        if (status != VMK_OK) {
                ionic_err("vmk_HelperCreate() failed, status: %s",
                          vmk_StatusToString(status));
                goto helper_err;
        }

        return work_queue;

helper_err:
        ionic_spinlock_destroy(work_queue->flush_lock);

name_init_err:
        ionic_heap_free(heap_id, work_queue);

        return NULL;
}

/*
 ******************************************************************************
 *
 * ionic_work_queue_destroy --
 *
 *     Destroy workqueue
 *
 *  Parameters:
 *     work_queue     - IN (queue to destroy)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void
ionic_work_queue_destroy(struct ionic_work_queue *work_queue)     // IN
{
        if (work_queue) {
                vmk_HelperDestroy(work_queue->helper_fun);
                ionic_spinlock_destroy(work_queue->flush_lock);
                ionic_heap_free(work_queue->heap_id, work_queue);
        }
}


/*
 ******************************************************************************
 *
 * ionic_work_queue_submit_safe --
 *
 *     Submit a request to a work queue, assume it is safe
 *     to do it as flush is not being done on this queue.
 *
 *  Parameters:
 *     work_queue      - IN (work queue to send the request to)
 *     work            - IN (work to submit)
 *     delay_ms        - IN (delay in ms for delayed work)
 *
 *  Results:
 *     vmk_ReturnStatus
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_work_queue_submit_safe(struct ionic_work_queue *work_queue,       // IN
                             struct ionic_work *work,                   // IN
                             vmk_uint32 delay_ms)                       // IN
{
        VMK_ReturnStatus status;
        vmk_HelperRequestProps request_props;

        vmk_HelperRequestPropsInit(&request_props);

        request_props.tag.ptr = work;
        work->queue = work_queue;

        if (delay_ms == 0) {
                status = vmk_HelperSubmitRequest(work_queue->helper_fun,
                                                 work->fun,
                                                 work->fun_data,
                                                 &request_props);
        } else {
                status = vmk_HelperSubmitDelayedRequest(work_queue->helper_fun,
                                                        work->fun,
                                                        work->fun_data,
                                                        delay_ms,
                                                        &request_props);
        }

        if (status != VMK_OK) {
                ionic_err("Workqueue %s: vmk_HelperSubmit%sRequest failed - %s",
                          vmk_NameToString(&work_queue->name),
                          (delay_ms == 0) ? "" : "Delayed",
                          vmk_StatusToString(status));
                work->queue = NULL;
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_work_queue_submit_allowed --
 *
 *     Check if submission of a new work is allowed
 *
 *  Parameters:
 *     queue      - IN (work queue to send the request to)
 *
 *  Results:
 *     VMK_OK for submission allowed
 *     vmk_ReturnStatus code otherwise
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static inline VMK_ReturnStatus
ionic_work_queue_submit_allowed(struct ionic_work_queue *work_queue)     // IN
{
        VMK_ReturnStatus status;

        vmk_SpinlockLockIgnoreDeathPending(work_queue->flush_lock);
        status = (work_queue->is_flush ? VMK_BUSY : VMK_OK);
        vmk_SpinlockUnlock(work_queue->flush_lock);

        if (status == VMK_BUSY) {
                ionic_err("Workqueue %s: is busy...",
                          vmk_NameToString(&work_queue->name));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_work_queue_submit --
 *
 *     Submit a delayed request to a work queue
 *
 *  Parameters:
 *     work_queue      - IN (work queue to send the request to)
 *     work            - IN (work to submit)
 *     delay_ms        - IN (delay in milliseconds, 0 for no delay)
 *
 *  Results:
 *     vmk_ReturnStatus
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline VMK_ReturnStatus
ionic_work_queue_submit(struct ionic_work_queue *work_queue,       // IN
                        struct ionic_work *work,                   // IN
                        vmk_uint32 delay_ms)                       // IN
{
        VMK_ReturnStatus status;

        status = ionic_work_queue_submit_allowed(work_queue);
        if (status != VMK_OK) {
                return status;
        }

        return ionic_work_queue_submit_safe(work_queue, work, delay_ms);
}


/*
 ******************************************************************************
 *
 * ionic_work_queue_cancel --
 *
 *     Cancel a work request
 *
 *  Parameters:
 *     work       - IN (work to cancel)
 *
 *  Results:
 *     VMK_TRUE if the job was canceled, VMK_FALSE otherwise
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_Bool
ionic_work_queue_cancel(struct ionic_work *work)                  // IN
{
        VMK_ReturnStatus status;
        vmk_uint32 num = 0;

        status = vmk_HelperCancelRequest(work->queue->helper_fun,
                                         work,
                                         &num);
        if (status != VMK_OK) {
                ionic_err("Workqueue %s: vmk_HelperCancelRequest failed, "
                          " status: %s", vmk_NameToString(&work->queue->name),
                          vmk_StatusToString(status));
                num = 0;
        }

        return (num > 0);
}


/*
 ******************************************************************************
 *
 * ionic_work_queue_last_work --
 *
 *     Internal auxiliary function to be used by the
 *     ionic_work_queue_flush function.
 *
 *  Parameters:
 *     data     - IN (data parameter)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static inline void
ionic_work_queue_last_work(vmk_AddrCookie data)                   // IN
{
        vmk_AtomicWrite64(&((struct ionic_work_queue *)data.ptr)->waiting_last_req, 0);
}


/*
 ******************************************************************************
 *
 * ionic_work_queue_flush --
 *
 *     Flush single threaded work queue.
 *     Deny new works submissions to this queue during flushing,
 *     wait till all the existing requests have completed.
 *
 *  Parameters:
 *     work_queue     - IN (queue to flush)
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
ionic_work_queue_flush(struct ionic_work_queue *work_queue)       // IN
{
        VMK_ReturnStatus status;
        vmk_uint32 sleep_ms = 100;
        struct ionic_work last_work;

        vmk_WorldAssertIsSafeToBlock();

        /*
         * Setting the in_flush flag to true:
         *  - ensure that there is only one flushing in progress
         *  - block all the incoming work submissions during flushing
         */

        vmk_SpinlockLockIgnoreDeathPending(work_queue->flush_lock);
        while (work_queue->is_flush) {
                vmk_SpinlockUnlock(work_queue->flush_lock);
                vmk_WorldSleep(VMK_USEC_PER_MSEC * sleep_ms);
                vmk_SpinlockLockIgnoreDeathPending(work_queue->flush_lock);
        }
        work_queue->is_flush = VMK_TRUE;
        vmk_SpinlockUnlock(work_queue->flush_lock);

        /* wait till there is no more pending requests */

        while (vmk_HelperCurrentRequestCount(work_queue->helper_fun)) {
                vmk_WorldSleep(VMK_USEC_PER_MSEC);
        }

        /*
         * No more pending requests, but there are still running requests.
         * Create a purging job and wait for its completion.
         */

        vmk_AtomicWrite64(&work_queue->waiting_last_req, 1);

        ionic_work_init(&last_work, ionic_work_queue_last_work, work_queue);

        status = ionic_work_queue_submit_safe(work_queue, &last_work, 0);

        if (status != VMK_OK) {
                ionic_err("Workqueue %s: failed at submitting last work.",
                          vmk_NameToString(&work_queue->name));
                vmk_AtomicWrite64(&work_queue->waiting_last_req, 0);
                goto out;
        }

        while (vmk_AtomicRead64(&work_queue->waiting_last_req)) {
                vmk_WorldSleep(VMK_USEC_PER_MSEC);
        }

out:
        /*
         * flushing is done
         */

        vmk_SpinlockLockIgnoreDeathPending(work_queue->flush_lock);
        work_queue->is_flush = VMK_FALSE;
        vmk_SpinlockUnlock(work_queue->flush_lock);
}

