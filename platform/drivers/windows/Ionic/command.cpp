
#include "common.h"

void
ionic_dev_cmd_go(struct ionic_dev *idev, union dev_cmd *cmd)
{
	if( !BooleanFlagOn( idev->ionic->Flags, IONIC_FLAG_DISABLE_CMD_INTERFACE)) {
		memcpy_toio(&idev->dev_cmd_regs->cmd, cmd, sizeof(*cmd));
		iowrite32(0, &idev->dev_cmd_regs->done);
		iowrite32(1, &idev->dev_cmd_regs->doorbell);
	}
}

bool
ionic_dev_cmd_done(struct ionic_dev *idev)
{
    return ioread32(&idev->dev_cmd_regs->done) & DEV_CMD_DONE;
}

u8
ionic_dev_cmd_status(struct ionic_dev *idev)
{
    return ioread8(&idev->dev_cmd_regs->comp.comp.status);
}

NDIS_STATUS
ionic_dev_cmd_wait(struct ionic *ionic, unsigned long max_seconds)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic_dev *idev = &ionic->idev;
    int opcode;
    int done;
    LARGE_INTEGER ts1, ts2, delta, timeout;
    int err = 0;

	if( BooleanFlagOn( ionic->Flags, IONIC_FLAG_DISABLE_CMD_INTERFACE)) {
		goto exit;
	}

    /* Wait for dev cmd to complete, retrying if we get EAGAIN,
     * but don't wait any longer than max_seconds.
     */
    timeout.QuadPart = max_seconds * 1000 * 1000 * 10;

    NdisGetCurrentSystemTime(&ts1);

    opcode = idev->dev_cmd_regs->cmd.cmd.opcode;

    do {

        status = NDIS_STATUS_FAILURE;

        done = ionic_dev_cmd_done(idev);

        if (done) {

            err = ionic_dev_cmd_status(&ionic->idev);
            if (err) {
                if (err == IONIC_RC_EAGAIN) {
                    NdisGetCurrentSystemTime(&ts2);
                    delta.QuadPart = ts2.QuadPart - ts1.QuadPart;

                    if (timeout.QuadPart > delta.QuadPart)
                        continue;
                }

                DbgTrace((TRACE_COMPONENT_COMMAND, TRACE_LEVEL_ERROR,
                          "%s DEV_CMD %s (%d) error, %s (%d) failed\n",
                          __FUNCTION__, ionic_opcode_to_str((cmd_opcode)opcode),
                          opcode, ionic_error_to_str((status_code)err), err));

                status = ionic_error_to_errno((status_code)err);
                break;
            }

            for (int i = 0; i < 10; i++) {
                // Stall for time for all data to flush for now.
                // Should be
                NdisStallExecution(100);
            }

            status = NDIS_STATUS_SUCCESS;
            break;
        }

        NdisStallExecution(100);
        NdisGetCurrentSystemTime(&ts2);

        delta.QuadPart = ts2.QuadPart - ts1.QuadPart;
    } while (timeout.QuadPart > delta.QuadPart);

	if( status == NDIS_STATUS_FAILURE) {
		SetFlag( ionic->Flags, IONIC_FLAG_DISABLE_CMD_INTERFACE);
	}

exit:

    return status;
}

void
ionic_dev_cmd_reset(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};

    cmd.reset.opcode = CMD_OPCODE_RESET,

    ionic_dev_cmd_go(idev, &cmd);
}

/* Device commands */
void
ionic_dev_cmd_identify(struct ionic_dev *idev, u8 ver)
{
    union dev_cmd cmd = {0};

    cmd.identify.opcode = CMD_OPCODE_IDENTIFY;
    cmd.identify.ver = ver;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_init(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};

    cmd.init.opcode = CMD_OPCODE_INIT;
    cmd.init.type = 0;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_speed(struct ionic_dev *idev, u32 speed)
{
    union dev_cmd cmd = {0};
    cmd.port_setattr.opcode = CMD_OPCODE_PORT_SETATTR;
    cmd.port_setattr.index = 0;
    cmd.port_setattr.attr = IONIC_PORT_ATTR_SPEED;
    cmd.port_setattr.speed = cpu_to_le32(speed);

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_get_speed(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};
    cmd.port_getattr.opcode = CMD_OPCODE_PORT_GETATTR;
    cmd.port_getattr.index = 0;
    cmd.port_getattr.attr = IONIC_PORT_ATTR_SPEED;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_autoneg(struct ionic_dev *idev, u8 an_enable)
{
    union dev_cmd cmd = {0};
    cmd.port_setattr.opcode = CMD_OPCODE_PORT_SETATTR;
    cmd.port_setattr.index = 0;
    cmd.port_setattr.attr = IONIC_PORT_ATTR_AUTONEG;
    cmd.port_setattr.an_enable = an_enable;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_get_autoneg(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};
    cmd.port_getattr.opcode = CMD_OPCODE_PORT_GETATTR;
    cmd.port_getattr.index = 0;
    cmd.port_getattr.attr = IONIC_PORT_ATTR_AUTONEG;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_fec(struct ionic_dev *idev, u8 fec_type)
{
    union dev_cmd cmd = {0};
    cmd.port_setattr.opcode = CMD_OPCODE_PORT_SETATTR;
    cmd.port_setattr.index = 0;
    cmd.port_setattr.attr = IONIC_PORT_ATTR_FEC;
    cmd.port_setattr.fec_type = cpu_to_le32(fec_type);

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_get_fec(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};
    cmd.port_getattr.opcode = CMD_OPCODE_PORT_GETATTR;
    cmd.port_getattr.index = 0;
    cmd.port_getattr.attr = IONIC_PORT_ATTR_FEC;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_pause(struct ionic_dev *idev, u8 pause_type)
{
    union dev_cmd cmd = {0};
    cmd.port_setattr.opcode = CMD_OPCODE_PORT_SETATTR;
    cmd.port_setattr.index = 0;
    cmd.port_setattr.attr = IONIC_PORT_ATTR_PAUSE;
    cmd.port_setattr.pause_type = cpu_to_le32(pause_type);

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_get_pause(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};
    cmd.port_getattr.opcode = CMD_OPCODE_PORT_GETATTR;
    cmd.port_getattr.index = 0;
    cmd.port_getattr.attr = IONIC_PORT_ATTR_PAUSE;

    ionic_dev_cmd_go(idev, &cmd);
}

/* Port commands */
void
ionic_dev_cmd_port_identify(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};

    cmd.port_init.opcode = CMD_OPCODE_PORT_IDENTIFY;
    cmd.port_init.index = 0;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_init(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};
    cmd.port_init.opcode = CMD_OPCODE_PORT_INIT;
    cmd.port_init.index = 0;
    cmd.port_init.info_pa = cpu_to_le64(idev->port_info_pa);

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_state(struct ionic_dev *idev, u8 state)
{
    union dev_cmd cmd = {0};

    cmd.port_setattr.opcode = CMD_OPCODE_PORT_SETATTR;
    cmd.port_setattr.index = 0;
    cmd.port_setattr.attr = IONIC_PORT_ATTR_STATE;
    cmd.port_setattr.state = state;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_port_reset(struct ionic_dev *idev)
{
    union dev_cmd cmd = {0};

    cmd.port_reset.opcode = CMD_OPCODE_PORT_RESET;
    cmd.port_reset.index = 0;

    ionic_dev_cmd_go(idev, &cmd);
}

/* LIF commands */
void
ionic_dev_cmd_lif_identify(struct ionic_dev *idev, u8 type, u8 ver)
{
    union dev_cmd cmd = {0};
    cmd.lif_identify.opcode = CMD_OPCODE_LIF_IDENTIFY;
    cmd.lif_identify.type = type;
    cmd.lif_identify.ver = ver;

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_lif_init(struct ionic_dev *idev,
                       u16 lif_index,
                       dma_addr_t info_pa)
{
    union dev_cmd cmd = {0};

    cmd.lif_init.opcode = CMD_OPCODE_LIF_INIT;
    cmd.lif_init.index = cpu_to_le16(lif_index);
    cmd.lif_init.info_pa = cpu_to_le64(info_pa);

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_dev_cmd_comp(struct ionic_dev *idev, union dev_cmd_comp *comp)
{
    memcpy_fromio(comp, &idev->dev_cmd_regs->comp, sizeof(*comp));
}

void
ionic_q_post(struct queue *q, bool ring_doorbell, desc_cb cb, void *cb_arg)
{
    struct lif *lif = q->lif;

    q->head->cb = cb;
    q->head->cb_arg = cb_arg;
    q->head = q->head->next;

    /*
    DbgTrace((TRACE_COMPONENT_COMMAND, TRACE_LEVEL_VERBOSE_2, "%s lif=%d
    qname=%s qid=%d qtype=%d p_index=%d ringdb=%d\n",
                                                                    __FUNCTION__,
                                                                    q->lif->index,
                                                                    q->name,
                                                                    q->hw_type,
                                                                    q->hw_index,
                                                                    q->head->index,
                                                                    ring_doorbell));
    */

    if (ring_doorbell)
        ionic_dbell_ring(lif->kern_dbpage, q->hw_type,
                         q->dbval | q->head->index);
}

unsigned int
ionic_q_space_avail(struct queue *q)
{
    unsigned int avail = q->tail->index;

    if (q->head->index >= avail)
        avail += q->head->left - 1;
    else
        avail -= q->head->index + 1;

    return avail;
}

bool
ionic_q_has_space(struct queue *q, unsigned int want)
{
    return ionic_q_space_avail(q) >= want;
}

static bool
ionic_adminq_cb(struct queue *q,
                struct desc_info *desc_info,
                struct cq_info *cq_info,
                void *cb_arg,
                void *reserved,
                void *reserved2)
{
    struct ionic_admin_ctx *ctx = (struct ionic_admin_ctx *)cb_arg;
    struct admin_comp *comp = (struct admin_comp *)cq_info->cq_desc;

    UNREFERENCED_PARAMETER(reserved);
    UNREFERENCED_PARAMETER(q);
    UNREFERENCED_PARAMETER(desc_info);
    UNREFERENCED_PARAMETER(reserved2);

    if (!ctx)
        return true;

    memcpy(&ctx->comp, comp, sizeof(*comp));

    NdisSetEvent(&ctx->CompEvent);

    // dynamic_hex_dump("comp ", DUMP_PREFIX_OFFSET, 16, 1,
    //		 &ctx->comp, sizeof(ctx->comp), true);

    // complete_all(&ctx->work);

    return true;
}

NDIS_STATUS
ionic_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
    struct queue *adminq = &lif->adminqcq->q;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    // WARN_ON(in_interrupt());

    NdisAcquireSpinLock(&lif->adminq_lock);
    if (!ionic_q_has_space(adminq, 1)) {
        status = NDIS_STATUS_RESOURCES;
        goto err_out;
    }

    status = ionic_heartbeat_check(lif->ionic);
    if (status != NDIS_STATUS_SUCCESS)
        goto err_out;

    memcpy(adminq->head->desc, &ctx->cmd, sizeof(ctx->cmd));

    // dynamic_hex_dump("cmd ", DUMP_PREFIX_OFFSET, 16, 1,
    //		 &ctx->cmd, sizeof(ctx->cmd), true);

    ionic_q_post(adminq, true, ionic_adminq_cb, ctx);

err_out:
    NdisReleaseSpinLock(&lif->adminq_lock);

    return status;
}

static void
ionic_adminq_flush(struct lif *lif)
{
    struct queue *adminq = &lif->adminqcq->q;

    NdisAcquireSpinLock(&lif->adminq_lock);

    while (adminq->tail != adminq->head) {
        memset(adminq->tail->desc, 0, sizeof(union adminq_cmd));
        adminq->tail->cb = NULL;
        adminq->tail->cb_arg = NULL;
        adminq->tail = adminq->tail->next;
    }
    NdisReleaseSpinLock(&lif->adminq_lock);
}

static NDIS_STATUS
ionic_adminq_check_err(struct lif *lif,
                       struct ionic_admin_ctx *ctx,
                       bool timeout)
{
    //	struct net_device *netdev = lif->netdev;
    const char *opcode_str;
    const char *status_str;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    if (ctx->comp.comp.status || timeout) {
        opcode_str = ionic_opcode_to_str((cmd_opcode)ctx->cmd.cmd.opcode);
        status_str = ionic_error_to_str((status_code)ctx->comp.comp.status);
        status = timeout
                     ? NDIS_STATUS_INVALID_DATA
                     : ionic_error_to_errno((status_code)ctx->comp.comp.status);

        DbgTrace((TRACE_COMPONENT_COMMAND, TRACE_LEVEL_ERROR,
                  "%s: %s (%d) failed: %s (%d)\n", __FUNCTION__, opcode_str,
                  ctx->cmd.cmd.opcode, timeout ? "TIMEOUT" : status_str,
                  status));
        IoPrint("%s: %s (%d) failed: %s (%d) code 0x%08lX\n", __FUNCTION__,
                opcode_str, ctx->cmd.cmd.opcode,
                timeout ? "TIMEOUT" : status_str, status,
                ctx->comp.comp.status);

        if (timeout)
            ionic_adminq_flush(lif);
    }

    return status;
}

NDIS_STATUS
ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx)
{
    unsigned long remaining = 0;
    const char *name;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

	if (BooleanFlagOn(lif->ionic->Flags, IONIC_FLAG_DISABLE_ADMINQ_INTERFACE)) {
		goto exit;
	}

    NdisInitializeEvent(&ctx->CompEvent);

    status = ionic_adminq_post(lif, ctx);
    if (status != NDIS_STATUS_SUCCESS) {
        name = ionic_opcode_to_str((cmd_opcode)ctx->cmd.cmd.opcode);
        DbgTrace((TRACE_COMPONENT_COMMAND, TRACE_LEVEL_ERROR,
                  "%s Posting of %s (%d) failed: %08lX\n", __FUNCTION__, name,
                  ctx->cmd.cmd.opcode, status));
        return status;
    }

    if (!NdisWaitEvent(&ctx->CompEvent, IONIC_ADMINQ_WAIT_TIME)) {
        DbgTrace((TRACE_COMPONENT_COMMAND, TRACE_LEVEL_ERROR,
                  "%s Timed out waiting on comp event\n", __FUNCTION__));
        remaining = 0;
		/* If we timed out then don't try more AdminQ commands */
		SetFlag( lif->ionic->Flags, IONIC_FLAG_DISABLE_ADMINQ_INTERFACE);
    } else {
        remaining = 1;
    }

    status = ionic_adminq_check_err(lif, ctx, (remaining == 0));

    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_COMMAND, TRACE_LEVEL_ERROR,
                  "%s AdminQ request complete status %08lX\n", __FUNCTION__,
                  status));
    }

exit:

    return status;
}

void
ionic_dev_cmd_lif_reset(struct ionic_dev *idev, u16 lif_index)
{
    union dev_cmd cmd = {0};
    cmd.lif_init.opcode = CMD_OPCODE_LIF_RESET;
    cmd.lif_init.index = cpu_to_le16(lif_index);

    ionic_dev_cmd_go(idev, &cmd);
}

void
ionic_lif_reset(struct lif *lif)
{
    struct ionic_dev *idev = &lif->ionic->idev;

    NdisAcquireSpinLock(&lif->ionic->dev_cmd_lock);
    ionic_dev_cmd_lif_reset(idev, (u16)lif->index);
    ionic_dev_cmd_wait(lif->ionic, devcmd_timeout);
    NdisReleaseSpinLock(&lif->ionic->dev_cmd_lock);
}

void
ionic_dev_cmd_adminq_init(struct ionic_dev *idev,
                          struct qcq *qcq,
                          u16 lif_index,
                          u16 intr_index)
{
    struct queue *q = &qcq->q;
    struct cq *cq = &qcq->cq;

    union dev_cmd cmd = {0};

    cmd.q_init.opcode = CMD_OPCODE_Q_INIT;
    cmd.q_init.lif_index = cpu_to_le16(lif_index);
    cmd.q_init.type = (u8)q->type;
    cmd.q_init.index = cpu_to_le32(q->index);
    cmd.q_init.flags = cpu_to_le16(IONIC_QINIT_F_IRQ | IONIC_QINIT_F_ENA);
    cmd.q_init.pid = (__le16)cpu_to_le16(q->pid);
    cmd.q_init.intr_index = cpu_to_le16(intr_index);
    cmd.q_init.ring_size = (u8)ilog2(q->num_descs);
    cmd.q_init.ring_base = cpu_to_le64(q->base_pa);
    cmd.q_init.cq_ring_base = cpu_to_le64(cq->base_pa);

    ionic_dev_cmd_go(idev, &cmd);
}
