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
 * ionic_en_mq.h --
 *
 * Definitions for multi-queue 
 */

#ifndef _IONIC_EN_MQ_H_
#define _IONIC_EN_MQ_H_

#include <vmkapi.h>

#define IONIC_EN_INVALID_SHARED_QUEUE_DATA_INDEX                  ((vmk_uint32)-1)

#define IONIC_EN_FOR_EACH_RX_SHARED_QUEUE_DATA_INDEX(priv_data, i)               \
   for (i = 0; i < priv_data->uplink_handle.max_rx_queues; i++)

#define IONIC_EN_FOR_EACH_RX_NORMAL_SHARED_QUEUE_DATA_INDEX(priv_data, i)        \
   for (i = 0; i < priv_data->uplink_handle.max_rx_normal_queues; i++)

#define IONIC_EN_FOR_EACH_RX_RSS_SHARED_QUEUE_DATA_INDEX(priv_data, i)           \
   for (i = priv_data->uplink_handle.max_rx_normal_queues;                       \
        i < priv_data->uplink_handle.max_rx_queues; i++)

#define IONIC_EN_FOR_EACH_TX_SHARED_QUEUE_DATA_INDEX(priv_data, i)               \
   for (i = priv_data->uplink_handle.max_rx_queues;                              \
        i < priv_data->uplink_handle.max_rx_queues +                             \
            priv_data->uplink_handle.max_tx_queues;                              \
        i++)

#define IONIC_EN_FOR_EACH_SHARED_QUEUE_DATA_INDEX(priv_data, i)                  \
   for (i = 0;                                                                   \
        i < priv_data->uplink_handle.max_rx_queues +                             \
            priv_data->uplink_handle.max_tx_queues;                              \
        i++)

#define IONIC_EN_VALIDATE_SHARED_QUEUE_IDX(uplink_handle, i, q_type)             \
   do {                                                                          \
      VMK_ASSERT(q_type != VMK_UPLINK_QUEUE_TYPE_INVALID);                       \
      if (q_type == VMK_UPLINK_QUEUE_TYPE_RX) {                                  \
         VMK_ASSERT((i) < uplink_handle->max_rx_queues);                         \
      }                                                                          \
      if (q_type == VMK_UPLINK_QUEUE_TYPE_TX) {                                  \
         VMK_ASSERT((uplink_handle->max_rx_queues >= (i)) &&                     \
                    ((i) < (uplink_handle->max_rx_queues +                       \
                            uplink_handle->max_tx_queues)));                     \
      }                                                                          \
   } while (0)

#define IONIC_EN_FOR_EACH_RX_NORMAL_RING_IDX(priv_data, i)                       \
   for (i = 0; i < priv_data->uplink_handle.max_rx_normal_queues; i++)           \

#define IONIC_EN_FOR_EACH_RX_RSS_RING_IDX(priv_data, i)                          \
   for (i = 0; i < priv_data->uplink_handle.max_rx_rss_queues; i++)              \

#define IONIC_EN_FOR_EACH_TX_RING_IDX(priv_data, i)                              \
   for (i = 0; i < priv_data->uplink_handle.max_tx_queues; i++)                  \



struct ionic_en_filter_info {
        vmk_UplinkQueueFilterClass       filter_class;
   union {
        vmk_UplinkQueueMACFilterInfo     *mac_filter_info;
        vmk_UplinkQueueVLANFilterInfo    *vlan_filter_info;
   };  
};

vmk_uint32
ionic_get_num_queues_supported(vmk_uint32 numTxQueues,
                               vmk_uint32 numRxQueues);

inline vmk_Bool
ionic_en_is_rss_q_idx(struct ionic_en_priv_data *priv_data,
                      vmk_uint32 idx);

inline vmk_uint32
ionic_en_convert_attached_rss_ring_idx(struct ionic_en_priv_data *priv_data,
                                       vmk_uint32 rss_ring_idx,
                                       vmk_uint32 num_attached_rx_rings,
                                       vmk_uint32 cur_rx_ring_idx);

VMK_ReturnStatus
ionic_en_default_q_alloc(struct ionic_en_priv_data *priv_data);

void
ionic_en_default_q_free(struct ionic_en_priv_data *priv_data);

VMK_ReturnStatus
ionic_en_queue_alloc(vmk_AddrCookie driver_data,
                     vmk_UplinkQueueType q_type,
                     vmk_UplinkQueueID *uplink_qid,
                     vmk_NetPoll *netPoll);

VMK_ReturnStatus
ionic_en_queue_alloc_with_attr(vmk_AddrCookie driver_data,
                               vmk_UplinkQueueType q_type,
                               vmk_uint16 num_attr,
                               vmk_UplinkQueueAttr *attr,
                               vmk_UplinkQueueID *uplink_qid,
                               vmk_NetPoll *netPoll);

VMK_ReturnStatus
ionic_en_queue_realloc_with_attr(vmk_AddrCookie driver_data,
                                 vmk_UplinkQueueReallocParams *params);

VMK_ReturnStatus
ionic_en_queue_free(vmk_AddrCookie driver_data,
                    vmk_UplinkQueueID uplink_qid);

void
ionic_en_rxq_start(struct ionic_en_uplink_handle *uplink_handle,
                   vmk_uint32 shared_q_data_idx);

void
ionic_en_rxq_quiesce(struct ionic_en_uplink_handle *uplink_handle,
                     vmk_uint32 shared_q_data_idx);

void
ionic_en_txq_start(struct ionic_en_uplink_handle *uplink_handle,
                   vmk_uint32 shared_q_data_idx,
                   vmk_Bool is_ping_upper_layer);

void
ionic_en_txq_quiesce(struct ionic_en_uplink_handle *uplink_handle,
                     vmk_uint32 shared_q_data_idx,
                     vmk_Bool is_ping_upper_layer);

VMK_ReturnStatus
ionic_en_queue_quiesce(vmk_AddrCookie driver_data,
                       vmk_UplinkQueueID uplink_qid);

vmk_Bool
ionic_en_is_queue_stop(struct ionic_en_uplink_handle *uplink_handle,
                       vmk_uint32 shared_q_data_idx);
VMK_ReturnStatus
ionic_en_queue_start(vmk_AddrCookie driver_data,
                     vmk_UplinkQueueID uplink_qid);

VMK_ReturnStatus
ionic_en_queue_apply_filter(vmk_AddrCookie driver_data,
                            vmk_UplinkQueueID uplink_qid,
                            vmk_UplinkQueueFilter *q_filter,
                            vmk_UplinkQueueFilterID *fid,
                            vmk_uint32 *pair_hw_qid);

VMK_ReturnStatus
ionic_en_queue_remove_filter(vmk_AddrCookie driver_data,
                             vmk_UplinkQueueID uplink_qid,
                             vmk_UplinkQueueFilterID fid);

VMK_ReturnStatus
ionic_en_queue_get_stats(vmk_AddrCookie driver_data,
                         vmk_UplinkQueueID qid,
                         struct vmk_UplinkStats *stats);

VMK_ReturnStatus
ionic_en_queue_toggle_feature(vmk_AddrCookie driver_data,
                              vmk_UplinkQueueID qid,
                              vmk_UplinkQueueFeature q_feature,
                              vmk_Bool is_set);

VMK_ReturnStatus
ionic_en_queue_set_priority(vmk_AddrCookie driver_data,
                            vmk_UplinkQueueID qid,
                            vmk_UplinkQueuePriority priority);

VMK_ReturnStatus
ionic_en_queue_set_coalesce_params(vmk_AddrCookie driver_data,
                                   vmk_UplinkQueueID qid,
                                   vmk_UplinkCoalesceParams *params);

#endif /* End of _IONIC_EN_MQ_H_ */
