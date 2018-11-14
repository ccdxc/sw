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

#ifndef _IONIC_LIF_H_
#define _IONIC_LIF_H_

#include <linux/if_ether.h>

#include <linux/workqueue.h>

#include "ionic_osdep.h"

#ifndef napi_struct
#define napi_struct work_struct
#endif

#define netif_napi_add(n, napi, poll, wt) do {	\
	(void)(n);				\
	(void)(wt);				\
	INIT_WORK(napi, poll);			\
} while (0)

#define netif_napi_del(napi) do {	\
	(void)(napi);			\
} while (0)

#define napi_enable schedule_work
#define napi_disable cancel_work_sync
#define napi_schedule schedule_work
#define napi_schedule_irqoff schedule_work
#define napi_complete_done(napi, done) ((void)(napi), (void)(done), 1)

#define NAPI_POLL_WEIGHT 64

struct napi_struct;

struct tx_stats {
	u64 dma_map_err;
	u64 pkts;
	u64 bytes;
	u64 clean;
	u64 drop;
	u64 linearize;
	u64 no_descs;
	u64 no_csum_offload;
	u64 csum_offload;
	//u64 crc32_csum;
	u64 tso_ipv4;
	u64 tso_ipv6;
	//u64 frags;
	u64 bad_ethtype;	/* Unknow Ethernetype frame. */
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
	uint64_t pa_addr; /* cache address to avoid access to command ring. */
};

struct ionic_tx_buf {
	struct mbuf *m;
	bus_dmamap_t dma_map;
	uint64_t pa_addr; /* cache address to avoid access to command ring. */
};

#define QUEUE_NAME_MAX_SZ		(32)

/* Top level Rx Q mgmt. */
struct rxque {
	char name[QUEUE_NAME_MAX_SZ];

	struct lif *lif;
	unsigned int num_descs; /* max number of descriptors. */
	uint32_t descs;	/* Descriptors posted in queue. */
	unsigned int index;	/* Queue number. */
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;

	/* S/w rx buffer descriptors. */
	struct ionic_rx_buf *rxbuf;
	bus_dma_tag_t buf_tag;

	/* DMA ring for command and completion h/w rings. */
	struct ionic_dma_info cmd_dma;

	struct doorbell __iomem *db;
	/* Cache DMA address */
	dma_addr_t cmd_ring_pa;

	struct mtx rx_mtx;

	/* Index for buffer and command descriptors. */
	int cmd_head_index;
	int cmd_tail_index;

	/* Index for completion descriptors. */
	int comp_index;

	int done_color; /* Expected comletion color status. */

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

/* Top level Tx Q mgmt. */
struct txque {
	char name[QUEUE_NAME_MAX_SZ];

	struct lif *lif;
	unsigned int num_descs;
	unsigned int index;	/* Queue number. */
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;

	/* S/w rx buffer descriptors. */
	struct ionic_tx_buf *txbuf;
	bus_dma_tag_t buf_tag;

	/* DMA ring for command and completion h/w rings. */
	struct ionic_dma_info cmd_dma;
	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct txq_desc *cmd_ring;
	struct txq_comp *comp_ring;
	struct txq_sg_desc *sg_ring;	/* SG descriptors. */

	struct doorbell __iomem *db;
	/* Cache DMA address */
	dma_addr_t cmd_ring_pa;

	struct mtx tx_mtx;

	/* Index for buffer and command descriptors. */
	int cmd_head_index;
	int cmd_tail_index;

	/* Index for completion descriptors. */
	int comp_index;
	int done_color; /* Expected comletion color status. */
	
	struct tx_stats stats;
	struct intr intr;

	struct task task;
	struct taskqueue *taskq;
	struct buf_ring		*br;
};


/* Top level admin Q mgmt. */
struct adminq {
	char name[QUEUE_NAME_MAX_SZ];

	struct lif *lif;
	unsigned int num_descs;
	unsigned int index;	/* Queue number. */
	unsigned int pid;
	unsigned int qid;
	unsigned int qtype;

	/* S/w rx buffer descriptors. */
//	struct ionic_tx_buf *txbuf;
	bus_dma_tag_t buf_tag;

	/* DMA ring for command and completion h/w rings. */
	struct ionic_dma_info cmd_dma;
	/*
	 * H/w command and completion descriptor rings.
	 * Points to area allocated by DMA.
	 */
	struct admin_cmd *cmd_ring;
	struct admin_comp *comp_ring;

	struct doorbell __iomem *db;
	/* Cache DMA address */
	dma_addr_t cmd_ring_pa;

	struct mtx mtx;

	/* Index for buffer and command descriptors. */
	int cmd_head_index;
	int cmd_tail_index;

	/* Index for completion descriptors. */
	int comp_index;
	int done_color; /* Expected comletion color status. */
	
	struct intr intr;

	struct napi_struct napi;
	//struct task task;
	//struct taskqueue *taskq;
};

#define LIF_NAME_MAX_SZ			(32)

struct lif {
	char name[LIF_NAME_MAX_SZ];
	struct list_head list;

	struct net_device *netdev;

	u8 dev_addr[ETHER_ADDR_LEN] __aligned(sizeof(int));

	struct ionic *ionic;

	struct ifmedia          media;
	bool registered;

	unsigned int index;

	unsigned int kern_pid;
	struct doorbell __iomem *kern_dbpage;

	struct workqueue_struct *adminq_wq;
	struct mtx adminq_lock;
	struct adminq *adminqcq;

	struct txque **txqs;
	struct rxque **rxqs;

	unsigned int neqs;
	unsigned int ntxqs;
	unsigned int nrxqs;

	unsigned int rx_mode;

	int rx_mbuf_size;		/* Rx mbuf size pool. */
	uint16_t max_frame_size; /* MTU size. */

	u32 hw_features;	/* Features enabled in hardware, e.g. checksum, TSO etc. */

	union stats_dump *stats_dump;
	dma_addr_t stats_dump_pa;

	u8 rss_hash_key[RSS_HASH_KEY_SIZE];
	u8 *rss_ind_tbl;
	dma_addr_t rss_ind_tbl_pa;

	u32 tx_coalesce_usecs;
	u32 rx_coalesce_usecs;

	struct mutex dbid_inuse_lock;
	unsigned long *dbid_inuse;
	unsigned int dbid_count;

	void *api_private;
	void (*api_reset_cb)(void *api_private);

	struct sysctl_oid *sysctl_ifnet;
	struct sysctl_ctx_list sysctl_ctx;
	struct mtx mtx;
};

#ifdef OVERRIDE_KASSERT
#undef KASSERT
/* Override KASSERT for debug only. */
#define KASSERT(c, msg)	do {												\
							if (!(c)) {										\
								printf("PANIC[%s:%d]",  __func__, __LINE__);\
								printf msg;									\
								printf("\n");								\
							}												\
} while(0)
#endif

/* lif lock. */
#define IONIC_CORE_LOCK(x)		mtx_lock(&(x)->mtx)
#define IONIC_CORE_UNLOCK(x)	mtx_unlock(&(x)->mtx)
#define IONIC_CORE_OWNED(x)		mtx_owned(&(x)->mtx)

#define IONIC_ADMIN_LOCK_INIT(x) mtx_init(&(x)->adminq_lock, "adminq", NULL, MTX_DEF)

#define IONIC_ADMIN_LOCK(x)	do { 		\
		IONIC_INFO("adminq locked\n");	\
		mtx_lock(&(x)->adminq_lock); 	\
} while (0)

#define IONIC_ADMIN_UNLOCK(x)	do { 	\
		IONIC_INFO("adminq unlocked\n");\
		mtx_unlock(&(x)->adminq_lock);	\
} while (0)

#define IONIC_TX_INIT(x)	mtx_init(&(x)->tx_mtx, (x)->name, NULL, MTX_DEF)
#define IONIC_TX_DESTROY(x)	mtx_destroy(&(x)->tx_mtx)

#define IONIC_TX_LOCK(x)	mtx_lock(&(x)->tx_mtx)
#define IONIC_TX_TRYLOCK(x)	mtx_trylock(&(x)->tx_mtx)
#define IONIC_TX_UNLOCK(x)	mtx_unlock(&(x)->tx_mtx)
#define IONIC_TX_OWNED(x)	mtx_owned(&(x)->tx_mtx)

#define IONIC_RX_INIT(x)	mtx_init(&(x)->rx_mtx, (x)->name, NULL, MTX_DEF)
#define IONIC_RX_DESTROY(x)	mtx_destroy(&(x)->rx_mtx)

#define IONIC_RX_LOCK(x)	do {						\
				struct mtx* mtx = &(x)->rx_mtx;			\
				IONIC_INFO("%s locked\n",(x)->name);	\
				mtx_lock(mtx);							\
	} while(0)

#define IONIC_RX_UNLOCK(x)	do {						\
				struct mtx* mtx = &(x)->rx_mtx;			\
				IONIC_INFO("%s unlocked\n",(x)->name);	\
				mtx_unlock(mtx);						\
	} while(0)

#define IONIC_RX_OWNED(x)	mtx_owned(&(x)->rx_mtx)

void ionic_open(void *arg);
int ionic_stop(struct net_device *netdev);

int ionic_lifs_alloc(struct ionic *ionic);
void ionic_lifs_free(struct ionic *ionic);
void ionic_lifs_deinit(struct ionic *ionic);
int ionic_lifs_init(struct ionic *ionic);
int ionic_lifs_register(struct ionic *ionic);
void ionic_lifs_unregister(struct ionic *ionic);
int ionic_lifs_size(struct ionic *ionic);

int ionic_intr_alloc(struct lif *lif, struct intr *intr);
void ionic_intr_free(struct lif *lif, struct intr *intr);

struct lif *ionic_netdev_lif(struct net_device *netdev);

int ionic_reinit(struct net_device *netdev);
int ionic_set_features(struct lif *lif, uint16_t set_feature);
int ionic_set_hw_feature(struct lif *lif, uint16_t set_feature);

int ionic_rss_ind_tbl_set(struct lif *lif, const u32 *indir);
int ionic_rss_hash_key_set(struct lif *lif, const u8 *key, uint16_t rss_types);

void ionic_rx_input(struct rxque *rxq, struct ionic_rx_buf *buf,
			   struct rxq_comp *comp, 	struct rxq_desc *desc);

#ifdef IONIC_NAPI
void ionic_rx_napi(struct napi_struct *napi);
#endif
int ionic_tx_clean(struct txque* txq , int tx_limit);

int ionic_change_mtu(struct net_device *netdev, int new_mtu);
#endif /* _IONIC_LIF_H_ */
