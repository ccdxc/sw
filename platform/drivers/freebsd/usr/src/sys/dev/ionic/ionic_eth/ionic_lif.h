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
#define MAX_VLAN_TAG 			4095

#define IONIC_GET_COS(ifp) ((ifp->if_pcp == IFNET_PCP_NONE) ? 0 : ifp->if_pcp)

struct ionic_adminq_stats {
	uint64_t comp_err;
};

struct ionic_tx_stats {
	uint64_t dma_map_err;
	uint64_t pkts;
	uint64_t bytes;
	uint64_t clean;
	uint64_t re_queue;
	uint64_t mbuf_defrag;
	uint64_t mbuf_defrag_err;
	uint64_t no_descs;
	uint64_t no_csum_offload;
	uint64_t csum_offload;
	uint64_t comp_err;
	uint64_t tso_ipv4;
	uint64_t tso_ipv6;
	uint64_t tso_max_sg;
	uint64_t tso_max_size;
	uint64_t tso_max_descs;
	uint64_t tso_too_big;
	uint64_t tso_no_descs;
	uint64_t tso_sparse;
	uint64_t tso_defrag_sparse;
	uint64_t pkt_sparse;
	uint64_t pkt_defrag_sparse;
	uint64_t bad_ethtype;	/* Unknown Ethernet frame. */
	uint64_t wdog_expired;
};

struct ionic_rx_stats {
	uint64_t dma_map_err;
	uint64_t alloc_err;
	uint64_t pkts;
	uint64_t bytes;
	uint64_t csum_ip_ok;
	uint64_t csum_ip_bad;
	uint64_t csum_l4_ok;
	uint64_t csum_l4_bad;
	/* Only for debugging. */
	uint64_t mbuf_alloc;
	uint64_t mbuf_free;
	uint64_t isr_count;
	uint64_t task;	/* Number of time task was invoked. */
	uint64_t comp_err;
	uint64_t length_err;

	uint64_t rss_ip4;
	uint64_t rss_tcp_ip4;
	uint64_t rss_udp_ip4;
	uint64_t rss_ip6;
	uint64_t rss_tcp_ip6;
	uint64_t rss_udp_ip6;
	uint64_t rss_unknown;
};

struct ionic_rx_buf {
	struct mbuf *m;
	bus_dmamap_t dma_map;
	uint32_t sg_buf_len;	/* If SG is used, its buffer length. */
};

struct ionic_tx_buf {
	struct mbuf *m;
	uint8_t is_tso;
	bus_dmamap_t dma_map;
	bus_dmamap_t tso_dma_map;
	uint64_t timestamp;
	uint64_t pa_addr; 		/* Cache address to avoid access to command ring. */
};

struct ionic_adminq {
	char name[IONIC_QUEUE_NAME_MAX_SZ];

	struct ionic_lif *lif;
	unsigned int num_descs;

	unsigned int pid;

	unsigned int index;
	unsigned int type;
	unsigned int hw_index;
	unsigned int hw_type;
	uint64_t dbval;

	bus_dma_tag_t buf_tag;
	struct ionic_dma_info cmd_dma; 		/* DMA ring for command and completion. */
	dma_addr_t cmd_ring_pa;
	dma_addr_t comp_ring_pa;
	uint32_t total_ring_size;

	bool stop;
	bool wq_stop;

	struct mtx mtx;
	char mtx_name[IONIC_QUEUE_NAME_MAX_SZ];
	unsigned int head_index;		/* Index for buffer and command descriptors. */
	unsigned int tail_index;
	unsigned int comp_index;		/* Index for completion descriptors. */
	int done_color; 			/* Expected completion color. */

	struct ionic_adminq_stats stats;
	struct intr intr;

	struct ionic_admin_ctx **ctx_ring;
	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct ionic_admin_cmd *cmd_ring;
	struct ionic_admin_comp *comp_ring;
};

struct ionic_notifyq {
	char name[IONIC_QUEUE_NAME_MAX_SZ];

	struct ionic_lif *lif;
	unsigned int num_descs;

	unsigned int pid;

	unsigned int index;
	unsigned int type;
	unsigned int hw_index;
	unsigned int hw_type;

	struct ionic_dma_info cmd_dma; 		/* DMA ring for command and completion. */
	dma_addr_t cmd_ring_pa;
	uint32_t total_ring_size;

	struct mtx mtx;
	char mtx_name[IONIC_QUEUE_NAME_MAX_SZ];
	int comp_index;						/* Index for completion descriptors. */
	uint64_t isr_count;
	uint64_t comp_count;

	struct intr intr;

	struct task task;
	struct taskqueue *taskq;
	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct ionic_notifyq_cmd *cmd_ring;
	union ionic_notifyq_comp *comp_ring;
};

struct ionic_rxque {
	char name[IONIC_QUEUE_NAME_MAX_SZ];

	struct ionic_lif *lif;
	unsigned int num_descs; /* Max number of descriptors. */

	unsigned int pid;

	unsigned int index;
	unsigned int type;
	unsigned int hw_index;
	unsigned int hw_type;
	uint64_t dbval;

	struct ionic_rx_buf *rxbuf; /* S/w rx buffer descriptors. */
	bus_dma_tag_t buf_tag;

	struct ionic_dma_info cmd_dma; /* DMA ring for command and completion. */
	dma_addr_t cmd_ring_pa;	
	dma_addr_t comp_ring_pa;
	dma_addr_t sg_ring_pa;
	uint32_t total_ring_size;

	struct mtx rx_mtx;
	char mtx_name[IONIC_QUEUE_NAME_MAX_SZ];

	unsigned int head_index;
	unsigned int tail_index;
	unsigned int comp_index;		/* Completion index. */
	int done_color; 			/* Expected completion color - 0/1. */

	struct ionic_rx_stats stats;
	struct intr intr;

	struct task task;			/* Queue completion handler. */
	struct task tx_task;			/* Tx deferred xmit handler. */
	struct taskqueue *taskq;

	struct lro_ctrl	lro;
	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct ionic_rxq_desc *cmd_ring;
	struct ionic_rxq_comp *comp_ring;
	struct ionic_rxq_sg_desc *sg_ring;	/* SG descriptors. */
};

/*
 * Transmit queue.
 * XXX: Interrupt resource for Tx is part of Rx.
 */
struct ionic_txque {
	char name[IONIC_QUEUE_NAME_MAX_SZ];

	struct ionic_lif *lif;
	unsigned int num_descs;

	unsigned int pid;

	unsigned int index;
	unsigned int type;
	unsigned int ver;
	unsigned int hw_index;
	unsigned int hw_type;
	uint64_t dbval;

	struct ionic_tx_buf *txbuf;	/* S/w rx buffer descriptors. */
	bus_dma_tag_t buf_tag;
	bus_dma_tag_t tso_buf_tag;
	struct ionic_dma_info cmd_dma; 	/* DMA ring for command and completion. */
	dma_addr_t cmd_ring_pa;
	dma_addr_t comp_ring_pa;
	dma_addr_t sg_ring_pa;
	uint32_t total_ring_size;
	uint8_t max_sg_elems;
	uint8_t sg_desc_stride;

	struct mtx tx_mtx;
	char mtx_name[IONIC_QUEUE_NAME_MAX_SZ];
	unsigned int head_index;	/* Index for buffer and command descriptors. */
	unsigned int tail_index;
	unsigned int comp_index;	/* Index for completion descriptors. */
	int done_color;			/* Expected completion color status. */

	unsigned long wdog_start;	/* In ticks */
	bool full;
	struct ionic_tx_stats stats;
	struct buf_ring	*br;

	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct ionic_txq_desc *cmd_ring;
	struct ionic_txq_comp *comp_ring;
	struct ionic_txq_sg_elem *sg_ring;	/* SG descriptors. */
};

struct ionic_mc_addr {
	uint8_t  addr[ETHER_ADDR_LEN];
	bool present;
};

struct ionic_lif_cfg {
	int index;
	enum ionic_api_prsn prsn;

	void *priv;
	void (*reset_cb)(void *priv);
};

struct ionic_lif {
	char name[LIF_NAME_MAX_SZ];
	struct list_head list;
	struct net_device *netdev;

	uint8_t dev_addr[ETHER_ADDR_LEN] __aligned(sizeof(int));

	struct ionic *ionic;

	struct ifmedia          media;
	bool registered;

	unsigned int index;
	unsigned int hw_index;

	unsigned int kern_pid;
	uint64_t __iomem *kern_dbpage;

	struct workqueue_struct *adminq_wq;
	struct ionic_adminq *adminq;

	struct mtx wdog_mtx;
	struct workqueue_struct *wdog_wq;

	struct delayed_work adq_hb_work;
	unsigned long adq_hb_interval;
	bool adq_hb_resched;

	struct delayed_work txq_wdog_work;
	unsigned long txq_wdog_timeout;
	bool txq_wdog_resched;

	struct ionic_notifyq *notifyq;
	struct ionic_txque **txqs;
	struct ionic_rxque **rxqs;

	unsigned int nnqs;
	unsigned int nrdma_eqs;
	unsigned int nrdma_eqs_avail;
	unsigned int ntxqs;
	unsigned int nrxqs;

	unsigned int rx_mode;
	uint8_t cos;

	int rx_mbuf_size;		/* Rx mbuf size pool. */
	uint16_t max_frame_size;	/* MTU size. */

	uint32_t hw_features;		/* Features enabled in hardware, e.g. checksum, TSO etc. */

	uint16_t rss_types;
	uint8_t rss_hash_key[IONIC_RSS_HASH_KEY_SIZE];
	uint8_t *rss_ind_tbl;
	dma_addr_t rss_ind_tbl_pa;
	struct ionic_dma_info rss_dma;
	uint32_t rss_ind_tbl_sz;

	int intr_coalesce_us;		/* Interrupt coalescing value in us. */
	int intr_coalesce_max_us;	/* Interrupt coalescing maximum value in us. */

	struct mutex dbid_inuse_lock;
	unsigned long *dbid_inuse;
	unsigned int dbid_count;

	bool stay_registered;
	/* TODO: Make this a list if more than one slave is supported */
	struct ionic_lif_cfg slave_lif_cfg;

	uint64_t spurious; /* Spurious interrupt counter in legacy mode. */

	struct sysctl_oid *sysctl_ifnet;
	struct sysctl_ctx_list sysctl_ctx;
	struct sx sx;
	char sx_name[IONIC_QUEUE_NAME_MAX_SZ];

	struct rx_filters rx_filters;

	struct ionic_mc_addr *mc_addrs;
	int num_mc_addrs;

	/* 4096 bit array for VLAN. */
	uint8_t vlan_bitmap[4096 / 8];
	int num_vlans;
	eventhandler_tag vlan_attach;
	eventhandler_tag vlan_detach;

	uint64_t last_eid;
#ifdef NETAPP_PATCH
	bool iff_up;
#endif
	bool link_up;
	bool stop;
	bool reinit_in_progress;

	uint32_t link_speed;		/* units of 1Mbps: e.g. 10000 = 10Gbps */
	uint16_t link_down_count;

	uint32_t info_sz;
	dma_addr_t info_pa;
	struct ionic_dma_info info_dma;
	struct ionic_lif_info *info;

	uint64_t num_dev_cmds;
	uint64_t num_resets;
	uint32_t wdog_error_trigger;
};


/* lif lock. */
#define IONIC_LIF_LOCK_INIT(x)		sx_init(&(x)->sx, (x)->sx_name)
#define IONIC_LIF_LOCK_DESTROY(x)	sx_destroy(&(x)->sx)
#define IONIC_LIF_LOCK(x)		sx_xlock(&(x)->sx)
#define IONIC_LIF_UNLOCK(x)		sx_xunlock(&(x)->sx)
#define IONIC_LIF_LOCK_OWNED(x)		sx_xlocked(&(x)->sx)

#define IONIC_ADMIN_LOCK_INIT(x) 	mtx_init(&(x)->mtx, (x)->mtx_name, NULL, MTX_DEF)
#define IONIC_ADMIN_LOCK_DESTROY(x)	mtx_destroy(&(x)->mtx)
#define IONIC_ADMIN_LOCK(x)		mtx_lock(&(x)->mtx);
#define IONIC_ADMIN_UNLOCK(x)		mtx_unlock(&(x)->mtx);
#define IONIC_ADMIN_LOCK_OWNED(x) 	mtx_owned(&(x)->mtx)

#define IONIC_TX_LOCK_INIT(x)		mtx_init(&(x)->tx_mtx, (x)->mtx_name, NULL, MTX_DEF)
#define IONIC_TX_LOCK_DESTROY(x) 	mtx_destroy(&(x)->tx_mtx)
#define IONIC_TX_LOCK(x)		mtx_lock(&(x)->tx_mtx)
#define IONIC_TX_TRYLOCK(x)		mtx_trylock(&(x)->tx_mtx)
#define IONIC_TX_UNLOCK(x)		mtx_unlock(&(x)->tx_mtx)
#define IONIC_TX_LOCK_OWNED(x)		mtx_owned(&(x)->tx_mtx)

#define IONIC_RX_LOCK_INIT(x)		mtx_init(&(x)->rx_mtx, (x)->mtx_name, NULL, MTX_DEF)
#define IONIC_RX_LOCK_DESTROY(x)	mtx_destroy(&(x)->rx_mtx)
#define IONIC_RX_LOCK(x)		mtx_lock(&(x)->rx_mtx)
#define IONIC_RX_UNLOCK(x)		mtx_unlock(&(x)->rx_mtx)
#define IONIC_RX_LOCK_OWNED(x)		mtx_owned(&(x)->rx_mtx)

#define IONIC_MOD_INC(q, index) (((q)->index + 1) % (q)->num_descs)

#define IONIC_Q_EMPTY(q)	((q)->head_index == (q)->tail_index)
#define IONIC_Q_LENGTH(q)	(((q)->head_index - (q)->tail_index) % (q)->num_descs)
#define IONIC_Q_REMAINING(q)	((q)->num_descs - IONIC_Q_LENGTH(q) - 1)
#define IONIC_Q_FULL(q)		(IONIC_Q_REMAINING(q) == 0)

int ionic_stop(struct ifnet *ifp);
void ionic_open_or_stop(struct ionic_lif *lif);

int ionic_lif_identify(struct ionic *ionic);
int ionic_lifs_alloc(struct ionic *ionic);
void ionic_lifs_free(struct ionic *ionic);
void ionic_lifs_deinit(struct ionic *ionic);
int ionic_lifs_init(struct ionic *ionic);
int ionic_lifs_register(struct ionic *ionic);
void ionic_lifs_unregister(struct ionic *ionic);
int ionic_lifs_size(struct ionic *ionic);

int ionic_slave_alloc(struct ionic *ionic, enum ionic_api_prsn prsn);
void ionic_slave_free(struct ionic *ionic, int index);

int ionic_txq_identify(struct ionic *ionic, uint8_t ver);

int ionic_adminq_clean(struct ionic_adminq* adminq, int limit);
int ionic_notifyq_clean(struct ionic_notifyq* notifyq);

int ionic_dev_intr_reserve(struct ionic_lif *lif, struct intr *intr);
void ionic_dev_intr_unreserve(struct ionic_lif *lif, struct intr *intr);

struct ionic_lif *ionic_netdev_lif(struct ifnet *ifp);

int ionic_set_hw_features(struct ionic_lif *lif, uint32_t features);

int ionic_lif_rss_config(struct ionic_lif *lif, uint16_t types,
	const uint8_t *key, const uint32_t *indir);

void ionic_rx_fill(struct ionic_rxque *rxq);
int ionic_rx_clean(struct ionic_rxque *rxq, int rx_limit);
void ionic_rx_input(struct ionic_rxque *rxq, struct ionic_rx_buf *buf,
		struct ionic_rxq_comp *comp, struct ionic_rxq_desc *desc);

void ionic_tx_ring_doorbell(struct ionic_txque *txq, int index);
int ionic_tx_clean(struct ionic_txque* txq, int tx_limit);

int ionic_change_mtu(struct ifnet *ifp, int new_mtu);
void ionic_set_rx_mode(struct ifnet *ifp);

int ionic_set_multi(struct ionic_lif* lif);

int ionic_set_mac(struct ifnet *ifp);
int ionic_lif_quiesce(struct ionic_lif *lif);
int ionic_lif_reinit(struct ionic_lif *lif, bool wdog_reset_path);
void ionic_lif_rx_mode(struct ionic_lif *lif, unsigned int rx_mode);

void ionic_adminq_hb_resched(struct ionic_lif *lif);
void ionic_txq_wdog_resched(struct ionic_lif *lif);

int ionic_setup_intr_coal(struct ionic_lif *lif, int coal);
int ionic_firmware_update(struct ionic_lif *lif, const void *const fw_data, size_t fw_sz);

int ionic_lif_reset_stats(struct ionic_lif *lif);

extern int ionic_devcmd_timeout;
extern int ionic_rx_stride;
extern int ionic_tx_stride;
extern int ionic_rx_sg_size;
extern int ionic_tx_descs;
extern int ionic_rx_descs;
extern int adminq_descs;
extern int ionic_notifyq_descs;
extern int ionic_rx_fill_threshold;
extern int ionic_rx_process_limit;
extern int ionic_intr_coalesce;
extern int ionic_adminq_hb_interval;
extern int ionic_txq_wdog_timeout;

#endif /* _IONIC_LIF_H_ */
