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

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/printk.h>

#include "ionic_api.h"

#include "ionic.h"
#include "ionic_if.h"
#include "ionic_bus.h"
#include "ionic_dev.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"

void *
ionic_get_handle_from_netdev(struct net_device *netdev,
    const char *api_version, enum ionic_api_prsn prsn)
{
	struct ionic_lif *lif;

	if (strcmp(api_version, IONIC_API_VERSION))
		return (ERR_PTR(-EINVAL));

	lif = ionic_netdev_lif(netdev);
	if (!lif || !lif->nrdma_eqs)
		return (ERR_PTR(-ENXIO));

	/* TODO: Rework if supporting more than one slave */
	if (lif->slave_lif_cfg.prsn != IONIC_PRSN_NONE &&
	    lif->slave_lif_cfg.prsn != prsn)
		return (ERR_PTR(-EBUSY));

	return (lif);
}
EXPORT_SYMBOL(ionic_get_handle_from_netdev);

bool
ionic_api_stay_registered(void *handle)
{
	struct ionic_lif *lif = handle;

	return (lif->stay_registered);
}
EXPORT_SYMBOL(ionic_api_stay_registered);

void
ionic_api_request_reset(void *handle)
{
	struct ionic_lif *lif = handle;
	struct ionic_dev *idev;
	int err;

	union ionic_dev_cmd cmd = {
		.cmd.opcode = IONIC_CMD_RDMA_RESET_LIF,
		.cmd.lif_index = cpu_to_le16(lif->slave_lif_cfg.index),
	};

	idev = &lif->ionic->idev;

	IONIC_DEV_LOCK(lif->ionic);

	ionic_dev_cmd_go(idev, &cmd);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);

	IONIC_DEV_UNLOCK(lif->ionic);

	if (err) {
		netdev_warn(lif->netdev, "request_reset: error %d\n", err);
		return;
	}

	if (lif->slave_lif_cfg.priv &&
	    lif->slave_lif_cfg.reset_cb)
		(*lif->slave_lif_cfg.reset_cb)(lif->slave_lif_cfg.priv);
}
EXPORT_SYMBOL(ionic_api_request_reset);

void *
ionic_api_get_private(void *handle, enum ionic_api_prsn prsn)
{
	struct ionic_lif *lif = handle;

	if (lif->slave_lif_cfg.prsn != prsn)
		return (NULL);

	return (lif->slave_lif_cfg.priv);
}
EXPORT_SYMBOL(ionic_api_get_private);

int
ionic_api_set_private(void *handle, void *priv, void (*reset_cb)(void *priv),
    enum ionic_api_prsn prsn)
{
	struct ionic_lif *lif = handle;
	struct ionic_lif_cfg *cfg = &lif->slave_lif_cfg;

	if (priv && cfg->priv)
		return (-EBUSY);

	if (lif->ionic->nlifs > 1) {
		if (priv) {
			/* Allocate a new slave LIF bit */
			cfg->index = ionic_slave_alloc(lif->ionic, prsn);
			if (cfg->index < 0)
				return (-ENOSPC);
		} else if (cfg->priv) {
			/* Free the existing slave LIF bit */
			ionic_slave_free(lif->ionic, cfg->index);
		}
	}

	cfg->priv = priv;
	cfg->prsn = prsn;
	cfg->reset_cb = reset_cb;

	return (0);
}
EXPORT_SYMBOL(ionic_api_set_private);

struct device *
ionic_api_get_device(void *handle)
{
	struct ionic_lif *lif = handle;

	return (lif->ionic->dev);
}
EXPORT_SYMBOL(ionic_api_get_device);

const struct ionic_devinfo *
ionic_api_get_devinfo(void *handle)
{
	struct ionic_lif *lif = handle;

	return (&lif->ionic->idev.dev_info);
}
EXPORT_SYMBOL(ionic_api_get_devinfo);

struct sysctl_oid *
ionic_api_get_debug_ctx(void *handle)
{
	struct ionic_lif *lif = handle;

	return (lif->sysctl_ifnet);
}
EXPORT_SYMBOL(ionic_api_get_debug_ctx);

const union ionic_lif_identity *
ionic_api_get_identity(void *handle, int *lif_index)
{
	struct ionic_lif *lif = handle;

	if (lif_index)
		*lif_index = lif->slave_lif_cfg.index;

	/* TODO: Do all LIFs have the same ident? */
	return (&lif->ionic->ident.lif);
}
EXPORT_SYMBOL(ionic_api_get_identity);

int
ionic_api_get_intr(void *handle, int *irq)
{
	struct ionic_lif *lif = handle;
	struct intr intr_obj = {
		.index = IONIC_INTR_INDEX_NOT_ASSIGNED
	};
	int err;

	if (!lif->nrdma_eqs_avail)
		return (-ENOSPC);

	err = ionic_dev_intr_reserve(lif, &intr_obj);
	if (err)
		return (-err);

	err = ionic_get_msix_irq(lif->ionic, intr_obj.index);
	if (err < 0) {
		ionic_dev_intr_unreserve(lif, &intr_obj);
		return (err);
	}

	lif->nrdma_eqs_avail--;

	*irq = err;
	return (intr_obj.index);
}
EXPORT_SYMBOL(ionic_api_get_intr);

void
ionic_api_put_intr(void *handle, int intr)
{
	struct ionic_lif *lif = handle;
	struct intr intr_obj = {
		.index = intr
	};

	ionic_dev_intr_unreserve(lif, &intr_obj);

	lif->nrdma_eqs_avail++;
}
EXPORT_SYMBOL(ionic_api_put_intr);

int
ionic_api_get_cmb(void *handle, uint32_t *pgid, phys_addr_t *pgaddr, int order)
{
	struct ionic_lif *lif = handle;
	struct ionic_dev *idev = &lif->ionic->idev;
	int ret;

	mutex_lock(&idev->cmb_inuse_lock);
	ret = bitmap_find_free_region(idev->cmb_inuse, idev->cmb_npages,
	    order);
	mutex_unlock(&idev->cmb_inuse_lock);

	if (ret < 0)
		return (ret);

	*pgid = (uint32_t)ret;
	*pgaddr = idev->phy_cmb_pages + ret * PAGE_SIZE;

	return (0);
}
EXPORT_SYMBOL(ionic_api_get_cmb);

void
ionic_api_put_cmb(void *handle, uint32_t pgid, int order)
{
	struct ionic_lif *lif = handle;
	struct ionic_dev *idev = &lif->ionic->idev;

	mutex_lock(&idev->cmb_inuse_lock);
	bitmap_release_region(idev->cmb_inuse, pgid, order);
	mutex_unlock(&idev->cmb_inuse_lock);
}
EXPORT_SYMBOL(ionic_api_put_cmb);

void
ionic_api_kernel_dbpage(void *handle, struct ionic_intr __iomem **intr_ctrl,
    uint32_t *dbid, uint64_t __iomem **dbpage)
{
	struct ionic_lif *lif = handle;

	*intr_ctrl = lif->ionic->idev.intr_ctrl;

	*dbid = lif->kern_pid;
	*dbpage = lif->kern_dbpage;
}
EXPORT_SYMBOL(ionic_api_kernel_dbpage);

int
ionic_api_get_dbid(void *handle, uint32_t *dbid, phys_addr_t *addr)
{
	struct ionic_lif *lif = handle;
	int id, dbpage_num;

	mutex_lock(&lif->dbid_inuse_lock);

	id = find_first_zero_bit(lif->dbid_inuse, lif->dbid_count);
	if (id == lif->dbid_count) {
		mutex_unlock(&lif->dbid_inuse_lock);
		return (-ENOMEM);
	}

	set_bit(id, lif->dbid_inuse);

	mutex_unlock(&lif->dbid_inuse_lock);

	dbpage_num = ionic_db_page_num(lif->ionic, lif->index, id);

	*dbid = id;
	*addr = ionic_bus_phys_dbpage(lif->ionic, dbpage_num);

	return (0);
}
EXPORT_SYMBOL(ionic_api_get_dbid);

void
ionic_api_put_dbid(void *handle, int dbid)
{
	struct ionic_lif *lif = handle;

	clear_bit(dbid, lif->dbid_inuse);
}
EXPORT_SYMBOL(ionic_api_put_dbid);

/* External users: post commands using dev_cmds */
#define IONIC_API_ADMINQ_WAIT_SEC 10
int
ionic_api_adminq_post(void *handle, struct ionic_admin_ctx *ctx)
{
	struct ionic_lif *lif = handle;
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;
	int err;

	IONIC_DEV_LOCK(ionic);
	lif->num_dev_cmds++;

	if (__IONIC_DEBUG) {
		IONIC_DEV_INFO(ionic->dev, "post external dev command:\n");
		print_hex_dump_debug("cmd ", DUMP_PREFIX_OFFSET, 16, 1,
		    &ctx->cmd, sizeof(ctx->cmd), true);
	}

	ionic_dev_cmd_go(idev, (union ionic_dev_cmd *)&ctx->cmd);

	err = ionic_dev_cmd_wait_check(idev, IONIC_API_ADMINQ_WAIT_SEC * HZ);
	if (err)
		goto err_out;

	ionic_dev_cmd_comp(idev, &ctx->comp);

	if (__IONIC_DEBUG) {
		IONIC_DEV_INFO(ionic->dev, "comp external dev command:\n");
		print_hex_dump_debug("comp ", DUMP_PREFIX_OFFSET, 16, 1,
		    &ctx->comp, sizeof(ctx->comp), true);
	}

err_out:
	IONIC_DEV_UNLOCK(ionic);

	if (!err) {
		complete_all(&ctx->work);
	}

	return (err);
}
EXPORT_SYMBOL(ionic_api_adminq_post);
