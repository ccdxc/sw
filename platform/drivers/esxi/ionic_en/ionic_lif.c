/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 *
 * This software is provided to you under the terms of the GNU
 * General Public License (GPL) Version 2.0, available from the file
 * [ionic_lif.c] in the main directory of this source tree.
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

        ionic_dbg("ionic_lif_deferred_work() called");

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

VMK_ReturnStatus
ionic_qcq_enable(struct qcq *qcq)
{
	VMK_ReturnStatus status;
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

	ionic_dbg("q_enable.qid %d\n", ctx.cmd.q_enable.qid);
	ionic_dbg("q_enable.qtype %d\n", ctx.cmd.q_enable.qtype);

        status  = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) { 
		ionic_err("ionic_adminq_post_wait() failed, status: %s",
			  vmk_StatusToString(status));
	        return status;
        }

        if (!qcq->is_netpoll_enabled) {
                vmk_NetPollInterruptSet(qcq->netpoll,
                                        qcq->intr.cookie);
                vmk_NetPollEnable(qcq->netpoll);
                qcq->is_netpoll_enabled = VMK_TRUE;
                vmk_IntrEnable(qcq->intr.cookie);
                ionic_intr_mask(&qcq->intr, VMK_FALSE);
        }

        return status;
}

VMK_ReturnStatus
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

	ionic_dbg("q_disable.qid %d\n", ctx.cmd.q_disable.qid);
	ionic_dbg("q_disable.qtype %d\n", ctx.cmd.q_disable.qtype);

        ionic_intr_mask(&qcq->intr, VMK_TRUE);

//	synchronize_irq(qcq->intr.vector);
//	napi_disable(&qcq->napi);


        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }
        
        if (qcq->is_netpoll_enabled) {
                vmk_IntrDisable(qcq->intr.cookie);
                vmk_IntrSync(qcq->intr.cookie);
                vmk_NetPollDisable(qcq->netpoll);
                qcq->is_netpoll_enabled = VMK_FALSE;
                vmk_NetPollFlushRx(qcq->netpoll);
                vmk_NetPollInterruptUnSet(qcq->netpoll);
        }

        return status;
}

VMK_ReturnStatus
ionic_open(struct lif *lif)
{
        VMK_ReturnStatus status = VMK_FAILURE;
        struct ionic_en_priv_data *priv_data;
        struct ionic_en_uplink_handle *uplink_handle;
        vmk_uint32 i, max_rx_normal_queues;
        vmk_uint32 shared_q_data_idx, max_rx_rss_queues;

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);
        uplink_handle = lif->uplink_handle;

        max_rx_normal_queues = uplink_handle->max_rx_normal_queues;
        for (i = 0; i < lif->nrxqcqs; i++) {
		ionic_rx_fill(&lif->rxqcqs[i]->q);
                if (i < max_rx_normal_queues) {
                        ionic_dbg("INIT normal rxq, ring_idx: %d, "
                                  "shared_q_idx: %d",
                                   i, i);
                        status = ionic_en_rx_ring_init(i,
                                                       i,
                                                       priv_data,
                                                       lif);
        		if (status != VMK_OK) {
                                ionic_err("ionic_en_rx_ring_init() failed, "
                                          "status: %s",
                                          vmk_StatusToString(status));
                                i--;
                                goto rxqcqs_err;
                        }

                }
        }

        max_rx_rss_queues = uplink_handle->max_rx_rss_queues;

        if (!uplink_handle->is_mgmt_nic) {
                ionic_en_rx_rss_init(priv_data,
                                     lif);
        }
        
        for (i = 0; i < lif->ntxqcqs; i++) {
		status = ionic_qcq_enable(lif->txqcqs[i]);
		if (status != VMK_OK) {
                        ionic_err("ionic_qcq_enable() failed, status: %s",
                                  vmk_StatusToString(status));
			goto txqcqs_err;
                }

                shared_q_data_idx = max_rx_normal_queues +
                                    max_rx_rss_queues + i;

                ionic_dbg("INIT txq, ring_idx: %d, "
                          "shared_q_idx: %d",
                           i, shared_q_data_idx);
                ionic_en_tx_ring_init(i,
                                      shared_q_data_idx,
                                      priv_data,
                                      lif);
	}

        return status;

txqcqs_err:
        for (; i > 0; i--) {
                ionic_qcq_disable(lif->txqcqs[i-1]);
                ionic_en_tx_ring_deinit(i,
                                        priv_data);
        }

        if (!uplink_handle->is_mgmt_nic) {
                ionic_en_rx_rss_deinit(priv_data,
                                       lif);
        }

        i = lif->nrxqcqs;

rxqcqs_err:
        for (; i > 0; i--) {
                if (i < max_rx_normal_queues) {
                        ionic_en_rx_ring_deinit(i,
                                                priv_data);
                }
        }

	return status;
}

VMK_ReturnStatus
ionic_stop(struct lif *lif)
{
        VMK_ReturnStatus status, status1 = VMK_OK;
        struct ionic_en_priv_data *priv_data;
        vmk_uint32 i, max_rx_normal_queues;
        vmk_uint32 max_rx_rss_queues, shared_q_data_idx;

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        max_rx_normal_queues = priv_data->uplink_handle.max_rx_normal_queues;

        for (i = 0; i < lif->nrxqcqs; i++) {
                status = ionic_qcq_disable(lif->rxqcqs[i]);
		if (status != VMK_OK) {
                        ionic_err("ionic_qcq_disable() failed, status: %s",
                                  vmk_StatusToString(status));
                        /* In the failure case, we still keep disabling
                           the next qcq element and record the status */
                        status1 = status;
                }
        }

        max_rx_rss_queues = priv_data->uplink_handle.max_rx_rss_queues;

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

                shared_q_data_idx =  max_rx_normal_queues +
                                     max_rx_rss_queues + i;
                ionic_dbg("DEINIT txq, ring_idx: %d, "
                          "shared_q_idx: %d",
                           i, shared_q_data_idx);

                ionic_en_tx_ring_deinit(i,
                                        priv_data);
        }

        for (i = 0; i < lif->nrxqcqs; i++) {
                ionic_rx_flush(&lif->rxqcqs[i]->cq);
                if (i < max_rx_normal_queues) {
                        ionic_dbg("DEINIT normal rxq, ring_idx: %d, "
                                  "shared_q_idx: %d", i, i);
                        ionic_en_rx_ring_deinit(i,
                                                priv_data);
                }
        }

        if (!priv_data->uplink_handle.is_mgmt_nic) {
                ionic_en_rx_rss_deinit(priv_data,
                                       lif);
        }

	return status1;
}

static bool ionic_adminq_service(struct cq *cq,
                                 struct cq_info *cq_info,
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


static VMK_ReturnStatus
ionic_qcq_recover(struct lif *lif,
                  unsigned int index,
                  const char *base,
                  unsigned int num_descs,
                  unsigned int desc_size,
                  unsigned int cq_desc_size,
                  unsigned int sg_desc_size,
                  unsigned int pid,
                  struct qcq *qcq)
{
        VMK_ReturnStatus status;
	void *q_base, *cq_base, *sg_base;
	dma_addr_t q_base_pa, cq_base_pa, sg_base_pa;
	unsigned int q_size = num_descs * desc_size;
	unsigned int cq_size = num_descs * cq_desc_size;

        struct ionic_dev *idev = &lif->ionic->en_dev.idev;

	status = ionic_q_init(lif, idev, &qcq->q, index, base, num_descs,
		              desc_size, sg_desc_size, pid);
	if (status != VMK_OK) {
                ionic_err("ionic_q_init() failed, status: %s",
                          vmk_StatusToString(status));
                VMK_ASSERT(0);
        }

	status = ionic_cq_init(lif, &qcq->cq, &qcq->intr,
	                       num_descs, cq_desc_size);
	if (status != VMK_OK) {
                ionic_err("ionic_cq_init() failed, status: %s",
                          vmk_StatusToString(status));
                VMK_ASSERT(0);
        }

        q_base = qcq->base;
	q_base_pa = qcq->base_pa;

	cq_base = (void *)IONIC_ALIGN((vmk_uintptr_t)q_base + q_size, VMK_PAGE_SIZE);
	cq_base_pa = IONIC_ALIGN(q_base_pa + q_size, VMK_PAGE_SIZE);

	if (qcq->flags & QCQ_F_SG) {
		sg_base = (void *)IONIC_ALIGN((vmk_uintptr_t)cq_base + cq_size,
	             			       VMK_PAGE_SIZE);
		sg_base_pa = IONIC_ALIGN(cq_base_pa + cq_size, VMK_PAGE_SIZE);
		ionic_q_sg_map(&qcq->q, sg_base, sg_base_pa);
	}

	ionic_q_map(&qcq->q, q_base, q_base_pa);
	ionic_cq_map(&qcq->cq, cq_base, cq_base_pa);
	ionic_cq_bind(&qcq->cq, &qcq->q);

        return status;
}


static VMK_ReturnStatus
ionic_qcqs_recover(struct ionic *ionic)
{
        vmk_ListLinks *link;
        VMK_ReturnStatus status;
        unsigned int i, j;
        struct lif *lif = NULL;

        VMK_LIST_FORALL(&ionic->lifs, link) {
                lif = VMK_LIST_ENTRY(link, struct lif, list);
        }

        status = ionic_qcq_recover(lif, 0, "admin", 1 << 4,
                                   sizeof(struct admin_cmd),
                                   sizeof(struct admin_comp),
                                   0, lif->kern_pid, lif->adminqcq);
        VMK_ASSERT(status == VMK_OK);

        if (lif->ionic->nnqs_per_lif) {
                status = ionic_qcq_recover(lif, 0, "notifyq", NOTIFYQ_LENGTH,
                                           sizeof(struct notifyq_cmd),
                                           sizeof(union notifyq_comp),
                                           0, lif->kern_pid, lif->notifyqcq);
                VMK_ASSERT(status == VMK_OK);
        }

	for (i = 0; i < lif->ntxqcqs; i++) {
		status = ionic_qcq_recover(lif, i, "tx", ntxq_descs,
                                          sizeof(struct txq_desc),
				          sizeof(struct txq_comp),
				          sizeof(struct txq_sg_desc),
				          lif->kern_pid, lif->txqcqs[i]);
                VMK_ASSERT(status == VMK_OK);
        }

	for (j = 0; j < lif->nrxqcqs; j++) {
		status = ionic_qcq_recover(lif, j, "rx", nrxq_descs,
		                           sizeof(struct rxq_desc),
				           sizeof(struct rxq_comp),
				           0, lif->kern_pid, lif->rxqcqs[j]);
                VMK_ASSERT(status == VMK_OK);
        }

        lif->rx_mode = 0;
        return status;
}


static VMK_ReturnStatus
ionic_dev_recover(struct ionic_en_priv_data *priv_data)
{
        VMK_ReturnStatus status;
        vmk_AddrCookie driver_data;

        driver_data.ptr = priv_data;

        status = ionic_en_uplink_quiesce_io(driver_data);
        if (status != VMK_OK) {
                ionic_err("ionic_en_uplink_quiesce_io() failed, status: %s",
                          vmk_StatusToString(status));
                VMK_ASSERT(0);
        }

        status = ionic_reset(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_err("ionic_reset() failed, status: %s",
                          vmk_StatusToString(status));
                VMK_ASSERT(0);
        }

	status = ionic_identify(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_err("ionic_identify() failed, status: %s",
                          vmk_StatusToString(status));
                VMK_ASSERT(0);
        } else {
	        ionic_info("ASIC: %s rev: 0x%X serial num: %s fw version: %s\n",
		           ionic_dev_asic_name(priv_data->ionic.ident->dev.asic_type),
        	           priv_data->ionic.ident->dev.asic_rev,
	                   priv_data->ionic.ident->dev.serial_num,
		           priv_data->ionic.ident->dev.fw_version);
        }


        // activeQueues number clean

        // Assume no change in lifs_size

        status = ionic_qcqs_recover(&priv_data->ionic);
        VMK_ASSERT(status == VMK_OK);

        status = ionic_lifs_init(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_err("ionic_lifs_init() failed, status: %s",
                          vmk_StatusToString(status));
                VMK_ASSERT(0);
        }

        ionic_en_uplink_default_coal_params_set(priv_data);

        status = ionic_en_uplink_start_io(driver_data);
        VMK_ASSERT(status == VMK_OK);

        ionic_info("done recovery");
        return status;
}


VMK_ReturnStatus
ionic_dev_recover_world(void *data)
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_priv_data *priv_data = data;

        ionic_dbg("ionic_dev_recover_world() called");

        while (status != VMK_DEATH_PENDING) {
                status = vmk_WorldWait(VMK_EVENT_NONE,
                                       VMK_LOCK_INVALID,
                                       VMK_TIMEOUT_UNLIMITED_MS,
                                       "waiting for recover event");

                if (status == VMK_DEATH_PENDING) {
                        break;
                }    

                ionic_dev_recover(priv_data);
        }

        return VMK_OK;
}


static void
ionic_notifyq_link_change_event(struct ionic_en_uplink_handle *uplink_handle,
                                union notifyq_comp *comp)
{
        vmk_SpinlockLockIgnoreDeathPending(uplink_handle->link_status_lock);
        uplink_handle->cur_hw_link_status.state =
                comp->link_change.link_status == PORT_OPER_STATUS_UP ?
                VMK_LINK_STATE_UP : VMK_LINK_STATE_DOWN;
        uplink_handle->cur_hw_link_status.duplex = VMK_LINK_DUPLEX_FULL;
        uplink_handle->cur_hw_link_status.speed = comp->link_change.link_speed;
        vmk_SpinlockUnlock(uplink_handle->link_status_lock); 

        vmk_WorldForceWakeup(uplink_handle->link_check_world);
}


VMK_ReturnStatus
ionic_lif_set_uplink_info(struct lif *lif)
{
        VMK_ReturnStatus status;
        const char *uplink_name_str;

        struct ionic_admin_ctx ctx = {
//                .work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
                .cmd.netdev_info = {
                        .opcode = CMD_OPCODE_SET_NETDEV_INFO,
                },
        };

        uplink_name_str = vmk_NameToString(&lif->uplink_handle->uplink_name);
        status = vmk_StringLCopy(ctx.cmd.netdev_info.nd_name,
                                 uplink_name_str,
                                 sizeof(ctx.cmd.netdev_info.nd_name),
                                 NULL);
        if (status != VMK_OK) {
                ionic_err("vmk_StringLCopy() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_StringLCopy(ctx.cmd.netdev_info.dev_name,
                                 VMK_PCI_BUS_NAME,
                                 sizeof(ctx.cmd.netdev_info.dev_name),
                                 NULL);
        if (status != VMK_OK) {
                ionic_err("vmk_StringLCopy() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

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

        ionic_info("Setting uplink device name: %s %s",
                   ctx.cmd.netdev_info.nd_name,
                   ctx.cmd.netdev_info.dev_name);

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


static bool ionic_notifyq_cb(struct cq *cq,
                             struct cq_info *cq_info,
                             void *cb_arg)
{
        union notifyq_comp *comp = cq_info->cq_desc;
        struct ionic_en_priv_data *priv_data;
        struct ionic_en_uplink_handle *uplink_handle;
        struct queue *q;
        struct lif *lif;

        q = cq->bound_q;
        lif = q->info[0].cb_arg;
//        netdev = lif->netdev;

        /* Have we run out of new completions to process? */
        if (!(comp->event.eid > lif->last_eid))
                return VMK_FALSE;

        lif->last_eid = comp->event.eid;

        /* TODO: pay attention to link_flap_count
         *       and make sure that enough interrupt credits get
         *       returned in case we missed some events
         */

        ionic_hex_dump("notifyq event ", 
                       comp,
                       sizeof(union notifyq_comp));

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);
        uplink_handle = &priv_data->uplink_handle;

        switch (comp->event.ecode) {
        case EVENT_OPCODE_LINK_CHANGE:
                ionic_info("%s: Notifyq EVENT_OPCODE_LINK_CHANGE eid=%ld",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                ionic_info("%s: Link_status=%d link_errors=0x%02x phy_type=%d link_speed=%d autoneg=%d",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->link_change.link_status,
                           comp->link_change.link_error_bits,
                           comp->link_change.phy_type,
                           comp->link_change.link_speed,
                           comp->link_change.autoneg_status);
                ionic_notifyq_link_change_event(uplink_handle,
                                                comp);
//                vmk_WorldForceWakeup(priv_data->dev_recover_world);
                break;
        case EVENT_OPCODE_RESET:
                ionic_info("%s: Notifyq EVENT_OPCODE_RESET eid=%ld",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                ionic_info("%s: reset_code=%d state=%d",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->reset.reset_code,
                           comp->reset.state);
                break;
        case EVENT_OPCODE_HEARTBEAT:
                ionic_info("%s: Notifyq EVENT_OPCODE_HEARTBEAT eid=%ld\n",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                break;
        case EVENT_OPCODE_LOG:
                ionic_info("%s: Notifyq EVENT_OPCODE_LOG eid=%ld\n",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                ionic_hex_dump("notifyq log",
                               comp->log.data,
                               sizeof(comp->log.data));
                break;
        default:
                ionic_warn("%s: Notifyq bad event ecode=%d eid=%ld\n",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.ecode, comp->event.eid);
                break;
        }

        return VMK_TRUE;
}

static vmk_Bool
ionic_notifyq_netpoll(vmk_AddrCookie priv,
                      vmk_uint32 budget)
{
        vmk_uint32 polled;
        vmk_Bool poll_again = VMK_TRUE;
        void *qcq = priv.ptr;

        ionic_dbg("ionic_notifyq_netpoll(), ring_idx: %d",
                  ((struct qcq*)qcq)->ring_idx);

	polled = ionic_netpoll(budget, ionic_notifyq_cb, qcq);

        if (polled != budget) {
                poll_again = VMK_FALSE;
        }

        return poll_again;
}


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

	ionic_info("lif: %p: rx_filter add ADDR %p (id %d)\n",
                   lif, addr, ctx.comp.rx_filter_add.filter_id);

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
        ionic_completion_destroy(&ctx.work);

        if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

	ionic_info("lif: %p: rx_filter del ADDR %p (id %d)\n",
                   lif, addr, ctx.cmd.rx_filter_del.filter_id);

	return status;
}

static VMK_ReturnStatus
ionic_lif_addr(struct lif *lif,
               const u8 *addr,
               bool add)
{
        if (add) {
	        return ionic_lif_addr_add(lif, addr);
        } else {
		return ionic_lif_addr_del(lif, addr);
	}
}


static VMK_ReturnStatus
ionic_lif_rx_mode(struct lif *lif,
                  unsigned int rx_mode)
{
        VMK_ReturnStatus status;

        ionic_dbg("ionic_lif_rx_mode() called");

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


VMK_ReturnStatus
ionic_set_rx_mode(struct lif *lif,
                  vmk_UplinkState new_state)
{
	unsigned int rx_mode;
	rx_mode = RX_MODE_F_UNICAST;

        if (new_state & VMK_UPLINK_STATE_DISABLED) {
                goto out;
        }

        VMK_ASSERT(new_state & VMK_UPLINK_STATE_ENABLED);

        rx_mode |= (new_state & VMK_UPLINK_STATE_MULTICAST_OK) ?
                   RX_MODE_F_MULTICAST : 0;
        rx_mode |= (new_state & VMK_UPLINK_STATE_MULTICAST_OK) ?
                   RX_MODE_F_ALLMULTI : 0;
	rx_mode |= (new_state & VMK_UPLINK_STATE_BROADCAST_OK ) ?
                   RX_MODE_F_BROADCAST : 0;
	rx_mode |= (new_state & VMK_UPLINK_STATE_PROMISC) ?
                   RX_MODE_F_PROMISC : 0;
out:
	if (lif->rx_mode != rx_mode) {
		lif->rx_mode = rx_mode;
		return ionic_lif_rx_mode(lif, rx_mode);
	}

        return VMK_OK;
}


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
	if (intr->index != INTR_INDEX_NOT_ASSIGNED) {
                vmk_BitVectorClear(lif->ionic->intrs.bit_vector,
                                   intr->index);
        }
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

        ionic_dbg("ionic_qcq_alloc() called");

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

        ionic_dbg("ionic_qcqs_alloc() called");

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

        if (lif->ionic->nnqs_per_lif) {
                flags = QCQ_F_INTR | QCQ_F_NOTIFYQ;
                status = ionic_qcq_alloc(lif, 0, "notifyq", flags, NOTIFYQ_LENGTH,
                                         sizeof(struct notifyq_cmd),
                                         sizeof(union notifyq_comp),
                                         0, lif->kern_pid, &lif->notifyqcq);
                if (status != VMK_OK) {
                        ionic_err("ionic_qcq_alloc() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto notifyq_err;
                }
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

        if (lif->ionic->nnqs_per_lif) {
                ionic_qcq_free(lif, lif->notifyqcq);
        }

notifyq_err:
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

        ionic_qcq_free(lif, lif->notifyqcq);
        lif->notifyqcq = NULL;

	ionic_qcq_free(lif, lif->adminqcq);
        lif->adminqcq = NULL;

        ionic_heap_free(ionic_driver.heap_id, lif->rxqcqs);
        ionic_heap_free(ionic_driver.heap_id, lif->txqcqs);
}

static VMK_ReturnStatus
ionic_lif_alloc(struct ionic *ionic,
                unsigned int index,
                struct ionic_en_uplink_handle *uplink_handle)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
	struct lif *lif;

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

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        /* notify block shared with NIC */
        lif->notifyblock_sz = IONIC_ALIGN(sizeof(*lif->notifyblock), VMK_PAGE_SIZE);
        lif->notifyblock = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                                  priv_data->dma_engine_coherent,
                                                  lif->notifyblock_sz,
                                                  VMK_PAGE_SIZE,
                                                  &lif->notifyblock_pa);
        if (!lif->notifyblock) {
                ionic_err("ionic_dma_zalloc_align() failed, status: %s",
                          vmk_StatusToString(status));
                status = VMK_NO_MEMORY;
                goto notify_bl_err;
        }

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
        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_coherent,
                       lif->notifyblock_sz,
                       lif->notifyblock,
                       lif->notifyblock_pa);
        lif->notifyblock_pa = 0;
        lif->notifyblock = NULL;

notify_bl_err:
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
        struct ionic_en_priv_data *priv_data = NULL;

        ionic_work_queue_flush(lif->def_work_queue);
        ionic_qcqs_free(lif);
        ionic_work_queue_destroy(lif->def_work_queue);
        ionic_spinlock_destroy(lif->deferred.lock); 
        ionic_spinlock_destroy(lif->adminq_lock);

        if (lif->notifyblock) {
                priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                               struct ionic_en_priv_data,
                                               ionic);
                ionic_dma_free(ionic_driver.heap_id,
                               priv_data->dma_engine_coherent,
                               lif->notifyblock_sz,
                               lif->notifyblock,
                               lif->notifyblock_pa);
                lif->notifyblock_pa = 0;
                lif->notifyblock = NULL;
        }

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

static VMK_ReturnStatus
ionic_lif_stats_start(struct lif *lif, unsigned int ver)
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_priv_data *priv_data;

	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_stats = {
			.opcode = CMD_OPCODE_LIF_STATS_START,
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

        lif->lif_stats = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                                priv_data->dma_engine_coherent,
                                                sizeof(*lif->lif_stats),
                                                0,
                                                &lif->lif_stats_pa);
	if (!lif->lif_stats) {
	        ionic_err("ionic_dma_zalloc_align() failed, "
                          "status: VMK_NO_MEMORY");
                ionic_completion_destroy(&ctx.work);
                return  VMK_NO_MEMORY;
	}

	ctx.cmd.lif_stats.addr = lif->lif_stats_pa;

	ionic_info("lif_stats START ver %d addr 0x%lx\n", ver,
		   lif->lif_stats_pa);

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
                       sizeof(*lif->lif_stats),
                       lif->lif_stats,
                       lif->lif_stats_pa);

        lif->lif_stats = NULL;
        lif->lif_stats_pa = 0;

        return status;
}

static void
ionic_lif_stats_stop(struct lif *lif)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	struct ionic_admin_ctx ctx = {
//		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.stats_dump = {
			.opcode = CMD_OPCODE_LIF_STATS_STOP,
		},
	};

        VMK_ASSERT(lif->lif_stats_pa);

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
                       sizeof(*lif->lif_stats),
                       lif->lif_stats,
                       lif->lif_stats_pa);

        lif->lif_stats = NULL;
        lif->lif_stats_pa = 0;
}

VMK_ReturnStatus
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
#ifdef FAKE_ADMINQ
                .side_data = lif->rss_ind_tbl,
                .side_data_len = RSS_IND_TBL_SIZE,
#endif
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

VMK_ReturnStatus
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
                               | RSS_TYPE_IPV6_UDP,
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
        vmk_uint16 ind_tbl_value;
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

	for (i = 0; i < RSS_IND_TBL_SIZE; i++) {
                if (DRSS == 0) {
                        ind_tbl_value = i % lif->nrxqcqs;
                } else {
                        ind_tbl_value = (i % DRSS) +
                                        priv_data->uplink_handle.max_rx_normal_queues;
                }  
		lif->rss_ind_tbl[i] = ind_tbl_value;
        }

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
        vmk_NetPollDestroy(qcq->netpoll); 
        qcq->is_netpoll_created = VMK_FALSE;
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
        if (!(lif->flags & LIF_F_INITED)) {
                return;
        }

	ionic_lif_stats_stop(lif);
	if (lif->uplink_handle->hw_features & ETH_HW_RX_HASH &&
            (!lif->uplink_handle->is_mgmt_nic)) {
                ionic_lif_rss_teardown(lif);
        }

        ionic_lif_txqs_deinit(lif);
	ionic_lif_rxqs_deinit(lif);
	ionic_rx_filters_deinit(lif);

        if (lif->notifyqcq->is_netpoll_enabled) {
                vmk_IntrDisable(lif->notifyqcq->intr.cookie);
                vmk_IntrSync(lif->notifyqcq->intr.cookie);
                vmk_NetPollDisable(lif->notifyqcq->netpoll);
                lif->notifyqcq->is_netpoll_enabled = VMK_FALSE;
                vmk_NetPollInterruptUnSet(lif->notifyqcq->netpoll);
        }
        ionic_lif_qcq_deinit(lif->notifyqcq);

        if (lif->adminqcq->is_netpoll_enabled) {
                vmk_IntrDisable(lif->adminqcq->intr.cookie);
                vmk_IntrSync(lif->adminqcq->intr.cookie);
                vmk_NetPollDisable(lif->adminqcq->netpoll);
                lif->adminqcq->is_netpoll_enabled = VMK_FALSE;
                vmk_NetPollInterruptUnSet(lif->adminqcq->netpoll);
        }
        ionic_lif_qcq_deinit(lif->adminqcq);

        lif->flags &= ~LIF_F_INITED;
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

static void 
ionic_record_irq(struct lif *lif, struct qcq *qcq)
{
	struct intr *intr = &qcq->intr;
	struct queue *q = &qcq->q;

	vmk_Snprintf(intr->name, sizeof(intr->name),
	             "%s-%s-%s", DRV_NAME, lif->name, q->name);
}

static VMK_ReturnStatus
ionic_lif_adminq_init(struct lif *lif)
{
        VMK_ReturnStatus status;
        struct ionic_dev *idev = &lif->ionic->en_dev.idev;
	struct qcq *qcq = lif->adminqcq;
	struct queue *q = &qcq->q;
        struct adminq_init_comp comp;

	ionic_dev_cmd_adminq_init(idev, q, 0, lif->index, qcq->cq.bound_intr->index);

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

        if (!qcq->is_netpoll_created) {
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
                qcq->is_netpoll_created = VMK_TRUE;
        }

        ionic_record_irq(lif, qcq);

	qcq->flags |= QCQ_F_INITED;

        if (!qcq->is_netpoll_enabled) {
                vmk_NetPollInterruptSet(qcq->netpoll,
                                        qcq->intr.cookie);
                vmk_NetPollEnable(qcq->netpoll);
                qcq->is_netpoll_enabled = VMK_TRUE;
                vmk_IntrEnable(qcq->intr.cookie);
	        ionic_intr_mask(&qcq->intr, VMK_FALSE);
        }
	return status;
}


VMK_ReturnStatus
ionic_lif_notifyq_init(struct lif *lif)
{
        VMK_ReturnStatus status;
        struct qcq *qcq = lif->notifyqcq;
        struct queue *q = &qcq->q;

        struct ionic_admin_ctx ctx = {
//                .work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
                .cmd.notifyq_init.opcode = CMD_OPCODE_NOTIFYQ_INIT,
                .cmd.notifyq_init.index = q->index,
                .cmd.notifyq_init.pid = q->pid,
                .cmd.notifyq_init.intr_index = qcq->intr.index,
                .cmd.notifyq_init.lif_index = lif->index,
                .cmd.notifyq_init.ring_size = ionic_ilog2(q->num_descs),
                .cmd.notifyq_init.ring_base = q->base_pa,
                .cmd.notifyq_init.notify_size = ionic_ilog2(lif->notifyblock_sz),
                .cmd.notifyq_init.notify_base = lif->notifyblock_pa,
        };

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_notifyq_ctx.work",
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

        q->qid = ctx.comp.notifyq_init.qid;
        q->qtype = ctx.comp.notifyq_init.qtype;
        q->db = NULL;

        /* preset the callback info */
        q->info[0].cb_arg = lif;

        if (!qcq->is_netpoll_created) {
                status = ionic_en_netpoll_create(&qcq->netpoll,
                                                 lif->uplink_handle,
                                                 IONIC_EN_NOTIFY_RING,
                                                 ionic_notifyq_netpoll,
                                                 qcq);
                if (status != VMK_OK) {
                        ionic_err("ionic_en_netpoll_create() failed, status: %s",
                                  vmk_StatusToString(status));
                        return status;
                }
                qcq->is_netpoll_created = VMK_TRUE;
        }

	ionic_record_irq(lif, qcq);

        qcq->flags |= QCQ_F_INITED;

        /* Enabling interrupts on notifyq from here on... */
        if (!qcq->is_netpoll_enabled) {
                ionic_info("notifyq netpoll enabled");
                vmk_NetPollInterruptSet(qcq->netpoll,
                                        qcq->intr.cookie);
                vmk_NetPollEnable(qcq->netpoll);
                qcq->is_netpoll_enabled = VMK_TRUE;
                vmk_IntrEnable(qcq->intr.cookie);
                ionic_intr_mask(&qcq->intr, VMK_FALSE);
        }

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
			.wanted = ETH_HW_VLAN_RX_FILTER
			        | ETH_HW_RX_HASH
				| ETH_HW_TX_SG
				| ETH_HW_TX_CSUM
				| ETH_HW_RX_CSUM
				| ETH_HW_TSO
				| ETH_HW_TSO_IPV6
				| ETH_HW_TSO_ECN,
		},
	};

        if (vlan_tx_insert) {
                ctx.cmd.features.wanted |= ETH_HW_VLAN_TX_TAG;
        }
        
        if (vlan_rx_strip) {
                ctx.cmd.features.wanted |= ETH_HW_VLAN_RX_STRIP;
        }

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
		ionic_dbg("feature ETH_HW_VLAN_TX_TAG\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_STRIP)
		ionic_dbg("feature ETH_HW_VLAN_RX_STRIP\n");
	if (lif->hw_features & ETH_HW_VLAN_RX_FILTER)
		ionic_dbg("feature ETH_HW_VLAN_RX_FILTER\n");
	if (lif->hw_features & ETH_HW_RX_HASH)
		ionic_dbg("feature ETH_HW_RX_HASH\n");
	if (lif->hw_features & ETH_HW_TX_SG)
		ionic_dbg("feature ETH_HW_TX_SG\n");
	if (lif->hw_features & ETH_HW_TX_CSUM)
		ionic_dbg("feature ETH_HW_TX_CSUM\n");
	if (lif->hw_features & ETH_HW_RX_CSUM)
		ionic_dbg("feature ETH_HW_RX_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO)
		ionic_dbg("feature ETH_HW_TSO\n");
	if (lif->hw_features & ETH_HW_TSO_IPV6)
		ionic_dbg("feature ETH_HW_TSO_IPV6\n");
	if (lif->hw_features & ETH_HW_TSO_ECN)
		ionic_dbg("feature ETH_HW_TSO_ECN\n");
	if (lif->hw_features & ETH_HW_TSO_GRE)
		ionic_dbg("feature ETH_HW_TSO_GRE\n");
	if (lif->hw_features & ETH_HW_TSO_GRE_CSUM)
		ionic_dbg("feature ETH_HW_TSO_GRE_CSUM\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP4)
		ionic_dbg("feature ETH_HW_TSO_IPXIP4\n");
	if (lif->hw_features & ETH_HW_TSO_IPXIP6)
		ionic_dbg("feature ETH_HW_TSO_IPXIP6\n");
	if (lif->hw_features & ETH_HW_TSO_UDP)
		ionic_dbg("feature ETH_HW_TSO_UDP\n");
	if (lif->hw_features & ETH_HW_TSO_UDP_CSUM)
		ionic_dbg("feature ETH_HW_TSO_UDP_CSUM\n");

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
			.I = VMK_TRUE,
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

	ionic_dbg("txq_init.pid %d\n", ctx.cmd.txq_init.pid);
	ionic_dbg("txq_init.index %d\n", ctx.cmd.txq_init.index);
	ionic_dbg("txq_init.ring_base 0x%lx\n",
	           ctx.cmd.txq_init.ring_base);
	ionic_dbg("txq_init.ring_size %d\n",
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

        if (!qcq->is_netpoll_created) {
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
                qcq->is_netpoll_created = VMK_TRUE;
        }

	ionic_record_irq(lif, qcq);

	qcq->flags |= QCQ_F_INITED;

	ionic_dbg("txq->qid %d\n", q->qid);
	ionic_dbg("txq->qtype %d\n", q->qtype);
	ionic_dbg("txq->db %p\n", q->db);

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
			.I = VMK_TRUE,
			.E = VMK_FALSE,
			.pid = q->pid,
			.intr_index = cq->bound_intr->index,
			.type = RXQ_TYPE_ETHERNET,
			.index = q->index,
			.ring_base = q->base_pa,
			.ring_size = ionic_ilog2(q->num_descs),
		},
	};

	ionic_dbg("rxq_init.pid %d\n", ctx.cmd.rxq_init.pid);
	ionic_dbg("rxq_init.index %d\n", ctx.cmd.rxq_init.index);
        ionic_dbg("rxq_init.intr_index %d\n", ctx.cmd.rxq_init.intr_index);
        ionic_dbg("rxq_init.ring_base 0x%lx\n",
		  ctx.cmd.rxq_init.ring_base);
	ionic_dbg("rxq_init.ring_size %d\n",
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

        if (!qcq->is_netpoll_created) {
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
                qcq->is_netpoll_created = VMK_TRUE;
        }

	ionic_record_irq(lif, qcq);

        qcq->flags |= QCQ_F_INITED;

	ionic_dbg("rxq->qid %d\n", q->qid);
	ionic_dbg("rxq->qtype %d\n", q->qtype);
	ionic_dbg("rxq->db %p\n", q->db);

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
                               VMK_FALSE);
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
                       VMK_TRUE);

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

        if (lif->ionic->nnqs_per_lif) {
                status = ionic_lif_notifyq_init(lif);
                if (status != VMK_OK) {
                        ionic_err("ionic_lif_notifyq_init() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto notifyq_err;
                }
                lif->uplink_handle->cur_hw_link_status.state =
                        lif->notifyblock->link_status == PORT_OPER_STATUS_UP ?
                        VMK_LINK_STATE_UP : VMK_LINK_STATE_DOWN;
                lif->uplink_handle->cur_hw_link_status.duplex =
                        VMK_LINK_DUPLEX_FULL;
                lif->uplink_handle->cur_hw_link_status.speed =
                        lif->notifyblock->link_speed;
        }

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
        
	if (lif->uplink_handle->hw_features & ETH_HW_RX_HASH &&
            (!lif->uplink_handle->is_mgmt_nic)) {
		status = ionic_lif_rss_setup(lif);
		if (status != VMK_OK) {
                        ionic_err("ionic_lif_rss_setup() failed, status: %s",
                                  vmk_StatusToString(status));
			goto rss_setup_err;
                }
	}

	status = ionic_lif_stats_start(lif, STATS_DUMP_VERSION_1);
	if (status != VMK_OK) {
                ionic_err("ionic_lif_stats_start() failed, status: %s",
                          vmk_StatusToString(status));
		goto stats_start_err;
        }

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
        lif->flags |= LIF_F_INITED;

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

stats_start_err:
        if (lif->uplink_handle->hw_features & ETH_HW_RX_HASH) {
                ionic_lif_rss_teardown(lif);
        }

rss_setup_err:
        ionic_lif_addr(lif,
                       lif->uplink_handle->vmk_mac_addr,
                       VMK_FALSE);

station_set_err:
        ionic_rx_filters_deinit(lif);

rx_filters_err:
        ionic_lif_rxqs_deinit(lif);

rxqs_init_err:
        ionic_lif_txqs_deinit(lif);

get_features_err:
        ionic_lif_qcq_deinit(lif->notifyqcq);

notifyq_err:
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


VMK_ReturnStatus
ionic_lifs_size(struct ionic *ionic)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
        vmk_uint32 granted = 0;

        union identity *ident = ionic->ident;
        unsigned int nlifs = ident->dev.nlifs;
        unsigned int neqs_per_lif = ident->dev.rdma_eq_qtype.qid_count; 
        unsigned int nnqs_per_lif = ident->dev.notify_qtype.qid_count;
        unsigned int ntxqs_per_lif = ident->dev.tx_qtype.qid_count;
        unsigned int nrxqs_per_lif = ident->dev.rx_qtype.qid_count;
        unsigned int nintrs, dev_nintrs = ident->dev.nintrs;

        ntxqs_per_lif = IONIC_MIN(ntxqs_per_lif,
                                  IONIC_MAX_NUM_TX_QUEUE);
        nrxqs_per_lif = IONIC_MIN(nrxqs_per_lif,
                                  IONIC_MAX_NUM_RX_QUEUE);

try_again:
        nintrs = nlifs * (nnqs_per_lif +
                          //neqs_per_lif +
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

        ionic->nnqs_per_lif = nnqs_per_lif;
        ionic->neqs_per_lif = neqs_per_lif;        
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
        if (nnqs_per_lif > 1) {
                --nnqs_per_lif;
                goto try_again;
        }
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
        return status;

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
