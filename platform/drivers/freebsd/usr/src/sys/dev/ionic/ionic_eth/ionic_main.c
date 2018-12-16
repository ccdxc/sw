/*
 * Copyright (c) 2017-2019 Pensando Systems, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_lif.h"

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Anish Gupta <anish@pensando.io>");
MODULE_VERSION(ionic, 1);

int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx,
	bool timeout)
{
	struct net_device *netdev = lif->netdev;
	static struct cmds {
		unsigned int opcode;
		char *name;
	} cmds[] = {
		{ CMD_OPCODE_TXQ_INIT, "CMD_OPCODE_TXQ_INIT" },
		{ CMD_OPCODE_RXQ_INIT, "CMD_OPCODE_RXQ_INIT" },
		{ CMD_OPCODE_FEATURES, "CMD_OPCODE_FEATURES" },
		{ CMD_OPCODE_Q_ENABLE, "CMD_OPCODE_Q_ENABLE" },
		{ CMD_OPCODE_Q_DISABLE, "CMD_OPCODE_Q_DISABLE" },
		{ CMD_OPCODE_NOTIFYQ_INIT, "CMD_OPCODE_NOTIFYQ_INIT" },
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

	for (i = 0; i < list_len; i++) {
		if (cmd[i].opcode == ctx->cmd.cmd.opcode) {
			name = cmd[i].name;
			break;
		}
	}

	if (ctx->comp.comp.status || timeout) {
		IONIC_NETDEV_ERROR(netdev, "(%d) %s failed: %d %s\n", ctx->cmd.cmd.opcode,
			name, ctx->comp.comp.status, timeout ? "(timeout)" : "");
		return EIO;
	}

	IONIC_NETDEV_INFO(netdev, "(%d) %s done\n", ctx->cmd.cmd.opcode,
		name);

	return 0;
}

int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	int err, remaining;

	err = ionic_api_adminq_post(lif, ctx);
	if (err) {
		IONIC_NETDEV_ERROR(lif->netdev, "ionic_api_adminq_post failed, error: %d\n",
			err);
		return err;
	}

	remaining = wait_for_completion_timeout(&ctx->work, ionic_devcmd_timeout * HZ);

	err = ionic_adminq_check_err(lif, ctx, remaining == 0);

	return (err);
}

static int ionic_dev_cmd_wait(struct ionic_dev *idev, unsigned long max_wait)
{
	unsigned long time;
	int done;

	time = jiffies + max_wait;
	do {

		done = ionic_dev_cmd_done(idev);
	//	IONIC_INFO("DEVCMD wait %ld secs (%ld jiffies)\n",
	//		       (jiffies + max_wait - time)/HZ, jiffies + max_wait - time);
#ifdef HAPS
		if (done)
			IONIC_INFO("DEVCMD done took %ld secs (%ld jiffies)\n",
			       (jiffies + max_wait - time)/HZ, jiffies + max_wait - time);
#endif
		if (done)
			return 0;

#ifdef __FreeBSD__
		/* XXX: use msleep but need mtx access. */
		DELAY(1000);
#else
		schedule_timeout_uninterruptible(HZ / 10);
#endif

	} while (time_after(time, jiffies));

	IONIC_ERROR("DEVCMD timeout after %ld secs\n", max_wait / HZ);

	return ETIMEDOUT;
}

static int ionic_dev_cmd_check_error(struct ionic_dev *idev)
{
	u8 status;

	status = ionic_dev_cmd_status(idev);

	if (status) {
		IONIC_ERROR("DEVCMD(%d) failed, status: 0x%x\n",
					idev->dev_cmd->cmd.cmd.opcode, status);
		return (EIO);
	}

	return status;
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

	/* 
	 * Query system for DMA addressing limitation for the device.
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

	/* XXX: 2GB? */
	dma_set_max_seg_size(dev, 2u * 1024 * 1024 * 1024);

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
		return ENOMEM;

	ident->drv.os_type = OS_TYPE_FREEBSD;
	ident->drv.os_dist = 0;
	strncpy(ident->drv.os_dist_str, "FreeBSD",
		sizeof(ident->drv.os_dist_str) - 1);
	ident->drv.kernel_ver = __FreeBSD_version;
	snprintf(ident->drv.kernel_ver_str, sizeof(ident->drv.kernel_ver_str) - 1,
		"%d", __FreeBSD_version);
	strncpy(ident->drv.driver_ver_str, DRV_VERSION,
		sizeof(ident->drv.driver_ver_str) - 1);

#ifdef HAPS
	for (i = 0; i < 512; i++)
		iowrite32(idev->ident->words[i], &ident->words[i]);
#endif

	ionic_dev_cmd_identify(idev, IDENTITY_VERSION_1, ident_pa);

	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
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
	return ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
}

static int __init ionic_init_module(void)
{
	ionic_struct_size_checks();
	pr_info("%s, ver: %s\n", DRV_DESCRIPTION, DRV_VERSION);
	return ionic_bus_register_driver();
}

static void __exit ionic_cleanup_module(void)
{
	ionic_bus_unregister_driver();
}

module_init(ionic_init_module);
module_exit(ionic_cleanup_module);

MODULE_DEPEND(ionic, linuxkpi, 1, 1, 1);
