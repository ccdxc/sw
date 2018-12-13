// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/pci.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_api.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"
#include "ionic_ethtool.h"
#include "ionic_debugfs.h"

static void ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode);
static int ionic_lif_addr_add(struct lif *lif, const u8 *addr);
static int ionic_lif_addr_del(struct lif *lif, const u8 *addr);

static void ionic_lif_deferred_work(struct work_struct *work)
{
	struct lif *lif = container_of(work, struct lif, deferred.work);
	struct deferred *def = &lif->deferred;
	struct deferred_work *w = NULL;

	spin_lock_bh(&def->lock);
	if (!list_empty(&def->list)) {
		w = list_first_entry(&def->list, struct deferred_work, list);
		list_del(&w->list);
	}
	spin_unlock_bh(&def->lock);

	if (w) {
		switch (w->type) {
		case DW_TYPE_RX_MODE:
			ionic_lif_rx_mode(lif, w->rx_mode);
			break;
		case DW_TYPE_RX_ADDR_ADD:
			ionic_lif_addr_add(lif, w->addr);
			break;
		case DW_TYPE_RX_ADDR_DEL:
			ionic_lif_addr_del(lif, w->addr);
			break;
		};
		kfree(w);
		schedule_work(&def->work);
	}
}

static void ionic_lif_deferred_enqueue(struct deferred *def,
				       struct deferred_work *work)
{
	spin_lock_bh(&def->lock);
	list_add_tail(&work->list, &def->list);
	spin_unlock_bh(&def->lock);
	schedule_work(&def->work);
}

static int ionic_qcq_enable(struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
	struct device *dev = lif->ionic->dev;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_enable = {
			.opcode = CMD_OPCODE_Q_ENABLE,
			.qid = q->qid,
			.qtype = q->qtype,
		},
	};
	int err;

	dev_dbg(dev, "q_enable.qid %d q_enable.qtype %d\n",
		ctx.cmd.q_enable.qid, ctx.cmd.q_enable.qtype);

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
	struct device *dev = lif->ionic->dev;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_disable = {
			.opcode = CMD_OPCODE_Q_DISABLE,
			.qid = q->qid,
			.qtype = q->qtype,
		},
	};

	dev_dbg(dev, "q_disable.qid %d q_disable.qtype %d\n",
		ctx.cmd.q_disable.qid, ctx.cmd.q_disable.qtype);

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
		return false;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return true;
}

static int ionic_adminq_napi(struct napi_struct *napi, int budget)
{
	return ionic_napi(napi, budget, ionic_adminq_service, NULL);
}

static void ionic_get_stats64(struct net_device *netdev,
			      struct rtnl_link_stats64 *ns)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_lif_stats *ls = lif->lif_stats;
	struct device *dev = lif->ionic->dev;
	unsigned int i;
	u64 cnt;

	/* use locally counted numbers for byte and packet counts */
	for (i = 0; i < lif->ntxqcqs; i++) {
		struct tx_stats *tx_stats = &lif->txqcqs[i]->stats.tx;

		ns->tx_packets += tx_stats->pkts;
		ns->tx_bytes += tx_stats->bytes;
	}

	for (i = 0; i < lif->nrxqcqs; i++) {
		struct rx_stats *rx_stats = &lif->rxqcqs[i]->stats.rx;

		ns->rx_packets += rx_stats->pkts;
		ns->rx_bytes += rx_stats->bytes;
	}

	/* double check stats counters against hw counts */
	cnt = ls->rx_ucast_packets + ls->rx_mcast_packets + ls->rx_bcast_packets;
	if (ns->rx_packets != cnt)
		dev_warn(dev, "rx_packets mismatch: sw=%llu nic=%llu\n",
			 ns->rx_packets, cnt);

	cnt = ls->tx_ucast_packets + ls->tx_mcast_packets + ls->tx_bcast_packets;
	if (ns->tx_packets != cnt)
		dev_warn(dev, "tx_packets mismatch: sw=%llu nic=%llu\n",
			 ns->tx_packets, cnt);

	cnt = ls->rx_ucast_bytes + ls->rx_mcast_bytes + ls->rx_bcast_bytes;
	if (ns->rx_bytes != cnt)
		dev_warn(dev, "rx_bytes mismatch: sw=%llu nic=%llu\n",
			 ns->rx_bytes, cnt);

	cnt = ls->tx_ucast_bytes + ls->tx_mcast_bytes + ls->tx_bcast_bytes;
	if (ns->tx_bytes != cnt)
		dev_warn(dev, "tx_bytes mismatch: sw=%llu nic=%llu\n",
			 ns->tx_bytes, cnt);

	/* report hw error counters */
	ns->rx_dropped = ls->rx_ucast_drop_packets +
			 ls->rx_mcast_drop_packets +
			 ls->rx_bcast_drop_packets;

	ns->tx_dropped = ls->tx_ucast_drop_packets +
			 ls->tx_mcast_drop_packets +
			 ls->tx_bcast_drop_packets;

	ns->multicast = ls->rx_mcast_packets;

	ns->rx_over_errors = ls->rx_queue_empty_drop;

	ns->rx_missed_errors = ls->rx_dma_error +
			       ls->rx_queue_disabled_drop +
			       ls->rx_queue_scheduled +
			       ls->rx_desc_fetch_error +
			       ls->rx_desc_data_error;

	ns->tx_aborted_errors = ls->tx_dma_error +
				ls->tx_queue_disabled +
				ls->tx_queue_scheduled +
				ls->tx_desc_fetch_error +
				ls->tx_desc_data_error;

	ns->rx_errors = ns->rx_over_errors +
			ns->rx_missed_errors;

	ns->tx_errors = ns->tx_aborted_errors;
}

static int ionic_lif_addr_add(struct lif *lif, const u8 *addr)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_add = {
			.opcode = CMD_OPCODE_RX_FILTER_ADD,
			.match = RX_FILTER_MATCH_MAC,
		},
	};
	int err;

	memcpy(ctx.cmd.rx_filter_add.mac.addr, addr, ETH_ALEN);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	netdev_info(lif->netdev, "rx_filter add ADDR %pM (id %d)\n", addr,
		    ctx.comp.rx_filter_add.filter_id);

	return ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
}

static int ionic_lif_addr_del(struct lif *lif, const u8 *addr)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_del = {
			.opcode = CMD_OPCODE_RX_FILTER_DEL,
		},
	};
	struct rx_filter *f;
	int err;

	spin_lock_bh(&lif->rx_filters.lock);

	f = ionic_rx_filter_by_addr(lif, addr);
	if (!f) {
		spin_unlock_bh(&lif->rx_filters.lock);
		return -ENOENT;
	}

	ctx.cmd.rx_filter_del.filter_id = f->filter_id;
	ionic_rx_filter_free(lif, f);
	spin_unlock_bh(&lif->rx_filters.lock);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	netdev_info(lif->netdev, "rx_filter del ADDR %pM (id %d)\n", addr,
		    ctx.cmd.rx_filter_del.filter_id);

	return 0;
}

static int ionic_lif_addr(struct lif *lif, const u8 *addr, bool add)
{
	struct deferred_work *work;

	if (in_interrupt()) {
		work = kzalloc(sizeof(*work), GFP_ATOMIC);
		if (!work) {
			netdev_err(lif->netdev, "%s OOM\n", __func__);
			return -ENOMEM;
		}
		work->type = add ? DW_TYPE_RX_ADDR_ADD : DW_TYPE_RX_ADDR_DEL;
		memcpy(work->addr, addr, ETH_ALEN);
		netdev_info(lif->netdev, "deferred: rx_filter %s %pM\n",
			   add ? "add" : "del", addr);
		ionic_lif_deferred_enqueue(&lif->deferred, work);
	} else {
		if (add)
			return ionic_lif_addr_add(lif, addr);
		else
			return ionic_lif_addr_del(lif, addr);
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

static void ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_mode_set = {
			.opcode = CMD_OPCODE_RX_MODE_SET,
			.rx_mode = rx_mode,
		},
	};
	char buf[128];
	int err;
	int i;
#define REMAIN(__x) (sizeof(buf) - (__x))

	i = snprintf(buf, sizeof(buf), "rx_mode:");
	if (rx_mode & RX_MODE_F_UNICAST)
		i += snprintf(&buf[i], REMAIN(i), " RX_MODE_F_UNICAST");
	if (rx_mode & RX_MODE_F_MULTICAST)
		i += snprintf(&buf[i], REMAIN(i), " RX_MODE_F_MULTICAST");
	if (rx_mode & RX_MODE_F_BROADCAST)
		i += snprintf(&buf[i], REMAIN(i), " RX_MODE_F_BROADCAST");
	if (rx_mode & RX_MODE_F_PROMISC)
		i += snprintf(&buf[i], REMAIN(i), " RX_MODE_F_PROMISC");
	if (rx_mode & RX_MODE_F_ALLMULTI)
		i += snprintf(&buf[i], REMAIN(i), " RX_MODE_F_ALLMULTI");
	netdev_info(lif->netdev, "%s\n", buf);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		netdev_warn(lif->netdev, "set rx_mode 0x%04x failed: %d\n",
			    rx_mode, err);
}

static void _ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode)
{
	struct deferred_work *work;

	if (in_interrupt()) {
		work = kzalloc(sizeof(*work), GFP_ATOMIC);
		if (!work) {
			netdev_err(lif->netdev, "%s OOM\n", __func__);
			return;
		}
		work->type = DW_TYPE_RX_MODE;
		work->rx_mode = rx_mode;
		netdev_info(lif->netdev, "deferred: rx_mode\n");
		ionic_lif_deferred_enqueue(&lif->deferred, work);
	} else {
		ionic_lif_rx_mode(lif, rx_mode);
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
		_ionic_lif_rx_mode(lif, rx_mode);
	}

	__dev_uc_sync(netdev, ionic_addr_add, ionic_addr_del);
	__dev_mc_sync(netdev, ionic_addr_add, ionic_addr_del);
}

static int ionic_set_mac_address(struct net_device *netdev, void *sa)
{
	struct sockaddr *addr = sa;
	u8 *mac = (u8*)addr->sa_data;

	if (ether_addr_equal(netdev->dev_addr, mac))
		return 0;

	if (!is_valid_ether_addr(mac))
		return -EADDRNOTAVAIL;

	if (!is_zero_ether_addr(netdev->dev_addr)) {
		netdev_info(netdev, "deleting mac addr %pM\n",
			   netdev->dev_addr);
		ionic_addr_del(netdev, netdev->dev_addr);
	}

	memcpy(netdev->dev_addr, mac, netdev->addr_len);
	netdev_info(netdev, "updating mac addr %pM\n", mac);

	return ionic_addr_add(netdev, mac);
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
	struct lif *lif = netdev_priv(netdev);
	struct ionic_dev *idev = &lif->ionic->idev;

	ionic_dev_cmd_hang_notify(idev);
	ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);

	// TODO implement reset and re-init queues and so on
	// TODO to get interface back on its feet
}

static int ionic_vlan_rx_add_vid(struct net_device *netdev, __be16 proto,
				 u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_add = {
			.opcode = CMD_OPCODE_RX_FILTER_ADD,
			.match = RX_FILTER_MATCH_VLAN,
			.vlan.vlan = vid,
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	netdev_info(netdev, "rx_filter add VLAN %d (id %d)\n", vid,
		    ctx.comp.rx_filter_add.filter_id);

	return ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
}

static int ionic_vlan_rx_kill_vid(struct net_device *netdev, __be16 proto,
				  u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_del = {
			.opcode = CMD_OPCODE_RX_FILTER_DEL,
		},
	};
	struct rx_filter *f;
	int err;

	spin_lock_bh(&lif->rx_filters.lock);

	f = ionic_rx_filter_by_vlan(lif, vid);
	if (!f) {
		spin_unlock_bh(&lif->rx_filters.lock);
		return -ENOENT;
	}

	ctx.cmd.rx_filter_del.filter_id = f->filter_id;
	ionic_rx_filter_free(lif, f);
	spin_unlock_bh(&lif->rx_filters.lock);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	netdev_info(netdev, "rx_filter del VLAN %d (id %d)\n", vid,
		    ctx.cmd.rx_filter_del.filter_id);

	return 0;
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

	err = ionic_cq_init(lif, &new->cq, &new->intr, num_descs, cq_desc_size);
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
	struct device *dev = lif->ionic->dev;

	if (!qcq)
		return;

	dma_free_coherent(lif->ionic->dev, qcq->total_size, qcq->base,
			  qcq->base_pa);
	qcq->base = NULL;
	qcq->base_pa = 0;

	ionic_intr_free(lif, &qcq->intr);
	dma_free_coherent(dev, qcq->total_size, qcq->base, qcq->base_pa);
	devm_kfree(dev, qcq->cq.info);
	devm_kfree(dev, qcq->q.info);
	devm_kfree(dev, qcq);
}

static int ionic_qcqs_alloc(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
	unsigned int flags;
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

	flags = QCQ_F_INTR;
	err = ionic_qcq_alloc(lif, 0, "admin", flags, 1 << 4,
			      sizeof(struct admin_cmd),
			      sizeof(struct admin_comp),
			      0, lif->kern_pid, &lif->adminqcq);
	if (err)
		return err;

	flags = QCQ_F_TX_STATS | QCQ_F_INTR | QCQ_F_SG;
	for (i = 0; i < lif->ntxqcqs; i++) {
		err = ionic_qcq_alloc(lif, i, "tx", flags, ntxq_descs,
				      sizeof(struct txq_desc),
				      sizeof(struct txq_comp),
				      sizeof(struct txq_sg_desc),
				      lif->kern_pid, &lif->txqcqs[i]);
		if (err)
			goto err_out_free_adminqcq;
	}

	flags = QCQ_F_RX_STATS | QCQ_F_INTR;
	for (i = 0; i < lif->nrxqcqs; i++) {
		err = ionic_qcq_alloc(lif, i, "rx", flags, nrxq_descs,
				      sizeof(struct rxq_desc),
				      sizeof(struct rxq_comp),
				      0, lif->kern_pid, &lif->rxqcqs[i]);
		if (err)
			goto err_out_free_txqcqs;
	}

	return 0;

err_out_free_txqcqs:
	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_qcq_free(lif, lif->txqcqs[i]);
	devm_kfree(dev, lif->txqcqs);
	lif->txqcqs = NULL;
err_out_free_adminqcq:
	ionic_qcq_free(lif, lif->adminqcq);
	lif->adminqcq = NULL;

	return err;
}

static void ionic_qcqs_free(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < lif->nrxqcqs; i++) {
		ionic_rx_empty(&lif->rxqcqs[i]->q);
		ionic_qcq_free(lif, lif->rxqcqs[i]);
	}
	devm_kfree(lif->ionic->dev, lif->rxqcqs);
	lif->rxqcqs = NULL;

	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_qcq_free(lif, lif->txqcqs[i]);
	devm_kfree(lif->ionic->dev, lif->txqcqs);
	lif->txqcqs = NULL;

	ionic_qcq_free(lif, lif->adminqcq);
	lif->adminqcq = NULL;
}

static int ionic_lif_alloc(struct ionic *ionic, unsigned int index)
{
	struct device *dev = ionic->dev;
	struct net_device *netdev;
	struct lif *lif;
	int err, dbpage_num;

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

	spin_lock_init(&lif->deferred.lock);
	INIT_LIST_HEAD(&lif->deferred.list);
	INIT_WORK(&lif->deferred.work, ionic_lif_deferred_work);

	netdev->netdev_ops = &ionic_netdev_ops;
	ionic_ethtool_set_ops(netdev);
	netdev->watchdog_timeo = 2 * HZ;

	netdev->min_mtu = IONIC_MIN_MTU;
	netdev->max_mtu = IONIC_MAX_MTU;

	mutex_init(&lif->dbid_inuse_lock);
	lif->dbid_count = lif->ionic->ident->dev.ndbpgs_per_lif;
	if (!lif->dbid_count) {
		dev_err(dev, "No doorbell pages, aborting\n");
		err = -EINVAL;
		goto err_out_free_netdev;
	}

	lif->dbid_inuse = kzalloc(BITS_TO_LONGS(lif->dbid_count) * sizeof(long),
				  GFP_KERNEL);
	if (!lif->dbid_inuse) {
		dev_err(dev, "Failed alloc doorbell id bitmap, aborting\n");
		err = -ENOMEM;
		goto err_out_free_netdev;
	}

	/* first doorbell id reserved for kernel (dbid aka pid == zero) */
	set_bit(0, lif->dbid_inuse);
	lif->kern_pid = 0;

	dbpage_num = ionic_db_page_num(&ionic->idev, index, 0);
	lif->kern_dbpage = ionic_bus_map_dbpage(ionic, dbpage_num);
	if (!lif->kern_dbpage) {
		dev_err(dev, "Cannot map dbpage, aborting\n");
		err = -ENOMEM;
		goto err_out_free_dbid;
	}

	err = ionic_qcqs_alloc(lif);
	if (err)
		goto err_out_unmap_dbell;

	list_add_tail(&lif->list, &ionic->lifs);

	return 0;

err_out_unmap_dbell:
	ionic_bus_unmap_dbpage(ionic, lif->kern_dbpage);
	lif->kern_dbpage = NULL;
err_out_free_dbid:
	kfree(lif->dbid_inuse);
	lif->dbid_inuse = NULL;
err_out_free_netdev:
	free_netdev(netdev);
	lif = NULL;

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
		cancel_work_sync(&lif->deferred.work);
		ionic_qcqs_free(lif);
		ionic_bus_unmap_dbpage(lif->ionic, lif->kern_dbpage);
		lif->kern_dbpage = NULL;
		kfree(lif->dbid_inuse);
		lif->dbid_inuse = NULL;
		free_netdev(lif->netdev);
	}
}

static int ionic_lif_stats_start(struct lif *lif, unsigned int ver)
{
	struct net_device *netdev = lif->netdev;
	struct device *dev = lif->ionic->dev;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_stats = {
			.opcode = CMD_OPCODE_LIF_STATS_START,
			.ver = ver,
		},
	};
	int err;

	lif->lif_stats = dma_alloc_coherent(dev, sizeof(*lif->lif_stats),
					    &lif->lif_stats_pa, GFP_KERNEL);

	if (!lif->lif_stats) {
		netdev_err(netdev, "%s OOM\n", __func__);
		return -ENOMEM;
	}

	ctx.cmd.lif_stats.addr = lif->lif_stats_pa;

	dev_dbg(dev, "lif_stats START ver %d addr 0x%llx\n", ver,
		lif->lif_stats_pa);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		goto err_out_free;

	return 0;

err_out_free:
	dma_free_coherent(dev, sizeof(*lif->lif_stats), lif->lif_stats,
			  lif->lif_stats_pa);
	lif->lif_stats = NULL;
	lif->lif_stats_pa = 0;

	return err;
}

static void ionic_lif_stats_stop(struct lif *lif)
{
	struct net_device *netdev = lif->netdev;
	struct device *dev = lif->ionic->dev;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_stats = {
			.opcode = CMD_OPCODE_LIF_STATS_STOP,
		},
	};
	int err;

	dev_dbg(dev, "lif_stats STOP\n");

	if (!lif->lif_stats_pa)
		return;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		netdev_err(netdev, "lif_stats cmd failed %d\n", err);
		return;
	}

	dma_free_coherent(dev, sizeof(*lif->lif_stats), lif->lif_stats,
			  lif->lif_stats_pa);
	lif->lif_stats = NULL;
	lif->lif_stats_pa = 0;
}

static void ionic_lif_rss_teardown(struct lif *lif);

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
	ionic_lif_rss_teardown(lif);
	return err;
}

static void ionic_lif_rss_teardown(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
	size_t tbl_size = sizeof(*lif->rss_ind_tbl) * RSS_IND_TBL_SIZE;
	dma_addr_t tbl_pa;

	if (!lif->rss_ind_tbl)
		return;

	/* clear the table from the NIC, then release the DMA space */
	tbl_pa = lif->rss_ind_tbl_pa;
	lif->rss_ind_tbl_pa = 0;
	ionic_rss_ind_tbl_set(lif, NULL);

	dma_free_coherent(dev, tbl_size, lif->rss_ind_tbl, tbl_pa);
	lif->rss_ind_tbl = NULL;
	lif->rss_ind_tbl_pa = 0;
}

static void ionic_lif_qcq_deinit(struct lif *lif, struct qcq *qcq)
{
	struct device *dev = lif->ionic->dev;

	if (!(qcq->flags & QCQ_F_INITED))
		return;
	ionic_intr_mask(&qcq->intr, true);
	synchronize_irq(qcq->intr.vector);
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
	if (!(lif->flags & LIF_F_INITED))
		return;
	lif->flags &= ~LIF_F_INITED;

	ionic_lif_stats_stop(lif);
	ionic_lif_txqs_deinit(lif);
	ionic_lif_rxqs_deinit(lif);
	ionic_rx_filters_deinit(lif);
	ionic_lif_rss_teardown(lif);

	/* drop adminq last */
	napi_disable(&lif->adminqcq->napi);
	ionic_lif_qcq_deinit(lif, lif->adminqcq);
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
		 "%s-%s-%s", DRV_NAME, dev_name(dev), q->name);

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
	err = ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
	if (err)
		return err;

	ionic_dev_cmd_comp(idev, &comp);
	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = ionic_db_map(lif, q);

	netif_napi_add(lif->netdev, napi, ionic_adminq_napi,
		       NAPI_POLL_WEIGHT);

	err = ionic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	/* Enabling interrupts on adminq from here on... */
	napi_enable(napi);
	ionic_intr_mask(&lif->adminqcq->intr, false);

	err = ionic_debugfs_add_qcq(lif, qcq);
	if (err)
		netdev_warn(lif->netdev, "debugfs add for adminq failed %d\n",
			    err);

	return 0;
}

static int ionic_get_features(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
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
				| ETH_HW_RX_CSUM
				| ETH_HW_TSO
				| ETH_HW_TSO_IPV6
				| ETH_HW_TSO_ECN,
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	lif->hw_features = ctx.cmd.features.wanted &
			   ctx.comp.features.supported;

	if (lif->hw_features & ETH_HW_VLAN_TX_TAG)
		dev_dbg(dev, "feature ETH_HW_VLAN_TX_TAG\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_STRIP)
		dev_dbg(dev, "feature ETH_HW_VLAN_RX_STRIP\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_FILTER)
		dev_dbg(dev, "feature ETH_HW_VLAN_RX_FILTER\n");
	if (lif->hw_features & ETH_HW_RX_HASH)
		dev_dbg(dev, "feature ETH_HW_RX_HASH\n");
	if (lif->hw_features & ETH_HW_TX_SG)
		dev_dbg(dev, "feature ETH_HW_TX_SG\n");
	if (lif->hw_features & ETH_HW_TX_CSUM)
		dev_dbg(dev, "feature ETH_HW_TX_CSUM\n");
	if (lif->hw_features & ETH_HW_RX_CSUM)
		dev_dbg(dev, "feature ETH_HW_RX_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO)
		dev_dbg(dev, "feature ETH_HW_TSO\n");
	if (lif->hw_features & ETH_HW_TSO_IPV6)
		dev_dbg(dev, "feature ETH_HW_TSO_IPV6\n");
	if (lif->hw_features & ETH_HW_TSO_ECN)
		dev_dbg(dev, "feature ETH_HW_TSO_ECN\n");
	if (lif->hw_features & ETH_HW_TSO_GRE)
		dev_dbg(dev, "feature ETH_HW_TSO_GRE\n");
	if (lif->hw_features & ETH_HW_TSO_GRE_CSUM)
		dev_dbg(dev, "feature ETH_HW_TSO_GRE_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP4)
		dev_dbg(dev, "feature ETH_HW_TSO_IPXIP4\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP6)
		dev_dbg(dev, "feature ETH_HW_TSO_IPXIP6\n");
	if (lif->hw_features & ETH_HW_TSO_UDP)
		dev_dbg(dev, "feature ETH_HW_TSO_UDP\n");
	if (lif->hw_features & ETH_HW_TSO_UDP_CSUM)
		dev_dbg(dev, "feature ETH_HW_TSO_UDP_CSUM\n");

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

static int ionic_lif_txq_init(struct lif *lif, struct qcq *qcq)
{
	struct device *dev = lif->ionic->dev;
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

	dev_dbg(dev, "txq_init.pid %d\n", ctx.cmd.txq_init.pid);
	dev_dbg(dev, "txq_init.index %d\n", ctx.cmd.txq_init.index);
	dev_dbg(dev, "txq_init.ring_base 0x%llx\n",
		ctx.cmd.txq_init.ring_base);
	dev_dbg(dev, "txq_init.ring_size %d\n",
		ctx.cmd.txq_init.ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	q->qid = ctx.comp.txq_init.qid;
	q->qtype = ctx.comp.txq_init.qtype;
	q->db = ionic_db_map(lif, q);

	netif_napi_add(lif->netdev, napi, ionic_tx_napi,
		       NAPI_POLL_WEIGHT);

	err = ionic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	dev_dbg(dev, "txq->qid %d\n", q->qid);
	dev_dbg(dev, "txq->qtype %d\n", q->qtype);
	dev_dbg(dev, "txq->db %p\n", q->db);

	err = ionic_debugfs_add_qcq(lif, qcq);
	if (err)
		netdev_warn(lif->netdev, "debugfs add for txq %d failed %d\n",
			    q->qid, err);

	return 0;
}

static int ionic_lif_txqs_init(struct lif *lif)
{
	unsigned int i;
	int err;

	for (i = 0; i < lif->ntxqcqs; i++) {
		err = ionic_lif_txq_init(lif, lif->txqcqs[i]);
		if (err)
			goto err_out;
	}

	return 0;

err_out:
	for (; i > 0; i--)
		ionic_lif_qcq_deinit(lif, lif->txqcqs[i-1]);

	return err;
}

static int ionic_lif_rxq_init(struct lif *lif, struct qcq *qcq)
{
	struct device *dev = lif->ionic->dev;
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

	dev_dbg(dev, "rxq_init.pid %d\n", ctx.cmd.rxq_init.pid);
	dev_dbg(dev, "rxq_init.index %d\n", ctx.cmd.rxq_init.index);
	dev_dbg(dev, "rxq_init.ring_base 0x%llx\n",
		   ctx.cmd.rxq_init.ring_base);
	dev_dbg(dev, "rxq_init.ring_size %d\n",
		   ctx.cmd.rxq_init.ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	q->qid = ctx.comp.rxq_init.qid;
	q->qtype = ctx.comp.rxq_init.qtype;
	q->db = ionic_db_map(lif, q);

	netif_napi_add(lif->netdev, napi, ionic_rx_napi,
		       NAPI_POLL_WEIGHT);

	err = ionic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	dev_dbg(dev, "rxq->qid %d\n", q->qid);
	dev_dbg(dev, "rxq->qtype %d\n", q->qtype);
	dev_dbg(dev, "rxq->db %p\n", q->db);

	err = ionic_debugfs_add_qcq(lif, qcq);
	if (err)
		netdev_warn(lif->netdev, "debugfs add for rxq %d failed %d\n",
			    q->qid, err);

	return 0;
}

static int ionic_lif_rxqs_init(struct lif *lif)
{
	unsigned int i;
	int err;

	for (i = 0; i < lif->nrxqcqs; i++) {
		err = ionic_lif_rxq_init(lif, lif->rxqcqs[i]);
		if (err)
			goto err_out;
	}

	return 0;

err_out:
	for (; i > 0; i--)
		ionic_lif_qcq_deinit(lif, lif->rxqcqs[i-1]);

	return err;
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

	err = ionic_debugfs_add_lif(lif);
	if (err)
		return err;

	ionic_dev_cmd_lif_init(idev, lif->index);
	err = ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
	if (err)
		return err;

	err = ionic_lif_adminq_init(lif);
	if (err)
		goto err_out_adminq_deinit;

	err = ionic_set_features(lif);
	if (err)
		goto err_out_adminq_deinit;

	err = ionic_lif_txqs_init(lif);
	if (err)
		goto err_out_adminq_deinit;

	err = ionic_lif_rxqs_init(lif);
	if (err)
		goto err_out_txqs_deinit;

	err = ionic_rx_filters_init(lif);
	if (err)
		goto err_out_rxqs_deinit;

	err = ionic_station_set(lif);
	if (err)
		goto err_out_rx_filter_deinit;

	if (lif->netdev->features & NETIF_F_RXHASH) {
		err = ionic_lif_rss_setup(lif);
		if (err)
			goto err_out_rx_filter_deinit;
	}

	err = ionic_lif_stats_start(lif, STATS_DUMP_VERSION_1);
	if (err)
		goto err_out_rss_teardown;

	ionic_set_rx_mode(lif->netdev);

	lif->api_private = NULL;
	lif->flags |= LIF_F_INITED;

	return 0;

err_out_rss_teardown:
	ionic_lif_rss_teardown(lif);
err_out_rx_filter_deinit:
	ionic_rx_filters_deinit(lif);
err_out_rxqs_deinit:
	ionic_lif_rxqs_deinit(lif);
err_out_txqs_deinit:
	ionic_lif_txqs_deinit(lif);
err_out_adminq_deinit:
	ionic_lif_qcq_deinit(lif, lif->adminqcq);

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

int ionic_lif_register(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
	int err;

	err = register_netdev(lif->netdev);
	if (err) {
		dev_err(dev, "Cannot register net device, aborting\n");
		return err;
	}

	lif->registered = true;

	return 0;
}

static void ionic_lif_notify_work(struct work_struct *ws)
{
}

static int ionic_lif_changeupper(struct ionic *ionic, struct lif *lif,
				 struct netdev_notifier_changeupper_info *info)
{
	struct netdev_lag_upper_info *upper_info;

	dev_dbg(ionic->dev, "lif %d is lag port %d\n",
		lif->index, netif_is_lag_port(lif->netdev));
	dev_dbg(ionic->dev, "lif %d upper %s is lag master %d\n",
		lif->index, info->upper_dev->name,
		netif_is_lag_master(info->upper_dev));
	dev_dbg(ionic->dev, "lif %d has upper info %d\n",
		lif->index, !!info->upper_info);

	if (!netif_is_lag_port(lif->netdev) ||
	    !netif_is_lag_master(info->upper_dev) ||
	    !info->upper_info)
		return 0;

	upper_info = info->upper_info;

	dev_dbg(ionic->dev, "upper tx type %d\n",
		upper_info->tx_type);

	return 0;
}

static int ionic_lif_changelowerstate(struct ionic *ionic, struct lif *lif,
			    struct netdev_notifier_changelowerstate_info *info)
{
	struct netdev_lag_lower_state_info *lower_info;

	dev_dbg(ionic->dev, "lif %d is lag port %d\n",
		lif->index, netif_is_lag_port(lif->netdev));
	dev_dbg(ionic->dev, "lif %d has lower info %d\n",
		lif->index, !!info->lower_state_info);

	if (!netif_is_lag_port(lif->netdev) ||
	    !info->lower_state_info)
		return 0;

	lower_info = info->lower_state_info;

	dev_dbg(ionic->dev, "link up %d enable %d\n",
		lower_info->link_up, lower_info->tx_enabled);

	return 0;
}

struct lif *ionic_netdev_lif(struct net_device *netdev)
{
	if (!netdev || netdev->netdev_ops->ndo_start_xmit != ionic_start_xmit)
		return NULL;

	return netdev_priv(netdev);
}

static int ionic_lif_notify(struct notifier_block *nb,
			    unsigned long event, void *info)
{
	struct ionic *ionic = container_of(nb, struct ionic, nb);
	struct net_device *ndev = netdev_notifier_info_to_dev(info);
	struct lif *lif = ionic_netdev_lif(ndev);
	int err;

	if (!lif || lif->ionic != ionic)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_CHANGEUPPER:
		err = ionic_lif_changeupper(ionic, lif, info);
		break;
	case NETDEV_CHANGELOWERSTATE:
		err = ionic_lif_changelowerstate(ionic, lif, info);
		break;
	default:
		return NOTIFY_DONE;
	}

	(void)err;

	return NOTIFY_DONE;
}

int ionic_lifs_register(struct ionic *ionic)
{
	struct list_head *cur;
	struct lif *lif;
	int err;

	INIT_WORK(&ionic->nb_work, ionic_lif_notify_work);

	ionic->nb.notifier_call = ionic_lif_notify;

	err = register_netdevice_notifier(&ionic->nb);
	if (err)
		ionic->nb.notifier_call = NULL;

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

	if (ionic->nb.notifier_call) {
		unregister_netdevice_notifier(&ionic->nb);
		cancel_work_sync(&ionic->nb_work);
	}
}

int ionic_lifs_size(struct ionic *ionic)
{
	union identity *ident = ionic->ident;
	unsigned int nlifs = ident->dev.nlifs;
	unsigned int neqs_per_lif = ident->dev.rdma_eq_qtype.qid_count;
	unsigned int nnqs_per_lif = ident->dev.notify_qtype.qid_count;
	unsigned int ntxqs_per_lif = ident->dev.tx_qtype.qid_count;
	unsigned int nrxqs_per_lif = ident->dev.rx_qtype.qid_count;
	unsigned int nintrs, dev_nintrs = ident->dev.nintrs;
	int err;

	ntxqs_per_lif = min(ntxqs_per_lif, num_online_cpus());
	nrxqs_per_lif = min(nrxqs_per_lif, num_online_cpus());

	if (ntxqs > 0)
		ntxqs_per_lif = min(ntxqs_per_lif, ntxqs);
	if (nrxqs > 0)
		nrxqs_per_lif = min(nrxqs_per_lif, nrxqs);

try_again:
	nintrs = nlifs * (nnqs_per_lif +
			  neqs_per_lif +
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
	if (nnqs_per_lif > 1) {
		--nnqs_per_lif;
		goto try_again;
	}
	if (neqs_per_lif > 1) {
		--neqs_per_lif;
		goto try_again;
	}
	if (ntxqs_per_lif > 1) {
		/* keep nTx == nRx */
		--ntxqs_per_lif;
		--nrxqs_per_lif;
		goto try_again;
	}
	return -ENOSPC;
}
