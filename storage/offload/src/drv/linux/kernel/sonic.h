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

#ifndef _SONIC_H_
#define _SONIC_H_

#include "sonic_dev.h"
#include "sonic_debugfs.h"


#define ADMINQ			1

#define DRV_NAME		"sonic"
#define DRV_DESCRIPTION		"Pensando Offload Driver"
#define DRV_VERSION		"0.1"

extern unsigned int devcmd_timeout;

struct sonic {
	struct pci_dev *pdev;
	struct platform_device *pfdev;
	struct device *dev;
	struct sonic_dev idev;
	struct dentry *dentry;
	struct sonic_dev_bar bars[SONIC_BARS_MAX];
	unsigned int num_bars;
	union identity *ident;
	dma_addr_t ident_pa;
	struct list_head lifs;
	unsigned int num_per_core_resources;
	unsigned int nintrs;
	DECLARE_BITMAP(intrs, INTR_CTRL_REGS_MAX);
#ifdef CONFIG_DEBUG_FS
#ifdef DEBUGFS_TEST_API
	void *scratch_bufs[NUM_SCRATCH_BUFS];
	dma_addr_t scratch_bufs_pa[NUM_SCRATCH_BUFS];
	struct debugfs_blob_wrapper scratch_bufs_blob[NUM_SCRATCH_BUFS];
#endif
#endif
#ifndef ADMINQ
	spinlock_t cmd_lock;
	struct list_head cmd_list;
	struct work_struct cmd_work;
#endif
};

/** ionic_admin_ctx - Admin command context.
 * @work:   Work completion wait queue element.
 * @cmd:    Admin command (64B) to be copied to the queue.
 * @comp:   Admin completion (16B) copied from the queue.
 *
 * @side_data:    Additional data to be copied to the doorbell page,
 *        if the command is issued as a dev cmd.
 * @side_data_len:  Length of additional data to be copied.
 *
 * TODO:
 * The side_data and side_data_len are temporary and will be removed.  For now,
 * they are used when admin commands referring to side-band data are posted as
 * dev commands instead.  Only single-indirect side-band data is supported.
 * Only 2K of data is supported, because first half of page is for registers.
 */
struct sonic_admin_ctx {
  struct completion work;
  union adminq_cmd cmd;
  union adminq_cpl comp;

#ifndef ADMINQ
  struct list_head list;
#endif
  void *side_data;
  size_t side_data_len;
};

int sonic_adminq_check_err(struct lif *lif, struct sonic_admin_ctx *ctx);
int sonic_adminq_post_wait(struct lif *lif, struct sonic_admin_ctx *ctx);
int sonic_dev_cmd_wait_check(struct sonic_dev *idev, unsigned long max_wait);
int sonic_set_dma_mask(struct sonic *sonic);
int sonic_setup(struct sonic *sonic);
int sonic_identify(struct sonic *sonic);
void sonic_forget_identity(struct sonic *sonic);
int sonic_reset(struct sonic *sonic);
int sonic_api_adminq_post(struct lif *lif, struct sonic_admin_ctx *ctx);

#endif /* _SONIC_H_ */
