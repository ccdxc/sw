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
#include <linux/slab.h>

#include "ionic_dev.h"
#include "ionic.h"

#define BAR0_SIZE					0x8000

#define BAR0_DEV_CMD_REGS_OFFSET	0x0000
#define BAR0_DEV_CMD_DB_OFFSET		0x1000
#define BAR0_INTR_STATUS_OFFSET		0x2000
#define BAR0_INTR_CTRL_OFFSET		0x3000

#define DEV_CMD_DONE				0x00000001

#define ASIC_TYPE_CAPRI				0

int ionic_dev_setup(struct ionic_dev *idev, struct ionic_dev_bar bars[],
		    unsigned int num_bars)
{
	struct ionic_dev_bar *bar = &bars[0];
	u32 sig;

	/*
	 * BAR0 resources
	 */

	if (num_bars < 1 || bar->len != BAR0_SIZE)
		return -EFAULT;

	idev->dev_cmd = bar->vaddr + BAR0_DEV_CMD_REGS_OFFSET;
	idev->dev_cmd_db = bar->vaddr + BAR0_DEV_CMD_DB_OFFSET;
	idev->intr_status = bar->vaddr + BAR0_INTR_STATUS_OFFSET;
	idev->intr_ctrl = bar->vaddr + BAR0_INTR_CTRL_OFFSET;
#ifdef HAPS
	idev->ident = bar->vaddr + 0x800;
#endif

	sig = ioread32(&idev->dev_cmd->signature);
	if (sig != DEV_CMD_SIGNATURE) {
		IONIC_ERROR("mismatch in devcmd signature[%p]: 0x%x\n",
			&idev->dev_cmd->signature, sig);
		return -EFAULT;
	}

	/*
	 * BAR1 resources
	 */

	bar++;

	idev->db_pages = bar->vaddr;
	idev->phy_db_pages = bar->bus_addr;

	/* 
	 * BAR2 resources
	 */

	mutex_init(&idev->cmb_inuse_lock);

	bar++;
	if (num_bars < 3) {
		idev->phy_cmb_pages = 0;
		idev->cmb_npages = 0;
		idev->cmb_inuse = NULL;
		return 0;
	}

	idev->phy_cmb_pages = bar->bus_addr;
	idev->cmb_npages = bar->len / PAGE_SIZE;
	idev->cmb_inuse = kzalloc(BITS_TO_LONGS(idev->cmb_npages) * sizeof(long),
				  GFP_KERNEL);
	if (!idev->cmb_inuse) {
		idev->phy_cmb_pages = 0;
		idev->cmb_npages = 0;
	}

	return 0;
}

u8 ionic_dev_cmd_status(struct ionic_dev *idev)
{
	return ioread8(&idev->dev_cmd->comp.status);
}

bool ionic_dev_cmd_done(struct ionic_dev *idev)
{
	return ioread32(&idev->dev_cmd->done) & DEV_CMD_DONE;
}

void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem)
{
	union dev_cmd_comp *comp = mem;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(comp->words); i++)
		comp->words[i] = ioread32(&idev->dev_cmd->comp.words[i]);
}

void ionic_dev_cmd_go(struct ionic_dev *idev, union dev_cmd *cmd)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cmd->words); i++)
		iowrite32(cmd->words[i], &idev->dev_cmd->cmd.words[i]);
	iowrite32(0, &idev->dev_cmd->done);
	iowrite32(1, &idev->dev_cmd_db->v);
}

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

char *ionic_dev_asic_name(u8 asic_type)
{
	switch (asic_type) {
	case ASIC_TYPE_CAPRI:
		return "Capri";
	default:
		return "Unknown";
	}
}

int ionic_db_page_num(struct ionic_dev *dev, int lif_id, int pid)
{
	return lif_id * dev->ident->dev.ndbpgs_per_lif + pid;
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

	KASSERT(intr->ctrl, ("%s intr->ctrl is NULL", intr->name));
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

	KASSERT(intr->ctrl, ("%s intr->ctrl is NULL", intr->name));
	IONIC_INFO("%s Intr credits: %d\n", intr->name, intr->ctrl->int_credits);

	iowrite32(*(u32 *)intr_to_credits(&ctrl),
		  intr_to_credits(intr->ctrl));
}

void ionic_intr_mask(struct intr *intr, bool mask)
{
	struct intr_ctrl ctrl = {
		.mask = mask ? 1 : 0,
	};

	IONIC_INFO("%s index: %d vector: %d credit: %d %s\n",
		intr->name, intr->index, intr->vector, intr->ctrl->int_credits,
		mask ? "masked" : "unmasked");

	iowrite32(*(u32 *)intr_to_mask(&ctrl),
		  intr_to_mask(intr->ctrl));
	(void)ioread32(intr_to_mask(intr->ctrl)); /* flush write */
}

void ionic_intr_coal_set(struct intr *intr, u32 intr_coal)
{
	struct intr_ctrl ctrl = {
		.coalescing_init = intr_coal > INTR_CTRL_COAL_MAX ?
			INTR_CTRL_COAL_MAX : intr_coal,
	};

	KASSERT(intr->ctrl, ("%s intr->ctrl is NULL", intr->name));

	iowrite32(*(u32 *)intr_to_coal(&ctrl), intr_to_coal(intr->ctrl));
	(void)ioread32(intr_to_coal(intr->ctrl)); /* flush write */
}

void ionic_ring_doorbell(struct doorbell *db_addr, uint32_t qid, uint16_t p_index)
{
	struct doorbell db_data = {
		.qid_lo = qid,
		.qid_hi = qid >> 8,
		.ring = 0,
		.p_index = p_index,
	};

	writeq(*(u64 *)&db_data, db_addr);
}

int ionic_desc_avail(int ndescs, int head, int tail) 
{
	int avail = tail;

	if (head >= tail)
		avail += ndescs - head - 1;
	else
		avail -= head + 1;

	return avail;
}
