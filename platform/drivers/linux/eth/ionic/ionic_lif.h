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

#define LIF_NAME_MAX_SZ			(32)

struct lif {
	char name[LIF_NAME_MAX_SZ];
	struct list_head list;
	struct net_device *netdev;
	struct ionic *ionic;
	bool registered;
	unsigned int index;
	struct workqueue_struct *adminq_wq;
	spinlock_t adminq_lock;
	struct qcq *adminqcq;
	struct qcq **txqcqs;
	struct qcq **rxqcqs;
	unsigned int ntxqcqs;
	unsigned int nrxqcqs;
	unsigned int rx_mode;
	u32 hw_features;
	union stats_dump *stats_dump;
	dma_addr_t stats_dump_pa;
};

#define lif_to_txq(lif, i)	(&lif->txqcqs[i]->q)
#define lif_to_rxq(lif, i)	(&lif->rxqcqs[i]->q)

int ionic_lifs_alloc(struct ionic *ionic);
void ionic_lifs_free(struct ionic *ionic);
void ionic_lifs_deinit(struct ionic *ionic);
int ionic_lifs_init(struct ionic *ionic);
int ionic_lifs_register(struct ionic *ionic);
void ionic_lifs_unregister(struct ionic *ionic);
int ionic_lifs_size(struct ionic *ionic);

#endif /* _IONIC_LIF_H_ */
