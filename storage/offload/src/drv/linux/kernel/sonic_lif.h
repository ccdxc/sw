/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef _SONIC_LIF_H_
#define _SONIC_LIF_H_

#include "sonic.h"

#define ADMINQ_LENGTH	16
#define NOTIFYQ_LENGTH	64

struct tx_stats {
	u64 dma_map_err;
	u64 pkts;
	u64 bytes;
	u64 clean;
	u64 drop;
	u64 linearize;
	u64 stop;
	u64 no_csum;
	u64 csum;
	u64 crc32_csum;
	u64 tso;
	u64 frags;
};

struct rx_stats {
	u64 dma_map_err;
	u64 alloc_err;
	u64 pkts;
	u64 bytes;
};

#define QCQ_F_INITED		BIT(0)
#define QCQ_F_SG		BIT(1)
#define QCQ_F_INTR		BIT(2)
#define QCQ_F_TX_STATS		BIT(3)
#define QCQ_F_RX_STATS		BIT(4)
#define QCQ_F_NOTIFYQ		BIT(5)

struct qcq {
	struct queue q;
	struct cq cq;
	struct intr intr;
	struct napi_struct napi;
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
	DW_TYPE_RESET,
} __attribute__((packed));

enum reset_ctl_state {
	RESET_CTL_ST_IDLE,
	RESET_CTL_ST_PRE_RESET,
	RESET_CTL_ST_RESET,
	RESET_CTL_ST_REINIT,
	RESET_CTL_ST_SHUTDOWN,

	RESET_CTL_ST_MAX /* must be last */
};

struct deferred_work {
	struct list_head list;
	enum deferred_work_type type;
	enum reset_ctl_state reset_state;
	uint64_t timestamp;
	u32 reason_code;
	u8 dont_free;
};

struct deferred {
	spinlock_t lock;
	struct list_head list;
	//struct work_struct work;
	struct delayed_work dwork;
	struct workqueue_struct *wq;
};

typedef int (*reset_ctl_cb)(struct lif *lif,
			    void *cb_arg);

#define MAX_RESET_CTL_SUSPECT_INFO_COUNT NOTIFYQ_LENGTH

struct reset_ctl {
        struct deferred_work work;
	atomic_t state;
	reset_ctl_cb cbs[RESET_CTL_ST_MAX];
	void *cb_args[RESET_CTL_ST_MAX];
	int8_t sense;
	bool uncond_desc_notify;
	atomic_t suspect_info_count;
	atomic_t suspect_info_sorted_count;
	uint64_t suspect_info[MAX_RESET_CTL_SUSPECT_INFO_COUNT];
};

#define LIF_F_INITED		BIT(0)

#define LIF_NAME_MAX_SZ		(32)
#define LIF_SEQ_Q_BATCH_HT_ORDER (4)
#define LIF_SEQ_Q_BATCH_HT_SZ	(1 << LIF_SEQ_Q_BATCH_HT_ORDER)

typedef int (*seq_q_batch_ht_cb)(struct seq_queue_batch *batch,
				 void *cb_arg);
struct lif {
	char name[LIF_NAME_MAX_SZ];
	struct list_head list;
	struct hlist_head seq_q_batch_ht[LIF_SEQ_Q_BATCH_HT_SZ];
	struct seq_queue_batch *curr_seq_q_batch;
	struct net_device dummy_netdev;
	struct sonic *sonic;
	bool registered;
	unsigned int num_seq_q_batches;
	unsigned int index;
	unsigned int lif_id;
	unsigned int seq_q_index;
	spinlock_t adminq_lock;
	struct qcq *adminqcq;
	struct qcq *notifyqcq;
	uint64_t last_eid;
	union stats_dump *stats_dump;
	dma_addr_t stats_dump_pa;
	struct deferred deferred;
	u32 tx_coalesce_usecs;
	u32 rx_coalesce_usecs;
	void *api_private;
	struct dentry *dentry;
	unsigned int flags;
	struct res res;
	struct reset_ctl reset_ctl;

	uint64_t num_resets;
};

enum sonic_queue_type {
	SONIC_QTYPE_CP_SQ,
	SONIC_QTYPE_DC_SQ,
	SONIC_QTYPE_CPDC_STATUS,
	SONIC_QTYPE_CRYPTO_ENC_SQ,
	SONIC_QTYPE_CRYPTO_DEC_SQ,
	SONIC_QTYPE_CRYPTO_STATUS
};

#define SONIC_SEQ_Q_DESC_SIZE        64
#define SONIC_SEQ_STATUS_Q_DESC_SIZE 128

extern struct lif *sonic_glif;

#define lif_to_txq(lif, i)	(&lif->txqcqs[i]->q)
#define lif_to_rxq(lif, i)	(&lif->rxqcqs[i]->q)

int sonic_lifs_alloc(struct sonic *sonic);
void sonic_lifs_free(struct sonic *sonic);
void sonic_lifs_deinit(struct sonic *sonic);
int sonic_lifs_init(struct sonic *sonic);
int sonic_lifs_register(struct sonic *sonic);
void sonic_lifs_unregister(struct sonic *sonic);
int sonic_lifs_size(struct sonic *sonic);
void sonic_lif_reset_ctl_register(struct lif *lif,
				  enum reset_ctl_state state,
				  reset_ctl_cb cb,
				  void *cb_arg);
int sonic_lif_reset_ctl_start(struct lif *lif);
int sonic_lif_reset_ctl_pre_reset(struct lif *lif, void *cb_arg);
int sonic_lif_reset_ctl_reset(struct lif *lif, void *cb_arg);
int sonic_lif_reset_ctl_reinit(struct lif *lif, void *cb_arg);
int sonic_lif_reset_ctl_shutdown(struct lif *lif, void *cb_arg);
int sonic_lif_reset_ctl_end(struct lif *lif, void *cb_arg);
bool sonic_lif_reset_ctl_pending(struct lif *lif);
bool sonic_lif_reset_addr_is_suspect(struct lif *lif, void *addr);
int sonic_lif_reinit(struct lif *lif);

int sonic_intr_alloc(struct lif *lif, struct intr *intr);
void sonic_intr_free(struct lif *lif, struct intr *intr);

int sonic_get_per_core_seq_sq(struct per_core_resource *res,
		enum sonic_queue_type qtype,
		struct queue **q);
int sonic_get_seq_sq(struct lif *lif, enum sonic_queue_type qtype,
		struct queue **q);

int sonic_get_seq_statusq(struct lif *lif, enum sonic_queue_type qtype,
		struct queue **ret_q_hint);
void sonic_put_seq_statusq(struct queue *q);

uint32_t sonic_get_num_per_core_res(struct lif *lif);
struct per_core_resource *sonic_get_per_core_res_by_res_id(struct lif *lif,
		uint32_t res_id);
struct per_core_resource *sonic_get_per_core_res(struct lif *lif);

void sonic_reserve_per_core_res(struct per_core_resource *pcr);
bool sonic_try_reserve_per_core_res(struct per_core_resource *pcr);
void sonic_unreserve_per_core_res(struct per_core_resource *pcr);

void sonic_reserve_exclusive_per_core_res(struct per_core_resource *pcr);
bool sonic_try_reserve_exclusive_per_core_res(struct per_core_resource *pcr);
void sonic_unreserve_exclusive_per_core_res(struct per_core_resource *pcr);

bool sonic_is_reserved_per_core_res(struct per_core_resource *pcr);

static inline struct lif *
sonic_get_lif(void)
{
	return sonic_glif;
}

static inline struct sonic_dev *
sonic_get_idev(void)
{
	return sonic_glif && sonic_glif->sonic ?
	       &sonic_glif->sonic->idev : NULL;
}

static inline uint32_t
sonic_get_seq_sq_num_descs(struct lif *lif,
			   enum sonic_queue_type sonic_qtype)
{
	struct queue	*q;
	struct per_core_resource *pc_res;

	pc_res = sonic_get_per_core_res_by_res_id(lif, 0);
	if (!sonic_get_per_core_seq_sq(pc_res, sonic_qtype, &q))
		return q->num_descs;

	return 0;
}

#endif /* _SONIC_LIF_H_ */
