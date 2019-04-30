/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
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
#define IONIC_MAX_NUM_TX_QUEUE                  16
#define IONIC_MAX_NUM_RX_QUEUE                  16
#define IONIX_MAX_NUM_RX_RSS_QUEUE               1

#define IONIC_EN_DEFAULT_MTU_SIZE               1500
#define IONIC_EN_MAX_BUF_SIZE_PRIV_STATS        4096 * 2

#define IONIC_EN_TX_COAL_USECS                  64
#define IONIC_EN_RX_COAL_USECS                  64

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
        vmk_Name                        uplink_name;
        vmk_UplinkState                 prev_state;
        vmk_UplinkRegData               uplink_reg_data;
        vmk_uint32                      DRSS;
        struct ionic_en_priv_data       *priv_data;

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
        struct ionic_en_rx_rss_ring     rx_rss_ring;

        vmk_EthAddress                  vmk_mac_addr;
        vmk_uint64                      mac_uint64;
        
        vmk_Lock                        link_status_lock;
        vmk_LinkStatus                  link_status;
        vmk_LinkStatus                  cur_hw_link_status;
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

        vmk_Semaphore                   coal_binary_sema;
        vmk_UplinkCoalesceParams        coal_params;

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

void
ionic_en_uplink_default_coal_params_set(struct ionic_en_priv_data *priv_data);

VMK_ReturnStatus
ionic_en_uplink_supported_mode_init(struct ionic_en_uplink_handle *uplink_handle);

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

VMK_ReturnStatus
ionic_en_uplink_coal_params_get(vmk_AddrCookie driver_data,
                                vmk_UplinkCoalesceParams *params);

VMK_ReturnStatus
ionic_en_uplink_coal_params_set(vmk_AddrCookie driver_data,
                                vmk_UplinkCoalesceParams *params);

#endif /* End of _IONIC_EN_UPLINK_H_ */
