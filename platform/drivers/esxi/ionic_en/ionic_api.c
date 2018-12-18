#include "ionic.h"

#if 0
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

void ionic_api_get_dbpages(struct lif *lif, u32 *dbid, u64 __iomem **dbpage,
			   phys_addr_t *phys_dbpage_base,
			   u32 __iomem **intr_ctrl)
{
	/* XXX dbpage of the eth driver, first page for now */
	/* XXX kernel should only ioremap one dbpage, not the whole BAR */
	*dbid = 0;
	*dbpage = (void *)lif->ionic->idev.db_pages;

	*phys_dbpage_base = lif->ionic->idev.phy_db_pages;

	*intr_ctrl = (void *)lif->ionic->idev.intr_ctrl;
}
EXPORT_SYMBOL_GPL(ionic_api_get_dbpages);

int ionic_api_get_dbid(struct lif *lif)
{
	/* XXX second dbid for rdma driver, eth will use dbid zero.
 * 	 * TODO: actual allocator here. */
	return 1;
}
EXPORT_SYMBOL_GPL(ionic_api_get_dbid);

void ionic_api_put_dbid(struct lif *lif, int dbid)
{
	/* TODO: actual allocator here. */
}
EXPORT_SYMBOL_GPL(ionic_api_put_dbid);

#endif

static void ionic_api_adminq_cb(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct ionic_admin_ctx *ctx = cb_arg;
	struct admin_comp *comp = cq_info->cq_desc;

	if (IONIC_WARN_ON(comp->comp_index != desc_info->index))
		return;

	vmk_Memcpy(&ctx->comp, comp, sizeof(*comp));

	//dev_dbg(&lif->netdev->dev, "comp admin queue command:\n");
//	ionic_dbg("comp admin queue command:\n");

//	dynamic_hex_dump("comp ", DUMP_PREFIX_OFFSET, 16, 1,
//			 &ctx->comp, sizeof(ctx->comp), true);

//	complete_all(&ctx->work);
	ionic_complete(&ctx->work);
}

VMK_ReturnStatus
ionic_api_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	VMK_ReturnStatus status = VMK_OK;	
	struct queue *adminq = &lif->adminqcq->q;

#ifdef FAKE_ADMINQ
        struct ionic *ionic = lif->ionic;

        if (!use_AQ) {
                goto fake_adminq;
        }
#endif

	vmk_SpinlockLock(lif->adminq_lock);
	if (!ionic_q_has_space(adminq, 1)) {
                status = VMK_NO_MEMORY;
		goto err_out;
	}

	vmk_Memcpy(adminq->head->desc, &ctx->cmd, sizeof(ctx->cmd));

//	dev_dbg(&lif->netdev->dev, "post admin queue command:\n");
	ionic_dbg("post admin queue command:\n");

//	dynamic_hex_dump("cmd ", DUMP_PREFIX_OFFSET, 16, 1,
//			 &ctx->cmd, sizeof(ctx->cmd), true);

	ionic_q_post(adminq, true, ionic_api_adminq_cb, ctx);

err_out:
	vmk_SpinlockUnlock(lif->adminq_lock);
	return status;

#ifdef FAKE_ADMINQ
fake_adminq:
        vmk_SpinlockLock(ionic->cmd_lock);
        vmk_ListInsert(&ctx->list, &ionic->cmd_list);
	vmk_SpinlockUnlock(ionic->cmd_lock);

        status = ionic_work_queue_submit(ionic->cmd_work_queue,
                                         &ionic->cmd_work,
                                         0);
        if (status != VMK_OK) {
                ionic_err("ionic_work_queue_submit() failed, status: %s",
                          vmk_StatusToString(status));
        }

	return status;
#endif
}
//EXPORT_SYMBOL_GPL(ionic_api_adminq_post);

