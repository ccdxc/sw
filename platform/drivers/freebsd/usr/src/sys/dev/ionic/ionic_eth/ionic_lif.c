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


void ionic_open(void *arg)
{
	struct lif *lif = arg;
	struct rx_qcq *rxqcq;
	struct tx_qcq *txqcq;
	unsigned int i;
	int err;

	for (i = 0; i < lif->ntxqcqs; i++) {
		txqcq = lif->txqcqs[i];
		mtx_lock(&txqcq->mtx);
		err = ionic_q_enable_disable(lif, &txqcq->intr, txqcq->qid, txqcq->qtype, true /* enable */);
		WARN_ON(err);
		mtx_unlock(&txqcq->mtx);
		//napi_enable(&lif->txqcqs[i]->napi);
	}

	for (i = 0; i < lif->nrxqcqs; i++) {
		rxqcq = lif->rxqcqs[i];
		mtx_lock(&rxqcq->mtx);
		ionic_rx_fill(rxqcq);
		err = ionic_q_enable_disable(lif, &rxqcq->intr, rxqcq->qid, rxqcq->qtype, true /* enable */);
		WARN_ON(err);
		mtx_unlock(&rxqcq->mtx);
	}

	ionic_up_link(lif->netdev);
}


static int ionic_stop(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	struct rx_qcq* rxqcq;
	struct tx_qcq* txqcq;
	unsigned int i;
	int err;


	for (i = 0; i < lif->ntxqcqs; i++) {
		// TODO post NOP Tx desc and wait for its completion
		// TODO before disabling Tx queue
		txqcq = lif->txqcqs[i];
		mtx_lock(&txqcq->mtx);
		err = ionic_q_enable_disable(lif, &txqcq->intr, txqcq->qid, txqcq->qtype, false /* disable */);
		if (err)
			return err;
		
		mtx_unlock(&txqcq->mtx);
	}

	for (i = 0; i < lif->nrxqcqs; i++) {
		rxqcq = lif->rxqcqs[i];
		mtx_lock(&rxqcq->mtx);
		err = ionic_q_enable_disable(lif, &rxqcq->intr, rxqcq->qid, rxqcq->qtype, false /* disable */);
		if (err) {
			/* XXX: should we continue? */
		}
		ionic_rx_flush(rxqcq);
		mtx_unlock(&rxqcq->mtx);
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

static void ionic_adminq_napi(struct napi_struct *napi)
{
	int budget = NAPI_POLL_WEIGHT;
	int work_done;

	work_done = ionic_napi(napi, budget, ionic_adminq_service, NULL);

	if (work_done == budget)
		napi_schedule(napi);
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

	IONIC_NETDEV_INFO(lif->netdev, "rx_filter %s %pM\n",
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

	IONIC_DEV_TRACE(lif->ionic->dev, "%02x:%02x:%02x:%02x:%02x:%02x %s\n",
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
		IONIC_NETDEV_INFO(lif->netdev, "deferred: rx_filter %s %pM\n",
			   add ? "add" : "del", addr);
		queue_work(lif->adminq_wq, &work->work);
	} else {
		return _ionic_lif_addr(lif, addr, add);
	}

	return 0;
}

static int ionic_addr_add(struct net_device *netdev, const u8 *addr)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEV_TRACE(lif->ionic->dev, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	return ionic_lif_addr(netdev_priv(netdev), addr, true);
}

static int ionic_addr_del(struct net_device *netdev, const u8 *addr)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEV_TRACE(lif->ionic->dev, "%02x:%02x:%02x:%02x:%02x:%02x\n",
		  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

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

	IONIC_DEV_TRACE(lif->ionic->dev, "%#x\n", rx_mode);

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

	IONIC_DEV_TRACE(lif->ionic->dev, "%#x\n", rx_mode);

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

	IONIC_DEV_TRACE(lif->ionic->dev, "\n");

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

int ionic_reinit_unlock(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);

	IONIC_DEV_TRACE(lif->ionic->dev, "reinit\n");

	if (netif_running(netdev))
		ionic_stop(netdev);


#if 0 // XXX: Why to reallocate mbufs???
	for (i = 0; i < lif->nrxqcqs; i++)
		ionic_rx_refill(lif->rxqcqs[i]);
#endif
	ionic_open(lif);

	return (0);
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
	int i, err;

	IONIC_DEV_TRACE(lif->ionic->dev, "\n");

	if (netif_running(netdev))
		ionic_stop(netdev);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	if_setmtu(netdev, new_mtu);


	for (i = 0; i < lif->nrxqcqs; i++)
		ionic_rx_refill(lif->rxqcqs[i]);


	if (netif_running(netdev))
		ionic_open(lif);

	return 0;
}

static void ionic_tx_timeout(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEV_TRACE(lif->ionic->dev, "\n");

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

	IONIC_DEV_TRACE(lif->ionic->dev, "\n");

	IONIC_NETDEV_INFO(netdev, "rx_filter %s VLAN %d\n", add ? "add" : "del", vid);

	return ionic_adminq_post_wait(lif, &ctx);
}

static int ionic_vlan_rx_add_vid(struct net_device *netdev,
				 __be16 proto, u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEV_TRACE(lif->ionic->dev, "\n");

	return ionic_vlan_rx_filter(netdev, true, proto, vid);
}

static int ionic_vlan_rx_kill_vid(struct net_device *netdev,
				  __be16 proto, u16 vid)
{
	struct lif *lif = netdev_priv(netdev);
	IONIC_DEV_TRACE(lif->ionic->dev, "\n");

	return ionic_vlan_rx_filter(netdev, false, proto, vid);
}

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

	new = kzalloc(sizeof(*new), GFP_KERNEL);
	if (!new)
		return -ENOMEM;

	new->flags = flags;

	new->q.info = kzalloc(sizeof(*new->q.info) * num_descs, GFP_KERNEL);
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

	new->cq.info = kzalloc(sizeof(*new->cq.info) * num_descs, GFP_KERNEL);
	if (!new->cq.info)
		return -ENOMEM;

	err = ionic_cq_init(lif, &new->cq, &new->intr,
			    num_descs, cq_desc_size);
	if (err)
		goto err_out_free_intr;

	new->base = dma_zalloc_coherent(dev, total_size, &new->base_pa,
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

static int ionic_rx_qcq_alloc(struct lif *lif, unsigned int qnum,
			   unsigned int num_descs, 
			   unsigned int pid, struct rx_qcq **prxqcq)
{
	struct rx_qcq *rxqcq;
	struct ionic_rx_buf *rxbuf;
	int i, irq, error = ENOMEM; 
	uint32_t cmd_ring_size, comp_ring_size, total_size;

	*prxqcq = NULL;

	rxqcq = malloc(sizeof(*rxqcq), M_IONIC, M_NOWAIT | M_ZERO);
	if(rxqcq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev, "failed to allocate rxq%d\n", qnum);
		return (error);
	}

	snprintf(rxqcq->name, sizeof(rxqcq->name) - 1, "RxQ%d", qnum);
	rxqcq->lif = lif;
	rxqcq->index = qnum;
	rxqcq->num_descs = num_descs;
	rxqcq->pid = pid;
	rxqcq->done_color = 1;

	mtx_init(&rxqcq->mtx, rxqcq->name, NULL, MTX_DEF);

	/* rx buffer and command are in tendom */
	rxqcq->cmd_head_index = rxqcq->cmd_tail_index = 0;
	rxqcq->comp_index = 0;

	/* Setup command ring. */
	rxqcq->rxbuf = malloc(sizeof(*rxqcq->rxbuf) * num_descs, M_IONIC, M_NOWAIT | M_ZERO);
	if (rxqcq->rxbuf == NULL) {
		IONIC_NETDEV_QERR(rxqcq, "Couldn't allocate rx buffer descriptors\n");
		goto failed_alloc;
	}

	/* Allocate DMA for command and completion rings. They must be consecutive. */
	cmd_ring_size = sizeof(*rxqcq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*rxqcq->comp_ring) * num_descs;
	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(cmd_ring_size, PAGE_SIZE);
		 
	if ((error = ionic_dma_alloc(rxqcq->lif->ionic, total_size, &rxqcq->cmd_dma, BUS_DMA_NOWAIT))) {
		IONIC_NETDEV_QERR(rxqcq, "failed to allocated DMA cmd ring, err: %d\n", error);
		goto failed_alloc;
	}

	rxqcq->cmd_ring_pa = rxqcq->cmd_dma.dma_paddr;
	rxqcq->cmd_ring = (struct rxq_desc *)rxqcq->cmd_dma.dma_vaddr;
	IONIC_NETDEV_QINFO(rxqcq, "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
		rxqcq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * We assume that competion ring is next to command ring.
	 */
	rxqcq->comp_ring = (struct rxq_comp *)(rxqcq->cmd_dma.dma_vaddr + ALIGN(cmd_ring_size, PAGE_SIZE));

	bzero((void *)rxqcq->cmd_ring, total_size);

	/* Setup interrupt */
	error = ionic_intr_alloc(lif, &rxqcq->intr);
	if (error) {
		IONIC_NETDEV_QERR(rxqcq, "no available interrupt, error: %d\n", error);
		goto failed_alloc;
	}

	irq = ionic_bus_get_irq(lif->ionic, rxqcq->intr.index);
	if (irq < 0) {
		IONIC_NETDEV_QERR(rxqcq, "no available IRQ, error: %d\n", error);
		goto failed_alloc;
	}

	rxqcq->intr.vector = irq;
	ionic_intr_mask_on_assertion(&rxqcq->intr);
	
	/* 
	 * Create just one tag for Rx bufferes. 
	 */
	error = bus_dma_tag_create(
	         /*      parent */ bus_get_dma_tag(rxqcq->lif->ionic->dev->bsddev),
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
	                           &rxqcq->buf_tag);

	if (error) {
		IONIC_NETDEV_QERR(rxqcq, "failed to create DMA tag, err: %d\n", error);
		goto free_intr;
	}

	for ( rxbuf = rxqcq->rxbuf, i = 0 ; rxbuf != NULL && i < num_descs; i++, rxbuf++ ) {
		error = bus_dmamap_create(rxqcq->buf_tag, 0, &rxbuf->dma_map);
		if (error) {	
			IONIC_NETDEV_QERR(rxqcq, "failed to create map for entry%d, err: %d\n", i, error);
			bus_dma_tag_destroy(rxqcq->buf_tag);
			goto free_intr;
		}
	}

	*prxqcq = rxqcq;

	return 0;

free_intr:
	ionic_intr_free(lif, &rxqcq->intr);

failed_alloc:
	if (rxqcq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(rxqcq->lif->ionic, &rxqcq->cmd_dma);
		rxqcq->cmd_ring = NULL;
		rxqcq->comp_ring = NULL;
	}

	if (rxqcq->rxbuf) {
		free(rxqcq->rxbuf, M_IONIC);
		rxqcq->rxbuf = NULL;
	}
	
	free(rxqcq, M_IONIC);

	return (error);
}

static int ionic_tx_qcq_alloc(struct lif *lif, unsigned int qnum,
			   unsigned int num_descs, 
			   unsigned int pid, struct tx_qcq **ptxqcq)
{
	struct tx_qcq *txqcq;
	struct ionic_tx_buf *txbuf;
	int i, irq, error = ENOMEM; 
	uint32_t cmd_ring_size, comp_ring_size, sg_ring_size, total_size;

	*ptxqcq = NULL;

	txqcq = malloc(sizeof(*txqcq), M_IONIC, M_NOWAIT | M_ZERO);
	if(txqcq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev, "failed to allocate rxq%d\n", qnum);
		return (error);
	}

	snprintf(txqcq->name, sizeof(txqcq->name) - 1, "TxQ%d", qnum);
	txqcq->lif = lif;
	txqcq->index = qnum;
	txqcq->num_descs = num_descs;
	txqcq->pid = pid;
	txqcq->done_color = 1;

	mtx_init(&txqcq->mtx, txqcq->name, NULL, MTX_DEF);

	/* rx buffer and command are in tendom */
	txqcq->cmd_head_index = txqcq->cmd_tail_index = 0;
	txqcq->sg_head_index = txqcq->sg_tail_index = 0;
	txqcq->comp_index = 0;

	/* Setup command ring. */
	txqcq->txbuf = malloc(sizeof(*txqcq->txbuf) * num_descs, M_IONIC, M_NOWAIT | M_ZERO);
	if (txqcq->txbuf == NULL) {
		IONIC_NETDEV_QERR(txqcq, "Couldn't allocate tx buffer descriptors\n");
		goto failed_alloc;
	}

	/* Allocate DMA for command and completion rings. They must be consecutive. */
	cmd_ring_size = sizeof(*txqcq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*txqcq->comp_ring) * num_descs;
	sg_ring_size = sizeof(*txqcq->sg_ring) * num_descs;
	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(sg_ring_size, PAGE_SIZE);
		 
	if ((error = ionic_dma_alloc(txqcq->lif->ionic, total_size, &txqcq->cmd_dma, BUS_DMA_NOWAIT))) {
		IONIC_NETDEV_QERR(txqcq, "failed to allocated DMA cmd ring, err: %d\n", error);
		goto failed_alloc;
	}

	txqcq->cmd_ring_pa = txqcq->cmd_dma.dma_paddr;
	txqcq->cmd_ring = (struct txq_desc *)txqcq->cmd_dma.dma_vaddr;
	IONIC_NETDEV_QINFO(txqcq, "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
		txqcq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * We assume that competion ring is next to command ring.
	 */
	txqcq->comp_ring = (struct txq_comp *)(txqcq->cmd_dma.dma_vaddr + ALIGN(cmd_ring_size, PAGE_SIZE));
	txqcq->sg_ring = (struct txq_sg_desc *)(txqcq->cmd_dma.dma_vaddr + ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(comp_ring_size, PAGE_SIZE));

	bzero((void *)txqcq->cmd_ring, total_size);

	/* Allocate buffere ring. */
	txqcq->br = buf_ring_alloc(4096, M_IONIC, M_WAITOK, &txqcq->mtx);
	if (txqcq->br == NULL) {
		IONIC_NETDEV_QERR(txqcq, "failed to allocated buffer ring\n");
		goto failed_alloc;
	}

	/* Setup interrupt */
	error = ionic_intr_alloc(lif, &txqcq->intr);
	if (error) {
		IONIC_NETDEV_QERR(txqcq, "no available interrupt, error: %d\n", error);
		goto failed_alloc;
	}

	irq = ionic_bus_get_irq(lif->ionic, txqcq->intr.index);
	if (irq < 0) {
		IONIC_NETDEV_QERR(txqcq, "no available IRQ, error: %d\n", error);
		goto failed_alloc;
	}

	txqcq->intr.vector = irq;
	ionic_intr_mask_on_assertion(&txqcq->intr);
	
	/* 
	 * Create just one tag for Rx bufferrs. 
	 */
	error = bus_dma_tag_create(
	         /*      parent */ bus_get_dma_tag(txqcq->lif->ionic->dev->bsddev),
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
	                           &txqcq->buf_tag);

	if (error) {
		IONIC_NETDEV_QERR(txqcq, "failed to create DMA tag, err: %d\n", error);
		goto free_intr;
	}

	for ( txbuf = txqcq->txbuf, i = 0 ; txbuf != NULL && i < num_descs; i++, txbuf++ ) {
		error = bus_dmamap_create(txqcq->buf_tag, 0, &txbuf->dma_map);
		if (error) {	
			IONIC_NETDEV_QERR(txqcq, "failed to create map for entry%d, err: %d\n", i, error);
			bus_dma_tag_destroy(txqcq->buf_tag);
			goto free_intr;
		}
	}

	IONIC_NETDEV_QINFO(txqcq, "create txq\n");
	*ptxqcq = txqcq;

	return 0;

free_intr:
	ionic_intr_free(lif, &txqcq->intr);

failed_alloc:
	if (txqcq->br) {
		buf_ring_free(txqcq->br, M_IONIC);
		txqcq->br = NULL;
	}

	if (txqcq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(txqcq->lif->ionic, &txqcq->cmd_dma);
		txqcq->cmd_ring = NULL;
		txqcq->comp_ring = NULL;
		txqcq->sg_ring = NULL;
	}

	if (txqcq->txbuf) {
		free(txqcq->txbuf, M_IONIC);
		txqcq->txbuf = NULL;
	}
	
	free(txqcq, M_IONIC);

	return (error);
}

static void ionic_rxqcq_free(struct lif *lif, struct rx_qcq *rxqcq)
{

	mtx_lock(&rxqcq->mtx);
	if (rxqcq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(rxqcq->lif->ionic, &rxqcq->cmd_dma);
		rxqcq->cmd_ring = NULL;
		rxqcq->comp_ring = NULL;
	}

	ionic_intr_free(lif, &rxqcq->intr);
	mtx_unlock(&rxqcq->mtx);
	mtx_destroy(&rxqcq->mtx);
}

static void ionic_txqcq_free(struct lif *lif, struct tx_qcq *txqcq)
{

	mtx_lock(&txqcq->mtx);
	if (txqcq->cmd_ring) {
		/* completion ring is part of command ring allocation. */
		ionic_dma_free(txqcq->lif->ionic, &txqcq->cmd_dma);
		txqcq->cmd_ring = NULL;
		txqcq->comp_ring = NULL;
	}

	ionic_intr_free(lif, &txqcq->intr);
	mtx_unlock(&txqcq->mtx);
	mtx_destroy(&txqcq->mtx);
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
	unsigned int flags;
	unsigned int pid;
	unsigned int i;
	int err = -ENOMEM;

	lif->txqcqs = kzalloc(sizeof(*lif->txqcqs) * lif->ntxqcqs, GFP_KERNEL);
	if (!lif->txqcqs)
		return -ENOMEM;

	lif->rxqcqs = kzalloc(sizeof(*lif->rxqcqs) * lif->nrxqcqs, GFP_KERNEL);
	if (!lif->rxqcqs)
		return -ENOMEM;

	pid = ionic_pid_get(lif, 0);
	flags = QCQ_F_INTR;
	/* XXX: we are tight on name description */
	err = ionic_qcq_alloc(lif, 0, "adq", flags, 1 << 4,
			      sizeof(struct admin_cmd),
			      sizeof(struct admin_comp),
			      0, pid, &lif->adminqcq);
	if (err)
		return err;

	pid = ionic_pid_get(lif, 0);
	for (i = 0; i < lif->ntxqcqs; i++) {
		err = ionic_tx_qcq_alloc(lif, i, ntxq_descs, pid, &lif->txqcqs[i]);
		if (err)
			goto err_out_free_adminqcq;
	}

	pid = ionic_pid_get(lif, 0);
	for (i = 0; i < lif->nrxqcqs; i++) {
		err = ionic_rx_qcq_alloc(lif, i, nrxq_descs, pid, &lif->rxqcqs[i]);
		if (err)
			goto err_out_free_txqcqs;
	}

	return 0;

err_out_free_txqcqs:
	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_txqcq_free(lif, lif->txqcqs[i]);
err_out_free_adminqcq:
	ionic_qcq_free(lif, lif->adminqcq);

	return err;
}

static void ionic_qcqs_free(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < lif->nrxqcqs; i++) {
		ionic_rx_empty(lif->rxqcqs[i]);
		ionic_rxqcq_free(lif, lif->rxqcqs[i]);
	}
	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_txqcq_free(lif, lif->txqcqs[i]);

	ionic_qcq_free(lif, lif->adminqcq);

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
	lif->ntxqcqs = ionic->ntxqs_per_lif;
	lif->nrxqcqs = ionic->nrxqs_per_lif;

	err = ionic_lif_netdev_alloc(lif, ntxq_descs);
	if (err) {
		dev_err(dev, "Cannot allocate netdev, aborting\n");
		return (err);
	}

	spin_lock_init(&lif->adminq_lock);
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

/* From ionic_ethtool.c */
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



static void ionic_lif_qcq_deinit(struct lif *lif, struct qcq *qcq)
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
	struct tx_qcq* txqcq;

	for (i = 0; i < lif->nrxqcqs; i++) {
		txqcq = lif->txqcqs[i];
		
		mtx_lock(&txqcq->mtx);

		ionic_intr_mask(&txqcq->intr, true);
		free_irq(txqcq->intr.vector, txqcq);

		if (txqcq->taskq)
			taskqueue_free(txqcq->taskq);
		
		mtx_unlock(&txqcq->mtx);
	}
}

static void ionic_lif_rxqs_deinit(struct lif *lif)
{
	unsigned int i;
	struct rx_qcq* rxqcq;

	for (i = 0; i < lif->nrxqcqs; i++) {
#ifdef IONIC_NAPI
		ionic_lif_qcq_deinit(lif, lif->rxqcqs[i]);
#else
		rxqcq = lif->rxqcqs[i];
		
		mtx_lock(&rxqcq->mtx);
		tcp_lro_free(&rxqcq->lro);

		ionic_intr_mask(&rxqcq->intr, true);
		free_irq(rxqcq->intr.vector, rxqcq);
		
		if (rxqcq->taskq)
			taskqueue_free(rxqcq->taskq);

		mtx_unlock(&rxqcq->mtx);
#endif
	}
}

static void ionic_lif_deinit(struct lif *lif)
{
	ether_ifdetach(lif->netdev);
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
	struct intr *intr = &qcq->intr;
	struct queue *q = &qcq->q;
	struct napi_struct *napi = &qcq->napi;

	/* XXX: FreeBSD bug, name is not used in request_irq. */
	/*
	 * Device name is already part of description, just put Q name.
	 * 
	 */
	snprintf(intr->name, sizeof(intr->name),
		 "%s", q->name);
		//"%s-%s-%s", DRV_NAME, lif->name, q->name);
	IONIC_NETDEV_INFO(lif->netdev, "Intr name:%s\n", intr->name);
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

int ionic_tx_clean(struct tx_qcq* txqcq , int tx_limit)
{
	struct txq_comp *comp;
	struct txq_desc *cmd;
	struct ionic_tx_buf *txbuf;
	int comp_index, cmd_index, processed, cmd_stop_index;

	for ( processed = 0 ; processed < tx_limit ; ) {
		comp_index = txqcq->comp_index;
		comp = &txqcq->comp_ring[comp_index];
		/* Sync every time descriptors. */
		bus_dmamap_sync(txqcq->cmd_dma.dma_tag, txqcq->cmd_dma.dma_map,
			BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);

		cmd_stop_index = comp->comp_index;
		cmd_index = txqcq->cmd_tail_index;
		txbuf = &txqcq->txbuf[cmd_index];
		cmd = &txqcq->cmd_ring[cmd_index];
			
		if (comp->color != txqcq->done_color)
			break;
		
		IONIC_NETDEV_QINFO(txqcq, "comp :%d cmd start: %d cmd stop: %d comp->color %d done_color %d\n",
			comp_index, cmd_index, cmd_stop_index, comp->color, txqcq->done_color);
		IONIC_NETDEV_QINFO(txqcq, "buf[%d] opcode:%d addr:0%lx len:0x%x\n",
			cmd_index, cmd->opcode, cmd->addr, cmd->len);

		for ( ; cmd_index == cmd_stop_index; cmd_index++, processed++ ) {
			txbuf = &txqcq->txbuf[cmd_index];
			cmd = &txqcq->cmd_ring[cmd_index];
			bus_dmamap_sync(txqcq->buf_tag, txbuf->dma_map, BUS_DMASYNC_POSTWRITE);
			bus_dmamap_unload(txqcq->buf_tag, txbuf->dma_map);
			m_freem(txbuf->m);
		} 

		txqcq->comp_index = (txqcq->comp_index + 1) % txqcq->num_descs;
		txqcq->cmd_tail_index = (txqcq->cmd_tail_index + 1) % txqcq->num_descs;
		/* Roll over condition, flip color. */
		if (txqcq->comp_index == 0) {
			txqcq->done_color = !txqcq->done_color;
		}
	}

	IONIC_NETDEV_QINFO(txqcq, "ionic_tx_clean processed %d\n", processed);

	if (comp->color == txqcq->done_color)
		taskqueue_enqueue(txqcq->taskq, &txqcq->task);


	return (processed);

}


static irqreturn_t ionic_tx_isr(int irq, void *data)
{
	struct tx_qcq* txqcq = data;
	//struct tx_stats* txstats = &txqcq->stats;
	int work_done = 0;

	mtx_lock(&txqcq->mtx);
	//txstats->isr_count++;

	ionic_intr_mask(&txqcq->intr, true);
 
	work_done = ionic_tx_clean(txqcq, 256/* XXX: tunable */);
	
	ionic_intr_return_credits(&txqcq->intr, work_done, 0, true);

	// Enable interrupt.
	ionic_intr_mask(&txqcq->intr, false);
	mtx_unlock(&txqcq->mtx);

	return IRQ_HANDLED;
}

static void
ionic_tx_task_handler(void *arg, int pendindg)
{

	struct tx_qcq* txqcq = arg;
	int err;

	KASSERT((txqcq == NULL), ("task handler called with txqcq == NULL"));

	if (drbr_empty(txqcq->lif->netdev, txqcq->br))
		return;

	mtx_lock(&txqcq->mtx);
	/* 
	 * Process all Tx frames.
	 */
	err = ionic_start_xmit_locked(txqcq->lif->netdev, txqcq);
	mtx_unlock(&txqcq->mtx);
}

static int ionic_lif_txq_init(struct lif *lif, struct tx_qcq *txqcq)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.txq_init = {
			.opcode = CMD_OPCODE_TXQ_INIT,
			.I = false,
			.E = false,
			.pid = txqcq->pid,
			.intr_index = txqcq->intr.index,
			.type = TXQ_TYPE_ETHERNET,
			.index = txqcq->index,
			.cos = 0,
			.ring_base = txqcq->cmd_ring_pa,
			.ring_size = ilog2(txqcq->num_descs),
		},
	};
	int err, bind_cpu;

#if 0
	IONIC_NETDEV_INFO(lif->netdev, "txq_init.pid %d\n", ctx.cmd.txq_init.pid);
	IONIC_NETDEV_INFO(lif->netdev, "txq_init.index %d\n", ctx.cmd.txq_init.index);
	IONIC_NETDEV_INFO(lif->netdev, "txq_init.ring_base 0x%lx\n",
	           ctx.cmd.txq_init.ring_base);
	IONIC_NETDEV_INFO(lif->netdev, "txq_init.ring_size %d\n",
		   ctx.cmd.txq_init.ring_size);
#endif
	IONIC_NETDEV_QINFO(txqcq, "qid %d pid %d index %d ring_base 0x%lx ring_size %d\n",
		ctx.comp.txq_init.qid, ctx.cmd.txq_init.pid, ctx.cmd.txq_init.index, ctx.cmd.txq_init.ring_base, ctx.cmd.txq_init.ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	txqcq->qid = ctx.comp.txq_init.qid;
	txqcq->qtype = ctx.comp.txq_init.qtype;
	txqcq->db  = (void *)txqcq->lif->ionic->idev.db_pages + (txqcq->pid * PAGE_SIZE);
	txqcq->db += txqcq->qtype;

	snprintf(txqcq->intr.name, sizeof(txqcq->intr.name), "%s", txqcq->name);

	request_irq(txqcq->intr.vector, ionic_tx_isr, 0, txqcq->intr.name, txqcq);

	TASK_INIT(&txqcq->task, 0, ionic_tx_task_handler, txqcq);
    txqcq->taskq = taskqueue_create_fast(txqcq->name, M_NOWAIT,
	    taskqueue_thread_enqueue, &txqcq->taskq);

#ifdef RSS
	bind_cpu = rss_getcpu(txqcq->index % rss_getnumbuckets());
#else
	bind_cpu = txqcq->index;
#endif
	err = bind_irq_to_cpu(txqcq->intr.vector, bind_cpu);
	if (err) {
		IONIC_NETDEV_QWARN(txqcq, "failed to bind to cpu%d\n", bind_cpu);
		
	}
	IONIC_NETDEV_QINFO(txqcq, "bound to cpu%d\n", bind_cpu);

	IONIC_NETDEV_QINFO(txqcq, "qid: %d qtype: %d db: %pd\n",
		txqcq->qid, txqcq->qtype, txqcq->db);

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

/* XXX rx_limit/pending handling. */
static int ionic_rx_clean(struct rx_qcq* rxqcq , int rx_limit)
{
	struct rxq_comp *comp;
	struct rxq_desc *cmd;
	struct ionic_rx_buf *rxbuf;
	int comp_index, cmd_index, processed, cmd_stop_index;

	for ( processed = 0 ; processed < rx_limit ; ) {
		comp_index = rxqcq->comp_index;
		comp = &rxqcq->comp_ring[comp_index];
		/* Sync every time descriptors. */
		bus_dmamap_sync(rxqcq->cmd_dma.dma_tag, rxqcq->cmd_dma.dma_map,
			BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);

		cmd_stop_index = comp->comp_index;
		cmd_index = rxqcq->cmd_tail_index;
		rxbuf = &rxqcq->rxbuf[cmd_index];
		cmd = &rxqcq->cmd_ring[cmd_index];
			
		if (comp->color != rxqcq->done_color)
			break;
		
		IONIC_NETDEV_QINFO(rxqcq, "comp :%d cmd start: %d cmd stop: %d comp->color %d done_color %d\n",
			comp_index, cmd_index, cmd_stop_index, comp->color, rxqcq->done_color);
		IONIC_NETDEV_QINFO(rxqcq, "buf[%d] opcode:%d addr:0%lx len:0x%x\n",
			cmd_index, cmd->opcode, cmd->addr, cmd->len);

		for ( ; cmd_index == cmd_stop_index; cmd_index++, processed++ ) {
			rxbuf = &rxqcq->rxbuf[cmd_index];
			cmd = &rxqcq->cmd_ring[cmd_index];
			ionic_rx_input(rxqcq, rxbuf, comp, cmd);
		} 

		rxqcq->comp_index = (rxqcq->comp_index + 1) % rxqcq->num_descs;
		rxqcq->cmd_tail_index = (rxqcq->cmd_tail_index + 1) % rxqcq->num_descs;
		/* Roll over condition, flip color. */
		if (rxqcq->comp_index == 0) {
			rxqcq->done_color = !rxqcq->done_color;
		}
	}

	IONIC_NETDEV_QINFO(rxqcq, "ionic_rx_clean processed %d\n", processed);

	if (comp->color == rxqcq->done_color)
		taskqueue_enqueue(rxqcq->taskq, &rxqcq->task);

	/* XXX: Refill mbufs if we have processed at least 10 packets. */	
	ionic_rx_fill(rxqcq);

	/* XXX: flush at the end of ISR or taskqueue handler? */
	tcp_lro_flush_all(&rxqcq->lro);

	return (processed);

}


static void
ionic_rx_task_handler(void *arg, int pendindg)
{

	struct rx_qcq* rxqcq = arg;
	int processed;

	KASSERT((rxqcq == NULL), ("task handler called with qcq == NULL"));

	mtx_lock(&rxqcq->mtx);
	/* 
	 * Process all Rx frames.
	 */
	processed = ionic_rx_clean(rxqcq, -1);
	mtx_unlock(&rxqcq->mtx);
}

void ionic_rx_flush(struct rx_qcq *rxqcq)
{
	unsigned int work_done;

	IONIC_NETDEV_QINFO(rxqcq, "\n");

	work_done = ionic_rx_clean(rxqcq, -1);

	if (work_done > 0)
		ionic_intr_return_credits(&rxqcq->intr, work_done, 0, true);

	taskqueue_drain(rxqcq->taskq, &rxqcq->task);
}

static irqreturn_t ionic_rx_isr(int irq, void *data)
{
	struct rx_qcq* rxqcq = data;
	struct rx_stats* rxstats = &rxqcq->stats;
	int work_done;

	mtx_lock(&rxqcq->mtx);
	rxstats->isr_count++;

	ionic_intr_mask(&rxqcq->intr, true);
 
	work_done = ionic_rx_clean(rxqcq, 256/* XXX: tunable */);
	
	ionic_intr_return_credits(&rxqcq->intr, work_done, 0, true);

	// Enable interrupt.
	ionic_intr_mask(&rxqcq->intr, false);
	mtx_unlock(&rxqcq->mtx);

	return IRQ_HANDLED;
}




static int ionic_lif_rxq_init(struct lif *lif, struct rx_qcq *rxqcq)
{
	struct lro_ctrl *lro = &rxqcq->lro;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rxq_init = {
			.opcode = CMD_OPCODE_RXQ_INIT,
			.I = false,
			.E = false,
			.pid = rxqcq->pid,
			.intr_index = rxqcq->intr.index,
			.type = RXQ_TYPE_ETHERNET,
			.index = rxqcq->index,
			.ring_base = rxqcq->cmd_ring_pa,
			.ring_size = ilog2(rxqcq->num_descs),
		},
	};
	int err, bind_cpu;

	IONIC_NETDEV_QINFO(rxqcq, "pid %d index %d ring_base 0x%lx ring_size %d\n",
		 ctx.cmd.rxq_init.pid, ctx.cmd.rxq_init.index, ctx.cmd.rxq_init.ring_base, ctx.cmd.rxq_init.ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	rxqcq->qid = ctx.comp.rxq_init.qid;
	rxqcq->qtype = ctx.comp.rxq_init.qtype;

	/* XXX: move to be part of ring doorbell */
	rxqcq->db  = (void *)rxqcq->lif->ionic->idev.db_pages + (rxqcq->pid * PAGE_SIZE);
	rxqcq->db += rxqcq->qtype;
	IONIC_NETDEV_QINFO(rxqcq, "doorbell: %p\n", rxqcq->db);

	snprintf(rxqcq->intr.name, sizeof(rxqcq->intr.name), "%s", rxqcq->name);
	request_irq(rxqcq->intr.vector, ionic_rx_isr, 0, rxqcq->intr.name, rxqcq);

#ifdef RSS
	bind_cpu = rss_getcpu(rxqcq->index % rss_getnumbuckets());
#else
	bind_cpu = rxqcq->index;
#endif
	err = bind_irq_to_cpu(rxqcq->intr.vector, bind_cpu);
	if (err) {
		IONIC_NETDEV_QWARN(rxqcq, "failed to bindto cpu%d\n", bind_cpu);
	}
	IONIC_NETDEV_QINFO(rxqcq, "bound to cpu%d\n", bind_cpu);

	TASK_INIT(&rxqcq->task, 0, ionic_rx_task_handler, rxqcq);
    rxqcq->taskq = taskqueue_create_fast(rxqcq->intr.name, M_NOWAIT,
	    taskqueue_thread_enqueue, &rxqcq->taskq);
	/* RSS task queue binding. */
#ifdef RSS
#else
    err = taskqueue_start_threads(&rxqcq->taskq, 1, PI_NET,
        "%s (que %s)", device_get_nameunit(lif->ionic->dev), rxqcq->intr.name);
#endif
	if (err) {
		IONIC_NETDEV_QERR(rxqcq, "failed to create task queue, error: %d\n",
			err);
		taskqueue_free(rxqcq->taskq);
		return (err);
	}

	if (lif->netdev->if_capenable & IFCAP_LRO) {
		err = tcp_lro_init(lro);
		if (err) {
			IONIC_NETDEV_QERR(rxqcq, "LRO setup failed, error: %d\n", err);
		} else {
			lro->ifp = lif->netdev;
		}
	}

	IONIC_NETDEV_QINFO(rxqcq, "qid: %d qtype: %d db:%p\n", rxqcq->qid, rxqcq->qtype, rxqcq->db);

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

	lif->buf_len = MCLBYTES;
#ifdef notyet
	if (lif->max_frame_size <= MCLBYTES)
		len = MCLBYTES;
	else
		len = MJUMPAGESIZE;
#endif

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
static int ionic_set_hw_feature(struct lif *lif, uint16_t set_feature)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.features = {
			.opcode = CMD_OPCODE_FEATURES,
			.set = FEATURE_SET_ETH_HW_FEATURES,
			.wanted = set_feature,
#if 0			
			.wanted = ETH_HW_VLAN_TX_TAG
				| ETH_HW_VLAN_RX_STRIP
				| ETH_HW_VLAN_RX_FILTER
				| ETH_HW_RX_HASH
				| ETH_HW_TX_SG
				| ETH_HW_TX_CSUM     
				| ETH_HW_RX_CSUM,
#endif				
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	if (ctx.cmd.features.wanted != ctx.comp.features.supported)
		IONIC_NETDEV_ERROR(lif->netdev, "Feature wanted 0x%X != supported 0x%X\n", ctx.cmd.features.wanted, ctx.comp.features.supported);

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
				| ETH_HW_RX_CSUM);
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
//	unsigned int ntxqs = ident->dev.ntxqs_per_lif;
//	unsigned int nrxqs = ident->dev.nrxqs_per_lif;
	/* Tx and Rx Qs are in pair. */
	int nqs = min(ident->dev.ntxqs_per_lif, ident->dev.nrxqs_per_lif);
	unsigned int nintrs, dev_nintrs = ident->dev.nintrs;
	int err;

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
