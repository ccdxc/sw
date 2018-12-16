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

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_api.h"
#include "ionic_rx_filter.h"

void ionic_rx_filter_free(struct lif *lif, struct rx_filter *f)
{
	hlist_del(&f->by_id);
	hlist_del(&f->by_hash);

	free(f, M_IONIC);
}

int ionic_rx_filter_del(struct lif *lif, struct rx_filter *f)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_del = {
			.opcode = CMD_OPCODE_RX_FILTER_DEL,
			.filter_id = f->filter_id,
		},
	};

	return ionic_api_adminq_post(lif, &ctx);
}

int ionic_rx_filters_init(struct lif *lif)
{
	unsigned int i;

	IONIC_RX_FILTER_INIT(&lif->rx_filters);

	for (i = 0; i < RX_FILTER_HLISTS; i++) {
		INIT_HLIST_HEAD(&lif->rx_filters.by_hash[i]);
		INIT_HLIST_HEAD(&lif->rx_filters.by_id[i]);
	}

	return 0;
}

void ionic_rx_filters_deinit(struct lif *lif)
{
	struct hlist_head *head;
	struct hlist_node *tmp;
	struct rx_filter *f;
	unsigned int i;

	for (i = 0; i < RX_FILTER_HLISTS; i++) {
		head = &lif->rx_filters.by_id[i];
		hlist_for_each_entry_safe(f, tmp, head, by_id)
			ionic_rx_filter_free(lif, f);
	}
}

int ionic_rx_filter_save(struct lif *lif, u32 flow_id, u16 rxq_index,
			 u32 hash, struct ionic_admin_ctx *ctx)
{
	struct rx_filter *f = malloc(sizeof(*f), M_IONIC, M_NOWAIT | M_ZERO);
	struct hlist_head *head;
	unsigned int key;

	if (!f)
		return ENOMEM;

	f->flow_id = flow_id;
	f->filter_id = ctx->comp.rx_filter_add.filter_id;
	f->rxq_index = rxq_index;
	f->visited = false;
	memcpy(&f->cmd, &ctx->cmd, sizeof(f->cmd));

	INIT_HLIST_NODE(&f->by_hash);
	INIT_HLIST_NODE(&f->by_id);

	switch (f->cmd.match) {
	case RX_FILTER_MATCH_VLAN:
		key = f->cmd.vlan.vlan & RX_FILTER_HLISTS_MASK;
		break;
	case RX_FILTER_MATCH_MAC:
		key = *(u32 *)f->cmd.mac.addr & RX_FILTER_HLISTS_MASK;
		break;
	case RX_FILTER_MATCH_MAC_VLAN:
		key = f->cmd.mac_vlan.vlan & RX_FILTER_HLISTS_MASK;
		break;
	default:
		return ENOTSUPP;
	}

//	IONIC_RX_FILTER_LOCK(&lif->rx_filters);

	head = &lif->rx_filters.by_hash[key];
	hlist_add_head(&f->by_hash, head);

	key = f->filter_id & RX_FILTER_HLISTS_MASK;
	head = &lif->rx_filters.by_id[key];
	hlist_add_head(&f->by_id, head);

//	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);

	return 0;
}

struct rx_filter *ionic_rx_filter_by_vlan(struct lif *lif, u16 vid)
{
	unsigned int key = vid & RX_FILTER_HLISTS_MASK;
	struct hlist_head *head = &lif->rx_filters.by_hash[key];
	struct rx_filter *f;

	hlist_for_each_entry(f, head, by_hash) {
		if (f->cmd.match != RX_FILTER_MATCH_VLAN)
			continue;
		if (f->cmd.vlan.vlan == vid)
			return f;
	}

	return NULL;
}

struct rx_filter *ionic_rx_filter_by_addr(struct lif *lif, const u8 *addr)
{
	unsigned int key = *(const u32 *)addr & RX_FILTER_HLISTS_MASK;
	struct hlist_head *head = &lif->rx_filters.by_hash[key];
	struct rx_filter *f;

	hlist_for_each_entry(f, head, by_hash) {
		if (f->cmd.match != RX_FILTER_MATCH_MAC)
			continue;
		if (memcmp(addr, f->cmd.mac.addr, ETH_ALEN) == 0)
			return f;
	}

	return NULL;
}
