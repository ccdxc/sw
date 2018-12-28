/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#define LINUXKPI_PARAM_PREFIX sonic_
#include <linux/module.h>
#ifndef __FreeBSD__
#include <linux/version.h>
#include <linux/utsname.h>
#else
#include <linux/io.h>
#include <linux/printk.h>
#endif
#include <linux/netdevice.h>
#include <linux/dma-mapping.h>

#include "sonic.h"
#include "sonic_bus.h"
#include "sonic_lif.h"
#include "sonic_debugfs.h"
#include "osal_logger.h"

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Pensando");
MODULE_LICENSE("Dual BSD/GPL");
#ifndef __FreeBSD__
MODULE_VERSION(DRV_VERSION);
#else
MODULE_VERSION(sonic, 1);
#endif

unsigned int devcmd_timeout = 30;
#ifdef HAPS
module_param(devcmd_timeout, uint, 0);
MODULE_PARM_DESC(devcmd_timeout, "Devcmd timeout in seconds (default 30 secs)");
#endif

unsigned int core_count = SONIC_DEFAULT_CORES;
module_param(core_count, uint, 0444);
MODULE_PARM_DESC(core_count, "max number of cores to use for sonic driver (default=1)");

static unsigned int log_level = OSAL_LOG_LEVEL_WARNING;
module_param(log_level, uint, 0444);
MODULE_PARM_DESC(log_level, "logging level: 0=EMERG,1=ALERT,2=CRIT,3=ERR,4=WARN(default),5=NOTICE,6=INFO,7=DBG");

int body(void);

int sonic_adminq_check_err(struct lif *lif, struct sonic_admin_ctx *ctx)
{
	static struct cmds {
		unsigned int cmd;
		char *name;
	} cmds[] = {
		{ CMD_OPCODE_SEQ_QUEUE_INIT, "CMD_OPCODE_SEQ_QUEUE_INIT" },
		{ CMD_OPCODE_SEQ_QUEUE_ENABLE, "CMD_OPCODE_SEQ_QUEUE_ENABLE" },
		{ CMD_OPCODE_SEQ_QUEUE_DISABLE,
			"CMD_OPCODE_SEQ_QUEUE_DISABLE" },
		{ CMD_OPCODE_HANG_NOTIFY, "CMD_OPCODE_HANG_NOTIFY" },
		{ CMD_OPCODE_SEQ_QUEUE_DUMP, "CMD_OPCODE_SEQ_QUEUE_DUMP" },
		{ CMD_OPCODE_CRYPTO_KEY_UPDATE,
			"CMD_OPCODE_CRYPTO_KEY_UPDATE" },
		{ 0, 0 }, /* keep last */
	};
	struct cmds *cmd = cmds;
	char *name = "UNKNOWN";

	if (ctx->comp.cpl.status) {
		while ((cmd++)->cmd)
			if (cmd->cmd == ctx->cmd.cmd.opcode)
				name = cmd->name;
		OSAL_LOG_ERROR("(%d) %s failed: %d\n",
			ctx->cmd.cmd.opcode, name, ctx->comp.cpl.status);
		return -EIO;
	}

	return 0;
}

int sonic_adminq_post_wait(struct lif *lif, struct sonic_admin_ctx *ctx)
{
	int err, timeout;

	err = sonic_api_adminq_post(lif, ctx);
	if (err)
		return err;

	timeout = wait_for_completion_timeout(&ctx->work, HZ * devcmd_timeout);

	/*
	 * No ISR from device was generated, timeout condition.
	 */
	if (timeout == 0) {
		OSAL_LOG_ERROR("adminq post timeout");
		return ETIMEDOUT;
	}

	return sonic_adminq_check_err(lif, ctx);
}

int sonic_napi(struct napi_struct *napi, int budget, sonic_cq_cb cb,
	       void *cb_arg)
{
	struct cq *cq = napi_to_cq(napi);
	unsigned int work_done;

	work_done = sonic_cq_service(cq, budget, cb, cb_arg);

	if (work_done > 0) {
		sonic_intr_return_credits(cq->bound_intr, work_done,
					  false, true);
	} else {
		OSAL_LOG_ERROR("No work_done but bottom half was scheduled");
	}

	if ((work_done < budget) && napi_complete_done(napi, work_done))
		sonic_intr_mask(cq->bound_intr, false);

	return work_done;
}

static int sonic_dev_cmd_wait(struct sonic_dev *idev, unsigned long max_wait)
{
	unsigned long time;
	signed long wait;
	int done;

#ifndef __FreeBSD__
	WARN_ON(in_interrupt());
#endif

	/* Wait for dev cmd to complete...but no more than max_wait
	 */

	time = jiffies + max_wait;
	do {

		done = sonic_dev_cmd_done(idev);
#ifdef HAPS
		if (done)
			OSAL_LOG_INFO("DEVCMD done took %ld secs (%ld jiffies)\n",
			       (jiffies + max_wait - time)/HZ,
			       jiffies + max_wait - time);
#endif
		if (done)
			return 0;

		wait = schedule_timeout_interruptible(HZ / 10);

	} while (time_after(time, jiffies));

#ifdef HAPS
	OSAL_LOG_ERROR("DEVCMD timeout after %ld secs\n", max_wait/HZ);
#endif
	return -ETIMEDOUT;
}

static int sonic_dev_cmd_check_error(struct sonic_dev *idev)
{
	u8 status;

	status = sonic_dev_cmd_status(idev);
	switch (status) {
	case 0:
		return 0;
	}

	return -EIO;
}

int sonic_dev_cmd_wait_check(struct sonic_dev *idev, unsigned long max_wait)
{
	int err;

	err = sonic_dev_cmd_wait(idev, max_wait);
	if (err)
		return err;
	return sonic_dev_cmd_check_error(idev);
}

int sonic_set_dma_mask(struct sonic *sonic)
{
	struct device *dev = sonic->dev;
	int err;

	/* Query system for DMA addressing limitation for the device.
	 */

	err = dma_set_mask(dev, DMA_BIT_MASK(64));
	if (err) {
		OSAL_LOG_ERROR("No usable 64-bit DMA configuration, aborting\n");
		return err;
	}

	err = dma_set_coherent_mask(dev, DMA_BIT_MASK(64));
	if (err)
		OSAL_LOG_ERROR("Unable to obtain 64-bit DMA for consistent allocations, aborting\n");

	return err;
}

#ifndef ADMINQ
#define XXX_DEVCMD_HALF_PAGE 0x800

// XXX temp func to get side-band data from 2nd half page of dev_cmd reg space.
static int SBD_get(struct sonic_dev *idev, void *dst, size_t len)
{
	u32 __iomem *page32 = (void __iomem *)idev->dev_cmd;
	u32 *dst32 = dst;
	unsigned int i, count;

	// check pointer and size alignment
	if ((unsigned long)dst & 0x3 || len & 0x3)
		return -EINVAL;

	// check length fits in 2nd half of page
	if (len > XXX_DEVCMD_HALF_PAGE)
		return -EINVAL;

	page32 += XXX_DEVCMD_HALF_PAGE / sizeof(*page32);
	count = len / sizeof(*page32);

	for (i = 0; i < count; ++i)
		dst32[i] = ioread32(&page32[i]);

	return 0;
}

// XXX temp func to put side-band data into 2nd half page of dev_cmd reg space.
static int SBD_put(struct sonic_dev *idev, void *src, size_t len)
{
	u32 __iomem *page32 = (void __iomem *)idev->dev_cmd;
	u32 *src32 = src;
	unsigned int i, count;

	// check pointer and size alignment
	if ((unsigned long)src & 0x3 || len & 0x3)
		return -EINVAL;

	// check length fits in 2nd half of page
	if (len > XXX_DEVCMD_HALF_PAGE)
		return -EINVAL;

	page32 += XXX_DEVCMD_HALF_PAGE / sizeof(*page32);
	count = len / sizeof(*page32);

	for (i = 0; i < count; ++i)
		iowrite32(src32[i], &page32[i]);

	return 0;
}

static void sonic_dev_cmd_work(struct work_struct *work)
{
	struct sonic *sonic = container_of(work, struct sonic, cmd_work);
	struct sonic_admin_ctx *ctx;
	unsigned long irqflags;
	int err = 0;

	spin_lock_irqsave(&sonic->cmd_lock, irqflags);
	if (list_empty(&sonic->cmd_list)) {
		spin_unlock_irqrestore(&sonic->cmd_lock, irqflags);
		return;
	}

	ctx = list_first_entry(&sonic->cmd_list,
			       struct sonic_admin_ctx, list);
	list_del(&ctx->list);
	spin_unlock_irqrestore(&sonic->cmd_lock, irqflags);

	OSAL_LOG_DEBUG("post admin dev command:\n");
	if (g_osal_log_level >= OSAL_LOG_LEVEL_DEBUG) {
		print_hex_dump_debug("sonic: cmd ", DUMP_PREFIX_OFFSET, 16, 1,
						 &ctx->cmd, sizeof(ctx->cmd), true);
	}

	if (ctx->side_data) {
		if (g_osal_log_level >= OSAL_LOG_LEVEL_DEBUG) {
			dynamic_hex_dump("sonic: data ", DUMP_PREFIX_OFFSET, 16, 1,
					 ctx->side_data, ctx->side_data_len, true);
		}

		err = SBD_put(&sonic->idev, ctx->side_data, ctx->side_data_len);
		if (err)
			goto err_out;
	}

	sonic_dev_cmd_go(&sonic->idev, (void *)&ctx->cmd);

	err = sonic_dev_cmd_wait_check(&sonic->idev, HZ * devcmd_timeout);
	if (err)
		goto err_out;

	sonic_dev_cmd_comp(&sonic->idev, &ctx->comp);

	if (ctx->side_data) {
		err = SBD_get(&sonic->idev, ctx->side_data, ctx->side_data_len);
		if (err)
			goto err_out;
	}

	OSAL_LOG_DEBUG("comp admin dev command:\n");
	if (g_osal_log_level >= OSAL_LOG_LEVEL_DEBUG) {
		print_hex_dump_debug("sonic: comp ", DUMP_PREFIX_OFFSET, 16, 1,
						 &ctx->comp, sizeof(ctx->comp), true);
	}

err_out:
	if (WARN_ON(err))
		memset(&ctx->comp, 0xAB, sizeof(ctx->comp));

	complete_all(&ctx->work);

	schedule_work(&sonic->cmd_work);
}
#endif

int sonic_setup(struct sonic *sonic)
{
	int err;

	err = sonic_dev_setup(&sonic->idev, sonic->bars, sonic->num_bars);
	if (err)
		return err;

#ifndef ADMINQ
	spin_lock_init(&sonic->cmd_lock);
	INIT_LIST_HEAD(&sonic->cmd_list);
	INIT_WORK(&sonic->cmd_work, sonic_dev_cmd_work);
#endif

	return sonic_debugfs_add_dev_cmd(sonic);
}

int sonic_identify(struct sonic *sonic)
{
	struct device *dev = sonic->dev;
	struct sonic_dev *idev = &sonic->idev;
	union identity *ident;
	struct sonic_accel_ring *ring;
	dma_addr_t ident_pa;
	int err;
#ifdef HAPS
	unsigned int i;
#endif

#ifndef __FreeBSD__
	ident = devm_kzalloc(dev, sizeof(*ident), GFP_KERNEL | GFP_DMA);
#else
	/*
	 * KPI doesn't support GFP_DMA and we don't need for our hardware,
	 * should be removed in Linux also.
	 */
	ident = devm_kzalloc(dev, sizeof(*ident), GFP_KERNEL);
#endif
	if (!ident)
		return -ENOMEM;
	ident_pa = dma_map_single(dev, ident, sizeof(*ident),
				  DMA_BIDIRECTIONAL);
	if (dma_mapping_error(dev, ident_pa))
		return -EIO;

#ifndef __FreeBSD__
	ident->drv.os_type = OS_TYPE_LINUX;
	ident->drv.os_dist = 0;
	strncpy(ident->drv.os_dist_str, utsname()->release,
		sizeof(ident->drv.os_dist_str) - 1);
	ident->drv.kernel_ver = LINUX_VERSION_CODE;
	strncpy(ident->drv.kernel_ver_str, utsname()->version,
		sizeof(ident->drv.kernel_ver_str) - 1);
#else
	ident->drv.os_type = OS_TYPE_FREEBSD;
	ident->drv.os_dist = 0;
	strncpy(ident->drv.os_dist_str, "FreeBSD",
		sizeof(ident->drv.os_dist_str) - 1);
	ident->drv.kernel_ver = __FreeBSD_version;
	snprintf(ident->drv.kernel_ver_str,
			sizeof(ident->drv.kernel_ver_str) - 1,
			"%d", __FreeBSD_version);
#endif
	strncpy(ident->drv.driver_ver_str, DRV_VERSION,
		sizeof(ident->drv.driver_ver_str) - 1);

#ifdef HAPS
	for (i = 0; i < 512; i++)
		iowrite32(idev->ident->words[i], &ident->words[i]);
#endif

	sonic_dev_cmd_identify(idev, IDENTITY_VERSION_1, ident_pa);

	err = sonic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
	if (err)
		goto err_out_unmap;

#ifdef HAPS
	for (i = 0; i < 512; i++)
		ident->words[i] = ioread32(&idev->ident->words[i]);
#endif

	sonic->ident = ident;
	sonic->ident_pa = ident_pa;

	for (i = 0, ring = idev->ring_tbl;
	     i < ACCEL_RING_ID_MAX;
	     i++, ring++) {
		ring->accel_ring = ident->dev.accel_ring_tbl[i];
		ring->accel_ring.ring_id = i;
		ring->name = sonic_accel_ring_name_get(i);
		osal_atomic_init(&ring->descs_inuse, 0);
	}

	err = sonic_debugfs_add_ident(sonic);
	if (err)
		goto err_out_unmap;

	return 0;

err_out_unmap:
	dma_unmap_single(dev, ident_pa, sizeof(*ident), DMA_BIDIRECTIONAL);
	return err;
}

void sonic_forget_identity(struct sonic *sonic)
{
	dma_unmap_single(sonic->dev, sonic->ident_pa,
			 sizeof(*sonic->ident), DMA_BIDIRECTIONAL);
}

int sonic_reset(struct sonic *sonic)
{
	struct sonic_dev *idev = &sonic->idev;

	sonic_dev_cmd_reset(idev);
	return sonic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
}

static int __init sonic_init_module(void)
{
	int err;

	osal_log_init(log_level, "sonic");
	if (core_count <= 0)
		core_count = 1;
	else if (core_count > SONIC_MAX_CORES)
		core_count = SONIC_MAX_CORES;

	sonic_struct_size_checks();
	sonic_debugfs_create();
	pr_info("%s, ver %s\n", DRV_DESCRIPTION, DRV_VERSION);

	err = sonic_bus_register_driver();
	if (err)
		return err;

#ifdef ENABLE_PNSO_SONIC_TEST
	return body();
#else
	return 0;
#endif
}

static void __exit sonic_cleanup_module(void)
{
	sonic_accel_rings_sanity_check();
	sonic_bus_unregister_driver();
	sonic_debugfs_destroy();
}

static void sonic_api_adminq_cb(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct sonic_admin_ctx *ctx = cb_arg;
	struct admin_cpl *comp = (struct admin_cpl *) cq_info->cq_desc;

	if (WARN_ON(comp->cpl_index != desc_info->index)) {
		OSAL_LOG_ERROR("cpl_index  %u comp 0x" PRIx64 " desc_info_index %u desc_info 0x" PRIx64,
				comp->cpl_index, (uint64_t)comp, desc_info->index, (uint64_t)desc_info);
		ctx->comp.cpl.status = -1;
		dynamic_hex_dump("sonic: desc ", DUMP_PREFIX_OFFSET, 16, 1,
			desc_info, sizeof(*desc_info), true);
		dynamic_hex_dump("sonic: comp ", DUMP_PREFIX_OFFSET, 16, 1,
			comp, sizeof(*comp), true);
		dynamic_hex_dump("sonic: cmd ", DUMP_PREFIX_OFFSET, 16, 1,
			&ctx->cmd, sizeof(ctx->cmd), true);
		/* Completion with error. */
		complete_all(&ctx->work);
		return;
	}

	memcpy(&ctx->comp, comp, sizeof(*comp));

	OSAL_LOG_DEBUG("comp admin queue command:\n");
	if (g_osal_log_level >= OSAL_LOG_LEVEL_DEBUG) {
		dynamic_hex_dump("sonic: comp ", DUMP_PREFIX_OFFSET, 16, 1,
				&ctx->comp, sizeof(ctx->comp), true);
	}

	complete_all(&ctx->work);
}

int sonic_api_adminq_post(struct lif *lif, struct sonic_admin_ctx *ctx)
{
	struct queue *adminq = &lif->adminqcq->q;
	int err = 0;

#ifndef __FreeBSD__
	WARN_ON(in_interrupt());
#endif

	spin_lock(&lif->adminq_lock);
	if (!sonic_q_has_space(adminq, 1)) {
		err = -ENOSPC;
		goto err_out;
	}

	memcpy(adminq->head->desc, &ctx->cmd, sizeof(ctx->cmd));

	OSAL_LOG_DEBUG("post admin queue command:\n");
	if (g_osal_log_level >= OSAL_LOG_LEVEL_DEBUG) {
		dynamic_hex_dump("sonic: cmd ", DUMP_PREFIX_OFFSET, 16, 1,
				 &ctx->cmd, sizeof(ctx->cmd), true);
	}

	sonic_q_post(adminq, true, sonic_api_adminq_cb, ctx);

err_out:
	spin_unlock(&lif->adminq_lock);

	return err;
}

void sonic_dev_cmd_identify(struct sonic_dev *idev, u16 ver, dma_addr_t addr)
{
	union dev_cmd cmd = {
		.identify.opcode = CMD_OPCODE_IDENTIFY,
		.identify.ver = ver,
		.identify.addr = addr,
	};

	sonic_dev_cmd_go(idev, &cmd);
}

void sonic_dev_cmd_lif_init(struct sonic_dev *idev, u32 index)
{
	union dev_cmd cmd = {
		.lif_init.opcode = CMD_OPCODE_LIF_INIT,
		.lif_init.index = index,
	};

	sonic_dev_cmd_go(idev, &cmd);
}

int sonic_crypto_key_index_update(const void *key1,
				  const void *key2,
				  uint32_t key_size,
				  uint32_t key_index)
{
	int err;
	struct lif *lif = sonic_get_lif();
	struct sonic_admin_ctx ctx0 = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx0.work),
		.cmd.crypto_key_update = {
			.opcode = CMD_OPCODE_CRYPTO_KEY_UPDATE,
			.key_index = key_index,
			.key_size = key_size,
			.key_part = CMD_CRYPTO_KEY_PART0,
		},
	};
	struct sonic_admin_ctx ctx1 = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx1.work),
		.cmd.crypto_key_update = {
			.opcode = CMD_OPCODE_CRYPTO_KEY_UPDATE,
			.key_index = key_index,
			.key_size = key_size,
			.key_part = CMD_CRYPTO_KEY_PART1,
		},
	};

	OSAL_LOG_INFO("crypto_key_update.key_index %u\n",
		 ctx0.cmd.crypto_key_update.key_index);
	OSAL_LOG_INFO("crypto_key_update.key_size %u\n",
		 ctx0.cmd.crypto_key_update.key_size);

	switch (key_size) {

	case CMD_CRYPTO_KEY_SIZE_AES128:
		ctx0.cmd.crypto_key_update.key_type =
			CMD_CRYPTO_KEY_TYPE_AES128;
		break;

	case CMD_CRYPTO_KEY_SIZE_AES256:
		ctx0.cmd.crypto_key_update.key_type =
			CMD_CRYPTO_KEY_TYPE_AES256;
		break;

	default:
		OSAL_LOG_ERROR("invalid key_size %u\n",
			ctx0.cmd.crypto_key_update.key_size);
		return EINVAL;
	}

	memcpy(ctx0.cmd.crypto_key_update.key_data, key1, key_size);
	err = sonic_adminq_post_wait(lif, &ctx0);
	if (!err) {
		ctx1.cmd.crypto_key_update.key_type =
			  ctx0.cmd.crypto_key_update.key_type;
		ctx1.cmd.crypto_key_update.key_part = CMD_CRYPTO_KEY_PART1;
		ctx1.cmd.crypto_key_update.trigger_update = true;
		memcpy(ctx1.cmd.crypto_key_update.key_data, key2, key_size);
		err = sonic_adminq_post_wait(lif, &ctx1);
	}
	return err;
}

module_init(sonic_init_module);
module_exit(sonic_cleanup_module);
