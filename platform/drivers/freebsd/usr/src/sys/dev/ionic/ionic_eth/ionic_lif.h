/*
 * Copyright (c) 2017-2019 Pensando Systems, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _IONIC_LIF_H_
#define _IONIC_LIF_H_

#include <linux/if_ether.h>

#include <linux/workqueue.h>

#include "ionic_osdep.h"
#include "ionic_rx_filter.h"

#define LIF_NAME_MAX_SZ			8
#define QUEUE_NAME_MAX_SZ		8
#define MAX_VLAN_TAG 			4095

struct adminq_stats {
	u64 comp_err;
};

struct tx_stats {
	u64 dma_map_err;
	u64 pkts;
	u64 bytes;
	u64 clean;
	u64 re_queue;
	u64 linearize;
	u64 linearize_err;
	u64 no_descs;
	u64 no_csum_offload;
	u64 csum_offload;
	u64 comp_err;
	u64 tso_ipv4;
	u64 tso_ipv6;
	u64 tso_max_sg;
	u64 tso_max_size;
	u64 bad_ethtype;	/* Unknown Ethernet frame. */
};

struct rx_stats {
	u64 dma_map_err;
	u64 alloc_err;
	u64 pkts;
	u64 bytes;
	u64 csum_ip_ok;
	u64 csum_ip_bad;
	u64 csum_l4_ok;
	u64 csum_l4_bad;
	/* Only for debugging. */
	u64 mbuf_alloc;
	u64 mbuf_free;
	u64 isr_count; 	// Not required.
	u64 task;		/* Number of time task was invoked. */
	u64 comp_err;
	u64 mem_err;	/* Completions w/o associated mbuf. */

	u64 rss_ip4;
	u64 rss_tcp_ip4;
	u64 rss_udp_ip4;
	u64 rss_ip6;
	u64 rss_tcp_ip6;
	u64 rss_udp_ip6;
	u64 rss_ip6_ex;
	u64 rss_tcp_ip6_ex;
	u64 rss_udp_ip6_ex;
	u64 rss_unknown;
};

struct ionic_rx_buf {
	struct mbuf *m;
	bus_dmamap_t dma_map;
	uint64_t pa_addr; 					/* Cache address to avoid access to command ring. */
};

struct ionic_tx_buf {
	struct mbuf *m;
	bus_dmamap_t dma_map;
	uint64_t timestamp;
	uint64_t pa_addr; 					/* Cache address to avoid access to command ring. */
};

struct adminq {
	char name[QUEUE_NAME_MAX_SZ];

	struct lif *lif;
	unsigned int num_descs;
	unsigned int index;					/* Queue number. */
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;

	bus_dma_tag_t buf_tag;
	struct ionic_dma_info cmd_dma; 		/* DMA ring for command and completion. */
	dma_addr_t cmd_ring_pa;

	struct mtx mtx;
	int head_index;						/* Index for buffer and command descriptors. */
	int tail_index;
	int comp_index;						/* Index for completion descriptors. */
	int done_color; 					/* Expected comletion color. */

	struct adminq_stats stats;
	struct intr intr;

	struct ionic_admin_ctx **ctx_ring;
	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct admin_cmd *cmd_ring;
	struct admin_comp *comp_ring;
};

struct notifyq {
	char name[QUEUE_NAME_MAX_SZ];

	struct lif *lif;
	unsigned int num_descs;
	unsigned int index;					/* Queue number. */
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;

	struct ionic_dma_info cmd_dma; 		/* DMA ring for command and completion. */
	dma_addr_t cmd_ring_pa;

	struct mtx mtx;
	int comp_index;						/* Index for completion descriptors. */

	struct intr intr;
	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct notifyq_cmd *cmd_ring;
	union notifyq_comp *comp_ring;
};

struct rxque {
	char name[QUEUE_NAME_MAX_SZ];

	struct lif *lif;
	unsigned int num_descs; /* Max number of descriptors. */
	uint32_t descs;			/* Descriptors posted in queue. */
	unsigned int index;		/* Queue number. */
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;

	struct ionic_rx_buf *rxbuf; /* S/w rx buffer descriptors. */
	bus_dma_tag_t buf_tag;

	struct ionic_dma_info cmd_dma; /* DMA ring for command and completion. */
	dma_addr_t cmd_ring_pa;	

	struct mtx rx_mtx;

	int head_index;
	int tail_index;
	int comp_index;				/* Completion index. */
	int done_color; 			/* Expected completion color - 0/1. */

	struct rx_stats stats;
	struct intr intr;

	struct task task;
	struct taskqueue *taskq;

	struct lro_ctrl	lro;
	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct rxq_desc *cmd_ring;
	struct rxq_comp *comp_ring;
};

struct txque {
	char name[QUEUE_NAME_MAX_SZ];

	struct lif *lif;
	unsigned int num_descs;
	unsigned int index;				/* Queue number. */
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;

	struct ionic_tx_buf *txbuf;		/* S/w rx buffer descriptors. */
	bus_dma_tag_t buf_tag;
	struct ionic_dma_info cmd_dma; 	/* DMA ring for command and completion. */

	dma_addr_t cmd_ring_pa;

	struct mtx tx_mtx;
	int head_index;					/* Index for buffer and command descriptors. */
	int tail_index;
	int comp_index;					/* Index for completion descriptors. */
	int done_color; 				/* Expected comletion color status. */
	
	struct tx_stats stats;
	struct intr intr;

	struct task task;
	struct taskqueue *taskq;
	struct buf_ring	*br;

	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct txq_desc *cmd_ring;
	struct txq_comp *comp_ring;
	struct txq_sg_desc *sg_ring;	/* SG descriptors. */
};

struct ionic_mc_addr {
	u8  addr[ETHER_ADDR_LEN];
};

struct lif {
	char name[LIF_NAME_MAX_SZ];
	struct list_head list;
	struct net_device *netdev;

	u8 dev_addr[ETHER_ADDR_LEN] __aligned(sizeof(int));

	struct ionic *ionic;

	struct ifmedia          media;
	bool registered;

	unsigned int index;

	u64 num_dev_cmds;
	unsigned int kern_pid;
	struct doorbell __iomem *kern_dbpage;

	struct workqueue_struct *adminq_wq;
	struct adminq *adminq;

	struct notifyq *notifyq;
	struct txque **txqs;
	struct rxque **rxqs;

	unsigned int nnqs;
	unsigned int neqs;
	unsigned int ntxqs;
	unsigned int nrxqs;

	unsigned int rx_mode;

	int rx_mbuf_size;			/* Rx mbuf size pool. */
	uint16_t max_frame_size;	/* MTU size. */

	u32 hw_features;			/* Features enabled in hardware, e.g. checksum, TSO etc. */

	struct ionic_dma_info stats_dma; 	/* DMA ring for command and completion. */
	struct ionic_lif_stats *stats_dump;

	dma_addr_t stats_dump_pa;

	u8 rss_hash_key[RSS_HASH_KEY_SIZE];
	u8 *rss_ind_tbl;
	dma_addr_t rss_ind_tbl_pa;

	u32 tx_coalesce_usecs;
	u32 rx_coalesce_usecs;

	struct mutex dbid_inuse_lock;
	unsigned long *dbid_inuse;
	unsigned int dbid_count;

	void *api_private;	/* For RoCE */
	void (*api_reset_cb)(void *api_private);

	uint64_t spurious; /* Spurious interrupt counter in legacy mode. */

	struct sysctl_oid *sysctl_ifnet;
	struct sysctl_ctx_list sysctl_ctx;
	struct mtx mtx;

	struct rx_filters rx_filters;

	struct ionic_mc_addr *mc_addrs;
	int num_mc_addrs;

	/* 4096 bit array for VLAN. */
	uint8_t vlan_bitmap[4096 / 8];
	int num_vlans;
	eventhandler_tag vlan_attach;
	eventhandler_tag vlan_detach;

	u64 last_eid;
	bool link_up;

#define IONIC_LINK_SPEED_100G	100000
#define IONIC_LINK_SPEED_1G	1000
	u32 link_speed;		/* units of 1Mbps: e.g. 10000 = 10Gbps */
	u16 phy_type;
	u16 autoneg_status;
	u16 link_flap_count;
	u16 link_error_bits;
	u32 notifyblock_sz;
	dma_addr_t notifyblock_pa;
	struct ionic_dma_info notify_dma;
	struct notify_block *notifyblock;
};

/* lif lock. */
#define IONIC_CORE_LOCK_INIT(x) 	mtx_init(&(x)->mtx, (x)->name, NULL, MTX_DEF)
#define IONIC_CORE_LOCK_DESTROY(x)	mtx_destroy(&(x)->tx_mtx)
#define IONIC_CORE_LOCK(x)			mtx_lock(&(x)->mtx)
#define IONIC_CORE_UNLOCK(x)		mtx_unlock(&(x)->mtx)
#define IONIC_CORE_LOCK_OWNED(x)	mtx_owned(&(x)->mtx)

#define IONIC_ADMIN_LOCK_INIT(x) 	mtx_init(&(x)->mtx, (x)->name, NULL, MTX_DEF)
#define IONIC_ADMIN_LOCK_DESTROY(x)	mtx_destroy(&(x)->mtx)
#define IONIC_ADMIN_LOCK(x)			mtx_lock(&(x)->mtx);
#define IONIC_ADMIN_UNLOCK(x)		mtx_unlock(&(x)->mtx);
#define IONIC_ADMIN_LOCK_OWNED(x) 	mtx_owned(&(x)->mtx)

#define IONIC_NOTIFYQ_LOCK_INIT(x) 		mtx_init(&(x)->mtx, (x)->name, NULL, MTX_DEF)
#define IONIC_NOTIFYQ_LOCK_DESTROY(x)	mtx_destroy(&(x)->mtx)
#define IONIC_NOTIFYQ_LOCK(x)			mtx_lock(&(x)->mtx);
#define IONIC_NOTIFYQ_UNLOCK(x)			mtx_unlock(&(x)->mtx);
#define IONIC_NOTIFYQ_LOCK_OWNED(x) 	mtx_owned(&(x)->mtx)

#define IONIC_TX_LOCK_INIT(x)		mtx_init(&(x)->tx_mtx, (x)->name, NULL, MTX_DEF)
#define IONIC_TX_LOCK_DESTROY(x) 	mtx_destroy(&(x)->tx_mtx)
#define IONIC_TX_LOCK(x)			mtx_lock(&(x)->tx_mtx)
#define IONIC_TX_TRYLOCK(x)			mtx_trylock(&(x)->tx_mtx)
#define IONIC_TX_UNLOCK(x)			mtx_unlock(&(x)->tx_mtx)
//#define IONIC_TX_LOCK_OWNED(x)	mtx_owned(&(x)->tx_mtx)

#define IONIC_RX_LOCK_INIT(x)		mtx_init(&(x)->rx_mtx, (x)->name, NULL, MTX_DEF)
#define IONIC_RX_LOCK_DESTROY(x)	mtx_destroy(&(x)->rx_mtx)
#define IONIC_RX_LOCK(x)			mtx_lock(&(x)->rx_mtx)
#define IONIC_RX_UNLOCK(x)			mtx_unlock(&(x)->rx_mtx)
#define IONIC_RX_LOCK_OWNED(x)		mtx_owned(&(x)->rx_mtx)

void ionic_open(void *arg);
int ionic_stop(struct net_device *netdev);

int ionic_lifs_alloc(struct ionic *ionic);
void ionic_lifs_free(struct ionic *ionic);
void ionic_lifs_deinit(struct ionic *ionic);
int ionic_lifs_init(struct ionic *ionic);
int ionic_lifs_register(struct ionic *ionic);
void ionic_lifs_unregister(struct ionic *ionic);
int ionic_lifs_size(struct ionic *ionic);

int ionic_adminq_clean(struct adminq* adminq, int limit);

int ionic_dev_intr_reserve(struct lif *lif, struct intr *intr);
void ionic_dev_intr_unreserve(struct lif *lif, struct intr *intr);

struct lif *ionic_netdev_lif(struct net_device *netdev);

int ionic_set_hw_features(struct lif *lif, uint32_t features);

int ionic_rss_ind_tbl_set(struct lif *lif, const u32 *indir);
int ionic_rss_hash_key_set(struct lif *lif, const u8 *key, uint16_t rss_types);

int ionic_rx_clean(struct rxque* rxq , int rx_limit);
void ionic_rx_input(struct rxque *rxq, struct ionic_rx_buf *buf,
			   struct rxq_comp *comp, 	struct rxq_desc *desc);

void ionic_tx_ring_doorbell(struct txque *txq, int index);
int ionic_tx_clean(struct txque* txq , int tx_limit);

int ionic_change_mtu(struct net_device *netdev, int new_mtu);
void ionic_set_rx_mode(struct net_device *netdev);
void ionic_clear_rx_mode(struct net_device *netdev);

void ionic_set_multi(struct lif* lif);

int ionic_set_mac(struct net_device *netdev);

extern int ionic_devcmd_timeout;
#endif /* _IONIC_LIF_H_ */
