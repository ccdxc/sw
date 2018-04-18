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

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_vlan.h>
#include <net/ip6_checksum.h>

#include "ionic.h"
#include "ionic_lif.h"

static void ionic_rx_clean(struct queue *q, struct desc_info *desc_info,
			   struct cq_info *cq_info, void *cb_arg);

static void ionic_rx_recycle(struct queue *q, struct desc_info *desc_info,
			     struct sk_buff *skb)
{
	struct rxq_desc *old = desc_info->desc;
	struct rxq_desc *new = q->head->desc;

	new->addr = old->addr;
	new->len = old->len;

	ionic_q_post(q, true, ionic_rx_clean, skb);
}

static void ionic_rx_clean(struct queue *q, struct desc_info *desc_info,
			   struct cq_info *cq_info, void *cb_arg)
{
	struct net_device *netdev = q->lif->netdev;
	struct device *dev = q->lif->ionic->dev;
	struct rxq_desc *desc = desc_info->desc;
	struct rxq_comp *comp = cq_info->cq_desc;
	struct sk_buff *skb = cb_arg;
	struct qcq *qcq = q_to_qcq(q);
	struct rx_stats *stats = q_to_rx_stats(q);
	dma_addr_t dma_addr;
#ifdef HAPS
	__sum16 csum;
#endif

	if (comp->status) {
		// TODO record errors
		ionic_rx_recycle(q, desc_info, skb);
		return;
	}

#ifdef HAPS
	if (comp->len > netdev->mtu + VLAN_ETH_HLEN) {
		printk(KERN_ERR "RX PKT TOO LARGE!  comp->len %d\n", comp->len);
		ionic_rx_recycle(q, desc_info, skb);
		return;
	}
#endif

	stats->pkts++;
	stats->bytes += comp->len;

	// TODO add copybreak to avoid allocating a new skb for small receive

	dma_addr = (dma_addr_t)desc->addr;
	dma_unmap_single(dev, dma_addr, desc->len, DMA_FROM_DEVICE);

	//prefetch(skb->data - NET_IP_ALIGN);
	skb_put(skb, comp->len);
	skb->protocol = eth_type_trans(skb, netdev);
#ifdef HAPS
	csum = ip_compute_csum(skb->data, skb->len);
#endif
	skb_record_rx_queue(skb, q->index);

	if (netdev->features & NETIF_F_RXHASH) {
		switch (comp->rss_type) {
		case RXQ_COMP_RSS_TYPE_IPV4:
		case RXQ_COMP_RSS_TYPE_IPV6:
		case RXQ_COMP_RSS_TYPE_IPV6_EX:
			skb_set_hash(skb, comp->rss_hash, PKT_HASH_TYPE_L3);
			break;
		case RXQ_COMP_RSS_TYPE_IPV4_TCP:
		case RXQ_COMP_RSS_TYPE_IPV6_TCP:
		case RXQ_COMP_RSS_TYPE_IPV6_TCP_EX:
		case RXQ_COMP_RSS_TYPE_IPV4_UDP:
		case RXQ_COMP_RSS_TYPE_IPV6_UDP:
		case RXQ_COMP_RSS_TYPE_IPV6_UDP_EX:
			skb_set_hash(skb, comp->rss_hash, PKT_HASH_TYPE_L4);
			break;
		}
	}

	if (netdev->features & NETIF_F_RXCSUM) {
		skb->ip_summed = CHECKSUM_COMPLETE;
		skb->csum = comp->csum;
#ifdef HAPS
		if (skb->csum != (u16)~csum)
			printk(KERN_ERR "Rx CSUM incorrect.  Want 0x%04x got 0x%04x, protocol 0x%04x\n", (u16)~csum, skb->csum, htons(skb->protocol));
#endif
	}

	if (netdev->features & NETIF_F_HW_VLAN_CTAG_RX) {
		if (comp->V)
			__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q),
					       comp->vlan_tci);
	}

	napi_gro_receive(&qcq->napi, skb);
}

static bool ionic_rx_service(struct cq *cq, struct cq_info *cq_info,
			     void *cb_arg)
{
	struct rxq_comp *comp = cq_info->cq_desc;

	//printk(KERN_ERR "ionic_rx_service comp->color %d cq->done_color %d\n", comp->color, cq->done_color);
	if (comp->color != cq->done_color)
		return false;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return true;
}

static struct sk_buff *ionic_rx_skb_alloc(struct queue *q, unsigned int len,
					  dma_addr_t *dma_addr)
{
	struct lif *lif = q->lif;
	struct net_device *netdev = lif->netdev;
	struct device *dev = lif->ionic->dev;
	struct rx_stats *stats = q_to_rx_stats(q);
	struct sk_buff *skb;

	//printk(KERN_ERR "%s ionic_rx_skb_alloc len %d\n", q->name, len);
	skb = netdev_alloc_skb_ip_align(netdev, len);
	if (!skb) {
		net_warn_ratelimited("%s: SKB alloc failed on %s!\n",
				     netdev->name, q->name);
		stats->alloc_err++;
		return NULL;
	}

	*dma_addr = dma_map_single(dev, skb->data, len, DMA_FROM_DEVICE);
	if (dma_mapping_error(dev, *dma_addr)) {
		dev_kfree_skb(skb);
		net_warn_ratelimited("%s: DMA single map failed on %s!\n",
				     netdev->name, q->name);
		stats->dma_map_err++;
		return NULL;
	}

	return skb;
}

static void ionic_rx_skb_free(struct queue *q, struct sk_buff *skb,
			      unsigned int len, dma_addr_t dma_addr)
{
	struct device *dev = q->lif->ionic->dev;

	//printk(KERN_ERR "%s ionic_rx_skb_free len %d\n", q->name, len);
	dma_unmap_single(dev, dma_addr, len, DMA_FROM_DEVICE);
	dev_kfree_skb(skb);
}

#define RX_RING_DOORBELL_STRIDE		((1 << 2) - 1)

void ionic_rx_fill(struct queue *q)
{
	struct net_device *netdev = q->lif->netdev;
	struct rxq_desc *desc;
	struct sk_buff *skb;
	unsigned int len = netdev->mtu + VLAN_ETH_HLEN;
	unsigned int i;
	dma_addr_t dma_addr;
	bool ring_doorbell;

	for (i = ionic_q_space_avail(q); i; i--) {

		skb = ionic_rx_skb_alloc(q, len, &dma_addr);
		if (!skb)
			return;

		desc = q->head->desc;
		desc->addr = dma_addr;
		desc->len = len;
		desc->opcode = RXQ_DESC_OPCODE_SIMPLE;

		ring_doorbell = ((q->head->index + 1) &
				RX_RING_DOORBELL_STRIDE) == 0;

		ionic_q_post(q, ring_doorbell, ionic_rx_clean, skb);
	}
}

void ionic_rx_refill(struct queue *q)
{
	struct net_device *netdev = q->lif->netdev;
	struct desc_info *cur = q->tail;
	struct rxq_desc *desc;
	struct sk_buff *skb;
	unsigned int len = netdev->mtu + VLAN_ETH_HLEN;
	dma_addr_t dma_addr;

	while (cur != q->head) {

		desc = cur->desc;

		ionic_rx_skb_free(q, cur->cb_arg, desc->len, desc->addr);
		skb = ionic_rx_skb_alloc(q, len, &dma_addr);
		if (!skb)
			return;

		cur->cb_arg = skb;
		desc->addr = dma_addr;
		desc->len = len;

		cur = cur->next;
	}
}

void ionic_rx_empty(struct queue *q)
{
	struct desc_info *cur = q->tail;
	struct rxq_desc *desc;

	while (cur != q->head) {
		desc = cur->desc;
		ionic_rx_skb_free(q, cur->cb_arg, desc->len, desc->addr);
		cur = cur->next;
	}
}

void ionic_rx_flush(struct cq *cq)
{
	unsigned int work_done;

	work_done = ionic_cq_service(cq, -1, ionic_rx_service, NULL);

	if (work_done > 0)
		ionic_intr_return_credits(cq->bound_intr, work_done, 0, true);
}

int ionic_rx_napi(struct napi_struct *napi, int budget)
{
	struct cq *cq = napi_to_cq(napi);
	unsigned int work_done;

	work_done = ionic_napi(napi, budget, ionic_rx_service, NULL);

	ionic_rx_fill(cq->bound_q);

	return work_done;
}

static dma_addr_t ionic_tx_map_single(struct queue *q, void *data, size_t len)
{
	struct device *dev = q->lif->ionic->dev;
	struct tx_stats *stats = q_to_tx_stats(q);
	dma_addr_t dma_addr;

	dma_addr = dma_map_single(dev, data, len, DMA_TO_DEVICE);
	if (dma_mapping_error(dev, dma_addr)) {
		net_warn_ratelimited("%s: DMA single map failed on %s!\n",
				     q->lif->netdev->name, q->name);
		stats->dma_map_err++;
		return 0;
	}
	return dma_addr;
}

static dma_addr_t ionic_tx_map_frag(struct queue *q, const skb_frag_t *frag,
				    size_t offset, size_t len)
{
	struct device *dev = q->lif->ionic->dev;
	struct tx_stats *stats = q_to_tx_stats(q);
	dma_addr_t dma_addr;

	dma_addr = skb_frag_dma_map(dev, frag, offset, len, DMA_TO_DEVICE);
	if (dma_mapping_error(dev, dma_addr)) {
		net_warn_ratelimited("%s: DMA frag map failed on %s!\n",
				     q->lif->netdev->name, q->name);
		stats->dma_map_err++;
		return 0;
	}
	return dma_addr;
}

static void ionic_tx_clean_sop(struct device *dev, struct txq_desc *desc)
{
	dma_addr_t dma_addr;

	switch (desc->opcode) {
	case TXQ_DESC_OPCODE_CALC_NO_CSUM:
	case TXQ_DESC_OPCODE_CALC_CSUM:
	case TXQ_DESC_OPCODE_TSO:
		dma_addr = (dma_addr_t)desc->addr;
		dma_unmap_single(dev, dma_addr, desc->len, DMA_TO_DEVICE);
		break;
	}
}

static void ionic_tx_clean_frags(struct device *dev, unsigned int elems,
				 struct txq_sg_desc *sg_desc)
{
	struct txq_sg_elem *elem = sg_desc->elems;
	dma_addr_t dma_addr;

	for (; elems; elems--, elem++) {
		dma_addr = (dma_addr_t)elem->addr;
		dma_unmap_page(dev, dma_addr, elem->len, DMA_TO_DEVICE);
	}
}

static void ionic_tx_clean(struct queue *q, struct desc_info *desc_info,
			   struct cq_info *cq_info, void *cb_arg)
{
	struct device *dev = q->lif->ionic->dev;
	struct tx_stats *stats = q_to_tx_stats(q);
	struct sk_buff *skb = cb_arg;

	ionic_tx_clean_sop(dev, desc_info->desc);
	ionic_tx_clean_frags(dev, skb_shinfo(skb)->nr_frags,
			     desc_info->sg_desc);
	dev_kfree_skb_any(skb);

	stats->clean++;
}

static void ionic_tx_clean_wake(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct sk_buff *skb = cb_arg;
	u16 queue_index = skb_get_queue_mapping(skb);

	ionic_tx_clean(q, desc_info, cq_info, cb_arg);
	netif_wake_subqueue(q->lif->netdev, queue_index);
}

static bool ionic_tx_service(struct cq *cq, struct cq_info *cq_info,
			     void *cb_arg)
{
	struct txq_comp *comp = cq_info->cq_desc;

	//printk(KERN_ERR "ionic_tx_service comp->color %d cq->done_color %d\n", comp->color, cq->done_color);
	if (comp->color != cq->done_color)
		return false;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return true;
}

int ionic_tx_napi(struct napi_struct *napi, int budget)
{
	return ionic_napi(napi, budget, ionic_tx_service, NULL);
}

static void ionic_tx_tcp_inner_pseudo_csum(struct sk_buff *skb)
{
	skb_cow_head(skb, 0); // TODO is this necessary before modifying hdrs?

	if (skb->protocol == cpu_to_be16(ETH_P_IP)) {
		inner_ip_hdr(skb)->check = 0;
		inner_tcp_hdr(skb)->check =
			~csum_tcpudp_magic(inner_ip_hdr(skb)->saddr,
					   inner_ip_hdr(skb)->daddr,
					   0, IPPROTO_TCP, 0);
	} else if (skb->protocol == cpu_to_be16(ETH_P_IPV6)) {
		inner_tcp_hdr(skb)->check =
			~csum_ipv6_magic(&inner_ipv6_hdr(skb)->saddr,
					 &inner_ipv6_hdr(skb)->daddr,
					 0, IPPROTO_TCP, 0);
	}

}

static void ionic_tx_tcp_pseudo_csum(struct sk_buff *skb)
{
	skb_cow_head(skb, 0); // TODO is this necessary before modifying hdrs?

	if (skb->protocol == cpu_to_be16(ETH_P_IP)) {
		ip_hdr(skb)->check = 0;
		tcp_hdr(skb)->check =
			~csum_tcpudp_magic(ip_hdr(skb)->saddr,
					   ip_hdr(skb)->daddr,
					   0, IPPROTO_TCP, 0);
	} else if (skb->protocol == cpu_to_be16(ETH_P_IPV6)) {
		tcp_hdr(skb)->check =
			~csum_ipv6_magic(&ipv6_hdr(skb)->saddr,
					 &ipv6_hdr(skb)->daddr,
					 0, IPPROTO_TCP, 0);
	}
}

static int ionic_tx_tso(struct queue *q, struct sk_buff *skb)
{
	struct txq_desc *desc = q->head->desc;
	struct tx_stats *stats = q_to_tx_stats(q);
	bool encap = skb->encapsulation;
	bool outer_csum_offload =
		(skb_shinfo(skb)->gso_type & SKB_GSO_GRE_CSUM) ||
		(skb_shinfo(skb)->gso_type & SKB_GSO_UDP_TUNNEL_CSUM);
	dma_addr_t addr;

	addr = ionic_tx_map_single(q, skb->data, skb_headlen(skb));
	if (!addr)
		return -ENOMEM;

	/* Preload inner-most TCP csum field with IP pseudo hdr
	 * calculated with IP length set to zero.  HW will later
	 * add in length to each TCP segment resulting from the TSO.
	 */

	if (encap)
		ionic_tx_tcp_inner_pseudo_csum(skb);
	else
		ionic_tx_tcp_pseudo_csum(skb);

	desc->opcode = TXQ_DESC_OPCODE_TSO;
	desc->num_sg_elems = skb_shinfo(skb)->nr_frags;
	desc->len = skb_headlen(skb);
	desc->addr = addr;
	//desc->addr = (addr & ~0xffffffff) | (addr & 0xffffffff);
	desc->vlan_tci = skb_vlan_tag_get(skb);
	if (encap)
		desc->hdr_len = skb_inner_transport_header(skb) - skb->data +
			inner_tcp_hdrlen(skb);
	else
		desc->hdr_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
	desc->mss = skb_shinfo(skb)->gso_size;
	desc->V = !!skb_vlan_tag_present(skb);
	desc->C = 1;
	desc->O = outer_csum_offload;

	stats->tso++;

	return 0;
}

static int ionic_tx_calc_csum(struct queue *q, struct sk_buff *skb)
{
	struct txq_desc *desc = q->head->desc;
	struct tx_stats *stats = q_to_tx_stats(q);
	bool encap = skb->encapsulation;
	dma_addr_t addr;

	addr = ionic_tx_map_single(q, skb->data, skb_headlen(skb));
	if (!addr)
		return -ENOMEM;

	desc->opcode = TXQ_DESC_OPCODE_CALC_CSUM;
	desc->num_sg_elems = skb_shinfo(skb)->nr_frags;
	desc->len = skb_headlen(skb);
	desc->addr = addr;
	desc->vlan_tci = skb_vlan_tag_get(skb);
	desc->hdr_len = skb_checksum_start_offset(skb);
	desc->csum_offset = skb->csum_offset;
	desc->V = !!skb_vlan_tag_present(skb);
	desc->C = 1;
	desc->O = encap;

	if (skb->csum_not_inet)
		stats->crc32_csum++;
	else
		stats->csum++;

	return 0;
}

static int ionic_tx_calc_no_csum(struct queue *q, struct sk_buff *skb)
{
	struct txq_desc *desc = q->head->desc;
	struct tx_stats *stats = q_to_tx_stats(q);
	dma_addr_t addr;

	addr = ionic_tx_map_single(q, skb->data, skb_headlen(skb));
	if (!addr)
		return -ENOMEM;

	desc->opcode = TXQ_DESC_OPCODE_CALC_NO_CSUM;
	desc->num_sg_elems = skb_shinfo(skb)->nr_frags;
	desc->len = skb_headlen(skb);
	desc->addr = addr;
	desc->vlan_tci = skb_vlan_tag_get(skb);
	desc->hdr_len = 0;
	desc->csum_offset = 0;
	desc->V = !!skb_vlan_tag_present(skb);
	desc->C = 1;
	desc->O = 0;

	stats->no_csum++;

	return 0;
}

static int ionic_tx_skb_sop(struct queue *q, struct sk_buff *skb)
{
	if (skb_is_gso(skb))
		return ionic_tx_tso(q, skb);
	else if (skb->ip_summed == CHECKSUM_PARTIAL)
		return ionic_tx_calc_csum(q, skb);

	return ionic_tx_calc_no_csum(q, skb);
}

static int ionic_tx_skb_frags(struct queue *q, struct sk_buff *skb)
{
	struct tx_stats *stats = q_to_tx_stats(q);
	unsigned int len_left = skb->len - skb_headlen(skb);
	struct txq_sg_desc *sg_desc = q->head->sg_desc;
	struct txq_sg_elem *elem = sg_desc->elems;
	skb_frag_t *frag;
	dma_addr_t dma_addr;

	BUG_ON(skb_shinfo(skb)->nr_frags > IONIC_TX_MAX_SG_ELEMS);

	for (frag = skb_shinfo(skb)->frags; len_left; frag++, elem++) {
		elem->len = skb_frag_size(frag);
		dma_addr = ionic_tx_map_frag(q, frag, 0, elem->len);
		if (!dma_addr)
			return -ENOMEM;
		elem->addr = dma_addr;
		len_left -= elem->len;
		stats->frags++;
	}

	return 0;
}

static bool ionic_tx_fit_check(struct queue *q, struct sk_buff *skb)
{
	struct tx_stats *stats = q_to_tx_stats(q);
	int err;

	if (skb_shinfo(skb)->nr_frags > IONIC_TX_MAX_SG_ELEMS) {
		err = skb_linearize(skb);
		if (err)
			return false;
		stats->linearize++;
	}

	return true;
}

netdev_tx_t ionic_start_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	u16 queue_index = skb_get_queue_mapping(skb);
	struct lif *lif = netdev_priv(netdev);
	struct queue *q = lif_to_txq(lif, queue_index);
	struct tx_stats *stats = q_to_tx_stats(q);
	bool ring_db = !skb->xmit_more;
	desc_cb clean_cb = ionic_tx_clean;
	int err;

	if (!ionic_tx_fit_check(q, skb))
		goto err_out_drop;

	if (!ionic_q_has_space(q, 1)) {
		netif_stop_subqueue(netdev, queue_index);
		stats->stop++;
		/* This is a hard error, log it */
		netdev_err(netdev, "BUG! Tx ring full when queue awake!\n");
		return NETDEV_TX_BUSY;
	}

	err = ionic_tx_skb_frags(q, skb);
	if (err)
		goto err_out_drop;

	err = ionic_tx_skb_sop(q, skb);
	if (err)
		goto err_out_drop;

	skb_tx_timestamp(skb);

	stats->pkts++;
	stats->bytes += skb->len;

	if (!ionic_q_has_space(q, 2)) {
		ring_db = true;
		clean_cb = ionic_tx_clean_wake;
		netif_stop_subqueue(netdev, queue_index);
		stats->stop++;
	}

	ionic_q_post(q, ring_db, clean_cb, skb);

	return NETDEV_TX_OK;

err_out_drop:
	stats->drop++;
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}
