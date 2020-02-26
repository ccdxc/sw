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
#include "ionic_bus.h"
#include "ionic_api.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"

#include "opt_rss.h"

#ifdef	RSS
#include <net/rss_config.h>
#include <netinet/in_rss.h>
#endif

void ionic_rx_fill(struct ionic_rxque *rxq);
static void ionic_rx_empty(struct ionic_rxque *rxq);
static void ionic_rx_refill(struct ionic_rxque *rxq);
static void ionic_tx_empty(struct ionic_txque *txq);

static int ionic_addr_add(struct ifnet *ifp, const uint8_t *addr);
static int ionic_addr_del(struct ifnet *ifp, const uint8_t *addr);

static void ionic_notifyq_task_handler(void *arg, int pendindg);
static irqreturn_t ionic_notifyq_isr(int irq, void *data);

static int ionic_lif_rss_alloc(struct ionic_lif *lif);
static void ionic_lif_rss_free(struct ionic_lif *lif);
static int ionic_lif_rss_init(struct ionic_lif *lif);
static int ionic_lif_rss_deinit(struct ionic_lif *lif);

static int ionic_ifmedia_xcvr(struct ionic_lif *lif);
static void ionic_media_status(struct ifnet *ifp, struct ifmediareq *ifmr);
static int ionic_media_change(struct ifnet *ifp);

static int ionic_set_features(struct ionic_lif *lif, uint32_t features);

struct ionic_lif_addr_work {
	struct work_struct work;
	struct ionic_lif *lif;
	uint16_t vid;
	uint8_t addr[ETH_ALEN];
	bool add;
};

struct rx_mode_work {
	struct work_struct work;
	struct ionic_lif *lif;
	unsigned int rx_mode;
};

struct ionic_lif_vlan_work {
	struct work_struct work;
	struct ionic_lif *lif;
	uint16_t vid;
	bool add;
};

/*
 * Enable/disable NIC queues.
 */
static int
ionic_q_enable_disable(struct ionic_lif *lif, unsigned int index,
    unsigned int qtype, bool enable)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_control = {
			.opcode = CMD_OPCODE_Q_CONTROL,
			.lif_index = lif->index,
			.type = qtype,
			.index = index,
			.oper = enable ? IONIC_Q_ENABLE : IONIC_Q_DISABLE,
		},
	};
	int err;

	IONIC_NETDEV_INFO(lif->netdev, "%s qid %d qtype:%d\n",
			  enable ? "Enable" : "Disable",
			  ctx.cmd.q_control.index, ctx.cmd.q_control.type);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		IONIC_NETDEV_ERROR(lif->netdev,
				   "Q enable failed for qid %d qtype:%d\n",
				   ctx.cmd.q_control.index,
				   ctx.cmd.q_control.type);
	return (err);
}

static void
ionic_rxq_enable(struct ionic_rxque *rxq)
{
	int err;

	IONIC_QUE_INFO(rxq, "Enabling\n");
	err = ionic_q_enable_disable(rxq->lif, rxq->index, rxq->type, true /* enable */);
	if (err)
		IONIC_QUE_WARN(rxq, "failed to enable, err: %d\n", err);
}

static void
ionic_rxq_disable(struct ionic_rxque *rxq)
{
	int err;

	IONIC_QUE_INFO(rxq, "Disabling\n");
	err = ionic_q_enable_disable(rxq->lif, rxq->index, rxq->type, false /* disable */);
	if (err)
		IONIC_QUE_WARN(rxq, "failed to disable, err: %d\n", err);
}

static void
ionic_txq_enable(struct ionic_txque *txq)
{
	int err;

	IONIC_QUE_INFO(txq, "Enabling\n");
	err = ionic_q_enable_disable(txq->lif, txq->index, txq->type, true /* enable */);
	if (err)
		IONIC_QUE_WARN(txq, "failed to enable, err: %d\n", err);
}

static void
ionic_txq_disable(struct ionic_txque *txq)
{
	int err;

	IONIC_QUE_INFO(txq, "Disabling\n");
	err = ionic_q_enable_disable(txq->lif, txq->index, txq->type, false /* disable */);
	if (err)
		IONIC_QUE_WARN(txq, "failed to disable, err: %d\n", err);
}

static void
ionic_adminq_hb_work(struct work_struct *work)
{
	struct ionic_lif *lif =
	    container_of(work, struct ionic_lif, adq_hb_work.work);
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.nop = {
			.opcode = CMD_OPCODE_NOP,
		},
	};
	int err;

	if (!lif->adq_hb_interval)
		return;

	/* Send a NOP command to monitor AdminQ */
	IONIC_LIF_LOCK(lif);
	err = ionic_adminq_post_wait(lif, &ctx);
	if (lif->wdog_error_trigger == IONIC_WDOG_TRIG_ADMINQ) {
		IONIC_QUE_WARN(lif->adminq, "injecting error\n");
		err = -1;
		lif->wdog_error_trigger = 0;
	}
	IONIC_LIF_UNLOCK(lif);
	if (err && err != -ESHUTDOWN) {
		IONIC_QUE_ERROR(lif->adminq, "heartbeat failed\n");
		err = ionic_lif_reinit(lif, true);
		if (err) {
			/* Disable the heartbeat */
			lif->adq_hb_interval = 0;
			return;
		}
	}

	IONIC_WDOG_LOCK(lif);
	if (lif->adq_hb_resched)
		queue_delayed_work(lif->wdog_wq, &lif->adq_hb_work,
		    lif->adq_hb_interval);
	IONIC_WDOG_UNLOCK(lif);
}

static void
ionic_adminq_hb_stop(struct ionic_lif *lif)
{

	IONIC_WDOG_LOCK(lif);
	lif->adq_hb_resched = false;
	IONIC_WDOG_UNLOCK(lif);
	cancel_delayed_work_sync(&lif->adq_hb_work);
}

void
ionic_adminq_hb_resched(struct ionic_lif *lif)
{

	/* Cancel all outstanding work */
	ionic_adminq_hb_stop(lif);

	/* Start again with the new hb_interval */
	IONIC_WDOG_LOCK(lif);
	lif->adq_hb_resched = true;
	queue_delayed_work(lif->wdog_wq, &lif->adq_hb_work,
	    lif->adq_hb_interval);
	IONIC_WDOG_UNLOCK(lif);
}

static void
ionic_txq_wdog_work(struct work_struct *work)
{
	struct ionic_lif *lif =
	    container_of(work, struct ionic_lif, txq_wdog_work.work);
	struct ionic_txque *txq;
	struct ionic_dev *idev = &lif->ionic->idev;
	struct ionic_rxque *rxq;
	uint64_t first_wdog_start = 0;
	uint32_t mask, rxq_intr;
	int i, err = 0, first_txq = 0;

	if (!lif->txq_wdog_timeout)
		return;

	IONIC_LIF_LOCK(lif);
	if (lif->link_up &&
	    (lif->netdev->if_drv_flags & IFF_DRV_RUNNING)) {
		/* Check each TxQ for timeouts */
		for (i = 0; i < lif->ntxqs; i++) {
			txq = lif->txqs[i];
			/* XXX: Unlocked to avoid causing data path jitter */
			if (txq->full &&
			    ticks > txq->wdog_start + lif->txq_wdog_timeout) {
				txq->stats.wdog_expired++;
				if (!err) {
					first_txq = i;
					first_wdog_start = txq->wdog_start;
				}
				err = ETIMEDOUT;
				txq->wdog_start = 0;
			}
		}
	}
	if (lif->wdog_error_trigger == IONIC_WDOG_TRIG_TXQ) {
		IONIC_QUE_WARN(lif->adminq, "injecting timeout\n");
		err = -1;
		lif->wdog_error_trigger = 0;
	}
	IONIC_LIF_UNLOCK(lif);

	if (err) {
		rxq = lif->rxqs[first_txq];
		mask = ioread32(&idev->intr_ctrl[rxq->intr.index].mask);
		rxq_intr = ioread32(&idev->intr_ctrl[rxq->intr.index].credits);

		IONIC_QUE_ERROR(lif->adminq,
		    "txq%d watchdog timeout (%lums), intr %#x mask %#x\n",
		    first_txq, (ticks - first_wdog_start) * 1000 / HZ,
		    rxq_intr, mask);

		err = ionic_lif_reinit(lif, true);
		if (err) {
			/* Disable the watchdog */
			lif->txq_wdog_timeout = 0;
			return;
		}
	}

	IONIC_WDOG_LOCK(lif);
	if (lif->txq_wdog_resched)
		queue_delayed_work(lif->wdog_wq, &lif->txq_wdog_work,
		    lif->txq_wdog_timeout);
	IONIC_WDOG_UNLOCK(lif);
}

static void
ionic_txq_wdog_stop(struct ionic_lif *lif)
{

	IONIC_WDOG_LOCK(lif);
	lif->txq_wdog_resched = false;
	IONIC_WDOG_UNLOCK(lif);
	cancel_delayed_work_sync(&lif->txq_wdog_work);
}

void
ionic_txq_wdog_resched(struct ionic_lif *lif)
{

	/* Cancel all outstanding work */
	ionic_txq_wdog_stop(lif);

	/* Start again with the new hb_interval */
	IONIC_WDOG_LOCK(lif);
	lif->txq_wdog_resched = true;
	queue_delayed_work(lif->wdog_wq, &lif->txq_wdog_work,
	    lif->txq_wdog_timeout);
	IONIC_WDOG_UNLOCK(lif);
}

/*
 * Calculate mbuf pool size based on MTU.
 */
static void
ionic_calc_rx_size(struct ionic_lif *lif)
{

	if (lif->max_frame_size > MJUM9BYTES)
		lif->rx_mbuf_size  = MJUM16BYTES;
	else if (lif->max_frame_size > MJUMPAGESIZE)
		lif->rx_mbuf_size  = MJUM9BYTES;
	else if (lif->max_frame_size > MCLBYTES)
		lif->rx_mbuf_size  = MJUMPAGESIZE;
	else
		lif->rx_mbuf_size  = MCLBYTES;
}

/*
 * Read the lif status to read the most recent link status.
 */
static void
ionic_get_link_status(struct ionic_lif *lif)
{
	struct lif_status *ls = &lif->info->status;

	if (ls == NULL)
		return;

	if (ls->eid < lif->last_eid)
		return;

	lif->link_speed = ls->link_speed;
	lif->link_up = (ls->link_status == PORT_OPER_STATUS_UP);
}

/*
 * Enable all the queues and unmask interrupts.
 */
static void
ionic_hw_open(struct ionic_lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct ionic_rxque *rxq;
	struct ionic_txque *txq;
	int i;

	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));
	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		IONIC_RX_LOCK(rxq);
		ionic_rx_fill(rxq);
		IONIC_RX_UNLOCK(rxq);
		ionic_rxq_enable(rxq);
		ionic_intr_mask(idev->intr_ctrl, rxq->intr.index,
		    IONIC_INTR_MASK_CLEAR);
	}

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		ionic_txq_enable(txq);
	}
}

static void
ionic_open(struct ionic_lif *lif)
{
	struct ifnet *ifp = lif->netdev;

	KASSERT(lif, ("lif is NULL"));
	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));

	/* already running? */
	if (ifp->if_drv_flags & IFF_DRV_RUNNING)
		return;

	IONIC_NETDEV_INFO(ifp, "starting interface\n");

	ionic_calc_rx_size(lif);

	lif->stop = false;
	ionic_hw_open(lif);
	ifp->if_drv_flags |= IFF_DRV_RUNNING;
	if_link_state_change(ifp, LINK_STATE_UP);
}

/*
 * Disable device, disable all queues, mask interrupts.
 */
int
ionic_stop(struct ifnet *ifp)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	struct ionic_dev *idev = &lif->ionic->idev;
	struct ionic_rxque *rxq;
	struct ionic_txque *txq;
	int i;

	KASSERT(lif, ("lif is NULL"));
	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));

	/* already stopped? */
	if (!(ifp->if_drv_flags & IFF_DRV_RUNNING))
		return (0);

	IONIC_NETDEV_INFO(ifp, "stopping interface\n");

	lif->stop = true;
	lif->netdev->if_drv_flags &= ~IFF_DRV_RUNNING;
	if_link_state_change(lif->netdev, LINK_STATE_DOWN);

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		ionic_rxq_disable(rxq);
		ionic_intr_mask(idev->intr_ctrl, rxq->intr.index,
		    IONIC_INTR_MASK_SET);
	}

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		ionic_txq_disable(txq);
	}

	ionic_lif_quiesce(lif);

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		IONIC_TX_LOCK(txq);
		ionic_tx_clean(txq, txq->num_descs);
		IONIC_TX_UNLOCK(txq);
	}

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		IONIC_RX_LOCK(rxq);
		ionic_rx_clean(rxq, rxq->num_descs);
		IONIC_RX_UNLOCK(rxq);
	}

	return (0);
}

void
ionic_open_or_stop(struct ionic_lif *lif)
{

	if (lif->netdev->if_flags & IFF_UP  && lif->link_up)
		ionic_open(lif);
	else
		ionic_stop(lif->netdev);
}

int
ionic_slave_alloc(struct ionic *ionic, enum ionic_api_prsn prsn)
{
	int index;

	/* slave index starts at 1, master_lif is 0 */
	index = find_first_zero_bit(ionic->lifbits, ionic->nlifs);
	if (index > ionic->nlifs)
		return (-ENOSPC);

	set_bit(index, ionic->lifbits);
	if (prsn == IONIC_PRSN_ETH)
		set_bit(index, ionic->ethbits);

	return (index);
}

void
ionic_slave_free(struct ionic *ionic, int index)
{
	if (index > ionic->nlifs)
		return;
	clear_bit(index, ionic->lifbits);
	clear_bit(index, ionic->ethbits);
}

/******************* AdminQ ******************************/
int
ionic_adminq_clean(struct ionic_adminq* adminq, int limit)
{
	struct admin_comp *comp;
	struct admin_cmd *cmd;
	struct ionic_admin_ctx *ctx;
	struct ionic_adminq_stats *stat = &adminq->stats;
	int comp_index, cmd_index, processed;

	KASSERT(IONIC_ADMIN_LOCK_OWNED(adminq), ("adminq is not locked"));

	/* Sync every time descriptors. */
	bus_dmamap_sync(adminq->cmd_dma.dma_tag, adminq->cmd_dma.dma_map,
	    BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);

	for (processed = 0; processed < limit; processed++) {
		comp_index = adminq->comp_index;
		comp = &adminq->comp_ring[comp_index];
		cmd_index = adminq->tail_index;
		cmd = &adminq->cmd_ring[cmd_index];
		ctx = adminq->ctx_ring[cmd_index];

		if (!color_match(comp->color, adminq->done_color))
			break;

		if (ctx) {
			memcpy(&ctx->comp, comp, sizeof(*comp));
			complete_all(&ctx->work);
			IONIC_QUE_INFO(adminq,
			    "completion done: %p\n", &ctx->work);
			adminq->ctx_ring[cmd_index] = NULL;
		}

		if (comp->status) {
			IONIC_QUE_ERROR(adminq,
			    "failed for opcode: %d status: %d\n",
			    cmd->opcode, comp->status);
			stat->comp_err++;
		}

		IONIC_NETDEV_INFO(adminq->lif->netdev, "admin comp:\n");
		if (__IONIC_DEBUG)
			print_hex_dump_debug("admin comp ", DUMP_PREFIX_OFFSET,
			    16, 1, comp, sizeof(struct admin_comp), true);

		adminq->comp_index = IONIC_MOD_INC(adminq, comp_index);
		adminq->tail_index = IONIC_MOD_INC(adminq, tail_index);
		/* Roll over condition, flip color. */
		if (adminq->comp_index == 0) {
			adminq->done_color = !adminq->done_color;
		}
	}

	IONIC_QUE_INFO(adminq, "head: %d tail: %d comp index: %d\n",
	    adminq->head_index, adminq->tail_index, adminq->comp_index);
	return (processed);
}

static irqreturn_t
ionic_adminq_isr(int irq, void *data)
{
	struct ionic_adminq* adminq = data;
	struct ionic_dev *idev = &adminq->lif->ionic->idev;
	int processed;

	KASSERT(adminq, ("adminq == NULL"));

	IONIC_ADMIN_LOCK(adminq);

	processed = ionic_adminq_clean(adminq, adminq->num_descs);

	IONIC_QUE_INFO(adminq, "processed: %d\n", processed);

	ionic_intr_credits(idev->intr_ctrl, adminq->intr.index,
	    processed, IONIC_INTR_CRED_REARM);
	IONIC_ADMIN_UNLOCK(adminq);

	if (!processed)
		IONIC_QUE_INFO(adminq, "nothing processed in adminq\n");

	return (IRQ_HANDLED);
}

static bool
ionic_adminq_wq_post(struct ionic_lif *lif, struct work_struct *work)
{
	struct ionic_adminq *adminq = lif->adminq;
	bool posted = true;

	IONIC_ADMIN_LOCK(adminq);
	if (adminq->wq_stop)
		posted = false;
	else
		queue_work(lif->adminq_wq, work);
	IONIC_ADMIN_UNLOCK(adminq);

	return (posted);
}

static void
ionic_adminq_wq_disable_drain(struct ionic_lif *lif)
{
	struct ionic_adminq *adminq = lif->adminq;

	IONIC_ADMIN_LOCK(adminq);
	adminq->wq_stop = true;
	IONIC_ADMIN_UNLOCK(adminq);

	drain_workqueue(lif->adminq_wq);
}

/*
 * MAC VLAN
 */

static bool
is_cdp_mac(const uint8_t *addr)
{

	return (!memcmp(addr, "\x01\x00\x0c\xcc\xcc\xcc", ETH_ALEN));
}

static bool
is_lldp_mac(const uint8_t *addr)
{

	return (!memcmp(addr, "\x01\x80\xc2\x00\x00\x00", ETH_ALEN) ||
		!memcmp(addr, "\x01\x80\xc2\x00\x00\x03", ETH_ALEN) ||
		!memcmp(addr, "\x01\x80\xc2\x00\x00\x0e", ETH_ALEN));
}

static int
_ionic_lif_macvlan_add(struct ionic_lif *lif, uint16_t vid, const uint8_t *addr)
{
	struct rx_filter *f;
	int err;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_add = {
			.opcode = CMD_OPCODE_RX_FILTER_ADD,
			.match = RX_FILTER_MATCH_MAC_VLAN,
			.mac_vlan.vlan = vid,
		},
	};

	IONIC_RX_FILTER_LOCK(&lif->rx_filters);
	f = ionic_rx_filter_by_vlan_addr(lif, vid, addr);
	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
	if (f) {
		IONIC_NETDEV_ADDR_DEBUG(lif->netdev, addr,
		    "VLAN: %d duplicate\n", vid);
		return (EINVAL);
	}

	memcpy(ctx.cmd.rx_filter_add.mac_vlan.addr, addr, ETH_ALEN);
	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		IONIC_NETDEV_ADDR_ERROR(lif->netdev, addr,
		    "VLAN: %d failed to add, err: %d", vid, err);
		return (err);
	} else {
		IONIC_NETDEV_ADDR_INFO(lif->netdev, addr,
		    "VLAN: %d added (filter id: %d)\n",
		    vid, ctx.comp.rx_filter_add.filter_id);
	}

	err = ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
	if (err)
		IONIC_NETDEV_ADDR_ERROR(lif->netdev, addr,
		    "VLAN: %d failed to save filter (filter id: %d)\n",
		    vid, ctx.comp.rx_filter_add.filter_id);

	return (err);
}

static int
_ionic_lif_macvlan_del(struct ionic_lif *lif, uint16_t vid, const uint8_t *addr)
{
	struct rx_filter *f;
	int err;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_del = {
			.opcode = CMD_OPCODE_RX_FILTER_DEL,
		},
	};

	IONIC_RX_FILTER_LOCK(&lif->rx_filters);
	f = ionic_rx_filter_by_addr(lif, addr);
	if (!f) {
		IONIC_NETDEV_ADDR_DEBUG(lif->netdev, addr,
		    "VLAN: %d not present", vid);
		IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
		return (ENOENT);
	}

	if (f->filter_id == 0) {
		IONIC_NETDEV_ADDR_INFO(lif->netdev, addr,
		    "VLAN: %d invalid filter id", vid);
		IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
		return (EINVAL);
	}

	ctx.cmd.rx_filter_del.filter_id = f->filter_id;
	ionic_rx_filter_free(lif, f);
	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		IONIC_NETDEV_ADDR_ERROR(lif->netdev, addr,
		    "VLAN: %d failed to delete, err: %d", vid, err);
	else
		IONIC_NETDEV_ADDR_INFO(lif->netdev, addr,
		    "VLAN: %d deleted (filter id: %d)", vid,
		    ctx.cmd.rx_filter_del.filter_id);

	return (err);
}

static void
ionic_lif_macvlan_work(struct work_struct *work)
{
	struct ionic_lif_addr_work *w =
	    container_of(work, struct ionic_lif_addr_work, work);

	IONIC_NETDEV_ADDR_INFO(w->lif->netdev, w->addr,
	    "VLAN: %d Work start: rx_filter: %s Work: %p",
	    w->vid, w->add ? "add" : "del", w);
	IONIC_LIF_LOCK(w->lif);
	if (w->add)
		_ionic_lif_macvlan_add(w->lif, w->vid, w->addr);
	else
		_ionic_lif_macvlan_del(w->lif, w->vid, w->addr);
	IONIC_LIF_UNLOCK(w->lif);

	free(w, M_IONIC);
}

static int
ionic_lif_macvlan(struct ionic_lif *lif, uint16_t vid, const uint8_t *addr,
    bool add)
{
	struct ionic_lif_addr_work *work;

	work = malloc(sizeof(*work), M_IONIC, M_NOWAIT | M_ZERO);
	if (!work) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to allocate memory for address work.\n");
		return (ENOMEM);
	}

	INIT_WORK(&work->work, ionic_lif_macvlan_work);
	work->lif = lif;
	work->vid = vid;
	memcpy(work->addr, addr, ETH_ALEN);
	work->add = add;

	if (ionic_adminq_wq_post(lif, &work->work))
		IONIC_NETDEV_ADDR_INFO(lif->netdev, addr,
		    "VLAN: %d deferred: rx_filter: %s ADDR Work: %p",
		    vid, add ? "add" : "del", work);
	else
		free(work, M_IONIC);

	return (0);
}

static int
ionic_macvlan_add(struct ifnet *ifp, uint16_t vid, const uint8_t *addr)
{
	struct ionic_lif *lif = if_getsoftc(ifp);

	return (ionic_lif_macvlan(lif, vid, addr, true));
}

static int
ionic_macvlan_del(struct ifnet *ifp, uint16_t vid, const uint8_t *addr)
{
	struct ionic_lif *lif = if_getsoftc(ifp);

	return (ionic_lif_macvlan(lif, vid, addr, false));
}

/*
 * MAC Filter
 */
static int
_ionic_lif_addr_add(struct ionic_lif *lif, const uint8_t *addr)
{
	struct rx_filter *f;
	int err;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_add = {
			.opcode = CMD_OPCODE_RX_FILTER_ADD,
			.match = RX_FILTER_MATCH_MAC,
		},
	};

	IONIC_RX_FILTER_LOCK(&lif->rx_filters);
	f = ionic_rx_filter_by_addr(lif, addr);
	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
	if (f) {
		IONIC_NETDEV_ADDR_DEBUG(lif->netdev, addr, "duplicate");
		return (EINVAL);
	}

	memcpy(ctx.cmd.rx_filter_add.mac.addr, addr, ETH_ALEN);
	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		IONIC_NETDEV_ADDR_ERROR(lif->netdev, addr,
		    "failed to add, err: %d", err);
		return (err);
	} else {
		IONIC_NETDEV_ADDR_INFO(lif->netdev, addr,
		    "added (filter id: %d)", ctx.comp.rx_filter_add.filter_id);
	}

	err = ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
	if (err)
		IONIC_NETDEV_ADDR_ERROR(lif->netdev, addr,
		    "failed to save filter (filter id: %d)",
		    ctx.comp.rx_filter_add.filter_id);

	return (err);
}

static int
_ionic_lif_addr_del(struct ionic_lif *lif, const uint8_t *addr)
{
	struct rx_filter *f;
	int err;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_del = {
			.opcode = CMD_OPCODE_RX_FILTER_DEL,
		},
	};

	IONIC_RX_FILTER_LOCK(&lif->rx_filters);
	f = ionic_rx_filter_by_addr(lif, addr);
	if (!f) {
		IONIC_NETDEV_ADDR_DEBUG(lif->netdev, addr, "not present");
		IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
		return (ENOENT);
	}

	if (f->filter_id == 0) {
		IONIC_NETDEV_ADDR_INFO(lif->netdev, addr, "invalid filter id");
		IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
		return (EINVAL);
	}

	ctx.cmd.rx_filter_del.filter_id = f->filter_id;
	ionic_rx_filter_free(lif, f);
	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		IONIC_NETDEV_ADDR_ERROR(lif->netdev, addr,
		    "failed to delete, err: %d", err);
	else
		IONIC_NETDEV_ADDR_INFO(lif->netdev, addr,
		    "deleted (filter id: %d)",
		    ctx.cmd.rx_filter_del.filter_id);

	return (err);
}

static void
ionic_lif_addr_work(struct work_struct *work)
{
	struct ionic_lif_addr_work *w =
	    container_of(work, struct ionic_lif_addr_work, work);

	IONIC_NETDEV_ADDR_INFO(w->lif->netdev, w->addr,
	    "Work start: rx_filter: %s ADDR Work: %p",
	    w->add ? "add" : "del", w);
	IONIC_LIF_LOCK(w->lif);
	if (w->add)
		_ionic_lif_addr_add(w->lif, w->addr);
	else
		_ionic_lif_addr_del(w->lif, w->addr);
	IONIC_LIF_UNLOCK(w->lif);

	free(w, M_IONIC);
}

/*
 * Schedule updating MAC filter list.
 */
static int
ionic_lif_addr(struct ionic_lif *lif, const uint8_t *addr, bool add)
{
	struct ionic_lif_addr_work *work;

	work = malloc(sizeof(*work), M_IONIC, M_NOWAIT | M_ZERO);
	if (!work) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to allocate memory for address work.\n");
		return (ENOMEM);
	}

	INIT_WORK(&work->work, ionic_lif_addr_work);
	work->lif = lif;
	memcpy(work->addr, addr, ETH_ALEN);
	work->add = add;

	if (ionic_adminq_wq_post(lif, &work->work))
		IONIC_NETDEV_ADDR_INFO(lif->netdev, addr,
		    "deferred: rx_filter: %s ADDR Work: %p",
		    add ? "add" : "del", work);
	else
		free(work, M_IONIC);

	return (0);
}

static int
ionic_addr_add(struct ifnet *ifp, const uint8_t *addr)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	int ret;

	ret = ionic_lif_addr(lif, addr, true);

	if (ionic_cdp_vlan && is_cdp_mac(addr))
		(void)ionic_macvlan_add(ifp, ionic_cdp_vlan, addr);
	if (ionic_lldp_vlan && is_lldp_mac(addr))
		(void)ionic_macvlan_add(ifp, ionic_lldp_vlan, addr);

	return (ret);
}

static int
ionic_addr_del(struct ifnet *ifp, const uint8_t *addr)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	int ret;

	ret = ionic_lif_addr(lif, addr, false);

	if (ionic_cdp_vlan && is_cdp_mac(addr))
		(void)ionic_macvlan_del(ifp, ionic_cdp_vlan, addr);
	if (ionic_lldp_vlan && is_lldp_mac(addr))
		(void)ionic_macvlan_del(ifp, ionic_lldp_vlan, addr);

	return (ret);
}

/*
 * Configure Rx mode of device.
 */
static void
_ionic_lif_rx_mode(struct ionic_lif *lif, unsigned int rx_mode)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_mode_set = {
			.opcode = CMD_OPCODE_RX_MODE_SET,
			.rx_mode = rx_mode,
		},
	};
        char buf[128];
        int i, err;
#define REMAIN(__x) (sizeof(buf) - (__x))

        i = snprintf(buf, sizeof(buf), "Rx mode:");
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
        if (rx_mode & RX_MODE_F_RDMA_SNIFFER)
                i += snprintf(&buf[i], REMAIN(i), " RX_MODE_F_RDMA_SNIFFER");
        if (rx_mode == 0)
                i += snprintf(&buf[i], REMAIN(i), " cleared");
        IONIC_NETDEV_PRINT(lif->netdev, "%s\n", buf);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "fail to configure rx mode, error: %d\n", err);
	}
}

static void
ionic_lif_rx_mode_work(struct work_struct *work)
{
	struct rx_mode_work *w = container_of(work, struct rx_mode_work, work);

	IONIC_LIF_LOCK(w->lif);
	if (w->rx_mode != w->lif->rx_mode) {
		w->lif->rx_mode = w->rx_mode;
		_ionic_lif_rx_mode(w->lif, w->rx_mode);
	}
	IONIC_LIF_UNLOCK(w->lif);

	free(w, M_IONIC);
}

/*
 * Schedule Rx mode work.
 */
void
ionic_lif_rx_mode(struct ionic_lif *lif, unsigned int rx_mode)
{
	struct rx_mode_work *work;

	IONIC_NETDEV_INFO(lif->netdev, "rx_mode: %#x\n", rx_mode);

	work = malloc(sizeof(*work), M_IONIC, M_NOWAIT | M_ZERO);
	if (!work) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to allocate for rx_mode\n");
		return;
	}
	INIT_WORK(&work->work, ionic_lif_rx_mode_work);
	work->lif = lif;
	work->rx_mode = rx_mode;

	if (ionic_adminq_wq_post(lif, &work->work))
		IONIC_NETDEV_INFO(lif->netdev, "deferred: rx_mode\n");
	else
		free(work, M_IONIC);
}

void
ionic_set_rx_mode(struct ifnet *ifp)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	unsigned int rx_mode;

	rx_mode = RX_MODE_F_UNICAST;
	rx_mode |= (ifp->if_flags & IFF_MULTICAST) ? RX_MODE_F_MULTICAST : 0;
	rx_mode |= (ifp->if_flags & IFF_BROADCAST) ? RX_MODE_F_BROADCAST : 0;
	rx_mode |= (ifp->if_flags & IFF_PROMISC) ? RX_MODE_F_PROMISC : 0;
	rx_mode |= (ifp->if_flags & IFF_ALLMULTI) ? RX_MODE_F_ALLMULTI : 0;
	rx_mode |= (lif->rx_mode & RX_MODE_F_RDMA_SNIFFER) ? RX_MODE_F_RDMA_SNIFFER : 0;

	IONIC_NETDEV_INFO(ifp, "Setting rx mode: %d\n", rx_mode);
	ionic_lif_rx_mode(lif, rx_mode);
}

/*
 * Program the multicast addresses.
 */
int
ionic_set_multi(struct ionic_lif *lif)
{
	struct ifmultiaddr *ifma;
	struct ifnet *ifp = lif->netdev;
	struct ionic_mc_addr *new_mc_addrs, *tmp;
	int i, j, mcnt = 0, max_maddrs;
	int err = 0, num_new_mc_addrs;
	uint8_t mac_addr[ETHER_ADDR_LEN];
	bool found;

	/* TODO: This is unsafe without LIF lock (mc_addrs) */
	//KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));

	max_maddrs = lif->ionic->ident.lif.eth.max_mcast_filters;

	if (lif->mc_addrs == NULL)
		return (EIO);

	if_maddr_rlock(ifp);
#if __FreeBSD_version >= 1200000
	CK_STAILQ_FOREACH(ifma, &ifp->if_multiaddrs, ifma_link) {
#else
	TAILQ_FOREACH(ifma, &ifp->if_multiaddrs, ifma_link) {
#endif
		if (ifma->ifma_addr->sa_family != AF_LINK)
			continue;
		bcopy(LLADDR((struct sockaddr_dl *) ifma->ifma_addr),
		    mac_addr, ETHER_ADDR_LEN);
		IONIC_NETDEV_ADDR_INFO(ifp, mac_addr, "new list[%d]", mcnt);
		mcnt++;
	}

	new_mc_addrs = malloc(sizeof(struct ionic_mc_addr) * mcnt, M_IONIC,
	    M_NOWAIT | M_ZERO);
	num_new_mc_addrs = 0;

	/* Find the new address we need to add. */
#if __FreeBSD_version >= 1200000
	CK_STAILQ_FOREACH(ifma, &ifp->if_multiaddrs, ifma_link) {
#else
	TAILQ_FOREACH(ifma, &ifp->if_multiaddrs, ifma_link) {
#endif
		if (ifma->ifma_addr->sa_family != AF_LINK)
			continue;
		bcopy(LLADDR((struct sockaddr_dl *) ifma->ifma_addr),
		    mac_addr, ETHER_ADDR_LEN);

		found = false;
		for (i = 0; i < lif->num_mc_addrs; i++) {
			if (bcmp(mac_addr, lif->mc_addrs[i].addr,
			    ETHER_ADDR_LEN) == 0) {
				found = true;
				lif->mc_addrs[i].present = true;
				break;
			}
		}

		if (!found)
			bcopy(mac_addr, new_mc_addrs[num_new_mc_addrs++].addr,
			    ETHER_ADDR_LEN);
	}

	/* Set all multi if we are out of filters. */
	if (mcnt >= max_maddrs) {
		if ((ifp->if_flags & IFF_ALLMULTI) == 0) {
			ifp->if_flags |= IFF_ALLMULTI;
			ionic_set_rx_mode(ifp);
		}
	} else {
		if (ifp->if_flags & IFF_ALLMULTI) {
			ifp->if_flags ^= IFF_ALLMULTI;
			ionic_set_rx_mode(ifp);
		}
	}

	/* Delete the addresses which are not in new list. */
	j = 0;
	for (i = 0; i < lif->num_mc_addrs; i++) {
		tmp = &lif->mc_addrs[i];
		if (!tmp->present) {
			tmp->present = false;
			err = ionic_addr_del(ifp, tmp->addr);
			if (err)
				goto err_out;
		} else {
			bcopy(lif->mc_addrs[i].addr, lif->mc_addrs[j++].addr,
			    ETHER_ADDR_LEN);
		}
	}
	lif->num_mc_addrs = j;

	/* Now add the addresses which are not present. */
	for (i = 0; i < num_new_mc_addrs && lif->num_mc_addrs < max_maddrs; i++) {
		IONIC_NETDEV_ADDR_INFO(ifp, new_mc_addrs[i].addr, "adding");
		bcopy(new_mc_addrs[i].addr,
		    lif->mc_addrs[lif->num_mc_addrs++].addr, ETHER_ADDR_LEN);
		err = ionic_addr_add(ifp, new_mc_addrs[i].addr);
		if (err)
			goto err_out;
	}

err_out:
	if_maddr_runlock(ifp);
	free(new_mc_addrs, M_IONIC);
	return (err);
}

/*
 * Handle MTU change ioctl call.
 */
int
ionic_change_mtu(struct ifnet *ifp, int new_mtu)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	struct ionic_rxque *rxq;
	uint32_t old_mbuf_size;
	int i, err;
	bool mbuf_size_changed;

	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_setattr = {
			.opcode = CMD_OPCODE_LIF_SETATTR,
			.index = lif->index,
			.attr = IONIC_LIF_ATTR_MTU,
			.mtu = new_mtu,
		},
	};

	if (if_getmtu(ifp) == new_mtu) {
		IONIC_NETDEV_INFO(ifp, "unchanged MTU: %d\n", new_mtu);
		return (0);
	}

	if (new_mtu < IONIC_MIN_MTU || new_mtu > IONIC_MAX_MTU) {
		IONIC_NETDEV_ERROR(ifp, "invalid MTU: %d\n", new_mtu);
		return (EINVAL);
	}

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return (err);

	/* Check rx mbuf size needs to be changed */
	lif->max_frame_size =
	    new_mtu + ETHER_HDR_LEN + ETHER_VLAN_ENCAP_LEN + ETHER_CRC_LEN;
	old_mbuf_size = lif->rx_mbuf_size;
	ionic_calc_rx_size(lif);
	mbuf_size_changed = (old_mbuf_size != lif->rx_mbuf_size);

	/*
	 * If mbuf size has not changed then there is no need to
	 * reprogram queues.
	 */
	if (mbuf_size_changed) {
		ionic_stop(ifp);

		for (i = 0; i < lif->nrxqs; i++) {
			rxq = lif->rxqs[i];
			IONIC_RX_LOCK(rxq);
			ionic_rx_refill(rxq);
			IONIC_RX_UNLOCK(rxq);
		}

		ionic_open_or_stop(lif);
	}

	if_setmtu(ifp, new_mtu);

	return (0);
}

/*
 * Deferred VLAN handling process.
 */
static int
_ionic_vlan_add(struct ifnet *ifp, uint16_t vid)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	struct rx_filter *f;
	int err;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_add = {
			.opcode = CMD_OPCODE_RX_FILTER_ADD,
			.match = RX_FILTER_MATCH_VLAN,
			.vlan.vlan = vid,
		},
	};

	IONIC_RX_FILTER_LOCK(&lif->rx_filters);
	f = ionic_rx_filter_by_vlan(lif, vid);
	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
	if (f) {
		IONIC_NETDEV_DEBUG(lif->netdev, "VLAN: %d duplicate\n", vid);
		return (EINVAL);
	}

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		IONIC_NETDEV_ERROR(ifp,
		    "failed to add VLAN: %d (filter id: %d), error: %d\n",
		    vid, ctx.comp.rx_filter_add.filter_id, err);
		return (err);
	}

	err = ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
	if (err)
		IONIC_NETDEV_ERROR(ifp,
		    "failed to save for VLAN: %d (filter id: %d), error: %d\n",
		    vid, ctx.comp.rx_filter_add.filter_id, err);

	return (err);
}

static int
_ionic_vlan_del(struct ifnet *ifp, uint16_t vid)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	struct rx_filter *f;
	int err;
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_del = {
			.opcode = CMD_OPCODE_RX_FILTER_DEL,
		},
	};

	IONIC_RX_FILTER_LOCK(&lif->rx_filters);
	f = ionic_rx_filter_by_vlan(lif, vid);
	if (!f) {
		IONIC_NETDEV_ERROR(ifp, "No VLAN: %d filter found\n", vid);
		IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);
		return (ENOENT);
	}

	KASSERT(f->filter_id, ("vlan del, filter id(%d) == 0", f->filter_id));
	ctx.cmd.rx_filter_del.filter_id = f->filter_id;
	ionic_rx_filter_free(lif, f);
	IONIC_RX_FILTER_UNLOCK(&lif->rx_filters);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		IONIC_NETDEV_ERROR(ifp,
		    "failed to del VLAN: %d (id: %d), error: %d\n",
		    vid, ctx.cmd.rx_filter_del.filter_id, err);

	return (err);
}

static void
ionic_lif_vlan_work(struct work_struct *work)
{
	struct ionic_lif_vlan_work *w =
	    container_of(work, struct ionic_lif_vlan_work, work);
	struct ifnet *ifp = w->lif->netdev;

	IONIC_LIF_LOCK(w->lif);
	if (w->add)
		_ionic_vlan_add(ifp, w->vid);
	else
		_ionic_vlan_del(ifp, w->vid);

	IONIC_LIF_UNLOCK(w->lif);

	free(w, M_IONIC);
}

static int
ionic_lif_vlan(struct ionic_lif *lif, const uint16_t vid, bool add)
{
	struct ionic_lif_vlan_work *work;

	work = malloc(sizeof(*work), M_IONIC, M_NOWAIT | M_ZERO);
	if (!work) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to allocate memory for address work.\n");
		return (ENOMEM);
	}

	INIT_WORK(&work->work, ionic_lif_vlan_work);
	work->lif = lif;
	work->vid = vid;
	work->add = add;

	if (ionic_adminq_wq_post(lif, &work->work))
		IONIC_NETDEV_INFO(lif->netdev,
		    "deferred: %s VLAN: %d", add ? "add" : "del", vid);
	else
		free(work, M_IONIC);

	return (0);
}

static void
ionic_reinit_vlan(struct ionic_lif *lif)
{
	int i, index, bit;

	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));

	for (i = 0; i < MAX_VLAN_TAG; i++) {
		index = i / 8;
		bit = i % 8;
		if (lif->vlan_bitmap[index] & BIT(bit)) {
			if (ionic_lif_vlan(lif, i, true))
				IONIC_NETDEV_ERROR(lif->netdev,
				    "VLAN: %d re-registration failed\n", i);
		}
	}
}

static void
ionic_register_vlan(void *arg, struct ifnet *ifp, uint16_t vtag)
{
	struct ionic_lif *lif = ifp->if_softc;
	int index = vtag / 8;
	int bit = vtag % 8;

	if (ifp->if_softc != arg)   /* Not our event */
		return;

	if ((vtag == 0) || (vtag > MAX_VLAN_TAG))  /* Invalid */
		return;

	IONIC_LIF_LOCK(lif);

	if (lif->vlan_bitmap[index] & BIT(bit)) {
		IONIC_NETDEV_WARN(ifp, "VLAN: %d is already registered\n", vtag);
		IONIC_LIF_UNLOCK(lif);
		return;
	}

	if (ionic_lif_vlan(lif, vtag, true)) {
		IONIC_NETDEV_ERROR(ifp, "VLAN: %d register failed\n", vtag);
		IONIC_LIF_UNLOCK(lif);
		return;
	}

	lif->vlan_bitmap[index] |= BIT(bit);
	++lif->num_vlans;

	IONIC_LIF_UNLOCK(lif);
	IONIC_NETDEV_INFO(ifp, "VLAN: %d registered\n", vtag);
}

static void
ionic_unregister_vlan(void *arg, struct ifnet *ifp, uint16_t vtag)
{
	struct ionic_lif *lif = ifp->if_softc;
	int index = vtag / 8;
	int bit = vtag % 8;

	if (ifp->if_softc != arg)
		return;

	if ((vtag == 0) || (vtag > MAX_VLAN_TAG))
		return;

	IONIC_LIF_LOCK(lif);

	if ((lif->vlan_bitmap[index] & BIT(bit)) == 0) {
		IONIC_NETDEV_WARN(ifp, "VLAN: %d is not registered\n", vtag);
		IONIC_LIF_UNLOCK(lif);
		return;
	}

	if (ionic_lif_vlan(lif, vtag, false)) {
		IONIC_NETDEV_ERROR(ifp, "VLAN: %d unregistration failed\n",
		    vtag);
		IONIC_LIF_UNLOCK(lif);
		return;
	}

	lif->vlan_bitmap[index] &= ~BIT(bit);
	--lif->num_vlans;

	IONIC_LIF_UNLOCK(lif);
	IONIC_NETDEV_INFO(ifp, "VLAN: %d unregistered\n", vtag);
}

/*
 * Allocate device level interrupt resources.
 */
int
ionic_dev_intr_reserve(struct ionic_lif *lif, struct intr *intr)
{
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;
	unsigned long index;

	KASSERT((intr->index == INTR_INDEX_NOT_ASSIGNED),
	    ("%s already has intr resource(%d)", intr->name, intr->index));

	index = find_first_zero_bit(ionic->intrs, ionic->nintrs);
	if (index == ionic->nintrs)
		return (ENOSPC);

	set_bit(index, ionic->intrs);

	return (ionic_intr_init(idev, intr, index));
}

void
ionic_dev_intr_unreserve(struct ionic_lif *lif, struct intr *intr)
{

	if (intr->index != INTR_INDEX_NOT_ASSIGNED)
		clear_bit(intr->index, lif->ionic->intrs);
}

/*
 * Handle OS level interrupt setup.
 */
static int
ionic_setup_intr(struct ionic_lif *lif, struct intr* intr)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	int error, irq;

	/* Setup interrupt */
	error = ionic_dev_intr_reserve(lif, intr);
	if (error) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "no available interrupt, error: %d\n", error);
		return (error);
	}

	/*
	 * Legacy IRQ allocation is done later on.
	 * Since its level trigger, don't mask on assert.
	 */
	if (ionic_enable_msix == 0)
		return (0);

	irq = ionic_get_msix_irq(lif->ionic, intr->index);
	if (irq < 0) {
		ionic_dev_intr_unreserve(lif, intr);
		IONIC_NETDEV_ERROR(lif->netdev,
		    "unreserve IRQ failed, error: %d\n", error);
		return (ENOSPC);
	}

	intr->vector = irq;
	ionic_intr_mask_assert(idev->intr_ctrl, intr->index,
	    IONIC_INTR_MASK_SET);

	return (0);
}

/*
 * Allocate adminQ and setup interrupt for it.
 */
static int
ionic_adminq_alloc(struct ionic_lif *lif, unsigned int qnum,
    unsigned int num_descs, unsigned int pid, struct ionic_adminq **padminq)
{
	struct ionic_adminq *adminq;
	int error = ENOMEM;
	uint32_t cmd_ring_size, comp_ring_size, total_size;

	*padminq = NULL;

	adminq = malloc(sizeof(*adminq), M_IONIC, M_NOWAIT | M_ZERO);
	if(adminq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to allocate adminq%d\n", qnum);
		return (error);
	}

	snprintf(adminq->name, sizeof(adminq->name) - 1, "aq%d", qnum);
	snprintf(adminq->intr.name, sizeof(adminq->intr.name), "%s",
	    adminq->name);
	snprintf(adminq->mtx_name, sizeof(adminq->mtx_name) - 1,
	    "aq%d_mtx", qnum);
	adminq->lif = lif;
	adminq->type = IONIC_QTYPE_ADMINQ;
	adminq->index = qnum;
	adminq->num_descs = num_descs;
	adminq->pid = lif->kern_pid;
	adminq->intr.index = INTR_INDEX_NOT_ASSIGNED;

	IONIC_ADMIN_LOCK_INIT(adminq);

	adminq->ctx_ring = malloc(sizeof(struct ionic_admin_ctx *) * num_descs,
	    M_IONIC, M_NOWAIT | M_ZERO);
	/*
	 * Allocate DMA for command and completion rings.
	 */
	cmd_ring_size = sizeof(*adminq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*adminq->comp_ring) * num_descs;
	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(comp_ring_size,
	    PAGE_SIZE);

	adminq->total_ring_size = total_size;

	error = ionic_dma_alloc(adminq->lif->ionic, total_size,
	    &adminq->cmd_dma, 0);
	if (error) {
		IONIC_QUE_ERROR(adminq,
		    "failed to allocate adminq ring, err: %d\n", error);
		goto error_out;
	}

	adminq->cmd_ring_pa = adminq->cmd_dma.dma_paddr;
	adminq->cmd_ring = (struct admin_cmd *)adminq->cmd_dma.dma_vaddr;
	IONIC_QUE_INFO(adminq,
	    "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
	    adminq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * Command and completion rings are side by side.
	 */
	adminq->comp_ring_pa = adminq->cmd_ring_pa +
	    ALIGN(cmd_ring_size, PAGE_SIZE);
	adminq->comp_ring = (struct admin_comp *)(adminq->cmd_dma.dma_vaddr +
	    ALIGN(cmd_ring_size, PAGE_SIZE));

	IONIC_QUE_INFO(adminq,
	    "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
	    adminq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);

	error = ionic_setup_intr(lif, &adminq->intr);
	if (error) {
		IONIC_QUE_ERROR(adminq,
		    "no available interrupt, error: %d\n", error);
		goto error_out;
	}
	if (ionic_enable_msix) {
		error = request_irq(adminq->intr.vector, ionic_adminq_isr, 0,
		    adminq->intr.name, adminq);
		if (error) {
			IONIC_QUE_ERROR(adminq,
			    "no available interrupt, error: %d\n", error);
			goto error_out;
		}
	}

	*padminq = adminq;
	return (0);

error_out:
	ionic_dev_intr_unreserve(lif, &adminq->intr);

	if (adminq->cmd_ring) {
		ionic_dma_free(adminq->lif->ionic, &adminq->cmd_dma);
		adminq->cmd_ring = NULL;
		adminq->comp_ring = NULL;
	}

	free(adminq->ctx_ring, M_IONIC);
	free(adminq, M_IONIC);
	return (error);
}

/*
 * Allocate Notify Queue and setup interrupt.
 */
static int
ionic_notifyq_alloc(struct ionic_lif *lif, unsigned int qnum,
    unsigned int num_descs, unsigned int pid, struct ionic_notifyq **pnotifyq)
{
	struct ionic_notifyq *notifyq;
	uint32_t cmd_ring_size, comp_ring_size, total_size;
	int error;
	char namebuf[16];

	*pnotifyq = NULL;

	notifyq = malloc(sizeof(*notifyq), M_IONIC, M_NOWAIT | M_ZERO);
	if(notifyq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to allocate notifyq%d\n", qnum);
		return (ENOMEM);
	}

	snprintf(notifyq->name, sizeof(notifyq->name) - 1, "nq%d", qnum);
	snprintf(notifyq->intr.name, sizeof(notifyq->intr.name), "%s",
	    notifyq->name);
	snprintf(notifyq->mtx_name, sizeof(notifyq->mtx_name) - 1, "nq%d_mtx",
	    qnum);
	notifyq->lif = lif;
	notifyq->type = IONIC_QTYPE_NOTIFYQ;
	notifyq->index = qnum;
	notifyq->num_descs = num_descs;
	notifyq->pid = pid;
	notifyq->intr.index = INTR_INDEX_NOT_ASSIGNED;

	/*
	 * Allocate DMA for command and completion rings.
	 */
	cmd_ring_size = sizeof(*notifyq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*notifyq->comp_ring) * num_descs;
	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) +
	    ALIGN(cmd_ring_size, PAGE_SIZE);
	notifyq->total_ring_size = total_size;

	error = ionic_dma_alloc(notifyq->lif->ionic, total_size,
	    &notifyq->cmd_dma, 0);
	if (error) {
		IONIC_QUE_ERROR(notifyq,
		    "failed to allocate DMA cmd ring, err: %d\n", error);
		goto error_out;
	}

	notifyq->cmd_ring_pa = notifyq->cmd_dma.dma_paddr;
	notifyq->cmd_ring = (struct notifyq_cmd *)notifyq->cmd_dma.dma_vaddr;
	IONIC_QUE_INFO(notifyq,
	    "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
	    notifyq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * Command and completion rings are side by side.
	 */
	notifyq->comp_ring = (union notifyq_comp *)(notifyq->cmd_dma.dma_vaddr +
	    ALIGN(cmd_ring_size, PAGE_SIZE));

	/*
	 * Create notifyQ task to process notifyQ events - link status, log
	 */
	TASK_INIT(&notifyq->task, 0, ionic_notifyq_task_handler, notifyq);
	snprintf(namebuf, sizeof(namebuf), "task-%s", notifyq->name);
	notifyq->taskq = taskqueue_create(namebuf, M_NOWAIT,
	    taskqueue_thread_enqueue, &notifyq->taskq);

	taskqueue_start_threads(&notifyq->taskq, 1, PI_NET, "%s (que %s)",
	    device_get_nameunit(lif->ionic->dev), notifyq->name);

	error = ionic_setup_intr(lif, &notifyq->intr);
	if (error) {
		IONIC_QUE_ERROR(notifyq,
		    "no available interrupt, error: %d\n", error);
		goto error_out;
	}
	/* Legacy interrupt allocation is done once. */
	if (ionic_enable_msix) {
		error = request_irq(notifyq->intr.vector, ionic_notifyq_isr, 0,
		    notifyq->intr.name, notifyq);
		if (error) {
			IONIC_QUE_ERROR(notifyq,
			    "request_irq failed, error: %d\n", error);
			goto error_out;
		}
	}

	*pnotifyq = notifyq;
	return (0);

error_out:
	ionic_dev_intr_unreserve(lif, &notifyq->intr);

	if (notifyq->cmd_ring) {
		ionic_dma_free(notifyq->lif->ionic, &notifyq->cmd_dma);
		notifyq->cmd_ring = NULL;
		notifyq->comp_ring = NULL;
	}

	free(notifyq, M_IONIC);
	return (error);
}

/*
 * Allocate receive queue and setup interrupt for Tx and Rx queue pair.
 */
static int
ionic_rxque_alloc(struct ionic_lif *lif, unsigned int qnum,
    unsigned int num_descs, unsigned int pid, struct ionic_rxque **prxq)
{
	struct ionic_rxque *rxq;
	struct ionic_rx_buf *rxbuf;
	struct lro_ctrl *lro;
	int i, error = ENOMEM;
	uint32_t cmd_ring_size, comp_ring_size, sg_ring_size, total_size;

	*prxq = NULL;

	rxq = malloc(sizeof(*rxq), M_IONIC, M_NOWAIT | M_ZERO);
	if(rxq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev, "failed to allocate rxq%d\n",
		    qnum);
		return (error);
	}

	snprintf(rxq->name, sizeof(rxq->name) - 1, "rxq%d", qnum);
	/* Interrupt is shared between tx and rx. */
	snprintf(rxq->intr.name, sizeof(rxq->intr.name), "rxtx%d", qnum);
	snprintf(rxq->mtx_name, sizeof(rxq->mtx_name) - 1, "rxq%d_mtx", qnum);
	rxq->lif = lif;
	rxq->type = IONIC_QTYPE_RXQ;
	rxq->index = qnum;
	rxq->num_descs = num_descs;
	rxq->pid = pid;
	rxq->intr.index = INTR_INDEX_NOT_ASSIGNED;

	IONIC_RX_LOCK_INIT(rxq);

	/* Setup command ring. */
	rxq->rxbuf = malloc(sizeof(*rxq->rxbuf) * num_descs, M_IONIC,
	    M_NOWAIT | M_ZERO);
	if (rxq->rxbuf == NULL) {
		IONIC_QUE_ERROR(rxq,
		    "Couldn't allocate rx buffer descriptors\n");
		goto error_out;
	}

	/* Allocate DMA for command and completion rings. */
	cmd_ring_size = sizeof(*rxq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*rxq->comp_ring) * num_descs;
	sg_ring_size = sizeof(*rxq->sg_ring) * num_descs;
	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) +
	    ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(sg_ring_size, PAGE_SIZE);
	rxq->total_ring_size = total_size;

	error = ionic_dma_alloc(rxq->lif->ionic, total_size,
	    &rxq->cmd_dma, 0);
	if (error) {
		IONIC_QUE_ERROR(rxq,
		    "failed to allocate rxq ring, err: %d\n", error);
		goto error_out;
	}

	rxq->cmd_ring_pa = rxq->cmd_dma.dma_paddr;
	rxq->cmd_ring = (struct rxq_desc *)rxq->cmd_dma.dma_vaddr;
	IONIC_QUE_INFO(rxq,
	    "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
	     rxq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * We assume that completion ring is next to command ring.
	 */
	rxq->comp_ring_pa = rxq->cmd_ring_pa + ALIGN(cmd_ring_size, PAGE_SIZE);
	rxq->comp_ring = (struct rxq_comp *)(rxq->cmd_dma.dma_vaddr +
	    ALIGN(cmd_ring_size, PAGE_SIZE));
	rxq->sg_ring_pa = rxq->comp_ring_pa + ALIGN(comp_ring_size, PAGE_SIZE);
	rxq->sg_ring = (struct rxq_sg_desc *)(rxq->cmd_dma.dma_vaddr +
	    ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(comp_ring_size, PAGE_SIZE));

	/* Setup interrupt */
	error = ionic_setup_intr(lif, &rxq->intr);
	if (error) {
		goto error_out;
	}

	/*
	 * Create just one tag for Rx buffers.
	 */
	error = bus_dma_tag_create(
	         /*      parent */ bus_get_dma_tag(rxq->lif->ionic->dev->bsddev),
	         /*   alignment */ 1,
	         /*      bounds */ 0,
	         /*     lowaddr */ IONIC_MAX_ADDR,
	         /*    highaddr */ BUS_SPACE_MAXADDR,
	         /*      filter */ NULL,
	         /*   filterarg */ NULL,
	         /*     maxsize */ MJUM16BYTES,
	         /*   nsegments */ ionic_rx_sg_size ? IONIC_RX_MAX_SG_ELEMS : 1,
	         /*  maxsegsize */ ionic_rx_sg_size ? ionic_rx_sg_size : MJUM16BYTES,
	         /*       flags */ 0,
	         /*    lockfunc */ NULL,
	         /* lockfuncarg */ NULL,
	                           &rxq->buf_tag);

	if (error) {
		IONIC_QUE_ERROR(rxq,
		    "failed to create DMA tag, err: %d\n", error);
		goto error_out;
	}

	for (rxbuf = rxq->rxbuf, i = 0; rxbuf != NULL && i < num_descs;
	    i++, rxbuf++) {
		error = bus_dmamap_create(rxq->buf_tag, 0, &rxbuf->dma_map);
		if (error) {
			IONIC_QUE_ERROR(rxq,
			    "failed to create map for entry%d, err: %d\n",
			    i, error);
			bus_dma_tag_destroy(rxq->buf_tag);
			goto error_out;
		}
	}

	lro = &rxq->lro;
	error = tcp_lro_init(lro);
	if (error) {
		IONIC_QUE_WARN(rxq, "LRO setup failed, error: %d\n", error);
		goto error_out;
	}
	lro->ifp = lif->netdev;

	/* Setup interrupt for rx and tx queue pairs. */
	error = ionic_setup_rx_intr(rxq);
	if (error) {
		IONIC_QUE_WARN(rxq, "interrupt setup failed, error: %d\n",
		    error);
		goto error_out;
	}

	*prxq = rxq;
	return (0);

error_out:
	ionic_dev_intr_unreserve(lif, &rxq->intr);

	if (rxq->cmd_ring) {
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

/*
 * Allocate transmit queue. Tx shares interrupt with Rx.
 */
static int
ionic_txque_alloc(struct ionic_lif *lif, unsigned int qnum,
    unsigned int num_descs, unsigned int pid, struct ionic_txque **ptxq)
{
	struct ionic_txque *txq;
	struct ionic_tx_buf *txbuf;
	int i, error = ENOMEM;
	uint32_t cmd_ring_size, comp_ring_size, sg_ring_size, total_size;

	*ptxq = NULL;

	txq = malloc(sizeof(*txq), M_IONIC, M_NOWAIT | M_ZERO);
	if(txq == NULL) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to allocate txq%d\n", qnum);
		return (error);
	}

	snprintf(txq->name, sizeof(txq->name) - 1, "txq%d", qnum);
	snprintf(txq->mtx_name, sizeof(txq->mtx_name) - 1, "tx%d_mtx", qnum);
	txq->lif = lif;
	txq->type = IONIC_QTYPE_TXQ;
	txq->index = qnum;
	txq->num_descs = num_descs;
	txq->pid = pid;
	txq->ver = lif->ionic->ident.txq.version;

	IONIC_TX_LOCK_INIT(txq);

	/* Setup command ring. */
	txq->txbuf = malloc(sizeof(*txq->txbuf) * num_descs, M_IONIC,
	    M_NOWAIT | M_ZERO);
	if (txq->txbuf == NULL) {
		IONIC_QUE_ERROR(txq, "Couldn't allocate tx buffer memory\n");
		goto failed_alloc;
	}

	/* Allocate DMA for command and completion rings. */
	cmd_ring_size = sizeof(*txq->cmd_ring) * num_descs;
	comp_ring_size = sizeof(*txq->comp_ring) * num_descs;
	if (txq->ver == 1) {
		sg_ring_size = sizeof(*txq->sg_ring) *
		    IONIC_TX_SG_DESC_STRIDE_V1 * num_descs;
	} else if (txq->ver == 0) {
		sg_ring_size = sizeof(*txq->sg_ring) *
		    IONIC_TX_SG_DESC_STRIDE * num_descs;
	} else {
		IONIC_QUE_ERROR(txq, "invalid queue version %d\n", txq->ver);
		goto failed_alloc;
	}

	total_size = ALIGN(cmd_ring_size, PAGE_SIZE) +
	    ALIGN(cmd_ring_size, PAGE_SIZE) + ALIGN(sg_ring_size, PAGE_SIZE);
	txq->total_ring_size = total_size;
	error = ionic_dma_alloc(txq->lif->ionic, total_size,
	    &txq->cmd_dma, 0);
	if (error) {
		IONIC_QUE_ERROR(txq,
		    "failed to allocate txq ring, err: %d\n", error);
		goto failed_alloc;
	}

	txq->cmd_ring_pa = txq->cmd_dma.dma_paddr;
	txq->cmd_ring = (struct txq_desc *)txq->cmd_dma.dma_vaddr;
	IONIC_QUE_INFO(txq,
	    "cmd base pa: 0x%lx size: 0x%x comp size: 0x%x total size: 0x%x\n",
	    txq->cmd_ring_pa, cmd_ring_size, comp_ring_size, total_size);
	/*
	 * Command and completion rings are side by side.
	 */
	txq->comp_ring_pa = txq->cmd_ring_pa + ALIGN(cmd_ring_size, PAGE_SIZE);
	txq->comp_ring = (struct txq_comp *)(txq->cmd_dma.dma_vaddr +
	    ALIGN(cmd_ring_size, PAGE_SIZE));

	txq->sg_ring_pa = txq->comp_ring_pa + ALIGN(comp_ring_size, PAGE_SIZE);
	if (txq->ver == 1) {
		txq->max_sg_elems = IONIC_TX_MAX_SG_ELEMS_V1;
		txq->sg_desc_stride = IONIC_TX_SG_DESC_STRIDE_V1;
	} else if (txq->ver == 0) {
		txq->max_sg_elems = IONIC_TX_MAX_SG_ELEMS;
		txq->sg_desc_stride = IONIC_TX_SG_DESC_STRIDE;
	} else {
		IONIC_QUE_ERROR(txq, "invalid queue version %d\n", txq->ver);
		goto failed_alloc;
	}

	txq->sg_ring = (struct txq_sg_elem *)(txq->cmd_dma.dma_vaddr +
	    ALIGN(cmd_ring_size, PAGE_SIZE) +
	    ALIGN(comp_ring_size, PAGE_SIZE));;

	/* Allocate buffer ring. */
	txq->br = buf_ring_alloc(4096, M_IONIC, M_WAITOK, &txq->tx_mtx);
	if (txq->br == NULL) {
		IONIC_QUE_ERROR(txq, "failed to allocate buffer ring\n");
		goto failed_alloc;
	}

	/*
	 * Create one tag for non-TSO buffers and another one for TSO buffers.
	 */
	error = bus_dma_tag_create(
	         /*      parent */ bus_get_dma_tag(txq->lif->ionic->dev->bsddev),
	         /*   alignment */ 1,
	         /*      bounds */ 0,
	         /*     lowaddr */ IONIC_MAX_ADDR,
	         /*    highaddr */ BUS_SPACE_MAXADDR,
	         /*      filter */ NULL,
	         /*   filterarg */ NULL,
	         /*     maxsize */ IONIC_MAX_TSO_SIZE,
	         /* HW supports one segment on desc ring and max_sg_elems on sg ring */
	         /*   nsegments */ txq->max_sg_elems + 1,
	         /*  maxsegsize */ IONIC_MAX_TSO_SEG_SIZE,
	         /*       flags */ 0,
	         /*    lockfunc */ NULL,
	         /* lockfuncarg */ NULL,
	                           &txq->buf_tag);

	if (error) {
		IONIC_QUE_ERROR(txq,
		    "failed to create DMA tag, err: %d\n", error);
		goto failed_alloc;
	}

	for (txbuf = txq->txbuf, i = 0; txbuf != NULL && i < num_descs;
	    i++, txbuf++) {
		error = bus_dmamap_create(txq->buf_tag, 0, &txbuf->dma_map);
		if (error) {
			IONIC_QUE_ERROR(txq,
			    "failed to create map for entry%d, err: %d\n",
			    i, error);
			bus_dma_tag_destroy(txq->buf_tag);
			goto failed_alloc;
		}
	}

	error = bus_dma_tag_create(
	         /*      parent */ bus_get_dma_tag(txq->lif->ionic->dev->bsddev),
	         /*   alignment */ 1,
	         /*      bounds */ 0,
	         /*     lowaddr */ IONIC_MAX_ADDR,
	         /*    highaddr */ BUS_SPACE_MAXADDR,
	         /*      filter */ NULL,
	         /*   filterarg */ NULL,
	         /*     maxsize */ IONIC_MAX_TSO_SIZE,
	         /*   nsegments */ IONIC_MAX_TSO_SEGMENTS,
	         /*  maxsegsize */ IONIC_MAX_TSO_SEG_SIZE,
	         /*       flags */ 0,
	         /*    lockfunc */ NULL,
	         /* lockfuncarg */ NULL,
	                           &txq->tso_buf_tag);

	if (error) {
		IONIC_QUE_ERROR(txq, "failed to create TSO DMA tag, err: %d\n",
		    error);
		bus_dma_tag_destroy(txq->buf_tag);
		goto failed_alloc;
	}

	for (txbuf = txq->txbuf, i = 0; txbuf != NULL && i < num_descs;
	    i++, txbuf++) {
		error = bus_dmamap_create(txq->tso_buf_tag, 0,
		    &txbuf->tso_dma_map);
		if (error) {
			IONIC_QUE_ERROR(txq,
			    "failed to create map for %d, err: %d\n", i, error);
			bus_dma_tag_destroy(txq->buf_tag);
			bus_dma_tag_destroy(txq->tso_buf_tag);
			goto failed_alloc;
		}
	}

	*ptxq = txq;
	return (0);

failed_alloc:
	if (txq->br) {
		buf_ring_free(txq->br, M_IONIC);
		txq->br = NULL;
	}

	if (txq->cmd_ring) {
		/* Completion ring is part of command ring allocation. */
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

static void
ionic_rxq_free(struct ionic_lif *lif, struct ionic_rxque *rxq)
{

	IONIC_RX_LOCK(rxq);
	ionic_dev_intr_unreserve(lif, &rxq->intr);

	tcp_lro_free(&rxq->lro);
	free(rxq->rxbuf, M_IONIC);

	if (rxq->cmd_ring) {
		ionic_dma_free(rxq->lif->ionic, &rxq->cmd_dma);
		rxq->cmd_ring = NULL;
		rxq->comp_ring = NULL;
	}

	IONIC_RX_UNLOCK(rxq);
	IONIC_RX_LOCK_DESTROY(rxq);
	/*
	 * free_irq() need to be outside since it uses sleepable lock.
	 */
	if (rxq->intr.vector)
		free_irq(rxq->intr.vector, rxq);
	if (rxq->taskq) {
		taskqueue_free(rxq->taskq);
		rxq->taskq = NULL;
	}
	free(rxq, M_IONIC);
}

static void
ionic_txq_free(struct ionic_lif *lif, struct ionic_txque *txq)
{

	IONIC_TX_LOCK(txq);
	if (txq->br) {
		buf_ring_free(txq->br, M_IONIC);
		txq->br = NULL;
	}

	free(txq->txbuf, M_IONIC);

	if (txq->cmd_ring) {
		ionic_dma_free(txq->lif->ionic, &txq->cmd_dma);
		txq->cmd_ring = NULL;
		txq->comp_ring = NULL;
	}

	IONIC_TX_UNLOCK(txq);
	IONIC_TX_LOCK_DESTROY(txq);

	free(txq, M_IONIC);
}

static void
ionic_adminq_free(struct ionic_lif *lif, struct ionic_adminq *adminq)
{

	IONIC_ADMIN_LOCK(adminq);
	ionic_dev_intr_unreserve(lif, &adminq->intr);

	if (adminq->cmd_ring) {
		ionic_dma_free(adminq->lif->ionic, &adminq->cmd_dma);
		adminq->cmd_ring = NULL;
		adminq->comp_ring = NULL;
	}

	if (adminq->ctx_ring)
		free(adminq->ctx_ring, M_IONIC);
	IONIC_ADMIN_UNLOCK(adminq);
	IONIC_ADMIN_LOCK_DESTROY(adminq);

	/*
	 * free_irq() must be outside since it uses sleepable lock.
	 */
	if (adminq->intr.vector)
		free_irq(adminq->intr.vector, adminq);

	free(adminq, M_IONIC);
}

static void
ionic_notifyq_free(struct ionic_lif *lif, struct ionic_notifyq *notifyq)
{

	if (notifyq->taskq) {
		taskqueue_free(notifyq->taskq);
		notifyq->taskq = NULL;
	}

	if (notifyq->intr.vector)
		free_irq(notifyq->intr.vector, notifyq);
	ionic_dev_intr_unreserve(lif, &notifyq->intr);

	if (notifyq->cmd_ring) {
		ionic_dma_free(notifyq->lif->ionic, &notifyq->cmd_dma);
		notifyq->cmd_ring = NULL;
		notifyq->comp_ring = NULL;
	}

	free(notifyq, M_IONIC);
}

/*
 * Allocate all the queues of a LIF.
 */
static int
ionic_qcqs_alloc(struct ionic_lif *lif)
{
	int i, err;

	lif->txqs = malloc(sizeof(*lif->txqs) * lif->ntxqs, M_IONIC,
	    M_ZERO | M_NOWAIT);
	if (!lif->txqs)
		return (ENOMEM);

	lif->rxqs = malloc(sizeof(*lif->rxqs) * lif->nrxqs, M_IONIC,
	    M_ZERO | M_NOWAIT);
	if (!lif->rxqs) {
		free(lif->txqs, M_IONIC);
		lif->txqs = NULL;
		return (ENOMEM);
	}

	err = ionic_adminq_alloc(lif, 0, adminq_descs, lif->kern_pid,
	    &lif->adminq);
	if (err)
		return (err);

	if (lif->ionic->nnqs_per_lif) {
		err = ionic_notifyq_alloc(lif, 0, ionic_notifyq_descs,
		    lif->kern_pid, &lif->notifyq);
		if (err)
			goto err_out_free_adminq;
	}

	for (i = 0; i < lif->ntxqs; i++) {
		err = ionic_txque_alloc(lif, i, ionic_tx_descs, lif->kern_pid,
		    &lif->txqs[i]);
		if (err)
			goto err_out_free_notifyq;
	}

	for (i = 0; i < lif->nrxqs; i++) {
		err = ionic_rxque_alloc(lif, i, ionic_rx_descs, lif->kern_pid,
		    &lif->rxqs[i]);
		if (err)
			goto err_out_free_txqs;
	}

	return (0);

err_out_free_txqs:
	for (i = 0; i < lif->ntxqs; i++)
		ionic_txq_free(lif, lif->txqs[i]);
err_out_free_notifyq:
	if (lif->notifyq)
		ionic_notifyq_free(lif, lif->notifyq);
err_out_free_adminq:
	ionic_adminq_free(lif, lif->adminq);

	free(lif->rxqs, M_IONIC);
	free(lif->txqs, M_IONIC);

	return (err);
}

static void
ionic_qcqs_free(struct ionic_lif *lif)
{
	struct ionic_rxque *rxq;
	struct ionic_txque *txq;
	int i;

	IONIC_LIF_LOCK(lif);
	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		ionic_rxq_free(lif, rxq);
	}
	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		ionic_txq_free(lif, txq);
	}

	if (lif->notifyq)
		ionic_notifyq_free(lif, lif->notifyq);
	if (lif->adminq)
		ionic_adminq_free(lif, lif->adminq);

	IONIC_LIF_UNLOCK(lif);

	free(lif->rxqs, M_IONIC);
	free(lif->txqs, M_IONIC);
}

int
ionic_setup_intr_coal(struct ionic_lif *lif, int coal)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct identity *ident = &lif->ionic->ident;
	struct ifnet *ifp = lif->netdev;
	struct ionic_rxque *rxq;
	uint32_t intr_coal;
	int i;

	if (ident->dev.intr_coal_div == 0)
		return (ENXIO);

	if (coal > lif->intr_coalesce_max_us) {
		IONIC_NETDEV_ERROR(ifp, "value %d out of range, max: %d\n",
		    coal, lif->intr_coalesce_max_us);
		return (ERANGE);
	}

	if (coal == lif->intr_coalesce_us)
		return (0);

	lif->intr_coalesce_us = coal;

	intr_coal = coal * ident->dev.intr_coal_mult / ident->dev.intr_coal_div;

	/*
	 * In case user set a value which is min that what we can set,
	 * roundup the value.
	 */
	if (!intr_coal && coal) {
		intr_coal = 1;
	}

	IONIC_NETDEV_INFO(ifp, "New intr coal: %d\n", intr_coal);
	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		ionic_intr_coal_init(idev->intr_ctrl, rxq->intr.index,
		    intr_coal);
	}

	return (0);
}

static inline void
ionic_ifmedia_add(struct ifmedia *ifm, int m)
{

	ifmedia_add(ifm, m, 0, NULL);
	ifmedia_add(ifm, m | IFM_FDX | IFM_ETH_TXPAUSE | IFM_ETH_RXPAUSE, 0, NULL);
}

static void
ionic_lif_ifnet_init(struct ionic_lif *lif)
{
	struct ifnet *ifp;

	ifp = lif->netdev;
	lif->max_frame_size = ifp->if_mtu + ETHER_HDR_LEN +
	    ETHER_VLAN_ENCAP_LEN + ETHER_CRC_LEN;
	/* Adverise h/w TSO limits. */
	ifp->if_hw_tsomax = IONIC_MAX_TSO_SIZE - (ETHER_HDR_LEN + ETHER_CRC_LEN);
	/* HW supports one segment on desc ring and max_sg_elems on sg ring */
	ifp->if_hw_tsomaxsegcount = IONIC_MAX_TSO_SEGMENTS;
	ifp->if_hw_tsomaxsegsize = IONIC_MAX_TSO_SEG_SIZE;

	ifmedia_init(&lif->media, IFM_IMASK, ionic_media_change, ionic_media_status);

	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_AUTO);

	if (lif->ionic->is_mgmt_nic) {
		ifmedia_add(&lif->media, IFM_ETHER | IFM_1000_KX, 0, NULL);
		return;
	}

	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_100G_CR4);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_100G_SR4);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_100G_LR4);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_40G_CR4);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_40G_SR4);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_40G_LR4);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_25G_CR);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_25G_SR);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_25G_LR);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_25G_AOC);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_10G_SR);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_10G_LR);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_10G_LRM);
	ionic_ifmedia_add(&lif->media, IFM_ETHER | IFM_10G_ER);

        ifmedia_set(&lif->media, IFM_ETHER | IFM_AUTO | IFM_FDX |
            IFM_ETH_RXPAUSE | IFM_ETH_TXPAUSE);
}

static int
ionic_lif_alloc(struct ionic *ionic, unsigned int index)
{
	struct device *dev = ionic->dev;
	struct identity *ident = &ionic->ident;
	struct ionic_lif *lif;
	int err, dbpage_num;
	char name[16];

	lif = malloc(sizeof(*lif), M_IONIC, M_NOWAIT | M_ZERO);
	if (!lif) {
		dev_err(dev, "Cannot allocate lif, aborting\n");
		return (-ENOMEM);
	}

	snprintf(lif->name, sizeof(lif->name), "%s%u", DRV_NAME, index);
	snprintf(lif->sx_name, sizeof(lif->sx_name), "%s-core", lif->name);
	IONIC_LIF_LOCK_INIT(lif);
	lif->ionic = ionic;
	lif->index = index;
	lif->neqs = ionic->neqs_per_lif;
	lif->ntxqs = ionic->ntxqs_per_lif;
	lif->nrxqs = ionic->nrxqs_per_lif;
	lif->nnqs = ionic->nnqs_per_lif;

	lif->mc_addrs =
	    malloc(sizeof(struct ionic_mc_addr) * ionic->ident.lif.eth.max_mcast_filters,
	    M_IONIC, M_NOWAIT | M_ZERO);
	if (!lif->mc_addrs) {
		dev_err(dev, "Cannot allocate mc_addrs, aborting\n");
		err = -ENOMEM;
		goto err_out_free_lif;
	}

	snprintf(name, sizeof(name), "adwq%d", index);
	lif->adminq_wq = create_singlethread_workqueue(name);
	if (!lif->adminq_wq) {
		dev_err(dev, "Cannot allocate admin wq, aborting\n");
		err = -ENOMEM;
		goto err_out_free_mcaddrs;
	}

	snprintf(name, sizeof(name), "wdwq%d", index);
	lif->wdog_wq = create_singlethread_workqueue(name);
	if (!lif->wdog_wq) {
		dev_err(dev, "Cannot allocate wdog wq, aborting\n");
		err = -ENOMEM;
		goto err_out_free_adminwq;
	}

	INIT_DELAYED_WORK(&lif->adq_hb_work, ionic_adminq_hb_work);
	INIT_DELAYED_WORK(&lif->txq_wdog_work, ionic_txq_wdog_work);
	IONIC_WDOG_LOCK_INIT(lif);

	lif->adq_hb_interval =
		(unsigned long)ionic_adminq_hb_interval * HZ / 1000;
	lif->txq_wdog_timeout =
		(unsigned long)ionic_txq_wdog_timeout * HZ / 1000;

	mutex_init(&lif->dbid_inuse_lock);
	lif->dbid_count = lif->ionic->ident.dev.ndbpgs_per_lif;
	if (!lif->dbid_count) {
		dev_err(dev, "No doorbell pages, aborting\n");
		err = -EINVAL;
		goto err_out_free_wdogwq;
	}

	lif->dbid_inuse = malloc(BITS_TO_LONGS(lif->dbid_count) * sizeof(long),
	    M_IONIC, M_NOWAIT | M_ZERO);
	if (!lif->dbid_inuse) {
		dev_err(dev, "Failed alloc doorbell id bitmap, aborting\n");
		err = -ENOMEM;
		goto err_out_free_wdogwq;
	}

	/* first doorbell id reserved for kernel (dbid aka pid == zero) */
	set_bit(0, lif->dbid_inuse);
	lif->kern_pid = 0;

	dbpage_num = ionic_db_page_num(ionic, index, 0);
	lif->kern_dbpage = ionic_bus_map_dbpage(ionic, dbpage_num);
	if (!lif->kern_dbpage) {
		dev_err(dev, "Cannot map dbpage, aborting\n");
		err = -ENOMEM;
		goto err_out_free_dbid;
	}

	/* Allocate lif info */
	lif->info_sz = ALIGN(sizeof(*lif->info), PAGE_SIZE);

	err = ionic_dma_alloc(lif->ionic, lif->info_sz, &lif->info_dma, 0);
	if (err) {
		dev_err(dev, "failed to allocate lif registers, err: %d\n", err);
		goto err_out_unmap_dbell;
	}

	lif->info_pa = lif->info_dma.dma_paddr;
	lif->info = (struct lif_info *)lif->info_dma.dma_vaddr;
	if (!lif->info) {
		dev_err(dev, "failed to allocate lif registers\n");
		err = -ENOMEM;
		goto err_out_unmap_dbell;
	}

	/* Allocate netdev; takes a reference on ifp */
	err = ionic_lif_netdev_alloc(lif, ionic_tx_descs);
	if (err) {
		dev_err(dev, "Cannot allocate netdev, aborting\n");
		goto err_out_free_info;
	}

	/* Allocate queues */
	err = ionic_qcqs_alloc(lif);
	if (err)
		goto err_out_free_netdev;

	/* Allocate rss indirection table */
	err = ionic_lif_rss_alloc(lif);
	if (err)
		goto err_out_free_qs;

	lif->intr_coalesce_max_us = INTR_CTRL_COAL_MAX * ident->dev.intr_coal_div /
	    ident->dev.intr_coal_mult;
	/* Setup tunables. */
	ionic_setup_intr_coal(lif, ionic_intr_coalesce);

	/* All queues are initialized, setup legacy interrupts now. */
	if (ionic_enable_msix == 0) {
		err = ionic_setup_legacy_intr(lif);
		if (err) {
			IONIC_NETDEV_ERROR(lif->netdev,
			    "Legacy interrupt setup failed, error = %d\n", err);
			goto err_out_free_rss;
		}
	}

	/* Register for VLAN events */
	lif->vlan_attach = EVENTHANDLER_REGISTER(vlan_config,
	    ionic_register_vlan, lif, EVENTHANDLER_PRI_FIRST);
	lif->vlan_detach = EVENTHANDLER_REGISTER(vlan_unconfig,
	    ionic_unregister_vlan, lif, EVENTHANDLER_PRI_FIRST);
	list_add_tail(&lif->list, &ionic->lifs);

	return (0);

err_out_free_rss:
	ionic_lif_rss_free(lif);
err_out_free_qs:
	ionic_qcqs_free(lif);
err_out_free_netdev:
	ionic_lif_netdev_free(lif);
err_out_free_info:
	ionic_dma_free(lif->ionic, &lif->info_dma);
err_out_unmap_dbell:
	ionic_bus_unmap_dbpage(ionic, lif->kern_dbpage);
err_out_free_dbid:
	free(lif->dbid_inuse, M_IONIC);
err_out_free_wdogwq:
	destroy_workqueue(lif->wdog_wq);
err_out_free_adminwq:
	destroy_workqueue(lif->adminq_wq);
err_out_free_mcaddrs:
	free(lif->mc_addrs, M_IONIC);
err_out_free_lif:
	IONIC_LIF_LOCK_DESTROY(lif);
	free(lif, M_IONIC);

	return (err);
}

int
ionic_lifs_alloc(struct ionic *ionic)
{
	int err;

	INIT_LIST_HEAD(&ionic->lifs);

	/* only build the first lif, others are for dynamic macvlan or rdma */
	set_bit(0, ionic->lifbits);
	set_bit(0, ionic->ethbits);

	err = ionic_lif_alloc(ionic, 0);
	if (err) {
		clear_bit(0, ionic->ethbits);
		clear_bit(0, ionic->lifbits);
		return (err);
	}

	return (0);
}

static void
ionic_lif_free(struct ionic_lif *lif)
{

	/* Unregister VLAN events */
	if (lif->vlan_attach != NULL)
		EVENTHANDLER_DEREGISTER(vlan_config, lif->vlan_attach);
	if (lif->vlan_detach != NULL)
		EVENTHANDLER_DEREGISTER(vlan_unconfig, lif->vlan_detach);

	/* Free legacy interrupt resources. */
	if (ionic_enable_msix == 0)
		free_irq(lif->ionic->pdev->irq, lif);

	/* destroy deferred contexts */
	flush_workqueue(lif->adminq_wq);
	destroy_workqueue(lif->adminq_wq);

	flush_workqueue(lif->wdog_wq);
	destroy_workqueue(lif->wdog_wq);
	IONIC_WDOG_LOCK_DESTROY(lif);

	/* free rss indirection table */
	ionic_lif_rss_free(lif);

	/* free queues */
	ionic_qcqs_free(lif);

	/* free netdev */
	ionic_lif_netdev_free(lif);

	/* free multicast addrs */
	free(lif->mc_addrs, M_IONIC);

	/* free lif info */
	ionic_dma_free(lif->ionic, &lif->info_dma);

	/* unmap doorbell page */
	ionic_bus_unmap_dbpage(lif->ionic, lif->kern_dbpage);
	free(lif->dbid_inuse, M_IONIC);

	/* free lif */
	list_del(&lif->list);
	IONIC_LIF_LOCK_DESTROY(lif);
	free(lif, M_IONIC);
}

void
ionic_lifs_free(struct ionic *ionic)
{
	struct list_head *cur, *tmp;
	struct ionic_lif *lif;

	list_for_each_safe(cur, tmp, &ionic->lifs) {
		lif = list_entry(cur, struct ionic_lif, list);
		ionic_lif_free(lif);
	}
}

int
ionic_lif_rss_config(struct ionic_lif *lif, uint16_t types,
    const uint8_t *key, const uint32_t *indir)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_setattr = {
			.opcode = CMD_OPCODE_LIF_SETATTR,
			.attr = IONIC_LIF_ATTR_RSS,
			.rss.types = types,
			.rss.addr = lif->rss_ind_tbl_pa,
		},
	};
	int i;

	lif->rss_types = types;

	if (key)
		memcpy(lif->rss_hash_key, key, IONIC_RSS_HASH_KEY_SIZE);

	if (indir)
		for (i = 0; i < lif->ionic->ident.lif.eth.rss_ind_tbl_sz; i++)
			lif->rss_ind_tbl[i] = indir[i];

	memcpy(ctx.cmd.lif_setattr.rss.key, lif->rss_hash_key,
	    IONIC_RSS_HASH_KEY_SIZE);

	return (ionic_adminq_post_wait(lif, &ctx));
}

static int
ionic_lif_rss_init(struct ionic_lif *lif)
{
#ifdef	RSS
	uint8_t toeplitz_symmetric_key[40];
	rss_getkey((uint8_t *) &toeplitz_symmetric_key);
#else
	static const uint8_t toeplitz_symmetric_key[] = {
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
	};
#endif
	int i, err;

	lif->rss_types = ionic_set_rss_type();

	for (i = 0; i < lif->ionic->ident.lif.eth.rss_ind_tbl_sz; i++) {
#ifdef RSS
		lif->rss_ind_tbl[i] = rss_get_indirection_to_bucket(i) % lif->nrxqs;
#else
		lif->rss_ind_tbl[i] = i % lif->nrxqs;
#endif
	}

	err = ionic_lif_rss_config(lif, lif->rss_types, toeplitz_symmetric_key,
	    NULL);
	if (err)
		goto err_out_free;

	return (0);

err_out_free:
	ionic_lif_rss_deinit(lif);
	return (err);
}

static int
ionic_lif_rss_deinit(struct ionic_lif *lif)
{

	/* Disable RSS on the NIC */
	return (ionic_lif_rss_config(lif, 0x0, NULL, NULL));
}

static int
ionic_lif_rss_alloc(struct ionic_lif *lif)
{
	int error;

	lif->rss_ind_tbl_sz = sizeof(*lif->rss_ind_tbl) * \
		lif->ionic->ident.lif.eth.rss_ind_tbl_sz;

	error = ionic_dma_alloc(lif->ionic, lif->rss_ind_tbl_sz,
	    &lif->rss_dma, 0);
	if (error) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to allocate RSS dma, err: %d\n", error);
		return (ENOMEM);
	}

	lif->rss_ind_tbl = (uint8_t *)lif->rss_dma.dma_vaddr;
	lif->rss_ind_tbl_pa = lif->rss_dma.dma_paddr;

	return (0);
}

static void
ionic_lif_rss_free(struct ionic_lif *lif)
{

	if (!lif->rss_ind_tbl)
		return;

	ionic_dma_free(lif->ionic, &lif->rss_dma);

	lif->rss_ind_tbl = NULL;
	lif->rss_ind_tbl_pa = 0;
}

static void
ionic_lif_adminq_deinit(struct ionic_lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct ionic_adminq *adminq = lif->adminq;

	if (adminq->intr.index == INTR_INDEX_NOT_ASSIGNED)
		return;

	IONIC_ADMIN_LOCK(adminq);
	adminq->stop = true;

	ionic_intr_mask(idev->intr_ctrl, adminq->intr.index,
	    IONIC_INTR_MASK_SET);
	IONIC_ADMIN_UNLOCK(adminq);
}

static void
ionic_lif_notifyq_disable(struct ionic_lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct ionic_notifyq *notifyq = lif->notifyq;

	if (notifyq == NULL)
		return;

	ionic_intr_mask(idev->intr_ctrl, notifyq->intr.index,
	    IONIC_INTR_MASK_SET);
}

static void
ionic_lif_notifyq_drain(struct ionic_lif *lif)
{
	struct ionic_notifyq *notifyq = lif->notifyq;

	if (notifyq == NULL || notifyq->taskq == NULL)
		return;

	while (taskqueue_cancel(notifyq->taskq, &notifyq->task, NULL) != 0)
		taskqueue_drain(notifyq->taskq, &notifyq->task);
}

static void
ionic_lif_txqs_disable(struct ionic_lif *lif)
{
	struct ionic_txque *txq;
	int i;

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		ionic_txq_disable(txq);
	}
}

static void
ionic_lif_txqs_clean_empty(struct ionic_lif *lif)
{
	struct ionic_txque *txq;
	int i;

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];

		/*
		 * RxQ deinit may schedule the task for tx clean but it's
		 * not guaranteed, so do explicit clean here.
		 */
		IONIC_TX_LOCK(txq);
		ionic_tx_clean(txq, txq->num_descs);
		ionic_tx_empty(txq);
		IONIC_TX_UNLOCK(txq);
	}
}

static void
ionic_lif_rxqs_disable(struct ionic_lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct ionic_rxque *rxq;
	int i;

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		ionic_intr_mask(idev->intr_ctrl, rxq->intr.index,
		    IONIC_INTR_MASK_SET);
		ionic_rxq_disable(rxq);
		if (rxq->taskq) {
			taskqueue_drain(rxq->taskq, &rxq->tx_task);
			taskqueue_drain(rxq->taskq, &rxq->task);
		}
	}
}

static void
ionic_lif_rxqs_clean_empty(struct ionic_lif *lif)
{
	struct ionic_rxque *rxq;
	int i;

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];

		IONIC_RX_LOCK(rxq);
		ionic_rx_clean(rxq, rxq->num_descs);
		ionic_rx_empty(rxq);
		IONIC_RX_UNLOCK(rxq);
	}
}

static void
ionic_lif_reset(struct ionic_lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	int err;

	IONIC_DEV_LOCK(lif->ionic);
	ionic_dev_cmd_lif_reset(idev, lif->index);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	if (err)
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to reset lif, error = %d\n", err);

	IONIC_DEV_UNLOCK(lif->ionic);
}

int
ionic_lif_quiesce(struct ionic_lif *lif)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_setattr = {
			.opcode = CMD_OPCODE_LIF_SETATTR,
			.attr = IONIC_LIF_ATTR_STATE,
			.index = lif->index,
			.state = IONIC_LIF_DISABLE
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "failed to quiesce lif, error = %d\n", err);
		return (err);
	}

	return (0);
}

static void
ionic_lif_deinit(struct ionic_lif *lif)
{

	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s is not locked", lif->name));

	/* NB: Caller must have already stopped notifyq! */
	ionic_rx_filters_deinit(lif);
	ionic_lif_rss_deinit(lif);

	ionic_lif_rxqs_disable(lif);
	ionic_lif_txqs_disable(lif);
	ionic_lif_quiesce(lif);
	ionic_lif_txqs_clean_empty(lif);
	ionic_lif_rxqs_clean_empty(lif);
	ionic_lif_adminq_deinit(lif);

	ionic_lif_reset(lif);
}

void
ionic_lifs_deinit(struct ionic *ionic)
{
	struct list_head *cur;
	struct ionic_lif *lif;

	list_for_each(cur, &ionic->lifs) {
		lif = list_entry(cur, struct ionic_lif, list);

		/* Shut down the watchdogs */
		ionic_adminq_hb_stop(lif);
		ionic_txq_wdog_stop(lif);

		/* Stop the NotifyQ */
		ionic_lif_notifyq_disable(lif);
		ionic_lif_notifyq_drain(lif);

		/* Block and drain the AdminQ workqueue */
		ionic_adminq_wq_disable_drain(lif);

		/* NB: Above work must be done before taking the LIF lock! */
		IONIC_LIF_LOCK(lif);
		ionic_lif_deinit(lif);
		IONIC_LIF_UNLOCK(lif);
	}
}

static int
ionic_lif_adminq_init(struct ionic_lif *lif)
{
	struct ionic_adminq *adminq = lif->adminq;
	struct ionic_dev *idev = &lif->ionic->idev;
	struct q_init_comp comp;
	int err = 0;

	union dev_cmd cmd = {
		.q_init.opcode = CMD_OPCODE_Q_INIT,
		.q_init.lif_index = lif->index,
		.q_init.cos = lif->cos,
		.q_init.type = adminq->type,
		.q_init.index = adminq->index,
		.q_init.flags = (IONIC_QINIT_F_IRQ | IONIC_QINIT_F_ENA),
		.q_init.pid = adminq->pid,
		.q_init.intr_index = adminq->intr.index,
		.q_init.ring_size = ilog2(adminq->num_descs),
		.q_init.ring_base = adminq->cmd_ring_pa,
		.q_init.cq_ring_base = adminq->comp_ring_pa,
	};

	adminq->head_index = adminq->tail_index = adminq->comp_index = 0;
	adminq->done_color = 1;

	bzero(adminq->cmd_ring, adminq->total_ring_size);
	IONIC_DEV_LOCK(lif->ionic);
	ionic_dev_cmd_go(idev, &cmd);

	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	ionic_dev_cmd_comp(idev, &comp);
	IONIC_DEV_UNLOCK(lif->ionic);
	if (err)
		return (err);

	IONIC_ADMIN_LOCK(adminq);
	adminq->hw_type = comp.hw_type;
	adminq->hw_index = comp.hw_index;
	adminq->dbval = IONIC_DBELL_QID(adminq->hw_index);
	adminq->stop = false;
	adminq->wq_stop = false;

	ionic_intr_mask(idev->intr_ctrl, adminq->intr.index,
	    IONIC_INTR_MASK_CLEAR);
	IONIC_ADMIN_UNLOCK(adminq);

	return (err);
}

static bool
ionic_process_event(struct ionic_notifyq* notifyq, union notifyq_comp *comp)
{
	struct ionic_lif *lif = notifyq->lif;
	struct ifnet *ifp = lif->netdev;
	uint64_t comp_eid;

	if (__IONIC_DEBUG) {
		IONIC_NETDEV_INFO(ifp, " notifyq event:\n");
		print_hex_dump_debug("event ", DUMP_PREFIX_OFFSET, 16, 1,
		    comp, sizeof(union notifyq_comp), true);
	}

	comp_eid = le64_to_cpu(comp->event.eid);
	if ((int64_t)(comp_eid - lif->last_eid) <= 0) {
		return (false);
	}

	if (comp_eid != lif->last_eid + 1) {
		IONIC_QUE_WARN(notifyq,
		    "NQ OOO event, eid: %ld != expected eid: %ld\n",
		    comp_eid, lif->last_eid + 1);
	}

	lif->last_eid = comp_eid;

	switch (comp->event.ecode) {
	case EVENT_OPCODE_LINK_CHANGE:
		IONIC_LIF_LOCK(lif);
		lif->link_speed = comp->link_change.link_speed;
		lif->link_up = comp->link_change.link_status ?  true : false;

		ionic_get_link_status(lif);
		if_printf(ifp, "[eid:%ld]link status: %s speed: %d\n",
		    comp_eid, (lif->link_up ? "up" : "down"),
		    lif->link_speed);
		ionic_open_or_stop(lif);

		IONIC_LIF_UNLOCK(lif);
		break;

	case EVENT_OPCODE_RESET:
		if_printf(ifp, "[eid:%ld]reset code: %d state: %d\n",
		    comp_eid, comp->reset.reset_code,
		    comp->reset.state);
		break;

	case EVENT_OPCODE_HEARTBEAT:
		if_printf(ifp, "[eid:%ld]heartbeat\n", comp_eid);
		break;

	case EVENT_OPCODE_LOG:
		if_printf(ifp, "[eid:%ld]log \n", comp_eid);
		print_hex_dump(KERN_INFO, "nq log", DUMP_PREFIX_OFFSET,
		    16, 1, comp->log.data, sizeof(comp->log.data), true);
		break;

	case EVENT_OPCODE_XCVR:
		if_printf(ifp, "[eid:%ld]xcvr event\n", comp_eid);
		ionic_ifmedia_xcvr(lif);
		break;

	default:
		if_printf(ifp, "[eid:%ld]unknown event: %d\n",
		    comp_eid, comp->event.ecode);
		break;
	}

	return (true);
}

/*
 * Process notifyQ events in a task.
 */
int
ionic_notifyq_clean(struct ionic_notifyq* notifyq)
{
	int i, comp_index;
	union notifyq_comp *comp;

	/* Sync every time descriptors. */
	bus_dmamap_sync(notifyq->cmd_dma.dma_tag, notifyq->cmd_dma.dma_map,
	    BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);

	IONIC_QUE_INFO(notifyq, " enter comp index: %d\n",
	    notifyq->comp_index);

	for (i = 0; i < notifyq->num_descs; i++) {
		comp_index = notifyq->comp_index;
		comp = &notifyq->comp_ring[comp_index];
		if (!ionic_process_event(notifyq, comp))
			break;
		notifyq->comp_index = IONIC_MOD_INC(notifyq, comp_index);
	}

	notifyq->comp_count += i;

	return (i);
}

static void
ionic_notifyq_task_handler(void *arg, int pendindg)
{
	struct ionic_notifyq* notifyq = arg;
	struct ionic_dev *idev = &notifyq->lif->ionic->idev;
	int processed;

	KASSERT(notifyq, ("notifyq == NULL"));

	processed = ionic_notifyq_clean(notifyq);

	if (!processed)
		IONIC_QUE_INFO(notifyq, "nothing processed in notifyq\n");

	ionic_intr_credits(idev->intr_ctrl, notifyq->intr.index,
	    processed, IONIC_INTR_CRED_REARM);
}

static irqreturn_t
ionic_notifyq_isr(int irq, void *data)
{
	struct ionic_notifyq* notifyq = data;

	notifyq->isr_count++;
	/* Schedule the task to process the notifyQ events. */
	taskqueue_enqueue(notifyq->taskq, &notifyq->task);

	return (IRQ_HANDLED);
}

static int
ionic_lif_notifyq_init(struct ionic_lif *lif,
    struct ionic_notifyq *notifyq)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	int err;

	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_init = {
			.opcode = CMD_OPCODE_Q_INIT,
			.lif_index = lif->index,
			.cos = lif->cos,
			.type = notifyq->type,
			.index = notifyq->index,
			.flags = (IONIC_QINIT_F_IRQ | IONIC_QINIT_F_ENA),
			.intr_index = notifyq->intr.index,
			.pid = notifyq->pid,
			.ring_size = ilog2(notifyq->num_descs),
			.ring_base = notifyq->cmd_ring_pa,
		},
	};

	notifyq->comp_index = 0;
	lif->last_eid = 1;	/* Valid events are non zero. */
	bzero(notifyq->cmd_ring, notifyq->total_ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err) {
		return (err);
	}

	notifyq->hw_type = ctx.comp.q_init.hw_type;
	notifyq->hw_index = ctx.comp.q_init.hw_index;

	ionic_intr_mask(idev->intr_ctrl, notifyq->intr.index,
	    IONIC_INTR_MASK_CLEAR);

	return (0);
}

/*************************** Transmit ************************/
int
ionic_tx_clean(struct ionic_txque *txq, int tx_limit)
{
	struct txq_comp *comp;
	struct ionic_tx_buf *txbuf;
	int comp_index, processed, cmd_stop_index, batch = 0;
	struct ionic_tx_stats *stats = &txq->stats;

	KASSERT(IONIC_TX_LOCK_OWNED(txq), ("%s is not locked", txq->name));
	stats->clean++;

	bus_dmamap_sync(txq->cmd_dma.dma_tag, txq->cmd_dma.dma_map,
	    BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);

	for (processed = 0; processed < tx_limit; processed++) {
		comp_index = txq->comp_index;

		comp = &txq->comp_ring[comp_index];
		cmd_stop_index = comp->comp_index;

		if (!color_match(comp->color, txq->done_color))
			break;

		batch++;

		txbuf = &txq->txbuf[cmd_stop_index];
		if (comp->status) {
			stats->comp_err++;
		}
		/* TSO last buffer points to head mbuf. */
		if (txbuf->m != NULL) {
			IONIC_TX_TRACE(txq,
			    "took %lu tsc to free %d descs txbuf index @:%d\n",
			    rdtsc() - txbuf->timestamp, batch, cmd_stop_index);
			if (txbuf->is_tso) {
				bus_dmamap_sync(txq->tso_buf_tag,
				    txbuf->tso_dma_map, BUS_DMASYNC_POSTWRITE);
				bus_dmamap_unload(txq->tso_buf_tag,
				    txbuf->tso_dma_map);
			} else {
				bus_dmamap_sync(txq->buf_tag, txbuf->dma_map,
				    BUS_DMASYNC_POSTWRITE);
				bus_dmamap_unload(txq->buf_tag, txbuf->dma_map);
			}
			m_freem(txbuf->m);
			txbuf->m = NULL;
			txbuf->is_tso = 0;
			batch = 0;
		}

		txq->comp_index = IONIC_MOD_INC(txq, comp_index);
		txq->tail_index = (cmd_stop_index + 1) % txq->num_descs;
		/* Roll over condition, flip color. */
		if (txq->comp_index == 0) {
			txq->done_color = !txq->done_color;
		}
	}

	return (processed);
}

static void
ionic_tx_empty(struct ionic_txque *txq)
{
	struct ionic_tx_buf *txbuf;

	for (; !IONIC_Q_EMPTY(txq);
	    txq->tail_index = IONIC_MOD_INC(txq, tail_index)) {
		txbuf = &txq->txbuf[txq->tail_index];
		if (txbuf->m == NULL)
			continue;

		if (txbuf->is_tso) {
			bus_dmamap_sync(txq->tso_buf_tag,
			    txbuf->tso_dma_map, BUS_DMASYNC_POSTWRITE);
			bus_dmamap_unload(txq->tso_buf_tag,
			    txbuf->tso_dma_map);
		} else {
			bus_dmamap_sync(txq->buf_tag, txbuf->dma_map,
			    BUS_DMASYNC_POSTWRITE);
			bus_dmamap_unload(txq->buf_tag, txbuf->dma_map);
		}

		m_freem(txbuf->m);
		txbuf->m = NULL;
	}
}

static int
ionic_lif_txq_init(struct ionic_lif *lif, struct ionic_txque *txq)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_init = {
			.opcode = CMD_OPCODE_Q_INIT,
			.lif_index = lif->index,
			.cos = lif->cos,
			.type = txq->type,
			.index = txq->index,
			.ver = txq->ver,
			.flags = (IONIC_QINIT_F_IRQ | IONIC_QINIT_F_SG),
			.intr_index = lif->rxqs[txq->index]->intr.index,
			.pid = txq->pid,
			.ring_base = txq->cmd_ring_pa,
			.ring_size = ilog2(txq->num_descs),
			.cq_ring_base = txq->comp_ring_pa,
			.sg_ring_base = txq->sg_ring_pa,
		},
	};
	int err;

	txq->head_index = txq->tail_index = txq->comp_index = 0;
	txq->done_color = 1;
	bzero(txq->cmd_ring, txq->total_ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return (err);

	txq->hw_type = ctx.comp.q_init.hw_type;
	txq->hw_index = ctx.comp.q_init.hw_index;
	txq->dbval = IONIC_DBELL_QID(txq->hw_index);
	txq->full = false;

	return (0);
}

static int
ionic_lif_txqs_init(struct ionic_lif *lif)
{
	int i, err;

	for (i = 0; i < lif->ntxqs; i++) {
		err = ionic_lif_txq_init(lif, lif->txqs[i]);
		if (err)
			return (err);
	}

	return (0);
}

void
ionic_tx_ring_doorbell(struct ionic_txque *txq, int index)
{

	ionic_dbell_ring(txq->lif->kern_dbpage, txq->hw_type,
	    txq->dbval | index);
}

/*******************************  RX side. ******************************/
static void
ionic_rx_ring_doorbell(struct ionic_rxque *rxq, int index)
{

	ionic_dbell_ring(rxq->lif->kern_dbpage, rxq->hw_type,
	    rxq->dbval | index);
}

void
ionic_rx_fill(struct ionic_rxque *rxq)
{
	struct ionic_rx_buf *rxbuf;
	int error, index;

	KASSERT(IONIC_RX_LOCK_OWNED(rxq), ("%s is not locked", rxq->name));

	index = 0;
	/* Fill till there is only one slot left empty which is Q full. */
	while (!IONIC_Q_FULL(rxq)) {
		index = rxq->head_index;
		rxbuf = &rxq->rxbuf[index];

		KASSERT((rxbuf->m == NULL),
		    ("%s: rxbuf not empty for %d", rxq->name, index));
		if ((error = ionic_rx_mbuf_alloc(rxq, index,
		    rxq->lif->rx_mbuf_size))) {
			IONIC_QUE_ERROR(rxq,
			    "mbuf alloc failed for: %d, error: %d\n",
			    index, error);
			break;
		}

		rxq->head_index = IONIC_MOD_INC(rxq, head_index);

		if (index % ionic_rx_stride == 0)
			ionic_rx_ring_doorbell(rxq, index);
	 }

	/* If we haven't rung the doorbell for remaining descriptors. */
	if (index % ionic_rx_stride)
		ionic_rx_ring_doorbell(rxq, index);

	IONIC_RX_TRACE(rxq, "head: %d tail: %d desc_posted: %d\n",
		       rxq->head_index, rxq->tail_index, IONIC_Q_LENGTH(rxq));
}

/*
 * Refill the rx queue, called during reinit.
 */
static void
ionic_rx_refill(struct ionic_rxque *rxq)
{
	struct ionic_rx_buf *rxbuf;
	int i, count = 0, error;

	KASSERT(IONIC_RX_LOCK_OWNED(rxq), ("%s is not locked", rxq->name));
	for (i = rxq->tail_index; i != rxq->head_index;
	    i = (i + 1) % rxq->num_descs) {
		rxbuf = &rxq->rxbuf[i];

		KASSERT(rxbuf->m,
		    ("%s: ionic_rx_refill rxbuf empty for %d",
		    rxq->name, i));
		ionic_rx_mbuf_free(rxq, rxbuf);
		if ((error = ionic_rx_mbuf_alloc(rxq, i,
		    rxq->lif->rx_mbuf_size))) {
			IONIC_QUE_ERROR(rxq,
			    "mbuf alloc failed for index: %d, error: %d\n",
			    i, error);
			break;
		}
		count++;
	};

	IONIC_RX_TRACE(rxq, "head: %d tail: %d refilled: %d\n",
	    rxq->head_index, rxq->tail_index, count);
}

/*
 * Empty Rx queue buffer, called from queue teardown.
 */
static void
ionic_rx_empty(struct ionic_rxque *rxq)
{
	struct ionic_rx_buf *rxbuf;

	KASSERT(IONIC_RX_LOCK_OWNED(rxq), ("%s is not locked", rxq->name));
	IONIC_RX_TRACE(rxq, "head: %d tail: %d desc_posted: %d\n",
	    rxq->head_index, rxq->tail_index, IONIC_Q_LENGTH(rxq));

	while (!IONIC_Q_EMPTY(rxq)) {
		rxbuf = &rxq->rxbuf[rxq->tail_index];
		KASSERT(rxbuf->m, ("%s: ionic_rx_empty rxbuf empty for %d",
		    rxq->name, rxq->tail_index));
		ionic_rx_mbuf_free(rxq, rxbuf);
		rxq->tail_index = IONIC_MOD_INC(rxq, tail_index);
	};

	IONIC_RX_TRACE(rxq, "head: %d tail: %d desc_posted: %d\n",
	    rxq->head_index, rxq->tail_index, IONIC_Q_LENGTH(rxq));
}

/*
 * Called from Rx completion paths.
 */
int
ionic_rx_clean(struct ionic_rxque *rxq , int rx_limit)
{
	struct rxq_comp *comp;
	struct rxq_desc *cmd;
	struct ionic_rx_buf *rxbuf;
	int i, comp_index, cmd_index;

	KASSERT(IONIC_RX_LOCK_OWNED(rxq), ("%s is not locked", rxq->name));
	IONIC_RX_TRACE(rxq, "comp: %d head: %d tail: %d desc_posted: %d\n",
	    rxq->comp_index, rxq->head_index, rxq->tail_index,
	    IONIC_Q_LENGTH(rxq));

	/* Sync descriptors. */
	bus_dmamap_sync(rxq->cmd_dma.dma_tag, rxq->cmd_dma.dma_map,
	    BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);

	/*
	 * Process Rx descriptors for the given limit or
	 * no more completions.
	 */
	for (i = 0; i < rx_limit; i++) {
		comp_index = rxq->comp_index;
		comp = &rxq->comp_ring[comp_index];

		if (!color_match(comp->pkt_type_color, rxq->done_color))
			break;

		IONIC_RX_TRACE(rxq, "comp: %d expected color: %d"
		    " color: %d nsegs: %d len: %d desc_posted: %d\n",
		    comp_index,
		    comp->pkt_type_color & IONIC_COMP_COLOR_MASK ? 1 : 0,
		    rxq->done_color, comp->num_sg_elems, comp->len,
		    IONIC_Q_LENGTH(rxq));

		if (IONIC_Q_EMPTY(rxq)) {
			IONIC_QUE_ERROR(rxq,
			    "rx completion on empty queue\n");
			continue;
		}

		cmd_index = rxq->tail_index;
		rxbuf = &rxq->rxbuf[cmd_index];
		cmd = &rxq->cmd_ring[cmd_index];

		ionic_rx_input(rxq, rxbuf, comp, cmd);

		rxq->comp_index = IONIC_MOD_INC(rxq, comp_index);
		/* Roll over condition, flip color. */
		if (rxq->comp_index == 0) {
			rxq->done_color = !rxq->done_color;
		}

		rxq->tail_index = IONIC_MOD_INC(rxq, tail_index);
	}

	IONIC_RX_TRACE(rxq, "comp: %d head: %d tail: %d desc_posted: %d"
	    " processed: %d\n", rxq->comp_index, rxq->head_index,
	    rxq->tail_index, IONIC_Q_LENGTH(rxq), i);

	return (i);
}

static int
ionic_lif_rxq_init(struct ionic_lif *lif, struct ionic_rxque *rxq)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_init = {
			.opcode = CMD_OPCODE_Q_INIT,
			.lif_index = lif->index,
			.cos = lif->cos,
			.type = rxq->type,
			.index = rxq->index,
			.flags = (IONIC_QINIT_F_IRQ | IONIC_QINIT_F_SG),
			.intr_index = rxq->intr.index,
			.pid = rxq->pid,
			.ring_size = ilog2(rxq->num_descs),
			.ring_base = rxq->cmd_ring_pa,
			.cq_ring_base = rxq->comp_ring_pa,
			.sg_ring_base = rxq->sg_ring_pa,
		},
	};
	int err;

	rxq->head_index = rxq->tail_index = rxq->comp_index = 0;
	rxq->done_color = 1;
	bzero(rxq->cmd_ring, rxq->total_ring_size);

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return (err);

	rxq->hw_type = ctx.comp.q_init.hw_type;
	rxq->hw_index = ctx.comp.q_init.hw_index;
	rxq->dbval = IONIC_DBELL_QID(rxq->hw_index);

	return (err);
}

static int
ionic_lif_rxqs_init(struct ionic_lif *lif)
{
	int i, err;

	for (i = 0; i < lif->nrxqs; i++) {
		err = ionic_lif_rxq_init(lif, lif->rxqs[i]);
		if (err)
			return (err);
	}

	return (0);
}

static int
ionic_ifmedia_xcvr(struct ionic_lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	unsigned int media = IFM_ETHER;

	switch(idev->port_info->status.xcvr.pid) {
	/* Copper */
	case XCVR_PID_QSFP_100G_CR4:
		media |= IFM_100G_CR4;
		break;

	case XCVR_PID_QSFP_40GBASE_CR4:
		media |= IFM_40G_CR4;
		break;

	case XCVR_PID_SFP_25GBASE_CR_S:
	case XCVR_PID_SFP_25GBASE_CR_L:
	case XCVR_PID_SFP_25GBASE_CR_N:
		media |= IFM_25G_CR;
		break;

	/* Fiber */
	case XCVR_PID_QSFP_100G_SR4:
	case XCVR_PID_QSFP_100G_AOC:
		media |= IFM_100G_SR4;
		break;

	case XCVR_PID_QSFP_100G_LR4:
	case XCVR_PID_QSFP_100G_ER4:
	case XCVR_PID_QSFP_100G_CWDM4:
	case XCVR_PID_QSFP_100G_PSM4:
		media |= IFM_100G_LR4;
		break;

	case XCVR_PID_QSFP_40GBASE_SR4:
	case XCVR_PID_QSFP_40GBASE_AOC:
		media |= IFM_40G_SR4;
		break;

	case XCVR_PID_QSFP_40GBASE_LR4:
		media |= IFM_40G_LR4;
		break;

	case XCVR_PID_SFP_25GBASE_SR:
		media |= IFM_25G_SR;
		break;

	case XCVR_PID_SFP_25GBASE_LR:
		media |= IFM_25G_LR;
		break;

	case XCVR_PID_SFP_25GBASE_AOC:
		media |= IFM_25G_AOC;
		break;

	case XCVR_PID_SFP_25GBASE_ACC:
		media |= IFM_25G_ACC;
		break;

	case XCVR_PID_SFP_10GBASE_SR:
		media |= IFM_10G_SR;
		break;

	case XCVR_PID_SFP_10GBASE_LR:
		media |= IFM_10G_LR;
		break;

	case XCVR_PID_SFP_10GBASE_LRM:
		media |= IFM_10G_LRM;
		break;

	case XCVR_PID_SFP_10GBASE_ER:
		media |= IFM_10G_ER;
		break;

	case XCVR_PID_QSFP_100G_ACC:
	case XCVR_PID_QSFP_40GBASE_ER4:
	case XCVR_PID_SFP_25GBASE_ER:
	case XCVR_PID_SFP_10GBASE_AOC:
	case XCVR_PID_SFP_10GBASE_CU:
	case XCVR_PID_UNKNOWN:
	default:
		media |= IFM_UNKNOWN;
		break;
	}

	return (media);
}

static void
ionic_media_status(struct ifnet *ifp, struct ifmediareq *ifmr)
{
	struct ionic_lif *lif = ifp->if_softc;
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;

	IONIC_LIF_LOCK(lif);
	ifmr->ifm_status = IFM_AVALID;
	ifmr->ifm_active = IFM_ETHER;

	if (!lif->link_up) {
		IONIC_LIF_UNLOCK(lif);
		return;
	}

	ifp->if_baudrate = IF_Mbps(lif->link_speed);

	ifmr->ifm_status |= IFM_ACTIVE;
	ifmr->ifm_active |= IFM_FDX;

	if (ionic->is_mgmt_nic) {
		ifmr->ifm_active |= IFM_1000_KX;
		IONIC_LIF_UNLOCK(lif);
		return;
	}

	ifmr->ifm_active |= ionic_ifmedia_xcvr(lif);

	if (idev->port_info->config.pause_type & IONIC_PAUSE_F_RX)
		ifmr->ifm_active |= IFM_ETH_RXPAUSE;
	if (idev->port_info->config.pause_type & IONIC_PAUSE_F_TX)
		ifmr->ifm_active |= IFM_ETH_TXPAUSE;

	IONIC_LIF_UNLOCK(lif);
}

static int
ionic_media_change(struct ifnet *ifp)
{
	struct ionic_lif *lif = ifp->if_softc;
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;
	struct ifmedia *ifm = &lif->media;
	uint32_t speed;
	uint8_t an_enable, pause_type;
	int err;

	if (IFM_TYPE(ifm->ifm_media) != IFM_ETHER)
		return (EINVAL);

	if (ionic->is_mgmt_nic)
		return (ENODEV);

	switch (IFM_SUBTYPE(ifm->ifm_media)) {
	case IFM_100G_CR4:
	case IFM_100G_SR4:
	case IFM_100G_LR4:
		speed = IONIC_SPEED_100G;
		an_enable = 0;
		break;
	case IFM_50G_CR2:
	case IFM_50G_KR2:
		speed = IONIC_SPEED_50G;
		an_enable = 0;
		break;
	case IFM_40G_CR4:
	case IFM_40G_SR4:
	case IFM_40G_LR4:
		speed = IONIC_SPEED_40G;
		an_enable = 0;
		break;
	case IFM_25G_CR:
	case IFM_25G_SR:
	case IFM_25G_LR:
	case IFM_25G_AOC:
		speed = IONIC_SPEED_25G;
		an_enable = 0;
		break;
	case IFM_10G_SR:
	case IFM_10G_LR:
	case IFM_10G_LRM:
	case IFM_10G_ER:
		speed = IONIC_SPEED_10G;
		an_enable = 0;
		break;
	case IFM_AUTO:
	default:
		speed = 0;
		an_enable = 1;
	}

	pause_type = idev->port_info->config.pause_type;
	pause_type &= ~(IONIC_PAUSE_F_TX | IONIC_PAUSE_F_RX);
	if (ifm->ifm_media & IFM_ETH_RXPAUSE)
		pause_type |= IONIC_PAUSE_F_RX;
	if (ifm->ifm_media & IFM_ETH_TXPAUSE)
		pause_type |= IONIC_PAUSE_F_TX;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_port_autoneg(idev, an_enable);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	if (err) {
		IONIC_DEV_UNLOCK(ionic);
		IONIC_NETDEV_ERROR(ifp,
		    "failed to set autoneg, error = %d\n", err);
		return (err);
	}

	ionic_dev_cmd_port_speed(idev, speed);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	if (err) {
		IONIC_DEV_UNLOCK(ionic);
		IONIC_NETDEV_ERROR(ifp,
		    "failed to set speed, error = %d\n", err);
		return (err);
	}

	ionic_dev_cmd_port_pause(idev, pause_type);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	if (err) {
		IONIC_DEV_UNLOCK(ionic);
		IONIC_NETDEV_ERROR(ifp,
		    "failed to set pause, error = %d\n", err);
		return (err);
	}

	IONIC_DEV_UNLOCK(ionic);

	return (0);
}

static void
ionic_lif_set_netdev_info(struct ionic_lif *lif)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_setattr = {
			.opcode = CMD_OPCODE_LIF_SETATTR,
			.index = lif->index,
			.attr = IONIC_LIF_ATTR_NAME,
		},
	};

	strlcpy(ctx.cmd.lif_setattr.name, lif->netdev->if_xname,
	    sizeof(ctx.cmd.lif_setattr.name));

	IONIC_NETDEV_INFO(lif->netdev, "SET_NETDEV_INFO %s\n",
	    ctx.cmd.lif_setattr.name);

	(void)ionic_adminq_post_wait(lif, &ctx);
}

int
ionic_lif_reset_stats(struct ionic_lif *lif)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_setattr = {
			.opcode = CMD_OPCODE_LIF_SETATTR,
			.index = lif->index,
			.attr = IONIC_LIF_ATTR_STATS_CTRL,
			.stats_ctl = STATS_CTL_RESET,
		},
	};

	IONIC_NETDEV_INFO(lif->netdev, "RESET_LIF_STATS %s\n",
	    ctx.cmd.lif_setattr.name);

	return (ionic_adminq_post_wait(lif, &ctx));
}
/*
 * Register device MAC address.
 */
static int
ionic_station_add(struct ionic_lif *lif)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_getattr = {
			.opcode = CMD_OPCODE_LIF_GETATTR,
			.index = lif->index,
			.attr = IONIC_LIF_ATTR_MAC,
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return (err);

	if (!is_zero_ether_addr(lif->dev_addr)) {
		ionic_addr_del(lif->netdev, lif->dev_addr);
	}
	memcpy(lif->dev_addr, ctx.comp.lif_getattr.mac,
	       ETHER_ADDR_LEN);

	ionic_addr_add(lif->netdev, lif->dev_addr);
	return (0);
}

int
ionic_set_mac(struct ifnet *ifp)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	uint8_t *addr;
	int err = 0;

	KASSERT(lif, ("lif is NULL"));
	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));

	if (!ifp->if_addr || !ifp->if_addr->ifa_addr) {
		IONIC_NETDEV_ERROR(lif->netdev, "No MAC configured\n");
		return (ENXIO);
	}

	addr = IF_LLADDR(ifp);
	if (is_zero_ether_addr(addr)) {
		IONIC_NETDEV_ERROR(lif->netdev, "Invalid MAC %6D\n", addr, ":");
		return (ENXIO);
	}

	/* if mac addr has not changed then nop */
	if (bcmp(addr, lif->dev_addr, ETHER_ADDR_LEN) == 0) {
		return (0);
	}

	/* add filter for new mac addr */
	err = ionic_addr_add(lif->netdev, addr);
	if (err) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "Failed to add MAC %6D, err: %d\n", lif->dev_addr,
		    ":", err);
		return (err);
	}

	/* remove the filter for old mac addr */
	err = ionic_addr_del(lif->netdev, lif->dev_addr);
	if (err) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "Failed to delete MAC %6D, err: %d\n", lif->dev_addr,
		    ":", err);
		return (err);
	}

	IONIC_NETDEV_INFO(lif->netdev, "Changed MAC from %6D to %6D\n",
	    lif->dev_addr, ":", IF_LLADDR(ifp), ":");

	bcopy(addr, lif->dev_addr, ETHER_ADDR_LEN);

	return (err);
}

static int
ionic_firmware_download(struct ionic_lif *lif, uint64_t addr,
    uint32_t offset, uint32_t length) {
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.fw_download = {
			.opcode = CMD_OPCODE_FW_DOWNLOAD,
			.offset = offset,
			.addr = addr,
			.length = length
		}
	};

	return (ionic_adminq_post_wait(lif, &ctx));
}

static int
ionic_firmware_install(struct ionic_lif *lif, uint8_t *slot)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.fw_control = {
			.opcode = CMD_OPCODE_FW_CONTROL,
			.oper = IONIC_FW_INSTALL
		}
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return (err);

	*slot = ctx.comp.fw_control.slot;
	return (0);
}

static int
ionic_firmware_activate(struct ionic_lif *lif, uint8_t slot)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.fw_control = {
			.opcode = CMD_OPCODE_FW_CONTROL,
			.oper = IONIC_FW_ACTIVATE,
			.slot = slot
		}
	};

	return (ionic_adminq_post_wait(lif, &ctx));
}

int
ionic_firmware_update(struct ionic_lif *lif, const void *const fw_data,
    size_t fw_sz)
{
	struct ionic_dma_info fw_dma;
	struct ifnet *ifp = lif->netdev;
	dma_addr_t buf_pa;
	void *buf;
	uint32_t buf_sz, copy_sz, offset;
	uint8_t fw_slot;
	int err;

	/* Transfer 1MB at a time. */
	buf_sz = (1 << 20);

	/* Allocate DMA'ble buffer for sending firmware to card */
	err = ionic_dma_alloc(lif->ionic, buf_sz, &fw_dma,
	    BUS_DMA_NOWAIT);
	if (err) {
		IONIC_NETDEV_ERROR(ifp,
		"failed to allocate buffer, err: %d\n", err);
		return (ENOMEM);
	}

	buf = (void *)fw_dma.dma_vaddr;
	buf_pa = fw_dma.dma_paddr;

	IONIC_LIF_LOCK(lif);
	offset = 0;
	while (offset < fw_sz) {
		copy_sz = min(buf_sz, fw_sz - offset);
		bcopy(fw_data + offset, buf, copy_sz);
		err = ionic_firmware_download(lif, buf_pa, offset, copy_sz);
		if (err) {
			IONIC_NETDEV_ERROR(ifp,
			    "upload failed at offset: 0x%x addr 0x%lx"
			    " len 0x%x\n", offset, buf_pa, copy_sz);
			goto err_out_free_buf;
		}
		offset += copy_sz;
	}

	if (ionic_firmware_install(lif, &fw_slot)) {
		IONIC_NETDEV_INFO(ifp, "firmware install failed\n");
		err = EIO;
		goto err_out_free_buf;
	}

	if (ionic_firmware_activate(lif, fw_slot)) {
		IONIC_NETDEV_INFO(ifp, "firmware activation failed\n");
		err = EIO;
		goto err_out_free_buf;
	}

err_out_free_buf:
	ionic_dma_free(lif->ionic, &fw_dma);
	IONIC_LIF_UNLOCK(lif);

	return (err);
}

static int
ionic_lif_init(struct ionic_lif *lif, bool wdog_reset_path)
{
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;
	struct ifnet *ifp = lif->netdev;
	struct q_init_comp comp;
	int err;

	KASSERT(IONIC_LIF_LOCK_OWNED(lif), ("%s lif not locked", lif->name));

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_lif_init(idev, lif->index, lif->info_pa);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	ionic_dev_cmd_comp(idev, &comp);
	IONIC_DEV_UNLOCK(ionic);
	if (err) {
		IONIC_NETDEV_ERROR(ifp, "lif init failed, error = %d\n",
		    err);
		return (err);
	}

	lif->hw_index = comp.hw_index;

	err = ionic_lif_adminq_init(lif);
	if (err) {
		IONIC_NETDEV_ERROR(ifp,
		    "adminq init failed, error = %d\n", err);
		return (err);
	}

	/* Enable notifyQ and arm it. */
	if (lif->notifyq) {
		err = ionic_lif_notifyq_init(lif, lif->notifyq);
		if (err) {
			IONIC_NETDEV_ERROR(ifp,
			    "notifyq init failed, error = %d\n", err);
			goto err_out_adminq_deinit;
		}
	}

	err = ionic_lif_txqs_init(lif);
	if (err)
		goto err_out_notifyq_disable;

	err = ionic_lif_rxqs_init(lif);
	if (err)
		goto err_out_txqs_deinit;

	err = ionic_lif_rss_init(lif);
	if (err) {
		IONIC_NETDEV_ERROR(ifp, "rss_init failed, error = %d\n",
		    err);
		goto err_out_rxqs_deinit;
	}

	err = ionic_rx_filters_init(lif);
	if (err) {
		IONIC_NETDEV_ERROR(ifp,
		    "filter init failed, error = %d\n",err);
		goto err_out_rss_deinit;
	}

	ionic_get_link_status(lif);

	/* Start the watchdogs */
	if (!wdog_reset_path) {
		IONIC_WDOG_LOCK(lif);
		lif->adq_hb_resched = true;
		queue_delayed_work(lif->wdog_wq, &lif->adq_hb_work,
		    lif->adq_hb_interval);

		lif->txq_wdog_resched = true;
		queue_delayed_work(lif->wdog_wq, &lif->txq_wdog_work,
		    lif->txq_wdog_timeout);
		IONIC_WDOG_UNLOCK(lif);
	}

	return (err);

err_out_rss_deinit:
	ionic_lif_rss_deinit(lif);
err_out_rxqs_deinit:
	ionic_lif_rxqs_disable(lif);
	ionic_lif_rxqs_clean_empty(lif);
err_out_txqs_deinit:
	ionic_lif_txqs_disable(lif);
	ionic_lif_txqs_clean_empty(lif);
err_out_notifyq_disable:
	/* NB: Drain later, after dropping LIF lock */
	ionic_lif_notifyq_disable(lif);
err_out_adminq_deinit:
	ionic_lif_adminq_deinit(lif);

	return (err);
}

int
ionic_lifs_init(struct ionic *ionic)
{
	struct list_head *cur;
	struct ionic_lif *lif;
	int err;

	list_for_each(cur, &ionic->lifs) {
		lif = list_entry(cur, struct ionic_lif, list);
		IONIC_LIF_LOCK(lif);
		err = ionic_lif_init(lif, false);
		IONIC_LIF_UNLOCK(lif);
		if (err) {
			/* Wait until after unlock to drain NotifyQ */
			ionic_lif_notifyq_drain(lif);
			return (err);
		}
	}

	return (0);
}

/*
 * Reinit the LIF which represent individual network port.
 * Stack is not involved in it, only link flap.
 * XXX: If the reinit fails due to a dev cmd timeout, the device will be
 *      disabled.
 */
int
ionic_lif_reinit(struct ionic_lif *lif, bool wdog_reset_path)
{
	struct ifnet *ifp;
	struct ionic_mc_addr *mc;
	int i, error = 0;
	uint32_t rx_mode;
	bool was_open, drain_notifyq = false;

	ifp = lif->netdev;

	IONIC_LIF_LOCK(lif);
	if (lif->reinit_in_progress) {
		IONIC_QUE_INFO(lif->adminq, "reset already in progress\n");
		goto out_unlock;
	}
	lif->reinit_in_progress = true;
	IONIC_LIF_UNLOCK(lif);

	IONIC_NETDEV_INFO(ifp, "resetting device\n");

	/*
	 * Shut down the watchdogs only if we are NOT
	 * cleaning up after a watchdog timeout (wdog_reset_path)
	 */
	if (!wdog_reset_path) {
		ionic_adminq_hb_stop(lif);
		ionic_txq_wdog_stop(lif);
	}

	/* Stop the NotifyQ */
	ionic_lif_notifyq_disable(lif);
	ionic_lif_notifyq_drain(lif);

	/* Block and drain the AdminQ workqueue */
	ionic_adminq_wq_disable_drain(lif);

	/* NB: Above work must be done before taking the LIF lock! */

	IONIC_LIF_LOCK(lif);
	lif->num_resets++;
	was_open = false;
	if (ifp->if_drv_flags & IFF_DRV_RUNNING) {
		was_open = true;
		lif->stop = true;
		ifp->if_drv_flags &= ~IFF_DRV_RUNNING;
	}

	ionic_lif_deinit(lif);

	/* LIF reset is already done by deinit. */

	error = ionic_lif_init(lif, wdog_reset_path);
	if (error) {
		IONIC_NETDEV_ERROR(ifp, "init failed, error = %d\n", error);
		drain_notifyq = true;
		goto out_abort;
	}

	if (is_zero_ether_addr(lif->dev_addr)) {
		IONIC_NETDEV_ERROR(ifp, "station address is missing\n");
		error = EINVAL;
		goto out_abort;
	}

	ionic_addr_add(ifp, lif->dev_addr);

	ionic_set_hw_features(lif, lif->hw_features);

	/* Program the Rx mode. */
	rx_mode = lif->rx_mode;
	lif->rx_mode = 0;
	ionic_lif_rx_mode(lif, rx_mode);

	ionic_lif_set_netdev_info(lif);

	/* Program the multicast list. */
	for (i = 0; i < lif->num_mc_addrs; i++) {
		mc = &lif->mc_addrs[i];
		error = ionic_addr_add(ifp, mc->addr);
		if (error)
			goto out_abort;
		IONIC_NETDEV_ADDR_INFO(ifp, mc->addr, "reinit, adding");
	}

	ionic_reinit_vlan(lif);

	if (was_open) {
		lif->stop = false;
		ionic_hw_open(lif);
		ifp->if_drv_flags |= IFF_DRV_RUNNING;
	}

out_abort:
	lif->reinit_in_progress = false;
out_unlock:
	IONIC_LIF_UNLOCK(lif);

	/* Wait until after unlock to drain NotifyQ */
	if (drain_notifyq)
		ionic_lif_notifyq_drain(lif);

	return (error);
}
/*
 * Configure the NIC for required capabilities.
 */
int
ionic_set_hw_features(struct ionic_lif *lif, uint32_t features)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_setattr = {
			.opcode = CMD_OPCODE_LIF_SETATTR,
			.index = lif->index,
			.attr = IONIC_LIF_ATTR_FEATURES,
			.features = features,
		},
	};
	int err;

	err = ionic_adminq_post_wait(lif, &ctx);
	if (err)
		return (err);

	lif->hw_features = ctx.cmd.lif_setattr.features &
						ctx.comp.lif_setattr.features;

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

	return (0);
}

/*
 * Set HW & SW capabilities
 */
static int
ionic_set_features(struct ionic_lif *lif, uint32_t features)
{
	int err;

	if (features == lif->hw_features) {
		IONIC_NETDEV_INFO(lif->netdev, "features unchanged\n");
		return (0);
	}

	err = ionic_set_hw_features(lif, features);
	if (err)
		return (err);

	err = ionic_set_os_features(lif->netdev, lif->hw_features);

	return (err);
}

/*
 * Register the network port to stack.
 */
static int
ionic_lif_register(struct ionic_lif *lif)
{
	struct ifnet *ifp;
	int err;

	ifp = lif->netdev;

	IONIC_LIF_LOCK(lif);
	err = ionic_station_add(lif);
	if (err) {
		IONIC_NETDEV_ERROR(ifp,
		    "station_add failed, error = %d\n", err);
		IONIC_LIF_UNLOCK(lif);
		return (EIO);
	}

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
	if (err) {
		IONIC_NETDEV_ERROR(ifp,
		    "set_features failed, error = %d\n", err);
		IONIC_LIF_UNLOCK(lif);
		return (EIO);
	}

	/* Initializes ifnet, ifmedia */
	ionic_lif_ifnet_init(lif);
	ionic_setup_sysctls(lif);

	lif->stay_registered = true;
	ether_ifattach(ifp, lif->dev_addr);

	ionic_lif_set_netdev_info(lif);
	lif->registered = true;

	IONIC_LIF_UNLOCK(lif);

	return (0);
}

struct ionic_lif *
ionic_netdev_lif(struct ifnet *ifp)
{

	if (!ifp || ifp->if_transmit != ionic_start_xmit)
		return (NULL);

	return (if_getsoftc(ifp));
}

int
ionic_lifs_register(struct ionic *ionic)
{
	struct list_head *cur;
	struct ionic_lif *lif;
	int err;

	list_for_each(cur, &ionic->lifs) {
		lif = list_entry(cur, struct ionic_lif, list);
		err = ionic_lif_register(lif);
		if (err)
			return (err);
	}

	return (0);
}

void
ionic_lifs_unregister(struct ionic *ionic)
{
	struct list_head *cur;
	struct ionic_lif *lif;

	list_for_each(cur, &ionic->lifs) {
		lif = list_entry(cur, struct ionic_lif, list);

		IONIC_LIF_LOCK(lif);
		lif->registered = false;
		lif->stay_registered = false;
		IONIC_LIF_UNLOCK(lif);

		/* NB: This must be done outside of the LIF lock! */
		ifmedia_removeall(&lif->media);
		ether_ifdetach(lif->netdev);
		ionic_lif_sysctl_free(lif);
	}
}

int
ionic_lif_identify(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	struct identity *ident = &ionic->ident;
	int i, err, nwords;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_lif_identify(idev, IONIC_LIF_TYPE_CLASSIC,
	    IONIC_IDENTITY_VERSION_1);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	if (err) {
		IONIC_DEV_UNLOCK(ionic);
		return (err);
	}

	nwords = min(ARRAY_SIZE(ident->lif.words),
	    ARRAY_SIZE(idev->dev_cmd_regs->data));
	for (i = 0; i < nwords; i++)
		ident->lif.words[i] = ioread32(&idev->dev_cmd_regs->data[i]);
	IONIC_DEV_UNLOCK(ionic);

	IONIC_DEV_INFO(ionic->dev, "capabilities 0x%lx\n",
	    ident->lif.capabilities);
	IONIC_DEV_INFO(ionic->dev, "eth.features 0x%lx\n",
	    ident->lif.eth.config.features);
	IONIC_DEV_INFO(ionic->dev, "eth.queue_count[IONIC_QTYPE_ADMINQ] %d\n",
	    ident->lif.eth.config.queue_count[IONIC_QTYPE_ADMINQ]);
	IONIC_DEV_INFO(ionic->dev, "eth.queue_count[IONIC_QTYPE_NOTIFYQ] %d\n",
	    ident->lif.eth.config.queue_count[IONIC_QTYPE_NOTIFYQ]);
	IONIC_DEV_INFO(ionic->dev, "eth.queue_count[IONIC_QTYPE_RXQ] %d\n",
	    ident->lif.eth.config.queue_count[IONIC_QTYPE_RXQ]);
	IONIC_DEV_INFO(ionic->dev, "eth.queue_count[IONIC_QTYPE_TXQ] %d\n",
	    ident->lif.eth.config.queue_count[IONIC_QTYPE_TXQ]);
	IONIC_DEV_INFO(ionic->dev, "eth.max_ucast_filters %d\n",
	    ident->lif.eth.max_ucast_filters);
	IONIC_DEV_INFO(ionic->dev, "eth.max_mcast_filters %d\n",
	    ident->lif.eth.max_mcast_filters);

	IONIC_DEV_INFO(ionic->dev, "rdma.version %d\n",
	    ident->lif.rdma.version);
	IONIC_DEV_INFO(ionic->dev, "rdma.qp_opcodes %d\n",
	    ident->lif.rdma.qp_opcodes);
	IONIC_DEV_INFO(ionic->dev, "rdma.admin_opcodes %d\n",
	    ident->lif.rdma.admin_opcodes);
	IONIC_DEV_INFO(ionic->dev, "rdma.npts_per_lif %d\n",
	    ident->lif.rdma.npts_per_lif);
	IONIC_DEV_INFO(ionic->dev, "rdma.nmrs_per_lif %d\n",
	    ident->lif.rdma.nmrs_per_lif);
	IONIC_DEV_INFO(ionic->dev, "rdma.nahs_per_lif %d\n",
	    ident->lif.rdma.nahs_per_lif);
	IONIC_DEV_INFO(ionic->dev,
	    "rdma.aq.qtype %d rdma.aq.base %d rdma.aq.count %d\n",
	    ident->lif.rdma.aq_qtype.qtype, ident->lif.rdma.aq_qtype.qid_base,
	    ident->lif.rdma.aq_qtype.qid_count);
	IONIC_DEV_INFO(ionic->dev,
	    "rdma.sq.qtype %d rdma.sq.base %d rdma.sq.count %d\n",
	    ident->lif.rdma.sq_qtype.qtype, ident->lif.rdma.sq_qtype.qid_base,
	    ident->lif.rdma.sq_qtype.qid_count);
	IONIC_DEV_INFO(ionic->dev,
	    "rdma.rq.qtype %d rdma.rq.base %d rdma.rq.count %d\n",
	    ident->lif.rdma.rq_qtype.qtype, ident->lif.rdma.rq_qtype.qid_base,
	    ident->lif.rdma.rq_qtype.qid_count);
	IONIC_DEV_INFO(ionic->dev,
	    "rdma.cq.qtype %d rdma.cq.base %d rdma.cq.count %d\n",
	    ident->lif.rdma.cq_qtype.qtype, ident->lif.rdma.cq_qtype.qid_base,
	    ident->lif.rdma.cq_qtype.qid_count);
	IONIC_DEV_INFO(ionic->dev,
	    "rdma.eq.qtype %d rdma.eq.base %d rdma.eq.count %d\n",
	    ident->lif.rdma.eq_qtype.qtype, ident->lif.rdma.eq_qtype.qid_base,
	    ident->lif.rdma.eq_qtype.qid_count);

	return (0);
}

/* TODO: This function needs to be completely redesigned */
int
ionic_lifs_size(struct ionic *ionic)
{
	struct identity *ident = &ionic->ident;
	int neqs = ident->lif.rdma.eq_qtype.qid_count;
	int nnqs = ident->lif.eth.config.queue_count[IONIC_QTYPE_NOTIFYQ];
	int ntxqs = ident->lif.eth.config.queue_count[IONIC_QTYPE_TXQ];
	int nrxqs = ident->lif.eth.config.queue_count[IONIC_QTYPE_RXQ];
	/* Tx and Rx Qs are in pair. */
	int err, nqs = min(ntxqs, nrxqs);
	int nintrs, dev_nintrs = ident->dev.nintrs;

	ionic->nlifs = le32_to_cpu(ident->dev.nlifs);

	/* Use only one notifyQ. */
	if (nnqs > 1) {
		IONIC_DEV_WARN(ionic->dev, "too many notifyQs(%d)\n", nnqs);
		nnqs = 1;
	}

	/* Limit the number of queues as specified by user. */
	if (ionic_max_queues && (nqs > ionic_max_queues))
		nqs = ionic_max_queues;

	/* Don't create number of queues more than number of cores. */
	nqs = min(nqs, mp_ncpus);

try_again:
#ifdef RSS
	/*
	 * Max number of Qs can't be more than number of RSS buckets.
	 */
	if (nqs > rss_getnumbuckets()) {
		nqs = rss_getnumbuckets();
		IONIC_DEV_INFO(ionic->dev,
		    "reducing number of Qs to %u based on RSS\n", nqs);
	}
#endif

	/* Interrupt is shared by transmit and receive. */
	nintrs = ionic->nlifs * (nnqs + neqs + nqs + 1 /* adminq */);
	if (nintrs > dev_nintrs) {
		goto try_fewer;
	}

	if (ionic_enable_msix) {
		err = ionic_alloc_msix_vectors(ionic, nintrs);
		if (err < 0 && err != -ENOSPC) {
			return (err);
		}

		if (err == -ENOSPC)
			goto try_fewer;

		if (err != nintrs) {
			ionic_free_msix_vector(ionic);
			goto try_fewer;
		}
	}

	ionic->neqs_per_lif = neqs;
	ionic->nnqs_per_lif = nnqs;
	ionic->ntxqs_per_lif = nqs;
	ionic->nrxqs_per_lif = nqs;
	ionic->nintrs = nintrs;

	dev_info(ionic->dev, "Lifs: %u, Intrs: %u/%u, NotifyQs: %u/%u, "
		"TxQs: %u/%u, RxQs: %u/%u, EQs: %u/%u\n",
		ionic->nlifs, nintrs, dev_nintrs,
		nnqs, ident->lif.eth.config.queue_count[IONIC_QTYPE_NOTIFYQ],
		nqs, ident->lif.eth.config.queue_count[IONIC_QTYPE_TXQ],
		nqs, ident->lif.eth.config.queue_count[IONIC_QTYPE_RXQ],
		neqs, ident->lif.eth.config.queue_count[IONIC_QTYPE_EQ]);

	dev_info(ionic->dev, "ucasts: %u, mcasts: %u, intr_coal mult: %u, div: %u\n",
		ident->lif.eth.max_ucast_filters, ident->lif.eth.max_mcast_filters,
		ident->dev.intr_coal_mult, ident->dev.intr_coal_div);

	return (0);

try_fewer:
	if (neqs > 1) {
		neqs /= 2;
		goto try_again;
	}
	if (nqs > 1) {
		nqs /= 2;
		goto try_again;
	}

	return (ENOSPC);
}

int
ionic_txq_identify(struct ionic *ionic, uint8_t ver)
{
	struct ionic_dev *idev = &ionic->idev;
	struct identity *ident = &ionic->ident;
	int i, err;
	uint32_t nwords;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_q_identify(idev, IONIC_LIF_TYPE_CLASSIC, IONIC_QTYPE_TXQ,
	    ver);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	if (err) {
		IONIC_DEV_UNLOCK(ionic);
		return (err);
	}

	nwords = min(ARRAY_SIZE(ident->txq.words),
	    ARRAY_SIZE(idev->dev_cmd_regs->data));
	for (i = 0; i < nwords; i++)
		ident->txq.words[i] = ioread32(&idev->dev_cmd_regs->data[i]);
	IONIC_DEV_UNLOCK(ionic);

	return (0);
}
