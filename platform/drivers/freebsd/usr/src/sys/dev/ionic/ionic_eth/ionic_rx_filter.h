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
	struct sx sx;
#else
	spinlock_t lock;
#endif
	struct hlist_head by_hash[RX_FILTER_HLISTS];	/* by skb hash */
	struct hlist_head by_id[RX_FILTER_HLISTS];		/* by filter_id */
};

#ifdef __FreeBSD__
#define IONIC_RX_FILTER_INIT(x) 					\
	sx_init(&(x)->sx, "rx filter")//mtx_init(&(x)->mtx, "rx filter", NULL, MTX_DEF)
#define IONIC_RX_FILTER_DESTROY(x)	sx_destroy(&(x)->sx)
#define IONIC_RX_FILTER_LOCK(x)		sx_xlock(&(x)->sx)
#define IONIC_RX_FILTER_UNLOCK(x)	sx_xunlock(&(x)->sx)
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
