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
#include <linux/types.h>
#include <linux/string.h>
#include <linux/errno.h>

#include "ionic_dev.h"

#ifndef ADMINQ

#define HZ		CONFIG_HZ	/* Internal kernel timer frequency */

int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait);

static int ionic_dev_cmd_wait(struct queue *adminq, desc_cb cb, void *cb_arg)
{
	struct admin_comp comp;
	struct cq_info cq_info = {
		.cq_desc = &comp,
	};
	int err;

	err = ionic_dev_cmd_wait_check(adminq->idev, HZ * 2);
	if (err)
		return err;
	ionic_dev_cmd_comp(adminq->idev, &comp);
	if (cb)
		cb(adminq, adminq->head, &cq_info, cb_arg);

	return 0;
}
#endif

int ionic_adminq_txq_init(struct queue *adminq, struct queue *txq,
			  struct cq *cq, unsigned int cos, desc_cb cb,
			  void *cb_arg)
{
#ifdef ADMINQ
	struct txq_init_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_TXQ_INIT;
	cmd->I = false;
	cmd->E = false;
	cmd->pid = txq->pid;
	cmd->intr_index = cq->bound_intr->index;
	cmd->type = TXQ_TYPE_ETHERNET;
	cmd->index = txq->index;
	cmd->cos = cos;
	cmd->ring_base = txq->base_pa;
	cmd->ring_size = ilog2(txq->num_descs);

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	ionic_dev_cmd_txq_init(adminq->idev, txq, cq, cos);
	return ionic_dev_cmd_wait(adminq, cb, cb_arg);
#endif
}

int ionic_adminq_rxq_init(struct queue *adminq, struct queue *rxq,
			  struct cq *cq, desc_cb cb, void *cb_arg)
{
#ifdef ADMINQ
	struct rxq_init_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_TXQ_INIT;
	cmd->I = false;
	cmd->E = false;
	cmd->pid = rxq->pid;
	cmd->intr_index = cq->bound_intr->index;
	cmd->type = TXQ_TYPE_ETHERNET;
	cmd->index = rxq->index;
	cmd->ring_base = rxq->base_pa;
	cmd->ring_size = ilog2(rxq->num_descs);

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	ionic_dev_cmd_rxq_init(adminq->idev, rxq, cq);
	return ionic_dev_cmd_wait(adminq, cb, cb_arg);
#endif
}

int ionic_adminq_q_enable(struct queue *adminq, struct queue *q, desc_cb cb,
			  void *cb_arg)
{
#ifdef ADMINQ
	struct q_enable_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_Q_ENABLE;
	cmd->qid = q->qid;
	cmd->qtype = q->qtype;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	ionic_dev_cmd_q_enable(adminq->idev, q);
	return ionic_dev_cmd_wait(adminq, cb, cb_arg);
#endif
}

int ionic_adminq_q_disable(struct queue *adminq, struct queue *q, desc_cb cb,
			   void *cb_arg)
{
#ifdef ADMINQ
	struct q_disable_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_Q_DISABLE;
	cmd->qid = q->qid;
	cmd->qtype = q->qtype;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	ionic_dev_cmd_q_disable(adminq->idev, q);
	return ionic_dev_cmd_wait(adminq, cb, cb_arg);
#endif
}

int ionic_adminq_station_get(struct queue *adminq, desc_cb cb, void *cb_arg)
{
#ifdef ADMINQ
	struct station_mac_addr_get_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_STATION_MAC_ADDR_GET;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	ionic_dev_cmd_station_get(adminq->idev);
	return ionic_dev_cmd_wait(adminq, cb, cb_arg);
#endif
}

int ionic_adminq_mtu_set(struct queue *adminq, int new_mtu, desc_cb cb,
			 void *cb_arg)
{
#ifdef ADMINQ
	struct mtu_set_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_MTU_SET;
	cmd->mtu = new_mtu;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	ionic_dev_cmd_mtu_set(adminq->idev, new_mtu);
	return ionic_dev_cmd_wait(adminq, cb, cb_arg);
#endif
}

int ionic_adminq_rx_mode_set(struct queue *adminq, unsigned int rx_mode,
			     desc_cb cb, void *cb_arg)
{
#ifdef ADMINQ
	struct rx_mode_set_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_RX_MODE_SET;
	cmd->rx_mode = rx_mode;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	// Issue cmd and forget...don't wait for completion
	ionic_dev_cmd_rx_mode_set(adminq->idev, rx_mode);
	return 0;
#endif
}

int ionic_adminq_rx_filter_mac(struct queue *adminq, const u8 *addr, bool add,
			       desc_cb cb, void *cb_arg)
{
#ifdef ADMINQ
	struct rx_filter_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
			    CMD_OPCODE_RX_FILTER_DEL;
	cmd->match = RX_FILTER_MATCH_MAC;
	memcpy(&cmd->addr, addr, sizeof(cmd->addr));

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	// Issue cmd and forget...don't wait for completion
	ionic_dev_cmd_rx_filter_mac(adminq->idev, addr, add);
	return 0;
#endif
}

int ionic_adminq_rx_filter_vlan(struct queue *adminq, u16 vid, bool add,
				desc_cb cb, void *cb_arg)
{
#ifdef ADMINQ
	struct rx_filter_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
			    CMD_OPCODE_RX_FILTER_DEL;
	cmd->match = RX_FILTER_MATCH_VLAN;
	cmd->vlan = vid;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	// Issue cmd and forget...don't wait for completion
	ionic_dev_cmd_rx_filter_vlan(adminq->idev, vid, add);
	return 0;
#endif
}

int ionic_adminq_features(struct queue *adminq, u16 set, desc_cb cb,
			  void *cb_arg)
{
#ifdef ADMINQ
	struct features_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_FEATURES;
	cmd->set = set;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
#else
	ionic_dev_cmd_features(adminq->idev, set);
	return ionic_dev_cmd_wait(adminq, cb, cb_arg);
#endif
}

int ionic_adminq_rdma_cmd(struct queue *adminq, struct admin_cmd *cmd,
			  desc_cb cb, void *cb_arg)
{
	switch (cmd->opcode) {
	case CMD_OPCODE_RDMA_CREATE_CQ:
	case CMD_OPCODE_RDMA_CREATE_MR:
	case CMD_OPCODE_RDMA_CREATE_QP:
		if (!ionic_q_has_space(adminq, 1))
			return -ENOSPC;
		ionic_q_post(adminq, true, cb, cb_arg);
		break;
	default:
		return -ENOTSUPP;
	}

	return 0;
}

bool ionic_adminq_service(struct cq *cq, struct cq_info *cq_info,
			  void *cb_arg)
{
	struct admin_comp *comp = cq_info->cq_desc;

	if (comp->color != cq->done_color)
		return 0;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return true;
}
