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

#ifndef _SONIC_LIF_H_
#define _SONIC_LIF_H_


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

struct qcq {
	void *base;
	dma_addr_t base_pa;
	unsigned int total_size;
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
	DW_TYPE_RX_MODE,
	DW_TYPE_RX_ADDR_ADD,
	DW_TYPE_RX_ADDR_DEL,
};

struct deferred_work {
	struct list_head list;
	enum deferred_work_type type;
};

struct deferred {
	spinlock_t lock;
	struct list_head list;
	struct work_struct work;
};

#define LIF_F_INITED		BIT(0)

#define LIF_NAME_MAX_SZ		(32)
struct lif {
	char name[LIF_NAME_MAX_SZ];
	struct list_head list;
	struct net_device dummy_netdev;
	struct sonic *sonic;
	bool registered;
	unsigned int index;
	spinlock_t adminq_lock;
	struct qcq *adminqcq;
	union stats_dump *stats_dump;
	dma_addr_t stats_dump_pa;
	struct deferred deferred;
	u32 tx_coalesce_usecs;
	u32 rx_coalesce_usecs;
	void *api_private;
	struct dentry *dentry;
	unsigned int flags;
	struct res res;
};

enum sonic_queue_type {
	SONIC_QTYPE_CP_SUB,
	SONIC_QTYPE_DC_SUB,
	SONIC_QTYPE_CPDC_STATUS,
	SONIC_QTYPE_CRYPTO_ENC_SUB,
	SONIC_QTYPE_CRYPTO_DEC_SUB,
	SONIC_QTYPE_CRYPTO_STATUS
};

#define SONIC_SEQ_Q_DESC_SIZE        64
#define SONIC_SEQ_STATUS_Q_DESC_SIZE 128

#define lif_to_txq(lif, i)	(&lif->txqcqs[i]->q)
#define lif_to_rxq(lif, i)	(&lif->rxqcqs[i]->q)

int sonic_lifs_alloc(struct sonic *sonic);
void sonic_lifs_free(struct sonic *sonic);
void sonic_lifs_deinit(struct sonic *sonic);
int sonic_lifs_init(struct sonic *sonic);
int sonic_lifs_register(struct sonic *sonic);
void sonic_lifs_unregister(struct sonic *sonic);
int sonic_lifs_size(struct sonic *sonic);

int sonic_intr_alloc(struct lif *lif, struct intr *intr);
void sonic_intr_free(struct lif *lif, struct intr *intr);

int sonic_lif_cpdc_seq_qs_init(struct per_core_resource *res);
int sonic_lif_crypto_seq_qs_init(struct per_core_resource *res);
int sonic_lif_cpdc_seq_qs_control(struct per_core_resource *res, uint16_t opcode);
int sonic_lif_crypto_seq_qs_control(struct per_core_resource *res, uint16_t opcode);

int get_seq_subq(struct lif *lif, enum sonic_queue_type qtype, struct queue **q); 
int alloc_seq_statusq(struct lif *lif, enum sonic_queue_type qtype, struct queue **q);
int free_seq_statusq(struct lif *lif, enum sonic_queue_type qtype, struct queue **q);

struct lif* sonic_get_lif(void);

#endif /* _SONIC_LIF_H_ */
