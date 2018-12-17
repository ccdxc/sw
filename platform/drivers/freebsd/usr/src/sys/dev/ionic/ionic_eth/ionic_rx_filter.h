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
#ifndef _IONIC_RX_FILTER_H_
#define _IONIC_RX_FILTER_H_

#define RXQ_INDEX_ANY		(0xFFFF)
struct rx_filter {
	u32 flow_id;
	u32 filter_id;
	u16 rxq_index;
	bool visited;  /* temp variable */
	struct rx_filter_add_cmd cmd;
	struct hlist_node by_hash;
	struct hlist_node by_id;
};

#define RX_FILTER_HLISTS		(1 << 10)
#define RX_FILTER_HLISTS_MASK	(RX_FILTER_HLISTS - 1)

struct rx_filters {
#ifdef __FreeBSD__
	struct mtx mtx;
#else
	spinlock_t lock;
#endif
	struct hlist_head by_hash[RX_FILTER_HLISTS];	/* by skb hash */
	struct hlist_head by_id[RX_FILTER_HLISTS];		/* by filter_id */
};

#ifdef __FreeBSD__
#define IONIC_RX_FILTER_INIT(x) 					\
	mtx_init(&(x)->mtx, "rx filter", NULL, MTX_DEF)
#define IONIC_RX_FILTER_DESTROY(x)	mtx_destroy(&(x)->mtx)
#define IONIC_RX_FILTER_LOCK(x)		mtx_lock(&(x)->mtx)
#define IONIC_RX_FILTER_UNLOCK(x)	mtx_unlock(&(x)->mtx)
#else
#define IONIC_RX_FILTER_INIT(x) 	spin_lock_init(&(x)->lock);
#define IONIC_RX_FILTER_DESTROY(x)
#define IONIC_RX_FILTER_LOCK(x)		spin_lock_bh(&(x)->lock);
#define IONIC_RX_FILTER_UNLOCK(x)	spin_unlock_bh(&(x)->lock);

#endif

struct ionic_admin_ctx;

void ionic_rx_filter_free(struct lif *lif, struct rx_filter *f);
int ionic_rx_filter_del(struct lif *lif, struct rx_filter *f);
int ionic_rx_filters_init(struct lif *lif);
void ionic_rx_filters_deinit(struct lif *lif);
int ionic_rx_filter_save(struct lif *lif, u32 flow_id, u16 rxq_index,
			 u32 hash, struct ionic_admin_ctx *ctx);
struct rx_filter *ionic_rx_filter_by_vlan(struct lif *lif, u16 vid);
struct rx_filter *ionic_rx_filter_by_addr(struct lif *lif, const u8 *addr);

#endif /* _IONIC_RX_FILTER_H_ */
