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
#include "pnso_batch.h"

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Pensando");
MODULE_LICENSE("Dual BSD/GPL");
#ifndef __FreeBSD__
MODULE_VERSION(DRV_VERSION);
#else
MODULE_VERSION(sonic, 1);
#endif

/* 60 seconds for firmware update */
unsigned int devcmd_timeout = 60;
module_param(devcmd_timeout, uint, 0);
MODULE_PARM_DESC(devcmd_timeout, "Devcmd timeout in seconds (default 60 secs)");

unsigned int core_count = 0;
module_param(core_count, uint, 0444);
MODULE_PARM_DESC(core_count, "max number of cores to use for sonic driver (default=#system CPU cores, capped at 32)");

module_param_named(log_level, g_osal_log_level, uint, 0644);
MODULE_PARM_DESC(log_level, "logging level: 0=EMERG,1=ALERT,2=CRIT,3=ERR,4=WARN(default),5=NOTICE,6=INFO,7=DBG");

bool rate_limit_src_en = true;
module_param(rate_limit_src_en, bool, 0444);
MODULE_PARM_DESC(rate_limit_src_en, "Source rate limiting enable (default=true)");

bool rate_limit_dst_en = false;
module_param(rate_limit_dst_en, bool, 0444);
MODULE_PARM_DESC(rate_limit_dst_en, "Destination rate limiting enable (default=false)");

bool chain_rate_limit_src_en = true;
module_param(chain_rate_limit_src_en, bool, 0444);
MODULE_PARM_DESC(chain_rate_limit_src_en, "Chaining source rate limiting enable (default=true)");

bool chain_rate_limit_dst_en = false;
module_param(chain_rate_limit_dst_en, bool, 0444);
MODULE_PARM_DESC(chain_rate_limit_dst_en, "Chaining destination rate limiting enable (default=false)");

unsigned int interm_buf_size = 8192;
module_param(interm_buf_size, uint, 0444);
MODULE_PARM_DESC(interm_buf_size, "Intermediate buffer size (default=8KB)");

bool error_reset_recovery_en = true;
module_param(error_reset_recovery_en, bool, 0444);
MODULE_PARM_DESC(error_reset_recovery_en, "Enable hardware error reset recovery (default=true)");

int body(void);

static const char *sonic_error_str_table[] = {
    ACCEL_RC_STR_TABLE
};

static const char *sonic_error_to_str(enum accel_status_code code)
{
	if (code < ARRAY_SIZE(sonic_error_str_table))
		return sonic_error_str_table[code];
	return "ACCEL_RC_UNKNOWN";
}

static const char *sonic_opcode_to_str(enum cmd_opcode opcode)
{
	switch (opcode) {

	ACCEL_DEVCMD_OPCODE_CASE_TABLE
	default:
		return "DEVCMD_UNKNOWN";
	}
}

int sonic_adminq_check_err(struct lif *lif, struct sonic_admin_ctx *ctx)
{
	if (ctx->cpl.status) {
		if (ctx->cpl.status == ACCEL_RC_EOPCODE)
		       return SONIC_DEVCMD_UNKNOWN;

		OSAL_LOG_ERROR("(%d) %s failed: %d", ctx->cmd.cmd.opcode,
				sonic_opcode_to_str(ctx->cmd.cmd.opcode),
				ctx->cpl.status);
		return SONIC_DEVCMD_ERROR;
	}

	return 0;
}

int sonic_adminq_post_wait(struct lif *lif, struct sonic_admin_ctx *ctx)
{
	struct admin_desc_info *desc_info;
	int err, timeout;

	err = sonic_api_adminq_post(lif, ctx, &desc_info);
	if (err)
		return err;

	timeout = wait_for_completion_timeout(&ctx->work, HZ * devcmd_timeout);

	/*
	 * No ISR from device was generated, timeout condition.
	 */
	if (timeout == 0) {
		OSAL_LOG_ERROR("adminq post timeout");
		desc_info->cb = NULL;
		return ETIMEDOUT;
	}

	return sonic_adminq_check_err(lif, ctx);
}

int sonic_napi(struct napi_struct *napi, int budget, sonic_cq_cb cb,
	       void *cb_arg)
{
	struct cq *cq = napi_to_cq(napi);
	unsigned int work_done;
	bool unmask = false;

	work_done = sonic_cq_service(cq, budget, cb, cb_arg);

	if ((work_done < budget) && napi_complete_done(napi, work_done))
		unmask = true;

	if (work_done || unmask)
		sonic_intr_return_credits(cq->bound_intr, work_done,
					  unmask, true);
	return work_done;
}

static int sonic_dev_cmd_wait(struct sonic_dev *idev, unsigned long max_seconds)
{
	unsigned long max_wait, start_time, duration;
	int done;
	int err;

#ifndef __FreeBSD__
	WARN_ON(in_interrupt());
#endif

	/* Wait for dev cmd to complete, retrying if we get EAGAIN,
	 * but don't wait any longer than max_seconds.
	 */
	max_wait = jiffies + (max_seconds * HZ);
try_again:
	start_time = jiffies;
	do {
		done = sonic_dev_cmd_done(idev);
		if (done)
			break;
		schedule_timeout_interruptible(HZ / 10);
	} while (!done && time_before(jiffies, max_wait));
	duration = jiffies - start_time;

	OSAL_LOG_INFO(
		 "DEVCMD %s (%d) done=%d took %ld secs (%ld jiffies)",
		 sonic_opcode_to_str(idev->dev_cmd->cmd.cmd.opcode),
		 idev->dev_cmd->cmd.cmd.opcode,
		 done, duration/HZ, duration);

	if (!done && !time_before(jiffies, max_wait)) {
		OSAL_LOG_WARN("DEVCMD %s (%d) timeout after %ld secs",
			 sonic_opcode_to_str(idev->dev_cmd->cmd.cmd.opcode),
			 idev->dev_cmd->cmd.cmd.opcode, max_seconds);
		return -ETIMEDOUT;
	}

	err = sonic_dev_cmd_status(idev);
	if (err) {
		if (err == ACCEL_RC_EAGAIN && !time_after(jiffies, max_wait)) {
			OSAL_LOG_DEBUG("DEV_CMD %s (%d) error, %s (%d) retrying...",
				sonic_opcode_to_str(idev->dev_cmd->cmd.cmd.opcode),
				idev->dev_cmd->cmd.cmd.opcode,
				sonic_error_to_str(err), err);

                        schedule_timeout_interruptible(HZ / 10);
			iowrite32(0, &idev->dev_cmd->done);
			iowrite32(1, &idev->dev_cmd->doorbell);
			goto try_again;
		}

		OSAL_LOG_ERROR("DEV_CMD %s (%d) error, %s (%d) failed",
			sonic_opcode_to_str(idev->dev_cmd->cmd.cmd.opcode),
			idev->dev_cmd->cmd.cmd.opcode,
			sonic_error_to_str(err), err);

		return -EIO;
	}

	return 0;
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

int sonic_dev_cmd_wait_check(struct sonic_dev *idev, unsigned long max_seconds)
{
	int err;

	err = sonic_dev_cmd_wait(idev, max_seconds);
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
		OSAL_LOG_ERROR("No usable 64-bit DMA configuration, aborting");
		return err;
	}

	err = dma_set_coherent_mask(dev, DMA_BIT_MASK(64));
	if (err)
		OSAL_LOG_ERROR("Unable to obtain 64-bit DMA for consistent allocations, aborting");

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

	OSAL_LOG_DEBUG("post admin dev command:");
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

	err = sonic_dev_cmd_wait_check(&sonic->idev, devcmd_timeout);
	if (err)
		goto err_out;

	sonic_dev_cmd_cpl(&sonic->idev, &ctx->cpl);

	if (ctx->side_data) {
		err = SBD_get(&sonic->idev, ctx->side_data, ctx->side_data_len);
		if (err)
			goto err_out;
	}

	OSAL_LOG_DEBUG("cpl admin dev command:");
	if (g_osal_log_level >= OSAL_LOG_LEVEL_DEBUG) {
		print_hex_dump_debug("sonic: cpl ", DUMP_PREFIX_OFFSET, 16, 1,
						 &ctx->cpl, sizeof(ctx->cpl), true);
	}

err_out:
	if (WARN_ON(err))
		memset(&ctx->cpl, 0xAB, sizeof(ctx->cpl));

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
	struct sonic_dev *idev = &sonic->idev;
	struct identity *ident = &sonic->ident;
	struct sonic_accel_ring *ring;
	uint32_t *wp;
	unsigned int nwords;
	unsigned int i;
	int err;

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

	nwords = min(ARRAY_SIZE(ident->drv.words),
		     ARRAY_SIZE(idev->dev_cmd->data));
	for (i = 0; i < nwords; i++)
		iowrite32(ident->drv.words[i], &idev->dev_cmd->data[i]);

	sonic_dev_cmd_identify(idev, IDENTITY_VERSION_1);
	err = sonic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err)
		goto err_out;

	nwords = min(ARRAY_SIZE(ident->dev.words),
		     ARRAY_SIZE(idev->dev_cmd->data));
	for (i = 0; i < nwords; i++)
		ident->dev.words[i] = ioread32(&idev->dev_cmd->data[i]);

	sonic_dev_cmd_lif_identify(idev, IDENTITY_VERSION_1);
	err = sonic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err)
		goto err_out;

	nwords = min(ARRAY_SIZE(ident->lif.words),
		     ARRAY_SIZE(idev->dev_cmd->data));
	for (i = 0; i < nwords; i++)
		ident->lif.words[i] = ioread32(&idev->dev_cmd->data[i]);

	nwords = sizeof(ident->info) / sizeof(uint32_t);
	wp = (uint32_t *)&ident->info;
	for (i = 0; i < nwords; i++)
		wp[i] = ioread32(&idev->dev_info->words[i]);

	for (i = 0, ring = idev->ring_tbl;
	     i < ACCEL_RING_ID_MAX;
	     i++, ring++) {
		ring->accel_ring = ident->lif.base.accel_ring_tbl[i];
		ring->accel_ring.ring_id = i;
		ring->name = sonic_accel_ring_name_get(i);
		osal_atomic_init(&ring->descs_inuse, 0);
	}

	err = sonic_debugfs_add_ident(sonic);
	if (err)
		goto err_out;

	return 0;

err_out:
	return err;
}

int sonic_reset(struct sonic *sonic)
{
	struct sonic_dev *idev = &sonic->idev;

	sonic_dev_cmd_reset(idev);
	return sonic_dev_cmd_wait_check(idev, devcmd_timeout);
}

static int __init sonic_init_module(void)
{
	int err;

	osal_log_init(g_osal_log_level, "sonic");
	if (core_count <= 0)
		core_count = osal_get_core_count();
	if (core_count > SONIC_MAX_CORES)
		core_count = SONIC_MAX_CORES;
	OSAL_ASSERT(core_count);

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

static void sonic_api_adminq_cb(struct queue *q, struct admin_desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct sonic_admin_ctx *ctx = cb_arg;
	struct admin_cpl *cpl = (struct admin_cpl *) cq_info->cq_desc;

	if (WARN_ON(cpl->cpl_index != desc_info->index)) {
		OSAL_LOG_ERROR("cpl_index  %u cpl 0x" PRIx64 " desc_info_index %u desc_info 0x" PRIx64,
				cpl->cpl_index, (uint64_t)cpl, desc_info->index, (uint64_t)desc_info);
		ctx->cpl.status = -1;
		dynamic_hex_dump("sonic: desc ", DUMP_PREFIX_OFFSET, 16, 1,
			desc_info, sizeof(*desc_info), true);
		dynamic_hex_dump("sonic: cpl ", DUMP_PREFIX_OFFSET, 16, 1,
			cpl, sizeof(*cpl), true);
		dynamic_hex_dump("sonic: cmd ", DUMP_PREFIX_OFFSET, 16, 1,
			&ctx->cmd, sizeof(ctx->cmd), true);
		/* Completion with error. */
		complete_all(&ctx->work);
		return;
	}

	memcpy(&ctx->cpl, cpl, sizeof(*cpl));

	OSAL_LOG_DEBUG("cpl admin queue command:");
	if (g_osal_log_level >= OSAL_LOG_LEVEL_DEBUG) {
		dynamic_hex_dump("sonic: cpl ", DUMP_PREFIX_OFFSET, 16, 1,
				&ctx->cpl, sizeof(ctx->cpl), true);
	}

	complete_all(&ctx->work);
}

int
sonic_api_adminq_post(struct lif *lif,
		struct sonic_admin_ctx *ctx,
		struct admin_desc_info **desc_info)
{
	struct queue *adminq = &lif->adminqcq->q;
	int err = 0;

	*desc_info = NULL;
#ifndef __FreeBSD__
	WARN_ON(in_interrupt());
#endif

	spin_lock(&lif->adminq_lock);
	if (!sonic_q_has_space(adminq, 1)) {
		err = -ENOSPC;
		goto err_out;
	}

	memcpy(adminq->admin_head->desc, &ctx->cmd, sizeof(ctx->cmd));

	OSAL_LOG_DEBUG("post admin queue command:");
	if (g_osal_log_level >= OSAL_LOG_LEVEL_DEBUG) {
		dynamic_hex_dump("sonic: cmd ", DUMP_PREFIX_OFFSET, 16, 1,
				 &ctx->cmd, sizeof(ctx->cmd), true);
	}

	*desc_info = sonic_q_post(adminq, true, sonic_api_adminq_cb, ctx);

err_out:
	spin_unlock(&lif->adminq_lock);

	return err;
}

void sonic_dev_cmd_identify(struct sonic_dev *idev, u16 ver)
{
	union dev_cmd cmd = {
		.dev_identify.opcode = CMD_OPCODE_IDENTIFY,
		.dev_identify.ver = ver,
		.dev_identify.type = ACCEL_DEV_TYPE_BASE,
	};

	sonic_dev_cmd_go(idev, &cmd);
}

void sonic_dev_cmd_lif_identify(struct sonic_dev *idev, u16 ver)
{
	union dev_cmd cmd = {
		.lif_identify.opcode = CMD_OPCODE_LIF_IDENTIFY,
		.lif_identify.ver = ver,
		.lif_identify.type = ACCEL_LIF_TYPE_BASE,
	};

	sonic_dev_cmd_go(idev, &cmd);
}

void sonic_dev_cmd_lif_init(struct sonic_dev *idev, u32 lif_index)
{
	union dev_cmd cmd = {
		.lif_init.opcode = CMD_OPCODE_LIF_INIT,
		.lif_init.lif_index = lif_index,
	};

	sonic_dev_cmd_go(idev, &cmd);
}

void sonic_dev_cmd_lif_reset(struct sonic_dev *idev, u32 lif_index)
{
	union dev_cmd cmd = {
		.lif_reset.opcode = CMD_OPCODE_LIF_RESET,
		.lif_reset.lif_index = lif_index,
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
			.lif_index = lif->index,
			.key_index = key_index,
			.key_size = key_size,
			.key_part = CMD_CRYPTO_KEY_PART0,
		},
	};
	struct sonic_admin_ctx ctx1 = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx1.work),
		.cmd.crypto_key_update = {
			.opcode = CMD_OPCODE_CRYPTO_KEY_UPDATE,
			.lif_index = lif->index,
			.key_index = key_index,
			.key_size = key_size,
			.key_part = CMD_CRYPTO_KEY_PART1,
		},
	};

	OSAL_LOG_INFO("crypto_key_update.key_index %u",
		 ctx0.cmd.crypto_key_update.key_index);
	OSAL_LOG_INFO("crypto_key_update.key_size %u",
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
		OSAL_LOG_ERROR("invalid key_size %u",
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
		err = SONIC_TO_PNSO_ERR(err);
	}
	return err;
}

module_init(sonic_init_module);
module_exit(sonic_cleanup_module);
