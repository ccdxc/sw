// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/kernel.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_dev.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"

/* This is a quick hack for handling the ionic struct name change
 * while in mid-stream.  Externally visible things like "struct lif"
 * need to be renamed to have the prefix "ionic_", especially as
 * enforced by the upstream Linux work.  The Linux driver has had
 * much of its internal structs changed, both upstream and internally,
 * but the ionic_if.h hasn't been reworked yet internally as that
 * forces changes throughout all our drivers and Naples code.
 *
 * Until we can do the global internal changes, we'll need to use
 * this ugly little oilslick hack for our API transition.
 */
struct lif {
	struct ionic_lif oilslick;
};

struct lif *get_netdev_ionic_lif(struct net_device *netdev,
				 const char *api_version,
				 enum ionic_api_prsn prsn)
{
	struct ionic_lif *lif;

	if (strcmp(api_version, IONIC_API_VERSION))
		return NULL;

	lif = ionic_netdev_lif(netdev);

	if (!lif || lif->ionic->is_mgmt_nic || prsn != IONIC_PRSN_RDMA)
		return NULL;

	return (struct lif *)lif;
}
EXPORT_SYMBOL_GPL(get_netdev_ionic_lif);

void ionic_api_request_reset(struct lif *lif)
{
	dev_warn(&lif->oilslick.netdev->dev, "not implemented\n");
}
EXPORT_SYMBOL_GPL(ionic_api_request_reset);

void *ionic_api_get_private(struct lif *lif, enum ionic_api_prsn prsn)
{
	if (prsn != IONIC_PRSN_RDMA)
		return NULL;

	return lif->oilslick.api_private;
}
EXPORT_SYMBOL_GPL(ionic_api_get_private);

int ionic_api_set_private(struct lif *lif, void *priv,
			  void (*reset_cb)(void *priv),
			  enum ionic_api_prsn prsn)
{
	if (prsn != IONIC_PRSN_RDMA)
		return -EINVAL;

	if (lif->oilslick.api_private && priv)
		return -EBUSY;

	lif->oilslick.api_private = priv;
	lif->oilslick.api_reset_cb = reset_cb;

	return 0;
}
EXPORT_SYMBOL_GPL(ionic_api_set_private);

const struct ionic_devinfo *ionic_api_get_devinfo(struct lif *lif)
{
	return &lif->oilslick.ionic->idev.dev_info;
}
EXPORT_SYMBOL_GPL(ionic_api_get_devinfo);

struct dentry *ionic_api_get_debugfs(struct lif *lif)
{
	return lif->oilslick.dentry;
}
EXPORT_SYMBOL_GPL(ionic_api_get_debugfs);

const union lif_identity *ionic_api_get_identity(struct lif *lif, int *lif_id)
{
	*lif_id = lif->oilslick.index;

	return &lif->oilslick.ionic->ident.lif;
}
EXPORT_SYMBOL_GPL(ionic_api_get_identity);

int ionic_api_get_intr(struct lif *lif, int *irq)
{
	struct ionic_intr_info *intr_obj;
	int err;

	if (!lif->oilslick.nrdma_eqs)
		return -ENOSPC;

	intr_obj = kzalloc(sizeof(*intr_obj), GFP_KERNEL);
	if (!intr_obj)
		return -ENOSPC;

	err = ionic_intr_alloc(lif->oilslick.ionic, intr_obj);
	if (err)
		goto done;

	err = ionic_bus_get_irq(lif->oilslick.ionic, intr_obj->index);
	if (err < 0) {
		ionic_intr_free(lif->oilslick.ionic, intr_obj->index);
		goto done;
	}

	--lif->oilslick.nrdma_eqs;

	*irq = err;
	err = intr_obj->index;
done:
	kfree(intr_obj);
	return err;
}
EXPORT_SYMBOL_GPL(ionic_api_get_intr);

void ionic_api_put_intr(struct lif *lif, int intr)
{
	ionic_intr_free(lif->oilslick.ionic, intr);

	++lif->oilslick.nrdma_eqs;
}
EXPORT_SYMBOL_GPL(ionic_api_put_intr);

int ionic_api_get_cmb(struct lif *lif, u32 *pgid,
		      phys_addr_t *pgaddr, int order)
{
	struct ionic_dev *idev = &lif->oilslick.ionic->idev;
	int ret;

	mutex_lock(&idev->cmb_inuse_lock);
	ret = bitmap_find_free_region(idev->cmb_inuse, idev->cmb_npages, order);
	mutex_unlock(&idev->cmb_inuse_lock);

	if (ret < 0)
		return ret;

	*pgid = (u32)ret;
	*pgaddr = idev->phy_cmb_pages + ret * PAGE_SIZE;

	return 0;
}
EXPORT_SYMBOL_GPL(ionic_api_get_cmb);

void ionic_api_put_cmb(struct lif *lif, u32 pgid, int order)
{
	struct ionic_dev *idev = &lif->oilslick.ionic->idev;

	mutex_lock(&idev->cmb_inuse_lock);
	bitmap_release_region(idev->cmb_inuse, pgid, order);
	mutex_unlock(&idev->cmb_inuse_lock);
}
EXPORT_SYMBOL_GPL(ionic_api_put_cmb);

void ionic_api_kernel_dbpage(struct lif *lif,
			     struct ionic_intr __iomem **intr_ctrl,
			     u32 *dbid, u64 __iomem **dbpage)
{
	*intr_ctrl = lif->oilslick.ionic->idev.intr_ctrl;

	*dbid = lif->oilslick.kern_pid;
	*dbpage = lif->oilslick.kern_dbpage;
}
EXPORT_SYMBOL_GPL(ionic_api_kernel_dbpage);

int ionic_api_get_dbid(struct lif *lif, u32 *dbid, phys_addr_t *addr)
{
	int id, dbpage_num;

	mutex_lock(&lif->oilslick.dbid_inuse_lock);

	id = find_first_zero_bit(lif->oilslick.dbid_inuse, lif->oilslick.dbid_count);

	if (id == lif->oilslick.dbid_count) {
		mutex_unlock(&lif->oilslick.dbid_inuse_lock);
		return -ENOMEM;
	}

	set_bit(id, lif->oilslick.dbid_inuse);

	mutex_unlock(&lif->oilslick.dbid_inuse_lock);

	dbpage_num = ionic_db_page_num(&lif->oilslick, id);

	*dbid = id;
	*addr = ionic_bus_phys_dbpage(lif->oilslick.ionic, dbpage_num);

	return 0;
}
EXPORT_SYMBOL_GPL(ionic_api_get_dbid);

void ionic_api_put_dbid(struct lif *lif, int dbid)
{
	clear_bit(dbid, lif->oilslick.dbid_inuse);
}
EXPORT_SYMBOL_GPL(ionic_api_put_dbid);

int ionic_api_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	return ionic_adminq_post(&lif->oilslick, ctx);
}
EXPORT_SYMBOL_GPL(ionic_api_adminq_post);
