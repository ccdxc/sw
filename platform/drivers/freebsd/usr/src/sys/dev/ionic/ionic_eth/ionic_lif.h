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
	u64 stop;
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
	u64 checksum_ip_ok;
	u64 checksum_ip_bad;
	u64 checksum_l4_ok;
	u64 checksum_l4_bad;
	u64 isr_count; 	// Not required.
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

#define QCQ_F_INITED		BIT(0)
#define QCQ_F_SG		BIT(1)
#define QCQ_F_INTR		BIT(2)
#define QCQ_F_TX_STATS		BIT(3)
#define QCQ_F_RX_STATS		BIT(4)

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

/* Top level Rx Q mgmt. */
struct rx_qcq {
	char name[QUEUE_NAME_MAX_SZ];

	struct lif *lif;
	unsigned int num_descs;
	unsigned int index;	/* Queue number. */
	unsigned int pid;
	unsigned int qid; 
	unsigned int qtype; 

	/* S/w rx buffer descriptors. */
	struct ionic_rx_buf *rxbuf;
	bus_dma_tag_t buf_tag;

	/* DMA ring for command and completion h/w rings. */
	struct ionic_dma_info cmd_dma;
	/* 
	 * H/w command and completion descriptor rings. 
	 * Points to area allocated by DMA.
	 */	
	struct rxq_desc *cmd_ring;
	struct rxq_comp *comp_ring;

	struct doorbell __iomem *db;
	/* Cache DMA address */
	dma_addr_t cmd_ring_pa;

	struct mtx mtx;

	/* Index for buffer and command descriptors. */
	int cmd_head_index;
	int cmd_tail_index;

	/* Index for completion descriptors. */
	int comp_index;

	int done_color;
		
	struct rx_stats stats;
	struct intr intr;

	struct task task;
	struct taskqueue *taskq;

	struct lro_ctrl	lro;
};

/* Top level Tx Q mgmt. */
struct tx_qcq {
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

	struct mtx mtx;

	/* Index for buffer and command descriptors. */
	int cmd_head_index;
	int cmd_tail_index;

	int sg_head_index;
	int sg_tail_index;
	/* Index for completion descriptors. */
	int comp_index;

	int done_color;
		
	struct tx_stats stats;
	struct intr intr;

//	struct task task;
//	struct taskqueue *taskq;
};

struct qcq {
	void *base;
	dma_addr_t base_pa;
	unsigned int total_size;
	//struct task task;
	///struct taskqueue *taskq;
	//struct lro_ctrl	lro;
	struct queue q;
	struct cq cq;
	struct intr intr;
	struct napi_struct napi;
	//union {
	//	struct tx_stats tx;
	//	struct rx_stats rx;
	//} stats;
	unsigned int flags;
};

//#define q_to_qcq(q)		container_of(q, struct qcq, q)
//#define q_to_tx_stats(q)	(&q_to_qcq(q)->stats.tx)
//#define q_to_rx_stats(q)	(&q_to_qcq(q)->stats.rx)
#define napi_to_qcq(napi)	container_of(napi, struct qcq, napi)
#define napi_to_cq(napi)	(&napi_to_qcq(napi)->cq)

#define LIF_NAME_MAX_SZ			(32)

struct lif {
	char name[LIF_NAME_MAX_SZ];
	struct list_head list;
	struct net_device *netdev;
	u8 dev_addr[ETHER_ADDR_LEN] __aligned(sizeof(int));
	struct ionic *ionic;
	bool registered;
	unsigned int index;
	struct workqueue_struct *adminq_wq;
	spinlock_t adminq_lock;
	struct qcq *adminqcq;
	struct tx_qcq **txqcqs;
	struct rx_qcq **rxqcqs;
	unsigned int neqs;
	unsigned int ntxqcqs;
	unsigned int nrxqcqs;
	unsigned int rx_mode;
	int buf_len;	// mbuf buffer length.
	u32 hw_features;
	union stats_dump *stats_dump;
	dma_addr_t stats_dump_pa;
	u8 rss_hash_key[RSS_HASH_KEY_SIZE];
	u8 *rss_ind_tbl;
	dma_addr_t rss_ind_tbl_pa;
	u32 tx_coalesce_usecs;
	u32 rx_coalesce_usecs;
	void *api_private;
	struct sysctl_oid *sysctl_ifnet;
	struct sysctl_ctx_list sysctl_ctx;
	struct mtx mtx;
};

//#define lif_to_txq(lif, i)	(&lif->txqcqs[i]->q)
//#define lif_to_rxq(lif, i)	(&lif->rxqcqs[i]->q)

void ionic_open(void *arg);

int ionic_lifs_alloc(struct ionic *ionic);
void ionic_lifs_free(struct ionic *ionic);
void ionic_lifs_deinit(struct ionic *ionic);
int ionic_lifs_init(struct ionic *ionic);
int ionic_lifs_register(struct ionic *ionic);
void ionic_lifs_unregister(struct ionic *ionic);
int ionic_lifs_size(struct ionic *ionic);

int ionic_intr_alloc(struct lif *lif, struct intr *intr);
void ionic_intr_free(struct lif *lif, struct intr *intr);

int ionic_reinit_unlock(struct net_device *netdev);
int ionic_set_features(struct lif *lif, uint16_t set_feature);

int ionic_rss_ind_tbl_set(struct lif *lif, const u32 *indir);
int ionic_rss_hash_key_set(struct lif *lif, const u8 *key, uint16_t rss_types);

void ionic_rx_fill(struct rx_qcq *rxqcq);
void ionic_rx_refill(struct rx_qcq *rxqcq);

void ionic_rx_empty(struct rx_qcq *rxqcq);
void ionic_rx_flush(struct rx_qcq *rxqcq);
#ifdef IONIC_NAPI
void ionic_rx_napi(struct napi_struct *napi);
#endif

void ionic_rx_input(struct rx_qcq *rxqcq, struct ionic_rx_buf *buf,
			   struct rxq_comp *comp, 	struct rxq_desc *desc);
#endif /* _IONIC_LIF_H_ */
