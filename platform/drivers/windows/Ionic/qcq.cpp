
#include "common.h"

NDIS_STATUS
ionic_q_init(struct lif *lif,
             struct ionic_dev *idev,
             struct queue *q,
             unsigned int index,
             const char *name,
             unsigned int num_descs,
             size_t desc_size,
             size_t sg_desc_size,
             unsigned int pid)
{
    unsigned int ring_size;
    struct desc_info *cur;
    unsigned int i;

    if (desc_size == 0 || !is_power_of_2(num_descs))
        return NDIS_STATUS_INVALID_DATA;

    ring_size = ilog2(num_descs);
    if (ring_size < 2 || ring_size > 16)
        return NDIS_STATUS_INVALID_DATA;

    q->lif = lif;
    q->idev = idev;
    q->index = index;
    q->num_descs = num_descs;
    q->desc_size = (unsigned int)desc_size;
    q->sg_desc_size = (unsigned int)sg_desc_size;
    q->tail = q->info;
    q->head = q->tail;
    q->pid = pid;

    q->rx_pkt_cnt = GetPktCount(lif->ionic, q);

    _snprintf_s(q->name, QUEUE_NAME_MAX_SZ, sizeof(q->name), "L%d-%s%u", lif->index, name, index);

    cur = q->info;

    for (i = 0; i < num_descs; i++) {
        if (i + 1 == num_descs)
            cur->next = q->info;
        else
            cur->next = cur + 1;
        cur->index = i;
        cur->left = num_descs - i;
        cur++;
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
ionic_cq_init(struct lif *lif,
              struct cq *cq,
              struct intr *intr,
              unsigned int num_descs,
              size_t desc_size)
{
    unsigned int ring_size;
    struct cq_info *cur;
    unsigned int i;

    if (desc_size == 0 || !is_power_of_2(num_descs))
        return NDIS_STATUS_INVALID_DATA;

    ring_size = ilog2(num_descs);
    if (ring_size < 2 || ring_size > 16)
        return NDIS_STATUS_INVALID_DATA;

    cq->lif = lif;
    cq->bound_intr = intr;
    cq->num_descs = num_descs;
    cq->desc_size = (unsigned int)desc_size;
    cq->tail = cq->info;
    cq->done_color = 1;

    cur = cq->info;

    for (i = 0; i < num_descs; i++) {
        if (i + 1 == num_descs) {
            cur->next = cq->info;
            cur->last = true;
        } else {
            cur->next = cur + 1;
        }
        cur->index = i;
        cur++;
    }

    return NDIS_STATUS_SUCCESS;
}

void
ionic_q_sg_map(struct queue *q, void *base, dma_addr_t base_pa)
{
    struct desc_info *cur;
    unsigned int i;

    q->sg_base = base;
    q->sg_base_pa = base_pa;

    for (i = 0, cur = q->info; i < q->num_descs; i++, cur++)
        cur->sg_desc = (void *)((char *)base + (i * q->sg_desc_size));
}

void
ionic_q_map(struct queue *q, void *base, dma_addr_t base_pa)
{
    struct desc_info *cur;
    unsigned int i;

    q->base = base;
    q->base_pa = base_pa;

    for (i = 0, cur = q->info; i < q->num_descs; i++, cur++)
        cur->desc = (void *)((char *)base + (i * q->desc_size));
}

void
ionic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa)
{
    struct cq_info *cur;
    unsigned int i;

    cq->base = base;
    cq->base_pa = base_pa;

    for (i = 0, cur = cq->info; i < cq->num_descs; i++, cur++)
        cur->cq_desc = (void *)((char *)base + (i * cq->desc_size));
}

void
ionic_cq_bind(struct cq *cq, struct queue *q)
{
    cq->bound_q = q;
}

NDIS_STATUS
ionic_link_master_qcq(struct qcq *qcq, struct qcqst *master_qs)
{
    struct lif *master_lif = qcq->q.lif->ionic->master_lif;
    unsigned int slot;

    slot = master_lif->ntxqs + qcq->q.lif->index - 1;

    /* TODO: should never be true */
    if (master_qs[slot].qcq) {
        IoPrint("%s bad slot number %d\n", __FUNCTION__, qcq->master_slot);
        return NDIS_STATUS_INVALID_DATA;
    }

    master_qs[slot].qcq = qcq;
    // master_qs[slot].stats = qcq->stats;
    qcq->master_slot = slot;

    return NDIS_STATUS_SUCCESS;
}

void
ionic_lif_qcq_deinit(struct lif *lif, struct qcq *qcq)
{
    struct ionic_dev *idev = &lif->ionic->idev;

    if (!qcq)
        return;

    if (!(qcq->flags & QCQ_F_INITED))
        return;

    if (qcq->flags & QCQ_F_INTR) {
        ionic_intr_mask(idev->intr_ctrl, qcq->intr.index, IONIC_INTR_MASK_SET);
        // synchronize_irq(qcq->intr.vector);
        // devm_free_irq(dev, qcq->intr.vector, &qcq->napi);
        // netif_napi_del(&qcq->napi);
    }

    qcq->flags &= ~QCQ_F_INITED;
}

unsigned int
ionic_cq_service(struct cq *cq,
                 unsigned int work_to_do,
                 ionic_cq_cb cb,
                 void *cb_arg,
                 ionic_cq_done_cb done_cb,
                 void *done_arg)
{
    unsigned int work_done = 0;

    if (work_to_do == 0)
        return 0;

    while (cb(cq, cq->tail, cb_arg)) {
        if (cq->tail->last)
            cq->done_color = !cq->done_color;
        cq->tail = cq->tail->next;
        // DEBUG_STATS_CQE_CNT(cq);

        if (++work_done >= work_to_do)
            break;
    }

    if (work_done && done_cb)
        done_cb(done_arg);

    return work_done;
}

NDIS_STATUS
ionic_qcq_enable(struct qcq *qcq)
{
    struct queue *q = &qcq->q;
    struct lif *lif = q->lif;
    struct ionic_dev *idev = &lif->ionic->idev;
    struct ionic_admin_ctx ctx = {0};

    ctx.cmd.q_control.opcode = CMD_OPCODE_Q_CONTROL;
    ctx.cmd.q_control.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.q_control.type = (u8)q->type;
    ctx.cmd.q_control.index = cpu_to_le32(q->index);
    ctx.cmd.q_control.oper = IONIC_Q_ENABLE;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s q_enable.index %d q_enable.qtype %d flags 0x%08lX\n",
              __FUNCTION__, ctx.cmd.q_control.index, ctx.cmd.q_control.type,
              qcq->flags));

    if (qcq->flags & QCQ_F_INTR) {
        // irq_set_affinity_hint(qcq->intr.vector,
        //		      &qcq->intr.affinity_mask);
        // napi_enable(&qcq->napi);
        ionic_intr_clean(idev->intr_ctrl, qcq->intr.index);
        ionic_intr_mask(idev->intr_ctrl, qcq->intr.index,
                        IONIC_INTR_MASK_CLEAR);
    }

    return ionic_adminq_post_wait(lif, &ctx);
}

NDIS_STATUS
ionic_qcq_disable(struct qcq *qcq)
{
    struct queue *q = &qcq->q;
    struct lif *lif = q->lif;
    struct ionic_dev *idev = &lif->ionic->idev;
    struct ionic_admin_ctx ctx = {0};

    ctx.cmd.q_control.opcode = CMD_OPCODE_Q_CONTROL;
    ctx.cmd.q_control.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.q_control.type = (u8)q->type;
    ctx.cmd.q_control.index = cpu_to_le32(q->index);
    ctx.cmd.q_control.oper = IONIC_Q_DISABLE;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s q_disable.index %d q_disable.qtype %d\n", __FUNCTION__,
              ctx.cmd.q_control.index, ctx.cmd.q_control.type));

    if (qcq->flags & QCQ_F_INTR) {
        ionic_intr_mask(idev->intr_ctrl, qcq->intr.index, IONIC_INTR_MASK_SET);
    }

    return ionic_adminq_post_wait(lif, &ctx);
}

void
ionic_reset_qcq(struct qcq *qcq)
{

    struct queue *q = &qcq->q;
    struct cq *cq = &qcq->cq;
    unsigned int num_descs = 0;
    void *q_base, *cq_base, *sg_base;
    dma_addr_t cq_base_pa = 0;
    dma_addr_t sg_base_pa = 0;
    dma_addr_t q_base_pa = 0;
    u32 q_desc_size, cq_desc_size, sg_desc_size;
    u32 q_size, cq_size, sg_size;
    u32 total_size = 0;

    if (q->type == IONIC_QTYPE_RXQ) {
        num_descs = qcq->q.lif->nrxq_descs;
        q_desc_size = sizeof(struct rxq_desc);
        cq_desc_size = sizeof(struct rxq_comp);
        sg_desc_size = sizeof(struct rxq_sg_desc);
    }
    else {
        num_descs = qcq->q.lif->ntxq_descs;
        q_desc_size = sizeof(struct txq_desc);
        cq_desc_size = sizeof(struct txq_comp);
        sg_desc_size = sizeof(struct txq_sg_desc);
    }

    q_size = num_descs * q_desc_size;
    cq_size = num_descs * cq_desc_size;
    sg_size = num_descs * sg_desc_size;

    total_size = ALIGN(q_size, PAGE_SIZE) + ALIGN(cq_size, PAGE_SIZE);
    /* Note: aligning q_size/cq_size is not enough due to cq_base
     * address aligning as q_base could be not aligned to the page.
     * Adding PAGE_SIZE.
     */
    total_size += PAGE_SIZE;
    if (qcq->flags & QCQ_F_SG) {
        total_size += ALIGN(sg_size, PAGE_SIZE);
        total_size += PAGE_SIZE;
    }

    q->tail = q->info;
    q->head = q->tail;

    cq->tail = cq->info;
    cq->done_color = 1;

	ASSERT( qcq->total_size == total_size);
   
    NdisZeroMemory(qcq->base, total_size);

    q_base = qcq->base;
    q_base_pa = qcq->base_pa;

    cq_base = (void *)ALIGN((uintptr_t)q_base + q_size, PAGE_SIZE);
    cq_base_pa = ALIGN(q_base_pa + q_size, PAGE_SIZE);

    if (qcq->flags & QCQ_F_SG) {
        sg_base = (void *)ALIGN((uintptr_t)cq_base + cq_size, PAGE_SIZE);
        sg_base_pa = ALIGN(cq_base_pa + cq_size, PAGE_SIZE);
        ionic_q_sg_map(q, sg_base, sg_base_pa);
    }

    ionic_q_map(q, q_base, q_base_pa);
    ionic_cq_map(cq, cq_base, cq_base_pa);
}