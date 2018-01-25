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

static void ionic_adminq_txq_init_comp(struct queue *q,
				       struct desc_info *desc_info,
				       struct cq_info *cq_info, void *cb_arg)
{
	struct queue *txq = cb_arg;
	struct txq_init_comp *comp = cq_info->cq_desc;

	txq->qid = comp->qid;
	txq->qtype = comp->qtype;
	txq->db = ionic_db_map(q->idev, txq);
}

int ionic_adminq_txq_init(struct queue *adminq, struct queue *txq,
			  struct cq *cq, unsigned int cos)
{
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

	ionic_q_post(adminq, true, ionic_adminq_txq_init_comp, txq);

	return 0;
}

static void ionic_adminq_rxq_init_comp(struct queue *q,
				       struct desc_info *desc_info,
				       struct cq_info *cq_info, void *cb_arg)
{
	struct queue *rxq = cb_arg;
	struct rxq_init_comp *comp = cq_info->cq_desc;

	rxq->qid = comp->qid;
	rxq->qtype = comp->qtype;
	rxq->db = ionic_db_map(q->idev, rxq);
}

int ionic_adminq_rxq_init(struct queue *adminq, struct queue *rxq,
			  struct cq *cq)
{
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

	ionic_q_post(adminq, true, ionic_adminq_rxq_init_comp, rxq);

	return 0;
}

int ionic_adminq_q_enable(struct queue *adminq, struct queue *q, desc_cb cb,
			  void *cb_arg)
{
	struct q_enable_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_Q_ENABLE;
	cmd->qid = q->qid;
	cmd->qtype = q->qtype;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
}

int ionic_adminq_q_disable(struct queue *adminq, struct queue *q, desc_cb cb,
			   void *cb_arg)
{
	struct q_disable_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_Q_DISABLE;
	cmd->qid = q->qid;
	cmd->qtype = q->qtype;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
}

int ionic_adminq_station_get(struct queue *adminq, desc_cb cb, void *cb_arg)
{
	struct station_mac_addr_get_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_STATION_MAC_ADDR_GET;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
}

int ionic_adminq_mtu_set(struct queue *adminq, int new_mtu, desc_cb cb,
			 void *cb_arg)
{
	struct mtu_set_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_MTU_SET;
	cmd->mtu = new_mtu;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
}

int ionic_adminq_rx_mode_set(struct queue *adminq, unsigned int rx_mode,
			     desc_cb cb, void *cb_arg)
{
	struct rx_mode_set_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_RX_MODE_SET;
	cmd->rx_mode = rx_mode;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
}

int ionic_adminq_rx_filter_mac(struct queue *adminq, const u8 *addr, bool add,
			       desc_cb cb, void *cb_arg)
{
	struct rx_filter_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
			    CMD_OPCODE_RX_FILTER_DEL;
	cmd->match = RX_FILTER_MATCH_MAC;
	memcpy(&cmd->addr, addr, sizeof(cmd->addr));

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
}

int ionic_adminq_rx_filter_vlan(struct queue *adminq, u16 vid, bool add,
				desc_cb cb, void *cb_arg)
{
	struct rx_filter_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
			    CMD_OPCODE_RX_FILTER_DEL;
	cmd->match = RX_FILTER_MATCH_VLAN;
	cmd->vlan = vid;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
}

int ionic_adminq_features(struct queue *adminq, u16 set, desc_cb cb,
			  void *cb_arg)
{
	struct features_cmd *cmd = adminq->head->desc;

	if (!ionic_q_has_space(adminq, 1))
		return -ENOSPC;

	cmd->opcode = CMD_OPCODE_FEATURES;
	cmd->set = set;

	ionic_q_post(adminq, true, cb, cb_arg);

	return 0;
}

int ionic_adminq_rdma_cmd(struct queue *adminq, struct admin_cmd *cmd,
			  desc_cb cb, void *cb_arg)
{
	switch (cmd->opcode) {
	case CMD_OPCODE_RDMA_CMD1:
	case CMD_OPCODE_RDMA_CMD2:
	case CMD_OPCODE_RDMA_CMD3:
		if (!ionic_q_has_space(adminq, 1))
			return -ENOSPC;
		ionic_q_post(adminq, true, cb, cb_arg);
		break;
	default:
		return -ENOTSUPP;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(ionic_adminq_rdma_cmd);

bool ionic_adminq_service(struct cq *cq, struct cq_info *cq_info,
			  void *cb_arg)
{
	struct admin_comp *comp = cq_info->cq_desc;

	if (comp->color != cq->done_color)
		return 0;

	ionic_q_service(cq->bound_q, cq_info, comp->comp_index);

	return true;
}
