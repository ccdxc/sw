
#include "common.h"

NDIS_STATUS
ionic_lif_notifyq_init(struct lif *lif)
{
    struct qcq *qcq = lif->notifyqcq;
    struct queue *q = &qcq->q;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    USHORT flags = 0;

    struct ionic_admin_ctx ctx = {0};

    ctx.cmd.q_init.opcode = CMD_OPCODE_Q_INIT;
    ctx.cmd.q_init.lif_index = (__le16)cpu_to_le16(lif->index);
    ctx.cmd.q_init.type = (u8)q->type;
    ctx.cmd.q_init.index = cpu_to_le32(q->index);
    if (lif->adminqcq->flags & IONIC_QINIT_F_IRQ) {
        flags |= IONIC_QINIT_F_IRQ;
        ctx.cmd.q_init.intr_index = (__le16)cpu_to_le16(lif->adminqcq->cq.bound_intr->index);
    }
    ctx.cmd.q_init.pid = (__le16)cpu_to_le16(q->pid);
    ctx.cmd.q_init.ring_size = (u8)ilog2(q->num_descs);
    ctx.cmd.q_init.ring_base = cpu_to_le64(q->base_pa);
    ctx.cmd.q_init.flags = cpu_to_le16(flags | IONIC_QINIT_F_ENA);

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s notifyq_init.pid %d\n", __FUNCTION__, ctx.cmd.q_init.pid));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s notifyq_init.index %d\n", __FUNCTION__,
              ctx.cmd.q_init.index));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s notifyq_init.ring_base 0x%llx\n", __FUNCTION__,
              ctx.cmd.q_init.ring_base));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s notifyq_init.ring_size %d\n", __FUNCTION__,
              ctx.cmd.q_init.ring_size));

    status = ionic_adminq_post_wait(lif, &ctx);
    if (status != NDIS_STATUS_SUCCESS)
        return status;

    q->hw_type = ctx.comp.q_init.hw_type;
    q->hw_index = le32_to_cpu(ctx.comp.q_init.hw_index);
    q->dbval = IONIC_DBELL_QID(q->hw_index);

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s notifyq->hw_type %d\n", __FUNCTION__, q->hw_type));
    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s notifyq->hw_index %d\n", __FUNCTION__, q->hw_index));

    /* preset the callback info */
    q->info[0].cb_arg = lif;

    qcq->flags |= QCQ_F_INITED;

    return NDIS_STATUS_SUCCESS;
}

static bool
ionic_notifyq_service(struct cq *cq, struct cq_info *cq_info, void *cb_arg)
{
    union notifyq_comp *comp = (union notifyq_comp *)cq_info->cq_desc;
    // struct net_device *netdev;
    struct queue *q;
    struct lif *lif;
    u64 eid;

    UNREFERENCED_PARAMETER(cb_arg);

    q = cq->bound_q;
    lif = (struct lif *)q->info[0].cb_arg;
    // netdev = lif->netdev;
    eid = le64_to_cpu(comp->event.eid);

    /* Have we run out of new completions to process? */
    if ((signed long long)(eid - lif->last_eid) <= 0) {
        return false;
    }

    lif->last_eid = eid;

    DbgTrace((TRACE_COMPONENT_NOTIFQ, TRACE_LEVEL_VERBOSE,
              "%s notifyq event:\n", __FUNCTION__));
    print_hex_dump("\tevent ", DUMP_PREFIX_OFFSET, 16, 1, comp, sizeof(*comp),
                   true);

    switch (le16_to_cpu(comp->event.ecode)) {
    case EVENT_OPCODE_LINK_CHANGE: {
        DbgTrace((TRACE_COMPONENT_NOTIFQ, TRACE_LEVEL_VERBOSE,
                  "%s Notifyq EVENT_OPCODE_LINK_CHANGE eid=%lld link_status=%d "
                  "link_speed=%d\n",
                  __FUNCTION__, eid, le16_to_cpu(comp->link_change.link_status),
                  le32_to_cpu(comp->link_change.link_speed)));

        RtlSetBit(&lif->state, LIF_LINK_CHECK_NEEDED);

        break;
    }

    case EVENT_OPCODE_RESET: {
        DbgTrace(
            (TRACE_COMPONENT_NOTIFQ, TRACE_LEVEL_VERBOSE,
             "%s Notifyq EVENT_OPCODE_RESET eid=%lld reset_code=%d state=%d\n",
             __FUNCTION__, eid, comp->reset.reset_code, comp->reset.state));

        RtlSetBit(&lif->state, LIF_LINK_CHECK_NEEDED);

        break;
    }

    case EVENT_OPCODE_HEARTBEAT: {
        DbgTrace((TRACE_COMPONENT_NOTIFQ, TRACE_LEVEL_VERBOSE,
                  "%s Notifyq EVENT_OPCODE_HEARTBEAT eid=%lld\n", __FUNCTION__,
                  eid));
        break;
    }

    case EVENT_OPCODE_LOG: {
        DbgTrace((TRACE_COMPONENT_NOTIFQ, TRACE_LEVEL_VERBOSE,
                  "%s Notifyq EVENT_OPCODE_LOG eid=%lld\n", __FUNCTION__, eid));
        print_hex_dump("notifyq ", DUMP_PREFIX_OFFSET, 16, 1, comp->log.data,
                       sizeof(comp->log.data), true);
        break;
    }

    default:
        DbgTrace((TRACE_COMPONENT_NOTIFQ, TRACE_LEVEL_VERBOSE,
                  "%s Notifyq unknown event ecode=%d eid=%lld\n", __FUNCTION__,
                  comp->event.ecode, eid));
        break;
    }

    return true;
}

int
ionic_notifyq_clean(struct lif *lif, unsigned int budget)
{

    struct ionic_dev *idev = &lif->ionic->idev;
    struct cq *cq = &lif->notifyqcq->cq;
    u32 work_done;
    NDIS_HANDLE workItem = NULL;

    work_done =
        ionic_cq_service(cq, budget, ionic_notifyq_service, NULL, NULL, NULL);
    if (work_done)
        ionic_intr_credits(idev->intr_ctrl, cq->bound_intr->index, work_done,
                           IONIC_INTR_CRED_RESET_COALESCE);

    /* If we ran out of budget, there are more events
     * to process and napi will reschedule us soon
     */
    if (work_done == (u32)budget)
        goto return_to_napi;

    /* After outstanding events are processed we can check on
     * the link status and any outstanding interrupt credits.
     *
     * We wait until here to check on the link status in case
     * there was a long list of link events from a flap episode.
     */
    if (RtlCheckBit(&lif->state, LIF_LINK_CHECK_NEEDED)) {

        workItem = NdisAllocateIoWorkItem(lif->ionic->adapterhandle);

        if (workItem != NULL) {

            NdisQueueIoWorkItem(workItem, CheckLinkStatusCb,
                                (void *)lif->ionic);
        }
    }

return_to_napi:
    return work_done;
}