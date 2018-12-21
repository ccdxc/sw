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
 * ionic_en_uplink.h --
 *
 * Definitions for uplink layer interface 
 */

#ifndef _IONIC_EN_UPLINK_H_
#define _IONIC_EN_UPLINK_H_

#include "ionic_txrx.h"

// TODO: Figure out the max
#define IONIC_MAX_NUM_TX_QUEUE                  32
#define IONIC_MAX_NUM_RX_QUEUE                  32
#define IONIC_NUM_RX_RSS_QUEUES                 0

#define IONIC_EN_DEFAULT_MTU_SIZE               1500

#define IONIC_EN_SHARED_AREA_BEGIN_WRITE(uplink_handle)                          \
{                                                                                \
   vmk_SpinlockLockIgnoreDeathPending((uplink_handle)->share_data_write_lock);   \
   vmk_VersionedAtomicBeginWrite(&(uplink_handle)->uplink_shared_data.lock);     \
}

#define IONIC_EN_SHARED_AREA_END_WRITE(uplink_handle)                            \
{                                                                                \
   vmk_VersionedAtomicEndWrite(&(uplink_handle)->uplink_shared_data.lock);       \
   vmk_SpinlockUnlock((uplink_handle)->share_data_write_lock);                   \
}

#define IONIC_EN_SHARED_AREA_BEGIN_READ(uplink_handle)                           \
   do {                                                                          \
      vmk_uint32 shared_var;                                                     \
      do {                                                                       \
         shared_var = vmk_VersionedAtomicBeginTryRead                            \
                              (&(uplink_handle)->uplink_shared_data.lock);

#define IONIC_EN_SHARED_AREA_END_READ(uplink_handle)                             \
      } while (!vmk_VersionedAtomicEndTryRead                                    \
                 (&(uplink_handle)->uplink_shared_data.lock, shared_var));       \
   } while (VMK_FALSE)


struct ionic_en_uplink_handle {
        vmk_Uplink                      uplink_dev;
        vmk_UplinkState                 prev_state;
        vmk_UplinkRegData               uplink_reg_data;

        vmk_Lock                        share_data_write_lock;
        vmk_UplinkSharedData            uplink_shared_data;
        vmk_UplinkSharedQueueData       uplink_q_data[IONIC_MAX_NUM_RX_QUEUE +
                                                      IONIC_MAX_NUM_TX_QUEUE];
        vmk_UplinkSharedQueueInfo       uplink_q_info;

        vmk_uint32                      max_tx_queues;
        vmk_uint32                      max_rx_queues;
        vmk_uint32                      max_rx_normal_queues;
        vmk_uint32                      max_rx_rss_queues;
        vmk_uint32                      rx_rings_per_rss_queue;

        struct ionic_en_tx_ring         tx_rings[IONIC_MAX_NUM_TX_QUEUE];
        struct ionic_en_rx_ring         rx_rings[IONIC_MAX_NUM_RX_QUEUE];
        struct ionic_en_rx_rss_ring     rx_rss_rings[IONIC_NUM_RX_RSS_QUEUES];

        vmk_EthAddress                  vmk_mac_addr;
        vmk_uint64                      mac_uint64;
        
        vmk_Lock                        link_status_lock;
        vmk_LinkStatus                  link_status;
        vmk_WorldID                     link_check_world;

        vmk_Lock                        admin_status_lock;
        vmk_LinkStatus                  admin_link_status;

        vmk_Semaphore                   vlan_filter_binary_sema;
        vmk_VLANBitmap                  vlan_filter_bitmap;

        vmk_UplinkTransceiverType       trans_type;
        vmk_uint32                      hw_features;

        vmk_Semaphore                   status_binary_sema;
        vmk_Bool                        is_started;        

        vmk_Semaphore                   mq_binary_sema;
        vmk_Semaphore                   stats_binary_sema;
        vmk_UplinkStats                 uplink_stats;

        vmk_Bool                        is_mgmt_nic;
        vmk_Bool                        is_ready_notify_linkup;
        vmk_Bool                        is_init;
};

VMK_ReturnStatus
ionic_en_uplink_transceiver_type_get(vmk_AddrCookie driver_data,
                                     vmk_UplinkTransceiverType *trans_type);

VMK_ReturnStatus
ionic_en_uplink_transceiver_type_set(vmk_AddrCookie driver_data,
                                     vmk_UplinkTransceiverType trans_type);


VMK_ReturnStatus
ionic_en_uplink_tx(vmk_AddrCookie driver_data,
                   vmk_PktList pkt_list);

VMK_ReturnStatus
ionic_en_uplink_mtu_set(vmk_AddrCookie driver_data,
                        vmk_uint32 mtu);

VMK_ReturnStatus
ionic_en_uplink_state_set(vmk_AddrCookie driver_data,
                          vmk_UplinkState newState);


VMK_ReturnStatus
ionic_en_uplink_stats_get(vmk_AddrCookie driver_data,
                          vmk_UplinkStats *stats);

VMK_ReturnStatus
ionic_en_uplink_associate(vmk_AddrCookie driver_data,
                          vmk_Uplink uplink);

VMK_ReturnStatus
ionic_en_uplink_disassociate(vmk_AddrCookie driver_data);


VMK_ReturnStatus
ionic_en_uplink_cap_enable(vmk_AddrCookie driver_data,
                           vmk_UplinkCap cap);

VMK_ReturnStatus
ionic_en_uplink_cap_disable(vmk_AddrCookie driver_data,
                            vmk_UplinkCap cap);

VMK_ReturnStatus
ionic_en_uplink_start_io(vmk_AddrCookie driver_data);

VMK_ReturnStatus
ionic_en_uplink_quiesce_io(vmk_AddrCookie driver_data);

VMK_ReturnStatus
ionic_en_uplink_reset(vmk_AddrCookie driver_data);

VMK_ReturnStatus
ionic_en_uplink_init(struct ionic_en_priv_data *priv_data);

void
ionic_en_uplink_cleanup(struct ionic_en_priv_data *priv_data);

VMK_ReturnStatus
ionic_en_vlan_filter_enable(vmk_AddrCookie driver_data);

VMK_ReturnStatus
ionic_en_vlan_filter_disable(vmk_AddrCookie driver_data);

VMK_ReturnStatus
ionic_en_vlan_filter_bitmap_get(vmk_AddrCookie driver_data,
                                vmk_VLANBitmap *bitmap);

VMK_ReturnStatus
ionic_en_vlan_filter_bitmap_set(vmk_AddrCookie driver_data,
                                vmk_VLANBitmap *bitmap);

VMK_ReturnStatus
ionic_en_vlan_filter_add(vmk_AddrCookie driver_data,
                         vmk_VlanID start_id,
                         vmk_VlanID end_id);
VMK_ReturnStatus
ionic_en_vlan_filter_remove(vmk_AddrCookie driver_data,
                            vmk_VlanID start_id,
                            vmk_VlanID end_id);

VMK_ReturnStatus
ionic_en_netpoll_create(vmk_NetPoll *netpoll,
                        struct ionic_en_uplink_handle *uplink_handle,
                        enum ionic_en_ring_type ring_type,
                        vmk_NetPollCallback poll,
                        void *poll_arg);

#endif /* End of _IONIC_EN_UPLINK_H_ */
