/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

#include "ionic.h"
#include "ionic_utilities.h"

extern struct ionic_driver ionic_driver;

void ionic_init_devinfo(struct ionic *ionic)
{
        struct ionic_dev *idev = &ionic->en_dev.idev;

        idev->dev_info.asic_type = ionic_readb_raw((vmk_VA)&idev->dev_info_regs->asic_type);
        idev->dev_info.asic_rev = ionic_readb_raw((vmk_VA)&idev->dev_info_regs->asic_rev);

        vmk_Memcpy(idev->dev_info.fw_version,
                   idev->dev_info_regs->fw_version,
                   IONIC_DEVINFO_FWVERS_BUFLEN);

        vmk_Memcpy(idev->dev_info.serial_num,
                   idev->dev_info_regs->serial_num,
                   IONIC_DEVINFO_SERIAL_BUFLEN);

        idev->dev_info.fw_version[IONIC_DEVINFO_FWVERS_BUFLEN] = 0;
        idev->dev_info.serial_num[IONIC_DEVINFO_SERIAL_BUFLEN] = 0;

        ionic_en_info("fw_version %s", idev->dev_info.fw_version);
}


VMK_ReturnStatus ionic_dev_setup(struct ionic *ionic)
{
        VMK_ReturnStatus status;
        struct ionic_dev_bar *bar = ionic->bars;
        unsigned int num_bars = ionic->num_bars;
        struct ionic_dev *idev = &ionic->en_dev.idev;
        u32 sig;

        /* BAR0 dev_cmd and interrupts */
        if (num_bars < 1) {
                ionic_en_err("No bars found, aborting");
                return VMK_BAD_ADDR_RANGE;
        }

        if (bar->len < IONIC_BAR0_SIZE) {
                ionic_en_err("Resource bar size %lu too small, aborting",
                             bar->len);
                return VMK_BAD_ADDR_RANGE;
        }


        idev->dev_info_regs = bar->vaddr + IONIC_BAR0_DEV_INFO_REGS_OFFSET;
        idev->dev_cmd_regs = bar->vaddr + IONIC_BAR0_DEV_CMD_REGS_OFFSET;
        idev->intr_status = bar->vaddr + IONIC_BAR0_INTR_STATUS_OFFSET;
        idev->intr_ctrl = bar->vaddr + IONIC_BAR0_INTR_CTRL_OFFSET;

        sig = ionic_readl_raw((vmk_VA)&idev->dev_info_regs->signature);
        if (sig != IONIC_DEV_INFO_SIGNATURE) {
            ionic_en_err("Incompatible firmware signature %x", sig);
            return VMK_BAD_ADDR_RANGE;
        }

        ionic_init_devinfo(ionic);

        /* BAR1: doorbells */

        bar++;
        if (num_bars < 2) {
                ionic_en_err("Doorbell bar missing, aborting");
                return VMK_BAD_ADDR_RANGE;
        }

        idev->db_pages = bar->vaddr;
        idev->phy_db_pages = bar->bus_addr;

        /* BAR2: optional controller memory mapping */

        status = ionic_mutex_create("ionic_cmb_mutex",
                                    ionic_driver.module_id,
                                    ionic_driver.heap_id,
                                    VMK_LOCKDOMAIN_INVALID,
                                    VMK_MUTEX,
                                    VMK_MUTEX_UNRANKED,
                                    &idev->cmb_inuse_lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_mutex_create() faild, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        bar++;
        if (num_bars < 3) {
                idev->phy_cmb_pages = 0;
                idev->cmb_npages = 0;
                idev->cmb_inuse = NULL;
                return status;
        }

        idev->phy_cmb_pages = bar->bus_addr;
        idev->cmb_npages = bar->len / VMK_PAGE_SIZE;
        idev->cmb_inuse = ionic_heap_zalloc(ionic_driver.heap_id,
                                            BITS_TO_LONGS(idev->cmb_npages) * sizeof(long));
        if (!idev->cmb_inuse) {
                idev->phy_cmb_pages = 0;
                idev->cmb_npages = 0;
        }

        return status;
}

void ionic_dev_clean(struct ionic *ionic)
{
        if (ionic->num_bars >= 3) {
                ionic_heap_free(ionic_driver.heap_id,
                                ionic->en_dev.idev.cmb_inuse);
                ionic->en_dev.idev.cmb_inuse = NULL;
                ionic->en_dev.idev.phy_cmb_pages = 0;
                ionic->en_dev.idev.cmb_npages = 0;
        }

        ionic_mutex_destroy(ionic->en_dev.idev.cmb_inuse_lock);
}


/* Devcmd Interface */
VMK_ReturnStatus
ionic_heartbeat_check(struct ionic *ionic)
{
        struct ionic_dev *idev = &ionic->en_dev.idev;
        vmk_uint64 hb_time;
        vmk_uint32 fw_status;
        vmk_uint32 hb;

         /* wait at least one second before testing again */
        hb_time = vmk_GetTimerCycles();
        if (IONIC_TIME_BEFORE(hb_time, (idev->last_hb_time + HZ))) {
                ionic_en_dbg("hb_time: %lu, last_hb_time + HZ: %lu",
                             hb_time, idev->last_hb_time + HZ);
                return VMK_STATUS_PENDING;
        }

         /* firmware is useful only if fw_status is non-zero */
        fw_status = ionic_readl_raw((vmk_VA)&idev->dev_info_regs->fw_status);
        if (!fw_status) {
                ionic_en_err("FW is not useable at this moment,"
                             "fw_status is: %d", fw_status);
                return VMK_FAILURE;
        }

        hb = ionic_readl_raw((vmk_VA)&idev->dev_info_regs->fw_heartbeat);
        if (!hb) {
                ionic_en_err("FW is not useable at this moment, "
                             "fw_heartbeat is: %d.", hb);
                return VMK_FAILURE;
        }

        if (hb == idev->last_hb) {
                /* only complain once for each stall seen */
                if (idev->last_hb_time != 1) {
                        ionic_en_err("FW heartbeat stalled at %d\n",
                                     idev->last_hb);
                        idev->last_hb_time = 1;
                }

                return VMK_FAILURE;
        }

        if (idev->last_hb_time == 1) {
                ionic_en_info("FW heartbeat restored at %d\n", hb);
        }

        idev->last_hb = hb;
        idev->last_hb_time = hb_time;

        return VMK_OK;
}


/* Devcmd Interface */
enum ionic_status_code
ionic_dev_cmd_status(struct ionic_dev *idev)
{
        return ionic_readb_raw((vmk_VA)&idev->dev_cmd_regs->comp.status);
}

bool ionic_dev_cmd_done(struct ionic_dev *idev)
{
        return ionic_readl_raw((vmk_VA)&idev->dev_cmd_regs->done) & IONIC_DEV_CMD_DONE;
}

void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem)
{
        union ionic_dev_cmd_comp *comp = mem;
        unsigned int i;

        for (i = 0; i < ARRAY_SIZE(comp->words); i++)
                comp->words[i] = ionic_readl_raw((vmk_VA)&idev->dev_cmd_regs->comp.words[i]);
}

void ionic_dev_cmd_go(struct ionic_dev *idev, union ionic_dev_cmd *cmd)
{
        unsigned int i;

        for (i = 0; i < ARRAY_SIZE(cmd->words); i++)
                ionic_writel_raw(cmd->words[i],
                                 (vmk_VA)&idev->dev_cmd_regs->cmd.words[i]);

        ionic_writel_raw(0, (vmk_VA)&idev->dev_cmd_regs->done);
        ionic_writel_raw(1, (vmk_VA)&idev->dev_cmd_regs->doorbell);

}

/* Device Commands */
void ionic_dev_cmd_identify(struct ionic_dev *idev, u16 ver)
{
        union ionic_dev_cmd cmd = {
                .identify.opcode = IONIC_CMD_IDENTIFY,
                .identify.ver = ver,
        };

        ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_init(struct ionic_dev *idev)
{
        union ionic_dev_cmd cmd = {
                .init.opcode = IONIC_CMD_INIT,
                .init.type = 0,
        };

        ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_reset(struct ionic_dev *idev)
{
        union ionic_dev_cmd cmd = {
                .reset.opcode = IONIC_CMD_RESET,
        };

        ionic_dev_cmd_go(idev, &cmd);
}

/* Port Commands */
void ionic_dev_cmd_port_identify(struct ionic_dev *idev)
{
        union ionic_dev_cmd cmd = {
                .port_init.opcode = IONIC_CMD_PORT_IDENTIFY,
                .port_init.index = idev->port_index,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_init(struct ionic_dev *idev)
{
        union ionic_dev_cmd cmd = {
                .port_init.opcode = IONIC_CMD_PORT_INIT,
                .port_init.index = idev->port_index,
                .port_init.info_pa = idev->port_info_pa,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_reset(struct ionic_dev *idev)
{
        union ionic_dev_cmd cmd = {
                .port_reset.opcode = IONIC_CMD_PORT_RESET,
                .port_reset.index = idev->port_index,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_state(struct ionic_dev *idev, u8 state)
{
        union ionic_dev_cmd cmd = {
                .port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
                .port_setattr.index = 0,
                .port_setattr.attr = IONIC_PORT_ATTR_STATE,
                .port_setattr.state = state,
        };

        ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_speed(struct ionic_dev *idev, uint32_t speed)
{
        union ionic_dev_cmd cmd = {
                .port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
                .port_setattr.index = idev->port_index,
                .port_setattr.attr = IONIC_PORT_ATTR_SPEED,
                .port_setattr.speed = speed,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_mtu(struct ionic_dev *idev, uint32_t mtu)
{
        union ionic_dev_cmd cmd = {
                .port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
                .port_setattr.index = idev->port_index,
                .port_setattr.attr = IONIC_PORT_ATTR_MTU,
                .port_setattr.mtu = mtu,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_autoneg(struct ionic_dev *idev, uint8_t an_enable)
{
        union ionic_dev_cmd cmd = {
                .port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
                .port_setattr.index = idev->port_index,
                .port_setattr.attr = IONIC_PORT_ATTR_AUTONEG,
                .port_setattr.an_enable = an_enable,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_fec(struct ionic_dev *idev, uint8_t fec_type)
{
        union ionic_dev_cmd cmd = {
                .port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
                .port_setattr.index = idev->port_index,
                .port_setattr.attr = IONIC_PORT_ATTR_FEC,
                .port_setattr.fec_type = fec_type,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_pause(struct ionic_dev *idev, uint8_t pause_type)
{
        union ionic_dev_cmd cmd = {
                .port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
                .port_setattr.index = idev->port_index,
                .port_setattr.attr = IONIC_PORT_ATTR_PAUSE,
                .port_setattr.pause_type = pause_type,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_port_loopback(struct ionic_dev *idev, uint8_t loopback_mode)
{
        union ionic_dev_cmd cmd = {
                .port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
                .port_setattr.index = idev->port_index,
                .port_setattr.attr = IONIC_PORT_ATTR_LOOPBACK,
                .port_setattr.loopback_mode = loopback_mode,
        };

         ionic_dev_cmd_go(idev, &cmd);
}

/* LIF commands */
void ionic_dev_cmd_lif_identify(struct ionic_dev *idev, u8 type, u8 ver)
{
    union ionic_dev_cmd cmd = {
        .lif_identify.opcode = IONIC_CMD_LIF_IDENTIFY,
        .lif_identify.type = type,
        .lif_identify.ver = ver,
    };

    ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u16 lif_index,
        dma_addr_t addr)
{
        union ionic_dev_cmd cmd = {
                .lif_init.opcode = IONIC_CMD_LIF_INIT,
                .lif_init.index = lif_index,
                .lif_init.info_pa = addr,
        };

        ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_lif_reset(struct ionic_dev *idev, u16 lif_index)
{
        union ionic_dev_cmd cmd = {
                .lif_init.opcode = IONIC_CMD_LIF_RESET,
                .lif_init.index = lif_index,
        };

        ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct qcq *qcq,
                               u16 lif_index, u16 intr_index)
{
        struct queue *q = &qcq->q;
        struct cq *cq = &qcq->cq;

        union ionic_dev_cmd cmd = {
                .q_init.opcode = IONIC_CMD_Q_INIT,
                .q_init.lif_index = lif_index,
                .q_init.type = q->type,
                .q_init.index = q->index,
                .q_init.flags = (IONIC_QINIT_F_IRQ | IONIC_QINIT_F_ENA),
                .q_init.pid = q->pid,
                .q_init.intr_index = intr_index,
                .q_init.ring_size = ionic_ilog2(q->num_descs),
                .q_init.ring_base = q->base_pa,
                .q_init.cq_ring_base = cq->base_pa,
        };

        ionic_dev_cmd_go(idev, &cmd);
}

char *ionic_dev_asic_name(u8 asic_type)
{
        switch (asic_type) {
        case IONIC_ASIC_TYPE_CAPRI:
                return "Capri";
        default:
                return "Unknown";
        }
}

struct ionic_doorbell __iomem *ionic_db_map(struct ionic_dev *idev, struct queue *q)
{
        struct ionic_doorbell __iomem *db;

        db = (void *)idev->db_pages + (q->pid * VMK_PAGE_SIZE);
        db += q->hw_type;

        return db;
}

void ionic_intr_clean(struct intr *intr)
{
        u32 credits;

        /* clear the credits by writing the current value back */
        credits = 0xffff &
                  ionic_readl_raw((vmk_VA)intr_to_credits(intr->ctrl));

        ionic_intr_return_credits(intr, credits, VMK_FALSE, VMK_TRUE);
}

int ionic_intr_init(struct ionic_dev *idev, struct intr *intr,
                    unsigned long index)
{
        intr->index = index;
        intr->ctrl = idev->intr_ctrl + index;

        ionic_intr_clean(intr);
        return 0;
}

void ionic_intr_mask_on_assertion(struct intr *intr)
{
        struct ionic_intr_ctrl ctrl = {
                .mask_on_assert = 1,
        };

        ionic_writel_raw(*(u32 *)intr_to_mask_on_assert(&ctrl),
                         (vmk_VA)intr_to_mask_on_assert(intr->ctrl));
}

void ionic_intr_return_credits(struct intr *intr, unsigned int credits,
                               bool unmask, bool reset_timer)
{
        struct ionic_intr_ctrl ctrl = {
                .int_credits = credits,
        };

    ctrl.flags |= unmask ? INTR_F_UNMASK : 0;
    ctrl.flags |= reset_timer ? INTR_F_TIMER_RESET : 0;

        ionic_writel_raw(*(u32 *)intr_to_credits(&ctrl),
                         (vmk_VA)intr_to_credits(intr->ctrl));
}

void ionic_intr_mask(struct intr *intr, bool mask)
{
        struct ionic_intr_ctrl ctrl = {
                .mask = mask ? 1 : 0,
        };

        ionic_writel_raw(*(u32 *)intr_to_mask(&ctrl),
                         (vmk_VA)intr_to_mask(intr->ctrl));
        ionic_readl_raw((vmk_VA)intr_to_mask(intr->ctrl)); /* flush write */

}

void ionic_intr_coal_set(struct intr *intr, u32 intr_coal)
{
        struct ionic_intr_ctrl ctrl = {
                .coalescing_init = intr_coal > IONIC_INTR_CTRL_COAL_MAX ?
                        IONIC_INTR_CTRL_COAL_MAX : intr_coal,
        };

        ionic_writel_raw(*(u32 *)intr_to_coal(&ctrl),
                         (vmk_VA)intr_to_coal(intr->ctrl));
        ionic_readl_raw((vmk_VA)intr_to_coal(intr->ctrl)); /* flush write */
}

VMK_ReturnStatus
ionic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
              unsigned int num_descs, size_t desc_size)
{
        struct cq_info *cur;
        unsigned int ring_size;
        unsigned int i;

        if (desc_size == 0 || !ionic_is_power_of_2(num_descs))
                return VMK_BAD_PARAM;

        ring_size = ionic_ilog2(num_descs);
        if (ring_size < 2 || ring_size > 16)
                return VMK_FAILURE;

        cq->lif = lif;
        cq->bound_intr = intr;
        cq->num_descs = num_descs;
        cq->desc_size = desc_size;
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

        return VMK_OK;
}

void ionic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa)
{
        struct cq_info *cur;
        unsigned int i;

        cq->base = base;
        cq->base_pa = base_pa;

        for (i = 0, cur = cq->info; i < cq->num_descs; i++, cur++)
                cur->cq_desc = base + (i * cq->desc_size);
}

void ionic_cq_bind(struct cq *cq, struct queue *q)
{
        // TODO support many:1 bindings using qid as index into bound_q array
        cq->bound_q = q;
}

unsigned int ionic_cq_service(struct cq *cq, unsigned int work_to_do,
                              ionic_cq_cb cb, void *cb_arg)
{
        unsigned int work_done = 0;

        if (work_to_do == 0)
                return 0;

        while (cb(cq, cq->tail, cb_arg)) {
                if (cq->tail->last)
                        cq->done_color = !cq->done_color;
                cq->tail = cq->tail->next;
                if (++work_done >= work_to_do)
                        break;
        }

        return work_done;
}

VMK_ReturnStatus
ionic_q_init(struct lif *lif, struct ionic_dev *idev, struct queue *q,
             unsigned int index, const char *base, unsigned int num_descs,
             size_t desc_size, size_t sg_desc_size, unsigned int pid)
{
        struct desc_info *cur;
        unsigned int ring_size;
        unsigned int i;

        if (desc_size == 0 || !ionic_is_power_of_2(num_descs))
                return VMK_BAD_PARAM;

        ring_size = ionic_ilog2(num_descs);
        if (ring_size < 2 || ring_size > 16)
                return VMK_FAILURE;

        q->lif = lif;
        q->idev = idev;
        q->index = index;
        q->num_descs = num_descs;
        q->desc_size = desc_size;
        q->sg_desc_size = sg_desc_size;
        q->head = q->tail = q->info;
        q->pid = pid;

        vmk_Snprintf(q->name, sizeof(q->name), "%s%u", base, index);

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

        return VMK_OK;
}

void ionic_q_map(struct queue *q, void *base, dma_addr_t base_pa)
{
        struct desc_info *cur;
        unsigned int i;

        q->base = base;
        q->base_pa = base_pa;

        for (i = 0, cur = q->info; i < q->num_descs; i++, cur++)
                cur->desc = base + (i * q->desc_size);
}

void ionic_q_sg_map(struct queue *q, void *base, dma_addr_t base_pa)
{
        struct desc_info *cur;
        unsigned int i;

        q->sg_base = base;
        q->sg_base_pa = base_pa;

        for (i = 0, cur = q->info; i < q->num_descs; i++, cur++)
                cur->sg_desc = base + (i * q->sg_desc_size);
}

void ionic_q_post(struct queue *q, bool ring_doorbell, desc_cb cb,
                  void *cb_arg)
{
        q->head->cb = cb;
        q->head->cb_arg = cb_arg;
        q->head = q->head->next;

        if (ring_doorbell) {
                struct ionic_doorbell db = {
                        .qid_lo = q->hw_index,
                        .qid_hi = q->hw_index >> 8,
                        .ring = 0,
                        .p_index = q->head->index,
                };

                ionic_writeq_raw(*(u64 *)&db, (vmk_VA)q->db);
        }
}

void ionic_q_rewind(struct queue *q, struct desc_info *start)
{
        struct desc_info *cur = start;

        while (cur != q->head) {
                if (cur->cb)
                        cur->cb(q, cur, NULL, cur->cb_arg);
                cur = cur->next;
        }

        q->head = start;
}

unsigned int ionic_q_space_avail(struct queue *q)
{
        unsigned int avail = q->tail->index;

        if (q->head->index >= avail)
                avail += q->head->left - 1;
        else
                avail -= q->head->index + 1;

        return avail;
}

bool ionic_q_has_space(struct queue *q, unsigned int want)
{
        return ionic_q_space_avail(q) >= want;
}

void ionic_q_service(struct queue *q, struct cq_info *cq_info,
                     unsigned int stop_index)
{
        struct desc_info *desc_info;

        /* check for empty queue */
        if (q->tail->index == q->head->index) {
                return;
        }

        do {
                desc_info = q->tail;
                if (desc_info->cb) {
                        desc_info->cb(q, desc_info, cq_info,
                                      desc_info->cb_arg);
                }
                desc_info->cb = NULL;
                desc_info->cb_arg = NULL;
                q->tail = q->tail->next;
        } while (desc_info->index != stop_index);
}
