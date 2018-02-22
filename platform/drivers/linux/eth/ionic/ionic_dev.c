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
#include <linux/errno.h>
#include <linux/io.h>

#include "ionic_dev.h"

/* BAR0 resources
 */

#define BAR0_SIZE			0x8000

#define BAR0_DEV_CMD_REGS_OFFSET	0x0000
#define BAR0_DEV_CMD_DB_OFFSET		0x1000
#define BAR0_INTR_CTRL_OFFSET		0x2000
#define BAR0_INTR_STATUS_OFFSET		0x3000

#define DEV_CMD_DONE			0x00000001

#define ASIC_TYPE_CAPRI			0

int ionic_dev_setup(struct ionic_dev *idev, struct ionic_dev_bar bars[],
		    unsigned int num_bars)
{
	struct ionic_dev_bar *bar = &bars[0];
	u32 sig;

	/* BAR0 resources
	 */

	if (num_bars < 1 || bar->len != BAR0_SIZE)
		return -EFAULT;

	idev->dev_cmd = bar->vaddr + BAR0_DEV_CMD_REGS_OFFSET;
	idev->dev_cmd_db = bar->vaddr + BAR0_DEV_CMD_DB_OFFSET;
	idev->intr_ctrl = bar->vaddr + BAR0_INTR_CTRL_OFFSET;
	idev->intr_status = bar->vaddr + BAR0_INTR_STATUS_OFFSET;

	sig = ioread32(&idev->dev_cmd->signature);
	if (sig != DEV_CMD_SIGNATURE)
		return -EFAULT;

	/* BAR1 resources
	 */

	bar++;
	if (num_bars < 2)
		return -EFAULT;

	idev->db_pages = bar->vaddr;
	idev->phy_db_pages = bar->bus_addr;

	return 0;
}

u8 ionic_dev_cmd_status(struct ionic_dev *idev)
{
	return ioread8(&idev->dev_cmd->comp.status);
}
EXPORT_SYMBOL_GPL(ionic_dev_cmd_status); //Need to remvoe later

bool ionic_dev_cmd_done(struct ionic_dev *idev)
{
	return ioread32(&idev->dev_cmd->done) & DEV_CMD_DONE;
}
EXPORT_SYMBOL_GPL(ionic_dev_cmd_done); //Need to remvoe later

void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem)
{
	union dev_cmd_comp *comp = mem;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(comp->words); i++)
		comp->words[i] = ioread32(&idev->dev_cmd->comp.words[i]);
}
EXPORT_SYMBOL_GPL(ionic_dev_cmd_comp); //Need to remvoe later

void ionic_dev_cmd_go(struct ionic_dev *idev, union dev_cmd *cmd)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cmd->words); i++)
		iowrite32(cmd->words[i], &idev->dev_cmd->cmd.words[i]);
	iowrite32(0, &idev->dev_cmd->done);
	iowrite32(1, &idev->dev_cmd_db->v);
}
EXPORT_SYMBOL_GPL(ionic_dev_cmd_go); //Need to remvoe later

void ionic_dev_cmd_reset(struct ionic_dev *idev)
{
	union dev_cmd cmd = {
		.reset.opcode = CMD_OPCODE_RESET,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_identify(struct ionic_dev *idev, u16 ver, dma_addr_t addr)
{
	union dev_cmd cmd = {
		.identify.opcode = CMD_OPCODE_IDENTIFY,
		.identify.ver = ver,
		.identify.addr = addr,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u32 index)
{
	union dev_cmd cmd = {
		.lif_init.opcode = CMD_OPCODE_LIF_INIT,
		.lif_init.index = index,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct queue *adminq,
			       unsigned int index, unsigned int lif_index,
			       unsigned int intr_index)
{
	union dev_cmd cmd = {
		.adminq_init.opcode = CMD_OPCODE_ADMINQ_INIT,
		.adminq_init.index = adminq->index,
		.adminq_init.pid = adminq->pid,
		.adminq_init.intr_index = intr_index,
		.adminq_init.lif_index = lif_index,
		.adminq_init.ring_size = ilog2(adminq->num_descs),
		.adminq_init.ring_base = adminq->base_pa,
	};

	printk(KERN_ERR "adminq_init.pid %d\n", cmd.adminq_init.pid);
	printk(KERN_ERR "adminq_init.index %d\n", cmd.adminq_init.index);
	printk(KERN_ERR "adminq_init.ring_base %llx\n",
	       cmd.adminq_init.ring_base);
	printk(KERN_ERR "adminq_init.ring_size %d\n",
	       cmd.adminq_init.ring_size);
	ionic_dev_cmd_go(idev, &cmd);
}

#ifndef ADMINQ
void ionic_dev_cmd_txq_init(struct ionic_dev *idev, struct queue *txq,
			    struct cq *cq, unsigned int cos)
{
	union dev_cmd cmd = {
		.txq_init.opcode = CMD_OPCODE_TXQ_INIT,
		.txq_init.I = false,
		.txq_init.E = false,
		.txq_init.pid = txq->pid,
		.txq_init.intr_index = cq->bound_intr->index,
		.txq_init.type = TXQ_TYPE_ETHERNET,
		.txq_init.index = txq->index,
		.txq_init.cos = cos,
		.txq_init.ring_base = txq->base_pa,
		.txq_init.ring_size = ilog2(txq->num_descs),
	};

	printk(KERN_ERR "txq_init.pid %d\n", cmd.txq_init.pid);
	printk(KERN_ERR "txq_init.index %d\n", cmd.txq_init.index);
	printk(KERN_ERR "txq_init.ring_base %llx\n", cmd.txq_init.ring_base);
	printk(KERN_ERR "txq_init.ring_size %d\n", cmd.txq_init.ring_size);
	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_rxq_init(struct ionic_dev *idev, struct queue *rxq,
			    struct cq *cq)
{
	union dev_cmd cmd = {
		.rxq_init.opcode = CMD_OPCODE_RXQ_INIT,
		.rxq_init.I = false,
		.rxq_init.E = false,
		.rxq_init.pid = rxq->pid,
		.rxq_init.intr_index = cq->bound_intr->index,
		.rxq_init.type = RXQ_TYPE_ETHERNET,
		.rxq_init.index = rxq->index,
		.rxq_init.ring_base = rxq->base_pa,
		.rxq_init.ring_size = ilog2(rxq->num_descs),
	};

	printk(KERN_ERR "rxq_init.pid %d\n", cmd.rxq_init.pid);
	printk(KERN_ERR "rxq_init.index %d\n", cmd.rxq_init.index);
	printk(KERN_ERR "rxq_init.ring_base %llx\n", cmd.rxq_init.ring_base);
	printk(KERN_ERR "rxq_init.ring_size %d\n", cmd.rxq_init.ring_size);
	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_q_enable(struct ionic_dev *idev, struct queue *q)
{
	union dev_cmd cmd = {
		.q_enable.opcode = CMD_OPCODE_Q_ENABLE,
		.q_enable.qid = q->qid,
		.q_enable.qtype = q->qtype,
	};

	printk(KERN_ERR "q_enable.qid %d\n", cmd.q_enable.qid);
	printk(KERN_ERR "q_enable.qtype %d\n", cmd.q_enable.qtype);
	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_q_disable(struct ionic_dev *idev, struct queue *q)
{
	union dev_cmd cmd = {
		.q_disable.opcode = CMD_OPCODE_Q_DISABLE,
		.q_disable.qid = q->qid,
		.q_disable.qtype = q->qtype,
	};

	printk(KERN_ERR "q_disable.qid %d\n", cmd.q_disable.qid);
	printk(KERN_ERR "q_disable.qtype %d\n", cmd.q_disable.qtype);
	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_station_get(struct ionic_dev *idev)
{
	union dev_cmd cmd = {
		.station_mac_addr_get.opcode = CMD_OPCODE_STATION_MAC_ADDR_GET,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_mtu_set(struct ionic_dev *idev, unsigned int mtu)
{
	union dev_cmd cmd = {
		.mtu_set.opcode = CMD_OPCODE_MTU_SET,
		.mtu_set.mtu = mtu,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_rx_mode_set(struct ionic_dev *idev, unsigned int rx_mode)
{
	union dev_cmd cmd = {
		.rx_mode_set.opcode = CMD_OPCODE_RX_MODE_SET,
		.rx_mode_set.rx_mode = rx_mode,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_rx_filter_vlan(struct ionic_dev *idev, u16 vlan, bool add)
{
	union dev_cmd cmd = {
		.rx_filter.opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
					  CMD_OPCODE_RX_FILTER_DEL,
		.rx_filter.match = RX_FILTER_MATCH_VLAN,
		.rx_filter.vlan = vlan,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_rx_filter_mac(struct ionic_dev *idev, const u8 *addr,
				 bool add)
{
	union dev_cmd cmd = {
		.rx_filter.opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
					  CMD_OPCODE_RX_FILTER_DEL,
		.rx_filter.match = RX_FILTER_MATCH_MAC,
	};

	memcpy(&cmd.rx_filter.addr, addr, sizeof(cmd.rx_filter.addr));
	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_rx_filter_macvlan(struct ionic_dev *idev, struct queue *q,
				     const u8 *addr, u16 vlan, bool add)
{
	union dev_cmd cmd = {
		.rx_filter.opcode = add ? CMD_OPCODE_RX_FILTER_ADD :
					  CMD_OPCODE_RX_FILTER_DEL,
		.rx_filter.match = RX_FILTER_MATCH_MAC_VLAN,
		.rx_filter.qid = q->qid,
		.rx_filter.qtype = q->qtype,
		.rx_filter.vlan = vlan,
	};

	memcpy(&cmd.rx_filter.addr, addr, sizeof(cmd.rx_filter.addr));
	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_features(struct ionic_dev *idev, u16 set)
{
	union dev_cmd cmd = {
		.features.opcode = CMD_OPCODE_FEATURES,
		.features.set = set,
	};

	ionic_dev_cmd_go(idev, &cmd);
}
#endif

char *ionic_dev_asic_name(u8 asic_type)
{
	switch (asic_type) {
	case ASIC_TYPE_CAPRI:
		return "Capri";
	default:
		return "Unknown";
	}
}

struct doorbell __iomem *ionic_db_map(struct ionic_dev *idev, struct queue *q)
{
	struct doorbell __iomem *db;

	db = (void *)idev->db_pages + (q->pid * PAGE_SIZE);
	db += q->qtype;

	return db;
}

int ionic_intr_init(struct ionic_dev *idev, struct intr *intr,
		    unsigned long index)
{
	intr->index = index;
	intr->ctrl = idev->intr_ctrl + index;

	return 0;
}

void ionic_intr_mask_on_assertion(struct intr *intr)
{
	struct intr_ctrl ctrl = {
		.mask_on_assert = 1,
	};

	iowrite32(*(u32 *)intr_to_mask_on_assert(&ctrl),
		  intr_to_mask_on_assert(intr->ctrl));
}

void ionic_intr_return_credits(struct intr *intr, unsigned int credits,
			       bool unmask, bool reset_timer)
{
	struct intr_ctrl ctrl = {
		.int_credits = credits,
		.unmask = unmask,
		.coal_timer_reset = reset_timer,
	};

	iowrite32(*(u32 *)intr_to_credits(&ctrl),
		  intr_to_credits(intr->ctrl));
}

void ionic_intr_mask(struct intr *intr, bool mask)
{
	struct intr_ctrl ctrl = {
		.mask = mask ? 1 : 0,
	};

	iowrite32(*(u32 *)intr_to_mask(&ctrl),
		  intr_to_mask(intr->ctrl));
	(void)ioread32(intr_to_mask(intr->ctrl)); /* flush write */
}

int ionic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
		  unsigned int num_descs, size_t desc_size)
{
	struct cq_info *cur;
	unsigned int ring_size;
	unsigned int i;

	if (desc_size == 0 || !is_power_of_2(num_descs))
		return -EINVAL;

	ring_size = ilog2(num_descs);
	if (ring_size < 2 || ring_size > 16)
		return -EINVAL;

	cq->lif = lif;
	cq->bound_intr = intr;
	cq->num_descs = num_descs;
	cq->desc_size = desc_size;
	cq->tail = cq->info;
	cq->done_color = 1;

	cur = cq->info;

	for (i = 0; i < num_descs; i++) {
		if (i + 1 == num_descs) {
			cur->next = cq->info;
			cur->last = true;
		} else {
			cur->next = cur + 1;
		}
		cur->index = i;
		cur++;
	}

	return 0;
}

void ionic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa)
{
	struct cq_info *cur;
	unsigned int i;

	cq->base = base;
	cq->base_pa = base_pa;

	for (i = 0, cur = cq->info; i < cq->num_descs; i++, cur++)
		cur->cq_desc = base + (i * cq->desc_size);
}

void ionic_cq_bind(struct cq *cq, struct queue *q)
{
	// TODO support many:1 bindings using qid as index into bound_q array
	cq->bound_q = q;
}

unsigned int ionic_cq_service(struct cq *cq, unsigned int work_to_do,
			      ionic_cq_cb cb, void *cb_arg)
{
	unsigned int work_done = 0;

	if (work_to_do == 0)
		return 0;

	while (cb(cq, cq->tail, cb_arg)) {
		if (cq->tail->last)
			cq->done_color = !cq->done_color;
		cq->tail = cq->tail->next;
		if (++work_done == work_to_do)
			break;
	}

	return work_done;
}

int ionic_q_init(struct lif *lif, struct ionic_dev *idev, struct queue *q,
		 unsigned int index, const char *base, unsigned int num_descs,
		 size_t desc_size, size_t sg_desc_size, unsigned int pid)
{
	struct desc_info *cur;
	unsigned int ring_size;
	unsigned int i;

	if (desc_size == 0 || !is_power_of_2(num_descs))
		return -EINVAL;

	ring_size = ilog2(num_descs);
	if (ring_size < 2 || ring_size > 16)
		return -EINVAL;

	q->lif = lif;
	q->idev = idev;
	q->index = index;
	q->num_descs = num_descs;
	q->desc_size = desc_size;
	q->sg_desc_size = sg_desc_size;
	q->head = q->tail = q->info;
	q->pid = pid;

	snprintf(q->name, sizeof(q->name), "%s%u", base, index);

	cur = q->info;

	for (i = 0; i < num_descs; i++) {
		if (i + 1 == num_descs)
			cur->next = q->info;
		else
			cur->next = cur + 1;
		cur->index = i;
		cur->left = num_descs - i;
		cur++;
	}

	return 0;
}

void ionic_q_map(struct queue *q, void *base, dma_addr_t base_pa)
{
	struct desc_info *cur;
	unsigned int i;

	q->base = base;
	q->base_pa = base_pa;

	for (i = 0, cur = q->info; i < q->num_descs; i++, cur++)
		cur->desc = base + (i * q->desc_size);
}

void ionic_q_sg_map(struct queue *q, void *base, dma_addr_t base_pa)
{
	struct desc_info *cur;
	unsigned int i;

	q->sg_base = base;
	q->sg_base_pa = base_pa;

	for (i = 0, cur = q->info; i < q->num_descs; i++, cur++)
		cur->sg_desc = base + (i * q->sg_desc_size);
}

void ionic_q_post(struct queue *q, bool ring_doorbell, desc_cb cb,
		  void *cb_arg)
{
	q->head->cb = cb;
	q->head->cb_arg = cb_arg;
	q->head = q->head->next;

	if (ring_doorbell) {
		struct doorbell db = {
			.qid_lo = q->qid,
			.qid_hi = q->qid >> 8,
			.ring = 0,
			.p_index = q->head->index,
		};

		//printk(KERN_ERR "XXXX  ring doorbell name %s qid %d ring "
		//	 "0 p_index %d db %p\n", q->name, q->qid,
		//	 q->head->index, q->db);
		writeq(*(u64 *)&db, q->db);
	}
}

void ionic_q_rewind(struct queue *q, struct desc_info *start)
{
	struct desc_info *cur = start;

	while (cur != q->head) {
		if (cur->cb)
			cur->cb(q, cur, NULL, cur->cb_arg);
		cur = cur->next;
	}

	q->head = start;
}

unsigned int ionic_q_space_avail(struct queue *q)
{
	unsigned int avail = q->tail->index;

	if (q->head->index >= avail)
		avail += q->head->left - 1;
	else
		avail -= q->head->index + 1;

	return avail;
}

bool ionic_q_has_space(struct queue *q, unsigned int want)
{
	return ionic_q_space_avail(q) >= want;
}

void ionic_q_service(struct queue *q, struct cq_info *cq_info,
		     unsigned int stop_index)
{
	struct desc_info *desc_info;

	do {
		desc_info = q->tail;
		if (desc_info->cb)
			desc_info->cb(q, desc_info, cq_info,
				      desc_info->cb_arg);
		q->tail = q->tail->next;
	} while (desc_info->index != stop_index);
}
