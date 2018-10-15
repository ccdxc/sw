#include <linux/kernel.h>

#include "ionic_api.h"

#include "ionic.h"
#include "ionic_if.h"
#include "ionic_bus.h"
#include "ionic_dev.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"

struct lif *get_netdev_ionic_lif(struct net_device *netdev,
				 const char *api_version)
{
	if (!netdev || netdev->netdev_ops->ndo_start_xmit != ionic_start_xmit)
		return NULL;

	if (strcmp(api_version, IONIC_API_VERSION))
		return NULL;

	return netdev_priv(netdev);
}
EXPORT_SYMBOL_GPL(get_netdev_ionic_lif);

void *ionic_api_get_private(struct lif *lif, enum ionic_api_private kind)
{
	if (kind != IONIC_RDMA_PRIVATE)
		return NULL;

	return lif->api_private;
}
EXPORT_SYMBOL_GPL(ionic_api_get_private);

int ionic_api_set_private(struct lif *lif, void *priv,
			  enum ionic_api_private kind)
{
	if (kind != IONIC_RDMA_PRIVATE)
		return -EINVAL;

	if (lif->api_private && priv)
		return -EBUSY;

	lif->api_private = priv;

	return 0;
}
EXPORT_SYMBOL_GPL(ionic_api_set_private);

struct dentry *ionic_api_get_debugfs(struct lif *lif)
{
	return lif->debugfs;
}
EXPORT_SYMBOL_GPL(ionic_api_get_debugfs);

const union identity *ionic_api_get_identity(struct lif *lif, int *lif_id)
{
	*lif_id = lif->index;

	return lif->ionic->ident;
}
EXPORT_SYMBOL_GPL(ionic_api_get_identity);

int ionic_api_get_intr(struct lif *lif, int *irq)
{
	struct intr intr_obj = {};
	int err;

	if (!lif->neqs)
		return -ENOSPC;

	err = ionic_intr_alloc(lif, &intr_obj);
	if (err)
		return err;

	err = ionic_bus_get_irq(lif->ionic, intr_obj.index);
	if (err < 0) {
		ionic_intr_free(lif, &intr_obj);
		return err;
	}

	--lif->neqs;

	*irq = err;
	return intr_obj.index;
}
EXPORT_SYMBOL_GPL(ionic_api_get_intr);

void ionic_api_put_intr(struct lif *lif, int intr)
{
	struct intr intr_obj = {
		.index = intr
	};

	ionic_intr_free(lif, &intr_obj);

	++lif->neqs;
}
EXPORT_SYMBOL_GPL(ionic_api_put_intr);

int ionic_api_get_cmb(struct lif *lif, u32 *pgid, phys_addr_t *pgaddr, int order)
{
	struct ionic_dev *idev = &lif->ionic->idev;
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
	struct ionic_dev *idev = &lif->ionic->idev;

	mutex_lock(&idev->cmb_inuse_lock);
	bitmap_release_region(idev->cmb_inuse, pgid, order);
	mutex_unlock(&idev->cmb_inuse_lock);
}
EXPORT_SYMBOL_GPL(ionic_api_put_cmb);

void ionic_api_get_dbpages(struct lif *lif, u32 *dbid,
			   u64 __iomem **dbpage,
			   phys_addr_t *xxx_dbpage_phys,
			   u32 __iomem **intr_ctrl)
{
	int dbpage_num;

	*dbid = lif->kern_pid;
	*dbpage = (void __iomem *)lif->kern_dbpage;
	*intr_ctrl = (void __iomem *)lif->ionic->idev.intr_ctrl;

	/* XXX remove when rdma drops xxx_kdbid workaround */
	dbpage_num = ionic_db_page_num(&lif->ionic->idev, lif->index, 0);
	*xxx_dbpage_phys = ionic_bus_phys_dbpage(lif->ionic, dbpage_num);
}
EXPORT_SYMBOL_GPL(ionic_api_get_dbpages);

int ionic_api_get_dbid(struct lif *lif, phys_addr_t *addr)
{
	int dbid, dbpage_num;

	mutex_lock(&lif->dbid_inuse_lock);

	dbid = find_first_zero_bit(lif->dbid_inuse, lif->dbid_count);

	if (dbid == lif->dbid_count) {
		mutex_unlock(&lif->dbid_inuse_lock);
		return -ENOMEM;
	}

	set_bit(dbid, lif->dbid_inuse);

	mutex_unlock(&lif->dbid_inuse_lock);

	dbpage_num = ionic_db_page_num(&lif->ionic->idev, lif->index, dbid);
	*addr = ionic_bus_phys_dbpage(lif->ionic, dbpage_num);

	return dbid;
}
EXPORT_SYMBOL_GPL(ionic_api_get_dbid);

void ionic_api_put_dbid(struct lif *lif, int dbid)
{
	clear_bit(dbid, lif->dbid_inuse);
}
EXPORT_SYMBOL_GPL(ionic_api_put_dbid);

#ifdef ADMINQ
static void ionic_api_adminq_cb(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct ionic_admin_ctx *ctx = cb_arg;
	struct admin_comp *comp = cq_info->cq_desc;

	if (WARN_ON(comp->comp_index != desc_info->index))
		return;

	memcpy(&ctx->comp, comp, sizeof(*comp));

	dev_dbg(&lif->netdev->dev, "comp admin queue command:\n");
	dynamic_hex_dump("comp ", DUMP_PREFIX_OFFSET, 16, 1,
			 &ctx->comp, sizeof(ctx->comp), true);

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

	dev_dbg(&lif->netdev->dev, "post admin queue command:\n");
	dynamic_hex_dump("cmd ", DUMP_PREFIX_OFFSET, 16, 1,
			 &ctx->cmd, sizeof(ctx->cmd), true);

	ionic_q_post(adminq, true, ionic_api_adminq_cb, ctx);

err_out:
	spin_unlock(&lif->adminq_lock);

	return err;
#else
	struct ionic *ionic = lif->ionic;
	unsigned long irqflags;

	spin_lock_irqsave(&ionic->cmd_lock, irqflags);
	list_add(&ctx->list, &ionic->cmd_list);
	spin_unlock_irqrestore(&ionic->cmd_lock, irqflags);

	/* schedule on a buddy cpu, in case this cpu needs to busy-wait */

#ifdef PENSANDO_MNIC
	schedule_work(&ionic->cmd_work);
#else
	schedule_work_on(raw_smp_processor_id()^1, &ionic->cmd_work);
#endif //PENSANDO_MNIC

	return 0;
#endif
}
EXPORT_SYMBOL_GPL(ionic_api_adminq_post);
