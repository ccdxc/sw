/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/slab.h>

#include "osal.h"
#include "sonic.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"
#include "osal_mem.h"

int sonic_dev_setup(struct sonic_dev *idev, struct sonic_dev_bar bars[],
		    unsigned int num_bars)
{
	struct sonic_dev_bar *bar = &bars[0];
	u32 sig;

	/* BAR0: dev_cmd and interrupts */
	if (num_bars < 1) {
		OSAL_LOG_ERROR("No bars found, aborting");
		return -EFAULT;
	}

	if (bar->len < BAR0_SIZE) {
		OSAL_LOG_ERROR("Resource bar size %lu less than minimum %d, aborting",
			 bar->len, BAR0_SIZE);
		return -EFAULT;
	}

	idev->dev_info = bar->vaddr + BAR0_DEV_INFO_REGS_OFFSET;
	idev->dev_cmd = bar->vaddr + BAR0_DEV_CMD_REGS_OFFSET;
	idev->intr_status = bar->vaddr + BAR0_INTR_STATUS_OFFSET;
	idev->intr_ctrl = bar->vaddr + BAR0_INTR_CTRL_OFFSET;

	sig = ioread32(&idev->dev_info->base.signature);
	if (sig != ACCEL_DEV_CMD_SIGNATURE) {
		OSAL_LOG_ERROR("Dev cmd Sig mismatch exp 0x%x actual 0x%x",
				ACCEL_DEV_CMD_SIGNATURE, sig);
		return -EFAULT;
	}

	/* BAR1: doorbells */
	bar++;
	if (num_bars < 2) {
		OSAL_LOG_ERROR("Doorbell bar missing, aborting");
		return -EFAULT;
	}

	idev->db_pages = bar->vaddr;
	idev->phy_db_pages = bar->bus_addr;

	/* BAR2: controller memory mapping */
	spin_lock_init(&idev->hbm_inuse_lock);

	bar++;
	if (num_bars < 3) {
		idev->phy_hbm_pages = 0;
		idev->hbm_npages = 0;
		return 0;
	}

	idev->hbm_iomem_vaddr = bar->vaddr;
	idev->phy_hbm_pages = bar->bus_addr;
	idev->hbm_npages = bar->len / PAGE_SIZE;
	idev->hbm_nallocs = 0;
	idev->hbm_nfrees = 0;

	return 0;
}

void sonic_dev_teardown(struct sonic *sonic)
{
	struct sonic_dev *idev = &sonic->idev;

	idev->phy_hbm_pages = 0;
	idev->hbm_iomem_vaddr = NULL;
	idev->hbm_npages = 0;
}

u8 sonic_dev_cmd_status(struct sonic_dev *idev)
{
	return ioread8(&idev->dev_cmd->cpl.status);
}

bool sonic_dev_cmd_done(struct sonic_dev *idev)
{
	return ioread32(&idev->dev_cmd->done) & DEV_CMD_DONE;
}

void sonic_dev_cmd_cpl(struct sonic_dev *idev, void *mem)
{
	union dev_cmd_cpl *cpl = mem;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cpl->words); i++)
		cpl->words[i] = ioread32(&idev->dev_cmd->cpl.words[i]);
}

void sonic_dev_cmd_go(struct sonic_dev *idev, union dev_cmd *cmd)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cmd->words); i++)
		iowrite32(cmd->words[i], &idev->dev_cmd->cmd.words[i]);
	iowrite32(0, &idev->dev_cmd->done);
	iowrite32(1, &idev->dev_cmd->doorbell);
}

void sonic_dev_cmd_reset(struct sonic_dev *idev)
{
	union dev_cmd cmd = {
		.dev_reset.opcode = CMD_OPCODE_RESET,
		.dev_reset.type = ACCEL_DEV_TYPE_BASE,
	};

	sonic_dev_cmd_go(idev, &cmd);
}

void sonic_dev_cmd_adminq_init(struct sonic_dev *idev, struct qcq *qcq,
			       unsigned int lif_index, unsigned int intr_index)
{
	struct queue *q = &qcq->q;
	struct cq *cq = &qcq->cq;

	union dev_cmd cmd = {
		.adminq_init.opcode = CMD_OPCODE_ADMINQ_INIT,
		.adminq_init.index = q->qid,
		.adminq_init.pid = q->pid,
		.adminq_init.intr_index = intr_index,
		.adminq_init.lif_index = lif_index,
		.adminq_init.ring_size = ilog2(q->num_descs),
		.adminq_init.ring_base =
			(dma_addr_t) sonic_hostpa_to_devpa(q->base_pa),
		.adminq_init.cq_ring_base =
			(dma_addr_t) sonic_hostpa_to_devpa(cq->base_pa),
	};

	//printk(KERN_ERR "adminq_init.pid %d\n", cmd.adminq_init.pid);
	//printk(KERN_ERR "adminq_init.index %d\n", cmd.adminq_init.index);
	//printk(KERN_ERR "adminq_init.ring_base %llx\n",
	//       cmd.adminq_init.ring_base);
	//printk(KERN_ERR "adminq_init.ring_size %d\n",
	//       cmd.adminq_init.ring_size);
	sonic_dev_cmd_go(idev, &cmd);
}

void sonic_dev_cmd_hang_notify(struct sonic_dev *idev,
			       uint32_t lif_index)
{
	union dev_cmd cmd = {
		.hang_notify.opcode = CMD_OPCODE_HANG_NOTIFY,
		.hang_notify.lif_index = lif_index,
	};

	OSAL_LOG_INFO("hang_notify lif_index %u", lif_index);
	sonic_dev_cmd_go(idev, &cmd);
}

char *sonic_dev_asic_name(u8 asic_type)
{
	switch (asic_type) {
	case ASIC_TYPE_CAPRI:
		return "Capri";
	default:
		return "Unknown";
	}
}

struct doorbell __iomem *sonic_db_map(struct sonic_dev *idev, struct queue *q)
{
	struct doorbell __iomem *db;

	db = (void *)idev->db_pages + (q->pid * PAGE_SIZE);
	db += q->qtype;

	return db;
}

void sonic_intr_clean(struct intr *intr)
{
	u32 credits;

	/* clear the credits by writing the current value back */
	credits = 0xffff & ioread32(intr_to_credits(intr->ctrl));
	sonic_intr_return_credits(intr, credits, false, true);
}

int sonic_intr_init(struct sonic_dev *idev, struct intr *intr,
		    unsigned long index)
{
	intr->index = index;
	intr->ctrl = idev->intr_ctrl + index;
	sonic_intr_clean(intr);

	return 0;
}

void sonic_intr_mask_on_assertion(struct intr *intr)
{
	struct intr_ctrl ctrl = {
		.mask_on_assert = 1,
	};

	iowrite32(*(u32 *)intr_to_mask_on_assert(&ctrl),
		  intr_to_mask_on_assert(intr->ctrl));
}

void sonic_intr_return_credits(struct intr *intr, unsigned int credits,
			       bool unmask, bool reset_timer)
{
	struct intr_ctrl ctrl = {
		.int_credits = credits,
	};

	ctrl.flags |= unmask ? INTR_F_UNMASK : 0;
	ctrl.flags |= reset_timer ? INTR_F_TIMER_RESET : 0;

	iowrite32(*(u32 *)intr_to_credits(&ctrl),
		  intr_to_credits(intr->ctrl));
}

void sonic_intr_mask(struct intr *intr, bool mask)
{
	struct intr_ctrl ctrl = {
		.mask = mask ? 1 : 0,
	};

	iowrite32(*(u32 *)intr_to_mask(&ctrl),
		  intr_to_mask(intr->ctrl));
	(void)ioread32(intr_to_mask(intr->ctrl)); /* flush write */
}

void sonic_intr_coal_set(struct intr *intr, u32 intr_coal)
{
	struct intr_ctrl ctrl = {
		.coalescing_init = intr_coal > INTR_CTRL_COAL_MAX ?
			INTR_CTRL_COAL_MAX : intr_coal,
	};

	iowrite32(*(u32 *)intr_to_coal(&ctrl), intr_to_coal(intr->ctrl));
	(void)ioread32(intr_to_coal(intr->ctrl)); /* flush write */
}

static int sonic_cq_list_init(struct cq *cq)
{
	struct cq_info *cur;
	unsigned int i;

	cq->tail = cq->info;
	cq->done_color = 1;

	cur = cq->info;

	for (i = 0; i < cq->num_descs; i++) {
                cur->last = false;
		if (i + 1 == cq->num_descs) {
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

int sonic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
		  unsigned int num_descs, size_t desc_size)
{
	unsigned int ring_size;

	if (desc_size == 0 || !is_power_of_2(num_descs))
		return -EINVAL;

	ring_size = ilog2(num_descs);
	if (ring_size < 1 || ring_size > 16)
		return -EINVAL;

	cq->lif = lif;
	cq->bound_intr = intr;
	cq->num_descs = num_descs;
	cq->desc_size = desc_size;
	return sonic_cq_list_init(cq);
}

int sonic_cq_reinit(struct cq *cq)
{
	memset(cq->base, 0, cq->num_descs * cq->desc_size);
	return sonic_cq_list_init(cq);
}

void sonic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa)
{
	struct cq_info *cur;
	unsigned int i;

	cq->base = base;
	cq->base_pa = base_pa;

	for (i = 0, cur = cq->info; i < cq->num_descs; i++, cur++)
		cur->cq_desc = base + (i * cq->desc_size);
}

void sonic_cq_bind(struct cq *cq, struct queue *q)
{
	// TODO support many:1 bindings using qid as index into bound_q array
	cq->bound_q = q;
}

unsigned int sonic_cq_service(struct cq *cq, unsigned int work_to_do,
			      sonic_cq_cb cb, void *cb_arg)
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

static int sonic_q_list_init(struct queue *q)
{
	struct admin_desc_info *cur;
	unsigned int i;

	q->admin_head = q->admin_tail = q->admin_info;
	q->ref_count = 0;
	osal_atomic_init(&q->descs_inuse, 0);

	cur = q->admin_info;
	for (i = 0; i < q->num_descs; i++) {
		if (i + 1 == q->num_descs)
			cur->next = q->admin_info;
		else
			cur->next = cur + 1;
		cur->index = i;
		cur->left = q->num_descs - i;
		cur++;
	}
	return 0;
}

int sonic_q_init(struct lif *lif, struct sonic_dev *idev, struct queue *q,
		 unsigned int index, const char *base, unsigned int num_descs,
		 size_t desc_size, unsigned int pid)
{
	unsigned int ring_size;

	if (desc_size == 0 || !is_power_of_2(num_descs))
		return -EINVAL;

	ring_size = ilog2(num_descs);
	if (ring_size < 1 || ring_size > 16)
		return -EINVAL;

	if (!q->admin_info)
		return -EINVAL;

	q->lif = lif;
	q->idev = idev;
	q->qid = index;
	q->num_descs = num_descs;
	q->desc_size = desc_size;
	q->pid = pid;

	OSAL_LOG_INFO("sonic_q_init q: " PRIx64 " q->head " PRIx64 " index %d",
			(u64) q, (u64) q->admin_head, index);

	snprintf(q->name, sizeof(q->name), "%s%u", base, index);
	return sonic_q_list_init(q);
}

int sonic_q_reinit(struct queue *q)
{
	memset(q->base, 0, q->num_descs * q->desc_size);
	return sonic_q_list_init(q);
}

void sonic_q_map(struct queue *q, void *base, dma_addr_t base_pa)
{
	struct admin_desc_info *cur;
	unsigned int i;

	q->base = base;
	q->base_pa = base_pa;

	OSAL_LOG_INFO("sonic_q_map base " PRIx64 " base_pa " PRIx64,
			(u64) base, (u64) base_pa);

	for (i = 0, cur = q->admin_info; i < q->num_descs; i++, cur++)
		cur->desc = base + (i * q->desc_size);
}

struct admin_desc_info *
sonic_q_post(struct queue *q, bool ring_doorbell, admin_desc_cb cb,
		  void *cb_arg)
{
	struct admin_desc_info *desc_info = q->admin_head;

	desc_info->cb = cb;
	desc_info->cb_arg = cb_arg;
	q->admin_head = desc_info->next;

	if (ring_doorbell)
		sonic_q_ringdb(q, q->admin_head->index);
	return desc_info;
}

void sonic_q_rewind(struct queue *q, struct admin_desc_info *start)
{
	struct admin_desc_info *cur = start;

	while (cur != q->admin_head) {
		if (cur->cb)
			cur->cb(q, cur, NULL, cur->cb_arg);
		cur = cur->next;
	}

	q->admin_head = start;
}

unsigned int sonic_q_space_avail(struct queue *q)
{
	unsigned int avail = q->admin_tail->index;

	if (q->admin_head->index >= avail)
		avail += q->admin_head->left - 1;
	else
		avail -= q->admin_head->index + 1;

	return avail;
}

bool sonic_q_has_space(struct queue *q, unsigned int want)
{
	return sonic_q_space_avail(q) >= want;
}

static int sonic_seq_q_give(struct queue *q, uint32_t count)
{
	int err = PNSO_OK;

	switch (q->qgroup) {
	case STORAGE_SEQ_QGROUP_CPDC:
	case STORAGE_SEQ_QGROUP_CRYPTO:
	case STORAGE_SEQ_QGROUP_CPDC_STATUS:
	case STORAGE_SEQ_QGROUP_CRYPTO_STATUS:
		err =  sonic_accounting_atomic_give(&q->descs_inuse,
						    count);
		break;

	default:
		break;
	}

	if (err)
		OSAL_LOG_WARN("seq queue %u potential underflow",
			       q->qid);

	return err;
}

static int sonic_seq_q_take(struct queue *q, uint32_t count)
{
	int err = PNSO_OK;

	switch (q->qgroup) {
	case STORAGE_SEQ_QGROUP_CPDC:
	case STORAGE_SEQ_QGROUP_CRYPTO:
	case STORAGE_SEQ_QGROUP_CPDC_STATUS:
	case STORAGE_SEQ_QGROUP_CRYPTO_STATUS:
		err = sonic_accounting_atomic_take(&q->descs_inuse, count,
						   q->num_descs);
		break;

	default:
		break;
	}

	if (err)
		OSAL_LOG_DEBUG("seq queue %u potential overflow",
			       q->qid);

	return err;
}

void sonic_q_service(struct queue *q, struct cq_info *cq_info,
		     unsigned int stop_index)
{
	struct admin_desc_info *desc_info;

	if (q->qtype == STORAGE_SEQ_QTYPE_SQ) {
		/* stop_index actually contains a count here */
		sonic_seq_q_give(q, stop_index);
		return;
	}

	do {
		desc_info = q->admin_tail;
		if (desc_info->cb)
			desc_info->cb(q, desc_info, cq_info,
				      desc_info->cb_arg);
		q->admin_tail = q->admin_tail->next;
	} while (desc_info->index != stop_index);
}

int sonic_q_unconsume(struct queue *q, uint32_t count)
{
	int err = PNSO_OK;
	unsigned int index;

	OSAL_LOG_DEBUG("unconsume %u entries for qtype=%u qgroup=%u",
		       count, q->qtype, q->qgroup);

	if (count == 0)
		goto out;

	if (count >= q->num_descs) {
		err = EPERM;
		goto out;
	}

	if (q->qtype == STORAGE_SEQ_QTYPE_SQ) {
		err = sonic_seq_q_give(q, count);
		if (err)
			goto out;
		if (q->pindex >= count) {
			q->pindex -= count;
		} else {
			q->pindex = (q->num_descs + q->pindex) - count;
		}
		goto out;
	}

	/* rewind the head pointer */
	index = q->admin_head->index;
	if (index >= count) {
		index -= count;
	} else {
		count -= index;
		index = q->num_descs - count;
	}
	q->admin_head = q->admin_info + index;

out:
	return err;
}

void *sonic_q_consume_entry(struct queue *q, uint32_t *index)
{
	void *ptr;

	if (q->qtype == STORAGE_SEQ_QTYPE_SQ) {
		if (sonic_seq_q_take(q, 1))
			return NULL;
		ptr = q->base + (q->pindex * q->desc_size);
		q->pindex = (q->pindex + 1) % q->num_descs;
		*index = q->pindex;
		return ptr;
	}

	ptr = q->admin_head->desc;
	q->admin_head = q->admin_head->next;
	*index = q->admin_head->index;
	return ptr;
}

u64 sonic_q_ringdb_data(struct queue *q, uint32_t index)
{
	struct doorbell db = {
		.qid_lo = q->qid,
		.qid_hi = q->qid >> 8,
		.ring = 0,
		.p_index = index,
	};

	//printk(KERN_DEBUG "sonic_q_ringdb ring doorbell name %s qid %d ring 0 p_index %d db 0x%llx\n",
	//	 q->name, q->qid,
	//	 index, (u64) q->db);

	return *((u64 *) &db);
}

void sonic_q_ringdb(struct queue *q, uint32_t index)
{
	writeq(sonic_q_ringdb_data(q, index), q->db);
}


