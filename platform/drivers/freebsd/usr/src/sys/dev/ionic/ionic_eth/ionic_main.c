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

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_lif.h"

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Scott Feldman <sfeldma@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(ionic, 1);

const char *ionic_module_dname = DRV_NAME;

int ntxq_descs = 1024;
int nrxq_descs = 1024;
module_param(ntxq_descs, uint, 0);
module_param(nrxq_descs, uint, 0);
MODULE_PARM_DESC(ntxq_descs, "Descriptors per Tx queue, must be power of 2");
MODULE_PARM_DESC(nrxq_descs, "Descriptors per Rx queue, must be power of 2");

int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	struct net_device *netdev = lif->netdev;
	static struct cmds {
		unsigned int cmd;
		char *name;
	} cmds[] = {
		{ CMD_OPCODE_TXQ_INIT, "CMD_OPCODE_TXQ_INIT" },
		{ CMD_OPCODE_RXQ_INIT, "CMD_OPCODE_RXQ_INIT" },
		{ CMD_OPCODE_FEATURES, "CMD_OPCODE_FEATURES" },
		{ CMD_OPCODE_Q_ENABLE, "CMD_OPCODE_Q_ENABLE" },
		{ CMD_OPCODE_Q_DISABLE, "CMD_OPCODE_Q_DISABLE" },
		{ CMD_OPCODE_STATION_MAC_ADDR_GET,
			"CMD_OPCODE_STATION_MAC_ADDR_GET" },
		{ CMD_OPCODE_MTU_SET, "CMD_OPCODE_MTU_SET" },
		{ CMD_OPCODE_RX_MODE_SET, "CMD_OPCODE_RX_MODE_SET" },
		{ CMD_OPCODE_RX_FILTER_ADD, "CMD_OPCODE_RX_FILTER_ADD" },
		{ CMD_OPCODE_RX_FILTER_DEL, "CMD_OPCODE_RX_FILTER_DEL" },
		{ CMD_OPCODE_RSS_HASH_SET, "CMD_OPCODE_RSS_HASH_SET" },
		{ CMD_OPCODE_RSS_INDIR_SET, "CMD_OPCODE_RSS_INDIR_SET" },
		{ CMD_OPCODE_STATS_DUMP_START, "CMD_OPCODE_STATS_DUMP_START" },
		{ CMD_OPCODE_STATS_DUMP_STOP, "CMD_OPCODE_STATS_DUMP_STOP" },
		{ 0, 0 }, /* keep last */
	};
	struct cmds *cmd = cmds;
	char *name = "UNKNOWN";

	if (ctx->comp.comp.status) {
		while ((++cmd)->cmd)
			if (cmd->cmd == ctx->cmd.cmd.opcode)
				name = cmd->name;
		netdev_err(netdev, "(%d) %s failed: %d\n", ctx->cmd.cmd.opcode,
			   name, ctx->comp.comp.status);
		return -EIO;
	}

	return 0;
}

int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	int err;

	err = ionic_api_adminq_post(lif, ctx);
	if (err)
		return err;

	wait_for_completion(&ctx->work);

	return ionic_adminq_check_err(lif, ctx);
}

int ionic_napi(struct napi_struct *napi, int budget, ionic_cq_cb cb,
	       void *cb_arg)
{
	struct cq *cq = napi_to_cq(napi);
	unsigned int work_done;

	work_done = ionic_cq_service(cq, budget, cb, cb_arg);

	if (work_done > 0)
		ionic_intr_return_credits(cq->bound_intr, work_done, 0, true);

	if ((work_done < budget) && napi_complete_done(napi, work_done))
		ionic_intr_mask(cq->bound_intr, false);

	return work_done;
}

static int ionic_dev_cmd_wait(struct ionic_dev *idev, unsigned long max_wait)
{
	unsigned long time;
	int done;

	/* Wait for dev cmd to complete...but no more than max_wait
	 */

	time = jiffies + max_wait;
	do {

		done = ionic_dev_cmd_done(idev);
#ifdef HAPS
		if (done)
			printk(KERN_ERR "DEVCMD done took %ld secs (%ld jiffies)\n",
			       (jiffies + max_wait - time)/HZ, jiffies + max_wait - time);
#endif
		if (done)
			return 0;

		schedule_timeout_uninterruptible(HZ / 10);

	} while (time_after(time, jiffies));

#ifdef HAPS
	printk(KERN_ERR "DEVCMD timeout after %ld secs\n", max_wait/HZ);
#endif
	return -ETIMEDOUT;
}

static int ionic_dev_cmd_check_error(struct ionic_dev *idev)
{
	u8 status;

	status = ionic_dev_cmd_status(idev);
	switch (status) {
	case 0:
		return 0;
	}

	return -EIO;
}

int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait)
{
	int err;

	err = ionic_dev_cmd_wait(idev, max_wait);
	if (err)
		return err;
	return ionic_dev_cmd_check_error(idev);
}

int ionic_set_dma_mask(struct ionic *ionic)
{
	struct device *dev = ionic->dev;
	int err;

	/* Query system for DMA addressing limitation for the device.
	 */

	err = dma_set_mask(dev, DMA_BIT_MASK(64));
	if (err) {
		dev_err(dev, "No usable 64-bit DMA configuration, aborting\n");
		return err;
	}

	err = dma_set_coherent_mask(dev, DMA_BIT_MASK(64));
	if (err)
		dev_err(dev, "Unable to obtain 64-bit DMA "
			"for consistent allocations, aborting\n");

	return err;
}

int ionic_setup(struct ionic *ionic)
{
	return ionic_dev_setup(&ionic->idev, ionic->bars, ionic->num_bars);
}

int ionic_identify(struct ionic *ionic)
{
	struct device *dev = ionic->dev;
	struct ionic_dev *idev = &ionic->idev;
	union identity *ident;
	dma_addr_t ident_pa;
	int err;
#ifdef HAPS
	unsigned int i;
#endif

	ident = dma_zalloc_coherent(dev, sizeof(*ident), &ident_pa, GFP_KERNEL);
	if (!ident)
		return -ENOMEM;

	ident->drv.os_type = OS_TYPE_LINUX;
	ident->drv.os_dist = 0;
	strncpy(ident->drv.os_dist_str, "FreeBSD",
		sizeof(ident->drv.os_dist_str) - 1);
	ident->drv.kernel_ver = 11;
	strncpy(ident->drv.kernel_ver_str, "11",
		sizeof(ident->drv.kernel_ver_str) - 1);
	strncpy(ident->drv.driver_ver_str, DRV_VERSION,
		sizeof(ident->drv.driver_ver_str) - 1);

#ifdef HAPS
	for (i = 0; i < 512; i++)
		iowrite32(idev->ident->words[i], &ident->words[i]);
#endif

	ionic_dev_cmd_identify(idev, IDENTITY_VERSION_1, ident_pa);

	err = ionic_dev_cmd_wait_check(idev, IONIC_DEVCMD_TIMEOUT);
	if (err)
		goto err_out_unmap;

#ifdef HAPS
	for (i = 0; i < 512; i++)
		ident->words[i] = ioread32(&idev->ident->words[i]);
#endif

	ionic->ident = ident;
	ionic->ident_pa = ident_pa;

	return 0;

err_out_unmap:
	dma_free_coherent(ionic->dev, sizeof(*ionic->ident),
			  ionic->ident, ionic->ident_pa);
	return err;
}

void ionic_forget_identity(struct ionic *ionic)
{
	dma_free_coherent(ionic->dev, sizeof(*ionic->ident),
			  ionic->ident, ionic->ident_pa);
}

int ionic_reset(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;

	ionic_dev_cmd_reset(idev);
	return ionic_dev_cmd_wait_check(idev, IONIC_DEVCMD_TIMEOUT);
}

static int __init ionic_init_module(void)
{
	ionic_struct_size_checks();
	pr_info("%s, ver %s\n", DRV_DESCRIPTION, DRV_VERSION);
	return ionic_bus_register_driver();
}

static void __exit ionic_cleanup_module(void)
{
	ionic_bus_unregister_driver();
}

module_init(ionic_init_module);
module_exit(ionic_cleanup_module);

MODULE_DEPEND(ionic, linuxkpi, 1, 1, 1);
