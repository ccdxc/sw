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
#include <sys/mutex.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_api.h"
#include "ionic_rx_filter.h"

void
ionic_rx_filter_free(struct ionic_lif *lif, struct rx_filter *f)
{

	hlist_del(&f->by_id);
	hlist_del(&f->by_hash);

	free(f, M_IONIC);
}

int
ionic_rx_filters_init(struct ionic_lif *lif)
{
	int i;

	/*
	 * Init the filter lock only once, not in reinit process.
	 */
	if (!lif->rx_filters.init)
		IONIC_RX_FILTER_INIT(&lif->rx_filters);

	for (i = 0; i < RX_FILTER_HLISTS; i++) {
		INIT_HLIST_HEAD(&lif->rx_filters.by_hash[i]);
		INIT_HLIST_HEAD(&lif->rx_filters.by_id[i]);
	}

	lif->rx_filters.init = true;
	return (0);
}

void
ionic_rx_filters_deinit(struct ionic_lif *lif)
{
	struct hlist_head *head;
	struct hlist_node *tmp;
	struct rx_filter *f;
	int i;

	if (!lif->rx_filters.init)
		return;

	IONIC_RX_FILTER_LOCK(&lif->rx_filters);
	for (i = 0; i < RX_FILTER_HLISTS; i++) {
		head = &lif->rx_filters.by_id[i];
		hlist_for_each_entry_safe(f, tmp, head, by_id)
			ionic_rx_filter_free(lif, f);
	}
	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
}

int
ionic_rx_filter_save(struct ionic_lif *lif, uint32_t flow_id,
    uint16_t rxq_index, uint32_t hash, struct ionic_admin_ctx *ctx)
{
	struct rx_filter *f = malloc(sizeof(*f), M_IONIC, M_NOWAIT | M_ZERO);
	struct hlist_head *head;
	unsigned int key;

	if (!f)
		return (ENOMEM);

	IONIC_RX_FILTER_LOCK(&lif->rx_filters);
	f->flow_id = flow_id;
	f->filter_id = ctx->comp.rx_filter_add.filter_id;
	f->rxq_index = rxq_index;
	f->visited = false;
	memcpy(&f->cmd, &ctx->cmd, sizeof(f->cmd));

	INIT_HLIST_NODE(&f->by_hash);
	INIT_HLIST_NODE(&f->by_id);

	switch (f->cmd.match) {
	case IONIC_RX_FILTER_MATCH_VLAN:
		key = f->cmd.vlan.vlan & RX_FILTER_HLISTS_MASK;
		break;
	case IONIC_RX_FILTER_MATCH_MAC:
		key = *(uint32_t *)f->cmd.mac.addr & RX_FILTER_HLISTS_MASK;
		break;
	case IONIC_RX_FILTER_MATCH_MAC_VLAN:
		key = f->cmd.mac_vlan.vlan & RX_FILTER_HLISTS_MASK;
		break;
	default:
		free(f, M_IONIC);
		IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
		return (ENOTSUPP);
	}

	head = &lif->rx_filters.by_hash[key];
	hlist_add_head(&f->by_hash, head);

	key = f->filter_id & RX_FILTER_HLISTS_MASK;
	head = &lif->rx_filters.by_id[key];
	hlist_add_head(&f->by_id, head);
	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);

	return (0);
}

struct rx_filter *
ionic_rx_filter_by_vlan_addr(struct ionic_lif *lif, uint16_t vid,
    const uint8_t *addr)
{
	unsigned int key = vid & RX_FILTER_HLISTS_MASK;
	struct hlist_head *head = &lif->rx_filters.by_hash[key];
	struct rx_filter *f;

	hlist_for_each_entry(f, head, by_hash) {
		if (f->cmd.match != IONIC_RX_FILTER_MATCH_MAC_VLAN)
			continue;
		if (f->cmd.mac_vlan.vlan != vid)
			continue;
		if (memcmp(addr, f->cmd.mac_vlan.addr, ETH_ALEN))
			continue;
		return (f);
	}

	return (NULL);
}

struct rx_filter *
ionic_rx_filter_by_vlan(struct ionic_lif *lif, uint16_t vid)
{
	unsigned int key = vid & RX_FILTER_HLISTS_MASK;
	struct hlist_head *head = &lif->rx_filters.by_hash[key];
	struct rx_filter *f;

	hlist_for_each_entry(f, head, by_hash) {
		if (f->cmd.match != IONIC_RX_FILTER_MATCH_VLAN)
			continue;
		if (f->cmd.vlan.vlan != vid)
			continue;
		return (f);
	}

	return (NULL);
}

struct rx_filter *
ionic_rx_filter_by_addr(struct ionic_lif *lif, const uint8_t *addr)
{
	unsigned int key = *(const uint32_t *)addr & RX_FILTER_HLISTS_MASK;
	struct hlist_head *head = &lif->rx_filters.by_hash[key];
	struct rx_filter *f;

	hlist_for_each_entry(f, head, by_hash) {
		if (f->cmd.match != IONIC_RX_FILTER_MATCH_MAC)
			continue;
		if (memcmp(addr, f->cmd.mac.addr, ETH_ALEN))
			continue;
		return (f);
	}

	return (NULL);
}
