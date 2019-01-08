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
 * ionic_en_mq.c --
 *
 * Implementations for multi-queue 
 */

#include "ionic.h"

/*
 ******************************************************************************
 *
 * ionic_get_num_queues_supported --
 *
 *    Get number of queues that device driver should expose to vmkernel,
 *
 *  Parameters:
 *     max_tx_queues      - IN (number of TX queues device can support)
 *     max_rx_queues      - IN (number of RX queues device can support)
 *
 *  Results:
 *     Return maxium number of queues we should use. 
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

vmk_uint32
ionic_get_num_queues_supported(vmk_uint32 numTxQueues,                     //IN
                               vmk_uint32 numRxQueues)                     //IN
{
        vmk_uint32 max_tx_queues;
        vmk_uint32 max_rx_queues;

        vmk_UplinkQueueGetNumQueuesSupported(numTxQueues,
                                             numRxQueues,
                                             &max_tx_queues,
                                             &max_rx_queues);

        return IONIC_MIN(max_tx_queues, max_rx_queues);
}


/*
 ******************************************************************************
 *
 *   ionic_en_is_rss_q_idx
 *
 *      Check if index is of queue that supports RSS
 *      feature.
 *
 *  Parameters:
 *    priv_data  - IN (private driver data)
 *    idx        - IN (shared queue data index)
 *
 *  Results:
 *     VMK_TRUE  if provided index is of queue that supports
 *               RSS feature
 *     VMK_FALSE otherwise
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_Bool
ionic_en_is_rss_q_idx(struct ionic_en_priv_data *priv_data,       // IN
                      vmk_uint32 idx)                             // IN
{
        vmk_uint32 i;
        IONIC_EN_FOR_EACH_RX_RSS_SHARED_QUEUE_DATA_INDEX(priv_data, i) { 
                if (i == idx) {
                        return VMK_TRUE;
                }    
        }

        return VMK_FALSE;
}


/*
 ******************************************************************************
 *
 * ionic_en_is_default_q
 *
 *      check if uplink qid is the driver default queue
 *
 *   Parameters:
 *       vmk_UplinkQueueID - IN (uplink qid)
 *
 *   Results:
 *      VMK_TRUE or VMK_FALSE
 *
 *   Side effects:
 *     None
 *
 ******************************************************************************
 */

static inline vmk_Bool
ionic_en_is_default_q(vmk_UplinkQueueID uplink_qid)               // IN
{
        vmk_uint32 usr_qid = vmk_UplinkQueueIDUserVal(uplink_qid);

        return ((usr_qid == 0) ? VMK_TRUE : VMK_FALSE);
}


/*
 ******************************************************************************
 *
 *  ionic_en_convert_shared_q_idx 
 *
 *      Convert from shared queue data index to ring index.
 *      Each queue in queue data structure is associated with one ring
 *
 *  Parameters:
 *    priv_data                 - IN (struct ionic_en_priv_data handler)
 *    shared_q_data_idx         - IN (shared queue data index)
 *
 *  Results:
 *     index of a ring associated to given shared queue data index.
 *
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

vmk_uint32
ionic_en_convert_shared_q_idx(struct ionic_en_priv_data *priv_data,     // IN
                              vmk_uint32 shared_q_data_idx)             // IN
{
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_info("ionic_en_convert_shared_q_idx() called");

        /* Normal rx ring index */
        if (shared_q_data_idx < uplink_handle->max_rx_normal_queues) {
                return shared_q_data_idx;
        }

        /* RSS rx index */
        if (shared_q_data_idx < uplink_handle->max_rx_queues) {
                VMK_ASSERT(ionic_en_is_rss_q_idx(priv_data,
                                                 shared_q_data_idx));
                return shared_q_data_idx -
                       uplink_handle->max_rx_normal_queues;
        }

        /* Tx ring index */
        if ((shared_q_data_idx < uplink_handle->max_rx_queues +
                                 uplink_handle->max_tx_queues)) {
                return shared_q_data_idx - uplink_handle->max_rx_queues;
        }

        return IONIC_EN_INVALID_SHARED_QUEUE_DATA_INDEX;
}


/*
 ******************************************************************************
 *
 *  ionic_en_convert_attached_rss_ring_idx 
 *
 *      Convert from attached rss ring index to rx ring index.
 *
 *  Parameters:
 *    priv_data                 - IN (struct ionic_en_priv_data handler)
 *    rss_ring_idx              - IN (rss ring index)
 *    num_attached_rx_rings     - IN (number of attached rx rings to this
 *                                    rx rss ring)
 *    cur_rx_ring_idx           - IN (current index of attached rx ring
 *                                    of this rx rss ring)
 *
 *  Results:
 *     index of the dedicated rx_ring from the array.
 *
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_uint32
ionic_en_convert_attached_rss_ring_idx(struct ionic_en_priv_data *priv_data,     // IN
                                       vmk_uint32 rss_ring_idx,                  // IN
                                       vmk_uint32 num_attached_rx_rings,         // IN
                                       vmk_uint32 cur_rx_ring_idx)               // IN
{
        vmk_uint32 idx;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        idx = uplink_handle->max_rx_normal_queues +
              rss_ring_idx * num_attached_rx_rings +
              cur_rx_ring_idx;

        VMK_ASSERT(idx >= uplink_handle->max_rx_normal_queues &&
                   idx < uplink_handle->max_rx_queues);

        return idx;
}


/*
 ******************************************************************************
 *
 *  ionic_en_rxq_alloc
 *
 *     Allocate a RX Queue
 *
 *  Uplink Shared Queue Data is flat, so index ranges are documented below:
 *
 *  RX Queue index for shared queue data:
 *    From: 0
 *    TO  : uplink_handle->max_rx_queues - 1
 *
 *  Parameters:
 *     priv_data     - IN (private driver data)
 *     uplink_qid    - OUT (pointer to Uplink queue id)
 *     netpoll       - OUT (pointer to Netpoll context)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_rxq_alloc(struct ionic_en_priv_data *priv_data,          // IN
                   vmk_UplinkQueueID *uplink_qid,                 // OUT
                   struct vmk_NetPollInt **netpoll)               // OUT
{
        VMK_ReturnStatus status = VMK_OK;
        vmk_uint32 flags, found_idx, rx_ring_idx, i;
        vmk_Bool is_found = VMK_FALSE;


        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_UplinkSharedQueueData *uplink_q_data     = uplink_handle->uplink_q_data;

        ionic_info("ionic_en_rxq_alloc() called");

        IONIC_EN_FOR_EACH_RX_SHARED_QUEUE_DATA_INDEX(priv_data, i) {
                if ((uplink_q_data[i].flags & VMK_UPLINK_QUEUE_FLAG_IN_USE) == 0) {
                        found_idx = i;
                        is_found = VMK_TRUE;
                        break;
                }
        }

        if (!is_found) {
                ionic_err("Failed to allocate RX queue, status: VMK_NOT_FOUND");
                return VMK_NOT_FOUND;
        }

        IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle,
                                           found_idx,
                                           VMK_UPLINK_QUEUE_TYPE_RX);

        rx_ring_idx = ionic_en_convert_shared_q_idx(priv_data, found_idx);

        status = vmk_UplinkQueueMkRxQueueID(uplink_qid, found_idx, rx_ring_idx);
        VMK_ASSERT(status == VMK_OK);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        *netpoll = uplink_q_data[found_idx].poll;

        flags = ionic_en_is_default_q(*uplink_qid) ?
                (VMK_UPLINK_QUEUE_FLAG_IN_USE | VMK_UPLINK_QUEUE_FLAG_DEFAULT) :
                VMK_UPLINK_QUEUE_FLAG_IN_USE;

        uplink_q_data[found_idx].flags &= ~(VMK_UPLINK_QUEUE_FLAG_DEFAULT |
                                            VMK_UPLINK_QUEUE_FLAG_IN_USE);
        uplink_q_data[found_idx].flags |= flags;
        uplink_q_data[found_idx].qid = *uplink_qid;

        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        ionic_info("RX queue %d is allocated", rx_ring_idx);

        return status;
}


/*
 ******************************************************************************
 *
 *  ionic_en_rxq_free
 *
 *     Free a RX Queue
 *
 *  Parameters:
 *     priv_data     - IN (ionic_en_priv_data handle)
 *     uplink_qid    - IN (pointer to uplink queue id)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_rxq_free(struct ionic_en_priv_data *priv_data,           // IN
                  vmk_UplinkQueueID uplink_qid)                   // IN
{  
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_uint32 uplink_q_data_idx;
        vmk_UplinkSharedQueueData *uplink_q_data;

        ionic_info("ionic_en_rxq_free() called");

        uplink_q_data_idx = vmk_UplinkQueueIDQueueDataIndex(uplink_qid);

        IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle,
                                           uplink_q_data_idx,
                                           VMK_UPLINK_QUEUE_TYPE_RX);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        uplink_q_data = &uplink_handle->uplink_q_data[uplink_q_data_idx];

        uplink_q_data->flags &= ~(VMK_UPLINK_QUEUE_FLAG_IN_USE |
                                  VMK_UPLINK_QUEUE_FLAG_DEFAULT);

        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        ionic_info("RX queue %d is freed",
                   vmk_UplinkQueueIDUserVal(uplink_qid));

        return VMK_OK;
}


/*
 ******************************************************************************
 *
 *  ionic_en_txq_alloc
 *
 *     Allocate a TX Queue
 *
 *  Uplink Shared Queue Data is flat, so index ranges are documented below:
 *
 *  TX Queue indexes for shared queue data:
 *    From: uplink_handle->max_rx_queues
 *    To  : uplink_handle->max_rx_queues + uplink_handle->max_tx_queues -1
 *
 *  Parameters:
 *     priv_data     - IN (driver private data)
 *     uplink_qid    - OUT (pointer to Uplink queue ID)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_txq_alloc(struct ionic_en_priv_data *priv_data,          // IN
                   vmk_UplinkQueueID *uplink_qid)                 // OUT
{
        VMK_ReturnStatus status = VMK_OK;
        vmk_Bool is_found = VMK_FALSE;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_UplinkSharedQueueData *uplink_q_data     = uplink_handle->uplink_q_data;
        vmk_uint32 flags, found_idx, tx_ring_idx, i;

        ionic_info("ionic_en_txq_alloc() called");

        IONIC_EN_FOR_EACH_TX_SHARED_QUEUE_DATA_INDEX(priv_data, i) {
                if ((uplink_q_data[i].flags & VMK_UPLINK_QUEUE_FLAG_IN_USE) == 0) {
                        found_idx = i;
                        is_found = VMK_TRUE;
                        break;
                }
        }

        if (!is_found) {
                ionic_info("Failed to allocate TX queue, status: VMK_NOT_FOUND");
                return  VMK_NOT_FOUND;
        }

        IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle,
                                           found_idx,
                                           VMK_UPLINK_QUEUE_TYPE_TX);

        tx_ring_idx = ionic_en_convert_shared_q_idx(priv_data, found_idx);

        status = vmk_UplinkQueueMkTxQueueID(uplink_qid, found_idx, tx_ring_idx);
        VMK_ASSERT(status == VMK_OK);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        flags = ionic_en_is_default_q(*uplink_qid) ?
                (VMK_UPLINK_QUEUE_FLAG_IN_USE | VMK_UPLINK_QUEUE_FLAG_DEFAULT) :
                VMK_UPLINK_QUEUE_FLAG_IN_USE;

        uplink_q_data[found_idx].flags &= ~(VMK_UPLINK_QUEUE_FLAG_DEFAULT |
                                            VMK_UPLINK_QUEUE_FLAG_IN_USE);
        uplink_q_data[found_idx].flags |= flags;
        uplink_q_data[found_idx].qid = *uplink_qid;

        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        ionic_info("TX queue %d is allocated", tx_ring_idx);

        return status;
}


/*
 ******************************************************************************
 *
 *  ionic_en_txq_free
 *
 *     Free a TX Queue
 *
 *  Parameters:
 *     priv_data     - IN (driver private data)
 *     uplink_qid    - IN (pointer to uplink queue id)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_txq_free(struct ionic_en_priv_data *priv_data,           // IN
                  vmk_UplinkQueueID uplink_qid)                   // IN
{
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_uint32 uplink_q_data_idx;
        vmk_UplinkSharedQueueData *uplink_q_data;

        ionic_info("ionic_en_txq_free() called");

        uplink_q_data_idx = vmk_UplinkQueueIDQueueDataIndex(uplink_qid);

        IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle,
                                           uplink_q_data_idx,
                                           VMK_UPLINK_QUEUE_TYPE_TX);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        uplink_q_data = &uplink_handle->uplink_q_data[uplink_q_data_idx];

        uplink_q_data->flags &= ~(VMK_UPLINK_QUEUE_FLAG_IN_USE |
                                  VMK_UPLINK_QUEUE_FLAG_DEFAULT);

        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        ionic_info("TX queue %d is freed",
                   vmk_UplinkQueueIDUserVal(uplink_qid));

        return VMK_OK;
}


/*
 ******************************************************************************
 *
 * ionic_en_default_q_alloc
 *
 *     Allocates default Tx and Rx queues
 *
 *  Parameters:
 *     priv_data     : IN (driver private data)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_default_q_alloc(struct ionic_en_priv_data *priv_data)    // IN
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_UplinkSharedQueueInfo *queue_info        = &uplink_handle->uplink_q_info;
        vmk_NetPoll dummy_poll;

        status = ionic_en_rxq_alloc(priv_data,
                                    &queue_info->defaultRxQueueID,
                                    &dummy_poll);
        if (status != VMK_OK) {
                ionic_err("ionic_en_rxq_alloc failed for default queue, "
                          "status: %s", vmk_StatusToString(status));
                return status;
        }

        status = ionic_en_txq_alloc(priv_data,
                                    &queue_info->defaultTxQueueID);
        if (status != VMK_OK) {
                ionic_err("ionic_en_txq_alloc failed for default queue, "
                          "status: %s", vmk_StatusToString(status));
                goto txq_default_err;
        }

        return status;

txq_default_err:
        ionic_en_rxq_free(priv_data,
                          queue_info->defaultRxQueueID);

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_default_q_free
 *
 *     Frees default Tx and Rx queues
 *
 *  Parameters:
 *     priv_data   - IN (driver private data)
 *
 *  Results:
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_en_default_q_free(struct ionic_en_priv_data *priv_data)     // IN
{
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_UplinkSharedQueueInfo *queue_info = &uplink_handle->uplink_q_info;

        ionic_en_txq_free(priv_data, queue_info->defaultTxQueueID);
        ionic_en_rxq_free(priv_data, queue_info->defaultRxQueueID);
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_alloc
 *
 *      uplink layer callback for allocating queue
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    q_type      - IN (Queue type RX/TX)
 *    uplink_qid  - OUT (queue ID)
 *    netpoll     - OUT (vmk_NetPoll that associated with this queue)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_alloc(vmk_AddrCookie driver_data,                  // IN 
                     vmk_UplinkQueueType q_type,                  // IN 
                     vmk_UplinkQueueID *uplink_qid,               // OUT
                     vmk_NetPoll *netpoll)                        // OUT
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
        struct ionic_en_uplink_handle *uplink_handle;
        
        priv_data = (struct ionic_en_priv_data *) driver_data.ptr;
        uplink_handle = &priv_data->uplink_handle;

        ionic_info("mq: ionic_en_queue_alloc() called, queue type: %d",
                   q_type);

        VMK_ASSERT(uplink_qid);
        VMK_ASSERT(netpoll);

        vmk_SemaLock(&uplink_handle->mq_binary_sema);

        switch (q_type) {
                case VMK_UPLINK_QUEUE_TYPE_RX:
                        status = ionic_en_rxq_alloc(priv_data,
                                                    uplink_qid,
                                                    netpoll);
                        ionic_info("multi-queue rx");
                        break;
                case VMK_UPLINK_QUEUE_TYPE_TX:
                        status = ionic_en_txq_alloc(priv_data,
                                                    uplink_qid);
                        ionic_info("multi-queue tx");
                        break;
                default:
                        ionic_err("Unknown queue type: %d", q_type);
                        status = VMK_BAD_PARAM;
                        break;
        }

        vmk_SemaUnlock(&uplink_handle->mq_binary_sema);

        if (status != VMK_OK) {
                ionic_err("Allocation for queue type: %d failed", q_type);
        }

        return status;
}


/*
 ******************************************************************************
 *
 *  ionic_en_rx_rss_q_alloc
 *
 *     Allocate a RX RSS queue
 *
 *  Parameters:
 *     priv_data     - IN (driver private handle)
 *     uplink_qid     - OUT (pointer to uplink queue id)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_rx_rss_q_alloc(struct ionic_en_priv_data *priv_data,                      //IN
                     vmk_UplinkQueueID *uplink_qid)                      //OUT
{
        VMK_ReturnStatus status                       = VMK_OK;
        struct ionic_en_uplink_handle *uplink_handle  = &priv_data->uplink_handle;
        vmk_UplinkSharedQueueData *shared_queue_data  = uplink_handle->uplink_q_data;
        vmk_Bool is_found                             = VMK_FALSE;
        vmk_uint32 i, found_idx, rss_ring_idx;

        ionic_info("ionic_en_rx_rss_q_alloc() called");

        IONIC_EN_FOR_EACH_RX_RSS_SHARED_QUEUE_DATA_INDEX(priv_data, i) {
                VMK_ASSERT(shared_queue_data[i].supportedFeatures &
                           VMK_UPLINK_QUEUE_FEAT_RSS);

                if ((shared_queue_data[i].flags & VMK_UPLINK_QUEUE_FLAG_IN_USE) == 0) {
                        is_found = VMK_TRUE;
                        found_idx = i;
                        break;
                }
        }

        if (!is_found) {
                ionic_err("Failed to allocate RX RSS queue, no entry found");
                status = VMK_NOT_FOUND;
                goto out;
        }

        VMK_ASSERT(found_idx < uplink_handle->max_rx_queues);

        rss_ring_idx = ionic_en_convert_shared_q_idx(priv_data, found_idx);

        VMK_ASSERT(rss_ring_idx < uplink_handle->max_rx_rss_queues &&
                   rss_ring_idx >= 0);

        status = vmk_UplinkQueueMkRxQueueID(uplink_qid, found_idx, rss_ring_idx);
        VMK_ASSERT(status == VMK_OK);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        shared_queue_data[found_idx].flags &= ~(VMK_UPLINK_QUEUE_FLAG_DEFAULT
                                                | VMK_UPLINK_QUEUE_FLAG_IN_USE);
        shared_queue_data[found_idx].flags |= VMK_UPLINK_QUEUE_FLAG_IN_USE;
        shared_queue_data[found_idx].qid = *uplink_qid;

        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);
        ionic_info("RX RSS queue is allocated, index: %d", rss_ring_idx);

out:
        return status;
}


/*
 ******************************************************************************
 *
 *   ionic_en_get_rx_sup_feat
 *
 *      Get sum of all supported features of rx queues
 *
 *  Parameters:
 *    priv_data - IN (Driver private data handler)
 *
 *  Results:
 *     Features supported by the driver
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static vmk_UplinkQueueFeature
ionic_en_get_rx_sup_feat(struct ionic_en_priv_data *priv_data)
{
        vmk_UplinkQueueFeature features = 0;
        vmk_uint32 i;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_info("ionic_en_get_rx_sup_feat() called");

        IONIC_EN_SHARED_AREA_BEGIN_READ(uplink_handle);

        IONIC_EN_FOR_EACH_RX_SHARED_QUEUE_DATA_INDEX(priv_data, i) {
                features |= uplink_handle->uplink_q_data[i].supportedFeatures;
        }

        IONIC_EN_SHARED_AREA_END_READ(uplink_handle);

        return features;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_alloc_with_attr
 *
 *      uplink layer callback for allocating queue with additional attributes
 *
 *  Parameters:
 *    driver_data - IN (private driver data pointing to the adapter)
 *    q_type      - IN (Queue type RX/TX)
 *    num_attr    - IN (Number of attributes)
 *    attr        - IN (Queue attributes)
 *    uplink_qid  - OUT (queue ID)
 *    netpoll     - OUT (vmk_NetPoll associated with this queue)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_alloc_with_attr(vmk_AddrCookie driver_data,        // IN 
                               vmk_UplinkQueueType q_type,        // IN 
                               vmk_uint16 num_attr,               // IN 
                               vmk_UplinkQueueAttr *attr,         // IN 
                               vmk_UplinkQueueID *uplink_qid,     // OUT
                               vmk_NetPoll *netpoll)              // OUT
{
        VMK_ReturnStatus status = VMK_OK;
        vmk_UplinkQueueFeature features;
        vmk_uint32 i;
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *)driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;

        ionic_info("ionic_en_queue_alloc_with_attr() called,"
                   " queue type: %d", q_type);

        vmk_SemaLock(&uplink_handle->mq_binary_sema);

        switch (q_type) {
                case VMK_UPLINK_QUEUE_TYPE_RX:
                if (!num_attr) {
                        goto alloc_normal_rx_q;
                }    

                if (!attr) {
                        ionic_err("Invalid attr value: %d ", num_attr);
                                status = VMK_BAD_PARAM;
                                break;
                }

                for (i = 0; i < num_attr; ++i) {
                        if (attr[i].type == VMK_UPLINK_QUEUE_ATTR_PRIOR) {
                                ionic_err("attr type PRIOR is not supported, continue to "
                                          "next attr");
                                break;
                        } else if (attr[i].type == VMK_UPLINK_QUEUE_ATTR_FEAT) {
                                features = attr[i].args.features;
                                if (features & ~(ionic_en_get_rx_sup_feat(priv_data))) {
                                        ionic_err("features 0x%x are not supported", features);
                                        status = VMK_BAD_PARAM;
                                        goto out; 
                                }    

                                if (features & VMK_UPLINK_QUEUE_FEAT_RSS) {
                                        status = ionic_en_rx_rss_q_alloc(priv_data,
                                                                         uplink_qid);
                                }

                                if (!(features & VMK_UPLINK_QUEUE_FEAT_RSS) ||
                                    (status == VMK_NOT_FOUND)) {
                                        goto alloc_normal_rx_q;
                                }
                                goto out;
                        } else {
                                ionic_warn("attr from type %d is not supported",
                                           attr[i].type);
                                status = VMK_BAD_PARAM;
                                goto out;
                        }
                }

alloc_normal_rx_q:
                status = ionic_en_rxq_alloc(priv_data, uplink_qid, netpoll);
                break;

                case VMK_UPLINK_QUEUE_TYPE_TX:
                        status = ionic_en_txq_alloc(priv_data, uplink_qid);
                        break;

                default:
                        ionic_warn("Unknown queue type=0x%d", q_type);
                        status = VMK_BAD_PARAM;
                        break;
        }

out:
        vmk_SemaUnlock(&uplink_handle->mq_binary_sema);

        return status;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_realloc_with_attr
 *
 *      uplink layer callback to to reallocate a Rx queue with extra attributes.
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    params      - IN (Realloc queue params)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_realloc_with_attr(vmk_AddrCookie driver_data,              // IN
                                 vmk_UplinkQueueReallocParams *params)    // IN
{
        ionic_info("ionic_en_queue_realloc_with_attr() called");

        return VMK_NOT_SUPPORTED;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_free
 *
 *      uplink layer callback for freeing queue
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    uplink_qid  - IN (queue ID)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_free(vmk_AddrCookie driver_data,                   // IN
                    vmk_UplinkQueueID uplink_qid)                 // IN
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_priv_data *priv_data;
        struct ionic_en_uplink_handle *uplink_handle;
        vmk_UplinkQueueType q_type = vmk_UplinkQueueIDType(uplink_qid);

        priv_data = (struct ionic_en_priv_data *) driver_data.ptr;
        uplink_handle = &priv_data->uplink_handle;

        ionic_info("mq: ionic_en_queue_free() called, queue type: %d",
                   q_type);

        VMK_ASSERT(uplink_qid);

        vmk_SemaLock(&uplink_handle->mq_binary_sema);

        switch (q_type) {
                case VMK_UPLINK_QUEUE_TYPE_RX:
                        ionic_en_rxq_free(priv_data,
                                          uplink_qid);
                        break;
                case VMK_UPLINK_QUEUE_TYPE_TX:
                        ionic_en_txq_free(priv_data,
                                          uplink_qid);
                        break;
                default:
                        ionic_err("Unknown queue type: %d", q_type);
                        status = VMK_BAD_PARAM;
                        break;
        }

        vmk_SemaUnlock(&uplink_handle->mq_binary_sema);

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_is_queue_stop
 *
 *     Check if a queue is stop
 *
 *  Parameters:
 *     uplink_handle        - IN (ionic_en_uplink_handle handle)
 *     shared_q_data_idx    - IN (index to shared queue data array)
 *
 *  Results:
 *     VMK_TRUE      if queue is stop
 *     VMK_FALSE     otherwise
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

vmk_Bool
ionic_en_is_queue_stop(struct ionic_en_uplink_handle *uplink_handle,   // IN 
                        vmk_uint32 shared_q_data_idx)                   // IN
{
        vmk_Bool result = VMK_FALSE;
        vmk_UplinkSharedQueueData *shared_q_data;

        IONIC_EN_SHARED_AREA_BEGIN_READ(uplink_handle);

        shared_q_data = &uplink_handle->uplink_q_data[shared_q_data_idx];
        if (shared_q_data->state == VMK_UPLINK_QUEUE_STATE_STOPPED) {
                result = VMK_TRUE;
        }

        IONIC_EN_SHARED_AREA_END_READ(uplink_handle);

        return result;
}


/*
 ******************************************************************************
 *
 *  ionic_en_rxq_start
 *
 *     start the given rx queue
 *
 *  Parameters:
 *     uplink_handle     - IN (ionic_en_uplink_handle handle)
 *     shared_q_data_idx - IN (index to shared queue data array)
 *
 *  Results:
 *     None
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_en_rxq_start(struct ionic_en_uplink_handle *uplink_handle,  // IN
                   vmk_uint32 shared_q_data_idx)                  // IN
{
        vmk_UplinkSharedQueueData *shared_q_data;

        ionic_info("ionic_en_rxq_start() called, shared_q_data_idx: %d",
                   shared_q_data_idx);

        IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle,
                                           shared_q_data_idx,
                                           VMK_UPLINK_QUEUE_TYPE_RX);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        shared_q_data = &uplink_handle->uplink_q_data[shared_q_data_idx];

        if (!(shared_q_data->flags & VMK_UPLINK_QUEUE_FLAG_IN_USE)) {
                goto out;
        }

        if (shared_q_data->state == VMK_UPLINK_QUEUE_STATE_STARTED) {
                goto out;
        }

        shared_q_data->state = VMK_UPLINK_QUEUE_STATE_STARTED;
        if (shared_q_data->supportedFeatures & VMK_UPLINK_QUEUE_FEAT_DYNAMIC) {
                shared_q_data->activeFeatures |= VMK_UPLINK_QUEUE_FEAT_DYNAMIC;
        }

        if (uplink_handle->max_tx_queues == uplink_handle->max_rx_queues &&
            (shared_q_data->supportedFeatures & VMK_UPLINK_QUEUE_FEAT_PAIR)) {
                shared_q_data->activeFeatures |= VMK_UPLINK_QUEUE_FEAT_PAIR;
        }

        (uplink_handle->uplink_q_info.activeRxQueues)++;

        ionic_info("shared idx: %d is actived", shared_q_data_idx);
out:
        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);
}


/*
 ******************************************************************************
 *
 *  ionic_en_rxq_quiesce
 *
 *     quiesce the given rx queue
 *
 *  Parameters:
 *     uplink_handle     - IN (ionic_en_uplink_handle handle)
 *     shared_q_data_idx - IN (index to shared queue data array)
 *
 *  Results:
 *     None
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_en_rxq_quiesce(struct ionic_en_uplink_handle *uplink_handle,   // IN   
                     vmk_uint32 shared_q_data_idx)                   // IN
{
        vmk_UplinkSharedQueueData *shared_q_data;

        ionic_info("ionic_en_rxq_quiesce() called, shared_q_data_idx: %d",
                   shared_q_data_idx);

        IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle,
                                           shared_q_data_idx,
                                           VMK_UPLINK_QUEUE_TYPE_RX);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        shared_q_data = &uplink_handle->uplink_q_data[shared_q_data_idx];

        if (!(shared_q_data->flags & VMK_UPLINK_QUEUE_FLAG_IN_USE)) {
                goto out;
        }

        if (shared_q_data->state == VMK_UPLINK_QUEUE_STATE_STOPPED) {
                goto out;
        }

        shared_q_data->state = VMK_UPLINK_QUEUE_STATE_STOPPED;
        (uplink_handle->uplink_q_info.activeRxQueues)--;

        ionic_info("shared idx: %d is quiesced", shared_q_data_idx);

out:
        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);
}


/*
 ******************************************************************************
 *
 *  ionic_en_txq_start
 *
 *     start the given tx queue
 *
 *  Parameters:
 *     uplink_handle               - IN (ionic_en_uplink_handle handle)
 *     shared_q_data_idx           - IN (index to shared queue data array)
 *     is_ping_upper_layer         - IN (whether or not notify the upper layer)
 *
 *  Results:
 *     None
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_en_txq_start(struct ionic_en_uplink_handle *uplink_handle,  // IN
                   vmk_uint32 shared_q_data_idx,                  // IN
                   vmk_Bool is_ping_upper_layer)                  // IN
{
        vmk_UplinkSharedQueueData *shared_q_data;

        ionic_info("ionic_en_txq_start() called, shared_idx: %d",
                   shared_q_data_idx);

        IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle,
                                           shared_q_data_idx,
                                           VMK_UPLINK_QUEUE_TYPE_TX);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        shared_q_data = &uplink_handle->uplink_q_data[shared_q_data_idx];

        if (!(shared_q_data->flags & VMK_UPLINK_QUEUE_FLAG_IN_USE)) {
                is_ping_upper_layer = VMK_FALSE;
                goto out;
        }

        if (shared_q_data->state == VMK_UPLINK_QUEUE_STATE_STARTED) {
                goto out;
        }

        shared_q_data->state = VMK_UPLINK_QUEUE_STATE_STARTED;

        if (uplink_handle->max_tx_queues == uplink_handle->max_rx_queues &&
            (shared_q_data->supportedFeatures & VMK_UPLINK_QUEUE_FEAT_PAIR)) {
                shared_q_data->activeFeatures |= VMK_UPLINK_QUEUE_FEAT_PAIR;
        }

        (uplink_handle->uplink_q_info.activeTxQueues)++;

        ionic_info("shared idx: %d is actived", shared_q_data_idx);

out:
        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        if (is_ping_upper_layer) {
                vmk_UplinkQueueStart(uplink_handle->uplink_dev,
                                     shared_q_data->qid);
        }
}


/*
 ******************************************************************************
 *
 *  ionic_en_txq_quiesce
 *
 *     quiesce the given tx queue
 *
 *  Parameters:
 *     uplink_handle        - IN (ionic_en_uplink_handle handle)
 *     shared_q_data_idx    - IN (index to shared queue data array)
 *     is_ping_upper_layer  - IN (whether or not notify the upper layer)
 *
 *  Results:
 *     None
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_en_txq_quiesce(struct ionic_en_uplink_handle *uplink_handle,    // IN
                     vmk_uint32 shared_q_data_idx,                    // IN
                     vmk_Bool is_ping_upper_layer)                    // IN
{
        vmk_UplinkSharedQueueData *shared_q_data;

        ionic_info("ionic_en_txq_quiesce() called, shared_q_data_idx: %d",
                   shared_q_data_idx);

        IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle,
                                           shared_q_data_idx,
                                           VMK_UPLINK_QUEUE_TYPE_TX);

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);

        shared_q_data = &uplink_handle->uplink_q_data[shared_q_data_idx];

        if (!(shared_q_data->flags & VMK_UPLINK_QUEUE_FLAG_IN_USE)) {
                is_ping_upper_layer = VMK_FALSE;
                goto out;
        }

        if (shared_q_data->state == VMK_UPLINK_QUEUE_STATE_STOPPED) {
                goto out;
        }

        shared_q_data->state = VMK_UPLINK_QUEUE_STATE_STOPPED;
        (uplink_handle->uplink_q_info.activeTxQueues)--;

        ionic_info("shared_q_data_idx: %d is quiesced", shared_q_data_idx);
out:
        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        if (is_ping_upper_layer) {
                vmk_UplinkQueueStop(uplink_handle->uplink_dev,
                                    shared_q_data->qid);
        }
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_quiesce
 *
 *      uplink layer callback for queue quiesce
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    uplink_qid  - IN (queue ID)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_quiesce(vmk_AddrCookie driver_data,                // IN
                       vmk_UplinkQueueID uplink_qid)              // IN
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_priv_data *priv_data;
        struct ionic_en_uplink_handle *uplink_handle;
        vmk_UplinkQueueType q_type = vmk_UplinkQueueIDType(uplink_qid);
        vmk_uint32 shared_q_data_idx;
        
        priv_data = (struct ionic_en_priv_data *) driver_data.ptr;
        uplink_handle = &priv_data->uplink_handle;
        shared_q_data_idx = vmk_UplinkQueueIDQueueDataIndex(uplink_qid);

        ionic_info("mq: ionic_en_queue_quiesce() called, queue type: %d",
                   q_type);

        vmk_SemaLock(&uplink_handle->mq_binary_sema);

        switch (q_type) {
                case VMK_UPLINK_QUEUE_TYPE_RX:
                        ionic_en_rxq_quiesce(uplink_handle,
                                             shared_q_data_idx);
                        break;
                case VMK_UPLINK_QUEUE_TYPE_TX:
                        ionic_en_txq_quiesce(uplink_handle,
                                             shared_q_data_idx,
                                             VMK_FALSE);
                        break;
                default:
                        ionic_err("Unknown queue type: %d", q_type);
                        status = VMK_BAD_PARAM;
                        break;
        }

        vmk_SemaUnlock(&uplink_handle->mq_binary_sema);

        return status;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_start
 *
 *      uplink layer callback for queue start
 *
 *  Parameters:
 *    driver_data - IN (private driver data pointing to the adapter)
 *    uplink_qid  - IN (queue ID)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_start(vmk_AddrCookie driver_data,                          //IN
                     vmk_UplinkQueueID uplink_qid)                        //IN
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_priv_data *priv_data;
        struct ionic_en_uplink_handle *uplink_handle;
        vmk_UplinkQueueType q_type = vmk_UplinkQueueIDType(uplink_qid);
        vmk_uint32 shared_q_data_idx;
        
        priv_data = (struct ionic_en_priv_data *) driver_data.ptr;
        uplink_handle = &priv_data->uplink_handle;
        shared_q_data_idx = vmk_UplinkQueueIDQueueDataIndex(uplink_qid);

        ionic_info("mq: ionic_en_queue_start() called, queue type: %d",
                   q_type);

        vmk_SemaLock(&uplink_handle->mq_binary_sema);

        switch (q_type) {
                case VMK_UPLINK_QUEUE_TYPE_RX:
                        ionic_en_rxq_start(uplink_handle,
                                           shared_q_data_idx);
                        break;
                case VMK_UPLINK_QUEUE_TYPE_TX:
                        ionic_en_txq_start(uplink_handle,
                                           shared_q_data_idx,
                                           VMK_FALSE);
                        break;
                default:
                        ionic_err("Unknown queue type: %d", q_type);
                        status = VMK_BAD_PARAM;
                        break;
        }

        vmk_SemaUnlock(&uplink_handle->mq_binary_sema);

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_is_mac_filter_exists
 *
 *     Check if the given MAC is associated to one of the filters in given
 *     filters array
 *
 *  Parameters:
 *     filter_attr       - IN (vmk_UplinkQueueMACFilterInfo array)
 *     filter_attr_size  - IN (vmk_UplinkQueueMACFilterInfo array size)
 *     req_mac_addr      - IN (MAC address to check)
 *
 *  Results:
 *     VMK_TRUE  if MAC is associated to one of the filters in the given
 *               filters array
 *     VMK_FALSE otherwise
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static inline vmk_Bool
ionic_en_is_mac_filter_exists(vmk_UplinkQueueMACFilterInfo *filter_attr,   // IN
                              vmk_uint32 filter_attr_size,                 // IN
                              vmk_EthAddress req_mac_addr)                 // IN
{
        vmk_uint32 i;

        for (i = 0; i < filter_attr_size; ++i) {
              if (ionic_is_eth_addr_equal(req_mac_addr,
                                          filter_attr[i].mac)) {
                      return VMK_TRUE;
              }
        }

        return VMK_FALSE;
}


/*
 ******************************************************************************
 *
 * ionic_en_is_vlan_filter_exists
 *
 *     Check if the given VLAN info is associated to one of the
 *     filters in given filters array
 *
 *  Parameters:
 *     filter_attr      - IN (vmk_UplinkQueueVLANFilterInfo array)
 *     filter_attr_size - IN (vmk_UplinkQueueVLANFilterInfo array size)
 *     req_vlan_info   - IN (VXLan info to check)
 *
 *  Results:
 *     VMK_TRUE  if VLAN info is associated to one of the filters in given
 *               filters array
 *     VMK_FALSE otherwise
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static inline vmk_Bool
ionic_en_is_vlan_filter_exists(vmk_UplinkQueueVLANFilterInfo *filter_attr,     // IN
                               vmk_uint32 filter_attr_size,                    // IN
                               vmk_UplinkQueueVLANFilterInfo *req_vlan_info)   // IN
{
        vmk_uint32 i;

        for (i = 0; i < filter_attr_size; ++i) {
                if (req_vlan_info->vlanID == filter_attr[i].vlanID) {
                        return VMK_TRUE;
                }
        }

        return VMK_FALSE;
}


/*
 ******************************************************************************
 *
 * ionic_en_is_req_exists
 *
 *     Check if the given filter information was previously applied.
 *
 *  Parameters:
 *     priv_data      - IN (private driver data)
 *     filter_info    - IN (filter information to check)
 *
 *  Results:
 *     VMK_TRUE if filter information was previously applied
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static vmk_Bool
ionic_en_is_req_exists(struct ionic_en_priv_data *priv_data,      // IN
                       struct ionic_en_filter_info *filter_info)  // IN
{
        vmk_uint32 i;
        struct ionic_en_rx_ring *rx_ring;
        struct ionic_en_rx_rss_ring *rx_rss_ring;
        vmk_Bool res = VMK_FALSE;

        ionic_info("ionic_en_is_req_exists() called");

        if (filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                IONIC_EN_FOR_EACH_RX_NORMAL_RING_IDX(priv_data, i) {
                        rx_ring = &priv_data->uplink_handle.rx_rings[i];
                        if (!rx_ring->is_init) {
                                continue;
                        }
                        res = ionic_en_is_mac_filter_exists(rx_ring->mac_filter,
                                        IONIC_EN_MAX_FILTERS_PER_RX_Q,
                                        filter_info->mac_filter_info->mac);
                        if (res) {
                                goto out;
                        }
                }

                IONIC_EN_FOR_EACH_RX_RSS_RING_IDX(priv_data, i) {
                        rx_rss_ring = &priv_data->uplink_handle.rx_rss_rings[i];
                        if (!rx_rss_ring->is_init) {
                                continue;
                        }

                        res = ionic_en_is_mac_filter_exists(rx_rss_ring->mac_filter,
                                        IONIC_EN_MAX_FILTERS_PER_RX_Q,
                                        filter_info->mac_filter_info->mac);
                        if (res) {
                                goto out;
                        }
                }
        } else if (filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY) {
                IONIC_EN_FOR_EACH_RX_NORMAL_RING_IDX(priv_data, i) {
                        rx_ring = &priv_data->uplink_handle.rx_rings[i];
                        if (!rx_ring->is_init) {
                                continue;
                        }
                        res = ionic_en_is_vlan_filter_exists(rx_ring->vlan_filter,
                                        IONIC_EN_MAX_FILTERS_PER_RX_Q,
                                        filter_info->vlan_filter_info);
                        if (res) {
                                goto out;
                        }
                }

                IONIC_EN_FOR_EACH_RX_RSS_RING_IDX(priv_data, i) {
                        rx_rss_ring = &priv_data->uplink_handle.rx_rss_rings[i];
                        if (!rx_rss_ring->is_init) {
                                continue;
                        }

                        res = ionic_en_is_vlan_filter_exists(rx_rss_ring->vlan_filter,
                                        IONIC_EN_MAX_FILTERS_PER_RX_Q,
                                        filter_info->vlan_filter_info);
                        if (res) {
                                goto out;
                        }
                }
        }

out:

        return res;
}


/*
 ******************************************************************************
 *
 *   ionic_en_get_ring_type
 *
 *      return queue type (RX, RX RSS, TX) based on shared queue data index
 *
 *  Parameters:
 *    priv_data            - IN (private driver data)
 *    shared_q_data_idx    - IN (shared queue data index)
 *
 *  Results:
 *     queue type string ("RX", "RX RSS", "TX") based on shared queue data index
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static inline char*
ionic_en_get_ring_type(struct ionic_en_priv_data *priv_data,       // IN
                       vmk_uint32  shared_q_data_idx)              // IN
{
        if (ionic_en_is_rss_q_idx(priv_data, shared_q_data_idx)) {
                return "RX RSS";
        }

        if (shared_q_data_idx < priv_data->uplink_handle.max_rx_queues) {
                return "RX";
        }

        return "TX";
}


/*
 ******************************************************************************
 *
 *   ionic_en_rx_validate_filter_attrs
 *
 *      Validate filters attributes
 *
 *  Parameters:
 *    priv_data   - IN (private driver data pointing to the adapter)
 *    uplink_qid  - IN (queue ID)
 *    q_filter    - IN (queue Filter)
 *
 *  Results:
 *     VMK_BAD_PARAM
 *     VMK_IGNORE
 *     VMK_OK
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_rx_validate_filter_attrs(struct ionic_en_priv_data *priv_data,    // IN
                                  vmk_UplinkQueueID uplink_qid,            // IN
                                  vmk_UplinkQueueFilter *q_filter)         // IN
{
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_UplinkQueueType q_type;
        vmk_uint32 ring_index, shared_q_data_idx;
        VMK_ReturnStatus status = VMK_BAD_PARAM;
        vmk_Bool is_rss;

        ionic_info("ionic_en_rx_validate_filter_attrs() called");

        q_type            = vmk_UplinkQueueIDType(uplink_qid);
        ring_index        = vmk_UplinkQueueIDUserVal(uplink_qid);
        shared_q_data_idx = vmk_UplinkQueueIDQueueDataIndex(uplink_qid);

        // TODO: Determine if it's RSS
        is_rss = ionic_en_is_rss_q_idx(priv_data, shared_q_data_idx);
 
        if (q_type != VMK_UPLINK_QUEUE_TYPE_RX) {
                ionic_err("filter queue type %u is not supported", q_type);
                goto out;
        }

        if (q_filter &&
            q_filter->class != VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY &&
            q_filter->class != VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY) {
                ionic_err("filter class %u not supported", q_filter->class);
                goto out;
        }

        if (shared_q_data_idx >= uplink_handle->max_rx_queues) {
                ionic_err("queue index %d is out of range, max number of "
                          "rx queues is %d",
                          shared_q_data_idx, uplink_handle->max_rx_queues);
                goto out;
        }

        if ((ring_index >= uplink_handle->max_rx_rss_queues && is_rss) ||
            (ring_index >= uplink_handle->max_rx_normal_queues && !is_rss)) {
                ionic_err("Ring type is: %s, ring index is :%d. Out of Range, "
                          "max index is: %d.",
                          ionic_en_get_ring_type(priv_data, shared_q_data_idx),
                          ring_index,
                          is_rss? (uplink_handle->max_rx_rss_queues - 1) :
                          (uplink_handle->max_rx_normal_queues -1));
                goto out;
        }

        if (ring_index == 0 &&
            !is_rss) {
                ionic_warn("Ignore filter on default queue");
                status = VMK_IGNORE;
                goto out;
        }

        status = VMK_OK;

out:
        return status;
}


/*
 ******************************************************************************
 *
 *   ionic_en_rx_filter_register
 *
 *     Add a new entry (unicast/vlan steering entry).
 *
 *  Parameters:
 *    priv_data   - IN (private driver data)
 *    filter_info - IN (Filter information)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_rx_filter_register(struct ionic_en_priv_data *priv_data,      // IN
                            struct ionic_en_filter_info *filter_info)  // IN
{
        VMK_ReturnStatus status = VMK_OK;
        struct lif *lif = NULL;
        vmk_EthAddress mac_addr;
        vmk_uint32 vlan_id = 0;

        ionic_info("ionic_en_rx_filter_register() called");

        VMK_ASSERT(filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY ||
                   filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY);

        if (filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                vmk_Memcpy(mac_addr,
                           filter_info->mac_filter_info->mac,
                           VMK_ETH_ADDR_LENGTH);
        } else if (filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY) {
                vlan_id = filter_info->vlan_filter_info->vlanID;
        } else {
                ionic_err("Unsupported class type: %d", filter_info->filter_class);
        }

//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);

        if (filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {

                status = ionic_lif_addr_add(lif,
                                            mac_addr); 

                if (status != VMK_OK) {
                        ionic_err("ionic_lif_addr_add() failed: %s, while "
                                   "applying filter on lif[%p] by "
                                   "MAC:"VMK_ETH_ADDR_FMT_STR,
                                   vmk_StatusToString(status),
                                   lif,
                                   VMK_ETH_ADDR_FMT_ARGS(mac_addr));
                }
        } else {
                status = ionic_vlan_rx_add_vid(lif,
                                               vlan_id);
                if (status != VMK_OK) {
                        ionic_err("ionic_vlan_rx_add_vid() failed: %s, while "
                                  "applying filter on lif[%p] by vlan ID: 0x%x",
                                  vmk_StatusToString(status),
                                  lif,
                                  vlan_id);
                }
        }

        return status;
}


/*
 ******************************************************************************
 *
 *   ionic_en_rx_filter_unregister
 *
 *      Remove entry (unicast/vlan steering entry).
 *
 *  Parameters:
 *    priv_data   - IN (private driver data)
 *    filter_info - IN (Filter information)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_rx_filter_unregister(struct ionic_en_priv_data *priv_data,       // IN
                              struct ionic_en_filter_info *filter_info)   // IN
{
        VMK_ReturnStatus status    = VMK_OK;
        struct lif *lif = NULL;
        vmk_EthAddress mac_addr;
        vmk_uint32 vlan_id = 0;

        ionic_info("ionic_en_rx_filter_unregister() called");

        VMK_ASSERT(filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY ||
                   filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY);

        if (filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                vmk_Memcpy(mac_addr,
                           filter_info->mac_filter_info->mac,
                           VMK_ETH_ADDR_LENGTH);
        } else if (filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY) {
                vlan_id = filter_info->vlan_filter_info->vlanID;
        } else {
                ionic_err("Unsupported class type: %d", filter_info->filter_class);
        }

//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);
        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);
//        vmk_SpinlockLock(priv_data->ionic.lifs_lock);

        if (filter_info->filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {

                status = ionic_lif_addr_del(lif,
                                            mac_addr);
                if (status != VMK_OK) {
                        ionic_err("ionic_lif_addr_del() failed: %s, "
                                  "lif [%p], MAC:"VMK_ETH_ADDR_FMT_STR,
                                  vmk_StatusToString(status),
                                  lif,
                                  VMK_ETH_ADDR_FMT_ARGS(mac_addr));
                }
        } else {
                status = ionic_vlan_rx_kill_vid(lif,
                                                vlan_id);
                if (status != VMK_OK) {
                        ionic_err("ionic_vlan_rx_kill_vid() failed: %s, "
                                  "lif[%p] vlan ID: 0x%x",
                                  vmk_StatusToString(status),
                                  lif,
                                  vlan_id);
                }
        }

        return status;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_apply_filter
 *
 *      uplink layer callback for apply filter
 *
 *  Parameters:
 *    driver_data   - IN (private driver data)
 *    uplink_qid    - IN (queue ID)
 *    q_filter      - IN (queue Filter)
 *    fid           - OUT (Filter ID)
 *    pair_hw_qid   - OUT (Potential paired tx queue hardware index)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_apply_filter(vmk_AddrCookie driver_data,           // IN
                            vmk_UplinkQueueID uplink_qid,         // IN
                            vmk_UplinkQueueFilter *q_filter,      // IN
                            vmk_UplinkQueueFilterID *fid,         // OUT
                            vmk_uint32 *pair_hw_qid)              // OUT
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *) driver_data.ptr;
        vmk_UplinkQueueFilterClass class             = q_filter->class;
        vmk_UplinkSharedQueueData *shared_queue_data = NULL;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        struct ionic_en_filter_info filter_info      = { .filter_class = class };
        vmk_Bool is_found                            = VMK_FALSE;
        vmk_uint32 i, filter_idx, shared_queue_data_idx, rx_ring_idx;
        vmk_UplinkQueueMACFilterInfo *mac_filters    = NULL;
        vmk_UplinkQueueVLANFilterInfo *vlan_filters  = NULL;
        vmk_EthAddress zero_mac_addr;
        vmk_UplinkQueueType q_type;
        vmk_Bool is_rss;

        ionic_info("ionic_en_queue_apply_filter() called");

        status = ionic_en_rx_validate_filter_attrs(priv_data, uplink_qid, q_filter);
        if (status == VMK_IGNORE) {
                status = VMK_OK;
                goto out;
        }

        if (status != VMK_OK) {
                ionic_err("ionic_en_rx_validate_filter_attrs() failed, status: %s",
                          vmk_StatusToString(status));
                goto out;
        }

        q_type                = vmk_UplinkQueueIDType(uplink_qid);
        rx_ring_idx           = vmk_UplinkQueueIDUserVal(uplink_qid);
        shared_queue_data_idx = vmk_UplinkQueueIDQueueDataIndex(uplink_qid);

        is_rss = ionic_en_is_rss_q_idx(priv_data,
                                       shared_queue_data_idx);

        if (class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                filter_info.mac_filter_info =
                        (vmk_UplinkQueueMACFilterInfo *)(q_filter->filterInfo.ptr);
                // TODO: Add RSS case
                mac_filters = is_rss? priv_data->uplink_handle.rx_rss_rings[rx_ring_idx].mac_filter :
                          priv_data->uplink_handle.rx_rings[rx_ring_idx].mac_filter;
        } else if (class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY) {
                filter_info.vlan_filter_info =
                        (vmk_UplinkQueueVLANFilterInfo *)(q_filter->filterInfo.ptr);
                vlan_filters = is_rss? priv_data->uplink_handle.rx_rss_rings[rx_ring_idx].vlan_filter :
                               priv_data->uplink_handle.rx_rings[rx_ring_idx].vlan_filter;
        } else {
                VMK_ASSERT(0);
        }

        vmk_SemaLock(&uplink_handle->mq_binary_sema);

        IONIC_EN_SHARED_AREA_BEGIN_READ(uplink_handle);

        shared_queue_data = &uplink_handle->uplink_q_data[shared_queue_data_idx];

        if (shared_queue_data->activeFilters > shared_queue_data->maxFilters) {
                ionic_err("The number of active filters is: %d which is "
                          "more than max filters allowed: %d",
                          shared_queue_data->activeFilters,
                          shared_queue_data->maxFilters);
                status = VMK_FAILURE;
        }

        IONIC_EN_SHARED_AREA_END_READ(uplink_handle);

        if (status != VMK_OK) {
                ionic_err("Failed to apply filter,"
                          "all filters for %s ring %u are allocated",
                          ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                          rx_ring_idx);
                goto filters_apply_err;
        }

        if (ionic_en_is_req_exists(priv_data, &filter_info)) {
                status = VMK_EXISTS;
                goto filters_apply_err;
        }

        if (class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                vmk_Memset(zero_mac_addr, 0, VMK_ETH_ADDR_LENGTH);
                for (i = 0; i < shared_queue_data->maxFilters; i++) {
                        if (!vmk_Memcmp(mac_filters[i].mac,
                                        zero_mac_addr,
                                        VMK_ETH_ADDR_LENGTH)) {
                                vmk_Memcpy(mac_filters[i].mac,
                                           filter_info.mac_filter_info->mac,
                                           VMK_ETH_ADDR_LENGTH);
                                filter_idx = i;
                                is_found = VMK_TRUE;
                                break;
                        }
                }
        } else if (class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY){
                for (i = 0; i < shared_queue_data->maxFilters; i++) {
                        if (vlan_filters[i].vlanID == 0) {
                                vmk_Memcpy(&(vlan_filters[i]),
                                           filter_info.vlan_filter_info,
                                           sizeof(vmk_UplinkQueueVLANFilterInfo));
                                filter_idx = i;
                                is_found = VMK_TRUE;
                                break;
                        }
                }
        }

        if (!is_found) {
                ionic_err("Failed to apply %s filter on %s ring %u "
                          "- all filters are allocated",
                          (class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) ?
                          "MAC" : "VLAN",
                          ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                          rx_ring_idx);
                goto filters_apply_err;
        }

        // TODO: When multi-lif comes, we can specify which lif
        status = ionic_en_rx_filter_register(priv_data, &filter_info);

        if (status != VMK_OK) {
                ionic_err("ionic_en_rx_filter_register() failed: %s, "
                          "on %s ring %u",
                          vmk_StatusToString(status),
                          ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                          rx_ring_idx);
                goto filter_reg_err;
        }

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);
        ++shared_queue_data->activeFilters;
        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        status = vmk_UplinkQueueMkFilterID(fid,
                        class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY ? filter_idx :
                        filter_idx + IONIC_EN_MAX_FILTERS_PER_RX_Q);
        VMK_ASSERT(status == VMK_OK);

        if (shared_queue_data->activeFeatures & VMK_UPLINK_QUEUE_FEAT_PAIR) {
                VMK_ASSERT((shared_queue_data->activeFeatures &
                           VMK_UPLINK_QUEUE_FEAT_RSS) == 0);
                *pair_hw_qid = rx_ring_idx;
        }

        vmk_SemaUnlock(&uplink_handle->mq_binary_sema);

        if (class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                ionic_info("MAC RX filter (class %u) at index %u is applied on "
                           "%s ring %u, Mac address "VMK_ETH_ADDR_FMT_STR,
                           class, filter_idx,
                           ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                           rx_ring_idx,
                           VMK_ETH_ADDR_FMT_ARGS(filter_info.mac_filter_info->mac));
        } else if (class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY) {
                ionic_info("VLAN RX filter (class %u) at index %u is applied on "
                           "%s ring %u, VLAN ID: 0x%x",
                           class,
                           filter_idx,
                           ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                           rx_ring_idx,
                           filter_info.vlan_filter_info->vlanID);
        }

        goto out;

filter_reg_err:
        if (class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                vmk_Memset(mac_filters[filter_idx].mac,
                           0,
                           VMK_ETH_ADDR_LENGTH);
        } else {
                vmk_Memset(&vlan_filters[filter_idx],
                           0,
                           sizeof(vmk_UplinkQueueVLANFilterInfo));
        }

filters_apply_err:
        vmk_SemaUnlock(&uplink_handle->mq_binary_sema);

out:

        return status;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_remove_filter
 *
 *      uplink layer callback for remove filter
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    uplink_qid  - IN (queue ID)
 *    fid         - IN (filter ID)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_remove_filter(vmk_AddrCookie driver_data,          // IN
                             vmk_UplinkQueueID uplink_qid,        // IN
                             vmk_UplinkQueueFilterID fid)         // IN
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *)driver_data.ptr;
        struct ionic_en_uplink_handle *uplink_handle = &priv_data->uplink_handle;
        vmk_uint32 shared_queue_data_idx, rx_ring_idx;
        vmk_uint32 filter_idx, vlan_filter_idx;
        vmk_EthAddress zero_mac_addr;
        vmk_UplinkQueueType q_type;
        vmk_Bool is_rss;
        struct ionic_en_filter_info filter_info;

        ionic_info("ionic_en_queue_remove_filter() called");

        status = ionic_en_rx_validate_filter_attrs(priv_data,
                                                   uplink_qid,
                                                   NULL);
        if (status == VMK_IGNORE) {
                status = VMK_OK;
                goto out;
        }

        if (status != VMK_OK) {
                ionic_err("ionic_en_rx_validate_filter_attrs() failed: %s",
                          vmk_StatusToString(status));
                goto out;
        }

        shared_queue_data_idx = vmk_UplinkQueueIDQueueDataIndex(uplink_qid);
        rx_ring_idx           = vmk_UplinkQueueIDUserVal(uplink_qid);
        q_type                = vmk_UplinkQueueIDType(uplink_qid);
        filter_idx            = vmk_UplinkQueueFilterIDVal(fid);

        is_rss = ionic_en_is_rss_q_idx(priv_data,
                                       shared_queue_data_idx);
 
        if (rx_ring_idx >= uplink_handle->max_rx_queues) {
                ionic_err("Failed to remove RX filter "
                          "ring %u is out of range", rx_ring_idx);
                status = VMK_BAD_PARAM;
                goto out;
        }

        if (filter_idx < IONIC_EN_MAX_FILTERS_PER_RX_Q) {
                filter_info.filter_class = VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY;

                filter_info.mac_filter_info = is_rss?
                        &priv_data->uplink_handle.rx_rss_rings[rx_ring_idx].mac_filter[filter_idx] :
                        &priv_data->uplink_handle.rx_rings[rx_ring_idx].mac_filter[filter_idx];

                vmk_Memset(zero_mac_addr, 0, VMK_ETH_ADDR_LENGTH); 

                if (!vmk_Memcmp(filter_info.mac_filter_info->mac,
                                zero_mac_addr,
                                VMK_ETH_ADDR_LENGTH)) {
                        ionic_err("Failed to remove RX filter on %s ring %u. "
                                  "Filter with zero MAC address is not applied",
                                  ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                                  rx_ring_idx);
                        status = VMK_BAD_PARAM;
                        goto out;
                }

        } else {
                vlan_filter_idx = filter_idx - IONIC_EN_MAX_FILTERS_PER_RX_Q;
                filter_info.filter_class = VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY;

                filter_info.vlan_filter_info = is_rss?
                        &priv_data->uplink_handle.rx_rss_rings[rx_ring_idx].vlan_filter[vlan_filter_idx] :
                        &priv_data->uplink_handle.rx_rings[rx_ring_idx].vlan_filter[vlan_filter_idx];
                if (filter_info.vlan_filter_info->vlanID == 0) {
                        ionic_err("Failed to remove RX filter on %s ring %u. "
                                  "Filter with zero VLAN ID is not applied  ",
                                  ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                                  rx_ring_idx);
                        status = VMK_BAD_PARAM;
                        goto out;
                }
        }

        vmk_SemaLock(&uplink_handle->mq_binary_sema);

        // TODO: When multi-lif comes, we can specify which lif
        status = ionic_en_rx_filter_unregister(priv_data, 
                                               &filter_info);
        if (status != VMK_OK) {
                ionic_err("ionic_en_rx_filter_unregister() failed: %s, on %s "
                          "ring %u ",
                           vmk_StatusToString(status),
                           ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                           rx_ring_idx);
                goto filter_unregister_err;
        }

        IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle);
        --uplink_handle->uplink_q_data[shared_queue_data_idx].activeFilters;
        IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle);

        if (filter_info.filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                ionic_info("MAC RX filter (class %u) at index %u is removed from "
                           "%s ring %u, Mac address "VMK_ETH_ADDR_FMT_STR,
                           filter_info.filter_class,
                           filter_idx,
                           ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                           rx_ring_idx,
                           VMK_ETH_ADDR_FMT_ARGS(filter_info.mac_filter_info->mac));
        } else if (filter_info.filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY) {
                ionic_info("VLAN RX filter (class %u) at index %u is removed from "
                           "%s ring %u, VLAN ID: 0x%x",
                           filter_info.filter_class,
                           filter_idx,
                           ionic_en_get_ring_type(priv_data, shared_queue_data_idx),
                           rx_ring_idx,
                           filter_info.vlan_filter_info->vlanID);
        } else {
                VMK_ASSERT(0);
        }

        if (filter_info.filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_MAC_ONLY) {
                vmk_Memset(filter_info.mac_filter_info,
                           0,
                           sizeof(vmk_UplinkQueueMACFilterInfo));
        } else if (filter_info.filter_class == VMK_UPLINK_QUEUE_FILTER_CLASS_VLAN_ONLY) {
                vmk_Memset(filter_info.vlan_filter_info,
                           0,
                           sizeof(vmk_UplinkQueueVLANFilterInfo));
        }

filter_unregister_err:
        vmk_SemaUnlock(&uplink_handle->mq_binary_sema);

out:

        return status;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_get_stats
 *
 *      uplink layer callback to get queue stats
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    qid         - IN (queue ID)
 *    stats       - OUT (stats of queue)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_get_stats(vmk_AddrCookie driver_data,              // IN
                         vmk_UplinkQueueID qid,                   // IN
                         struct vmk_UplinkStats *stats)           // OUT
{
        ionic_info("ionic_en_queue_get_stats() called");

        return VMK_NOT_SUPPORTED;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_toggle_feature
 *
 *      uplink layer callback to toggle a queue feature
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    qid         - IN (queue ID)
 *    q_feature   - IN (queue Feature)
 *    is_set      - IN (set or unset the feature)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_toggle_feature(vmk_AddrCookie driver_data,         // IN
                              vmk_UplinkQueueID qid,              // IN
                              vmk_UplinkQueueFeature q_feature,   // IN
                              vmk_Bool is_set)                    // IN
{
        ionic_info("ionic_en_queue_toggle_feature() called");

        return VMK_NOT_SUPPORTED;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_set_priority
 *
 *      uplink layer callback to set queue priority
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    qid         - IN (queue ID)
 *    priority    - IN (queue priority)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_set_priority(vmk_AddrCookie driver_data,           // IN
                            vmk_UplinkQueueID qid,                // IN
                            vmk_UplinkQueuePriority priority)     // IN
{
        ionic_info("ionic_en_queue_set_priority() called");

        return VMK_NOT_SUPPORTED;
}


/*
 ******************************************************************************
 *
 *   ionic_en_queue_set_coalesce_params
 *
 *      uplink layer callback to to set queue coalesce parameters
 *
 *  Parameters:
 *    driver_data - IN (private driver data)
 *    qid         - IN (queue ID)
 *    params      - IN (coalesce params)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_en_queue_set_coalesce_params(vmk_AddrCookie driver_data,         // IN
                                   vmk_UplinkQueueID qid,              // IN
                                   vmk_UplinkCoalesceParams *params)   // IN
{
        ionic_info("ionic_en_queue_set_coalesce_params() called");

        return VMK_NOT_SUPPORTED;
}

