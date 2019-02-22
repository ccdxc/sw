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

unsigned int rx_copybreak = IONIC_RX_COPYBREAK_DEFAULT;
module_param(rx_copybreak, uint, 0);
MODULE_PARM_DESC(rx_copybreak,
	"Maximum size of packet that is copied to a bounce buffer on RX");

unsigned int devcmd_timeout = 30;
module_param(devcmd_timeout, uint, 0);
MODULE_PARM_DESC(devcmd_timeout, "Devcmd timeout in seconds (default 30 secs)");

static const char *ionic_error_to_str(enum status_code code)
{
	switch (code) {
	case IONIC_RC_SUCCESS:
		return "IONIC_RC_SUCCESS";
	case IONIC_RC_EVERSION:
		return "IONIC_RC_EVERSION";
	case IONIC_RC_EOPCODE:
		return "IONIC_RC_EOPCODE";
	case IONIC_RC_EIO:
		return "IONIC_RC_EIO";
	case IONIC_RC_EPERM:
		return "IONIC_RC_EPERM";
	case IONIC_RC_EQID:
		return "IONIC_RC_EQID";
	case IONIC_RC_EQTYPE:
		return "IONIC_RC_EQTYPE";
	case IONIC_RC_ENOENT:
		return "IONIC_RC_ENOENT";
	case IONIC_RC_EINTR:
		return "IONIC_RC_EINTR";
	case IONIC_RC_EAGAIN:
		return "IONIC_RC_EAGAIN";
	case IONIC_RC_ENOMEM:
		return "IONIC_RC_ENOMEM";
	case IONIC_RC_EFAULT:
		return "IONIC_RC_EFAULT";
	case IONIC_RC_EBUSY:
		return "IONIC_RC_EBUSY";
	case IONIC_RC_EEXIST:
		return "IONIC_RC_EEXIST";
	case IONIC_RC_EINVAL:
		return "IONIC_RC_EINVAL";
	case IONIC_RC_ENOSPC:
		return "IONIC_RC_ENOSPC";
	case IONIC_RC_ERANGE:
		return "IONIC_RC_ERANGE";
	case IONIC_RC_BAD_ADDR:
		return "IONIC_RC_BAD_ADDR";
	case IONIC_RC_DEV_CMD:
		return "IONIC_RC_DEV_CMD";
	case IONIC_RC_ERROR:
		return "IONIC_RC_ERROR";
	case IONIC_RC_ERDMA:
		return "IONIC_RC_ERDMA";
	default:
		return "IONIC_RC_UNKNOWN";
	}
}

static const char *ionic_opcode_to_str(enum cmd_opcode opcode)
{
	switch (opcode) {
	case CMD_OPCODE_NOP:
		return "CMD_OPCODE_NOP";
	case CMD_OPCODE_RESET:
		return "CMD_OPCODE_RESET";
	case CMD_OPCODE_IDENTIFY:
		return "CMD_OPCODE_IDENTIFY";
	case CMD_OPCODE_LIF_INIT:
		return "CMD_OPCODE_LIF_INIT";
	case CMD_OPCODE_LIF_RESET:
		return "CMD_OPCODE_LIF_RESET";
	case CMD_OPCODE_ADMINQ_INIT:
		return "CMD_OPCODE_ADMINQ_INIT";
	case CMD_OPCODE_TXQ_INIT:
		return "CMD_OPCODE_TXQ_INIT";
	case CMD_OPCODE_RXQ_INIT:
		return "CMD_OPCODE_RXQ_INIT";
	case CMD_OPCODE_FEATURES:
		return "CMD_OPCODE_FEATURES";
	case CMD_OPCODE_HANG_NOTIFY:
		return "CMD_OPCODE_HANG_NOTIFY";
	case CMD_OPCODE_Q_ENABLE:
		return "CMD_OPCODE_Q_ENABLE";
	case CMD_OPCODE_Q_DISABLE:
		return "CMD_OPCODE_Q_DISABLE";
	case CMD_OPCODE_NOTIFYQ_INIT:
		return "CMD_OPCODE_NOTIFYQ_INIT";
	case CMD_OPCODE_STATION_MAC_ADDR_GET:
		return "CMD_OPCODE_STATION_MAC_ADDR_GET";
	case CMD_OPCODE_MTU_SET:
		return "CMD_OPCODE_MTU_SET";
	case CMD_OPCODE_RX_MODE_SET:
		return "CMD_OPCODE_RX_MODE_SET";
	case CMD_OPCODE_RX_FILTER_ADD:
		return "CMD_OPCODE_RX_FILTER_ADD";
	case CMD_OPCODE_RX_FILTER_DEL:
		return "CMD_OPCODE_RX_FILTER_DEL";
	case CMD_OPCODE_LIF_STATS_START:
		return "CMD_OPCODE_LIF_STATS_START";
	case CMD_OPCODE_LIF_STATS_STOP:
		return "CMD_OPCODE_LIF_STATS_STOP";
	case CMD_OPCODE_DEBUG_Q_DUMP:
		return "CMD_OPCODE_DEBUG_Q_DUMP";
	case CMD_OPCODE_RSS_HASH_SET:
		return "CMD_OPCODE_RSS_HASH_SET";
	case CMD_OPCODE_RSS_INDIR_SET:
		return "CMD_OPCODE_RSS_INDIR_SET";
	case CMD_OPCODE_RDMA_RESET_LIF:
		return "CMD_OPCODE_RDMA_RESET_LIF";
	case CMD_OPCODE_RDMA_CREATE_EQ:
		return "CMD_OPCODE_RDMA_CREATE_EQ";
	case CMD_OPCODE_RDMA_CREATE_CQ:
		return "CMD_OPCODE_RDMA_CREATE_CQ";
	case CMD_OPCODE_RDMA_CREATE_ADMINQ:
		return "CMD_OPCODE_RDMA_CREATE_ADMINQ";
	default:
		return "DEVCMD_UNKNOWN";
	}
}

int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx,
			   bool timeout)
{
	struct net_device *netdev = lif->netdev;
	const char *name;

	if (ctx->comp.comp.status || timeout) {
		name = ionic_opcode_to_str(ctx->cmd.cmd.opcode);
		netdev_err(netdev, "%s (%d) failed: %d %s\n",
			   name, ctx->cmd.cmd.opcode, ctx->comp.comp.status,
			   (timeout ? "(timeout)" : ""));
		return -EIO;
	}

	return 0;
}

int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	struct net_device *netdev = lif->netdev;
	unsigned long remaining;
	const char *name;
	int err;

	err = ionic_api_adminq_post(lif, ctx);
	if (err) {
		name = ionic_opcode_to_str(ctx->cmd.cmd.opcode);
		netdev_err(netdev, "Posting of %s (%d) failed: %d\n",
			   name, ctx->cmd.cmd.opcode, err);
		return err;
	}

	remaining = wait_for_completion_timeout(&ctx->work,
						HZ * devcmd_timeout);
	return ionic_adminq_check_err(lif, ctx, (remaining == 0));
}

int ionic_napi(struct napi_struct *napi, int budget, ionic_cq_cb cb,
	       ionic_cq_done_cb done_cb, void *done_arg)
{
	struct qcq *qcq = napi_to_qcq(napi);
	struct cq *cq = &qcq->cq;
	unsigned int work_done;

	work_done = ionic_cq_service(cq, budget, cb, done_cb, done_arg);
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

int ionic_dev_cmd_wait(struct ionic *ionic, unsigned long max_seconds)
{
	struct ionic_dev *idev = &ionic->idev;
	unsigned long max_wait, start_time, duration;
	int done;
	int err;

	WARN_ON(in_interrupt());

	/* Wait for dev cmd to complete, retrying if we get EAGAIN,
	 * but don't wait any longer than max_seconds.
	 */
	max_wait = jiffies + (max_seconds * HZ);
try_again:
	start_time = jiffies;
	do {
		done = ionic_dev_cmd_done(idev);
		if (done)
			break;
		msleep(10);
	} while (!done && time_before(jiffies, max_wait));
	duration = jiffies - start_time;

	dev_dbg(ionic->dev,
		 "DEVCMD %s (%d) done took %ld secs (%ld jiffies)\n",
		 ionic_opcode_to_str(idev->dev_cmd->cmd.cmd.opcode),
		 idev->dev_cmd->cmd.cmd.opcode,
		 duration/HZ, duration);

	if (!done && time_after(jiffies, max_wait)) {
		dev_warn(ionic->dev, "DEVCMD %s (%d) timeout after %ld secs\n",
			 ionic_opcode_to_str(idev->dev_cmd->cmd.cmd.opcode),
			 idev->dev_cmd->cmd.cmd.opcode, max_seconds);
		return -ETIMEDOUT;
	}

	err = ionic_dev_cmd_status(&ionic->idev);
	if (err) {
		if (err == IONIC_RC_EAGAIN && !time_after(jiffies, max_wait)) {
			dev_err(ionic->dev, "DEV_CMD %s (%d) error, %s (%d) retrying...\n",
				ionic_opcode_to_str(idev->dev_cmd->cmd.cmd.opcode),
				idev->dev_cmd->cmd.cmd.opcode,
				ionic_error_to_str(err), err);

			msleep(1000);
			iowrite32(0, &idev->dev_cmd->done);
			iowrite32(1, &idev->dev_cmd_db->v);
			goto try_again;
		}

		dev_err(ionic->dev, "DEV_CMD %s (%d) error, %s (%d) failed\n",
			ionic_opcode_to_str(idev->dev_cmd->cmd.cmd.opcode),
			idev->dev_cmd->cmd.cmd.opcode,
			ionic_error_to_str(err), err);

		return -EIO;
	}

	return 0;
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

	err = ionic_dev_setup(ionic);
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

	mutex_lock(&ionic->dev_cmd_lock);
	ionic_dev_cmd_identify(idev, IDENTITY_VERSION_1, ident_pa);
	err = ionic_dev_cmd_wait(ionic, devcmd_timeout);
	mutex_unlock(&ionic->dev_cmd_lock);
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
	ionic->ident = NULL;
	ionic->ident_pa = 0;
}

int ionic_reset(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	int err;

	mutex_lock(&ionic->dev_cmd_lock);
	ionic_dev_cmd_reset(idev);
	err = ionic_dev_cmd_wait(ionic, devcmd_timeout);
	mutex_unlock(&ionic->dev_cmd_lock);

	return err;
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
	pr_info("%s removed\n", DRV_NAME);
}

module_init(ionic_init_module);
module_exit(ionic_cleanup_module);
