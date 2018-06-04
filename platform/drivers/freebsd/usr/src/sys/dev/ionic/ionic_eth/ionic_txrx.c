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

#include <linux/kernel.h>
#include <linux/if_vlan.h>
#include <linux/dma-mapping.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"

static void ionic_rx_clean(struct queue *q, struct desc_info *desc_info,
			   struct cq_info *cq_info, void *cb_arg);

static void ionic_rx_recycle(struct queue *q, struct desc_info *desc_info,
			     struct sk_buff *skb)
{
	struct rxq_desc *old = desc_info->desc;
	struct rxq_desc *new = q->head->desc;

	//dev_trace(q->lif->ionic->dev, "\n");

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
	//struct qcq *qcq = q_to_qcq(q);
	struct rx_stats *stats = q_to_rx_stats(q);
	dma_addr_t dma_addr;

	//dev_trace(q->lif->ionic->dev, "\n");

	if (comp->status) {
		// TODO record errors
		printk(KERN_ERR "RX Status %d\n", comp->status);
		ionic_rx_recycle(q, desc_info, skb);
		return;
	}

#ifdef HAPS
	if (comp->len > ETHER_MAX_FRAME(netdev, ETHERTYPE_VLAN, 1)) {
		printk(KERN_ERR "RX PKT TOO LARGE!  comp->len %d\n", comp->len);
		ionic_rx_recycle(q, desc_info, skb);
		return;
	}
#endif

	stats->pkts++;
	stats->bytes += comp->len;

	// TODO add copybreak to avoid allocating a new skb for small receive

	dma_addr = (dma_addr_t)desc->addr;

	dma_addr -= ETHER_ALIGN;

	dma_unmap_single(dev, dma_addr, desc->len, DMA_FROM_DEVICE);

	//prefetch(skb->data - NET_IP_ALIGN);
	skb->m_pkthdr.rcvif = netdev;
	skb->m_pkthdr.len = comp->len;
	skb->m_len = comp->len;
	//skb->m_pkthdr.csum_flags =
	//	CSUM_IP_CHECKED | CSUM_IP_VALID |
	//	CSUM_DATA_VALID | CSUM_PSEUDO_HDR;
	//skb->m_pkthdr.csum_data = htons(0xffff);

	dev_trace(q->lif->ionic->dev, "len %u\n", skb->m_len);
	dev_trace(q->lif->ionic->dev,
		  "data %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
		  skb->m_data[0], skb->m_data[1],
		  skb->m_data[2], skb->m_data[3],
		  skb->m_data[4], skb->m_data[5],
		  skb->m_data[6], skb->m_data[7]);
	dev_trace(q->lif->ionic->dev,
		  "data end %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
		  skb->m_data[skb->m_len - 8], skb->m_data[skb->m_len - 7],
		  skb->m_data[skb->m_len - 6], skb->m_data[skb->m_len - 5],
		  skb->m_data[skb->m_len - 4], skb->m_data[skb->m_len - 3],
		  skb->m_data[skb->m_len - 2], skb->m_data[skb->m_len - 1]);

#if 0
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
#endif

	netdev->if_input(netdev, skb);
}

static bool ionic_rx_service(struct cq *cq, struct cq_info *cq_info,
			     void *cb_arg)
{
	struct rxq_comp *comp = cq_info->cq_desc;

	dev_trace(cq->bound_q->lif->ionic->dev, "\n");

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
	//struct net_device *netdev = lif->netdev;
	struct device *dev = lif->ionic->dev;
	struct rx_stats *stats = q_to_rx_stats(q);
	struct sk_buff *skb;

	//dev_trace(lif->ionic->dev, "\n");

	skb = m_get2(len, M_NOWAIT, MT_DATA, M_PKTHDR);
	if (!skb) {
		stats->alloc_err++;
		return NULL;
	}

	*dma_addr = dma_map_single(dev, skb->m_data, len, DMA_FROM_DEVICE);
	if (dma_mapping_error(dev, *dma_addr)) {
		m_free(skb);
		stats->dma_map_err++;
		return NULL;
	}

	skb->m_data += ETHER_ALIGN;
	*dma_addr += ETHER_ALIGN;

	return skb;
}

static void ionic_rx_skb_free(struct queue *q, struct sk_buff *skb,
			      unsigned int len, dma_addr_t dma_addr)
{
	struct device *dev = q->lif->ionic->dev;

	//dev_trace(q->lif->ionic->dev, "\n");

	dma_addr -= ETHER_ALIGN;

	//printk(KERN_ERR "%s ionic_rx_skb_free len %d\n", q->name, len);
	dma_unmap_single(dev, dma_addr, len, DMA_FROM_DEVICE);
	m_free(skb);
}

#define RX_RING_DOORBELL_STRIDE		((1 << 2) - 1)

void ionic_rx_fill(struct queue *q)
{
	struct net_device *netdev = q->lif->netdev;
	struct rxq_desc *desc;
	struct sk_buff *skb;
	unsigned int len = ETHER_MAX_FRAME(netdev, ETHERTYPE_VLAN, 1);
	unsigned int i;
	dma_addr_t dma_addr;
	bool ring_doorbell;

	dev_trace(q->lif->ionic->dev, "\n");

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
	unsigned int len = ETHER_MAX_FRAME(netdev, ETHERTYPE_VLAN, 1);
	dma_addr_t dma_addr;

	dev_trace(q->lif->ionic->dev, "\n");

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

	dev_trace(q->lif->ionic->dev, "\n");

	while (cur != q->head) {
		desc = cur->desc;
		ionic_rx_skb_free(q, cur->cb_arg, desc->len, desc->addr);
		cur = cur->next;
	}
}

void ionic_rx_flush(struct cq *cq)
{
	unsigned int work_done;

	dev_trace(cq->bound_q->lif->ionic->dev, "\n");

	work_done = ionic_cq_service(cq, -1, ionic_rx_service, NULL);

	if (work_done > 0)
		ionic_intr_return_credits(cq->bound_intr, work_done, 0, true);
}

void ionic_rx_napi(struct napi_struct *napi)
{
	struct cq *cq = napi_to_cq(napi);
	unsigned int work_done;
	int budget = NAPI_POLL_WEIGHT;

	dev_trace(cq->bound_q->lif->ionic->dev, "\n");

	work_done = ionic_napi(napi, budget, ionic_rx_service, NULL);

	ionic_rx_fill(cq->bound_q);

	if (work_done == budget)
		napi_schedule(napi);
}

static dma_addr_t ionic_tx_map_single(struct queue *q, void *data, size_t len)
{
	struct device *dev = q->lif->ionic->dev;
	struct tx_stats *stats = q_to_tx_stats(q);
	dma_addr_t dma_addr;

	dma_addr = dma_map_single(dev, data, len, DMA_TO_DEVICE);
	if (dma_mapping_error(dev, dma_addr)) {
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

static void ionic_tx_clean(struct queue *q, struct desc_info *desc_info,
			   struct cq_info *cq_info, void *cb_arg)
{
	struct device *dev = q->lif->ionic->dev;
	struct tx_stats *stats = q_to_tx_stats(q);
	struct sk_buff *skb = cb_arg;

	ionic_tx_clean_sop(dev, desc_info->desc);
	m_free(skb);

	stats->clean++;
}

static void ionic_tx_clean_wake(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	ionic_tx_clean(q, desc_info, cq_info, cb_arg);
}

static bool ionic_tx_service(struct cq *cq, struct cq_info *cq_info,
			     void *cb_arg)
{
	struct txq_comp *comp = cq_info->cq_desc;

	dev_trace(cq->bound_q->lif->ionic->dev, "\n");

	//printk(KERN_ERR "ionic_tx_service comp->color %d cq->done_color %d\n", comp->color, cq->done_color);
	if (comp->color != cq->done_color)
		return false;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return true;
}

void ionic_tx_napi(struct napi_struct *napi)
{
	int budget = NAPI_POLL_WEIGHT;
	int work_done;

	work_done = ionic_napi(napi, budget, ionic_tx_service, NULL);

	if (work_done == budget)
		napi_schedule(napi);
}

static int ionic_tx_calc_no_csum(struct queue *q, struct sk_buff *skb)
{
	struct txq_desc *desc = q->head->desc;
	struct tx_stats *stats = q_to_tx_stats(q);
	dma_addr_t addr;

	addr = ionic_tx_map_single(q, skb->m_data, skb->m_len);
	if (!addr)
		return -ENOMEM;

	desc->opcode = TXQ_DESC_OPCODE_CALC_NO_CSUM;
	desc->num_sg_elems = 0;
	desc->len = skb->m_len;
	desc->addr = addr;
	desc->vlan_tci = 0;
	desc->hdr_len = 0;
	desc->csum_offset = 0;
	desc->V = 0;
	desc->C = 1;
	desc->O = 0;

	stats->no_csum++;

	return 0;
}

static int ionic_tx_skb_sop(struct queue *q, struct sk_buff
			    *skb)
{
	return ionic_tx_calc_no_csum(q, skb);
}

static bool ionic_tx_fit_check(struct queue *q, struct sk_buff **skb)
{
	struct sk_buff *sk_defrag;

	sk_defrag = m_defrag(*skb, M_NOWAIT);
	if (!sk_defrag)
		return false;

	*skb = sk_defrag;
	return true;
}

int ionic_start_xmit(struct net_device *netdev, struct sk_buff *skb)
{
	struct lif *lif = netdev_priv(netdev);
	struct queue *q = lif_to_txq(lif, 0);
	struct tx_stats *stats = q_to_tx_stats(q);
	bool ring_db = 1;
	desc_cb clean_cb = ionic_tx_clean;
	int err;

	netdev_err(netdev, "start xmit\n");

	if (!ionic_tx_fit_check(q, &skb))
		goto err_out_drop;

	if (!ionic_q_has_space(q, 1)) {
		stats->stop++;
		/* This is a hard error, log it */
		netdev_err(netdev, "BUG! Tx ring full when queue awake!\n");
		return ENOBUFS;
	}

	err = ionic_tx_skb_sop(q, skb);
	if (err)
		goto err_out_drop;

	stats->pkts++;
	stats->bytes += skb->m_len;

	if (!ionic_q_has_space(q, 2)) {
		ring_db = true;
		clean_cb = ionic_tx_clean_wake;
		stats->stop++;
	}

	ionic_q_post(q, ring_db, clean_cb, skb);

	return 0;

err_out_drop:
	stats->drop++;
	m_free(skb);
	return 0;
}
