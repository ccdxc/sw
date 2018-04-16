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

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_api.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"
#include "ionic_ethtool.h"
#include "ionic_debugfs.h"

static int ionic_qcq_enable(struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_enable = {
			.opcode = CMD_OPCODE_Q_ENABLE,
			.qid = q->qid,
			.qtype = q->qtype,
		},
	};
	int err;

	netdev_info(lif->netdev, "q_enable.qid %d\n", ctx.cmd.q_enable.qid);
	netdev_info(lif->netdev, "q_enable.qtype %d\n", ctx.cmd.q_enable.qtype);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	napi_enable(&qcq->napi);
	ionic_intr_mask(&qcq->intr, false);

	return 0;
}

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

	netif_carrier_on(netdev);

	return 0;
}

static int ionic_qcq_disable(struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_disable = {
			.opcode = CMD_OPCODE_Q_DISABLE,
			.qid = q->qid,
			.qtype = q->qtype,
		},
	};

	netdev_info(lif->netdev, "q_disable.qid %d\n", ctx.cmd.q_disable.qid);
	netdev_info(lif->netdev, "q_disable.qtype %d\n",
		   ctx.cmd.q_disable.qtype);

	ionic_intr_mask(&qcq->intr, true);
	synchronize_irq(qcq->intr.vector);
	napi_disable(&qcq->napi);

	return ionic_adminq_post_wait(lif, &ctx);
}

static int ionic_stop(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	unsigned int i;
	int err;

	netif_carrier_off(netdev);
	netif_tx_disable(netdev);

	for (i = 0; i < lif->ntxqcqs; i++) {
		// TODO post NOP Tx desc and wait for its completion
		// TODO before disabling Tx queue
		err = ionic_qcq_disable(lif->txqcqs[i]);
		if (err)
			return err;
	}

	for (i = 0; i < lif->nrxqcqs; i++) {
		err = ionic_qcq_disable(lif->rxqcqs[i]);
		if (err)
			return err;
		ionic_rx_flush(&lif->rxqcqs[i]->cq);
	}

	return 0;
}

static bool ionic_adminq_service(struct cq *cq, struct cq_info *cq_info,
				 void *cb_arg)
{
	struct admin_comp *comp = cq_info->cq_desc;

	if (comp->color != cq->done_color)
		return 0;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return true;
}

static int ionic_adminq_napi(struct napi_struct *napi, int budget)
{
	return ionic_napi(napi, budget, ionic_adminq_service, NULL);
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

static int _ionic_lif_addr(struct lif *lif, const u8 *addr, bool add)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter = {
			.opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
					CMD_OPCODE_RX_FILTER_DEL,
			.match = RX_FILTER_MATCH_MAC,
		},
	};

	memcpy(ctx.cmd.rx_filter.addr, addr, ETH_ALEN);

	netdev_info(lif->netdev, "rx_filter %s %pM\n",
		   add ? "add" : "del", addr);

	return ionic_adminq_post_wait(lif, &ctx);
}

struct lif_addr_work {
	struct work_struct work;
	struct lif *lif;
	u8 addr[ETH_ALEN];
	bool add;
};

static void ionic_lif_addr_work(struct work_struct *work)
{
	struct lif_addr_work *w  = container_of(work, struct lif_addr_work,
						work);

	_ionic_lif_addr(w->lif, w->addr, w->add);
	kfree(w);
}

static int ionic_lif_addr(struct lif *lif, const u8 *addr, bool add)
{
	struct lif_addr_work *work;

	if (in_interrupt()) {
		work = kmalloc(sizeof(*work), GFP_ATOMIC);
		if (!work) {
			netdev_err(lif->netdev, "%s OOM\n", __func__);
			return -ENOMEM;
		}
		INIT_WORK(&work->work, ionic_lif_addr_work);
		work->lif = lif;
		memcpy(work->addr, addr, ETH_ALEN);
		work->add = add;
		netdev_info(lif->netdev, "deferred: rx_filter %s %pM\n",
			   add ? "add" : "del", addr);
		queue_work(lif->adminq_wq, &work->work);
	} else {
		return _ionic_lif_addr(lif, addr, add);
	}

	return 0;
}

static int ionic_addr_add(struct net_device *netdev, const u8 *addr)
{
	return ionic_lif_addr(netdev_priv(netdev), addr, true);
}

static int ionic_addr_del(struct net_device *netdev, const u8 *addr)
{
	return ionic_lif_addr(netdev_priv(netdev), addr, false);
}

static void _ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_mode_set = {
			.opcode = CMD_OPCODE_RX_MODE_SET,
			.rx_mode = rx_mode,
		},
	};
	int err;

	if (rx_mode & RX_MODE_F_UNICAST)
		netdev_info(lif->netdev, "rx_mode RX_MODE_F_UNICAST\n");
	if (rx_mode & RX_MODE_F_MULTICAST)
		netdev_info(lif->netdev, "rx_mode RX_MODE_F_MULTICAST\n");
	if (rx_mode & RX_MODE_F_BROADCAST)
		netdev_info(lif->netdev, "rx_mode RX_MODE_F_BROADCAST\n");
	if (rx_mode & RX_MODE_F_PROMISC)
		netdev_info(lif->netdev, "rx_mode RX_MODE_F_PROMISC\n");
	if (rx_mode & RX_MODE_F_ALLMULTI)
		netdev_info(lif->netdev, "rx_mode RX_MODE_F_ALLMULTI\n");

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		// XXX handle err
	}
}

struct rx_mode_work {
	struct work_struct work;
	struct lif *lif;
	unsigned int rx_mode;
};

static void ionic_lif_rx_mode_work(struct work_struct *work)
{
	struct rx_mode_work *w  = container_of(work, struct rx_mode_work, work);
	
	_ionic_lif_rx_mode(w->lif, w->rx_mode);
	kfree(w);
}

static void ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode)
{
	struct rx_mode_work *work;

	if (in_interrupt()) {
		work = kmalloc(sizeof(*work), GFP_ATOMIC);
		if (!work) {
			netdev_err(lif->netdev, "%s OOM\n", __func__);
			return;
		}
		INIT_WORK(&work->work, ionic_lif_rx_mode_work);
		work->lif = lif;
		work->rx_mode = rx_mode;
		netdev_info(lif->netdev, "deferred: rx_mode\n");
		queue_work(lif->adminq_wq, &work->work);
	} else {
		_ionic_lif_rx_mode(lif, rx_mode);
	}
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

	if (netdev_uc_count(netdev) + 1 > ident->dev.nucasts_per_lif)
		rx_mode |= RX_MODE_F_PROMISC;
	if (netdev_mc_count(netdev) > ident->dev.nmcasts_per_lif)
		rx_mode |= RX_MODE_F_ALLMULTI;

	if (lif->rx_mode != rx_mode) {
		lif->rx_mode = rx_mode;
		ionic_lif_rx_mode(lif, rx_mode);
	}

	__dev_uc_sync(netdev, ionic_addr_add, ionic_addr_del);
	__dev_mc_sync(netdev, ionic_addr_add, ionic_addr_del);
}

static int ionic_set_mac_address(struct net_device *netdev, void *addr)
{
	// TODO implement
	netdev_err(netdev, "%s SET MAC ADDRESS not implemented\n",
		   netdev->name);
	return 0;
}

static int ionic_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.mtu_set = {
			.opcode = CMD_OPCODE_MTU_SET,
			.mtu = new_mtu,
		},
	};
	unsigned int i;
	int err;

	if (netif_running(netdev))
		ionic_stop(netdev);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	netdev->mtu = new_mtu;

	for (i = 0; i < lif->nrxqcqs; i++)
		ionic_rx_refill(&lif->rxqcqs[i]->q);

	if (netif_running(netdev))
		ionic_open(netdev);

	return 0;
}

static void ionic_tx_timeout(struct net_device *netdev)
{
	// TODO implement
}

static int ionic_vlan_rx_filter(struct net_device *netdev, bool add,
				__be16 proto, u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter = {
			.opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
					CMD_OPCODE_RX_FILTER_DEL,
			.match = RX_FILTER_MATCH_VLAN,
			.vlan = vid,
		},
	};

	netdev_info(netdev, "rx_filter %s VLAN %d\n", add ? "add" : "del", vid);

	return ionic_adminq_post_wait(lif, &ctx);
}

static int ionic_vlan_rx_add_vid(struct net_device *netdev,
				 __be16 proto, u16 vid)
{
	return ionic_vlan_rx_filter(netdev, true, proto, vid);
}

static int ionic_vlan_rx_kill_vid(struct net_device *netdev,
				  __be16 proto, u16 vid)
{
	return ionic_vlan_rx_filter(netdev, false, proto, vid);
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

int ionic_intr_alloc(struct lif *lif, struct intr *intr)
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

void ionic_intr_free(struct lif *lif, struct intr *intr)
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
	struct ionic_dev *idev = &lif->ionic->idev;
	struct device *dev = lif->ionic->dev;
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

	err = ionic_q_init(lif, idev, &new->q, index, base, num_descs,
			   desc_size, sg_desc_size, pid);
	if (err)
		return err;

	if (flags & QCQ_F_INTR) {
		err = ionic_intr_alloc(lif, &new->intr);
		if (err)
			return err;
		err = ionic_bus_get_irq(lif->ionic, new->intr.index);
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

	new->base = dma_alloc_coherent(dev, total_size, &new->base_pa,
				       GFP_KERNEL);
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

	dma_free_coherent(lif->ionic->dev, qcq->total_size, qcq->base,
			  qcq->base_pa);
	ionic_intr_free(lif, &qcq->intr);
}

static unsigned int ionic_pid_get(struct lif *lif, unsigned int page)
{
	unsigned int ndbpgs_per_lif = lif->ionic->ident->dev.ndbpgs_per_lif;

	BUG_ON(ndbpgs_per_lif < page + 1);

	return lif->index * ndbpgs_per_lif + page;
}

static int ionic_qcqs_alloc(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
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
	struct device *dev = ionic->dev;
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
	lif->neqs = ionic->neqs_per_lif;
	lif->ntxqcqs = ionic->ntxqs_per_lif;
	lif->nrxqcqs = ionic->nrxqs_per_lif;

	snprintf(lif->name, sizeof(lif->name), "lif%u", index);

	spin_lock_init(&lif->adminq_lock);
	lif->adminq_wq = create_workqueue(lif->name);

	netdev->netdev_ops = &ionic_netdev_ops;
	ionic_ethtool_set_ops(netdev);
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

int ionic_lifs_alloc(struct ionic *ionic)
{
	unsigned int i;
	int err;

	INIT_LIST_HEAD(&ionic->lifs);

	for (i = 0; i < ionic->ident->dev.nlifs; i++) {
		err = ionic_lif_alloc(ionic, i);
		if (err)
			return err;
	}

	return 0;
}

void ionic_lifs_free(struct ionic *ionic)
{
	struct list_head *cur, *tmp;
	struct lif *lif;

	list_for_each_safe(cur, tmp, &ionic->lifs) {
		lif = list_entry(cur, struct lif, list);
		list_del(&lif->list);
		flush_workqueue(lif->adminq_wq);
		destroy_workqueue(lif->adminq_wq);
		ionic_qcqs_free(lif);
		free_netdev(lif->netdev);
	}
}

static int ionic_lif_stats_dump_start(struct lif *lif, unsigned int ver)
{
	struct net_device *netdev = lif->netdev;
	struct device *dev = lif->ionic->dev;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.stats_dump = {
			.opcode = CMD_OPCODE_STATS_DUMP_START,
			.ver = ver,
		},
	};
#ifndef HAPS
	int err;
#endif

	lif->stats_dump = dma_alloc_coherent(dev, sizeof(*lif->stats_dump),
					     &lif->stats_dump_pa, GFP_KERNEL);

	if (!lif->stats_dump) {
		netdev_err(netdev, "%s OOM\n", __func__);
		return -ENOMEM;
	}

	ctx.cmd.stats_dump.addr = lif->stats_dump_pa;

	netdev_info(netdev, "stats_dump START ver %d addr 0x%llx\n", ver,
		    lif->stats_dump_pa);

#ifdef HAPS
	return 0;
#else
	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		goto err_out_free;

	return 0;

err_out_free:
	dma_free_coherent(dev, sizeof(*lif->stats_dump), lif->stats_dump,
			  lif->stats_dump_pa);
	return err;
#endif
}

static void ionic_lif_stats_dump_stop(struct lif *lif)
{
#ifndef HAPS
	struct net_device *netdev = lif->netdev;
#endif
	struct device *dev = lif->ionic->dev;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.stats_dump = {
			.opcode = CMD_OPCODE_STATS_DUMP_STOP,
		},
	};
#ifndef HAPS
	int err;

	netdev_info(netdev, "stats_dump STOP\n");

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		netdev_err(netdev, "stats_dump cmd failed %d\n", err);
		return;
	}
#endif

	dma_free_coherent(dev, sizeof(*lif->stats_dump), lif->stats_dump,
			  lif->stats_dump_pa);
}

static int ionic_lif_rss_setup(struct lif *lif)
{
	struct net_device *netdev = lif->netdev;
	struct device *dev = lif->ionic->dev;
	size_t tbl_size = sizeof(*lif->rss_ind_tbl) * RSS_IND_TBL_SIZE;
	static const u8 toeplitz_symmetric_key[] = {
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
	};
	unsigned int i;
	int err;

	lif->rss_ind_tbl = dma_alloc_coherent(dev, tbl_size,
					      &lif->rss_ind_tbl_pa,
					      GFP_KERNEL);

	if (!lif->rss_ind_tbl) {
		netdev_err(netdev, "%s OOM\n", __func__);
		return -ENOMEM;
	}

	/* Fill indirection table with 'default' values */

	for (i = 0; i < RSS_IND_TBL_SIZE; i++)
		lif->rss_ind_tbl[i] = i % lif->nrxqcqs;

	err = ionic_rss_ind_tbl_set(lif, NULL);
	if (err)
		goto err_out_free;

	err = ionic_rss_hash_key_set(lif, toeplitz_symmetric_key);
	if (err)
		goto err_out_free;

	return 0;

err_out_free:
	dma_free_coherent(dev, tbl_size, lif->rss_ind_tbl,
			  lif->rss_ind_tbl_pa);
	return err;
}

static void ionic_lif_rss_teardown(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
	size_t tbl_size = sizeof(*lif->rss_ind_tbl) * RSS_IND_TBL_SIZE;

	dma_free_coherent(dev, tbl_size, lif->rss_ind_tbl,
			  lif->rss_ind_tbl_pa);
}

static void ionic_lif_qcq_deinit(struct lif *lif, struct qcq *qcq)
{
	struct device *dev = lif->ionic->dev;

	if (!(qcq->flags & QCQ_F_INITED))
		return;
	ionic_intr_mask(&qcq->intr, true);
	devm_free_irq(dev, qcq->intr.vector, &qcq->napi);
	netif_napi_del(&qcq->napi);
	qcq->flags &= ~QCQ_F_INITED;
}

static void ionic_lif_txqs_deinit(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_lif_qcq_deinit(lif, lif->txqcqs[i]);
}

static void ionic_lif_rxqs_deinit(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < lif->nrxqcqs; i++)
		ionic_lif_qcq_deinit(lif, lif->rxqcqs[i]);
}

static void ionic_lif_deinit(struct lif *lif)
{
	ionic_lif_stats_dump_stop(lif);
	ionic_lif_rss_teardown(lif);
	ionic_lif_qcq_deinit(lif, lif->adminqcq);
	ionic_lif_txqs_deinit(lif);
	ionic_lif_rxqs_deinit(lif);
}

void ionic_lifs_deinit(struct ionic *ionic)
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
	struct device *dev = lif->ionic->dev;
	struct intr *intr = &qcq->intr;
	struct queue *q = &qcq->q;
	struct napi_struct *napi = &qcq->napi;

	snprintf(intr->name, sizeof(intr->name),
		 "%s-%s-%s", DRV_NAME, lif->name, q->name);
	return devm_request_irq(dev, intr->vector, ionic_isr,
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
	err = ionic_dev_cmd_wait_check(idev, IONIC_DEVCMD_TIMEOUT);
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

	qcq->flags |= QCQ_F_INITED;

	return 0;
}

static int ionic_lif_txq_init(struct lif *lif, struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct cq *cq = &qcq->cq;
	struct napi_struct *napi = &qcq->napi;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.txq_init = {
			.opcode = CMD_OPCODE_TXQ_INIT,
			.I = false,
			.E = false,
			.pid = q->pid,
			.intr_index = cq->bound_intr->index,
			.type = TXQ_TYPE_ETHERNET,
			.index = q->index,
			.cos = 0,
			.ring_base = q->base_pa,
			.ring_size = ilog2(q->num_descs),
		},
	};
	int err;

	netdev_info(lif->netdev, "txq_init.pid %d\n", ctx.cmd.txq_init.pid);
	netdev_info(lif->netdev, "txq_init.index %d\n", ctx.cmd.txq_init.index);
	netdev_info(lif->netdev, "txq_init.ring_base 0x%llx\n",
	           ctx.cmd.txq_init.ring_base);
	netdev_info(lif->netdev, "txq_init.ring_size %d\n",
		   ctx.cmd.txq_init.ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	q->qid = ctx.comp.txq_init.qid;
	q->qtype = ctx.comp.txq_init.qtype;
	q->db = ionic_db_map(q->idev, q);

	netif_napi_add(lif->netdev, napi, ionic_tx_napi,
		       NAPI_POLL_WEIGHT);

	err = ionic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	netdev_info(lif->netdev, "txq->qid %d\n", q->qid);
	netdev_info(lif->netdev, "txq->qtype %d\n", q->qtype);
	netdev_info(lif->netdev, "txq->db %p\n", q->db);

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
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rxq_init = {
			.opcode = CMD_OPCODE_RXQ_INIT,
			.I = false,
			.E = false,
			.pid = q->pid,
			.intr_index = cq->bound_intr->index,
			.type = RXQ_TYPE_ETHERNET,
			.index = q->index,
			.ring_base = q->base_pa,
			.ring_size = ilog2(q->num_descs),
		},
	};
	int err;

	netdev_info(lif->netdev, "rxq_init.pid %d\n", ctx.cmd.rxq_init.pid);
	netdev_info(lif->netdev, "rxq_init.index %d\n", ctx.cmd.rxq_init.index);
	netdev_info(lif->netdev, "rxq_init.ring_base 0x%llx\n",
		   ctx.cmd.rxq_init.ring_base);
	netdev_info(lif->netdev, "rxq_init.ring_size %d\n",
		   ctx.cmd.rxq_init.ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	q->qid = ctx.comp.rxq_init.qid;
	q->qtype = ctx.comp.rxq_init.qtype;
	q->db = ionic_db_map(q->idev, q);

	netif_napi_add(lif->netdev, napi, ionic_rx_napi,
		       NAPI_POLL_WEIGHT);

	err = ionic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	netdev_info(lif->netdev, "rxq->qid %d\n", q->qid);
	netdev_info(lif->netdev, "rxq->qtype %d\n", q->qtype);
	netdev_info(lif->netdev, "rxq->db %p\n", q->db);

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

static int ionic_station_set(struct lif *lif)
{
	struct net_device *netdev = lif->netdev;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.station_mac_addr_get = {
			.opcode = CMD_OPCODE_STATION_MAC_ADDR_GET,
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	if (!is_zero_ether_addr(netdev->dev_addr)) {
		netdev_info(lif->netdev, "deleting station MAC addr %pM\n",
			   netdev->dev_addr);
		ionic_lif_addr(lif, netdev->dev_addr, false);
	}
	memcpy(netdev->dev_addr, ctx.comp.station_mac_addr_get.addr,
	       netdev->addr_len);
	netdev_info(lif->netdev, "adding station MAC addr %pM\n",
		   netdev->dev_addr);
	ionic_lif_addr(lif, netdev->dev_addr, true);

	return 0;
}

static int ionic_lif_init(struct lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	int err;

	ionic_dev_cmd_lif_init(idev, lif->index);
	err = ionic_dev_cmd_wait_check(idev, IONIC_DEVCMD_TIMEOUT);
	if (err)
		return err;

	err = ionic_lif_adminq_init(lif);
	if (err)
		return err;

	/* Enabling interrupts on adminq from here on... */
	ionic_intr_mask(&lif->adminqcq->intr, false);

	err = ionic_lif_txqs_init(lif);
	if (err)
		goto err_out_mask_adminq;

	err = ionic_lif_rxqs_init(lif);
	if (err)
		goto err_out_txqs_deinit;

	err = ionic_station_set(lif);
	if (err)
		goto err_out_rxqs_deinit;

	err = ionic_lif_rss_setup(lif);
	if (err)
		goto err_out_rxqs_deinit;

	err = ionic_lif_stats_dump_start(lif, STATS_DUMP_VERSION_1);
	if (err)
		goto err_out_rss_teardown;

	ionic_set_rx_mode(lif->netdev);

	lif->api_private = NULL;

	return 0;

err_out_rss_teardown:
	ionic_lif_rss_teardown(lif);
err_out_rxqs_deinit:
	ionic_lif_rxqs_deinit(lif);
err_out_txqs_deinit:
	ionic_lif_txqs_deinit(lif);
err_out_mask_adminq:
	ionic_intr_mask(&lif->adminqcq->intr, true);

	return err;
}

int ionic_lifs_init(struct ionic *ionic)
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

static int ionic_get_features(struct lif *lif)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.features = {
			.opcode = CMD_OPCODE_FEATURES,
			.set = FEATURE_SET_ETH_HW_FEATURES,
			.wanted = ETH_HW_VLAN_TX_TAG
				| ETH_HW_VLAN_RX_STRIP
				| ETH_HW_VLAN_RX_FILTER
				| ETH_HW_RX_HASH
				| ETH_HW_TX_SG
				| ETH_HW_TX_CSUM
				| ETH_HW_RX_CSUM,
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	lif->hw_features = ctx.cmd.features.wanted &
			   ctx.comp.features.supported;

	if (lif->hw_features & ETH_HW_VLAN_TX_TAG)
		netdev_info(lif->netdev, "feature ETH_HW_VLAN_TX_TAG\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_STRIP)
		netdev_info(lif->netdev, "feature ETH_HW_VLAN_RX_STRIP\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_FILTER)
		netdev_info(lif->netdev, "feature ETH_HW_VLAN_RX_FILTER\n");
	if (lif->hw_features & ETH_HW_RX_HASH)
		netdev_info(lif->netdev, "feature ETH_HW_RX_HASH\n");
	if (lif->hw_features & ETH_HW_TX_SG)
		netdev_info(lif->netdev, "feature ETH_HW_TX_SG\n");
	if (lif->hw_features & ETH_HW_TX_CSUM)
		netdev_info(lif->netdev, "feature ETH_HW_TX_CSUM\n");
	if (lif->hw_features & ETH_HW_RX_CSUM)
		netdev_info(lif->netdev, "feature ETH_HW_RX_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO)
		netdev_info(lif->netdev, "feature ETH_HW_TSO\n");
	if (lif->hw_features & ETH_HW_TSO_IPV6)
		netdev_info(lif->netdev, "feature ETH_HW_TSO_IPV6\n");
	if (lif->hw_features & ETH_HW_TSO_ECN)
		netdev_info(lif->netdev, "feature ETH_HW_TSO_ECN\n");
	if (lif->hw_features & ETH_HW_TSO_GRE)
		netdev_info(lif->netdev, "feature ETH_HW_TSO_GRE\n");
	if (lif->hw_features & ETH_HW_TSO_GRE_CSUM)
		netdev_info(lif->netdev, "feature ETH_HW_TSO_GRE_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP4)
		netdev_info(lif->netdev, "feature ETH_HW_TSO_IPXIP4\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP6)
		netdev_info(lif->netdev, "feature ETH_HW_TSO_IPXIP6\n");
	if (lif->hw_features & ETH_HW_TSO_UDP)
		netdev_info(lif->netdev, "feature ETH_HW_TSO_UDP\n");
	if (lif->hw_features & ETH_HW_TSO_UDP_CSUM)
		netdev_info(lif->netdev, "feature ETH_HW_TSO_UDP_CSUM\n");

	return 0;
}

static int ionic_set_features(struct lif *lif)
{
	struct net_device *netdev = lif->netdev;
	int err;

	err = ionic_get_features(lif);
	if (err)
		return err;

	netdev->features |= NETIF_F_HIGHDMA;

	if (lif->hw_features & ETH_HW_VLAN_TX_TAG)
		netdev->hw_features |= NETIF_F_HW_VLAN_CTAG_TX;
	if (lif->hw_features & ETH_HW_VLAN_RX_STRIP)
		netdev->hw_features |= NETIF_F_HW_VLAN_CTAG_RX;
	if (lif->hw_features & ETH_HW_VLAN_RX_FILTER)
		netdev->hw_features |= NETIF_F_HW_VLAN_CTAG_FILTER;
	if (lif->hw_features & ETH_HW_RX_HASH)
		netdev->hw_features |= NETIF_F_RXHASH;
	if (lif->hw_features & ETH_HW_TX_SG)
		netdev->hw_features |= NETIF_F_SG;

	if (lif->hw_features & ETH_HW_TX_CSUM)
		netdev->hw_enc_features |= NETIF_F_HW_CSUM;
	if (lif->hw_features & ETH_HW_RX_CSUM)
		netdev->hw_enc_features |= NETIF_F_RXCSUM;
	if (lif->hw_features & ETH_HW_TSO)
		netdev->hw_enc_features |= NETIF_F_TSO;
	if (lif->hw_features & ETH_HW_TSO_IPV6)
		netdev->hw_enc_features |= NETIF_F_TSO6;
	if (lif->hw_features & ETH_HW_TSO_ECN)
		netdev->hw_enc_features |= NETIF_F_TSO_ECN;
	if (lif->hw_features & ETH_HW_TSO_GRE)
		netdev->hw_enc_features |= NETIF_F_GSO_GRE;
	if (lif->hw_features & ETH_HW_TSO_GRE_CSUM)
		netdev->hw_enc_features |= NETIF_F_GSO_GRE_CSUM;
	if (lif->hw_features & ETH_HW_TSO_IPXIP4)
		netdev->hw_enc_features |= NETIF_F_GSO_IPXIP4;
	if (lif->hw_features & ETH_HW_TSO_IPXIP6)
		netdev->hw_enc_features |= NETIF_F_GSO_IPXIP6;
	if (lif->hw_features & ETH_HW_TSO_UDP)
		netdev->hw_enc_features |= NETIF_F_GSO_UDP_TUNNEL;
	if (lif->hw_features & ETH_HW_TSO_UDP_CSUM)
		netdev->hw_enc_features |= NETIF_F_GSO_UDP_TUNNEL_CSUM;

	netdev->hw_features |= netdev->hw_enc_features;
	netdev->features |= netdev->hw_features;
	netdev->vlan_features |= netdev->features;

	return 0;
}

int ionic_lif_register(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
	int err;

	err = ionic_set_features(lif);
	if (err)
		return err;

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

int ionic_lifs_register(struct ionic *ionic)
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

void ionic_lifs_unregister(struct ionic *ionic)
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

int ionic_lifs_size(struct ionic *ionic)
{
	union identity *ident = ionic->ident;
	unsigned int nlifs = ident->dev.nlifs;
	unsigned int neqs_per_lif = ident->dev.neqs_per_lif;
	unsigned int ntxqs_per_lif = ident->dev.ntxqs_per_lif;
	unsigned int nrxqs_per_lif = ident->dev.nrxqs_per_lif;
	unsigned int nintrs, dev_nintrs = ident->dev.nintrs;
	int err;

try_again:
	nintrs = nlifs * (neqs_per_lif +
			  ntxqs_per_lif +
			  nrxqs_per_lif +
			  1 /* adminq */);

	if (nintrs > dev_nintrs)
		goto try_fewer;

	err = ionic_bus_alloc_irq_vectors(ionic, nintrs);
	if (err < 0 && err != -ENOSPC)
		return err;
	if (err == -ENOSPC)
		goto try_fewer;
	if (err != nintrs) {
		ionic_bus_free_irq_vectors(ionic);
		goto try_fewer;
	}

	ionic->neqs_per_lif = neqs_per_lif;
	ionic->ntxqs_per_lif = ntxqs_per_lif;
	ionic->nrxqs_per_lif = nrxqs_per_lif;
	ionic->nintrs = nintrs;

	return ionic_debugfs_add_sizes(ionic);

try_fewer:
	if (neqs_per_lif > 1) {
		--neqs_per_lif;
		goto try_again;
	}
	if (ntxqs_per_lif > 1) {
		--ntxqs_per_lif;
		goto try_again;
	}
	if (nrxqs_per_lif > 1) {
		--nrxqs_per_lif;
		goto try_again;
	}
	return -ENOSPC;
}
