/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef _IONIC_LIF_H_
#define _IONIC_LIF_H_

#define NOTIFYQ_LENGTH	64	/* must be a power of two */

#include "ionic_rx_filter.h"

/* Compile this out for removing debug stats */
#define IONIC_DEBUG_STATS

#define GET_NAPI_CNTR_IDX(work_done)	(work_done)
#define MAX_NUM_NAPI_CNTR	(NAPI_POLL_WEIGHT + 1)
#define GET_SG_CNTR_IDX(num_sg_elems)	(num_sg_elems)
#define MAX_NUM_SG_CNTR		(IONIC_TX_MAX_SG_ELEMS + 1)

struct tx_stats {
	u64 dma_map_err;
	u64 pkts;
	u64 bytes;
	u64 clean;
	u64 drop;
	u64 linearize;
	u64 stop;
	u64 wake;
	u64 no_csum;
	u64 csum;
	u64 crc32_csum;
	u64 tso;
	u64 frags;
	u64 sg_cntr[MAX_NUM_SG_CNTR];
};

struct rx_stats {
	u64 dma_map_err;
	u64 alloc_err;
	u64 pkts;
	u64 bytes;
	u64 csum_none;
	u64 csum_complete;
	u64 buffers_posted;
};

#define QCQ_F_INITED		BIT(0)
#define QCQ_F_SG		BIT(1)
#define QCQ_F_INTR		BIT(2)
#define QCQ_F_TX_STATS		BIT(3)
#define QCQ_F_RX_STATS		BIT(4)
#define QCQ_F_NOTIFYQ		BIT(5)

struct napi_stats {
	u64 poll_count;
	u64 work_done_cntr[MAX_NUM_NAPI_CNTR];
};

struct qcq {
	void *base;
	dma_addr_t base_pa;
	unsigned int total_size;
	struct queue q;
	struct cq cq;
	struct intr intr;
	struct napi_struct napi;
	struct napi_stats napi_stats;
	union {
		struct tx_stats tx;
		struct rx_stats rx;
	} stats;
	unsigned int flags;
};

#define q_to_qcq(q)		container_of(q, struct qcq, q)
#define q_to_tx_stats(q)	(&q_to_qcq(q)->stats.tx)
#define q_to_rx_stats(q)	(&q_to_qcq(q)->stats.rx)
#define napi_to_qcq(napi)	container_of(napi, struct qcq, napi)
#define napi_to_cq(napi)	(&napi_to_qcq(napi)->cq)

enum deferred_work_type {
	DW_TYPE_RX_MODE,
	DW_TYPE_RX_ADDR_ADD,
	DW_TYPE_RX_ADDR_DEL,
	DW_TYPE_LINK_STATUS,
};

struct deferred_work {
	struct list_head list;
	enum deferred_work_type type;
	union {
		unsigned int rx_mode;
		u8 addr[ETH_ALEN];
	};
};

struct deferred {
	spinlock_t lock;
	struct list_head list;
	struct work_struct work;
};

struct lif_sw_stats {
	u64 tx_packets;
	u64 tx_bytes;
	u64 rx_packets;
	u64 rx_bytes;
	u64 tx_tso;
	u64 rx_csum_none;
	u64 rx_csum_unnecessary;
	u64 rx_csum_complete;
	u64 rx_csum_tcp_bad;
	u64 rx_csum_udp_bad;
	u64 rx_csum_ip_bad;
};

enum lif_state_flags {
	LIF_INITED,
	LIF_SW_DEBUG_STATS,
	LIF_UP,
	LIF_LINK_CHECK_NEEDED,

	/* leave this as last */
	LIF_STATE_SIZE
};

#define LIF_NAME_MAX_SZ		(32)
struct lif {
	char name[LIF_NAME_MAX_SZ];
	struct list_head list;
	struct net_device *netdev;
	DECLARE_BITMAP(state, LIF_STATE_SIZE);
	struct ionic *ionic;
	bool registered;
	unsigned int index;
	unsigned int kern_pid;
	struct doorbell __iomem *kern_dbpage;
	spinlock_t adminq_lock;
	struct qcq *adminqcq;
	struct qcq *notifyqcq;
	struct qcq **txqcqs;
	struct qcq **rxqcqs;
	u64 last_eid;
	unsigned int neqs;
	unsigned int nxqs;
	unsigned int ntxq_descs;
	unsigned int nrxq_descs;
	unsigned int rx_mode;
	u32 hw_features;
	struct ionic_lif_stats *lif_stats;
	dma_addr_t lif_stats_pa;
	u8 rss_hash_key[RSS_HASH_KEY_SIZE];
	u8 *rss_ind_tbl;
	dma_addr_t rss_ind_tbl_pa;
	struct rx_filters rx_filters;
	struct deferred deferred;
	u32 tx_coalesce_usecs;
	u32 rx_coalesce_usecs;
	struct mutex dbid_inuse_lock;
	unsigned long *dbid_inuse;
	unsigned int dbid_count;
	void *api_private;
	void (*api_reset_cb)(void *api_private);
	struct dentry *dentry;
	u32 flags;
	u32 notifyblock_sz;
	struct notify_block *notifyblock;
	dma_addr_t notifyblock_pa;
};

#define lif_to_txqcq(lif, i)	(lif->txqcqs[i])
#define lif_to_rxqcq(lif, i)	(lif->rxqcqs[i])
#define lif_to_txq(lif, i)	(&lif_to_txqcq(lif, i)->q)
#define lif_to_rxq(lif, i)	(&lif_to_txqcq(lif, i)->q)

int ionic_lifs_alloc(struct ionic *ionic);
void ionic_lifs_free(struct ionic *ionic);
void ionic_lifs_deinit(struct ionic *ionic);
int ionic_lifs_init(struct ionic *ionic);
int ionic_lifs_register(struct ionic *ionic);
void ionic_lifs_unregister(struct ionic *ionic);
int ionic_lifs_size(struct ionic *ionic);

int ionic_intr_alloc(struct lif *lif, struct intr *intr);
void ionic_intr_free(struct lif *lif, struct intr *intr);
int ionic_open(struct net_device *netdev);
int ionic_stop(struct net_device *netdev);

struct lif *ionic_netdev_lif(struct net_device *netdev);

#ifdef IONIC_DEBUG_STATS
static void inline debug_stats_txq_post(struct qcq *qcq,
	   struct txq_desc *desc, bool dbell)
{
	u64 num_sg_elems = desc->num_sg_elems;
	u64 sg_cntr_idx;

	qcq->q.dbell_count += dbell;

	sg_cntr_idx = GET_SG_CNTR_IDX(num_sg_elems);
	if (sg_cntr_idx > (MAX_NUM_SG_CNTR - 1))
		sg_cntr_idx = MAX_NUM_SG_CNTR - 1;

	qcq->stats.tx.sg_cntr[sg_cntr_idx]++;
}

static inline void debug_stats_napi_poll(struct qcq *qcq,
	   unsigned int work_done)
{
	u32 napi_cntr_idx;
	qcq->napi_stats.poll_count++;

	napi_cntr_idx = GET_NAPI_CNTR_IDX(work_done);
	if (napi_cntr_idx > (MAX_NUM_NAPI_CNTR - 1))
		napi_cntr_idx = MAX_NUM_NAPI_CNTR - 1;

	qcq->napi_stats.work_done_cntr[napi_cntr_idx]++;
}

#define DEBUG_STATS_CQE_CNT(cq) \
	do { \
		(cq)->compl_count++; \
	} while (0);

#define DEBUG_STATS_RX_BUFF_CNT(qcq) \
	do { \
		(qcq)->stats.rx.buffers_posted++; \
	} while (0);

#define DEBUG_STATS_INTR_REARM(intr) \
	do { \
		(intr)->rearm_count++; \
	} while (0);

#define DEBUG_STATS_TXQ_POST(qcq, txdesc, dbell) \
	debug_stats_txq_post(qcq, txdesc, dbell)

#define DEBUG_STATS_NAPI_POLL(qcq, work_done) \
	debug_stats_napi_poll(qcq, work_done)

#else
#define DEBUG_STATS_RX_BUFF_CNT(qcq)
#define DEBUG_STATS_TXQ_POST(qcq, txdesc, dbell)
#define DEBUG_STATS_NAPI_POLL(qcq, work_done)
#define DEBUG_STATS_INTR_REARM(intr)
#define DEBUG_STATS_CQE_CNT(cq)
#endif

#endif /* _IONIC_LIF_H_ */
