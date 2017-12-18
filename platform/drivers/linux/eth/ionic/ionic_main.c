/*
 * Copyright 2017 Pensando Systems, Inc.  All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/log2.h>
#include <linux/pci.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_vlan.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "ionic.h"
#include "ionic_dev.h"
#include "ionic_adminq.h"
#include "ionic_debugfs.h"

#define DRV_DESCRIPTION		"Pensando Ethernet NIC Driver"
#define DRV_VERSION		"0.1"

// TODO move PCI_VENDOR_ID_PENSANDO to include/linux/pci_ids.h
#define PCI_VENDOR_ID_PENSANDO			0x1dd8

#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF	0x1002
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF	0x1003
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT	0x1004

/* Supported devices */
static const struct pci_device_id ionic_id_table[] = {
	{ PCI_VDEVICE(PENSANDO, PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF) },
	{ PCI_VDEVICE(PENSANDO, PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF) },
	{ PCI_VDEVICE(PENSANDO, PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT) },
	{ 0, }	/* end of table */
};

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Scott Feldman <sfeldma@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DEVICE_TABLE(pci, ionic_id_table);

static int ntxq_descs = 16;
static int nrxq_descs = 16;
module_param(ntxq_descs, uint, 0);
module_param(nrxq_descs, uint, 0);
MODULE_PARM_DESC(ntxq_descs, "Descriptors per Tx queue, must be power of 2");
MODULE_PARM_DESC(nrxq_descs, "Descriptors per Rx queue, must be power of 2");

static int ionic_map_bars(struct ionic *ionic)
{
	struct pci_dev *pdev = ionic->pdev;
	struct device *dev = &pdev->dev;
	struct ionic_dev_bar *bars = ionic->bars;
	unsigned int i, j;

	ionic->num_bars = 0;
	for (i = 0, j = 0; i < IONIC_BARS_MAX; i++) {
		if (!(pci_resource_flags(pdev, i) & IORESOURCE_MEM))
			continue;
		bars[j].len = pci_resource_len(pdev, i);
		bars[j].vaddr = pci_iomap(pdev, i, bars[j].len);
		if (!bars[j].vaddr) {
			dev_err(dev, "Cannot memory-map BAR %d, aborting\n", j);
			return -ENODEV;
		}
		bars[j].bus_addr = pci_resource_start(pdev, i);
		ionic->num_bars++;
		j++;
	}

	return ionic_debugfs_add_bars(ionic);
}

static void ionic_unmap_bars(struct ionic *ionic)
{
	struct ionic_dev_bar *bars = ionic->bars;
	unsigned int i;

	for (i = 0; i < IONIC_BARS_MAX; i++)
		if (bars[i].vaddr)
			iounmap(bars[i].vaddr);
}

static int ionic_set_dma_mask(struct pci_dev *pdev, bool *using_dac)
{
	struct device *dev = &pdev->dev;
	int err;

	/* Query PCI controller on system for DMA addressing
	 * limitation for the device.  Try 64-bit first, and
	 * fail to 32-bit.
	 */

	*using_dac = false;
	err = pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	if (err) {
		err = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
		if (err) {
			dev_err(dev, "No usable DMA configuration, aborting\n");
			return err;
		}
		err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
		if (err) {
			dev_err(dev, "Unable to obtain 32-bit DMA "
				"for consistent allocations, aborting\n");
			return err;
		}
	} else {
		err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
		if (err) {
			dev_err(dev, "Unable to obtain 64-bit DMA "
				"for consistent allocations, aborting\n");
			return err;
		}
		*using_dac = true;
	}

	return 0;
}

static int ionic_dev_cmd_wait(struct ionic_dev *idev, unsigned long max_wait)
{
	unsigned long time;
	int done;

	BUG_ON(in_interrupt());

	/* Wait for dev cmd to complete...but no more than max_wait
	 */

	time = jiffies + max_wait;
	do {

		done = ionic_dev_cmd_done(idev);
		if (done)
			return 0;

		schedule_timeout_uninterruptible(HZ / 10);

	} while (time_after(time, jiffies));

	return -ETIMEDOUT;
}

static int ionic_dev_cmd_check_error(struct ionic_dev *idev)
{
	u8 status;

	status = ionic_dev_cmd_status(idev);
	switch (status) {
	case 0:
		return 0;
	}

	return -EIO;
}

static int ionic_dev_cmd_wait_check(struct ionic_dev *idev,
				    unsigned long max_wait)
{
	int err;

	err = ionic_dev_cmd_wait(idev, max_wait);
	if (err)
		return err;
	return ionic_dev_cmd_check_error(idev);
}

static int ionic_setup(struct ionic *ionic)
{
	int err;

	err = ionic_dev_setup(&ionic->idev, ionic->bars, ionic->num_bars);
	if (err)
		return err;

	return ionic_debugfs_add_dev_cmd(ionic);
}

static int ionic_identify(struct ionic *ionic)
{
	struct pci_dev *pdev = ionic->pdev;
	struct device *dev = &pdev->dev;
	struct ionic_dev *idev = &ionic->idev;
	union identity *ident;
	dma_addr_t ident_pa;
	int err;

	ident = devm_kzalloc(dev, sizeof(*ident), GFP_KERNEL | GFP_DMA);
	if (!ident)
		return -ENOMEM;
	ident_pa = pci_map_single(pdev, ident, sizeof(*ident),
				  PCI_DMA_FROMDEVICE);
	if (pci_dma_mapping_error(pdev, ident_pa))
		return -EIO;

	ionic_dev_cmd_identify(idev, ident_pa);

	err = ionic_dev_cmd_wait_check(idev, HZ * 2);
	if (err)
		goto err_out_unmap;

	err = ionic_debugfs_add_ident(ionic);
	if (err)
		goto err_out_unmap;

	ionic->ident = ident;
	ionic->ident_pa = ident_pa;

	return 0;

err_out_unmap:
	pci_unmap_single(pdev, ident_pa, sizeof(*ident),
			 PCI_DMA_FROMDEVICE);
	return err;
}

static void ionic_forget_identity(struct ionic *ionic)
{
	pci_unmap_single(ionic->pdev, ionic->ident_pa, sizeof(*ionic->ident),
			 PCI_DMA_FROMDEVICE);
}

static int ionic_reset(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;

	ionic_dev_cmd_reset(idev);
	return ionic_dev_cmd_wait_check(idev, HZ * 2);
}

static void ionic_get_drvinfo(struct net_device *netdev,
			      struct ethtool_drvinfo *drvinfo)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic *ionic = lif->ionic;

	strlcpy(drvinfo->driver, DRV_NAME, sizeof(drvinfo->driver));
	strlcpy(drvinfo->version, DRV_VERSION, sizeof(drvinfo->version));
	strlcpy(drvinfo->fw_version, ionic->ident->fw_version,
		sizeof(drvinfo->fw_version));
	strlcpy(drvinfo->bus_info, pci_name(ionic->pdev),
		sizeof(drvinfo->bus_info));
}

static void ionic_get_ringparam(struct net_device *netdev,
				struct ethtool_ringparam *ring)
{
	ring->tx_max_pending = 1 << 16;
	ring->tx_pending = ntxq_descs;
	ring->rx_max_pending = 1 << 16;
	ring->rx_pending = nrxq_descs;
}

static const struct ethtool_ops ionic_ethtool_ops = {
	.get_drvinfo		= ionic_get_drvinfo,
	.get_link		= ethtool_op_get_link,
	.get_ringparam		= ionic_get_ringparam,
};

#ifdef ADMINQ
static void ionic_qcq_enable_cb(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct qcq *qcq = cb_arg;

	napi_enable(&qcq->napi);
	ionic_intr_mask(&qcq->intr, false);
}
#endif

static int ionic_qcq_enable(struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
#ifdef ADMINQ
	struct queue *adminq = &lif->adminqcq->q;
#endif
	struct ionic_dev *idev = &lif->ionic->idev;
	q_enable_comp comp;
	int err;

#ifdef ADMINQ
	return ionic_adminq_q_enable(adminq, q, ionic_qcq_enable_cb, qcq);
#else
	ionic_dev_cmd_q_enable(idev, q);
	err = ionic_dev_cmd_wait_check(idev, HZ * 2);
	if (err)
		return err;
	ionic_dev_cmd_comp(idev, &comp);
	napi_enable(&qcq->napi);
	ionic_intr_mask(&qcq->intr, false);
	return 0;
#endif
}

static void ionic_rx_fill(struct queue *q);

static int ionic_open(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	unsigned int i;
	int err;

	for (i = 0; i < lif->ntxqcqs; i++) {
		err = ionic_qcq_enable(lif->txqcqs[i]);
		if (err)
			return err;
	}

	netif_tx_wake_all_queues(netdev);

	for (i = 0; i < lif->nrxqcqs; i++) {
		ionic_rx_fill(&lif->rxqcqs[i]->q);
		err = ionic_qcq_enable(lif->rxqcqs[i]);
		if (err)
			return err;
	}

	return 0;
}

static int ionic_qcq_disable(struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
#ifdef ADMINQ
	struct queue *adminq = &lif->adminqcq->q;
#endif
	struct ionic_dev *idev = &lif->ionic->idev;
	q_disable_comp comp;
	int err;

	ionic_intr_mask(&qcq->intr, true);
	synchronize_irq(qcq->intr.vector);
	napi_disable(&qcq->napi);

#ifdef ADMINQ
	return ionic_adminq_q_disable(adminq, q, NULL, NULL);
#else
	ionic_dev_cmd_q_disable(idev, q);
	err = ionic_dev_cmd_wait_check(idev, HZ * 2);
	if (err)
		return err;
	ionic_dev_cmd_comp(idev, &comp);
	return 0;
#endif
}

static int ionic_stop(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	unsigned int i;
	int err;

	netif_carrier_off(netdev);
	netif_tx_disable(netdev);

	for (i = 0; i < lif->ntxqcqs; i++) {
		err = ionic_qcq_disable(lif->txqcqs[i]);
		if (err)
			return err;
	}

	for (i = 0; i < lif->nrxqcqs; i++) {
		err = ionic_qcq_disable(lif->rxqcqs[i]);
		if (err)
			return err;
	}

	return 0;
}

static int ionic_napi(struct napi_struct *napi, int budget, ionic_cq_cb cb,
		      void *cb_arg)
{
	struct cq *cq = napi_to_cq(napi);
	unsigned int work_done;

	work_done = ionic_cq_service(cq, budget, cb, cb_arg);

	if (work_done > 0)
		ionic_intr_return_credits(cq->bound_intr, work_done, 0, true);

	if ((work_done < budget) && napi_complete_done(napi, work_done))
		ionic_intr_mask(cq->bound_intr, false);

	return work_done;
}

static int ionic_adminq_napi(struct napi_struct *napi, int budget)
{
	return ionic_napi(napi, budget, ionic_adminq_service, NULL);
}

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
	struct pci_dev *pdev = q->lif->ionic->pdev;
	struct rxq_desc *desc = desc_info->desc;
	struct rxq_comp *comp = cq_info->cq_desc;
	struct sk_buff *skb = cb_arg;
	struct qcq *qcq = q_to_qcq(q);
	struct rx_stats *stats = q_to_rx_stats(q);
	dma_addr_t dma_addr;

	if (comp->status) {
		// TODO record errors
		ionic_rx_recycle(q, desc_info, skb);
		return;
	}

	stats->pkts++;
	stats->bytes += comp->len;

	// TODO add copybreak to avoid allocating a new skb for small receive

	dma_addr = (dma_addr_t)desc->addr;
	pci_unmap_single(pdev, dma_addr, desc->len, PCI_DMA_FROMDEVICE);

	//prefetch(skb->data - NET_IP_ALIGN);
	skb_put(skb, comp->len);
	skb->protocol = eth_type_trans(skb, netdev);
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
		if (comp->csum_level) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			skb->csum_level = comp->csum_level - 1;
		} else {
			skb->ip_summed = CHECKSUM_COMPLETE;
			skb->csum = comp->csum;
		}
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
	struct pci_dev *pdev = lif->ionic->pdev;
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

	*dma_addr = pci_map_single(pdev, skb->data, len, PCI_DMA_FROMDEVICE);
	if (pci_dma_mapping_error(pdev, *dma_addr)) {
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
	struct pci_dev *pdev = q->lif->ionic->pdev;

	//printk(KERN_ERR "%s ionic_rx_skb_free len %d\n", q->name, len);
	pci_unmap_single(pdev, dma_addr, len, PCI_DMA_FROMDEVICE);
	dev_kfree_skb(skb);
}

#define RX_RING_DOORBELL_STRIDE		((1 << 2) - 1)

static void ionic_rx_fill(struct queue *q)
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

static void ionic_rx_refill(struct queue *q)
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

static void ionic_rx_empty(struct queue *q)
{
	struct desc_info *cur = q->tail;
	struct rxq_desc *desc;

	while (cur != q->head) {
		desc = cur->desc;
		ionic_rx_skb_free(q, cur->cb_arg, desc->len, desc->addr);
		cur = cur->next;
	}
}

static int ionic_rx_napi(struct napi_struct *napi, int budget)
{
	struct cq *cq = napi_to_cq(napi);
	unsigned int work_done;

	work_done = ionic_napi(napi, budget, ionic_rx_service, NULL);

	ionic_rx_fill(cq->bound_q);

	return work_done;
}

static dma_addr_t ionic_tx_map_single(struct queue *q, void *data, size_t len)
{
	struct pci_dev *pdev = q->lif->ionic->pdev;
	struct tx_stats *stats = q_to_tx_stats(q);
	dma_addr_t dma_addr;

	dma_addr = pci_map_single(pdev, data, len, PCI_DMA_TODEVICE);
	if (pci_dma_mapping_error(pdev, dma_addr)) {
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
	struct pci_dev *pdev = q->lif->ionic->pdev;
	struct tx_stats *stats = q_to_tx_stats(q);
	dma_addr_t dma_addr;

	dma_addr = skb_frag_dma_map(&pdev->dev, frag, offset, len,
				    DMA_TO_DEVICE);
	if (pci_dma_mapping_error(pdev, dma_addr)) {
		net_warn_ratelimited("%s: DMA frag map failed on %s!\n",
				     q->lif->netdev->name, q->name);
		stats->dma_map_err++;
		return 0;
	}
	return dma_addr;
}

static void ionic_tx_clean_sop(struct pci_dev *pdev, struct txq_desc *desc)
{
	dma_addr_t dma_addr;

	switch (desc->opcode) {
	case TXQ_DESC_OPCODE_NOP:
		break;
	case TXQ_DESC_OPCODE_CALC_NO_CSUM:
	case TXQ_DESC_OPCODE_CALC_CSUM:
	case TXQ_DESC_OPCODE_TSO:
		dma_addr = (dma_addr_t)desc->addr;
		pci_unmap_single(pdev, dma_addr, desc->len, PCI_DMA_TODEVICE);
		break;
	}
}

static void ionic_tx_clean_frags(struct pci_dev *pdev, unsigned int elems,
				 struct txq_sg_desc *sg_desc)
{
	struct txq_sg_elem *elem = sg_desc->elems;
	dma_addr_t dma_addr;

	for (; elems; elems--, elem++) {
		dma_addr = (dma_addr_t)elem->addr;
		pci_unmap_page(pdev, dma_addr, elem->len, PCI_DMA_TODEVICE);
	}
}

static void ionic_tx_clean(struct queue *q, struct desc_info *desc_info,
			   struct cq_info *cq_info, void *cb_arg)
{
	struct pci_dev *pdev = q->lif->ionic->pdev;
	struct tx_stats *stats = q_to_tx_stats(q);
	struct sk_buff *skb = cb_arg;

	ionic_tx_clean_sop(pdev, desc_info->desc);
	ionic_tx_clean_frags(pdev, skb_shinfo(skb)->nr_frags,
			     desc_info->sg_desc);
	dev_kfree_skb_any(skb);

	stats->clean++;
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

static int ionic_tx_napi(struct napi_struct *napi, int budget)
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

	desc->opcode = skb->csum_not_inet ?
		TXQ_DESC_OPCODE_CALC_CRC32_CSUM :
		TXQ_DESC_OPCODE_CALC_CSUM;
	desc->num_sg_elems = skb_shinfo(skb)->nr_frags;
	desc->len = skb_headlen(skb);
	desc->addr = addr;
	desc->vlan_tci = skb_vlan_tag_get(skb);
	desc->hdr_len = skb_checksum_start_offset(skb);
	desc->csum_offset = desc->hdr_len + skb->csum_offset;
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

static netdev_tx_t ionic_start_xmit(struct sk_buff *skb,
				    struct net_device *netdev)
{
	u16 queue_index = skb_get_queue_mapping(skb);
	struct lif *lif = netdev_priv(netdev);
	struct queue *q = lif_to_txq(lif, queue_index);
	struct tx_stats *stats = q_to_tx_stats(q);
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
	// TODO do we honor skb->xmit_more to not ring doorbell?
	ionic_q_post(q, true, ionic_tx_clean, skb);

	stats->pkts++;
	stats->bytes += skb->len;

	if (!ionic_q_has_space(q, 1)) {
		netif_stop_subqueue(netdev, queue_index);
		stats->stop++;
	}

	return NETDEV_TX_OK;

err_out_drop:
	stats->drop++;
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

static void ionic_get_stats64(struct net_device *netdev,
			      struct rtnl_link_stats64 *net_stats)
{
	struct lif *lif = netdev_priv(netdev);
	struct rx_stats *rx_stats;
	struct tx_stats *tx_stats;
	unsigned int i;

	for (i = 0; i < lif->ntxqcqs; i++) {
		tx_stats = &lif->txqcqs[i]->stats.tx;
		net_stats->tx_packets += tx_stats->pkts;
		net_stats->tx_bytes += tx_stats->bytes;
	}

	for (i = 0; i < lif->nrxqcqs; i++) {
		rx_stats = &lif->rxqcqs[i]->stats.rx;
		net_stats->rx_packets += rx_stats->pkts;
		net_stats->rx_bytes += rx_stats->bytes;
	}
}

static void ionic_rx_filter_cb(struct queue *q, struct desc_info *desc_info,
			       struct cq_info *cq_info, void *cb_arg)
{
	struct rx_filter_cmd *cmd = desc_info->desc;

	switch (cmd->match) {
	case RX_FILTER_MATCH_VLAN:
		printk(KERN_ERR "adminq rx_filter vlan %d\n", cmd->vlan);
		break;
	case RX_FILTER_MATCH_MAC:
		printk(KERN_ERR "adminq rx_filter mac %pM\n", cmd->addr);
		break;
	case RX_FILTER_MATCH_MAC_VLAN:
		printk(KERN_ERR "adminq rx_filter mac/vlan %pM/%d on qid %d\n",
		       cmd->addr, cmd->vlan, cmd->qid);
		break;
	}
}

static int ionic_lif_addr(struct lif *lif, const u8 *addr, bool add)
{
#ifdef ADMINQ
	struct queue *adminq = &lif->adminqcq->q;

	return ionic_adminq_rx_filter_mac(adminq, addr, add,
					  ionic_rx_filter_cb, NULL);
#else
	struct ionic_dev *idev = &lif->ionic->idev;

	printk(KERN_ERR "dev_cmd %s rx_filter %pM\n", add ? "add" : "del", addr);
	ionic_dev_cmd_rx_filter_mac(idev, addr, add);
	return 0;
#endif
}

static int ionic_addr_add(struct net_device *netdev, const u8 *addr)
{
	struct lif *lif = netdev_priv(netdev);

	return ionic_lif_addr(lif, addr, true);
}

static int ionic_addr_del(struct net_device *netdev, const u8 *addr)
{
	struct lif *lif = netdev_priv(netdev);

	return ionic_lif_addr(lif, addr, false);
}

#ifdef NOT_YET
static void ionic_lif_rx_mode_cb(struct queue *q, struct desc_info *desc_info,
				 struct cq_info *cq_info, void *cb_arg)
{
	struct lif *lif = cb_arg;
	printk(KERN_ERR "adminq set rx_mode %d\n", lif->rx_mode);
}
#endif

static int ionic_lif_rx_mode(struct lif *lif)
{
#ifdef NOT_YET
	struct queue *adminq = &lif->adminqcq->q;

	return ionic_adminq_rx_mode_set(adminq, lif->rx_mode,
					ionic_lif_rx_mode_cb, lif);
#else
	return 0;
#endif
}

static void ionic_set_rx_mode(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	union identity *ident = lif->ionic->ident;
	unsigned int rx_mode;

	rx_mode = RX_MODE_F_UNICAST;
	rx_mode |= (netdev->flags & IFF_MULTICAST) ? RX_MODE_F_MULTICAST : 0;
	rx_mode |= (netdev->flags & IFF_BROADCAST) ? RX_MODE_F_BROADCAST : 0;
	rx_mode |= (netdev->flags & IFF_PROMISC) ? RX_MODE_F_PROMISC : 0;
	rx_mode |= (netdev->flags & IFF_ALLMULTI) ? RX_MODE_F_ALLMULTI : 0;

	if (netdev_uc_count(netdev) + 1 > ident->nucasts_per_lif)
		rx_mode |= RX_MODE_F_PROMISC;
	if (netdev_mc_count(netdev) > ident->nmcasts_per_lif)
		rx_mode |= RX_MODE_F_ALLMULTI;

	if (lif->rx_mode != rx_mode) {
		lif->rx_mode = rx_mode;
		ionic_lif_rx_mode(lif);
	}

	__dev_uc_sync(netdev, ionic_addr_add, ionic_addr_del);
	__dev_mc_sync(netdev, ionic_addr_add, ionic_addr_del);
}

static int ionic_set_mac_address(struct net_device *netdev, void *addr)
{
	// TODO implement
	printk(KERN_ERR "%s SET MAC ADDRESS not implememnted\n", netdev->name);
	return 0;
}

static void ionic_change_mtu_cb(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct lif *lif = q->lif;
	struct net_device *netdev = lif->netdev;
	struct mtu_set_cmd *cmd = desc_info->desc;
	unsigned int i;

	rtnl_lock();

	netdev->mtu = cmd->mtu;

	for (i = 0; i < lif->nrxqcqs; i++)
		ionic_rx_refill(&lif->rxqcqs[i]->q);

	if (netif_running(netdev))
		ionic_open(netdev);

	rtnl_unlock();
}

static int ionic_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct lif *lif = netdev_priv(netdev);
	struct queue *adminq = &lif->adminqcq->q;

	if (netif_running(netdev))
		ionic_stop(netdev);

	return ionic_adminq_mtu_set(adminq, new_mtu, ionic_change_mtu_cb,
				    NULL);
}

static void ionic_tx_timeout(struct net_device *netdev)
{
	// TODO implement
}

static int ionic_vlan_rx_add_vid(struct net_device *netdev,
				 __be16 proto, u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	struct queue *adminq = &lif->adminqcq->q;

	return ionic_adminq_rx_filter_vlan(adminq, vid, true,
					   ionic_rx_filter_cb, NULL);
}

static int ionic_vlan_rx_kill_vid(struct net_device *netdev,
				  __be16 proto, u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	struct queue *adminq = &lif->adminqcq->q;

	return ionic_adminq_rx_filter_vlan(adminq, vid, false,
					   ionic_rx_filter_cb, NULL);
}

static const struct net_device_ops ionic_netdev_ops = {
	.ndo_open               = ionic_open,
	.ndo_stop               = ionic_stop,
	.ndo_start_xmit		= ionic_start_xmit,
	.ndo_get_stats64	= ionic_get_stats64,
	.ndo_set_rx_mode	= ionic_set_rx_mode,
	.ndo_set_mac_address	= ionic_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= ionic_change_mtu,
	.ndo_tx_timeout		= ionic_tx_timeout,
	.ndo_vlan_rx_add_vid	= ionic_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= ionic_vlan_rx_kill_vid,
};

static irqreturn_t ionic_isr(int irq, void *data)
{
	struct napi_struct *napi = data;

	napi_schedule_irqoff(napi);

	return IRQ_HANDLED;
}

static int ionic_intr_alloc(struct lif *lif, struct intr *intr)
{
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;
	unsigned long index;

	index = find_first_zero_bit(ionic->intrs, ionic->nintrs);
	if (index == ionic->nintrs)
		return -ENOSPC;
	set_bit(index, ionic->intrs);

	return ionic_intr_init(idev, intr, index);
}

static void ionic_intr_free(struct lif *lif, struct intr *intr)
{
	if (intr->index != INTR_INDEX_NOT_ASSIGNED)
		clear_bit(intr->index, lif->ionic->intrs);
}

static int ionic_qcq_alloc(struct lif *lif, unsigned int index,
			   const char *base, unsigned int flags,
			   unsigned int num_descs, unsigned int desc_size,
			   unsigned int cq_desc_size,
			   unsigned int sg_desc_size,
			   unsigned int pid, struct qcq **qcq)
{
	struct pci_dev *pdev = lif->ionic->pdev;
	struct device *dev = &pdev->dev;
	struct qcq *new;
	unsigned int q_size = num_descs * desc_size;
	unsigned int cq_size = num_descs * cq_desc_size;
	unsigned int sg_size = num_descs * sg_desc_size;
	unsigned int total_size = ALIGN(q_size, PAGE_SIZE) +
				  ALIGN(cq_size, PAGE_SIZE) +
				  ALIGN(sg_size, PAGE_SIZE);
	void *q_base, *cq_base, *sg_base;
	dma_addr_t q_base_pa, cq_base_pa, sg_base_pa;
	int err;

	*qcq = NULL;

	total_size = ALIGN(q_size, PAGE_SIZE) + ALIGN(cq_size, PAGE_SIZE);
	if (flags & QCQ_F_SG)
		total_size += ALIGN(sg_size, PAGE_SIZE);

	new = devm_kzalloc(dev, sizeof(*new), GFP_KERNEL);
	if (!new)
		return -ENOMEM;

	new->flags = flags;

	new->q.info = devm_kzalloc(dev, sizeof(*new->q.info) * num_descs,
				   GFP_KERNEL);
	if (!new->q.info)
		return -ENOMEM;

	err = ionic_q_init(lif, &new->q, index, base, num_descs, desc_size,
			   sg_desc_size, pid);
	if (err)
		return err;

	if (flags & QCQ_F_INTR) {
		err = ionic_intr_alloc(lif, &new->intr);
		if (err)
			return err;
		err = pci_irq_vector(pdev, new->intr.index);
		if (err < 0)
			goto err_out_free_intr;
		new->intr.vector = err;
		ionic_intr_mask_on_assertion(&new->intr);
	} else {
		new->intr.index = INTR_INDEX_NOT_ASSIGNED;
	}

	new->cq.info = devm_kzalloc(dev, sizeof(*new->cq.info) * num_descs,
				    GFP_KERNEL);
	if (!new->cq.info)
		return -ENOMEM;

	err = ionic_cq_init(lif, &new->cq, &new->intr,
			    num_descs, cq_desc_size);
	if (err)
		goto err_out_free_intr;

	new->base = pci_alloc_consistent(pdev, total_size,
					 &new->base_pa);
	if (!new->base) {
		err = -ENOMEM;
		goto err_out_free_intr;
	}

	new->total_size = total_size;

	q_base = new->base;
	q_base_pa = new->base_pa;

	cq_base = (void *)ALIGN((uintptr_t)q_base + q_size, PAGE_SIZE);
	cq_base_pa = ALIGN(q_base_pa + q_size, PAGE_SIZE);

	if (flags & QCQ_F_SG) {
		sg_base = (void *)ALIGN((uintptr_t)cq_base + cq_size,
					PAGE_SIZE);
		sg_base_pa = ALIGN(cq_base_pa + cq_size, PAGE_SIZE);
		ionic_q_sg_map(&new->q, sg_base, sg_base_pa);
	}

	ionic_q_map(&new->q, q_base, q_base_pa);
	ionic_cq_map(&new->cq, cq_base, cq_base_pa);
	ionic_cq_bind(&new->cq, &new->q);

	*qcq = new;

	return 0;

err_out_free_intr:
	ionic_intr_free(lif, &new->intr);

	return err;
}

static void ionic_qcq_free(struct lif *lif, struct qcq *qcq)
{
	if (!qcq)
		return;

	pci_free_consistent(lif->ionic->pdev, qcq->total_size,
			    qcq->base, qcq->base_pa);
	ionic_intr_free(lif, &qcq->intr);
}

static unsigned int ionic_pid_get(struct lif *lif, unsigned int page)
{
	unsigned int ndbpgs_per_lif = lif->ionic->ident->ndbpgs_per_lif;

	BUG_ON(ndbpgs_per_lif < page + 1);

	return lif->index * ndbpgs_per_lif + page;
}

static int ionic_qcqs_alloc(struct lif *lif)
{
	struct device *dev = &lif->ionic->pdev->dev;
	unsigned int flags;
	unsigned int pid;
	unsigned int i;
	int err = -ENOMEM;

	lif->txqcqs = devm_kzalloc(dev, sizeof(*lif->txqcqs) * lif->ntxqcqs,
				   GFP_KERNEL);
	if (!lif->txqcqs)
		return -ENOMEM;

	lif->rxqcqs = devm_kzalloc(dev, sizeof(*lif->rxqcqs) * lif->nrxqcqs,
				   GFP_KERNEL);
	if (!lif->rxqcqs)
		return -ENOMEM;

	pid = ionic_pid_get(lif, 0);
	flags = QCQ_F_INTR;
	err = ionic_qcq_alloc(lif, 0, "admin", flags, 1 << 4,
			      sizeof(struct admin_cmd),
			      sizeof(struct admin_comp),
			      0, pid, &lif->adminqcq);
	if (err)
		return err;

	pid = ionic_pid_get(lif, 0);
	flags = QCQ_F_TX_STATS | QCQ_F_INTR | QCQ_F_SG;
	for (i = 0; i < lif->ntxqcqs; i++) {
		err = ionic_qcq_alloc(lif, i, "tx", flags, ntxq_descs,
				      sizeof(struct txq_desc),
				      sizeof(struct txq_comp),
				      sizeof(struct txq_sg_desc),
				      pid, &lif->txqcqs[i]);
		if (err)
			goto err_out_free_adminqcq;
	}

	pid = ionic_pid_get(lif, 0);
	flags = QCQ_F_RX_STATS | QCQ_F_INTR;
	for (i = 0; i < lif->nrxqcqs; i++) {
		err = ionic_qcq_alloc(lif, i, "rx", flags, nrxq_descs,
				      sizeof(struct rxq_desc),
				      sizeof(struct rxq_comp),
				      0, pid, &lif->rxqcqs[i]);
		if (err)
			goto err_out_free_txqcqs;
	}

	return 0;

err_out_free_txqcqs:
	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_qcq_free(lif, lif->txqcqs[i]);
err_out_free_adminqcq:
	ionic_qcq_free(lif, lif->adminqcq);

	return err;
}

static void ionic_qcqs_free(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < lif->nrxqcqs; i++) {
		ionic_rx_empty(&lif->rxqcqs[i]->q);
		ionic_qcq_free(lif, lif->rxqcqs[i]);
	}
	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_qcq_free(lif, lif->txqcqs[i]);
	ionic_qcq_free(lif, lif->adminqcq);
}

static int ionic_lif_alloc(struct ionic *ionic, unsigned int index)
{
	struct device *dev = &ionic->pdev->dev;
	struct net_device *netdev;
	struct lif *lif;
	int err;

	netdev = alloc_etherdev_mqs(sizeof(*lif),
				    ionic->ntxqs_per_lif,
				    ionic->nrxqs_per_lif);
	if (!netdev) {
		dev_err(dev, "Cannot allocate netdev, aborting\n");
		return -ENOMEM;
	}

	SET_NETDEV_DEV(netdev, dev);

	lif = netdev_priv(netdev);
	lif->netdev = netdev;
	lif->ionic = ionic;
	lif->index = index;
	lif->ntxqcqs = ionic->ntxqs_per_lif;
	lif->nrxqcqs = ionic->nrxqs_per_lif;

	snprintf(lif->name, sizeof(lif->name), "lif%u", index);

//	netdev->hw_features |= NETIF_F_HW_VLAN_CTAG_TX;
//	netdev->hw_features |= NETIF_F_HW_VLAN_CTAG_RX;
//	netdev->hw_features |= NETIF_F_HW_VLAN_CTAG_FILTER;
//	netdev->hw_features |= NETIF_F_RXHASH;
	netdev->hw_features |= NETIF_F_SG;

	netdev->hw_enc_features |= NETIF_F_HW_CSUM;
//	netdev->hw_enc_features |= NETIF_F_RXCSUM | NETIF_F_HW_CSUM;
//	netdev->hw_enc_features |= NETIF_F_TSO | NETIF_F_TSO6;
//	netdev->hw_enc_features |= NETIF_F_TSO_ECN;
//	netdev->hw_enc_features |= NETIF_F_GSO_GRE | NETIF_F_GSO_GRE_CSUM;
//	netdev->hw_enc_features |= NETIF_F_GSO_IPXIP4 | NETIF_F_GSO_IPXIP6;
//	netdev->hw_enc_features |= NETIF_F_GSO_UDP_TUNNEL;
//	netdev->hw_enc_features |= NETIF_F_GSO_UDP_TUNNEL_CSUM;
//	netdev->hw_enc_features |= NETIF_F_SCTP_CRC;

	netdev->hw_features |= netdev->hw_enc_features;
	netdev->features |= netdev->hw_features;

	if (ionic->using_dac)
		netdev->features |= NETIF_F_HIGHDMA;

	netdev->vlan_features |= netdev->features;

	netdev->netdev_ops = &ionic_netdev_ops;
	netdev->ethtool_ops = &ionic_ethtool_ops;
	netdev->watchdog_timeo = 2 * HZ;

	netdev->min_mtu = IONIC_MIN_MTU;
	netdev->max_mtu = IONIC_MAX_MTU;

	err = ionic_qcqs_alloc(lif);
	if (err)
		goto err_out_free_netdev;

	list_add_tail(&lif->list, &ionic->lifs);

	return 0;

err_out_free_netdev:
	free_netdev(netdev);

	return err;
}

static int ionic_lifs_alloc(struct ionic *ionic)
{
	unsigned int i;
	int err;

	INIT_LIST_HEAD(&ionic->lifs);

	for (i = 0; i < ionic->ident->nlifs; i++) {
		err = ionic_lif_alloc(ionic, i);
		if (err)
			return err;
	}

	return 0;
}

static void ionic_lifs_free(struct ionic *ionic)
{
	struct list_head *cur, *tmp;
	struct lif *lif;

	list_for_each_safe(cur, tmp, &ionic->lifs) {
		lif = list_entry(cur, struct lif, list);
		list_del(&lif->list);
		ionic_qcqs_free(lif);
		free_netdev(lif->netdev);
	}
}

static void ionic_lif_qcq_deinit(struct qcq *qcq)
{
	if (!(qcq->flags & QCQ_F_INITED))
		return;
	ionic_intr_mask(&qcq->intr, true);
	free_irq(qcq->intr.vector, &qcq->napi);
	netif_napi_del(&qcq->napi);
	qcq->flags &= ~QCQ_F_INITED;
}

static void ionic_lif_txqs_deinit(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_lif_qcq_deinit(lif->txqcqs[i]);
}

static void ionic_lif_rxqs_deinit(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < lif->nrxqcqs; i++)
		ionic_lif_qcq_deinit(lif->rxqcqs[i]);
}

static void ionic_lif_deinit(struct lif *lif)
{
	ionic_lif_qcq_deinit(lif->adminqcq);
	ionic_lif_txqs_deinit(lif);
	ionic_lif_rxqs_deinit(lif);
}

static void ionic_lifs_deinit(struct ionic *ionic)
{
	struct list_head *cur;
	struct lif *lif;

	list_for_each(cur, &ionic->lifs) {
		lif = list_entry(cur, struct lif, list);
		ionic_lif_deinit(lif);
	}
}

static int ionic_request_irq(struct lif *lif, struct qcq *qcq)
{
	struct intr *intr = &qcq->intr;
	struct queue *q = &qcq->q;
	struct napi_struct *napi = &qcq->napi;

	snprintf(intr->name, sizeof(intr->name),
		 "%s-%s-%s", DRV_NAME, lif->name, q->name);
	return request_irq(intr->vector, ionic_isr,
			   0, intr->name, napi);
}

static int ionic_lif_adminq_init(struct lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct qcq *qcq = lif->adminqcq;
	struct queue *q = &qcq->q;
	struct napi_struct *napi = &qcq->napi;
	struct adminq_init_comp comp;
	int err;

	ionic_dev_cmd_adminq_init(idev, q, 0, lif->index, 0);
	err = ionic_dev_cmd_wait_check(idev, HZ * 2);
	if (err)
		return err;

	ionic_dev_cmd_comp(idev, &comp);
	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = ionic_db_map(idev, q);

	netif_napi_add(lif->netdev, napi, ionic_adminq_napi,
		       NAPI_POLL_WEIGHT);

	err = ionic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	lif->adminq_cmd = ionic_adminq_rdma_cmd;

	qcq->flags |= QCQ_F_INITED;

	return 0;
}

#ifdef ADMINQ
static void ionic_station_set(struct queue *q, struct desc_info *desc_info,
			      struct cq_info *cq_info, void *cb_arg)
{
	struct lif *lif = cb_arg;
	struct net_device *netdev = lif->netdev;
	struct station_mac_addr_get_comp *comp = cq_info->cq_desc;

	printk(KERN_ERR "deleting station MAC addr %pM\n", netdev->dev_addr);
	ionic_lif_addr(lif, netdev->dev_addr, false);
	memcpy(netdev->dev_addr, comp->addr, netdev->addr_len);
	printk(KERN_ERR "adding station MAC addr %pM\n", netdev->dev_addr);
	ionic_lif_addr(lif, netdev->dev_addr, true);
}
#endif

static int ionic_lif_txq_init(struct lif *lif, struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct cq *cq = &qcq->cq;
	struct napi_struct *napi = &qcq->napi;
#ifdef ADMINQ
	struct queue *adminq = &lif->adminqcq->q;
	struct cq *admincq = &lif->adminqcq->cq;
#else
	struct ionic_dev *idev = &lif->ionic->idev;
	struct txq_init_comp comp;
#endif
	int err;

#ifdef ADMINQ
	err = ionic_adminq_txq_init(adminq, q, cq, 0);
	if (err)
		return err;
	ionic_cq_service(admincq, 1, ionic_adminq_service, NULL);
#else
	ionic_dev_cmd_txq_init(idev, q, cq, 0);
	err = ionic_dev_cmd_wait_check(idev, HZ * 2);
	if (err)
		return err;
	ionic_dev_cmd_comp(idev, &comp);
	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = ionic_db_map(idev, q);
#endif

	netif_napi_add(lif->netdev, napi, ionic_tx_napi,
		       NAPI_POLL_WEIGHT);

	err = ionic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	return 0;
}

static int ionic_lif_txqs_init(struct lif *lif)
{
	unsigned int i;
	int err;

	for (i = 0; i < lif->ntxqcqs; i++) {
		err = ionic_lif_txq_init(lif, lif->txqcqs[i]);
		if (err)
			return err;
	}

	return 0;
}

static int ionic_lif_rxq_init(struct lif *lif, struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct cq *cq = &qcq->cq;
	struct napi_struct *napi = &qcq->napi;
#ifdef ADMINQ
	struct queue *adminq = &lif->adminqcq->q;
	struct cq *admincq = &lif->adminqcq->cq;
#else
	struct ionic_dev *idev = &lif->ionic->idev;
	struct rxq_init_comp comp;
#endif
	int err;

#ifdef ADMINQ
	err = ionic_adminq_rxq_init(adminq, q, cq);
	if (err)
		return err;
	ionic_cq_service(admincq, 1, ionic_adminq_service, NULL);
#else
	ionic_dev_cmd_rxq_init(idev, q, cq);
	err = ionic_dev_cmd_wait_check(idev, HZ * 2);
	if (err)
		return err;
	ionic_dev_cmd_comp(idev, &comp);
	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = ionic_db_map(idev, q);
#endif

	netif_napi_add(lif->netdev, napi, ionic_rx_napi,
		       NAPI_POLL_WEIGHT);

	err = ionic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	return 0;
}

static int ionic_lif_rxqs_init(struct lif *lif)
{
	unsigned int i;
	int err;

	for (i = 0; i < lif->nrxqcqs; i++) {
		err = ionic_lif_rxq_init(lif, lif->rxqcqs[i]);
		if (err)
			return err;
	}

	return 0;
}

static int ionic_lif_init(struct lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
#ifndef ADMINQ
	struct station_mac_addr_get_comp comp;
#endif
	int err;

	ionic_dev_cmd_lif_init(idev, lif->index);
	err = ionic_dev_cmd_wait_check(idev, HZ * 2);
	if (err)
		return err;

	err = ionic_lif_adminq_init(lif);
	if (err)
		return err;

	err = ionic_lif_txqs_init(lif);
	if (err)
		return err;

	err = ionic_lif_rxqs_init(lif);
	if (err)
		goto err_out_txqs_deinit;

	// TODO move this above ionic_lif_txqs_init once ADMINQ is working...

	/* Enabling interrupts on adminq from here on... */
	ionic_intr_mask(&lif->adminqcq->intr, false);

#ifdef ADMINQ
	err = ionic_adminq_station_get(&lif->adminqcq->q,
				       ionic_station_set, lif);
	if (err)
		goto err_out_mask_adminq;
#else
	ionic_dev_cmd_station_get(idev);
	err = ionic_dev_cmd_wait_check(idev, HZ * 2);
	if (err)
		goto err_out_mask_adminq;
	ionic_dev_cmd_comp(idev, &comp);
	memcpy(lif->netdev->dev_addr, comp.addr, lif->netdev->addr_len);
	ionic_lif_addr(lif, lif->netdev->dev_addr, true);
#endif

	ionic_set_rx_mode(lif->netdev);

	return 0;

err_out_mask_adminq:
	ionic_intr_mask(&lif->adminqcq->intr, true);
err_out_txqs_deinit:
	ionic_lif_txqs_deinit(lif);

	return err;
}

static int ionic_lifs_init(struct ionic *ionic)
{
	struct list_head *cur;
	struct lif *lif;
	int err;

	list_for_each(cur, &ionic->lifs) {
		lif = list_entry(cur, struct lif, list);
		err = ionic_lif_init(lif);
		if (err)
			return err;
	}

	return 0;
}

static int ionic_lif_register(struct lif *lif)
{
	struct device *dev = &lif->ionic->pdev->dev;
	int err;

	err = ionic_debugfs_add_lif(lif);
	if (err)
		return err;

	err = register_netdev(lif->netdev);
	if (err) {
		dev_err(dev, "Cannot register net device, aborting\n");
		return err;
	}

	lif->registered = true;

	return 0;
}

static int ionic_lifs_register(struct ionic *ionic)
{
	struct list_head *cur;
	struct lif *lif;
	int err;

	list_for_each(cur, &ionic->lifs) {
		lif = list_entry(cur, struct lif, list);
		err = ionic_lif_register(lif);
		if (err)
			return err;
	}

	return 0;
}

static void ionic_lifs_unregister(struct ionic *ionic)
{
	struct list_head *cur;
	struct lif *lif;

	list_for_each(cur, &ionic->lifs) {
		lif = list_entry(cur, struct lif, list);
		if (lif->registered) {
			unregister_netdev(lif->netdev);
			lif->registered = false;
		}
	}
}

static int ionic_lifs_size(struct ionic *ionic)
{
	struct pci_dev *pdev = ionic->pdev;
	union identity *ident = ionic->ident;
	unsigned int nlifs = ident->nlifs;
	unsigned int ntxqs_per_lif = ident->ntxqs_per_lif;
	unsigned int nrxqs_per_lif = ident->nrxqs_per_lif;
	unsigned int nintrs = ident->nintrs;
	int err;

try_again:
	nintrs = nlifs * (ntxqs_per_lif + nrxqs_per_lif + 1 /* adminq */);

	while (nintrs > ident->nintrs)
	{
		if (ntxqs_per_lif-- > 1)
			goto try_again;
		if (nrxqs_per_lif-- > 1)
			goto try_again;
		return -ENOSPC;
	}

	err = pci_alloc_irq_vectors(pdev, nintrs, nintrs, PCI_IRQ_MSIX);
	if (err < 0 && err != -ENOSPC)
		return err;
	if (err == -ENOSPC)
		goto try_again;
	if (err != nintrs) {
		pci_free_irq_vectors(pdev);
		goto try_again;
	}

	ionic->ntxqs_per_lif = ntxqs_per_lif;
	ionic->nrxqs_per_lif = nrxqs_per_lif;
	ionic->nintrs = nintrs;

	return ionic_debugfs_add_sizes(ionic);
}

static int ionic_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct device *dev = &pdev->dev;
	struct ionic *ionic;
	struct ionic_dev *idev;
	int err;

	ionic = devm_kzalloc(dev, sizeof(*ionic), GFP_KERNEL);
	if (!ionic)
		return -ENOMEM;

	ionic->pdev = pdev;
	pci_set_drvdata(pdev, ionic);

	err = ionic_debugfs_add_dev(ionic);
	if (err) {
		dev_err(dev, "Cannot add device debugfs, aborting\n");
		return err;
	}

	/* Setup PCI device
	 */

	err = pci_enable_device_mem(pdev);
	if (err) {
		dev_err(dev, "Cannot enable PCI device, aborting\n");
		goto err_out_debugfs_del_dev;
	}

	err = pci_request_regions(pdev, DRV_NAME);
	if (err) {
		dev_err(dev, "Cannot request PCI regions, aborting\n");
		goto err_out_disable_device;
	}

	pci_set_master(pdev);

	err = ionic_set_dma_mask(pdev, &ionic->using_dac);
	if (err)
		goto err_out_release_regions;

	err = ionic_map_bars(ionic);
	if (err)
		goto err_out_unmap_bars;

	/* Discover ionic dev resources
	 */

	idev = &ionic->idev;

	err = ionic_setup(ionic);
	if (err) {
		dev_err(dev, "Cannot setup device, aborting\n");
		goto err_out_unmap_bars;
	}

	err = ionic_reset(ionic);
	if (err) {
		dev_err(dev, "Cannot reset device, aborting\n");
		goto err_out_unmap_bars;
	}

	err = ionic_identify(ionic);
	if (err) {
		dev_err(dev, "Cannot identify device, aborting\n");
		goto err_out_unmap_bars;
	}

	dev_info(dev, "ASIC %s rev 0x%X serial num %s fw version %s\n",
		 ionic_dev_asic_name(ionic->ident->asic_type),
		 ionic->ident->asic_rev, ionic->ident->serial_num,
		 ionic->ident->fw_version);

	/* Allocate and init LIFs, creating a netdev per LIF
	 */

	err = ionic_lifs_size(ionic);
	if (err) {
		dev_err(dev, "Cannot size LIFs, aborting\n");
		goto err_out_forget_identity;
	}

	err = ionic_lifs_alloc(ionic);
	if (err) {
		dev_err(dev, "Cannot allocate LIFs, aborting\n");
		goto err_out_free_lifs;
	}

	err = ionic_lifs_init(ionic);
	if (err) {
		dev_err(dev, "Cannot init LIFs, aborting\n");
		goto err_out_deinit_lifs;
	}

	err = ionic_lifs_register(ionic);
	if (err) {
		dev_err(dev, "Cannot register LIFs, aborting\n");
		goto err_out_deinit_lifs;
	}

	return 0;

err_out_deinit_lifs:
	ionic_lifs_deinit(ionic);
err_out_free_lifs:
	ionic_lifs_free(ionic);
	pci_free_irq_vectors(pdev);
err_out_forget_identity:
	ionic_forget_identity(ionic);
err_out_unmap_bars:
	ionic_unmap_bars(ionic);
err_out_release_regions:
	pci_release_regions(pdev);
err_out_disable_device:
	pci_disable_device(pdev);
err_out_debugfs_del_dev:
	ionic_debugfs_del_dev(ionic);
	pci_set_drvdata(pdev, NULL);

	return err;
}

static void ionic_remove(struct pci_dev *pdev)
{
	struct ionic *ionic = pci_get_drvdata(pdev);

	if (ionic) {
		ionic_debugfs_del_dev(ionic);
		ionic_lifs_unregister(ionic);
		ionic_lifs_deinit(ionic);
		ionic_lifs_free(ionic);
		pci_free_irq_vectors(pdev);
		ionic_forget_identity(ionic);
		ionic_unmap_bars(ionic);
		pci_release_regions(pdev);
		pci_disable_sriov(pdev);
		pci_disable_device(pdev);
	}
}

static int ionic_sriov_configure(struct pci_dev *pdev, int numvfs)
{
	int err;

	if (numvfs > 0) {
		err = pci_enable_sriov(pdev, numvfs);
		if (err) {
			dev_err(&pdev->dev, "Cannot enable SRIOV, err=%d\n",
				err);
			return err;
		}
	}

	if (numvfs == 0)
		pci_disable_sriov(pdev);

	return numvfs;
}

static struct pci_driver ionic_driver = {
	.name = DRV_NAME,
	.id_table = ionic_id_table,
	.probe = ionic_probe,
	.remove = ionic_remove,
	.sriov_configure = ionic_sriov_configure,
};

static int __init ionic_init_module(void)
{
	ionic_struct_size_checks();
	ionic_debugfs_create();
	pr_info("%s, ver %s\n", DRV_DESCRIPTION, DRV_VERSION);
	return pci_register_driver(&ionic_driver);
}

static void __exit ionic_cleanup_module(void)
{
	pci_unregister_driver(&ionic_driver);
	ionic_debugfs_destroy();
}

module_init(ionic_init_module);
module_exit(ionic_cleanup_module);
