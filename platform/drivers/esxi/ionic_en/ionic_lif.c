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

#include "ionic.h"
#include "ionic_lif.h"


static VMK_ReturnStatus
ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode);

static void ionic_lif_deferred_work(struct ionic_work *work)
{
        VMK_ReturnStatus status;

        ionic_info("ionic_lif_deferred_work() called");

	struct lif *lif = IONIC_CONTAINER_OF(work,
                                             struct lif,
                                             deferred.work);
	struct deferred *def = &lif->deferred;
	struct deferred_work *w = NULL;

	vmk_SpinlockLock(def->lock);
	if (!vmk_ListIsEmpty(&def->list)) {
		w = VMK_LIST_ENTRY(&def->list, struct deferred_work, list);
		vmk_ListRemove(&w->list);
	}
	vmk_SpinlockUnlock(def->lock);

	if (w) {
		switch (w->type) {
		case DW_TYPE_RX_MODE:
                        ionic_info("DW_TYPE_RX_MODE");
                        ionic_lif_rx_mode(lif, w->rx_mode);
			break;
		case DW_TYPE_RX_ADDR_ADD:
                        ionic_info("DW_TYPE_RX_ADDR_ADD");
                        ionic_lif_addr_add(lif, w->addr);
			break;
		case DW_TYPE_RX_ADDR_DEL:
                        ionic_info("DW_TYPE_RX_ADDR_DEL");
			ionic_lif_addr_del(lif, w->addr);
			break;
		};
		//kfree(w);
                ionic_heap_free(ionic_driver.heap_id, w);
                //schedule_work(&def->work);
                status = ionic_work_queue_submit(lif->def_work_queue,
                                                 work,
                                                 0);
                if (status != VMK_OK) {
                        ionic_err("ionic_work_queue_submit() failed, status: %s",
                                  vmk_StatusToString(status));
                }
	}
}

#if 0
static void ionic_lif_deferred_enqueue(struct deferred *def,
				       struct deferred_work *work)
{
	spin_lock_bh(&def->lock);
	list_add_tail(&work->list, &def->list);
	spin_unlock_bh(&def->lock);
	schedule_work(&def->work);
}
#endif

static
VMK_ReturnStatus
ionic_qcq_enable(struct qcq *qcq)
{
	VMK_ReturnStatus status;//, status1;
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_enable = {
			.opcode = CMD_OPCODE_Q_ENABLE,
			.qid = q->qid,
			.qtype = q->qtype,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	ionic_info("q_enable.qid %d\n", ctx.cmd.q_enable.qid);
	ionic_info("q_enable.qtype %d\n", ctx.cmd.q_enable.qtype);

        status  = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) { 
		ionic_err("ionic_adminq_post_wait() failed, status: %s",
			  vmk_StatusToString(status));
	        return status;
        }

//	napi_enable(&qcq->napi);
/*        status = vmk_NetPollActivate(qcq->netpoll);
        if (status != VMK_OK) {
                ionic_err("vmk_NetPollActivate() failed, status: %s",
                          vmk_StatusToString(status));
                goto netpoll_act_err;
        }
*/
        vmk_NetPollEnable(qcq->netpoll);
	ionic_intr_mask(&qcq->intr, VMK_FALSE);
        return status;
/*
netpoll_act_err:
        ctx.cmd.q_enable.opcode = CMD_OPCODE_Q_DISABLE;
        status1  = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status1 != VMK_OK) { 
		ionic_err("ionic_adminq_post_wait() failed, status: %s",
			  vmk_StatusToString(status1));
        }

	return status; */
}

static VMK_ReturnStatus
ionic_qcq_disable(struct qcq *qcq)
{
        VMK_ReturnStatus status;

	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.q_disable = {
			.opcode = CMD_OPCODE_Q_DISABLE,
			.qid = q->qid,
			.qtype = q->qtype,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	ionic_info("q_disable.qid %d\n", ctx.cmd.q_disable.qid);
	ionic_info("q_disable.qtype %d\n", ctx.cmd.q_disable.qtype);

	ionic_intr_mask(&qcq->intr, VMK_TRUE);
//	synchronize_irq(qcq->intr.vector);
//	napi_disable(&qcq->napi);
        vmk_IntrSync(qcq->intr.cookie);
        vmk_NetPollDisable(qcq->netpoll);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}

VMK_ReturnStatus
ionic_open(struct lif *lif)
{
        VMK_ReturnStatus status = VMK_FAILURE;
	unsigned int i;

	for (i = 0; i < lif->ntxqcqs; i++) {
		status = ionic_qcq_enable(lif->txqcqs[i]);
		if (status != VMK_OK) {
                        ionic_err("ionic_qcq_enable() failed, status: %s",
                                  vmk_StatusToString(status));
			goto txqcqs_err;
                }
	}

//	netif_tx_wake_all_queues(netdev);

	for (i = 0; i < lif->nrxqcqs; i++) {
		ionic_rx_fill(&lif->rxqcqs[i]->q);
                status = ionic_qcq_enable(lif->rxqcqs[i]);
		if (status != VMK_OK) {
                        ionic_err("ionic_qcq_enable() failed, status: %s",
                                  vmk_StatusToString(status));
			goto rxqcqs_err;
                };
        }

//	netif_carrier_on(netdev);

        return status;

rxqcqs_err:
        for (; i > 0; i--) {
                ionic_qcq_disable(lif->rxqcqs[i-1]);
        }

txqcqs_err:
        for (; i > 0; i--) {
                ionic_qcq_disable(lif->txqcqs[i-1]);
        }

	return status;
}

VMK_ReturnStatus
ionic_stop(struct lif *lif)
{
        VMK_ReturnStatus status, status1 = VMK_OK;
	unsigned int i;

//	netif_carrier_off(netdev);
//	netif_tx_disable(netdev);

	for (i = 0; i < lif->ntxqcqs; i++) {
		// TODO post NOP Tx desc and wait for its completion
		// TODO before disabling Tx queue
		status = ionic_qcq_disable(lif->txqcqs[i]);
		if (status != VMK_OK) {
                        ionic_err("ionic_qcq_disable() failed, status: %s",
                                  vmk_StatusToString(status));
                        /* In the failure case, we still keep disabling
                           the next qcq element and record the status */
                        status1 = status;
                }
	}


	for (i = 0; i < lif->nrxqcqs; i++) {
		status = ionic_qcq_disable(lif->rxqcqs[i]);
		if (status != VMK_OK) {
                        ionic_err("ionic_qcq_disable() failed, status: %s",
                                  vmk_StatusToString(status));
                        /* In the failure case, we still keep disabling
                           the next qcq element and record the status */
                        status1 = status;
                }
                //ionic_rx_flush(&lif->rxqcqs[i]->cq);
       }

	return status1;
}

static bool ionic_adminq_service(struct cq *cq, struct cq_info *cq_info,
				 void *cb_arg)
{
	struct admin_comp *comp = cq_info->cq_desc;

	if (comp->color != cq->done_color)
		return VMK_FALSE;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return VMK_TRUE;
}

static vmk_Bool
ionic_adminq_netpoll(vmk_AddrCookie priv,
                     vmk_uint32 budget)
{
        vmk_uint32 polled;
        vmk_Bool poll_again = VMK_TRUE;
        void *qcq = priv.ptr;

	polled = ionic_netpoll(budget, ionic_adminq_service, qcq);

        if (polled != budget) {
                poll_again = VMK_FALSE;
        }

        return poll_again;
}


#if 0
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
#endif

VMK_ReturnStatus
ionic_lif_addr_add(struct lif *lif, const u8 *addr)
{
        VMK_ReturnStatus status;
        struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_add = {
			.opcode = CMD_OPCODE_RX_FILTER_ADD,
			.match = RX_FILTER_MATCH_MAC,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	vmk_Memcpy(ctx.cmd.rx_filter_add.mac.addr, addr, ETH_ALEN);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	ionic_info("rx_filter add ADDR %p (id %d)\n", addr,
	           ctx.comp.rx_filter_add.filter_id);

	return ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
}

VMK_ReturnStatus
ionic_lif_addr_del(struct lif *lif, const u8 *addr)
{
        VMK_ReturnStatus status;
	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_del = {
			.opcode = CMD_OPCODE_RX_FILTER_DEL,
		},
	};
	struct rx_filter *f;
//	int err;
	
        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

//	spin_lock_bh(&lif->rx_filters.lock);

        vmk_SpinlockLock(lif->rx_filters.lock);
	f = ionic_rx_filter_by_addr(lif, addr);
	if (!f) {
//		spin_unlock_bh(&lif->rx_filters.lock);
                vmk_SpinlockUnlock(lif->rx_filters.lock);
		return VMK_FAILURE;
	}

	ctx.cmd.rx_filter_del.filter_id = f->filter_id;
	ionic_rx_filter_free(f);
//	spin_unlock_bh(&lif->rx_filters.lock);
        vmk_SpinlockUnlock(lif->rx_filters.lock);

	status = ionic_adminq_post_wait(lif, &ctx);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        ionic_completion_destroy(&ctx.work);

	ionic_info("rx_filter del ADDR %pM (id %d)\n", addr,
	           ctx.cmd.rx_filter_del.filter_id);

	return status;
}

static VMK_ReturnStatus
ionic_lif_addr(struct lif *lif, const u8 *addr, bool add, bool is_intr)
{
//	struct deferred_work *work;

	if (is_intr) {
/*		work = kzalloc(sizeof(*work), GFP_ATOMIC);
		if (!work) {
			netdev_err(lif->netdev, "%s OOM\n", __func__);
			return -ENOMEM;
		}
		work->type = add ? DW_TYPE_RX_ADDR_ADD : DW_TYPE_RX_ADDR_DEL;
		memcpy(work->addr, addr, ETH_ALEN);
		ionic_info(lif->netdev, "deferred: rx_filter %s %pM\n",
			   add ? "add" : "del", addr);
		ionic_lif_deferred_enqueue(&lif->deferred, work);
*/
	} else {
		if (add)
			return ionic_lif_addr_add(lif, addr);
		else
			return ionic_lif_addr_del(lif, addr);
	}

	return VMK_OK;
}

#if 0
static int ionic_addr_add(struct net_device *netdev, const u8 *addr)
{
	return ionic_lif_addr(netdev_priv(netdev), addr, true);
}

static int ionic_addr_del(struct net_device *netdev, const u8 *addr)
{
	return ionic_lif_addr(netdev_priv(netdev), addr, false);
}
#endif

static VMK_ReturnStatus
ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode)
{
        VMK_ReturnStatus status;

        ionic_info("ionic_lif_rx_mode() called");

	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_mode_set = {
			.opcode = CMD_OPCODE_RX_MODE_SET,
			.rx_mode = rx_mode,
		},
	};
//	int err;

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	if (rx_mode & RX_MODE_F_UNICAST)
		ionic_info("rx_mode RX_MODE_F_UNICAST\n");
	if (rx_mode & RX_MODE_F_MULTICAST)
		ionic_info("rx_mode RX_MODE_F_MULTICAST\n");
	if (rx_mode & RX_MODE_F_BROADCAST)
		ionic_info("rx_mode RX_MODE_F_BROADCAST\n");
	if (rx_mode & RX_MODE_F_PROMISC)
		ionic_info("rx_mode RX_MODE_F_PROMISC\n");
	if (rx_mode & RX_MODE_F_ALLMULTI)
		ionic_info("rx_mode RX_MODE_F_ALLMULTI\n");

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
	}

        return status;
}

static VMK_ReturnStatus
_ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode, bool is_intr)
{
//	struct deferred_work *work;

	if (is_intr) {
/*		work = kzalloc(sizeof(*work), GFP_ATOMIC);
		if (!work) {
			netdev_err(lif->netdev, "%s OOM\n", __func__);
			return;
		}
		work->type = DW_TYPE_RX_MODE;
		work->rx_mode = rx_mode;
		ionic_info(lif->netdev, "deferred: rx_mode\n");
		ionic_lif_deferred_enqueue(&lif->deferred, work); */
	} else {
		return ionic_lif_rx_mode(lif, rx_mode);
	}
        return VMK_OK;
}

VMK_ReturnStatus
ionic_set_rx_mode(struct lif *lif,
                  vmk_UplinkState new_state)
{
//	union identity *ident = lif->ionic->ident;
	unsigned int rx_mode;

	rx_mode = RX_MODE_F_UNICAST;

        if (new_state & VMK_UPLINK_STATE_DISABLED) {
                goto out;
        }

        VMK_ASSERT(new_state & VMK_UPLINK_STATE_ENABLED);

        rx_mode |= (new_state & VMK_UPLINK_STATE_MULTICAST_OK) ?
                   RX_MODE_F_MULTICAST : 0;
	rx_mode |= (new_state & VMK_UPLINK_STATE_BROADCAST_OK ) ?
                   RX_MODE_F_BROADCAST : 0;
	rx_mode |= (new_state & VMK_UPLINK_STATE_PROMISC) ?
                   RX_MODE_F_PROMISC : 0;
//	rx_mode |= (netdev->flags & IFF_ALLMULTI) ? RX_MODE_F_ALLMULTI : 0;
/*
	if (netdev_uc_count(netdev) + 1 > ident->dev.nucasts_per_lif)
		rx_mode |= RX_MODE_F_PROMISC;
	if (netdev_mc_count(netdev) > ident->dev.nmcasts_per_lif)
		rx_mode |= RX_MODE_F_ALLMULTI;
*/

out:
	if (lif->rx_mode != rx_mode) {
		lif->rx_mode = rx_mode;
		return _ionic_lif_rx_mode(lif, rx_mode, VMK_FALSE);
	}

        return VMK_OK;
//	__dev_uc_sync(netdev, ionic_addr_add, ionic_addr_del);
//	__dev_mc_sync(netdev, ionic_addr_add, ionic_addr_del);
}
#if 0
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
	struct lif *lif = netdev_priv(netdev);
	struct ionic_dev *idev = &lif->ionic->idev;

	ionic_dev_cmd_hang_notify(idev);
	ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);

	// TODO implement reset and re-init queues and so on
	// TODO to get interface back on its feet
}
#endif

VMK_ReturnStatus
ionic_vlan_rx_add_vid(struct lif *lif,
                      u16 vid)
{
        VMK_ReturnStatus status;

	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_add = {
			.opcode = CMD_OPCODE_RX_FILTER_ADD,
			.match = RX_FILTER_MATCH_VLAN,
			.vlan.vlan = vid,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	ionic_info("rx_filter add VLAN %d (id %d)\n", vid,
		    ctx.comp.rx_filter_add.filter_id);

	return ionic_rx_filter_save(lif, 0, RXQ_INDEX_ANY, 0, &ctx);
}


VMK_ReturnStatus
ionic_vlan_rx_kill_vid(struct lif *lif,
                       u16 vid)
{
        VMK_ReturnStatus status;
	struct rx_filter *f;

        struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rx_filter_del = {
			.opcode = CMD_OPCODE_RX_FILTER_DEL,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	vmk_SpinlockLock(lif->rx_filters.lock);

	f = ionic_rx_filter_by_vlan(lif, vid);
	if (!f) {
                vmk_SpinlockUnlock(lif->rx_filters.lock);
		return VMK_FAILURE;
	}

	ctx.cmd.rx_filter_del.filter_id = f->filter_id;
	ionic_rx_filter_free(f);
	vmk_SpinlockUnlock(lif->rx_filters.lock);

        status  = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	ionic_info("rx_filter del VLAN %d (id %d)\n", vid,
		   ctx.cmd.rx_filter_del.filter_id);

	return status;
}


#if 0
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
#endif

VMK_ReturnStatus
ionic_intr_alloc(struct lif *lif, struct intr *intr)
{
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->en_dev.idev;
	vmk_uint32 index;
        vmk_Bool is_found;

        is_found = vmk_BitVectorNextBit(ionic->intrs.bit_vector,
                                        0, /* start from the beginning */
                                        VMK_FALSE,
                                        &index);
        if (!is_found) {
                ionic_err("vmk_BitVectorNextBit() is not able "
                          " to find a free bit");
                return VMK_NOT_FOUND;
        }

//	index = find_first_zero_bit(ionic->intrs, ionic->nintrs);
//	if (index == ionic->nintrs)
//		return -ENOSPC;
//	set_bit(index, ionic->intrs);
        vmk_BitVectorSet(ionic->intrs.bit_vector,
                         index);

	ionic_intr_init(idev, intr, index);

        return VMK_OK;
}

void ionic_intr_free(struct lif *lif, struct intr *intr)
{
	if (intr->index != INTR_INDEX_NOT_ASSIGNED)
                vmk_BitVectorClear(lif->ionic->intrs.bit_vector,
                                   intr->index);
//		clear_bit(intr->index, lif->ionic->intrs);
}

static void
ionic_msix_handle(void *handler_data,                       // IN
                  vmk_IntrCookie intr_cookie)               // IN
{
        struct qcq *qcq = (struct qcq *) handler_data;

        if (VMK_LIKELY(qcq && qcq->netpoll)) {
                vmk_NetPollActivate(qcq->netpoll);
        }
}

static VMK_ReturnStatus 
ionic_qcq_alloc(struct lif *lif,
                unsigned int index,
		const char *base,
                unsigned int flags,
		unsigned int num_descs,
                unsigned int desc_size,
		unsigned int cq_desc_size,
		unsigned int sg_desc_size,
		unsigned int pid,
                struct qcq **qcq)
{
        VMK_ReturnStatus status = VMK_NO_MEMORY;
        struct ionic_en_priv_data *priv_data;
        struct ionic_dev *idev = &lif->ionic->en_dev.idev;
//	struct device *dev = lif->ionic->dev;
	struct qcq *new;
	unsigned int q_size = num_descs * desc_size;
	unsigned int cq_size = num_descs * cq_desc_size;
	unsigned int sg_size = num_descs * sg_desc_size;
	unsigned int total_size;
	void *q_base, *cq_base, *sg_base;
	dma_addr_t q_base_pa, cq_base_pa, sg_base_pa;

	*qcq = NULL;

        ionic_info("ionic_qcq_alloc() called");

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

	total_size = IONIC_ALIGN(q_size, VMK_PAGE_SIZE) +
                     IONIC_ALIGN(cq_size, VMK_PAGE_SIZE);
	if (flags & QCQ_F_SG)
		total_size += IONIC_ALIGN(sg_size, VMK_PAGE_SIZE);

	//new = devm_kzalloc(dev, sizeof(*new), GFP_KERNEL);
        new = ionic_heap_zalloc(ionic_driver.heap_id,
                                sizeof(*new));
        if (!new) {
                ionic_err("ionic_heap_zalloc() failed, status: "
                          "VMK_NO_MEMORY");
		return status;
        }

        new->ring_idx = index;
	new->flags = flags;

//	new->q.info = devm_kzalloc(dev, sizeof(*new->q.info) * num_descs,
//				   GFP_KERNEL);
        new->q.info = ionic_heap_zalloc(ionic_driver.heap_id,
                                        sizeof(*new->q.info) * num_descs);
        if (!new->q.info) {
                ionic_err("ionic_heap_zalloc() failed, status: "
                          "VMK_NO_MEMORY");
                goto q_info_alloc_err;
        }

	status = ionic_q_init(lif, idev, &new->q, index, base, num_descs,
		              desc_size, sg_desc_size, pid);
	if (status != VMK_OK) {
                ionic_err("ionic_q_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto q_init_err;
        }

	if (flags & QCQ_F_INTR) {
		status = ionic_intr_alloc(lif, &new->intr);
		if (status != VMK_OK) {
                        ionic_err("ionic_intr_alloc() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto q_init_err;
                }

                new->intr.cookie = priv_data->intr_cookie_array[new->intr.index];
//                err = ionic_bus_get_irq(lif->ionic, new->intr.index);
                status = ionic_int_register(ionic_driver.module_id,
                                            priv_data->ionic.en_dev.vmk_device,
                                            new->intr.cookie,
                                            "msix_handle",
                                            ionic_msix_handle,
                                            new);
                if (status != VMK_OK) {
                        ionic_err("ionic_int_register() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto int_reg_err;
                }

//                if (err < 0)
//			goto err_out_free_intr;
//		new->intr.vector = err;
		ionic_intr_mask_on_assertion(&new->intr);
	} else {
		new->intr.index = INTR_INDEX_NOT_ASSIGNED;
	}

//	new->cq.info = devm_kzalloc(dev, sizeof(*new->cq.info) * num_descs,
//				    GFP_KERNEL);
        new->cq.info = ionic_heap_zalloc(ionic_driver.heap_id,
                                         sizeof(*new->cq.info) * num_descs);
        if (!new->cq.info) {
                ionic_info("ionic_heap_zalloc() failed, status: VMK_NO_MEMORY");
                status = VMK_NO_MEMORY;
                goto cq_info_err;
        }

	status = ionic_cq_init(lif, &new->cq, &new->intr,
	                       num_descs, cq_desc_size);
	if (status != VMK_OK) {
                ionic_err("ionic_cq_init() failed, status: %s",
                          vmk_StatusToString(status));
		goto cq_init_err;
        }

//	new->base = dma_alloc_coherent(dev, total_size, &new->base_pa,
//				       GFP_KERNEL);
        new->base = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                           priv_data->dma_engine_coherent,
                                           total_size,
                                           VMK_PAGE_SIZE,
                                           &new->base_pa);
        if (!new->base) {
                ionic_err("ionic_dma_zalloc_align() failed, "
                          "status: VMK_NO_MEMORY");
                status = VMK_NO_MEMORY;
                goto cq_init_err;
	}

	new->total_size = total_size;

	q_base = new->base;
	q_base_pa = new->base_pa;

	cq_base = (void *)IONIC_ALIGN((vmk_uintptr_t)q_base + q_size, VMK_PAGE_SIZE);
	cq_base_pa = IONIC_ALIGN(q_base_pa + q_size, VMK_PAGE_SIZE);

	if (flags & QCQ_F_SG) {
		sg_base = (void *)IONIC_ALIGN((vmk_uintptr_t)cq_base + cq_size,
	             			       VMK_PAGE_SIZE);
		sg_base_pa = IONIC_ALIGN(cq_base_pa + cq_size, VMK_PAGE_SIZE);
		ionic_q_sg_map(&new->q, sg_base, sg_base_pa);
	}

	ionic_q_map(&new->q, q_base, q_base_pa);
	ionic_cq_map(&new->cq, cq_base, cq_base_pa);
	ionic_cq_bind(&new->cq, &new->q);

	*qcq = new;

	return status;

cq_init_err:
        ionic_heap_free(ionic_driver.heap_id, new->cq.info);

if (flags & QCQ_F_INTR) {
cq_info_err:
        ionic_int_unregister(ionic_driver.module_id,
                             new->intr.cookie,
                             new);

int_reg_err:
        ionic_intr_free(lif, &new->intr);
}

q_init_err:
        ionic_heap_free(ionic_driver.heap_id, new->q.info);

q_info_alloc_err:
        ionic_heap_free(ionic_driver.heap_id, new);

	return status;
}

static void ionic_qcq_free(struct lif *lif, struct qcq *qcq)
{
        struct ionic_en_priv_data *priv_data;

        if (!qcq)
		return;

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

//	dma_free_coherent(lif->ionic->dev, qcq->total_size, qcq->base,
//			  qcq->base_pa);
        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_coherent,
                       qcq->total_size,
                       qcq->base,
                       qcq->base_pa);

        ionic_heap_free(ionic_driver.heap_id, qcq->cq.info);

        if (qcq->flags & QCQ_F_INTR) {
                ionic_int_unregister(ionic_driver.module_id,
                                     qcq->intr.cookie,
                                     qcq);
                ionic_intr_free(lif, &qcq->intr);
        }

        ionic_heap_free(ionic_driver.heap_id, qcq->q.info);

        ionic_heap_free(ionic_driver.heap_id, qcq);
}

static VMK_ReturnStatus
ionic_qcqs_alloc(struct lif *lif)
{
        VMK_ReturnStatus status;
	unsigned int flags;
	unsigned int i, j;

        ionic_info("ionic_qcqs_alloc() called");

//	lif->txqcqs = devm_kzalloc(dev, sizeof(*lif->txqcqs) * lif->ntxqcqs,
//				   GFP_KERNEL);
        lif->txqcqs = ionic_heap_zalloc(ionic_driver.heap_id,
                                        sizeof(*lif->txqcqs) * lif->ntxqcqs);
        if (!lif->txqcqs) {
                ionic_err("ionic_heap_zalloc() failed, status: "
                          "VMK_NO_MEMORY");
		return VMK_NO_MEMORY;
        }

//	lif->rxqcqs = devm_kzalloc(dev, sizeof(*lif->rxqcqs) * lif->nrxqcqs,
//				   GFP_KERNEL);
        lif->rxqcqs = ionic_heap_zalloc(ionic_driver.heap_id,
                                        sizeof(*lif->rxqcqs) * lif->nrxqcqs);
        if (!lif->rxqcqs) {
                ionic_err("ionic_heap_zalloc() failed, status: "
                          "VMK_NO_MEMORY");
		status = VMK_NO_MEMORY;
                goto rxqcqs_err;
        }

	flags = QCQ_F_INTR;
	status = ionic_qcq_alloc(lif, 0, "admin", flags, 1 << 4,
	                         sizeof(struct admin_cmd),
			         sizeof(struct admin_comp),
			         0, lif->kern_pid, &lif->adminqcq);
	if (status != VMK_OK) {
                ionic_err("ionic_qcq_alloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto adminqcq_err;
        }

	flags = QCQ_F_TX_STATS | QCQ_F_INTR | QCQ_F_SG;
	for (i = 0; i < lif->ntxqcqs; i++) {
		status = ionic_qcq_alloc(lif, i, "tx", flags, ntxq_descs,
				         sizeof(struct txq_desc),
				         sizeof(struct txq_comp),
				         sizeof(struct txq_sg_desc),
				         lif->kern_pid, &lif->txqcqs[i]);
	        if (status != VMK_OK) {
                        ionic_err("ionic_qcq_alloc() failed, status: %s",
                                  vmk_StatusToString(status));
			goto tx_qcq_alloc_err;
                }
	}

	flags = QCQ_F_RX_STATS | QCQ_F_INTR;
	for (j = 0; j < lif->nrxqcqs; j++) {
		status = ionic_qcq_alloc(lif, j, "rx", flags, nrxq_descs,
		                         sizeof(struct rxq_desc),
				         sizeof(struct rxq_comp),
				         0, lif->kern_pid, &lif->rxqcqs[j]);
	        if (status != VMK_OK) {
                        ionic_err("ionic_qcq_alloc() failed, status: %s",
                                  vmk_StatusToString(status));
			goto rx_qcq_alloc_err;
                }
        }

	return status;

rx_qcq_alloc_err:
        for (; j > 0; j--) {
                ionic_qcq_free(lif, lif->rxqcqs[j-1]);
        }

tx_qcq_alloc_err:
        for (; i > 0; i--) {
                ionic_qcq_free(lif, lif->txqcqs[i-1]);
        }
        ionic_qcq_free(lif, lif->adminqcq);

adminqcq_err:
        ionic_heap_free(ionic_driver.heap_id, lif->rxqcqs);

rxqcqs_err:
        ionic_heap_free(ionic_driver.heap_id, lif->txqcqs);

        return status;
}

static void ionic_qcqs_free(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < lif->nrxqcqs; i++) {
                //TODO: WE NEED TO FREE ESXi packet list in ionic_rx_empty()
                ionic_rx_empty(&lif->rxqcqs[i]->q);
		ionic_qcq_free(lif, lif->rxqcqs[i]);
	}
	for (i = 0; i < lif->ntxqcqs; i++)
		ionic_qcq_free(lif, lif->txqcqs[i]);
	ionic_qcq_free(lif, lif->adminqcq);

        ionic_heap_free(ionic_driver.heap_id, lif->rxqcqs);
        ionic_heap_free(ionic_driver.heap_id, lif->txqcqs);
}

static VMK_ReturnStatus
ionic_lif_alloc(struct ionic *ionic,
                unsigned int index,
                struct ionic_en_uplink_handle *uplink_handle)
{
        VMK_ReturnStatus status;
//	struct device *dev = ionic->dev;
//	struct net_device *netdev;
	struct lif *lif;
//	int err;
/*
	netdev = alloc_etherdev_mqs(sizeof(*lif),
				    ionic->ntxqs_per_lif,
				    ionic->nrxqs_per_lif);
	if (!netdev) {
		dev_err(dev, "Cannot allocate netdev, aborting\n");
		return -ENOMEM;
	}

	SET_NETDEV_DEV(netdev, dev);

	lif = netdev_priv(netdev);
*/
        lif = ionic_heap_zalign(ionic_driver.heap_id,
                                sizeof(struct lif),
                                VMK_L1_CACHELINE_SIZE);
        if (VMK_UNLIKELY(!lif)) {
                ionic_err("ionic_heap_zalign() failed, status: VMK_NO_MEMORY");
                return VMK_NO_MEMORY;
        } 

        lif->kern_pid = 0;
	lif->uplink_handle = uplink_handle;
	lif->ionic = ionic;
	lif->index = index;
	lif->neqs = ionic->neqs_per_lif;
	lif->ntxqcqs = ionic->ntxqs_per_lif;
	lif->nrxqcqs = ionic->nrxqs_per_lif;

	vmk_Snprintf(lif->name, sizeof(lif->name), "lif%u", index);

        status = ionic_spinlock_create("lif->adminq_lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK,
                                       IONIC_LOCK_RANK_NORMAL,
                                       &lif->adminq_lock);
        if (status != VMK_OK) {
                ionic_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto adminq_lock_err;
        }

        status = ionic_spinlock_create("lif->deferred.lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK,
                                       IONIC_LOCK_RANK_NORMAL,
                                       &lif->deferred.lock);
        if (status != VMK_OK) {
                ionic_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto deferred_lock_err;
        }                

        lif->def_work_queue = ionic_work_queue_create(ionic_driver.heap_id,
                                                      ionic_driver.module_id,
                                                      "lif->def_work_queue");
        if (lif->def_work_queue == NULL) {
                ionic_err("ionic_work_queue_create() failed, status: %s",
                          vmk_StatusToString(status));
                status = VMK_NO_MEMORY;
                goto def_wq_create_err;
        }

        vmk_ListInit(&lif->deferred.list);
        ionic_work_init(&lif->deferred.work,
                        (vmk_HelperRequestFunc) ionic_lif_deferred_work,
                        &lif->deferred.work);
        
//	spin_lock_init(&lif->adminq_lock);
//	spin_lock_init(&lif->deferred.lock);
//	INIT_LIST_HEAD(&lif->deferred.list);
//	INIT_WORK(&lif->deferred.work, ionic_lif_deferred_work);


	//netdev->netdev_ops = &ionic_netdev_ops;
	//ionic_ethtool_set_ops(netdev);
	//netdev->watchdog_timeo = 2 * HZ;

	//netdev->min_mtu = IONIC_MIN_MTU;
	//netdev->max_mtu = IONIC_MAX_MTU;

	status = ionic_qcqs_alloc(lif);
	if (status != VMK_OK) {
                ionic_err("ionic_qcqs_alloc() failed, status: %s",
                          vmk_StatusToString(status));
		goto qcq_alloc_err;
        }

        vmk_SpinlockLock(ionic->lifs_lock);
	vmk_ListInsert(&lif->list, &ionic->lifs);
        vmk_SpinlockUnlock(ionic->lifs_lock);

	return status;

qcq_alloc_err:
        ionic_work_queue_destroy(lif->def_work_queue);

def_wq_create_err:
        ionic_spinlock_destroy(lif->deferred.lock); 

deferred_lock_err:
        ionic_spinlock_destroy(lif->adminq_lock);

adminq_lock_err:
        ionic_heap_free(ionic_driver.heap_id,
                        lif);

//err_out_free_netdev:
//	free_netdev(netdev);

	return status;
}

static void
ionic_lif_free(struct lif *lif)
{
        ionic_work_queue_flush(lif->def_work_queue);
        ionic_qcqs_free(lif);
        ionic_work_queue_destroy(lif->def_work_queue);
        ionic_spinlock_destroy(lif->deferred.lock); 
        ionic_spinlock_destroy(lif->adminq_lock);
        ionic_heap_free(ionic_driver.heap_id,
                        lif);
}

VMK_ReturnStatus
ionic_lifs_alloc(struct ionic *ionic)
{
        VMK_ReturnStatus status = VMK_FAILURE;
        struct ionic_en_priv_data *priv_data;
        struct lif *lif = NULL;
        vmk_ListLinks *link, *next;
        unsigned int i;

//	INIT_LIST_HEAD(&ionic->lifs);
        vmk_ListInit(&ionic->lifs);

        priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

	for (i = 0; i < ionic->ident->dev.nlifs; i++) {
		status = ionic_lif_alloc(ionic, i, &priv_data->uplink_handle);
		if (status != VMK_OK) {
                        ionic_err("ionic_lif_alloc() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto lif_alloc_err;
                }
	}

	return status;

lif_alloc_err:
        vmk_SpinlockLock(ionic->lifs_lock);
        VMK_LIST_FORALL_SAFE(&ionic->lifs, link, next) {
                lif = VMK_LIST_ENTRY(link, struct lif, list);
                vmk_ListRemove(&lif->list);
                vmk_SpinlockUnlock(ionic->lifs_lock);
                ionic_lif_free(lif);
                vmk_SpinlockLock(ionic->lifs_lock);
        }
        vmk_SpinlockUnlock(ionic->lifs_lock);

        return status;
}


void ionic_lifs_free(struct ionic *ionic)
{
//	struct list_head *cur, *tmp;
	struct lif *lif;
        vmk_ListLinks *link, *next;
 
        vmk_SpinlockLock(ionic->lifs_lock);
        VMK_LIST_FORALL_SAFE(&ionic->lifs, link, next) {
                lif = VMK_LIST_ENTRY(link, struct lif, list);
                vmk_ListRemove(&lif->list);
                vmk_SpinlockUnlock(ionic->lifs_lock);
                ionic_lif_free(lif);
                vmk_SpinlockLock(ionic->lifs_lock);
        }
        vmk_SpinlockUnlock(ionic->lifs_lock);

/*
	list_for_each_safe(cur, tmp, &ionic->lifs) {
		lif = list_entry(cur, struct lif, list);
		list_del(&lif->list);
		flush_scheduled_work();
		ionic_qcqs_free(lif);
		free_netdev(lif->netdev);
	}
*/
}

#if 0
static VMK_ReturnStatus
ionic_lif_stats_dump_start(struct lif *lif, unsigned int ver)
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_priv_data *priv_data;

	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.stats_dump = {
			.opcode = CMD_OPCODE_STATS_DUMP_START,
			.ver = ver,
		},
	};

     	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

//	lif->stats_dump = dma_alloc_coherent(dev, sizeof(*lif->stats_dump),
//					     &lif->stats_dump_pa, GFP_KERNEL);

        lif->stats_dump = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                                 priv_data->dma_engine_coherent,
                                                 sizeof(*lif->stats_dump),
                                                 0,
                                                 &lif->stats_dump_pa);;
	if (!lif->stats_dump) {
	        ionic_err("ionic_dma_zalloc_align() failed, "
                          "status: VMK_NO_MEMORY");
                ionic_completion_destroy(&ctx.work);
                return  VMK_NO_MEMORY;
	}

	ctx.cmd.stats_dump.addr = lif->stats_dump_pa;

	ionic_info("stats_dump START ver %d addr 0x%lx\n", ver,
		   lif->stats_dump_pa);

        // TODO: need to complete
	return VMK_OK;

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
                goto adminq_post_err;
        }

	return status;

adminq_post_err:
        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_coherent,
                       sizeof(*lif->stats_dump),
                       lif->stats_dump,
                       lif->stats_dump_pa);

        return status;
}

static void ionic_lif_stats_dump_stop(struct lif *lif)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.stats_dump = {
			.opcode = CMD_OPCODE_STATS_DUMP_STOP,
		},
	};

	ionic_info("stats_dump STOP\n");
     	
        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_coherent,
                       sizeof(*lif->stats_dump),
                       lif->stats_dump,
                       lif->stats_dump_pa);
}
#endif

static VMK_ReturnStatus
ionic_rss_ind_tbl_set(struct lif *lif, const u32 *indir)
{
        VMK_ReturnStatus status;
        unsigned int i;

        struct ionic_admin_ctx ctx = { 
//                .work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
                .cmd.rss_indir_set = { 
                        .opcode = CMD_OPCODE_RSS_INDIR_SET,
                        .addr = lif->rss_ind_tbl_pa,
                },  
#ifdef HAPS
                .side_data = lif->rss_ind_tbl,
                .side_data_len = RSS_IND_TBL_SIZE,
#endif
        };  

     	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        if (indir)
                for (i = 0; i < RSS_IND_TBL_SIZE; i++)
                        lif->rss_ind_tbl[i] = indir[i];

        ionic_info("rss_ind_tbl_set\n");

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
       	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}

static VMK_ReturnStatus
ionic_rss_hash_key_set(struct lif *lif, const u8 *key)
{
        VMK_ReturnStatus status;
        struct ionic_admin_ctx ctx = { 
//                .work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
                .cmd.rss_hash_set = { 
                        .opcode = CMD_OPCODE_RSS_HASH_SET,
                        .types = RSS_TYPE_IPV4
                               | RSS_TYPE_IPV4_TCP
                               | RSS_TYPE_IPV4_UDP
                               | RSS_TYPE_IPV6
                               | RSS_TYPE_IPV6_TCP
                               | RSS_TYPE_IPV6_UDP
                               | RSS_TYPE_IPV6_EX
                               | RSS_TYPE_IPV6_TCP_EX
                               | RSS_TYPE_IPV6_UDP_EX,
                },  
        };  

     	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        vmk_Memcpy(lif->rss_hash_key, key, RSS_HASH_KEY_SIZE);

        vmk_Memcpy(ctx.cmd.rss_hash_set.key, lif->rss_hash_key,
                   RSS_HASH_KEY_SIZE);

        ionic_info("rss_hash_key_set\n");

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}

VMK_ReturnStatus
ionic_lif_rss_setup(struct lif *lif)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
//	struct net_device *netdev = lif->netdev;
//	struct device *dev = lif->ionic->dev;
	unsigned int i;

	size_t tbl_size = sizeof(*lif->rss_ind_tbl) * RSS_IND_TBL_SIZE;
	static const u8 toeplitz_symmetric_key[] = {
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
	};

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);
//	lif->rss_ind_tbl = dma_alloc_coherent(dev, tbl_size,
//					      &lif->rss_ind_tbl_pa,
//					      GFP_KERNEL);
        lif->rss_ind_tbl = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                                  priv_data->dma_engine_coherent,
                                                  tbl_size,
                                                  VMK_PAGE_SIZE,
                                                  &lif->rss_ind_tbl_pa);

	if (!lif->rss_ind_tbl) {
	        ionic_err("ionic_dma_zalloc_align() failed, "
                          "status: VMK_NO_MEMORY");
                return  VMK_NO_MEMORY;
        }

	/* Fill indirection table with 'default' values */

	for (i = 0; i < RSS_IND_TBL_SIZE; i++)
		lif->rss_ind_tbl[i] = i % lif->nrxqcqs;

	status = ionic_rss_ind_tbl_set(lif, NULL);
	if (status != VMK_OK) {
                ionic_err("ionic_rss_ind_tbl_set() failed, status: %s",
                          vmk_StatusToString(status));
		goto rss_ind_tbl_set_err;
        }

	status = ionic_rss_hash_key_set(lif, toeplitz_symmetric_key);
	if (status != VMK_OK) {
                ionic_err("ionic_rss_hash_key_set() failed, status: %s",
                          vmk_StatusToString(status));
		goto rss_ind_tbl_set_err;
        }

	return status;

rss_ind_tbl_set_err:
        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_coherent,
                       tbl_size,
                       lif->rss_ind_tbl,
                       lif->rss_ind_tbl_pa);

	return status;
}

static void ionic_lif_rss_teardown(struct lif *lif)
{
        struct ionic_en_priv_data *priv_data;
	size_t tbl_size = sizeof(*lif->rss_ind_tbl) * RSS_IND_TBL_SIZE;

	if (!lif->rss_ind_tbl)
		return;

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_coherent,
                       tbl_size,
                       lif->rss_ind_tbl,
                       lif->rss_ind_tbl_pa);

	lif->rss_ind_tbl = NULL;
}

static void ionic_lif_qcq_deinit(struct qcq *qcq)
{
	if (!(qcq->flags & QCQ_F_INITED))
		return;
	ionic_intr_mask(&qcq->intr, VMK_TRUE);
//	devm_free_irq(dev, qcq->intr.vector, &qcq->napi);
        vmk_IntrSync(qcq->intr.cookie);
        vmk_NetPollInterruptUnSet(qcq->netpoll);
//	netif_napi_del(&qcq->napi);
        vmk_NetPollDestroy(qcq->netpoll); 
	qcq->flags &= ~QCQ_F_INITED;
}


static void ionic_lif_adminq_deinit(struct lif *lif)
{
        ionic_lif_qcq_deinit(lif->adminqcq);
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
//	ionic_lif_stats_dump_stop(lif);
	ionic_rx_filters_deinit(lif);
	ionic_lif_rss_teardown(lif);
	ionic_lif_qcq_deinit(lif->adminqcq);
	ionic_lif_txqs_deinit(lif);
	ionic_lif_rxqs_deinit(lif);
}

void ionic_lifs_deinit(struct ionic *ionic)
{
        vmk_ListLinks *link;
	struct lif *lif;

        VMK_LIST_FORALL(&ionic->lifs, link) {
                lif = VMK_LIST_ENTRY(link, struct lif, list);
                ionic_lif_deinit(lif);
        }
}

static VMK_ReturnStatus
ionic_request_irq(struct lif *lif, struct qcq *qcq)
{
        VMK_ReturnStatus status;
//	struct device *dev = lif->ionic->dev;
	struct intr *intr = &qcq->intr;
	struct queue *q = &qcq->q;
//	struct napi_struct *napi = &qcq->napi;

	vmk_Snprintf(intr->name, sizeof(intr->name),
	             "%s-%s-%s", DRV_NAME, lif->name, q->name);
//return devm_request_irq(dev, intr->vector, ionic_isr,
//				0, intr->name, napi);

        status = vmk_NetPollInterruptSet(qcq->netpoll,
                                         qcq->intr.cookie);
        if (status != VMK_OK) {
                ionic_err("vmk_NetPollInterruptSet() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}

static VMK_ReturnStatus
ionic_lif_adminq_init(struct lif *lif)
{
        VMK_ReturnStatus status;
        struct ionic_dev *idev = &lif->ionic->en_dev.idev;
	struct qcq *qcq = lif->adminqcq;
	struct queue *q = &qcq->q;
//	struct napi_struct *napi = &qcq->napi;
        struct adminq_init_comp comp;

	ionic_dev_cmd_adminq_init(idev, q, 0, lif->index, 0);
	status = ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
	if (status != VMK_OK) {
                ionic_err("ionic_dev_cmd_wait_check() failed, status: %s",
                          vmk_StatusToString(status));
	        return status;
        }

	ionic_dev_cmd_comp(idev, &comp);
	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = ionic_db_map(idev, q);

//	netif_napi_add(lif->netdev, napi, ionic_adminq_netpoll,
//		       NAPI_POLL_WEIGHT);
        status = ionic_en_netpoll_create(&qcq->netpoll,
                                         lif->uplink_handle,
                                         IONIC_EN_ADMIN_RING,
                                         ionic_adminq_netpoll,
                                         qcq);

        if (status != VMK_OK) {
                ionic_err("ionic_en_netpoll_create() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = ionic_request_irq(lif, qcq);
	if (status != VMK_OK) {
                ionic_err("ionic_request_irq() failed, status: %s",
                          vmk_StatusToString(status));
                vmk_NetPollDestroy(qcq->netpoll);
                return status;
	}

	qcq->flags |= QCQ_F_INITED;

	return status;
}

static VMK_ReturnStatus
ionic_get_features(struct lif *lif)
{
        VMK_ReturnStatus status;

	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
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

     	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	lif->hw_features = ctx.cmd.features.wanted &
			   ctx.comp.features.supported;

	if (lif->hw_features & ETH_HW_VLAN_TX_TAG)
		ionic_info("feature ETH_HW_VLAN_TX_TAG\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_STRIP)
		ionic_info("feature ETH_HW_VLAN_RX_STRIP\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_FILTER)
		ionic_info("feature ETH_HW_VLAN_RX_FILTER\n");
	if (lif->hw_features & ETH_HW_RX_HASH)
		ionic_info("feature ETH_HW_RX_HASH\n");
	if (lif->hw_features & ETH_HW_TX_SG)
		ionic_info("feature ETH_HW_TX_SG\n");
	if (lif->hw_features & ETH_HW_TX_CSUM)
		ionic_info("feature ETH_HW_TX_CSUM\n");
	if (lif->hw_features & ETH_HW_RX_CSUM)
		ionic_info("feature ETH_HW_RX_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO)
		ionic_info("feature ETH_HW_TSO\n");
	if (lif->hw_features & ETH_HW_TSO_IPV6)
		ionic_info("feature ETH_HW_TSO_IPV6\n");
	if (lif->hw_features & ETH_HW_TSO_ECN)
		ionic_info("feature ETH_HW_TSO_ECN\n");
	if (lif->hw_features & ETH_HW_TSO_GRE)
		ionic_info("feature ETH_HW_TSO_GRE\n");
	if (lif->hw_features & ETH_HW_TSO_GRE_CSUM)
		ionic_info("feature ETH_HW_TSO_GRE_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP4)
		ionic_info("feature ETH_HW_TSO_IPXIP4\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP6)
		ionic_info("feature ETH_HW_TSO_IPXIP6\n");
	if (lif->hw_features & ETH_HW_TSO_UDP)
		ionic_info("feature ETH_HW_TSO_UDP\n");
	if (lif->hw_features & ETH_HW_TSO_UDP_CSUM)
		ionic_info("feature ETH_HW_TSO_UDP_CSUM\n");

        lif->uplink_handle->hw_features = lif->hw_features;
	return status;
}


static VMK_ReturnStatus
ionic_lif_txq_init(struct lif *lif, struct qcq *qcq)
{
        VMK_ReturnStatus status;
        struct queue *q = &qcq->q;
	struct cq *cq = &qcq->cq;
	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.txq_init = {
			.opcode = CMD_OPCODE_TXQ_INIT,
			.I = VMK_FALSE,
			.E = VMK_FALSE,
			.pid = q->pid,
			.intr_index = cq->bound_intr->index,
			.type = TXQ_TYPE_ETHERNET,
			.index = q->index,
			.cos = 0,
			.ring_base = q->base_pa,
			.ring_size = ionic_ilog2(q->num_descs),
		},
	};

	ionic_info("txq_init.pid %d\n", ctx.cmd.txq_init.pid);
	ionic_info("txq_init.index %d\n", ctx.cmd.txq_init.index);
	ionic_info("txq_init.ring_base 0x%lx\n",
	           ctx.cmd.txq_init.ring_base);
	ionic_info("txq_init.ring_size %d\n",
		   ctx.cmd.txq_init.ring_size);

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	q->qid = ctx.comp.txq_init.qid;
	q->qtype = ctx.comp.txq_init.qtype;
	q->db = ionic_db_map(q->idev, q);

//	netif_napi_add(lif->netdev, napi, ionic_tx_napi,
//		       NAPI_POLL_WEIGHT);

        status = ionic_en_netpoll_create(&qcq->netpoll,
                                         lif->uplink_handle,
                                         IONIC_EN_TX_RING,
                                         ionic_tx_netpoll,
                                         qcq);
        if (status != VMK_OK) {
                ionic_err("ionic_en_netpoll_create() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	status = ionic_request_irq(lif, qcq);
	if (status != VMK_OK) {
                ionic_err("ionic_request_irq() failed, status: %s",
                          vmk_StatusToString(status));
                vmk_NetPollDestroy(qcq->netpoll);
                return status;
	}

	qcq->flags |= QCQ_F_INITED;

	ionic_info("txq->qid %d\n", q->qid);
	ionic_info("txq->qtype %d\n", q->qtype);
	ionic_info("txq->db %p\n", q->db);

	return status;
}

static VMK_ReturnStatus
ionic_lif_txqs_init(struct lif *lif)
{
        VMK_ReturnStatus status = VMK_FAILURE;
        unsigned int i;

	for (i = 0; i < lif->ntxqcqs; i++) {
		status = ionic_lif_txq_init(lif, lif->txqcqs[i]);
		if (status != VMK_OK) {
                        ionic_err("ionic_lif_txq_init() failed, "
                                  "status: %s",
                                  vmk_StatusToString(status));
			goto err_out;
	        }
        }

	return status;

err_out:
	for (; i > 0; i--)
		ionic_lif_qcq_deinit(lif->txqcqs[i-1]);

	return status;
}


static VMK_ReturnStatus
ionic_lif_rxq_init(struct lif *lif, struct qcq *qcq)
{
        VMK_ReturnStatus status;
        struct queue *q = &qcq->q;
	struct cq *cq = &qcq->cq;
	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rxq_init = {
			.opcode = CMD_OPCODE_RXQ_INIT,
			.I = VMK_FALSE,
			.E = VMK_FALSE,
			.pid = q->pid,
			.intr_index = cq->bound_intr->index,
			.type = RXQ_TYPE_ETHERNET,
			.index = q->index,
			.ring_base = q->base_pa,
			.ring_size = ionic_ilog2(q->num_descs),
		},
	};

	ionic_info("rxq_init.pid %d\n", ctx.cmd.rxq_init.pid);
	ionic_info("rxq_init.index %d\n", ctx.cmd.rxq_init.index);
        ionic_info("rxq_init.intr_index %d\n", ctx.cmd.rxq_init.intr_index);
        ionic_info("rxq_init.ring_base 0x%lx\n",
		   ctx.cmd.rxq_init.ring_base);
	ionic_info("rxq_init.ring_size %d\n",
		   ctx.cmd.rxq_init.ring_size);

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	q->qid = ctx.comp.rxq_init.qid;
	q->qtype = ctx.comp.rxq_init.qtype;
	q->db = ionic_db_map(q->idev, q);

//	netif_napi_add(lif->netdev, napi, ionic_rx_napi,
//		       NAPI_POLL_WEIGHT);

        status = ionic_en_netpoll_create(&qcq->netpoll,
                                         lif->uplink_handle,
                                         IONIC_EN_RX_RING,
                                         ionic_rx_netpoll,
                                         qcq);
        if (status != VMK_OK) {
                ionic_err("ionic_en_netpoll_create() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	status = ionic_request_irq(lif, qcq);
	if (status != VMK_OK) {
                ionic_err("ionic_request_irq() failed, status: %s",
                          vmk_StatusToString(status));
                vmk_NetPollDestroy(qcq->netpoll);
                return status;
	}

	qcq->flags |= QCQ_F_INITED;

	ionic_info("rxq->qid %d\n", q->qid);
	ionic_info("rxq->qtype %d\n", q->qtype);
	ionic_info("rxq->db %p\n", q->db);

	return status;
}

static VMK_ReturnStatus
ionic_lif_rxqs_init(struct lif *lif)
{
        VMK_ReturnStatus status = VMK_FAILURE;
        unsigned int i;

	for (i = 0; i < lif->nrxqcqs; i++) {
		status = ionic_lif_rxq_init(lif, lif->rxqcqs[i]);
		if (status != VMK_OK) {
                        ionic_err("ionic_lif_rxq_init() failed, "
                                  "status: %s",
                                  vmk_StatusToString(status));
			goto err_out;
                }
	}

	return status;

err_out:
	for (; i > 0; i--)
		ionic_lif_qcq_deinit(lif->rxqcqs[i-1]);

	return status;
}

VMK_ReturnStatus
ionic_station_set(struct lif *lif)
{
        VMK_ReturnStatus status;
        vmk_EthAddress zero_eth_addr;

        struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.station_mac_addr_get = {
			.opcode = CMD_OPCODE_STATION_MAC_ADDR_GET,
		},
	};

     	status = ionic_completion_create(ionic_driver.module_id,
                                         ionic_driver.heap_id,
                                         ionic_driver.lock_domain,
                                         "ionic_admin_ctx.work",
                                         &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

        vmk_Memset(&zero_eth_addr, 0, VMK_ETH_ADDR_LENGTH);
        if (vmk_Memcmp(lif->uplink_handle->vmk_mac_addr,
                       zero_eth_addr,
                       VMK_ETH_ADDR_LENGTH)) {
                ionic_info("deleting station MAC addr");
                ionic_lif_addr(lif,
                               lif->uplink_handle->vmk_mac_addr,
                               VMK_FALSE,
                               VMK_FALSE);  // Not in interrupt context
        }            

        vmk_Memcpy(lif->uplink_handle->vmk_mac_addr,
                   ctx.comp.station_mac_addr_get.addr,
                   VMK_ETH_ADDR_LENGTH);
        vmk_Memcpy(lif->uplink_handle->uplink_shared_data.macAddr,
                   lif->uplink_handle->vmk_mac_addr,
                   VMK_ETH_ADDR_LENGTH);
        vmk_Memcpy(lif->uplink_handle->uplink_shared_data.hwMacAddr,
                   lif->uplink_handle->vmk_mac_addr,
                   VMK_ETH_ADDR_LENGTH);
 
        ionic_info("adding station MAC addr");
        ionic_lif_addr(lif,
                       lif->uplink_handle->vmk_mac_addr,
                       VMK_TRUE,
                       VMK_FALSE);  // Not in interrupt context

	return status;
}

static VMK_ReturnStatus
ionic_lif_init(struct lif *lif)
{
        VMK_ReturnStatus status;
        vmk_UplinkState init_state;
        struct ionic_dev *idev = &lif->ionic->en_dev.idev;

	ionic_dev_cmd_lif_init(idev, lif->index);

	status = ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
	if (status != VMK_OK) {
                ionic_err("ionic_dev_cmd_wait_check() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	status = ionic_lif_adminq_init(lif);
	if (status != VMK_OK) {
                ionic_err("ionic_lif_adminq_init() failed, status: %s",
                          vmk_StatusToString(status));
                // TODO: we might need to add LIF_DINIT cmd_opcode to dinit the lif
                return status;
        }

	/* Enabling interrupts on adminq from here on... */
	ionic_intr_mask(&lif->adminqcq->intr, VMK_FALSE);

	status  = ionic_get_features(lif);
	if (status != VMK_OK) {
                ionic_err("ionic_get_features() failed, status: %s",
                          vmk_StatusToString(status));
		goto get_features_err;
        }

	status = ionic_lif_txqs_init(lif);
	if (status != VMK_OK) {
                ionic_err("ionic_lif_txqs_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto get_features_err;
        }


	status = ionic_lif_rxqs_init(lif);
	if (status != VMK_OK) {
                ionic_err("ionic_lif_rxqs_init() failed, status: %s",
                          vmk_StatusToString(status));
		goto rxqs_init_err;
        }

	status = ionic_rx_filters_init(lif);
	if (status != VMK_OK) {
                ionic_err("ionic_rx_filters_init() failed, status: %s",
                          vmk_StatusToString(status));
		goto rx_filters_err;
        }

        status = ionic_station_set(lif);
	if (status != VMK_OK) {
                ionic_err("ionic_station_set() failed, status: %s",
                          vmk_StatusToString(status));
		goto station_set_err;
        }
        
	if (lif->uplink_handle->hw_features & ETH_HW_RX_HASH) {
		status = ionic_lif_rss_setup(lif);
		if (status != VMK_OK) {
                        ionic_err("ionic_lif_rss_setup() failed, status: %s",
                                  vmk_StatusToString(status));
			goto rss_setup_err;
                }
	}

/*
	status = ionic_lif_stats_dump_start(lif, STATS_DUMP_VERSION_1);
	if (status != VMK_OK) {
                ionic_err("ionic_lif_stats_dump_start() failed, status: %s",
                          vmk_StatusToString(status));
		goto stats_dump_start_err;
        }
*/
        init_state = VMK_UPLINK_STATE_ENABLED |
                     VMK_UPLINK_STATE_BROADCAST_OK |
                     VMK_UPLINK_STATE_MULTICAST_OK |
                     VMK_UPLINK_STATE_PROMISC;

        ionic_set_rx_mode(lif, init_state);

        lif->uplink_handle->prev_state = init_state;
        IONIC_EN_SHARED_AREA_BEGIN_WRITE(lif->uplink_handle);
        lif->uplink_handle->uplink_shared_data.state = init_state;
        IONIC_EN_SHARED_AREA_END_WRITE(lif->uplink_handle);

	lif->api_private = NULL;

	return status;
/*
err_out_rss_teardown:
	ionic_lif_rss_teardown(lif);
err_out_rx_filter_deinit:
	ionic_rx_filters_deinit(lif);
err_out_rxqs_deinit:
	ionic_lif_rxqs_deinit(lif);
*/
//rxqs_init_err:
//        ionic_lif_txqs_deinit(lif);

#if 0
stats_dump_start_err:
        if (lif->uplink_handle->hw_features & ETH_HW_RX_HASH) {
                ionic_lif_rss_teardown(lif);
        }
#endif

rss_setup_err:
        ionic_lif_addr(lif,
                       lif->uplink_handle->vmk_mac_addr,
                       VMK_FALSE,
                       VMK_FALSE);  // Not in interrupt context

station_set_err:
        ionic_rx_filters_deinit(lif);

rx_filters_err:
        ionic_lif_rxqs_deinit(lif);

rxqs_init_err:
        ionic_lif_txqs_deinit(lif);

get_features_err:
	ionic_intr_mask(&lif->adminqcq->intr, VMK_TRUE);
        ionic_lif_adminq_deinit(lif);
	return status;
}

VMK_ReturnStatus
ionic_lifs_init(struct ionic *ionic)
{
//	struct list_head *cur;
        VMK_ReturnStatus status = VMK_FAILURE;
        vmk_ListLinks *link;
        struct lif *lif;

        VMK_LIST_FORALL(&ionic->lifs, link) {
                lif = VMK_LIST_ENTRY(link, struct lif, list);
                status = ionic_lif_init(lif);
                if (status != VMK_OK) {
                        ionic_err("ionic_lif_init() failed, status: %s",
                                  vmk_StatusToString(status));
                        break;
                }
        }

	return status;
}

#if 0
int ionic_lif_register(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
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

#endif

VMK_ReturnStatus
ionic_lifs_size(struct ionic *ionic)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
        vmk_uint32 granted = 0;

        union identity *ident = ionic->ident;
        unsigned int nlifs = ident->dev.nlifs;
        unsigned int neqs_per_lif = ident->dev.neqs_per_lif;
        unsigned int ntxqs_per_lif = ident->dev.ntxqs_per_lif;
        unsigned int nrxqs_per_lif = ident->dev.nrxqs_per_lif;
        unsigned int nintrs, dev_nintrs = ident->dev.nintrs;
//        int err;

        if (ntxqs > 0) {
                ntxqs_per_lif = IONIC_MIN(ntxqs_per_lif, ntxqs);
                VMK_ASSERT(ntxqs_per_lif <= IONIC_MAX_NUM_TX_QUEUE);
        }
        if (nrxqs > 0) {
                nrxqs_per_lif = IONIC_MIN(nrxqs_per_lif, nrxqs);
                VMK_ASSERT(nrxqs_per_lif <= IONIC_MAX_NUM_RX_QUEUE);
        }

try_again:
        nintrs = nlifs * (/*neqs_per_lif +*/
                          ntxqs_per_lif +
                          nrxqs_per_lif +
                          1 /* adminq */);

        if (nintrs > dev_nintrs)
                goto try_fewer;

        priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);
        priv_data->intr_cookie_array = ionic_heap_zalloc(ionic_driver.heap_id,
                                                         sizeof(vmk_IntrCookie) *
                                                         nintrs);
        if (VMK_UNLIKELY(!priv_data->intr_cookie_array)) {
                ionic_err("ionic_heap_zalloc() failed, status: NO MEMORY, size: %ld",
                          sizeof(vmk_IntrCookie) * nintrs);
                return VMK_NO_MEMORY;
        }
        
/*
        err = ionic_bus_alloc_irq_vectors(ionic, nintrs);
        if (err < 0 && err != -ENOSPC)
                return err;
        if (err == -ENOSPC)
                goto try_fewer;
        if (err != nintrs) {
                ionic_bus_free_irq_vectors(ionic);
                goto try_fewer;
        }
*/
        status = ionic_int_alloc(ionic_driver.module_id,
                                 ionic_driver.heap_id,
                                 ionic->en_dev.pci_device,
                                 VMK_PCI_INTERRUPT_TYPE_MSIX,
                                 nintrs,
                                 nintrs,
                                 priv_data->intr_cookie_array,
                                 &granted);
        if (status != VMK_OK) {
                ionic_err("ionic_int_alloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto intr_alloc_err;
        }
                                 
        ionic->neqs_per_lif = neqs_per_lif;
        ionic_info("ntxqs_per_lif:%d, nrxqs_per_lif: %d, nints: %d",
                  ntxqs_per_lif, nrxqs_per_lif, nintrs);
        
        ionic->ntxqs_per_lif = ntxqs_per_lif;
        ionic->nrxqs_per_lif = nrxqs_per_lif;
        ionic->nintrs = nintrs;

        ionic->intrs.bit_vector = vmk_BitVectorAlloc(ionic_driver.heap_id,
                                                     INTR_CTRL_REGS_MAX);
        if (!ionic->intrs.bit_vector) {
                ionic_err("vmk_BitVectorAlloc() failed, "
                          "status: VMK_NO_MEMORY");
                status = VMK_NO_MEMORY;
                goto intrs_bit_vector_err;
        }

        ionic_info("neqs_per_lif: %d, ntxqs_per_lif: %d, "
                   "nrxqs_per_lif: %d, nintrs: %d, nlifs: %d", neqs_per_lif,
                   ntxqs_per_lif, nrxqs_per_lif, nintrs, nlifs);

//        return ionic_debugfs_add_sizes(ionic);
        return status;

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
        return VMK_FAILURE;
//        return -ENOSPC;

intrs_bit_vector_err:
        ionic_int_free(ionic_driver.module_id,
                       ionic_driver.heap_id,
                       ionic->en_dev.pci_device,
                       priv_data->intr_cookie_array);

intr_alloc_err:
        ionic_heap_free(ionic_driver.heap_id,
                        priv_data->intr_cookie_array);

        return status;
}



void
ionic_lifs_size_undo(struct ionic_en_priv_data *priv_data)
{
        vmk_BitVectorFree(ionic_driver.heap_id,
                          priv_data->ionic.intrs.bit_vector);

        ionic_int_free(ionic_driver.module_id,
                       ionic_driver.heap_id,
                       priv_data->ionic.en_dev.pci_device,
                       priv_data->intr_cookie_array);
}
