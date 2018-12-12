// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_api.h"
#include "ionic_rx_filter.h"

void ionic_rx_filter_free(struct lif *lif, struct rx_filter *f)
{
	struct device *dev = lif->ionic->dev;

	hlist_del(&f->by_id);
	hlist_del(&f->by_hash);
	devm_kfree(dev, f);
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

	spin_lock_init(&lif->rx_filters.lock);

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
	struct device *dev = lif->ionic->dev;
	struct rx_filter *f = devm_kzalloc(dev, sizeof(*f), GFP_KERNEL);
	struct hlist_head *head;
	unsigned int key;

	if (!f)
		return -ENOMEM;

	f->flow_id = flow_id;
	f->filter_id = ctx->comp.rx_filter_add.filter_id;
	f->rxq_index = rxq_index;
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
		return -ENOTSUPP;
	}

	spin_lock_bh(&lif->rx_filters.lock);

	head = &lif->rx_filters.by_hash[key];
	hlist_add_head(&f->by_hash, head);

	key = f->filter_id & RX_FILTER_HLISTS_MASK;
	head = &lif->rx_filters.by_id[key];
	hlist_add_head(&f->by_id, head);

	spin_unlock_bh(&lif->rx_filters.lock);

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
	unsigned int key = *(u32 *)addr & RX_FILTER_HLISTS_MASK;
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
