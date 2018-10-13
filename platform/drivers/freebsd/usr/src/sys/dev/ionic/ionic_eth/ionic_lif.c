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
#include <linux/if_ether.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_api.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"

#include "opt_rss.h"

#ifdef	RSS
#include <net/rss_config.h>
#include <netinet/in_rss.h>
#endif

static void ionic_rx_fill(struct rxque *rxq);
//static void ionic_rx_refill(struct rxque *rxq);
static void ionic_rx_empty(struct rxque *rxq, bool destroy);
//static void ionic_rx_flush(struct rxque *rxq);
static int ionic_rx_clean(struct rxque* rxq , int rx_limit);

static int ionic_q_enable_disable(struct lif* lif, struct intr *intr, unsigned int qid, unsigned int qtype, bool enable)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_enable = {
			.opcode = enable ? CMD_OPCODE_Q_ENABLE : CMD_OPCODE_Q_DISABLE,
			.qid = qid,
			.qtype = qtype,
		},
	};
	int err;

	IONIC_NETDEV_INFO(lif->netdev, "%s qid %d qtype:%d\n", enable ? "Enable" : "Disable",
		ctx.cmd.q_enable.qid, ctx.cmd.q_enable.qtype);

	if (enable) {
		err = ionic_adminq_post_wait(lif, &ctx);
		if (err) {
			IONIC_NETDEV_ERROR(lif->netdev, "AdminQ enbale failed for qid %d qtype:%d\n",
				ctx.cmd.q_enable.qid, ctx.cmd.q_enable.qtype);
			return err;
		}
		ionic_intr_mask(intr, false);

		return (0);
	}

	ionic_intr_mask(intr, true);
#ifndef FREEBSD
	synchronize_irq(qcq->intr.vector);
#endif
	return ionic_adminq_post_wait(lif, &ctx);
}

/* Calculate mbuf pool size based on MTU. */
static void ionic_calc_rx_size(struct lif *lif)
{

	if (lif->max_frame_size > MJUM9BYTES)
		lif->rx_mbuf_size  = MJUM16BYTES;
	else if (lif->max_frame_size > MJUMPAGESIZE)
		lif->rx_mbuf_size  = MJUM9BYTES;
	else if (lif->max_frame_size > MCLBYTES)	
		lif->rx_mbuf_size  = MJUMPAGESIZE;
	else
		lif->rx_mbuf_size  = MCLBYTES;

	IONIC_NETDEV_INFO(lif->netdev, "MTU: %d mbuf size pool: %d\n", lif->max_frame_size,
		lif->rx_mbuf_size);
}

void ionic_open(void *arg)
{
	struct lif *lif = arg;
	struct rxque *rxq;
	struct txque *txq;
	unsigned int i;
	int err;

	KASSERT(lif, ("lif is NULL"));

	ionic_calc_rx_size(lif);

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		IONIC_TX_LOCK(txq);
		err = ionic_q_enable_disable(lif, &txq->intr, txq->qid, txq->qtype, true /* enable */);
		WARN_ON(err);
		IONIC_TX_UNLOCK(txq);
	}

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		IONIC_RX_LOCK(rxq);
		ionic_rx_fill(rxq);
		err = ionic_q_enable_disable(lif, &rxq->intr, rxq->qid, rxq->qtype, true /* enable */);
		WARN_ON(err);
		IONIC_RX_UNLOCK(rxq);
	}

	ionic_up_link(lif->netdev);
}

int ionic_reinit(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	struct rxque *rxq;
	struct txque *txq;
	unsigned int i, old_mbuf_size;
	int err;

	KASSERT(lif, ("lif is NULL"));

	old_mbuf_size = lif->rx_mbuf_size;
	ionic_calc_rx_size(lif);

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		IONIC_TX_LOCK(txq);
		err = ionic_q_enable_disable(lif, &txq->intr, txq->qid, txq->qtype, true /* enable */);
		WARN_ON(err);
		IONIC_TX_UNLOCK(txq);
	}

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		IONIC_RX_LOCK(rxq);

		/* If mbuf size has changed, refill Rx buffers. */
		if (old_mbuf_size != lif->rx_mbuf_size) {
			/* Free the old mbus and post the new ones. */
			ionic_rx_empty(rxq, false);
			ionic_rx_fill(rxq);
		}
		err = ionic_q_enable_disable(lif, &rxq->intr, rxq->qid, rxq->qtype, true /* enable */);
		WARN_ON(err);
		IONIC_RX_UNLOCK(rxq);
	}

	ionic_up_link(lif->netdev);

	return (0);
}

/*
 * Called before reinit.
 */
int ionic_stop(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	struct rxque* rxq;
	struct txque* txq;
	unsigned int i;
	int err;

	KASSERT(lif, ("lif is NULL"));
	IONIC_NETDEV_INFO(netdev, "stopping interface\n");
	KASSERT(IONIC_CORE_OWNED(lif), ("%s is not locked", lif->name));

	netdev->if_drv_flags &= ~IFF_DRV_RUNNING;

	for (i = 0; i < lif->ntxqs; i++) {
		// TODO post NOP Tx desc and wait for its completion
		// TODO before disabling Tx queue
		txq = lif->txqs[i];
		IONIC_TX_LOCK(txq);
		err = ionic_q_enable_disable(lif, &txq->intr, txq->qid, txq->qtype, false /* disable */);
		if (err)
			return err;
 		IONIC_TX_UNLOCK(txq);
		taskqueue_drain(txq->taskq, &txq->task);

	}

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		IONIC_RX_LOCK(rxq);
		err = ionic_q_enable_disable(lif, &rxq->intr, rxq->qid, rxq->qtype, false /* disable */);
		if (err) {
			/* XXX: should we continue? */
		}
		IONIC_RX_UNLOCK(rxq);
		taskqueue_drain(rxq->taskq, &rxq->task);

	}

	return 0;
}

/******************* AdminQ ******************************/
static irqreturn_t ionic_adminq_isr(int irq, void *data)
{
	struct adminq* adminq = data;

	KASSERT(adminq, ("adminq == NULL"));
	ionic_intr_mask(&adminq->intr, true);

	napi_schedule_irqoff(&adminq->napi);

	return IRQ_HANDLED;
}

static void ionic_adminq_napi(struct napi_struct *napi)
{
	struct admin_comp *comp;
	struct admin_cmd *cmd;
	int comp_index, cmd_index, processed, cmd_stop_index;
	int budget = NAPI_POLL_WEIGHT;
	struct adminq* adminq = container_of(napi, struct adminq, napi);

	mtx_lock(&adminq->mtx);

	for ( processed = 0 ; processed < budget ; ) {
		comp_index = adminq->comp_index;
		comp = &adminq->comp_ring[comp_index];
		/* Sync every time descriptors. */
		bus_dmamap_sync(adminq->cmd_dma.dma_tag, adminq->cmd_dma.dma_map,
			BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);

		cmd_stop_index = comp->comp_index;
		cmd_index = adminq->cmd_tail_index;
		cmd = &adminq->cmd_ring[cmd_index];

		if (comp->color != adminq->done_color)
			break;

		IONIC_QUE_INFO(adminq, "comp :%d cmd start: %d cmd stop: %d comp->color %d done_color %d\n",
			comp_index, cmd_index, cmd_stop_index, comp->color, adminq->done_color);
		IONIC_QUE_INFO(adminq, "buf[%d] opcode:%d\n", cmd_index, cmd->opcode);

		for ( ; cmd_index == cmd_stop_index; cmd_index++, processed++ ) {
			/* XXX: loop to do???? */
			cmd = &adminq->cmd_ring[cmd_index];
		}

		adminq->comp_index = (adminq->comp_index + 1) % adminq->num_descs;
		adminq->cmd_tail_index = (adminq->cmd_tail_index + 1) % adminq->num_descs;
		/* Roll over condition, flip color. */
		if (adminq->comp_index == 0) {
			adminq->done_color = !adminq->done_color;
		}
	}

	IONIC_QUE_INFO(adminq, "ionic_adminq_napi processed %d\n", processed);

	if (processed == budget)
		napi_schedule(&adminq->napi);

	ionic_intr_return_credits(&adminq->intr, processed, 0, true);

	// Enable interrupt.
	ionic_intr_mask(&adminq->intr, false);
	mtx_unlock(&adminq->mtx);
}

static int _ionic_lif_addr(struct lif *lif, const u8 *addr, bool add)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_add = {
			.opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
					CMD_OPCODE_RX_FILTER_DEL,
			.match = RX_FILTER_MATCH_MAC,
		},
	};

	memcpy(ctx.cmd.rx_filter_add.mac.addr, addr, ETH_ALEN);

	IONIC_NETDEV_INFO(lif->netdev, "rx_filter %s %02x:%02x:%02x:%02x:%02x:%02x\n",
		  add ? "add" : "del", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

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

	IONIC_DEVICE_INFO(lif->ionic->dev, "%02x:%02x:%02x:%02x:%02x:%02x %s\n",
		  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],
		  add ? "add" : "del");

	if (true) {
		work = kmalloc(sizeof(*work), GFP_ATOMIC);
		if (!work) {
			IONIC_NETDEV_ERROR(lif->netdev, "failed to allocate memory for address.\n");
			return -ENOMEM;
		}
		INIT_WORK(&work->work, ionic_lif_addr_work);
		work->lif = lif;
		memcpy(work->addr, addr, ETH_ALEN);
		work->add = add;
		IONIC_NETDEV_INFO(lif->netdev, "deferred: rx_filter %s %02x:%02x:%02x:%02x:%02x:%02x\n",
			   add ? "add" : "del", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
		queue_work(lif->adminq_wq, &work->work);
	} else {
		return _ionic_lif_addr(lif, addr, add);
	}

	return 0;
}

static int ionic_addr_add(struct net_device *netdev, const u8 *addr)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEVICE_INFO(lif->ionic->dev, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	return ionic_lif_addr(lif, addr, true);
}

static int ionic_addr_del(struct net_device *netdev, const u8 *addr)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEVICE_INFO(lif->ionic->dev, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	return ionic_lif_addr(lif, addr, false);
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

	IONIC_DEVICE_INFO(lif->ionic->dev, "%#x\n", rx_mode);

	if (rx_mode & RX_MODE_F_UNICAST)
		IONIC_NETDEV_INFO(lif->netdev, "rx_mode RX_MODE_F_UNICAST\n");
	if (rx_mode & RX_MODE_F_MULTICAST)
		IONIC_NETDEV_INFO(lif->netdev, "rx_mode RX_MODE_F_MULTICAST\n");
	if (rx_mode & RX_MODE_F_BROADCAST)
		IONIC_NETDEV_INFO(lif->netdev, "rx_mode RX_MODE_F_BROADCAST\n");
	if (rx_mode & RX_MODE_F_PROMISC)
		IONIC_NETDEV_INFO(lif->netdev, "rx_mode RX_MODE_F_PROMISC\n");
	if (rx_mode & RX_MODE_F_ALLMULTI)
		IONIC_NETDEV_INFO(lif->netdev, "rx_mode RX_MODE_F_ALLMULTI\n");

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

	IONIC_DEVICE_INFO(lif->ionic->dev, "%#x\n", rx_mode);

	if (true) {
		work = kmalloc(sizeof(*work), GFP_ATOMIC);
		if (!work) {
			IONIC_NETDEV_ERROR(lif->netdev, "%s OOM\n", __func__);
			return;
		}
		INIT_WORK(&work->work, ionic_lif_rx_mode_work);
		work->lif = lif;
		work->rx_mode = rx_mode;
		IONIC_NETDEV_INFO(lif->netdev, "deferred: rx_mode\n");
		queue_work(lif->adminq_wq, &work->work);
	} else {
		_ionic_lif_rx_mode(lif, rx_mode);
	}
}

static void ionic_set_rx_mode(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	unsigned int rx_mode;

	IONIC_DEVICE_INFO(lif->ionic->dev, "\n");

	rx_mode = RX_MODE_F_UNICAST;
	rx_mode |= (netdev->if_flags & IFF_MULTICAST) ? RX_MODE_F_MULTICAST : 0;
	rx_mode |= (netdev->if_flags & IFF_BROADCAST) ? RX_MODE_F_BROADCAST : 0;
	rx_mode |= (netdev->if_flags & IFF_PROMISC) ? RX_MODE_F_PROMISC : 0;
	rx_mode |= (netdev->if_flags & IFF_ALLMULTI) ? RX_MODE_F_ALLMULTI : 0;

	if (lif->rx_mode != rx_mode) {
		lif->rx_mode = rx_mode;
		ionic_lif_rx_mode(lif, rx_mode);
	}
}

static int ionic_set_mac_address(struct net_device *netdev, void *addr)
{
	// TODO implement
	IONIC_NETDEV_ERROR(netdev, "SET MAC ADDRESS not implemented\n");
	return 0;
}

int ionic_change_mtu(struct net_device *netdev, int new_mtu)
{
	struct lif *lif = netdev_priv(netdev);
	int err;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.mtu_set = {
			.opcode = CMD_OPCODE_MTU_SET,
			.mtu = new_mtu,
		},
	};


	IONIC_DEVICE_INFO(lif->ionic->dev, "\n");

	if (netif_running(netdev))
		ionic_stop(netdev);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	if_setmtu(netdev, new_mtu);

	ionic_reinit(netdev);

	return 0;
}

#ifdef notyet
static void ionic_tx_timeout(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEVICE_INFO(lif->ionic->dev, "\n");

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

	IONIC_DEVICE_INFO(lif->ionic->dev, "\n");

	IONIC_NETDEV_INFO(netdev, "rx_filter %s VLAN %d\n", add ? "add" : "del", vid);

	return ionic_adminq_post_wait(lif, &ctx);
}

static int ionic_vlan_rx_add_vid(struct net_device *netdev,
				 __be16 proto, u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEVICE_INFO(lif->ionic->dev, "\n");

	return ionic_vlan_rx_filter(netdev, true, proto, vid);
}

static int ionic_vlan_rx_kill_vid(struct net_device *netdev,
				  __be16 proto, u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEVICE_INFO(lif->ionic->dev, "\n");

	return ionic_vlan_rx_filter(netdev, false, proto, vid);
}
#endif
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

static int ionic_adminq_alloc(struct lif *lif, unsigned int qnum,
			unsigned int num_descs, unsigned int pid,
			struct adminq **padminq)
{
	struct adminq *adminq;
	int irq, error = ENOMEM; 
	uint32_t cmd_ring_size, comp_ring_size, total_size;

	*padminq = NULL;

	adminq = malloc(sizeof(*adminq), M_IONIC, M_NOWAIT | M_ZERO);
	if(adminq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev, "failed to allocate rxq%d\n", qnum);
		return (error);
	}

	snprintf(adminq->name, sizeof(adminq->name) - 1, "adq%d", qnum);
	adminq->lif = lif;
	adminq->index = qnum;
	adminq->num_descs = num_descs;
	adminq->pid = pid;
	adminq->done_color = 1;

	mtx_init(&adminq->mtx, adminq->name, NULL, MTX_DEF);

	adminq->cmd_head_index = adminq->cmd_tail_index = 0;
	adminq->comp_index = 0;

	/* Allocate DMA for command and completion rings. They must be consecutive. */
	cmd_ring_size = sizeof(*adminq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*adminq->comp_ring) * num_descs;
	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(cmd_ring_size, PAGE_SIZE);

	if ((error = ionic_dma_alloc(adminq->lif->ionic, total_size, &adminq->cmd_dma, BUS_DMA_NOWAIT))) {
		IONIC_QUE_ERR(adminq, "failed to allocated DMA cmd ring, err: %d\n", error);
		goto failed_alloc;
	}

	adminq->cmd_ring_pa = adminq->cmd_dma.dma_paddr;
	adminq->cmd_ring = (struct admin_cmd *)adminq->cmd_dma.dma_vaddr;
	IONIC_QUE_INFO(adminq, "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
		adminq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * We assume that competion ring is next to command ring.
	 */
	adminq->comp_ring = (struct admin_comp *)(adminq->cmd_dma.dma_vaddr + ALIGN(cmd_ring_size, PAGE_SIZE));

	bzero((void *)adminq->cmd_ring, total_size);

	/* Setup interrupt */
	error = ionic_intr_alloc(lif, &adminq->intr);
	if (error) {
		IONIC_QUE_ERR(adminq, "no available interrupt, error: %d\n", error);
		goto failed_alloc;
	}

	irq = ionic_bus_get_irq(lif->ionic, adminq->intr.index);
	if (irq < 0) {
		IONIC_QUE_ERR(adminq, "no available IRQ, error: %d\n", error);
		goto free_intr;
	}

	adminq->intr.vector = irq;
	ionic_intr_mask_on_assertion(&adminq->intr);
 
	*padminq = adminq;
	return 0;

free_intr:
	ionic_intr_free(lif, &adminq->intr);

failed_alloc:
	if (adminq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(adminq->lif->ionic, &adminq->cmd_dma);
		adminq->cmd_ring = NULL;
		adminq->comp_ring = NULL;
	}

	free(adminq, M_IONIC);
	return (error);
}

static int ionic_rxque_alloc(struct lif *lif, unsigned int qnum,
			unsigned int num_descs, unsigned int pid,
			struct rxque **prxq)
{
	struct rxque *rxq;
	struct ionic_rx_buf *rxbuf;
	int i, irq, error = ENOMEM; 
	uint32_t cmd_ring_size, comp_ring_size, total_size;

	*prxq = NULL;

	rxq = malloc(sizeof(*rxq), M_IONIC, M_NOWAIT | M_ZERO);
	if(rxq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev, "failed to allocate rxq%d\n", qnum);
		return (error);
	}

	snprintf(rxq->name, sizeof(rxq->name) - 1, "rxq%d", qnum);
	rxq->lif = lif;
	rxq->index = qnum;
	rxq->num_descs = num_descs;
	rxq->pid = pid;
	rxq->done_color = 1;

	IONIC_RX_INIT(rxq);

	/* rx buffer and command are in tendom */
	rxq->cmd_head_index = rxq->cmd_tail_index = 0;
	rxq->comp_index = 0;

	/* Setup command ring. */
	rxq->rxbuf = malloc(sizeof(*rxq->rxbuf) * num_descs, M_IONIC, M_NOWAIT | M_ZERO);
	if (rxq->rxbuf == NULL) {
		IONIC_QUE_ERR(rxq, "Couldn't allocate rx buffer descriptors\n");
		goto failed_alloc;
	}

	/* Allocate DMA for command and completion rings. They must be consecutive. */
	cmd_ring_size = sizeof(*rxq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*rxq->comp_ring) * num_descs;
	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(cmd_ring_size, PAGE_SIZE);

	if ((error = ionic_dma_alloc(rxq->lif->ionic, total_size, &rxq->cmd_dma, BUS_DMA_NOWAIT))) {
		IONIC_QUE_ERR(rxq, "failed to allocated DMA cmd ring, err: %d\n", error);
		goto failed_alloc;
	}

	rxq->cmd_ring_pa = rxq->cmd_dma.dma_paddr;
	rxq->cmd_ring = (struct rxq_desc *)rxq->cmd_dma.dma_vaddr;
	IONIC_QUE_INFO(rxq, "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
		rxq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * We assume that competion ring is next to command ring.
	 */
	rxq->comp_ring = (struct rxq_comp *)(rxq->cmd_dma.dma_vaddr + ALIGN(cmd_ring_size, PAGE_SIZE));

	bzero((void *)rxq->cmd_ring, total_size);

	/* Setup interrupt */
	error = ionic_intr_alloc(lif, &rxq->intr);
	if (error) {
		IONIC_QUE_ERR(rxq, "no available interrupt, error: %d\n", error);
		goto failed_alloc;
	}

	irq = ionic_bus_get_irq(lif->ionic, rxq->intr.index);
	if (irq < 0) {
		IONIC_QUE_ERR(rxq, "no available IRQ, error: %d\n", error);
		goto failed_alloc;
	}

	rxq->intr.vector = irq;
	ionic_intr_mask_on_assertion(&rxq->intr);
 
	/*
	 * Create just one tag for Rx bufferes.
	 */
	error = bus_dma_tag_create(
	         /*      parent */ bus_get_dma_tag(rxq->lif->ionic->dev->bsddev),
	         /*   alignment */ 1,
	         /*      bounds */ 0,
	         /*     lowaddr */ BUS_SPACE_MAXADDR,
	         /*    highaddr */ BUS_SPACE_MAXADDR,
	         /*      filter */ NULL,
	         /*   filterarg */ NULL,
	         /*     maxsize */ MJUM16BYTES,
	         /*   nsegments */ 1,
	         /*  maxsegsize */ MJUM16BYTES,
	         /*       flags */ 0,
	         /*    lockfunc */ NULL,
	         /* lockfuncarg */ NULL,
	                           &rxq->buf_tag);

	if (error) {
		IONIC_QUE_ERR(rxq, "failed to create DMA tag, err: %d\n", error);
		goto free_intr;
	}

	for ( rxbuf = rxq->rxbuf, i = 0 ; rxbuf != NULL && i < num_descs; i++, rxbuf++ ) {
		error = bus_dmamap_create(rxq->buf_tag, 0, &rxbuf->dma_map);
		if (error) {
			IONIC_QUE_ERR(rxq, "failed to create map for entry%d, err: %d\n", i, error);
			bus_dma_tag_destroy(rxq->buf_tag);
			goto free_intr;
		}
	}

	*prxq = rxq;

	return 0;

free_intr:
	ionic_intr_free(lif, &rxq->intr);

failed_alloc:
	if (rxq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(rxq->lif->ionic, &rxq->cmd_dma);
		rxq->cmd_ring = NULL;
		rxq->comp_ring = NULL;
	}

	if (rxq->rxbuf) {
		free(rxq->rxbuf, M_IONIC);
		rxq->rxbuf = NULL;
	}

	free(rxq, M_IONIC);

	return (error);
}

static int ionic_txque_alloc(struct lif *lif, unsigned int qnum,
			   unsigned int num_descs, unsigned int pid,
			   struct txque **ptxq)
{
	struct txque *txq;
	struct ionic_tx_buf *txbuf;
	int i, irq, error = ENOMEM;
	uint32_t cmd_ring_size, comp_ring_size, sg_ring_size, total_size;

	*ptxq = NULL;

	txq = malloc(sizeof(*txq), M_IONIC, M_NOWAIT | M_ZERO);
	if(txq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev, "failed to allocate rxq%d\n", qnum);
		return (error);
	}

	snprintf(txq->name, sizeof(txq->name) - 1, "txq%d", qnum);
	txq->lif = lif;
	txq->index = qnum;
	txq->num_descs = num_descs;
	txq->pid = pid;
	txq->done_color = 1;

	IONIC_TX_INIT(txq);

	/* rx buffer and command are in tendom */
	txq->cmd_head_index = txq->cmd_tail_index = 0;
	txq->comp_index = 0;

	/* Setup command ring. */
	txq->txbuf = malloc(sizeof(*txq->txbuf) * num_descs, M_IONIC, M_NOWAIT | M_ZERO);
	if (txq->txbuf == NULL) {
		IONIC_QUE_ERR(txq, "Couldn't allocate tx buffer descriptors\n");
		goto failed_alloc;
	}

	/* Allocate DMA for command and completion rings. They must be consecutive. */
	cmd_ring_size = sizeof(*txq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*txq->comp_ring) * num_descs;
	sg_ring_size = sizeof(*txq->sg_ring) * num_descs;
	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(sg_ring_size, PAGE_SIZE);

	if ((error = ionic_dma_alloc(txq->lif->ionic, total_size, &txq->cmd_dma, BUS_DMA_NOWAIT))) {
		IONIC_QUE_ERR(txq, "failed to allocated DMA cmd ring, err: %d\n", error);
		goto failed_alloc;
	}

	txq->cmd_ring_pa = txq->cmd_dma.dma_paddr;
	txq->cmd_ring = (struct txq_desc *)txq->cmd_dma.dma_vaddr;
	IONIC_QUE_INFO(txq, "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
		txq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * We assume that competion ring is next to command ring.
	 */
	txq->comp_ring = (struct txq_comp *)(txq->cmd_dma.dma_vaddr + ALIGN(cmd_ring_size, PAGE_SIZE));
	txq->sg_ring = (struct txq_sg_desc *)(txq->cmd_dma.dma_vaddr + ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(comp_ring_size, PAGE_SIZE));

	bzero((void *)txq->cmd_ring, total_size);

	/* Allocate buffere ring. */
	txq->br = buf_ring_alloc(4096, M_IONIC, M_WAITOK, &txq->tx_mtx);
	if (txq->br == NULL) {
		IONIC_QUE_ERR(txq, "failed to allocated buffer ring\n");
		goto failed_alloc;
	}

	/* Setup interrupt */
	error = ionic_intr_alloc(lif, &txq->intr);
	if (error) {
		IONIC_QUE_ERR(txq, "no available interrupt, error: %d\n", error);
		goto failed_alloc;
	}

	irq = ionic_bus_get_irq(lif->ionic, txq->intr.index);
	if (irq < 0) {
		IONIC_QUE_ERR(txq, "no available IRQ, error: %d\n", error);
		goto failed_alloc;
	}

	txq->intr.vector = irq;
	ionic_intr_mask_on_assertion(&txq->intr);
 
	/*
	 * Create just one tag for Rx bufferrs. 
	 */
	error = bus_dma_tag_create(
	         /*      parent */ bus_get_dma_tag(txq->lif->ionic->dev->bsddev),
	         /*   alignment */ 1,
	         /*      bounds */ 0,
	         /*     lowaddr */ BUS_SPACE_MAXADDR,
	         /*    highaddr */ BUS_SPACE_MAXADDR,
	         /*      filter */ NULL,
	         /*   filterarg */ NULL,
	         /*     maxsize */ 64000,
	         /*   nsegments */ 16,
	         /*  maxsegsize */ 4096,
	         /*       flags */ 0,
	         /*    lockfunc */ NULL,
	         /* lockfuncarg */ NULL,
	                           &txq->buf_tag);

	if (error) {
		IONIC_QUE_ERR(txq, "failed to create DMA tag, err: %d\n", error);
		goto free_intr;
	}

	for ( txbuf = txq->txbuf, i = 0 ; txbuf != NULL && i < num_descs; i++, txbuf++ ) {
		error = bus_dmamap_create(txq->buf_tag, 0, &txbuf->dma_map);
		if (error) {
			IONIC_QUE_ERR(txq, "failed to create map for entry%d, err: %d\n", i, error);
			bus_dma_tag_destroy(txq->buf_tag);
			goto free_intr;
		}
	}

	IONIC_QUE_INFO(txq, "create txq\n");
	*ptxq = txq;

	return 0;

free_intr:
	ionic_intr_free(lif, &txq->intr);

failed_alloc:
	if (txq->br) {
		buf_ring_free(txq->br, M_IONIC);
		txq->br = NULL;
	}

	if (txq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(txq->lif->ionic, &txq->cmd_dma);
		txq->cmd_ring = NULL;
		txq->comp_ring = NULL;
		txq->sg_ring = NULL;
	}

	if (txq->txbuf) {
		free(txq->txbuf, M_IONIC);
		txq->txbuf = NULL;
	}

	free(txq, M_IONIC);

	return (error);
}

static void ionic_rxq_free(struct lif *lif, struct rxque *rxq)
{

	IONIC_RX_LOCK(rxq);

	free(rxq->rxbuf, M_IONIC);

	if (rxq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(rxq->lif->ionic, &rxq->cmd_dma);
		rxq->cmd_ring = NULL;
		rxq->comp_ring = NULL;
	}

	ionic_intr_free(lif, &rxq->intr);

	IONIC_RX_UNLOCK(rxq);
	IONIC_RX_DESTROY(rxq);

	free(rxq, M_IONIC);
}

static void ionic_txq_free(struct lif *lif, struct txque *txq)
{
	IONIC_TX_LOCK(txq);

	if (txq->br) {
		buf_ring_free(txq->br, M_IONIC);
		txq->br = NULL;
	}

	free(txq->txbuf, M_IONIC);

	if (txq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(txq->lif->ionic, &txq->cmd_dma);
		txq->cmd_ring = NULL;
		txq->comp_ring = NULL;
	}

	ionic_intr_free(lif, &txq->intr);
	IONIC_TX_UNLOCK(txq);
	IONIC_TX_DESTROY(txq);

	free(txq, M_IONIC);
}

static void ionic_adminq_free(struct lif *lif, struct adminq *adminq)
{

	mtx_lock(&adminq->mtx);
	if (adminq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(adminq->lif->ionic, &adminq->cmd_dma);
		adminq->cmd_ring = NULL;
		adminq->comp_ring = NULL;
	}

	ionic_intr_free(lif, &adminq->intr);
	mtx_unlock(&adminq->mtx);
	mtx_destroy(&adminq->mtx);

	free(adminq, M_IONIC);
}


static unsigned int ionic_pid_get(struct lif *lif, unsigned int page)
{
	unsigned int ndbpgs_per_lif = lif->ionic->ident->dev.ndbpgs_per_lif;

	BUG_ON(ndbpgs_per_lif < page + 1);

	return lif->index * ndbpgs_per_lif + page;
}

static int ionic_qcqs_alloc(struct lif *lif)
{
	unsigned int pid;
	unsigned int i;
	int err = -ENOMEM;

	lif->txqs = kzalloc(sizeof(*lif->txqs) * lif->ntxqs, GFP_KERNEL);
	if (!lif->txqs)
		return -ENOMEM;

	lif->rxqs = kzalloc(sizeof(*lif->rxqs) * lif->nrxqs, GFP_KERNEL);
	if (!lif->rxqs)
		return -ENOMEM;

	pid = ionic_pid_get(lif, 0);

	/* XXX: we are tight on name description */
	err = ionic_adminq_alloc(lif, 0, adminq_descs, pid, &lif->adminqcq);
	if (err)
		return err;


	for (i = 0; i < lif->ntxqs; i++) {
		err = ionic_txque_alloc(lif, i, ntxq_descs, pid, &lif->txqs[i]);
		if (err)
			goto err_out_free_adminqcq;
	}

	for (i = 0; i < lif->nrxqs; i++) {
		err = ionic_rxque_alloc(lif, i, nrxq_descs, pid, &lif->rxqs[i]);
		if (err)
			goto err_out_free_txqs;
	}

	return 0;

err_out_free_txqs:
	for (i = 0; i < lif->ntxqs; i++)
		ionic_txq_free(lif, lif->txqs[i]);
err_out_free_adminqcq:
	ionic_adminq_free(lif, lif->adminqcq);

	return err;
}

static void ionic_qcqs_free(struct lif *lif)
{
	unsigned int i;
	struct rxque *rxq;
	struct txque *txq;

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		ionic_rx_empty(rxq, true);
		ionic_rxq_free(lif, rxq);
	}
	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		ionic_txq_free(lif, txq);
	}

	ionic_adminq_free(lif, lif->adminqcq);

}

static int ionic_lif_alloc(struct ionic *ionic, unsigned int index)
{
	struct device *dev = ionic->dev;
//	struct net_device *netdev;
	struct lif *lif;
	int err;

	lif = kzalloc(sizeof(*lif), GFP_KERNEL);
	if (!lif) {
		dev_err(dev, "Cannot allocate lif, aborting\n");
		return -ENOMEM;
	}

	snprintf(lif->name, sizeof(lif->name), "ionic%u", index);
	lif->ionic = ionic;
	lif->index = index;
	lif->neqs = ionic->neqs_per_lif;
	lif->ntxqs = ionic->ntxqs_per_lif;
	lif->nrxqs = ionic->nrxqs_per_lif;

	err = ionic_lif_netdev_alloc(lif, ntxq_descs);
	if (err) {
		dev_err(dev, "Cannot allocate netdev, aborting\n");
		return (err);
	}

	IONIC_ADMIN_LOCK_INIT(lif);
	lif->adminq_wq = create_workqueue(lif->name);

	err = ionic_qcqs_alloc(lif);
	if (err)
		goto err_out_free_netdev;

	list_add_tail(&lif->list, &ionic->lifs);

	return 0;

err_out_free_netdev:
	ionic_lif_netdev_free(lif);
	kfree(lif);

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
		ionic_lif_netdev_free(lif);
		kfree(lif);
	}
}

#ifdef notyet
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
	int err;

	lif->stats_dump = dma_alloc_coherent(dev, sizeof(*lif->stats_dump),
					     &lif->stats_dump_pa, GFP_KERNEL);

	if (!lif->stats_dump) {
		IONIC_NETDEV_ERROR(netdev, "%s OOM\n", __func__);
		return -ENOMEM;
	}

	ctx.cmd.stats_dump.addr = lif->stats_dump_pa;

	IONIC_NETDEV_INFO(netdev, "stats_dump START ver %d addr 0x%llx\n", ver,
		    lif->stats_dump_pa);

	return 0;
	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		goto err_out_free;

	return 0;

err_out_free:
	dma_free_coherent(dev, sizeof(*lif->stats_dump), lif->stats_dump,
			  lif->stats_dump_pa);
	return err;
}

static void ionic_lif_stats_dump_stop(struct lif *lif)
{
	struct net_device *netdev = lif->netdev;
	struct device *dev = lif->ionic->dev;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.stats_dump = {
			.opcode = CMD_OPCODE_STATS_DUMP_STOP,
		},
	};
	int err;

	IONIC_NETDEV_INFO(netdev, "stats_dump STOP\n");

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		IONIC_NETDEV_ERROR(netdev, "stats_dump cmd failed %d\n", err);
		return;
	}

	dma_free_coherent(dev, sizeof(*lif->stats_dump), lif->stats_dump,
			  lif->stats_dump_pa);
}

#endif /* notyet */


int
ionic_rss_ind_tbl_set(struct lif *lif, const u32 *indir)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rss_indir_set = {
			.opcode = CMD_OPCODE_RSS_INDIR_SET,
			.addr = lif->rss_ind_tbl_pa,
		},
#ifdef HAPS
		.side_data = lif->rss_ind_tbl,
		.side_data_len = RSS_IND_TBL_SIZE,
#endif
	};
	unsigned int i;

	if (indir)
		for (i = 0; i < RSS_IND_TBL_SIZE; i++)
			lif->rss_ind_tbl[i] = indir[i];

	IONIC_NETDEV_INFO(lif->netdev, "rss_ind_tbl_set\n");

	return ionic_adminq_post_wait(lif, &ctx);
}

int ionic_rss_hash_key_set(struct lif *lif, const u8 *key, uint16_t rss_types)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rss_hash_set = {
			.opcode = CMD_OPCODE_RSS_HASH_SET,
			.types = rss_types,
		},
	};

	memcpy(lif->rss_hash_key, key, RSS_HASH_KEY_SIZE);

	memcpy(ctx.cmd.rss_hash_set.key, lif->rss_hash_key,
	       RSS_HASH_KEY_SIZE);

	IONIC_NETDEV_INFO(lif->netdev, "rss_hash_key_set\n");

	return ionic_adminq_post_wait(lif, &ctx);
}



static void ionic_lif_adminq_deinit(struct lif *lif, struct adminq *adminq)
{
	ionic_intr_mask(&adminq->intr, true);
	free_irq(adminq->intr.vector, &adminq->napi);
	netif_napi_del(&adminq->napi);
}


static void ionic_lif_txqs_deinit(struct lif *lif)
{
	unsigned int i;
	struct txque* txq;

	for (i = 0; i < lif->nrxqs; i++) {
		txq = lif->txqs[i];

		IONIC_TX_LOCK(txq);

		ionic_intr_mask(&txq->intr, true);
		free_irq(txq->intr.vector, txq);

		if (txq->taskq) {
			taskqueue_drain(txq->taskq, &txq->task);
			taskqueue_free(txq->taskq);
		}

		IONIC_TX_UNLOCK(txq);
	}
}

static void ionic_lif_rxqs_deinit(struct lif *lif)
{
	unsigned int i;
	struct rxque* rxq;

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];

		IONIC_RX_LOCK(rxq);
		tcp_lro_free(&rxq->lro);

		ionic_intr_mask(&rxq->intr, true);
		free_irq(rxq->intr.vector, rxq);
		
		if (rxq->taskq) {
			taskqueue_drain(rxq->taskq, &rxq->task);
			taskqueue_free(rxq->taskq);
		}

		IONIC_RX_UNLOCK(rxq);
	}
}

static void ionic_lif_deinit(struct lif *lif)
{
	ifmedia_removeall(&lif->media);
	ether_ifdetach(lif->netdev);
	ionic_lif_adminq_deinit(lif, lif->adminqcq);
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

static int ionic_lif_adminq_init(struct lif *lif)
{
	struct adminq *adminq = lif->adminqcq;
	struct ionic_dev *idev = &lif->ionic->idev;
	struct napi_struct *napi = &adminq->napi;
	struct adminq_init_comp comp;
	int err = 0;

	union dev_cmd cmd = {
		.adminq_init.opcode = CMD_OPCODE_ADMINQ_INIT,
		.adminq_init.index = adminq->index,
		.adminq_init.pid = adminq->pid,
		.adminq_init.intr_index = 0,//intr_index,
		.adminq_init.lif_index = lif->index,
		.adminq_init.ring_size = ilog2(adminq->num_descs),
		.adminq_init.ring_base = adminq->cmd_ring_pa,
	};

	//printk(KERN_ERR "adminq_init.pid %d\n", cmd.adminq_init.pid);
	//printk(KERN_ERR "adminq_init.index %d\n", cmd.adminq_init.index);
	//printk(KERN_ERR "adminq_init.ring_base %llx\n",
	//       cmd.adminq_init.ring_base);
	//printk(KERN_ERR "adminq_init.ring_size %d\n",
	//       cmd.adminq_init.ring_size);
	ionic_dev_cmd_go(idev, &cmd);

	err = ionic_dev_cmd_wait_check(idev, IONIC_DEVCMD_TIMEOUT);
	if (err)
		return err;

	ionic_dev_cmd_comp(idev, &comp);

	IONIC_QUE_INFO(adminq, "qid %d pid %d index %d ring_base 0x%lx ring_size %d\n",
		comp.qid, cmd.adminq_init.pid, cmd.adminq_init.index, cmd.adminq_init.ring_base,
		cmd.adminq_init.ring_size);

	adminq->qid = comp.qid;
	adminq->qtype = comp.qtype;
	adminq->db  = (void *)adminq->lif->ionic->idev.db_pages + (adminq->pid * PAGE_SIZE);
	adminq->db += adminq->qtype;

	snprintf(adminq->intr.name, sizeof(adminq->intr.name), "%s", adminq->name);

	netif_napi_add(lif->netdev, &adminq->napi, ionic_adminq_napi,
		       NAPI_POLL_WEIGHT);

	err = request_irq(adminq->intr.vector, ionic_adminq_isr, 0, adminq->intr.name, adminq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	IONIC_QUE_INFO(adminq, "qid: %d qtype: %d db: %pd\n",
		adminq->qid, adminq->qtype, adminq->db);

	return 0;
}

/*************************** Tx ************************/
int ionic_tx_clean(struct txque* txq , int tx_limit)
{
	struct txq_comp *comp;
	struct ionic_tx_buf *txbuf;
	int comp_index, processed, cmd_stop_index;
	struct tx_stats * stats = &txq->stats;

	stats->clean++;
	
	IONIC_NETDEV_TX_TRACE(txq, "Enter: head: %d tail :%d\n",
		txq->cmd_head_index, txq->cmd_tail_index);
	/* Sync every time descriptors. */
	bus_dmamap_sync(txq->cmd_dma.dma_tag, txq->cmd_dma.dma_map,
		BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);
	
	for ( processed = 0 ; processed < tx_limit ; processed++) {
		comp_index = txq->comp_index;

		comp = &txq->comp_ring[comp_index];
		cmd_stop_index = comp->comp_index;

		
		if (comp->color != txq->done_color)
			break;

		IONIC_NETDEV_TX_TRACE(txq, "comp @ %d for desc @ %d comp->color %d done_color %d\n",
			comp_index, cmd_stop_index, comp->color, txq->done_color);

		txbuf = &txq->txbuf[cmd_stop_index];
		/* TSO last buffer only points to valid mbuf. */
		if (txbuf->m != NULL) {
			IONIC_NETDEV_TX_TRACE(txq, "free txbuf @:%d\n", cmd_stop_index);
			bus_dmamap_sync(txq->buf_tag, txbuf->dma_map, BUS_DMASYNC_POSTWRITE);
			bus_dmamap_unload(txq->buf_tag, txbuf->dma_map);
			m_freem(txbuf->m);
		}

		txq->comp_index = (txq->comp_index + 1) % txq->num_descs;
		/* XXX: should we comp stop index to jump for TSO. */
		txq->cmd_tail_index = (cmd_stop_index + 1) % txq->num_descs;
		/* Roll over condition, flip color. */
		if (txq->comp_index == 0) {
			txq->done_color = !txq->done_color;
		}
	}

	KASSERT(processed, ("%s tx_clean didn't processed any packet", txq->name));
	IONIC_NETDEV_TX_TRACE(txq, "ionic_tx_clean processed %d\n", processed);

	if (comp->color == txq->done_color)
		taskqueue_enqueue(txq->taskq, &txq->task);

	IONIC_NETDEV_TX_TRACE(txq, "Exit: head: %d tail :%d\n",
		txq->cmd_head_index, txq->cmd_tail_index);
	return (processed);
}

static irqreturn_t ionic_tx_isr(int irq, void *data)
{
	struct txque* txq = data;
	int work_done = 0;
	struct ifnet *ifp = txq->lif->netdev;

	KASSERT(txq, ("txq is NULL"));
	if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
		return (IRQ_NONE);

	IONIC_TX_LOCK(txq);

	ionic_intr_mask(&txq->intr, true);
 
	work_done = ionic_tx_clean(txq, 256/* XXX: tunable */);
	IONIC_NETDEV_TX_TRACE(txq, "ionic_tx_is processed %d descriptors\n", work_done);
	
	ionic_intr_return_credits(&txq->intr, work_done, 0, true);

	// Enable interrupt.
	ionic_intr_mask(&txq->intr, false);
	IONIC_TX_UNLOCK(txq);

	return (IRQ_HANDLED);
}

static void
ionic_tx_task_handler(void *arg, int pendindg)
{
	struct txque* txq = arg;
	int err;

	KASSERT(txq, ("task handler called with txq == NULL"));

	if (drbr_empty(txq->lif->netdev, txq->br))
		return;

	IONIC_NETDEV_TX_TRACE(txq, "Enter: head: %d tail :%d\n",
		txq->cmd_head_index, txq->cmd_tail_index);

	IONIC_TX_LOCK(txq);
	/*
	 * Process all Tx frames.
	 */
	err = ionic_start_xmit_locked(txq->lif->netdev, txq);
	IONIC_TX_UNLOCK(txq);
	IONIC_NETDEV_TX_TRACE(txq, "Exit: head: %d tail :%d\n",
		txq->cmd_head_index, txq->cmd_tail_index);
}

static int ionic_lif_txq_init(struct lif *lif, struct txque *txq)
{
#ifdef RSS
	cpuset_t        cpu_mask;
#endif
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.txq_init = {
			.opcode = CMD_OPCODE_TXQ_INIT,
			.I = false,
			.E = false,
			.pid = txq->pid,
			.intr_index = txq->intr.index,
			.type = TXQ_TYPE_ETHERNET,
			.index = txq->index,
			.cos = 0,
			.ring_base = txq->cmd_ring_pa,
			.ring_size = ilog2(txq->num_descs),
		},
	};
	int err, bind_cpu;

	IONIC_QUE_INFO(txq, "qid %d pid %d index %d ring_base 0x%lx ring_size %d\n",
		ctx.comp.txq_init.qid, ctx.cmd.txq_init.pid, ctx.cmd.txq_init.index, ctx.cmd.txq_init.ring_base, ctx.cmd.txq_init.ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	txq->qid = ctx.comp.txq_init.qid;
	txq->qtype = ctx.comp.txq_init.qtype;
	txq->db  = (void *)txq->lif->ionic->idev.db_pages + (txq->pid * PAGE_SIZE);
	txq->db += txq->qtype;

	snprintf(txq->intr.name, sizeof(txq->intr.name), "%s", txq->name);

	request_irq(txq->intr.vector, ionic_tx_isr, 0, txq->intr.name, txq);

#ifdef RSS
	bind_cpu = rss_getcpu(txq->index % rss_getnumbuckets());
	CPU_SETOF(bind_cpu, &cpu_mask);
#else
	bind_cpu = txq->index;
#endif
	/* XXX: for performance regression, bind to next core. */
	err = bind_irq_to_cpu(txq->intr.vector, bind_cpu);
	if (err) {
		IONIC_QUE_WARN(txq, "failed to bind to cpu%d\n", bind_cpu);
	}
	IONIC_QUE_INFO(txq, "bound to cpu%d\n", bind_cpu);

	TASK_INIT(&txq->task, 0, ionic_tx_task_handler, txq);
    txq->taskq = taskqueue_create_fast(txq->name, M_NOWAIT,
	    taskqueue_thread_enqueue, &txq->taskq);

#ifdef RSS
    err = taskqueue_start_threads_cpuset(&txq->taskq, 1, PI_NET, &cpu_mask,
#else
	err = taskqueue_start_threads_cpuset(&txq->taskq, 1, PI_NET, NULL,
#endif
        "%s (que %d)", device_get_nameunit(lif->ionic->dev), bind_cpu);
	
	if (err) {
		IONIC_QUE_ERR(txq, "failed to create task queue, error: %d\n", err);
		taskqueue_free(txq->taskq);
		return (err);
	}

	IONIC_QUE_INFO(txq, "qid: %d qtype: %d db: %p\n",
		txq->qid, txq->qtype, txq->db);

	return 0;
}

static int ionic_lif_txqs_init(struct lif *lif)
{
	unsigned int i;
	int err;

	for (i = 0; i < lif->ntxqs; i++) {
		err = ionic_lif_txq_init(lif, lif->txqs[i]);
		if (err)
			return err;
	}

	return 0;
}

/************************************************************************
 *******************************  RX side. ******************************
 ************************************************************************/
static void ionic_rx_ring_doorbell(struct rxque *rxq, int index)
{
	IONIC_NETDEV_RX_TRACE(rxq, "ring doorbell for index: %d\n", index);
	ionic_ring_doorbell(rxq->db, rxq->qid, index);
}

/* 
 * Called at init time or from Rx clean path, submits new mbuf starting from head.
 */
static void ionic_rx_fill(struct rxque *rxq)
{
	struct rxq_desc *desc;
	struct ionic_rx_buf *rxbuf;
	int error, i, index, count = 0;

	KASSERT(IONIC_RX_OWNED(rxq), ("%s is not locked", rxq->name));

	IONIC_NETDEV_RX_TRACE(rxq, "head: %d tail :%d desc_posted: %d\n",
		rxq->cmd_head_index, rxq->cmd_tail_index, rxq->descs);

	for ( i = 0 ; i < rxq->num_descs && rxq->descs < rxq->num_descs; i++, rxq->descs++) {
		index = rxq->cmd_head_index;
		rxbuf = &rxq->rxbuf[index];
		desc = &rxq->cmd_ring[index];

		KASSERT((rxbuf->m == NULL), ("%s: ionic_rx_fill rxbuf not empty for %d",
			rxq->name, index));
		if ((error = ionic_rx_mbuf_alloc(rxq, index, rxq->lif->rx_mbuf_size))) {
			IONIC_QUE_ERR(rxq, "rx_fill mbuf alloc failed for p_index :%d, error: %d\n",
				index, error);
			break;
		}

		count++;
		desc->addr = rxbuf->pa_addr;
		desc->len = rxq->lif->rx_mbuf_size;
		desc->opcode = RXQ_DESC_OPCODE_SIMPLE;

		IONIC_QUE_INFO(rxq, "rx_fill @%d mbuf pa: 0x%lx \n", index, rxbuf->pa_addr);

		rxq->cmd_head_index = (rxq->cmd_head_index + 1) % rxq->num_descs;

		if (index % ionic_rx_stride == 0)
			ionic_rx_ring_doorbell(rxq, index);
	 }
	
	/* If we haven't ring the doorbell, do it now. */
	if (index % ionic_rx_stride)
		ionic_rx_ring_doorbell(rxq, index);
	IONIC_NETDEV_RX_TRACE(rxq, "head: %d tail :%d desc_posted: %d\n",
		rxq->cmd_head_index, rxq->cmd_tail_index, rxq->descs);
}

/* Empty Rx queue buffer, called from reinit or when queue is teardown. */
static void ionic_rx_empty(struct rxque *rxq, bool destroy)
{
	struct ionic_rx_buf *rxbuf;
	int i;

	IONIC_NETDEV_RX_TRACE(rxq, "head: %d tail :%d desc_posted: %d\n",
		rxq->cmd_head_index, rxq->cmd_tail_index, rxq->descs);
	IONIC_RX_LOCK(rxq);
	for ( i = 0 ; i < rxq->num_descs && rxq->descs; i++, rxq->descs--) {
		rxbuf = &rxq->rxbuf[rxq->cmd_tail_index];

		KASSERT(rxbuf->m, ("%s: ionic_rx_empty rxbuf empty for %d",
			rxq->name, rxq->cmd_tail_index));

		IONIC_NETDEV_RX_TRACE(rxq, "empty @%d\n", rxq->cmd_tail_index);
		ionic_rx_mbuf_free(rxq, rxbuf);

		if (destroy)
			ionic_rx_destroy_map(rxq, rxbuf);

		rxq->cmd_tail_index = (rxq->cmd_tail_index + 1) % rxq->num_descs;
	};

	IONIC_NETDEV_RX_TRACE(rxq, "head: %d tail :%d desc_posted: %d\n",
		rxq->cmd_head_index, rxq->cmd_tail_index, rxq->descs);
	IONIC_RX_UNLOCK(rxq);
}

static int ionic_rx_clean(struct rxque* rxq , int rx_limit)
{
	struct rxq_comp *comp;
	struct rxq_desc *cmd;
	struct ionic_rx_buf *rxbuf;
	int i, comp_index, cmd_index;

	KASSERT(IONIC_RX_OWNED(rxq), ("%s is not locked", rxq->name));
	IONIC_NETDEV_RX_TRACE(rxq, "comp index: %d head: %d tail :%d desc_posted: %d\n",
		rxq->comp_index, rxq->cmd_head_index, rxq->cmd_tail_index, rxq->descs);
	
	/* Sync descriptors. */
	bus_dmamap_sync(rxq->cmd_dma.dma_tag, rxq->cmd_dma.dma_map,
		BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);

	/* Process Rx descriptors for the given limit or till Que is empty. */
	for ( i = 0 ; i < rx_limit && rxq->descs; i++, rxq->descs--) {
		comp_index = rxq->comp_index;
		comp = &rxq->comp_ring[comp_index];

		if (comp->color != rxq->done_color)
			break;

		cmd_index = rxq->cmd_tail_index;
		rxbuf = &rxq->rxbuf[cmd_index];
		cmd = &rxq->cmd_ring[cmd_index];

		IONIC_NETDEV_RX_TRACE(rxq, "comp :%d cmd start: %d comp->color %d done_color %d\n",
			comp_index, cmd_index, comp->color, rxq->done_color);
		//IONIC_NETDEV_RX_TRACE(rxq, "buf[%d] opcode:%d addr:0%lx len:0x%x\n",
		//	cmd_index, cmd->opcode, cmd->addr, cmd->len);

		ionic_rx_input(rxq, rxbuf, comp, cmd);

		/* We have process this completion, peek to next for next rx_clean. */
		rxq->comp_index = (rxq->comp_index + 1) % rxq->num_descs;
		/* Roll over condition, flip color. */
		if (rxq->comp_index == 0) {
			rxq->done_color = !rxq->done_color;
		}

		rxq->cmd_tail_index = (rxq->cmd_tail_index + 1) % rxq->num_descs;
	}

	IONIC_NETDEV_RX_TRACE(rxq, "comp index: %d head: %d tail :%d desc_posted: %d processed: %d\n",
		rxq->comp_index, rxq->cmd_head_index, rxq->cmd_tail_index, rxq->descs, i);

	/* XXX: flush at the end of ISR or taskqueue handler? */
	tcp_lro_flush_all(&rxq->lro);

	if ((rxq->num_descs - rxq->descs) >= ionic_rx_fill_threshold)
		ionic_rx_fill(rxq);

	return (i);
}

static void
ionic_rx_task_handler(void *arg, int pendindg)
{
	struct rxque* rxq = arg;
	int processed;

	KASSERT(rxq, ("task handler called with rxq == NULL"));
	IONIC_RX_LOCK(rxq);

	rxq->stats.task++;
	IONIC_NETDEV_RX_TRACE(rxq, "comp index: %d head: %d tail :%d\n",
		rxq->comp_index, rxq->cmd_head_index, rxq->cmd_tail_index);
	/* 
	 * Process all Rx frames.
	 */
	processed = ionic_rx_clean(rxq, rxq->num_descs);
	IONIC_NETDEV_RX_TRACE(rxq, "processed %d packets\n", processed);

	KASSERT(processed, ("%s task handler didn't processed any packets", rxq->name));

	/* Reset the time only in ISR path. */
	ionic_intr_return_credits(&rxq->intr, processed, 0, false);

	IONIC_RX_UNLOCK(rxq);
}

static irqreturn_t ionic_rx_isr(int irq, void *data)
{
	struct rxque* rxq = data;
	struct ifnet *ifp = rxq->lif->netdev;
	struct rx_stats* rxstats = &rxq->stats;
	struct rxq_comp *comp;
	int work_done = 0;

	KASSERT(rxq, ("rxq is NULL"));
	KASSERT(ifp, ("%s ifp == NULL", rxq->name));

	/* Protect against spurious interrupts */
	if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
		return (IRQ_NONE);

	IONIC_RX_LOCK(rxq);

	IONIC_NETDEV_RX_TRACE(rxq, "comp index: %d head: %d tail :%d\n",
		rxq->comp_index, rxq->cmd_head_index, rxq->cmd_tail_index);
	
	ionic_intr_mask(&rxq->intr, true);

	rxstats->isr_count++;

	work_done = ionic_rx_clean(rxq, ionic_rx_process_limit);
	IONIC_NETDEV_RX_TRACE(rxq, "processed %d packets\n", work_done);

	ionic_intr_return_credits(&rxq->intr, work_done, 0, true);

	/* If here are more packet to process, schedule the task handler. */
	comp = &rxq->comp_ring[rxq->comp_index];
	if (comp->color == rxq->done_color)
		taskqueue_enqueue(rxq->taskq, &rxq->task);

	// Enable interrupt.
	ionic_intr_mask(&rxq->intr, false);
	IONIC_RX_UNLOCK(rxq);

	return (IRQ_HANDLED);
}


static int ionic_lif_rxq_init(struct lif *lif, struct rxque *rxq)
{
	struct lro_ctrl *lro = &rxq->lro;
#ifdef RSS
	cpuset_t        cpu_mask;
#endif
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rxq_init = {
			.opcode = CMD_OPCODE_RXQ_INIT,
			.I = false,
			.E = false,
			.pid = rxq->pid,
			.intr_index = rxq->intr.index,
			.type = RXQ_TYPE_ETHERNET,
			.index = rxq->index,
			.ring_base = rxq->cmd_ring_pa,
			.ring_size = ilog2(rxq->num_descs),
		},
	};
	int err, bind_cpu;

	IONIC_QUE_INFO(rxq, "pid %d index %d ring_base 0x%lx ring_size %d\n",
		 ctx.cmd.rxq_init.pid, ctx.cmd.rxq_init.index, ctx.cmd.rxq_init.ring_base, ctx.cmd.rxq_init.ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	rxq->qid = ctx.comp.rxq_init.qid;
	rxq->qtype = ctx.comp.rxq_init.qtype;

	/* XXX: move to be part of ring doorbell */
	rxq->db  = (void *)rxq->lif->ionic->idev.db_pages + (rxq->pid * PAGE_SIZE);
	rxq->db += rxq->qtype;
	IONIC_QUE_INFO(rxq, "doorbell: %p\n", rxq->db);

	snprintf(rxq->intr.name, sizeof(rxq->intr.name), "%s", rxq->name);
	request_irq(rxq->intr.vector, ionic_rx_isr, 0, rxq->intr.name, rxq);

#ifdef RSS
	bind_cpu = rss_getcpu(rxq->index % rss_getnumbuckets());
	CPU_SETOF(bind_cpu, &cpu_mask);
#else
	bind_cpu = rxq->index;
#endif
	err = bind_irq_to_cpu(rxq->intr.vector, bind_cpu);
	if (err) {
		IONIC_QUE_WARN(rxq, "failed to bindto cpu%d\n", bind_cpu);
	}
	IONIC_QUE_INFO(rxq, "bound to cpu%d\n", bind_cpu);

	TASK_INIT(&rxq->task, 0, ionic_rx_task_handler, rxq);
    rxq->taskq = taskqueue_create_fast(rxq->intr.name, M_NOWAIT,
	    taskqueue_thread_enqueue, &rxq->taskq);

#ifdef RSS
    err = taskqueue_start_threads_cpuset(&rxq->taskq, 1, PI_NET, &cpu_mask,
#else
    err = taskqueue_start_threads_cpuset(&rxq->taskq, 1, PI_NET, NULL,
#endif
        "%s (que %d)", device_get_nameunit(lif->ionic->dev), bind_cpu);

	if (err) {
		IONIC_QUE_ERR(rxq, "failed to create task queue, error: %d\n",
			err);
		taskqueue_free(rxq->taskq);
		return (err);
	}

	err = tcp_lro_init(lro);
	if (err) {
		IONIC_QUE_ERR(rxq, "LRO setup failed, error: %d\n", err);
	} else {
		lro->ifp = lif->netdev;
	}

	IONIC_QUE_INFO(rxq, "qid: %d qtype: %d db:%p\n", rxq->qid, rxq->qtype, rxq->db);

	return 0;
}

static int ionic_lif_rxqs_init(struct lif *lif)
{
	unsigned int i;
	int err;

	for (i = 0; i < lif->nrxqs; i++) {
		err = ionic_lif_rxq_init(lif, lif->rxqs[i]);
		if (err)
			return err;
	}

	return 0;
}

static void
ionic_media_status(struct ifnet *ifp, struct ifmediareq *ifmr)
{
	ifmr->ifm_status = IFM_AVALID;
	ifmr->ifm_active = IFM_ETHER;

	/* XXX: if link is not up return */

	ifmr->ifm_status |= IFM_ACTIVE;
	ifmr->ifm_active |= IFM_FDX;

	/* XXX: detect media type */
	ifmr->ifm_active |= IFM_100G_CR4;
}

static int
ionic_media_change(struct ifnet *ifp)
{
	struct lif* lif = ifp->if_softc;
	struct ifmedia *ifm = &lif->media;

	if (IFM_TYPE(ifm->ifm_media) != IFM_ETHER)
		return (EINVAL);

	if_printf(ifp, "Changing speed is not supported\n");

	return (ENODEV);
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

	if (!is_zero_ether_addr(lif->dev_addr)) {
		IONIC_NETDEV_INFO(netdev, "deleting station MAC addr %pM\n",
			   lif->dev_addr);
		ether_ifdetach(netdev);
		ionic_lif_addr(lif, lif->dev_addr, false);
	}
	memcpy(lif->dev_addr, ctx.comp.station_mac_addr_get.addr,
	       ETHER_ADDR_LEN);
	IONIC_NETDEV_INFO(netdev, "adding station MAC addr %pM\n",
		   lif->dev_addr);
	ionic_lif_addr(lif, lif->dev_addr, true);
	ether_ifattach(netdev, lif->dev_addr);

	lif->max_frame_size = netdev->if_mtu + ETHER_HDR_LEN + ETHER_CRC_LEN;

	ifmedia_init(&lif->media, IFM_IMASK, ionic_media_change,
	    ionic_media_status);

	/* XXX: add more media type */
	ifmedia_add(&lif->media, IFM_ETHER | IFM_25G_SR, 0, NULL);
	ifmedia_add(&lif->media, IFM_ETHER | IFM_40G_SR4, 0, NULL);
	ifmedia_add(&lif->media, IFM_ETHER | IFM_100G_SR4, 0, NULL);
	ifmedia_add(&lif->media, IFM_ETHER | IFM_AUTO, 0, NULL);

	ifmedia_set(&lif->media, IFM_ETHER | IFM_AUTO);

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
#ifdef notyet
	err = ionic_lif_stats_dump_start(lif, STATS_DUMP_VERSION_1);
	if (err)
		goto err_out_rss_teardown;
#endif
	ionic_set_rx_mode(lif->netdev);

	ionic_setup_sysctls(lif);

	lif->api_private = NULL;

	return 0;

//err_out_rss_teardown:
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

/*
 * Configure the NIC for required capabilities.
 */
int ionic_set_hw_feature(struct lif *lif, uint16_t set_feature)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.features = {
			.opcode = CMD_OPCODE_FEATURES,
			.set = FEATURE_SET_ETH_HW_FEATURES,
			.wanted = set_feature,
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	/* XXX: lif->hw_features = ctx.comp.features.supported; ???*/
	lif->hw_features = ctx.cmd.features.wanted &
			   ctx.comp.features.supported;

	if (lif->hw_features & ETH_HW_VLAN_TX_TAG)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_VLAN_TX_TAG\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_STRIP)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_VLAN_RX_STRIP\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_FILTER)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_VLAN_RX_FILTER\n");
	if (lif->hw_features & ETH_HW_RX_HASH)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_RX_HASH\n");
	if (lif->hw_features & ETH_HW_TX_SG)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TX_SG\n");
	if (lif->hw_features & ETH_HW_TX_CSUM)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TX_CSUM\n");
	if (lif->hw_features & ETH_HW_RX_CSUM)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_RX_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO\n");
	if (lif->hw_features & ETH_HW_TSO_IPV6)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO_IPV6\n");
	if (lif->hw_features & ETH_HW_TSO_ECN)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO_ECN\n");
	if (lif->hw_features & ETH_HW_TSO_GRE)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO_GRE\n");
	if (lif->hw_features & ETH_HW_TSO_GRE_CSUM)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO_GRE_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP4)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO_IPXIP4\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP6)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO_IPXIP6\n");
	if (lif->hw_features & ETH_HW_TSO_UDP)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO_UDP\n");
	if (lif->hw_features & ETH_HW_TSO_UDP_CSUM)
		IONIC_NETDEV_INFO(lif->netdev, "feature ETH_HW_TSO_UDP_CSUM\n");

	if (ctx.cmd.features.wanted != ctx.comp.features.supported) {
		IONIC_NETDEV_WARN(lif->netdev, "Feature wanted 0x%X != enabled 0x%X\n", ctx.cmd.features.wanted, ctx.comp.features.supported);
	}

	return 0;
}

int ionic_set_features(struct lif *lif, uint16_t set_features)
{
	//struct net_device *netdev = lif->netdev;
	int err;

	IONIC_NETDEV_INFO(lif->netdev, "Setting capabilities: 0x%x\n", set_features);
	err = ionic_set_hw_feature(lif, set_features);
	if (err)
		return err;

#ifdef FREEBSD
	ionic_set_os_features(lif->netdev, lif->hw_features);
#else
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
#endif

	return 0;
}

static int ionic_lif_register(struct lif *lif)
{
	int err;

	err = ionic_set_features(lif,
				 ETH_HW_VLAN_TX_TAG
				| ETH_HW_VLAN_RX_STRIP
				| ETH_HW_VLAN_RX_FILTER
				| ETH_HW_RX_HASH
				| ETH_HW_TX_SG
				| ETH_HW_TX_CSUM
				| ETH_HW_RX_CSUM
				| ETH_HW_TSO
				| ETH_HW_TSO_IPV6);

	if (err)
		return err;

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
			lif->registered = false;
		}
	}
}

int ionic_lifs_size(struct ionic *ionic)
{
	union identity *ident = ionic->ident;
	unsigned int nlifs = ident->dev.nlifs;
	unsigned int neqs = ident->dev.neqs_per_lif;
	/* Tx and Rx Qs are in pair. */
	int nqs = min(ident->dev.ntxqs_per_lif, ident->dev.nrxqs_per_lif);
	unsigned int nintrs, dev_nintrs = ident->dev.nintrs;
	int err;

	if (ionic_max_queues && (nqs < ionic_max_queues))
		nqs = ionic_max_queues;

	dev_info(ionic->dev, "dev_nintrs %u\n", dev_nintrs);
try_again:
#ifdef RSS
	/*
	 * Max number of Qs can't be more than number of RSS buckets,
	 * since those Qs will not get any traffic.
	 */
	if (nqs > rss_getnumbuckets()) {
		nqs = rss_getnumbuckets();
		dev_info(ionic->dev, "reduced number of Qs to %u based on RSS buckets\n", nqs);
	} 
#endif

	nintrs = nlifs * (neqs + 2 * nqs + 1 /* adminq */);
	if (nintrs > dev_nintrs) {
		goto try_fewer;
	}

	err = ionic_bus_alloc_irq_vectors(ionic, nintrs);
	if (err < 0 && err != -ENOSPC) {
		return err;
	}

	if (err == -ENOSPC)
		goto try_fewer;

	if (err != nintrs) {
		ionic_bus_free_irq_vectors(ionic);
		goto try_fewer;
	}

	ionic->neqs_per_lif = neqs;
	ionic->ntxqs_per_lif = nqs;
	ionic->nrxqs_per_lif = nqs;
	ionic->nintrs = nintrs;

	dev_info(ionic->dev, "dev_nintrs %d Tx/Rx Qs: %d\n", dev_nintrs, nqs);
	ionic_max_queues = nqs;

	return 0;
try_fewer:
	if (neqs > 1) {
		neqs /= 2;
		goto try_again;
	}
	if (nqs > 1) {
		nqs /= 2;
		goto try_again;
	}

	return -ENOSPC;
}
