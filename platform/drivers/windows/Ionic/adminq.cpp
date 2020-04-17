#include "common.h"

NDIS_STATUS
ionic_lif_adminq_init(struct lif *lif)
{
    struct ionic_dev *idev = &lif->ionic->idev;
    struct qcq *qcq = lif->adminqcq;
    struct queue *q = &qcq->q;
    struct q_init_comp comp;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    NdisAcquireSpinLock(&lif->ionic->dev_cmd_lock);
    ionic_dev_cmd_adminq_init(idev, qcq, (u16)lif->index);
    status = ionic_dev_cmd_wait(lif->ionic, devcmd_timeout);
    ionic_dev_cmd_comp(idev, (union dev_cmd_comp *)&comp);
    NdisReleaseSpinLock(&lif->ionic->dev_cmd_lock);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_ADMINQ, TRACE_LEVEL_ERROR,
                  "%s adminq init failed %08lX\n", __FUNCTION__, status));
        return status;
    }

    q->hw_type = comp.hw_type;
    q->hw_index = le32_to_cpu(comp.hw_index);
    q->dbval = IONIC_DBELL_QID(q->hw_index);

    DbgTrace((TRACE_COMPONENT_ADMINQ, TRACE_LEVEL_VERBOSE,
              "%s hw_type %d hw_index %d\n", __FUNCTION__, q->hw_type,
              q->hw_index));

    if (qcq->flags & QCQ_F_INTR)
        ionic_intr_mask(idev->intr_ctrl, qcq->cq.bound_intr->index,
                        IONIC_INTR_MASK_CLEAR);

    qcq->flags |= QCQ_F_INITED;

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_lif_rss_config(struct lif *lif,
                     const u16 types,
                     const u8 *key,
                     const u8 *indir)
{
    struct ionic_admin_ctx ctx;

    UNREFERENCED_PARAMETER(indir);

    ctx.cmd.lif_setattr.opcode = CMD_OPCODE_LIF_SETATTR;
    ctx.cmd.lif_setattr.attr = IONIC_LIF_ATTR_RSS;
    ctx.cmd.lif_setattr.rss.types = cpu_to_le16(types);
    ctx.cmd.lif_setattr.rss.addr = cpu_to_le64(lif->rss_ind_tbl_pa);

    lif->rss_types = types;

    if (key)
        NdisMoveMemory(lif->rss_hash_key, key, IONIC_RSS_HASH_KEY_SIZE);

    NdisMoveMemory(ctx.cmd.lif_setattr.rss.key, lif->rss_hash_key,
                   IONIC_RSS_HASH_KEY_SIZE);

    return ionic_adminq_post_wait(lif, &ctx);
}
