/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

#include "ionic.h"
#include "ionic_lif.h"


static VMK_ReturnStatus
ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode);

static VMK_ReturnStatus
ionic_lif_rss_setup(struct lif *lif);

static void
ionic_lif_rss_teardown(struct lif *lif);

static inline void
ionic_qcq_netpoll_enable(struct qcq *qcq)
{
        vmk_NetPollInterruptSet(qcq->netpoll,
                                qcq->intr.cookie);
        vmk_NetPollEnable(qcq->netpoll);
        qcq->is_netpoll_enabled = VMK_TRUE;
        vmk_IntrEnable(qcq->intr.cookie);
        ionic_intr_mask(&qcq->intr, VMK_FALSE);
}

static inline void
ionic_qcq_netpoll_disable(struct qcq *qcq)
{
        ionic_intr_mask(&qcq->intr, VMK_TRUE);
        vmk_IntrDisable(qcq->intr.cookie);
        vmk_IntrSync(qcq->intr.cookie);
        vmk_NetPollDisable(qcq->netpoll);
        qcq->is_netpoll_enabled = VMK_FALSE;
        vmk_NetPollFlushRx(qcq->netpoll);
        vmk_NetPollInterruptUnSet(qcq->netpoll);
}

VMK_ReturnStatus
ionic_qcq_enable(struct qcq *qcq)
{
	VMK_ReturnStatus status;
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
        struct ionic_admin_ctx ctx = {
		.cmd.q_control = {
			.opcode = IONIC_CMD_Q_CONTROL,
                        .lif_index = lif->index,
			.type = q->type,
			.index = q->index,
                        .oper = IONIC_Q_ENABLE,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        if (qcq->is_netpoll_created &&
            !qcq->is_netpoll_enabled) {
                ionic_qcq_netpoll_enable(qcq);
        }

        status  = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) { 
		ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
			  vmk_StatusToString(status));
                ionic_qcq_netpoll_disable(qcq);
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
		.cmd.q_control = {
			.opcode = IONIC_CMD_Q_CONTROL,
                        .lif_index = lif->index,
			.type = q->type,
			.index = q->index,
                        .oper = IONIC_Q_DISABLE,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }
        
        if (qcq->is_netpoll_created &&
            qcq->is_netpoll_enabled) {
                ionic_qcq_netpoll_disable(qcq);
        }

        return status;
}

VMK_ReturnStatus
ionic_open(struct lif *lif)
{
        VMK_ReturnStatus status = VMK_FAILURE;
        struct ionic_en_priv_data *priv_data;
        vmk_uint32 i, max_rx_normal_queues = 0;
        vmk_uint32 shared_q_data_idx = 0;
        vmk_uint32 max_rx_rss_queues = 0;
        struct ionic *ionic = lif->ionic;

        ionic_en_info("%s: ionic_open",
                      vmk_NameToString(&lif->uplink_handle->uplink_name));

        priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        if (!lif->uplink_handle->is_mgmt_nic) {
                vmk_MutexLock(ionic->dev_cmd_lock);
                ionic_dev_cmd_port_state(&ionic->en_dev.idev,
                                         IONIC_PORT_ADMIN_STATE_UP);
                ionic_dev_cmd_wait_check(ionic,
                                         HZ * devcmd_timeout);
                vmk_MutexUnlock(ionic->dev_cmd_lock);
        }

        if (lif->uplink_handle->cur_hw_link_status.state != VMK_LINK_STATE_UP) {
                ionic_en_info("%s, Port is not UP, skip enabling all the queues.",
                              vmk_NameToString(&lif->uplink_handle->uplink_name));
                priv_data->is_queues_enabled = VMK_FALSE;
                return VMK_NOT_READY;
        }

        max_rx_normal_queues = lif->uplink_handle->max_rx_normal_queues;
        for (i = 0; i < lif->nrxqcqs; i++) {
		ionic_rx_fill(&lif->rxqcqs[i]->q);
                if (i < max_rx_normal_queues) {
                        ionic_en_dbg("INIT normal rxq, ring_idx: %d, "
                                  "shared_q_idx: %d",
                                   i, i);
                        status = ionic_en_rx_ring_init(i,
                                                       i,
                                                       priv_data,
                                                       lif);
        		if (status != VMK_OK) {
                                ionic_en_err("ionic_en_rx_ring_init() failed, "
                                          "status: %s",
                                          vmk_StatusToString(status));
                                i--;
                                goto rxqcqs_err;
                        }

                }
        }

        max_rx_rss_queues = lif->uplink_handle->max_rx_rss_queues;

        if (!lif->uplink_handle->is_mgmt_nic) {
                status = ionic_en_rx_rss_init(priv_data,
                                              lif);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_en_rx_rss_init() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto rxqcqs_err;
                }
        }
        
        for (i = 0; i < lif->ntxqcqs; i++) {
		status = ionic_qcq_enable(lif->txqcqs[i]);
		if (status != VMK_OK) {
                        ionic_en_err("ionic_qcq_enable() failed, status: %s",
                                  vmk_StatusToString(status));
			goto txqcqs_err;
                }

                shared_q_data_idx = max_rx_normal_queues +
                                    max_rx_rss_queues + i;

                ionic_en_dbg("INIT txq, ring_idx: %d, "
                          "shared_q_idx: %d",
                           i, shared_q_data_idx);
                ionic_en_tx_ring_init(i,
                                      shared_q_data_idx,
                                      priv_data,
                                      lif);
	}

        lif->uplink_handle->is_started = VMK_TRUE;
        priv_data->is_queues_enabled = VMK_TRUE;
        return status;

txqcqs_err:
        for (; i > 0; i--) {
                ionic_qcq_disable(lif->txqcqs[i-1]);
                ionic_en_tx_ring_deinit(i,
                                        priv_data);
        }

        for (i = 0; i < lif->nrxqcqs; i++) {
                ionic_qcq_disable(lif->rxqcqs[i]);
        }

        for (i = 0; i < lif->nrxqcqs; i++) {
                ionic_rx_flush(&lif->rxqcqs[i]->cq);
        }

        if (!lif->uplink_handle->is_mgmt_nic) {
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
        
        priv_data->is_queues_enabled = VMK_FALSE;
	return status;
}

VMK_ReturnStatus
ionic_stop(struct lif *lif)
{
        VMK_ReturnStatus status, status1 = VMK_OK;
        struct ionic_en_priv_data *priv_data;
        vmk_uint32 i, max_rx_normal_queues = 0;
        vmk_uint32 max_rx_rss_queues = 0;
        vmk_uint32 shared_q_data_idx = 0;
        struct ionic *ionic = lif->ionic;

        ionic_en_info("%s: ionic_stop",
                      vmk_NameToString(&lif->uplink_handle->uplink_name));

        priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        if (!priv_data->is_queues_enabled) {
                ionic_en_info("%s, Queues are not enabled, we don't need to disable them.",
                              vmk_NameToString(&lif->uplink_handle->uplink_name));
                return VMK_OK;
        }

        max_rx_normal_queues = lif->uplink_handle->max_rx_normal_queues;

        for (i = 0; i < lif->nrxqcqs; i++) {
                status = ionic_qcq_disable(lif->rxqcqs[i]);
		if (status != VMK_OK) {
                        ionic_en_err("rx ionic_qcq_disable() failed, i=%d, status: %s",
                                     i, vmk_StatusToString(status));
                        /* In the failure case, we still keep disabling
                           the next qcq element and record the status */
                        status1 = status;
                }
        }

        max_rx_rss_queues = lif->uplink_handle->max_rx_rss_queues;

	for (i = 0; i < lif->ntxqcqs; i++) {
                ionic_en_tx_ring_deinit(i,
                                        priv_data);

                // TODO post NOP Tx desc and wait for its completion
		// TODO before disabling Tx queue
		status = ionic_qcq_disable(lif->txqcqs[i]);
		if (status != VMK_OK) {
                        ionic_en_err("tx ionic_qcq_disable() failed, i=%d, status: %s",
                                     i, vmk_StatusToString(status));
                        /* In the failure case, we still keep disabling
                           the next qcq element and record the status */
                        status1 = status;
                }

                shared_q_data_idx =  max_rx_normal_queues +
                                     max_rx_rss_queues + i;
                ionic_en_dbg("DEINIT txq, ring_idx: %d, "
                             "shared_q_idx: %d",
                             i, shared_q_data_idx);
        }

        ionic_lif_quiesce(lif);

        for (i = 0; i < lif->nrxqcqs; i++) {
                ionic_rx_flush(&lif->rxqcqs[i]->cq);
                if (i < max_rx_normal_queues) {
                        ionic_en_dbg("DEINIT normal rxq, ring_idx: %d, "
                                     "shared_q_idx: %d", i, i);
                        ionic_en_rx_ring_deinit(i,
                                                priv_data);
                }
        }

        for (i = 0; i < lif->ntxqcqs; i++) {
                ionic_tx_flush(&lif->txqcqs[i]->cq);
        }

        if (!lif->uplink_handle->is_mgmt_nic) {
                ionic_en_rx_rss_deinit(priv_data,
                                       lif);
        }

        priv_data->is_queues_enabled = VMK_FALSE;
	return status1;
}

static bool ionic_adminq_service(struct cq *cq,
                                 struct cq_info *cq_info,
				 void *cb_arg)
{
	struct ionic_admin_comp *comp = cq_info->cq_desc;

	if (!color_match(comp->color, cq->done_color))
		return VMK_FALSE;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return VMK_TRUE;
}

void ionic_adminq_flush(struct lif *lif)
{
        struct queue *adminq = &lif->adminqcq->q;

        vmk_SpinlockLock(lif->adminq_lock);

        while (adminq->tail != adminq->head) {
                vmk_Memset(adminq->tail->desc, 0,
			   sizeof(union ionic_adminq_cmd));
                adminq->tail->cb = NULL;
                adminq->tail->cb_arg = NULL;
                adminq->tail = adminq->tail->next;
        }

        vmk_SpinlockUnlock(lif->adminq_lock);

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
        unsigned int sg_size = num_descs * sg_desc_size;

        struct ionic_dev *idev = &lif->ionic->en_dev.idev;

        vmk_Memset(qcq->q.info,
                   0,
                   sizeof(*qcq->q.info) * num_descs);
        vmk_Memset(qcq->cq.info,
                   0,
                   sizeof(*qcq->cq.info) * num_descs);

	status = ionic_q_init(lif, idev, &qcq->q, index, base, num_descs,
		              desc_size, sg_desc_size, pid);
	if (status != VMK_OK) {
                ionic_en_err("ionic_q_init() failed, status: %s",
                             vmk_StatusToString(status));
                return status;
        }

	status = ionic_cq_init(lif, &qcq->cq, &qcq->intr,
	                       num_descs, cq_desc_size);
	if (status != VMK_OK) {
                ionic_en_err("ionic_cq_init() failed, status: %s",
                             vmk_StatusToString(status));
                return status;
        }

        q_base = qcq->base;
	q_base_pa = qcq->base_pa;
        vmk_Memset(q_base, 0, q_size);

	cq_base = (void *)IONIC_ALIGN((vmk_uintptr_t)q_base + q_size, VMK_PAGE_SIZE);
	cq_base_pa = IONIC_ALIGN(q_base_pa + q_size, VMK_PAGE_SIZE);
        vmk_Memset(cq_base, 0, cq_size);

	if (qcq->flags & QCQ_F_SG) {
		sg_base = (void *)IONIC_ALIGN((vmk_uintptr_t)cq_base + cq_size,
	             			       VMK_PAGE_SIZE);
                vmk_Memset(sg_base, 0, sg_size);
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
                                   sizeof(struct ionic_admin_cmd),
                                   sizeof(struct ionic_admin_comp),
                                   0, lif->kern_pid, lif->adminqcq);
        if (status != VMK_OK) {
                ionic_en_err("ionic_qcq_recover for adminq failed, status: %s",
                             vmk_StatusToString(status));
                return status;
        }

        if (lif->ionic->nnqs_per_lif) {
                status = ionic_qcq_recover(lif, 0, "notifyq", NOTIFYQ_LENGTH,
                                           sizeof(struct ionic_notifyq_cmd),
                                           sizeof(union ionic_notifyq_comp),
                                           0, lif->kern_pid, lif->notifyqcq);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_qcq_recover for notifyq failed,"
                                     "status: %s", vmk_StatusToString(status));
                        return status;
                }
        }

        for (i = 0; i < lif->ntxqcqs; i++) {
                status = ionic_qcq_recover(lif, i, "tx", ntxq_descs,
                                          sizeof(struct ionic_txq_desc),
                                          sizeof(struct ionic_txq_comp),
                                          sizeof(struct ionic_txq_sg_desc),
                                          lif->kern_pid, lif->txqcqs[i]);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_qcq_recover for txq[%d] failed,"
                                     "status: %s", i, vmk_StatusToString(status));
                        return status;
                }
        }

        for (j = 0; j < lif->nrxqcqs; j++) {
                status = ionic_qcq_recover(lif, j, "rx", nrxq_descs,
                                           sizeof(struct ionic_rxq_desc),
                                           sizeof(struct ionic_rxq_comp),
                                           0, lif->kern_pid, lif->rxqcqs[j]);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_qcq_recover for rxq[%d] failed,"
                                     "status: %s", i, vmk_StatusToString(status));
                        return status;
                }
        }

        lif->rx_mode = 0;
        return status;
}


void
ionic_stop_control_path(struct lif *lif)
{
        struct qcq *adminqcq, *notifyqcq;
        adminqcq = lif->adminqcq;
        notifyqcq = lif->notifyqcq;

        ionic_intr_mask(&adminqcq->intr, VMK_TRUE);
        vmk_IntrDisable(adminqcq->intr.cookie);
        vmk_IntrSync(adminqcq->intr.cookie);
        vmk_NetPollDisable(adminqcq->netpoll);
        adminqcq->is_netpoll_enabled = VMK_FALSE;
        vmk_NetPollFlushRx(adminqcq->netpoll);
        vmk_NetPollInterruptUnSet(adminqcq->netpoll);
        ionic_adminq_flush(lif);

        vmk_IntrDisable(notifyqcq->intr.cookie);
        vmk_IntrSync(notifyqcq->intr.cookie);
        vmk_NetPollDisable(notifyqcq->netpoll);
        notifyqcq->is_netpoll_enabled = VMK_FALSE;
        vmk_NetPollFlushRx(notifyqcq->netpoll);
        vmk_NetPollInterruptUnSet(notifyqcq->netpoll);
}


static void
ionic_clean_priv_sw_stats(struct lif *lif)
{
        vmk_uint32 i;
        struct rx_stats *rx_stats = NULL;
        struct tx_stats *tx_stats = NULL;

        for (i = 0; i < lif->nrxqcqs; i++) {
                rx_stats = &lif->rxqcqs[i]->stats.rx;
                vmk_Memset(rx_stats, 0, sizeof(struct rx_stats));
        }

        for (i = 0; i < lif->ntxqcqs; i++) {
                tx_stats = &lif->txqcqs[i]->stats.tx;
                vmk_Memset(tx_stats, 0, sizeof(struct tx_stats));
        }
}


static VMK_ReturnStatus
ionic_dev_recover(struct ionic_en_priv_data *priv_data)
{
        VMK_ReturnStatus status;
        vmk_AddrCookie driver_data;
        struct lif *lif;

        driver_data.ptr = priv_data;

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        status = ionic_init(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_init() failed, status: %s",
                             vmk_StatusToString(status));
                goto out_err;
        }

	status = ionic_identify(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_identify() failed, status: %s",
                             vmk_StatusToString(status));
                goto out_err;
        }

        vmk_BitVectorZap(priv_data->uplink_handle.uplink_q_info.activeQueues);

        // Assume no change in lifs_size

        status = ionic_qcqs_recover(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_qcqs_recover() failed, status: %s",
                             vmk_StatusToString(status));
                goto out_err;
        }

        vmk_Memset(priv_data->uplink_handle.vmk_mac_addr,
                   0,
                   VMK_ETH_ADDR_LENGTH);

        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                             struct lif, list);

        lif->last_eid = 0;

        status = ionic_lifs_init(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_en_err("ionic_lifs_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto out_err;
        }

        lif->is_skip_res_alloc_after_fw = VMK_FALSE;

        ionic_lif_set_uplink_info(lif);

        ionic_en_uplink_default_coal_params_set(priv_data);

        ionic_clean_priv_sw_stats(lif);
        status = ionic_en_uplink_start_io(driver_data);
        if (status != VMK_OK) {
                ionic_en_err("ionic_en_uplink_start_io() failed, status: %s",
                             vmk_StatusToString(status));
                goto out_err;
        }

        ionic_en_info("Recovery from FW upgrade has been completed!");

        return status;

out_err:
        ionic_en_err("Recovery from FW upgrade failed!");
        return status;
}


VMK_ReturnStatus
ionic_dev_recover_world(void *data)
{
        VMK_ReturnStatus status = VMK_OK;
        struct ionic_en_priv_data *priv_data = data;
        vmk_uint32 wait_time = VMK_TIMEOUT_UNLIMITED_MS;
        vmk_Bool is_done_fw_upgrade = VMK_FALSE;
        struct lif *lif;

        ionic_en_dbg("ionic_dev_recover_world() called");

        while (status != VMK_DEATH_PENDING) {
                status = vmk_WorldWait(VMK_EVENT_NONE,
                                       VMK_LOCK_INVALID,
                                       wait_time,
                                       "waiting for recover event");

                if (status == VMK_DEATH_PENDING) {
                        break;
                }    

                /* wait one second */
                wait_time = 1000;

                if (!is_done_fw_upgrade) {
                        lif = VMK_LIST_ENTRY(vmk_ListFirst(&priv_data->ionic.lifs),
                                             struct lif,
                                             list);
                        lif->is_skip_res_alloc_after_fw = VMK_TRUE;
                        ionic_stop_control_path(lif);
                        is_done_fw_upgrade = VMK_TRUE;
                }

                if (priv_data->ionic.en_dev.idev.dev_info_regs->fw_status) {
                        ionic_dev_recover(priv_data);
                        wait_time = VMK_TIMEOUT_UNLIMITED_MS;
                        is_done_fw_upgrade = VMK_FALSE;
                }
        }

        return VMK_OK;
}


static void
ionic_notifyq_link_change_event(struct lif *lif)
{
        VMK_ReturnStatus status;
        vmk_Bool is_start_io = VMK_FALSE;
        struct ionic_en_uplink_handle *uplink_handle = lif->uplink_handle;
        vmk_Bool is_link_state_changed = VMK_FALSE;
        vmk_LinkState new_link_state;
        vmk_AddrCookie driver_data;

        if (uplink_handle->uplink_dev == NULL) {
                ionic_en_warn("Ignore link change actions since device is "
                              "not fully initialized");
                return;
        }

        vmk_SpinlockLockIgnoreDeathPending(uplink_handle->link_status_lock);
        new_link_state = (lif->info->status.link_status == IONIC_PORT_OPER_STATUS_UP) ?
                         VMK_LINK_STATE_UP : VMK_LINK_STATE_DOWN;

        is_link_state_changed = (lif->uplink_handle->cur_hw_link_status.state != new_link_state);

        uplink_handle->cur_hw_link_status.state = new_link_state;
        uplink_handle->cur_hw_link_status.duplex = VMK_LINK_DUPLEX_FULL;
        uplink_handle->cur_hw_link_status.speed = lif->info->status.link_speed;
        vmk_SpinlockUnlock(uplink_handle->link_status_lock); 

        if (is_link_state_changed &&
            new_link_state == VMK_LINK_STATE_UP) {
                is_start_io = VMK_TRUE;
        } else if (is_link_state_changed &&
                   new_link_state == VMK_LINK_STATE_DOWN) {
                is_start_io = VMK_FALSE;
        }

        ionic_en_info("is_link_state_changed: %d, is_start_io: %d",
                      is_link_state_changed, is_start_io);

        if (is_link_state_changed) {
                driver_data.ptr = uplink_handle->priv_data;
                if (is_start_io) {
                        status = ionic_en_uplink_start_io(driver_data);
                        VMK_ASSERT(status == VMK_OK);
                } else {
                        status = ionic_en_uplink_quiesce_io(driver_data);
                        VMK_ASSERT(status == VMK_OK);
                }
        }

        vmk_WorldForceWakeup(uplink_handle->link_check_world);
}


VMK_ReturnStatus
ionic_lif_set_uplink_info(struct lif *lif)
{
        VMK_ReturnStatus status;
        const char *uplink_name_str;

        struct ionic_admin_ctx ctx = {
//                .work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.lif_setattr = {
			.opcode = IONIC_CMD_LIF_SETATTR,
			 lif->index,
			.attr = IONIC_LIF_ATTR_NAME,
		},
        };

        uplink_name_str = vmk_NameToString(&lif->uplink_handle->uplink_name);
        status = vmk_StringLCopy(ctx.cmd.lif_setattr.name,
                                 uplink_name_str,
                                 sizeof(ctx.cmd.lif_setattr.name),
                                 NULL);
        if (status != VMK_OK) {
                ionic_en_err("vmk_StringLCopy() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        ionic_en_info("Setting uplink device name: %s",
                   ctx.cmd.lif_setattr.name);

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


static void
ionic_lif_reset(struct lif *lif)
{
        struct ionic_dev *idev = &lif->ionic->en_dev.idev;

        vmk_MutexLock(lif->ionic->dev_cmd_lock);
        ionic_dev_cmd_lif_reset(idev, lif->index);
        ionic_dev_cmd_wait_check(lif->ionic, HZ * devcmd_timeout);
        vmk_MutexUnlock(lif->ionic->dev_cmd_lock);
}


static void
ionic_all_rxq_empty(struct lif *lif)
{
        unsigned int i;

        for (i = 0; i < lif->nrxqcqs; i++) {
                ionic_rx_empty(&lif->rxqcqs[i]->q);
        }
}


static bool ionic_notifyq_cb(struct cq *cq,
                             struct cq_info *cq_info,
                             void *cb_arg)
{
        union ionic_notifyq_comp *comp = cq_info->cq_desc;
        struct ionic_en_priv_data *priv_data;
        struct ionic_en_uplink_handle *uplink_handle;
        struct queue *q;
        struct lif *lif;

        q = cq->bound_q;
        lif = q->info[0].cb_arg;

        /* Have we run out of new completions to process? */
        if (!(comp->event.eid > lif->last_eid))
                return VMK_FALSE;

        if (comp->event.eid != lif->last_eid + 1) {
                ionic_en_warn("Notifyq missed events, eid=%ld, expected=%ld\n",
                              comp->event.eid, lif->last_eid + 1);
        }

        lif->last_eid = comp->event.eid;

        ionic_hex_dump("notifyq event ", 
                       comp,
                       sizeof(union ionic_notifyq_comp),
                       ionic_driver.log_component);

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);
        uplink_handle = &priv_data->uplink_handle;

        switch (comp->event.ecode) {
        case IONIC_EVENT_LINK_CHANGE:
                ionic_en_info("%s: Notifyq IONIC_EVENT_LINK_CHANGE eid=%ld",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                ionic_en_info("%s: link_status=%d link_speed=%d",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->link_change.link_status,
                           comp->link_change.link_speed);
                ionic_notifyq_link_change_event(lif);
                break;
        case IONIC_EVENT_RESET:
                ionic_en_info("%s: Notifyq IONIC_EVENT_RESET eid=%ld",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                ionic_en_info("%s: reset_code=%d state=%d",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->reset.reset_code,
                           comp->reset.state);
                ionic_intr_mask(&lif->notifyqcq->intr, VMK_TRUE);

                /* Skip disabling netpoll */
                lif->notifyqcq->is_netpoll_enabled = VMK_FALSE;

                ionic_qcq_disable(lif->notifyqcq);
                ionic_all_rxq_empty(lif);
                ionic_lif_reset(lif);
                ionic_reset(&priv_data->ionic);
                vmk_WorldForceWakeup(priv_data->dev_recover_world);
                break;
        case IONIC_EVENT_HEARTBEAT:
                ionic_en_info("%s: Notifyq IONIC_EVENT_HEARTBEAT eid=%ld\n",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                break;
        case IONIC_EVENT_LOG:
                ionic_en_info("%s: Notifyq IONIC_EVENT_LOG eid=%ld\n",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                ionic_hex_dump("notifyq log",
                               comp->log.data,
                               sizeof(comp->log.data),
                               ionic_driver.log_component);
                break;
        case IONIC_EVENT_XCVR:
                ionic_en_info("%s: Notifyq IONIC_EVENT_XCVR eid=%ld\n",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.eid);
                break;
        default:
                ionic_en_warn("%s: Notifyq bad event ecode=%d eid=%ld\n",
                           vmk_NameToString(&uplink_handle->uplink_name),
                           comp->event.ecode, comp->event.eid);
                break;
        }

        return VMK_TRUE;
}


void ionic_notifyq_flush(struct cq *cq)
{
        unsigned int work_done;

        work_done = ionic_cq_service(cq, cq->num_descs, ionic_notifyq_cb, NULL);

        if (work_done)
                ionic_intr_return_credits(cq->bound_intr, work_done, 0, VMK_TRUE);
}


static vmk_Bool
ionic_notifyq_netpoll(vmk_AddrCookie priv,
                      vmk_uint32 budget)
{
        vmk_uint32 polled;
        vmk_Bool poll_again = VMK_TRUE;
        void *qcq = priv.ptr;

        ionic_en_dbg("ionic_notifyq_netpoll(), ring_idx: %d",
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
		.cmd.rx_filter_add = {
			.opcode = IONIC_CMD_RX_FILTER_ADD,
			.match = IONIC_RX_FILTER_MATCH_MAC,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	vmk_Memcpy(ctx.cmd.rx_filter_add.mac.addr, addr, ETH_ALEN);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	ionic_en_info("lif: %p: rx_filter add ADDR %p (id %d)\n",
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
			.opcode = IONIC_CMD_RX_FILTER_DEL,
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
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        vmk_SpinlockLock(lif->rx_filters.lock);
	f = ionic_rx_filter_by_addr(lif, addr);
	if (!f) {
                vmk_SpinlockUnlock(lif->rx_filters.lock);
                return VMK_FAILURE;
	}

	ctx.cmd.rx_filter_del.filter_id = f->filter_id;
	ionic_rx_filter_free(f);
        vmk_SpinlockUnlock(lif->rx_filters.lock);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);

        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

	ionic_en_info("lif: %p: rx_filter del ADDR %p (id %d)",
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

        ionic_en_dbg("ionic_lif_rx_mode() called");

	struct ionic_admin_ctx ctx = {
		.cmd.rx_mode_set = {
			.opcode = IONIC_CMD_RX_MODE_SET,
			.rx_mode = rx_mode,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	if (rx_mode & IONIC_RX_MODE_F_UNICAST)
		ionic_en_info("rx_mode RX_MODE_F_UNICAST\n");
	if (rx_mode & IONIC_RX_MODE_F_MULTICAST)
		ionic_en_info("rx_mode RX_MODE_F_MULTICAST\n");
	if (rx_mode & IONIC_RX_MODE_F_BROADCAST)
		ionic_en_info("rx_mode RX_MODE_F_BROADCAST\n");
	if (rx_mode & IONIC_RX_MODE_F_PROMISC)
		ionic_en_info("rx_mode RX_MODE_F_PROMISC\n");
	if (rx_mode & IONIC_RX_MODE_F_ALLMULTI)
		ionic_en_info("rx_mode RX_MODE_F_ALLMULTI\n");

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
	}

        return status;
}


VMK_ReturnStatus
ionic_set_rx_mode(struct lif *lif,
                  vmk_UplinkState new_state)
{
	unsigned int rx_mode;
	rx_mode = IONIC_RX_MODE_F_UNICAST;

        if (new_state & VMK_UPLINK_STATE_DISABLED) {
                goto out;
        }

        VMK_ASSERT(new_state & VMK_UPLINK_STATE_ENABLED);

        rx_mode |= (new_state & VMK_UPLINK_STATE_MULTICAST_OK) ?
                   IONIC_RX_MODE_F_MULTICAST : 0;
        rx_mode |= (new_state & VMK_UPLINK_STATE_MULTICAST_OK) ?
                   IONIC_RX_MODE_F_ALLMULTI : 0;
	rx_mode |= (new_state & VMK_UPLINK_STATE_BROADCAST_OK ) ?
                   IONIC_RX_MODE_F_BROADCAST : 0;
	rx_mode |= (new_state & VMK_UPLINK_STATE_PROMISC) ?
                   IONIC_RX_MODE_F_PROMISC : 0;
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
		.cmd.rx_filter_add = {
			.opcode = IONIC_CMD_RX_FILTER_ADD,
			.match = IONIC_RX_FILTER_MATCH_VLAN,
			.vlan.vlan = vid,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	ionic_en_info("rx_filter add VLAN %d (id %d)", vid,
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
		.cmd.rx_filter_del = {
			.opcode = IONIC_CMD_RX_FILTER_DEL,
		},
	};

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
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
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	ionic_en_info("rx_filter del VLAN %d (id %d)", vid,
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
                ionic_en_err("vmk_BitVectorNextBit() is not able "
                          " to find a free bit");
                return VMK_NOT_FOUND;
        }

        vmk_BitVectorSet(ionic->intrs.bit_vector,
                         index);

	ionic_intr_init(idev, intr, index);

        return VMK_OK;
}

void IONIC_QINIT_Free(struct lif *lif, struct intr *intr)
{
	if (intr->index != IONIC_INTR_INDEX_NOT_ASSIGNED) {
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
                unsigned int type,
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
	struct qcq *new;
	unsigned int q_size = num_descs * desc_size;
	unsigned int cq_size = num_descs * cq_desc_size;
	unsigned int sg_size = num_descs * sg_desc_size;
	unsigned int total_size;
	void *q_base, *cq_base, *sg_base;
	dma_addr_t q_base_pa, cq_base_pa, sg_base_pa;

	*qcq = NULL;

        ionic_en_dbg("ionic_qcq_alloc() called");

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

	total_size = IONIC_ALIGN(q_size, VMK_PAGE_SIZE) +
                     IONIC_ALIGN(cq_size, VMK_PAGE_SIZE);
	if (flags & QCQ_F_SG)
		total_size += IONIC_ALIGN(sg_size, VMK_PAGE_SIZE);

        new = ionic_heap_zalloc(ionic_driver.heap_id,
                                sizeof(*new));
        if (!new) {
                ionic_en_err("ionic_heap_zalloc() failed, status: "
                          "VMK_NO_MEMORY");
		return status;
        }

        new->q.type = type;
        new->ring_idx = index;
	new->flags = flags;

        new->q.info = ionic_heap_zalloc(ionic_driver.heap_id,
                                        sizeof(*new->q.info) * num_descs);
        if (!new->q.info) {
                ionic_en_err("ionic_heap_zalloc() failed, status: "
                          "VMK_NO_MEMORY");
                goto q_info_alloc_err;
        }

	status = ionic_q_init(lif, idev, &new->q, index, base, num_descs,
		              desc_size, sg_desc_size, pid);
	if (status != VMK_OK) {
                ionic_en_err("ionic_q_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto q_init_err;
        }

	if (flags & QCQ_F_INTR) {
		status = ionic_intr_alloc(lif, &new->intr);
		if (status != VMK_OK) {
                        ionic_en_err("ionic_intr_alloc() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto q_init_err;
                }

                new->intr.cookie = priv_data->intr_cookie_array[new->intr.index];
                status = ionic_int_register(ionic_driver.module_id,
                                            priv_data->ionic.en_dev.vmk_device,
                                            new->intr.cookie,
                                            "msix_handle",
                                            ionic_msix_handle,
                                            new);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_int_register() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto int_reg_err;
                }

		ionic_intr_mask_on_assertion(&new->intr);
	} else {
		new->intr.index = IONIC_INTR_INDEX_NOT_ASSIGNED;
	}

        new->cq.info = ionic_heap_zalloc(ionic_driver.heap_id,
                                         sizeof(*new->cq.info) * num_descs);
        if (!new->cq.info) {
                ionic_en_info("ionic_heap_zalloc() failed, status: VMK_NO_MEMORY");
                status = VMK_NO_MEMORY;
                goto cq_info_err;
        }

	status = ionic_cq_init(lif, &new->cq, &new->intr,
	                       num_descs, cq_desc_size);
	if (status != VMK_OK) {
                ionic_en_err("ionic_cq_init() failed, status: %s",
                          vmk_StatusToString(status));
		goto cq_init_err;
        }

        new->base = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                           priv_data->dma_engine_coherent,
                                           total_size,
                                           VMK_PAGE_SIZE,
                                           &new->base_pa);
        if (!new->base) {
                ionic_en_err("ionic_dma_zalloc_align() failed, "
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
        IONIC_QINIT_Free(lif, &new->intr);
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
                IONIC_QINIT_Free(lif, &qcq->intr);
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

        ionic_en_dbg("ionic_qcqs_alloc() called");

        lif->txqcqs = ionic_heap_zalloc(ionic_driver.heap_id,
                                        sizeof(*lif->txqcqs) * lif->ntxqcqs);
        if (!lif->txqcqs) {
                ionic_en_err("ionic_heap_zalloc() failed, status: "
                          "VMK_NO_MEMORY");
		return VMK_NO_MEMORY;
        }

        lif->rxqcqs = ionic_heap_zalloc(ionic_driver.heap_id,
                                        sizeof(*lif->rxqcqs) * lif->nrxqcqs);
        if (!lif->rxqcqs) {
                ionic_en_err("ionic_heap_zalloc() failed, status: "
                          "VMK_NO_MEMORY");
		status = VMK_NO_MEMORY;
                goto rxqcqs_err;
        }

	flags = QCQ_F_INTR;
	status = ionic_qcq_alloc(lif, IONIC_QTYPE_ADMINQ, 0, "admin", flags,
                                 1 << 4,
	                         sizeof(struct ionic_admin_cmd),
			         sizeof(struct ionic_admin_comp),
			         0, lif->kern_pid, &lif->adminqcq);
	if (status != VMK_OK) {
                ionic_en_err("ionic_qcq_alloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto adminqcq_err;
        }

        if (lif->ionic->nnqs_per_lif) {
                flags = QCQ_F_INTR | QCQ_F_NOTIFYQ;
                status = ionic_qcq_alloc(lif, IONIC_QTYPE_NOTIFYQ, 0, "notifyq", flags,
                                         NOTIFYQ_LENGTH,
                                         sizeof(struct ionic_notifyq_cmd),
                                         sizeof(union ionic_notifyq_comp),
                                         0, lif->kern_pid, &lif->notifyqcq);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_qcq_alloc() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto notifyq_err;
                }
        }

        flags = QCQ_F_TX_STATS | QCQ_F_INTR | QCQ_F_SG;
        for (i = 0; i < lif->ntxqcqs; i++) {
                status = ionic_qcq_alloc(lif, IONIC_QTYPE_TXQ, i, "tx", flags,
                                         ntxq_descs,
                                         sizeof(struct ionic_txq_desc),
                                         sizeof(struct ionic_txq_comp),
                                         sizeof(struct ionic_txq_sg_desc),
                                         lif->kern_pid, &lif->txqcqs[i]);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_qcq_alloc() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto tx_qcq_alloc_err;
                }
        }

        flags = QCQ_F_RX_STATS | QCQ_F_INTR;
        for (j = 0; j < lif->nrxqcqs; j++) {
                status = ionic_qcq_alloc(lif, IONIC_QTYPE_RXQ, j, "rx", flags,
                                         nrxq_descs,
                                         sizeof(struct ionic_rxq_desc),
                                         sizeof(struct ionic_rxq_comp),
                                         0, lif->kern_pid, &lif->rxqcqs[j]);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_qcq_alloc() failed, status: %s",
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

        if (lif->notifyqcq) {
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
                ionic_en_err("ionic_heap_zalign() failed, status: VMK_NO_MEMORY");
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
                ionic_en_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto adminq_lock_err;
        }

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        /* notify block shared with NIC */
        lif->info_sz = IONIC_ALIGN(sizeof(*lif->info), VMK_PAGE_SIZE);
        lif->info = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                                  priv_data->dma_engine_coherent,
                                                  lif->info_sz,
                                                  VMK_PAGE_SIZE,
                                                  &lif->info_pa);
        if (!lif->info) {
                ionic_en_err("ionic_dma_zalloc_align() failed, status: %s",
                          vmk_StatusToString(status));
                status = VMK_NO_MEMORY;
                goto notify_bl_err;
        }

	status = ionic_qcqs_alloc(lif);
	if (status != VMK_OK) {
                ionic_en_err("ionic_qcqs_alloc() failed, status: %s",
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
                       lif->info_sz,
                       lif->info,
                       lif->info_pa);
        lif->info_pa = 0;
        lif->info = NULL;

notify_bl_err:
        ionic_spinlock_destroy(lif->adminq_lock);

adminq_lock_err:
        ionic_heap_free(ionic_driver.heap_id,
                        lif);

	return status;
}

static void
ionic_lif_free(struct lif *lif)
{
        struct ionic_en_priv_data *priv_data = NULL;

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                        struct ionic_en_priv_data,
                                        ionic);

        ionic_lif_reset(lif);
        ionic_qcqs_free(lif);
        ionic_spinlock_destroy(lif->adminq_lock);

        ionic_dma_free(ionic_driver.heap_id,
                        priv_data->dma_engine_coherent,
                        lif->info_sz,
                        lif->info,
                        lif->info_pa);
        lif->info_pa = 0;
        lif->info = NULL;

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
        struct ionic_identity *ident = &ionic->ident;

        vmk_ListInit(&ionic->lifs);

        priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

	for (i = 0; i < ident->dev.nlifs; i++) {
		status = ionic_lif_alloc(ionic, i, &priv_data->uplink_handle);
		if (status != VMK_OK) {
                        ionic_en_err("ionic_lif_alloc() failed, status: %s",
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
}


VMK_ReturnStatus
ionic_lif_rss_config(struct lif *lif, const u16 types, const u8 *key, const u32 *indir)
{
        VMK_ReturnStatus status;
        unsigned int i;

        struct ionic_admin_ctx ctx = { 
                .cmd.lif_setattr = { 
                        .opcode = IONIC_CMD_LIF_SETATTR,
                        .attr = IONIC_LIF_ATTR_RSS,
                        .rss.types = types,
                        .rss.addr = lif->rss_ind_tbl_pa,
                },
        };

        status = ionic_completion_create(ionic_driver.module_id,
                                         ionic_driver.heap_id,
                                         ionic_driver.lock_domain,
                                         "ionic_admin_ctx.work",
                                         &ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_completion_create() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        ionic_completion_init(&ctx.work);

        lif->rss_types = types;

        if (key)
                vmk_Memcpy(lif->rss_hash_key, key, IONIC_RSS_HASH_KEY_SIZE);

        if (indir)
                for (i = 0; i < lif->ionic->ident.lif.eth.rss_ind_tbl_sz; i++)
                        lif->rss_ind_tbl[i] = indir[i];

        vmk_Memcpy(ctx.cmd.lif_setattr.rss.key, lif->rss_hash_key,
                   IONIC_RSS_HASH_KEY_SIZE);

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}

static VMK_ReturnStatus
ionic_lif_rss_setup(struct lif *lif)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
        vmk_uint16 ind_tbl_value;
        unsigned int i;

	size_t tbl_size = sizeof(*lif->rss_ind_tbl) *
                          lif->ionic->ident.lif.eth.rss_ind_tbl_sz;
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

        lif->rss_types = IONIC_RSS_TYPE_IPV4
                        | IONIC_RSS_TYPE_IPV4_TCP
                        | IONIC_RSS_TYPE_IPV4_UDP
                        | IONIC_RSS_TYPE_IPV6
                        | IONIC_RSS_TYPE_IPV6_TCP
                        | IONIC_RSS_TYPE_IPV6_UDP;

        if (!lif->is_skip_res_alloc_after_fw) {
                lif->rss_ind_tbl = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                                          priv_data->dma_engine_coherent,
                                                          tbl_size,
                                                          VMK_PAGE_SIZE,
                                                          &lif->rss_ind_tbl_pa);

        	if (!lif->rss_ind_tbl) {
	                ionic_en_err("ionic_dma_zalloc_align() failed, "
                                  "status: VMK_NO_MEMORY");
                        return  VMK_NO_MEMORY;
                }
        } else {
                vmk_Memset(lif->rss_ind_tbl, 0, tbl_size);
        }

	ionic_en_info("rss_ind_tbl_pa: %lu", lif->rss_ind_tbl_pa);

	for (i = 0; i < lif->ionic->ident.lif.eth.rss_ind_tbl_sz; i++) {
                ind_tbl_value = (i % lif->uplink_handle->DRSS) +
                                priv_data->uplink_handle.max_rx_normal_queues;
		lif->rss_ind_tbl[i] = ind_tbl_value;
        }

	status = ionic_lif_rss_config(lif, lif->rss_types,
                                      toeplitz_symmetric_key, NULL);
	if (status != VMK_OK) {
                ionic_en_err("ionic_lif_rss_config() failed, status: %s",
                          vmk_StatusToString(status));
		goto lif_rss_config_err;
        }

	return status;

lif_rss_config_err:
        ionic_lif_rss_teardown(lif);
	return status;
}

static void
ionic_lif_rss_teardown(struct lif *lif)
{
        struct ionic_en_priv_data *priv_data;
	size_t tbl_size = sizeof(*lif->rss_ind_tbl) *
                          lif->ionic->ident.lif.eth.rss_ind_tbl_sz;

	if (!lif->rss_ind_tbl)
		return;

        ionic_lif_rss_config(lif, 0x0, NULL, NULL);

        priv_data = IONIC_CONTAINER_OF(lif->ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_coherent,
                       tbl_size,
                       lif->rss_ind_tbl,
                       lif->rss_ind_tbl_pa);

	lif->rss_ind_tbl = NULL;
        lif->rss_ind_tbl_pa = 0;
}

static void ionic_lif_qcq_deinit(struct qcq *qcq)
{
	if (!(qcq->flags & QCQ_F_INITED))
		return;
	ionic_intr_mask(&qcq->intr, VMK_TRUE);
        vmk_NetPollDestroy(qcq->netpoll); 
        qcq->is_netpoll_enabled = VMK_FALSE;
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


VMK_ReturnStatus
ionic_lif_quiesce(struct lif *lif)
{
        VMK_ReturnStatus status;
        struct ionic_admin_ctx ctx = {
                .cmd.lif_setattr = {
                        .opcode = IONIC_CMD_LIF_SETATTR,
                        .attr = IONIC_LIF_ATTR_STATE,
                        .index = lif->index,
                        .state = IONIC_LIF_DISABLE
                },
        };

	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


static void ionic_lif_deinit(struct lif *lif)
{
        if (!(lif->flags & LIF_F_INITED)) {
                return;
        }

        if (lif->notifyqcq->is_netpoll_enabled) {
                ionic_qcq_disable(lif->notifyqcq);
        }
        ionic_lif_qcq_deinit(lif->notifyqcq);

	if (lif->uplink_handle->hw_features & IONIC_ETH_HW_RX_HASH &&
            (!lif->uplink_handle->is_mgmt_nic) && lif->uplink_handle->DRSS) {
                ionic_lif_rss_teardown(lif);
        }

        ionic_lif_txqs_deinit(lif);
	ionic_lif_rxqs_deinit(lif);
        ionic_rx_filters_deinit(lif);

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
        struct ionic_q_init_comp comp;

        vmk_MutexLock(lif->ionic->dev_cmd_lock);
        ionic_dev_cmd_adminq_init(idev, qcq, lif->index, qcq->cq.bound_intr->index);

        status = ionic_dev_cmd_wait_check(lif->ionic, HZ * devcmd_timeout);
        vmk_MutexUnlock(lif->ionic->dev_cmd_lock);
	if (status != VMK_OK) {
                ionic_en_err("ionic_dev_cmd_wait_check() failed, status: %s",
                          vmk_StatusToString(status));
	        return status;
        }

	ionic_dev_cmd_comp(idev, &comp);
	q->hw_index = comp.hw_index;
	q->hw_type = comp.hw_type;
	q->db = ionic_db_map(idev, q);

        if (!qcq->is_netpoll_created) {
                status = ionic_en_netpoll_create(&qcq->netpoll,
                                                 lif->uplink_handle,
                                                 IONIC_EN_ADMIN_RING,
                                                 ionic_adminq_netpoll,
                                                 qcq);

                if (status != VMK_OK) {
                        ionic_en_err("ionic_en_netpoll_create() failed, status: %s",
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
                ionic_intr_clean(&qcq->intr);
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
                .cmd.q_init = {
                        .opcode = IONIC_CMD_Q_INIT,
                        .lif_index = lif->index,
                        .type = q->type,
                        .index = q->index,
                        .flags = (IONIC_QINIT_F_IRQ | IONIC_QINIT_F_ENA),
                        .intr_index = qcq->intr.index,
                        .pid = q->pid,
                        .ring_size = ionic_ilog2(q->num_descs),
                        .ring_base = q->base_pa,
                }
        };

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_notifyq_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

        status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

        q->hw_index = ctx.comp.q_init.hw_index;
        q->hw_type = ctx.comp.q_init.hw_type;
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
                        ionic_en_err("ionic_en_netpoll_create() failed, status: %s",
                                  vmk_StatusToString(status));
                        return status;
                }
                qcq->is_netpoll_created = VMK_TRUE;
        }

	ionic_record_irq(lif, qcq);

        qcq->flags |= QCQ_F_INITED;

        /* Enabling interrupts on notifyq from here on... */
        if (!qcq->is_netpoll_enabled) {
                vmk_NetPollInterruptSet(qcq->netpoll,
                                        qcq->intr.cookie);
                vmk_NetPollEnable(qcq->netpoll);
                qcq->is_netpoll_enabled = VMK_TRUE;
                vmk_IntrEnable(qcq->intr.cookie);
                ionic_intr_clean(&qcq->intr);
                ionic_intr_mask(&qcq->intr, VMK_FALSE);
        }

        return status;
}



static VMK_ReturnStatus
ionic_get_features(struct lif *lif)
{
        VMK_ReturnStatus status;

	struct ionic_admin_ctx ctx = {
		.cmd.lif_setattr = {
			.opcode = IONIC_CMD_LIF_SETATTR,
			 lif->index,
			.attr = IONIC_LIF_ATTR_FEATURES,
			.features = IONIC_ETH_HW_VLAN_RX_FILTER
                                | IONIC_ETH_HW_TX_SG
                                | IONIC_ETH_HW_TX_CSUM
                                | IONIC_ETH_HW_RX_CSUM
                                | IONIC_ETH_HW_TSO
                                | IONIC_ETH_HW_TSO_IPV6
                                | IONIC_ETH_HW_TSO_ECN,
		},
	};

        if (lif->uplink_handle->DRSS) {
                ctx.cmd.lif_setattr.features |= IONIC_ETH_HW_RX_HASH;
        }

        if (vlan_tx_insert) {
                ctx.cmd.lif_setattr.features |= IONIC_ETH_HW_VLAN_TX_TAG;
        }
        
        if (vlan_rx_strip) {
                ctx.cmd.lif_setattr.features |= IONIC_ETH_HW_VLAN_RX_STRIP;
        }

        if (geneve_offload && !lif->uplink_handle->is_mgmt_nic) {
                ctx.cmd.lif_setattr.features |= IONIC_ETH_HW_RX_CSUM_GENEVE
                                             | IONIC_ETH_HW_TX_CSUM_GENEVE
                                             | IONIC_ETH_HW_TSO_GENEVE;
        }

     	status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	lif->hw_features = ctx.cmd.lif_setattr.features &
			   ctx.comp.lif_setattr.features;

	if (lif->hw_features & IONIC_ETH_HW_VLAN_TX_TAG)
		ionic_en_dbg("feature IONIC_ETH_HW_VLAN_TX_TAG\n");
	if (lif->hw_features & IONIC_ETH_HW_VLAN_RX_STRIP)
		ionic_en_dbg("feature IONIC_ETH_HW_VLAN_RX_STRIP\n");
	if (lif->hw_features & IONIC_ETH_HW_VLAN_RX_FILTER)
		ionic_en_dbg("feature IONIC_ETH_HW_VLAN_RX_FILTER\n");
	if (lif->hw_features & IONIC_ETH_HW_RX_HASH)
		ionic_en_dbg("feature IONIC_ETH_HW_RX_HASH\n");
	if (lif->hw_features & IONIC_ETH_HW_TX_SG)
		ionic_en_dbg("feature IONIC_ETH_HW_TX_SG\n");
	if (lif->hw_features & IONIC_ETH_HW_TX_CSUM)
		ionic_en_dbg("feature IONIC_ETH_HW_TX_CSUM\n");
	if (lif->hw_features & IONIC_ETH_HW_RX_CSUM)
		ionic_en_dbg("feature IONIC_ETH_HW_RX_CSUM\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_IPV6)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_IPV6\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_ECN)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_ECN\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_GRE)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_GRE\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_GRE_CSUM)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_GRE_CSUM\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_IPXIP4)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_IPXIP4\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_IPXIP6)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_IPXIP6\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_UDP)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_UDP\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_UDP_CSUM)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_UDP_CSUM\n");
	if (lif->hw_features & IONIC_ETH_HW_RX_CSUM_GENEVE)
		ionic_en_dbg("feature IONIC_ETH_HW_RX_CSUM_GENEVE\n");
	if (lif->hw_features & IONIC_ETH_HW_TX_CSUM_GENEVE)
		ionic_en_dbg("feature IONIC_ETH_HW_TX_CSUM_GENEVE\n");
	if (lif->hw_features & IONIC_ETH_HW_TSO_GENEVE)
		ionic_en_dbg("feature IONIC_ETH_HW_TSO_GENEVE\n");

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
		.cmd.q_init = {
			.opcode = IONIC_CMD_Q_INIT,
			.lif_index = lif->index,
			.type = q->type,
			.index = q->index,
			.flags = (IONIC_QINIT_F_IRQ | IONIC_QINIT_F_SG),
			.intr_index = cq->bound_intr->index,
			.pid = q->pid,
			.ring_size = ionic_ilog2(q->num_descs),
			.ring_base = q->base_pa,
                        .cq_ring_base = cq->base_pa,
                        .sg_ring_base = q->sg_base_pa,
		},
	};

	ionic_en_dbg("txq_init.pid %d\n", ctx.cmd.q_init.pid);
	ionic_en_dbg("txq_init.index %d\n", ctx.cmd.q_init.index);
	ionic_en_dbg("txq_init.ring_base 0x%lx\n", ctx.cmd.q_init.ring_base);
	ionic_en_dbg("txq_init.ring_size %d\n", ctx.cmd.q_init.ring_size);

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	q->hw_index = ctx.comp.q_init.hw_index;
	q->hw_type = ctx.comp.q_init.hw_type;
	q->db = ionic_db_map(q->idev, q);

        if (!qcq->is_netpoll_created) {
                status = ionic_en_netpoll_create(&qcq->netpoll,
                                                 lif->uplink_handle,
                                                 IONIC_EN_TX_RING,
                                                 ionic_tx_netpoll,
                                                 qcq);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_en_netpoll_create() failed, status: %s",
                                  vmk_StatusToString(status));
                        return status;
                }
                qcq->is_netpoll_created = VMK_TRUE;
        }

	ionic_record_irq(lif, qcq);

	qcq->flags |= QCQ_F_INITED;

	ionic_en_dbg("txq->hw_index %d\n", q->hw_index);
	ionic_en_dbg("txq->hw_type %d\n", q->hw_type);
	ionic_en_dbg("txq->db %p\n", q->db);

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
                        ionic_en_err("ionic_lif_txq_init() failed, "
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
		.cmd.q_init = {
			.opcode = IONIC_CMD_Q_INIT,
			.lif_index = lif->index,
			.type = q->type,
			.index = q->index,
			.flags = IONIC_QINIT_F_IRQ,
			.intr_index = cq->bound_intr->index,
			.pid = q->pid,
			.ring_size = ionic_ilog2(q->num_descs),
			.ring_base = q->base_pa,
			.cq_ring_base = cq->base_pa,
		},
	};

	ionic_en_dbg("rxq_init.pid %d\n", ctx.cmd.q_init.pid);
	ionic_en_dbg("rxq_init.index %d\n", ctx.cmd.q_init.index);
        ionic_en_dbg("rxq_init.intr_index %d\n", ctx.cmd.q_init.intr_index);
        ionic_en_dbg("rxq_init.ring_base 0x%lx\n", ctx.cmd.q_init.ring_base);
	ionic_en_dbg("rxq_init.ring_size %d\n", ctx.cmd.q_init.ring_size);

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
        if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

	q->hw_index = ctx.comp.q_init.hw_index;
	q->hw_type = ctx.comp.q_init.hw_type;
	q->db = ionic_db_map(q->idev, q);

        if (!qcq->is_netpoll_created) {
                status = ionic_en_netpoll_create(&qcq->netpoll,
                                                 lif->uplink_handle,
                                                 IONIC_EN_RX_RING,
                                                 ionic_rx_netpoll,
                                                 qcq);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_en_netpoll_create() failed, status: %s",
                                  vmk_StatusToString(status));
                        return status;
                }
                qcq->is_netpoll_created = VMK_TRUE;
        }

	ionic_record_irq(lif, qcq);

        qcq->flags |= QCQ_F_INITED;

	ionic_en_dbg("rxq->hw_index %d\n", q->hw_index);
	ionic_en_dbg("rxq->hw_type %d\n", q->hw_type);
	ionic_en_dbg("rxq->db %p\n", q->db);

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
                        ionic_en_err("ionic_lif_rxq_init() failed, "
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
		.cmd.lif_getattr = {
			.opcode = IONIC_CMD_LIF_GETATTR,
			 lif->index,
			.attr = IONIC_LIF_ATTR_MAC,
		},
	};

     	status = ionic_completion_create(ionic_driver.module_id,
                                         ionic_driver.heap_id,
                                         ionic_driver.lock_domain,
                                         "ionic_admin_ctx.work",
                                         &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

	ionic_completion_init(&ctx.work);

	status = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

        vmk_Memset(&zero_eth_addr, 0, VMK_ETH_ADDR_LENGTH);
        if (vmk_Memcmp(lif->uplink_handle->vmk_mac_addr,
                       zero_eth_addr,
                       VMK_ETH_ADDR_LENGTH)) {
                ionic_en_info("deleting station MAC addr");
                ionic_lif_addr(lif,
                               lif->uplink_handle->vmk_mac_addr,
                               VMK_FALSE);
        }            

        vmk_Memcpy(lif->uplink_handle->vmk_mac_addr,
                   ctx.comp.lif_getattr.mac,
                   VMK_ETH_ADDR_LENGTH);
        vmk_Memcpy(lif->uplink_handle->uplink_shared_data.macAddr,
                   lif->uplink_handle->vmk_mac_addr,
                   VMK_ETH_ADDR_LENGTH);
        vmk_Memcpy(lif->uplink_handle->uplink_shared_data.hwMacAddr,
                   lif->uplink_handle->vmk_mac_addr,
                   VMK_ETH_ADDR_LENGTH);
 
        ionic_en_info("adding station MAC addr");
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
        struct ionic_q_init_comp comp;

        vmk_MutexLock(lif->ionic->dev_cmd_lock);
	ionic_dev_cmd_lif_init(idev, lif->index, lif->info_pa);
	status = ionic_dev_cmd_wait_check(lif->ionic, HZ * devcmd_timeout);
        ionic_dev_cmd_comp(idev, &comp);
        vmk_MutexUnlock(lif->ionic->dev_cmd_lock);
	if (status != VMK_OK) {
                ionic_en_err("ionic_dev_cmd_wait_check() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

        lif->hw_index = comp.hw_index;

        lif->uplink_handle->cur_hw_link_status.state =
                lif->info->status.link_status == IONIC_PORT_OPER_STATUS_UP ?
                VMK_LINK_STATE_UP : VMK_LINK_STATE_DOWN;
        lif->uplink_handle->cur_hw_link_status.duplex =
                VMK_LINK_DUPLEX_FULL;
        lif->uplink_handle->cur_hw_link_status.speed =
                lif->info->status.link_speed;

	status = ionic_lif_adminq_init(lif);
	if (status != VMK_OK) {
                ionic_en_err("ionic_lif_adminq_init() failed, status: %s",
                          vmk_StatusToString(status));
                // TODO: we might need to add LIF_DINIT cmd_opcode to dinit the lif
                return status;
        }

        if (lif->ionic->nnqs_per_lif) {
                status = ionic_lif_notifyq_init(lif);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_lif_notifyq_init() failed, status: %s",
                                  vmk_StatusToString(status));
                        goto notifyq_err;
                }
        }

	status  = ionic_get_features(lif);
	if (status != VMK_OK) {
                ionic_en_err("ionic_get_features() failed, status: %s",
                          vmk_StatusToString(status));
		goto get_features_err;
        }

	status = ionic_lif_txqs_init(lif);
	if (status != VMK_OK) {
                ionic_en_err("ionic_lif_txqs_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto get_features_err;
        }

	status = ionic_lif_rxqs_init(lif);
	if (status != VMK_OK) {
                ionic_en_err("ionic_lif_rxqs_init() failed, status: %s",
                          vmk_StatusToString(status));
		goto rxqs_init_err;
        }

	status = ionic_rx_filters_init(lif);
	if (status != VMK_OK) {
                ionic_en_err("ionic_rx_filters_init() failed, status: %s",
                          vmk_StatusToString(status));
		goto rx_filters_err;
        }

        status = ionic_station_set(lif);
	if (status != VMK_OK) {
                ionic_en_err("ionic_station_set() failed, status: %s",
                          vmk_StatusToString(status));
		goto station_set_err;
        }
        
	if (lif->uplink_handle->hw_features & IONIC_ETH_HW_RX_HASH &&
            (!lif->uplink_handle->is_mgmt_nic)) {
                if (lif->uplink_handle->DRSS) {
        		status = ionic_lif_rss_setup(lif);
	        	if (status != VMK_OK) {
                                ionic_en_err("ionic_lif_rss_setup() failed, status: %s",
                                          vmk_StatusToString(status));
        			goto rss_setup_err;
                        }
                }
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
        ionic_lif_reset(lif);

notifyq_err:
	ionic_intr_mask(&lif->adminqcq->intr, VMK_TRUE);
        ionic_lif_adminq_deinit(lif);
	return status;
}

VMK_ReturnStatus
ionic_lifs_init(struct ionic *ionic)
{
        VMK_ReturnStatus status = VMK_FAILURE;
        vmk_ListLinks *link;
        struct lif *lif;

        VMK_LIST_FORALL(&ionic->lifs, link) {
                lif = VMK_LIST_ENTRY(link, struct lif, list);
                status = ionic_lif_init(lif);
                if (status != VMK_OK) {
                        ionic_en_err("ionic_lif_init() failed, status: %s",
                                  vmk_StatusToString(status));
                        break;
                }
        }

	return status;
}

VMK_ReturnStatus
ionic_lif_identify(struct ionic *ionic)
{
        VMK_ReturnStatus status;
        struct ionic_dev *idev = &ionic->en_dev.idev;
        struct ionic_identity *ident = &ionic->ident;
        struct ionic_en_priv_data *priv_data;
        vmk_uint32 i, max_filters;
        unsigned int nwords;

        vmk_MutexLock(ionic->dev_cmd_lock);
        ionic_dev_cmd_lif_identify(idev, IONIC_LIF_TYPE_CLASSIC,
                                   IONIC_IDENTITY_VERSION_1);
        status = ionic_dev_cmd_wait_check(ionic, HZ * devcmd_timeout);
	vmk_MutexUnlock(ionic->dev_cmd_lock);
	if (status != VMK_OK) {
                ionic_en_err("ionic_dev_cmd_wait_check() failed, status: %s",
                          vmk_StatusToString(status));
		return status;
        }

        priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

	nwords = IONIC_MIN(ARRAY_SIZE(ident->lif.words),
                           ARRAY_SIZE(idev->dev_cmd_regs->data));
	for (i = 0; i < nwords; i++)
		ident->lif.words[i] = ionic_readl_raw((vmk_VA)&idev->dev_cmd_regs->data[i]);

	ionic_en_info("capabilities 0x%lx ", ident->lif.capabilities);
	ionic_en_info("eth.features 0x%lx ", ident->lif.eth.config.features);
	ionic_en_info("eth.queue_count[IONIC_QTYPE_ADMINQ] 0x%x ",
                      ident->lif.eth.config.queue_count[IONIC_QTYPE_ADMINQ]);
	ionic_en_info("eth.queue_count[IONIC_QTYPE_NOTIFYQ] 0x%x ",
                      ident->lif.eth.config.queue_count[IONIC_QTYPE_NOTIFYQ]);
	ionic_en_info("eth.queue_count[IONIC_QTYPE_RXQ] 0x%x ",
                      ident->lif.eth.config.queue_count[IONIC_QTYPE_RXQ]);
	ionic_en_info("eth.queue_count[IONIC_QTYPE_TXQ] 0x%x ",
                      ident->lif.eth.config.queue_count[IONIC_QTYPE_TXQ]);
	ionic_en_info("eth.max_ucast_filters 0x%x ",
                      ident->lif.eth.max_ucast_filters);
	ionic_en_info("eth.max_mcast_filters 0x%x ",
                      ident->lif.eth.max_mcast_filters);

        max_filters = IONIC_MIN(ident->lif.eth.max_mcast_filters,
                                ident->lif.eth.max_ucast_filters);

        priv_data->max_filters = IONIC_MIN(max_filters,
                                           IONIC_EN_MAX_FILTERS_PER_RX_Q);

	return VMK_OK;
}

VMK_ReturnStatus
ionic_lifs_size(struct ionic *ionic)
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
        vmk_uint32 granted = 0;

        struct ionic_identity *ident = &ionic->ident;
        unsigned int nlifs = ident->dev.nlifs;
        unsigned int neqs_per_lif = ident->lif.rdma.eq_qtype.qid_count; 
        unsigned int nnqs_per_lif = ident->lif.eth.config.queue_count[IONIC_QTYPE_NOTIFYQ];
        unsigned int ntxqs_per_lif = ident->lif.eth.config.queue_count[IONIC_QTYPE_TXQ];
        unsigned int nrxqs_per_lif = ident->lif.eth.config.queue_count[IONIC_QTYPE_RXQ];
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
                ionic_en_err("ionic_heap_zalloc() failed, status: NO MEMORY, size: %ld",
                          sizeof(vmk_IntrCookie) * nintrs);
                return VMK_NO_MEMORY;
        }
        
        status = ionic_int_alloc(ionic_driver.module_id,
                                 ionic_driver.heap_id,
                                 ionic->en_dev.pci_device,
                                 VMK_PCI_INTERRUPT_TYPE_MSIX,
                                 nintrs,
                                 nintrs,
                                 priv_data->intr_cookie_array,
                                 &granted);
        if (status != VMK_OK) {
                ionic_en_err("ionic_int_alloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto intr_alloc_err;
        }

        ionic->neqs_per_lif = neqs_per_lif;
        ionic_en_info("ntxqs_per_lif:%d, nrxqs_per_lif: %d, nints: %d",
                  ntxqs_per_lif, nrxqs_per_lif, nintrs);

        ionic->nnqs_per_lif = nnqs_per_lif;
        ionic->neqs_per_lif = neqs_per_lif;
        ionic->ntxqs_per_lif = ntxqs_per_lif;
        ionic->nrxqs_per_lif = nrxqs_per_lif;
        ionic->nintrs = nintrs;

        ionic->intrs.bit_vector = vmk_BitVectorAlloc(ionic_driver.heap_id,
                                                     IONIC_INTR_CTRL_REGS_MAX);
        if (!ionic->intrs.bit_vector) {
                ionic_en_err("vmk_BitVectorAlloc() failed, "
                             "status: VMK_NO_MEMORY");
                status = VMK_NO_MEMORY;
                goto intrs_bit_vector_err;
        }

        ionic_en_info("neqs_per_lif: %d, ntxqs_per_lif: %d, "
                      "nrxqs_per_lif: %d, nintrs: %d, nlifs: %d", neqs_per_lif,
                      ntxqs_per_lif, nrxqs_per_lif, nintrs, nlifs);

        priv_data->is_lifs_size_compl = VMK_TRUE;
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

        priv_data->is_lifs_size_compl = VMK_FALSE;
}
