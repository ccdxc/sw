// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/utsname.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_lif.h"
#include "ionic_debugfs.h"

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Pensando Systems, Inc");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

unsigned int ntxq_descs = 1024;
unsigned int nrxq_descs = 1024;
module_param(ntxq_descs, uint, 0);
module_param(nrxq_descs, uint, 0);
MODULE_PARM_DESC(ntxq_descs, "Descriptors per Tx queue, must be power of 2");
MODULE_PARM_DESC(nrxq_descs, "Descriptors per Rx queue, must be power of 2");

unsigned int ntxqs;
unsigned int nrxqs;
module_param(ntxqs, uint, 0);
module_param(nrxqs, uint, 0);
MODULE_PARM_DESC(ntxqs, "Hard set the number of Tx queues per LIF");
MODULE_PARM_DESC(nrxqs, "Hard set the number of Rx queues per LIF");

unsigned int devcmd_timeout = 30;
module_param(devcmd_timeout, uint, 0);
MODULE_PARM_DESC(devcmd_timeout, "Devcmd timeout in seconds (default 30 secs)");

int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx,
			   bool timeout)
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
		{ CMD_OPCODE_STATS_DUMP_START, "CMD_OPCODE_STATS_DUMP_START" },
		{ CMD_OPCODE_STATS_DUMP_STOP, "CMD_OPCODE_STATS_DUMP_STOP" },
		{ CMD_OPCODE_RSS_HASH_SET, "CMD_OPCODE_RSS_HASH_SET" },
		{ CMD_OPCODE_RSS_INDIR_SET, "CMD_OPCODE_RSS_INDIR_SET" },
	};
	int list_len = ARRAY_SIZE(cmds);
	struct cmds *cmd = cmds;
	char *name = "UNKNOWN";
	int i;

	if (ctx->comp.comp.status || timeout) {
		for (i = 0; i < list_len; i++) {
			if (cmd[i].cmd == ctx->cmd.cmd.opcode) {
				name = cmd[i].name;
				break;
			}
		}
		netdev_err(netdev, "(%d) %s failed: %d %s\n",
			   ctx->cmd.cmd.opcode, name, ctx->comp.comp.status,
			   (timeout ? "(timeout)" : ""));
		return -EIO;
	}

	return 0;
}

int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	unsigned long remaining;
	int err;

	err = ionic_api_adminq_post(lif, ctx);
	if (err) {
		struct net_device *netdev = lif->netdev;

		netdev_err(netdev, "adminq_post cmd %d failed: %d\n",
			   ctx->cmd.cmd.opcode, err);

		return err;
	}

	remaining = wait_for_completion_timeout(&ctx->work,
						HZ * devcmd_timeout);
	return ionic_adminq_check_err(lif, ctx, (remaining == 0));
}

int ionic_napi(struct napi_struct *napi, int budget, ionic_cq_cb cb,
	       void *cb_arg)
{
	struct qcq *qcq = napi_to_qcq(napi);
	struct cq *cq = &qcq->cq;
	unsigned int work_done;

	work_done = ionic_cq_service(cq, budget, cb, cb_arg);
	if (work_done > 0)
		ionic_intr_return_credits(cq->bound_intr, work_done,
					  false, true);

	if ((work_done < budget) && napi_complete_done(napi, work_done)) {
		DEBUG_STATS_INTR_REARM(cq->bound_intr);
		ionic_intr_return_credits(cq->bound_intr, 0,
					  true, true);
	}
	DEBUG_STATS_NAPI_POLL(qcq, work_done);

	return work_done;
}

static int ionic_dev_cmd_wait(struct ionic_dev *idev, unsigned long max_wait)
{
	unsigned long time;
	signed long wait;
	int done;

	WARN_ON(in_interrupt());

	/* Wait for dev cmd to complete...but no more than max_wait
	 */

	time = jiffies + max_wait;
	do {

		done = ionic_dev_cmd_done(idev);
#ifdef HAPS
		if (done)
			pr_debug("DEVCMD %d done took %ld secs (%ld jiffies)\n",
				 idev->dev_cmd->cmd.cmd.opcode,
				 (jiffies + max_wait - time)/HZ,
				 jiffies + max_wait - time);
#endif
		if (done)
			return 0;

		wait = schedule_timeout_interruptible(HZ / 10);
		if (wait > 0)
			return -EINTR;

	} while (time_after(time, jiffies));

#ifdef HAPS
	pr_debug("DEVCMD %d timeout after %ld secs\n",
		 idev->dev_cmd->cmd.cmd.opcode,
		 max_wait/HZ);
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

	/* Query system for DMA addressing limitation for the device. */
	err = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(52));
	if (err)
		dev_err(dev, "Unable to obtain 64-bit DMA for consistent allocations, aborting.  err=%d\n",
			err);

	return err;
}

int ionic_setup(struct ionic *ionic)
{
	int err;

	err = ionic_dev_setup(&ionic->idev, ionic->bars, ionic->num_bars);
	if (err)
		return err;

	return ionic_debugfs_add_dev_cmd(ionic);
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

	ident = devm_kzalloc(dev, sizeof(*ident), GFP_KERNEL | GFP_DMA);
	if (!ident)
		return -ENOMEM;
	ident_pa = dma_map_single(dev, ident, sizeof(*ident),
				  DMA_BIDIRECTIONAL);
	if (dma_mapping_error(dev, ident_pa))
		return -EIO;

	ident->drv.os_type = OS_TYPE_LINUX;
	ident->drv.os_dist = 0;
	strncpy(ident->drv.os_dist_str, utsname()->release,
		sizeof(ident->drv.os_dist_str) - 1);
	ident->drv.kernel_ver = LINUX_VERSION_CODE;
	strncpy(ident->drv.kernel_ver_str, utsname()->version,
		sizeof(ident->drv.kernel_ver_str) - 1);
	strncpy(ident->drv.driver_ver_str, DRV_VERSION,
		sizeof(ident->drv.driver_ver_str) - 1);

#ifdef HAPS
	for (i = 0; i < 512; i++)
		iowrite32(idev->ident->words[i], &ident->words[i]);
#endif

	ionic_dev_cmd_identify(idev, IDENTITY_VERSION_1, ident_pa);

	err = ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
	if (err)
		goto err_out_unmap;

#ifdef HAPS
	for (i = 0; i < 512; i++)
		ident->words[i] = ioread32(&idev->ident->words[i]);
#endif

	ionic->ident = ident;
	ionic->ident_pa = ident_pa;

	err = ionic_debugfs_add_ident(ionic);
	if (err)
		goto err_out_unmap;

	return 0;

err_out_unmap:
	dma_unmap_single(dev, ident_pa, sizeof(*ident), DMA_BIDIRECTIONAL);
	return err;
}

void ionic_forget_identity(struct ionic *ionic)
{
	dma_unmap_single(ionic->dev, ionic->ident_pa,
			 sizeof(*ionic->ident), DMA_BIDIRECTIONAL);
}

int ionic_reset(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;

	ionic_dev_cmd_reset(idev);
	return ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
}

static int __init ionic_init_module(void)
{
	ionic_struct_size_checks();
	ionic_debugfs_create();
	pr_info("%s, ver %s\n", DRV_DESCRIPTION, DRV_VERSION);
	return ionic_bus_register_driver();
}

static void __exit ionic_cleanup_module(void)
{
	ionic_bus_unregister_driver();
	ionic_debugfs_destroy();
}

module_init(ionic_init_module);
module_exit(ionic_cleanup_module);
