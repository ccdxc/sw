#include <linux/kernel.h>

#include "ionic_api.h"

#include "ionic.h"
#include "ionic_if.h"
#include "ionic_dev.h"
#include "ionic_lif.h"

struct lif *get_netdev_ionic_lif(struct net_device *netdev,
				 const char *api_version)
{
	if (!netdev || !netdev->dev.parent || !netdev->dev.parent->driver)
		return NULL;

	if (netdev->dev.parent->driver->owner != THIS_MODULE)
		return NULL;

	if (strcmp(api_version, IONIC_API_VERSION))
		return NULL;

	return netdev_priv(netdev);
}
EXPORT_SYMBOL_GPL(get_netdev_ionic_lif);

int ionic_api_get_intr(struct lif *lif, int *irq)
{
	/* XXX second intr for rdma driver, eth will use intr zero.
	 * TODO: actual allocator here. */
	return 1;
}
EXPORT_SYMBOL_GPL(ionic_api_get_intr);

void ionic_api_put_intr(struct lif *lif, int intr)
{
	/* TODO: actual allocator here. */
}
EXPORT_SYMBOL_GPL(ionic_api_put_intr);

int ionic_api_get_dbpages(struct lif *lif,
			  u64 __iomem **dbpage,
			  phys_addr_t *phys_dbpage_base)
{
	/* XXX dbpage of the eth driver, first page for now */
	/* XXX kernel should only ioremap one dbpage, not the whole BAR */
	*dbpage = (void *)lif->ionic->idev.db_pages;

	*phys_dbpage_base = lif->ionic->idev.phy_db_pages;

	/* XXX dbid of the eth driver, zero for now */
	return 0;
}
EXPORT_SYMBOL_GPL(ionic_api_get_dbpages);

int ionic_api_get_dbid(struct lif *lif)
{
	/* XXX second dbid for rdma driver, eth will use dbid zero.
	 * TODO: actual allocator here. */
	return 1;
}
EXPORT_SYMBOL_GPL(ionic_api_get_dbid);

void ionic_api_put_dbid(struct lif *lif, int dbid)
{
	/* TODO: actual allocator here. */
}
EXPORT_SYMBOL_GPL(ionic_api_put_dbid);

#ifndef ADMINQ
#define XXX_DEVCMD_HALF_PAGE 0x800

// XXX temp func to get side-band data from 2nd half page of dev_cmd reg space.
static int SBD_get(struct ionic_dev *idev, void *dst, size_t len)
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
static int SBD_put(struct ionic_dev *idev, void *src, size_t len)
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
#else
static void ionic_api_adminq_cb(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct ionic_admin_ctx *ctx = cb_arg;
	struct admin_comp *comp = cq_info->cq_desc;

	if (WARN_ON(comp->comp_index != desc_info->index))
		return;

	memcpy(&ctx->comp, comp, sizeof(*comp));

	complete_all(&ctx->work);
}
#endif

int ionic_api_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
#ifdef ADMINQ
	struct queue *adminq = &lif->adminqcq->q;
	int err;

	WARN_ON(in_interrupt());

	spin_lock(&lif->adminq_lock);
	if (!ionic_q_has_space(adminq, 1)) {
		err = -ENOSPC;
		goto err_out;
	}

	memcpy(adminq->head->desc, &ctx->cmd, sizeof(ctx->cmd));

	ionic_q_post(adminq, true, ionic_api_adminq_cb, ctx);

err_out:
	spin_unlock(&lif->adminq_lock);

	return err;
#else
	struct ionic_dev *idev = &lif->ionic->idev;
	int err;

	WARN_ON(in_interrupt());

	spin_lock(&lif->adminq_lock);

	if (ctx->side_data) {
		err = SBD_put(idev, ctx->side_data, ctx->side_data_len);
		if (err)
			goto err_out;
        }

	ionic_dev_cmd_go(idev, (void *)&ctx->cmd);

	/* sleep while holding spinlock... this is just temporary */
	err = ionic_dev_cmd_wait_check(idev, HZ * 10);
	if (err)
		goto err_out;

	ionic_dev_cmd_comp(idev, &ctx->comp);

	if (ctx->side_data) {
		err = SBD_get(idev, ctx->side_data, ctx->side_data_len);
		if (err)
			goto err_out;
	}

err_out:
	spin_unlock(&lif->adminq_lock);

	if (!err) {
		complete_all(&ctx->work);
	}

	return err;
#endif
}
EXPORT_SYMBOL_GPL(ionic_api_adminq_post);
