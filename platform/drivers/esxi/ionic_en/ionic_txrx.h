/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

#ifndef _IONIC_TXRX_H_
#define _IONIC_TXRX_H_

#include <vmkapi.h>
#include "ionic_lif.h"

#define IONIC_MAX_TX_BUF_SIZE                   (1 << 14)
#define IONIC_TXD_NEEDED(size) (((size) + IONIC_MAX_TX_BUF_SIZE - 1) / \
                                IONIC_MAX_TX_BUF_SIZE)

#define IONIC_MAX_CSUM_OFFSET                   1024
#define IONIC_EN_ETH_HLEN                       sizeof(vmk_EthHdr)
#define IONIC_EN_VLAN_HLEN                      sizeof(vmk_VLANHdr)
#define IONIC_EN_MAX_FILTERS_PER_RX_Q           64
#define IONIC_MAX_NUM_RX_RINGS_PER_RSS_QUEUE    16
#define IONIC_EN_STOP_QUEUE_THRESHOLD           4
#define IONIC_EN_WAKE_QUEUE_THRESHOLD           IONIC_MIN(64, ntxq_descs >> 1)
#define IONIC_EN_TX_MAX_BURST                   64
#define IONIC_VLAN_MASK                         0xfff
#define IONIC_VLAN_PRIO_SHIFT                   13
#define IONIC_VLAN_PRIO_MASK                    0x7

struct ionic_en_priv_data;
struct ionic_en_uplink_handle;

enum ionic_en_ring_type{
        IONIC_EN_ADMIN_RING,
        IONIC_EN_NOTIFY_RING,
        IONIC_EN_TX_RING,
        IONIC_EN_RX_RING,
};

struct ionic_en_tx_ring {
        vmk_uint32                      ring_idx;
        vmk_uint32                      shared_q_data_idx;
        struct ionic_en_uplink_handle   *uplink_handle;
        struct qcq                      *txqcq;
        vmk_NetPoll                     netpoll;
        vmk_Bool                        is_init;
        vmk_Bool                        is_actived;
};

struct ionic_en_rx_ring {
        vmk_uint32                      ring_idx;
        vmk_uint32                      shared_q_data_idx;
        struct ionic_en_uplink_handle   *uplink_handle;
        struct qcq                      *rxqcq;
        vmk_NetPoll                     netpoll;
        vmk_UplinkQueueMACFilterInfo    mac_filter[IONIC_EN_MAX_FILTERS_PER_RX_Q];
        vmk_UplinkQueueVLANFilterInfo   vlan_filter[IONIC_EN_MAX_FILTERS_PER_RX_Q];
        vmk_Bool                        is_init;
        vmk_Bool                        is_actived;
        vmk_PktList                     pkt_list;
};

struct ionic_en_rx_rss_ring {
        vmk_uint32                      ring_idx;
        vmk_uint32                      shared_q_data_idx;
        struct ionic_en_priv_data       *priv_data;
        struct ionic_en_rx_ring         *p_attached_rx_rings[IONIC_MAX_NUM_RX_RINGS_PER_RSS_QUEUE];
        vmk_uint32                      num_attached_rx_rings;

        vmk_UplinkQueueMACFilterInfo    mac_filter[IONIC_EN_MAX_FILTERS_PER_RX_Q];
        vmk_UplinkQueueVLANFilterInfo   vlan_filter[IONIC_EN_MAX_FILTERS_PER_RX_Q];

        vmk_Bool                        is_init;
        vmk_Bool                        is_actived;
};       


typedef enum {
        IONIC_TX_TSO                    = 1 << 0,
        IONIC_TX_CSO                    = 1 << 1,
        IONIC_TX_VLAN                   = 1 << 2,
} ionic_en_offload_flags;

typedef struct {
        ionic_en_offload_flags          offload_flags;

        vmk_Bool                        is_tso_needed;
        vmk_Bool                        is_encap;

        vmk_ByteCountSmall              frame_len;
        vmk_ByteCountSmall              all_sgs_len;

        /* only valid for TSO packets */
        vmk_PktHeaderEntry              *l3_hdr_entry;

        /* only valid for TSO/CSO packets */
        vmk_PktHeaderEntry              *l4_hdr_entry;

        /* only valid for TSO packets */
        vmk_uint32                      mss;

        /* only valid for CSO packets */
        vmk_uint32                      csum_offset;

        vmk_uint32                      copy_len;
        vmk_uint32                      mapped_len;

        vmk_uint16                      vlan_id;

        vmk_VlanPriority                priority;

        vmk_uint32                      nr_frags;
} ionic_tx_ctx;

void ionic_rx_refill(struct queue *q);

void
ionic_rx_empty(struct queue *q);

void
ionic_rx_flush(struct cq *cq);

void
ionic_tx_flush(struct cq *cq);

void
ionic_rx_fill(struct queue *q);

vmk_Bool
ionic_tx_netpoll(vmk_AddrCookie priv,
                 vmk_uint32 budget);

vmk_Bool
ionic_rx_netpoll(vmk_AddrCookie priv,
                 vmk_uint32 budget);

VMK_ReturnStatus
ionic_start_xmit(vmk_PktHandle *pkt,
                 struct ionic_en_uplink_handle *uplink_handle,
                 struct ionic_en_tx_ring *tx_ring,
                 vmk_Bool is_last_pkt);

inline void
ionic_en_tx_ring_init(vmk_uint32 ring_idx,
                      vmk_uint32 shared_q_data_idx,
                      struct ionic_en_priv_data *priv_data,
                      struct lif *lif);

inline void
ionic_en_tx_ring_deinit(vmk_uint32 ring_idx,
                        struct ionic_en_priv_data *priv_data);

VMK_ReturnStatus
ionic_en_rx_ring_init(vmk_uint32 ring_idx,
                      vmk_uint32 shared_q_data_idx,
                      struct ionic_en_priv_data *priv_data,
                      struct lif *lif);
                     

inline void
ionic_en_rx_ring_deinit(vmk_uint32 ring_idx,
                        struct ionic_en_priv_data *priv_data);

VMK_ReturnStatus
ionic_en_rx_rss_init(struct ionic_en_priv_data *priv_data,
                     struct lif *lif);

void
ionic_en_rx_rss_deinit(struct ionic_en_priv_data *priv_data,
                       struct lif *lif);


#endif /* _IONIC_TXRX_H_ */

