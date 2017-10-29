/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "src/lib/misc/include/maclib.h"
#include "src/lib/misc/include/bdf.h"
#include "src/sim/libsimlib/include/simserver.h"
#include "src/sim/libsimdev/src/dev_utils.h"
#include "src/sim/libsimdev/src/simdev_impl.h"

/* Supply these for ionic_if.h */
#define __packed __attribute__((packed))
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef u_int64_t dma_addr_t;

#include "src/sim/libsimdev/src/ionic_if.h"

typedef struct ethparams_s {
    int lif;
    int adq_type;
    int adq_count;
    int adq_qidbase;
    int txq_type;
    int txq_count;
    int txq_qidbase;
    int rxq_type;
    int rxq_count;
    int rxq_qidbase;
    int intr_base;
    int intr_count;
    int qidbase[8];
    u_int64_t qstate_addr;
    mac_t mac;
} ethparams_t;

typedef struct qstate_app_eth {
    uint8_t     enable;
    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
} __attribute__((packed)) qstate_app_eth_t;

typedef qstate_app_eth_t qstate_app_ethtx_t;
typedef qstate_app_eth_t qstate_app_ethrx_t;

static simdev_t *current_sd;

static int
eth_lif(simdev_t *sd)
{
    ethparams_t *ep = sd->priv;
    return ep->lif;
}

static int
eth_intrb(simdev_t *sd)
{
    ethparams_t *ep = sd->priv;
    return ep->intr_base;
}

static int
eth_intrc(simdev_t *sd)
{
    ethparams_t *ep = sd->priv;
    return ep->intr_count;
}

static u_int64_t
eth_qstate_addr(simdev_t *sd, const int type, const int qid)
{
    ethparams_t *ep = sd->priv;
    /* XXX assumes all qstate entries are 64B */
    return (ep->qstate_addr + ((ep->qidbase[type] + qid) * sizeof(qstate_t)));
}

static int
eth_read_qstate(simdev_t *sd, const int type, const int qid, qstate_t *qs)
{
    const u_int64_t addr = eth_qstate_addr(sd, type, qid);
    return simdev_read_mem(addr, qs, sizeof(qstate_t));
}

static int
eth_write_qstate(simdev_t *sd, const int type, const int qid, qstate_t *qs)
{
    const u_int64_t addr = eth_qstate_addr(sd, type, qid);
    return simdev_write_mem(addr, qs, sizeof(qstate_t));
}

static int
eth_read_txqstate(simdev_t *sd, const int txqid, qstate_t *qs)
{
    ethparams_t *ep = sd->priv;
    const int qid = ep->txq_qidbase + txqid;
    return eth_read_qstate(sd, ep->txq_type, qid, qs);
}

static int
eth_write_txqstate(simdev_t *sd, const int txqid, qstate_t *qs)
{
    ethparams_t *ep = sd->priv;
    const int qid = ep->txq_qidbase + txqid;
    return eth_write_qstate(sd, ep->txq_type, qid, qs);
}

static int
eth_read_rxqstate(simdev_t *sd, const int rxqid, qstate_t *qs)
{
    ethparams_t *ep = sd->priv;
    const int qid = ep->rxq_qidbase + rxqid;
    return eth_read_qstate(sd, ep->rxq_type, qid, qs);
}

static int
eth_write_rxqstate(simdev_t *sd, const int rxqid, qstate_t *qs)
{
    ethparams_t *ep = sd->priv;
    const int qid = ep->rxq_qidbase + rxqid;
    return eth_write_qstate(sd, ep->rxq_type, qid, qs);
}

static u_int64_t
bar_mem_rd(u_int64_t offset, u_int8_t size, void *buf)
{
    u_int8_t *b = buf;
    u_int64_t v;

    switch (size) {
    case 1: v = *(u_int8_t  *)&b[offset]; break;
    case 2: v = *(u_int16_t *)&b[offset]; break;
    case 4: v = *(u_int32_t *)&b[offset]; break;
    case 8: v = *(u_int64_t *)&b[offset]; break;
    default: v = -1; break;
    }
    return v;
}

static void
bar_mem_wr(u_int64_t offset, u_int8_t size, void *buf, u_int64_t v)
{
    u_int8_t *b = buf;

    switch (size) {
    case 1: *(u_int8_t  *)&b[offset] = v; break;
    case 2: *(u_int16_t *)&b[offset] = v; break;
    case 4: *(u_int32_t *)&b[offset] = v; break;
    case 8: *(u_int64_t *)&b[offset] = v; break;
    default: break;
    }
}

/*
 * ================================================================
 * dev_cmd regs
 * ----------------------------------------------------------------
 */

#define PACKED __attribute__((packed))

struct dev_cmd_regs {
    u_int32_t signature;
    u_int32_t done;
    u_int32_t cmd[16];
    u_int32_t response[4];
} PACKED;

static struct dev_cmd_regs dev_cmd_regs = {
    .signature = DEV_CMD_SIGNATURE,
};

static void
devcmd_nop(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    simdev_log("devcmd_nop:\n");
}

static void
devcmd_reset(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    simdev_log("devcmd_reset:\n");
}

static void
devcmd_identify(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct identify_cmd *cmd = (void *)acmd;
    struct identify_comp *comp = (void *)acomp;
    simdev_t *sd = current_sd;
    ethparams_t *ep = sd->priv;
    union identity ident = {
        .asic_type = 0x00,
        .asic_rev = 0xa0,
        .serial_num = "serial_num0001",
        .fw_version = "fwvers0002",
        .nlifs = 1,
        .ndbpgs_per_lif = 1,
        .ntxqs_per_lif = ep->txq_count,
        .nrxqs_per_lif = ep->rxq_count,
        .ncqs_per_lif = 0,
        .nrdmasqs_per_lif = 0,
        .nrdmarqs_per_lif = 0,
        .neqs_per_lif = 0,
        .nintrs = ep->intr_count,
    };

    simdev_log("devcmd_identify: addr 0x%"PRIx64" size %ld\n", 
               cmd->addr, sizeof(ident));
    if (sims_memwr(sd->fd, sd->bdf,
                   cmd->addr, sizeof(ident), &ident) < 0) {
        simdev_error("devcmd_identify: sims_memwr failed\n");
        comp->status = 1;
    }
}

static void
devcmd_lif_init(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct lif_init_cmd *cmd = (void *)acmd;

    simdev_log("devcmd_lif_init: lif %d\n", cmd->index);
}

static void
devcmd_adminq_init(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct adminq_init_cmd *cmd = (void *)acmd;
    struct adminq_init_comp *comp = (void *)acomp;
    ethparams_t *ep = current_sd->priv;

    simdev_log("devcmd_adminq_init: "
               "pid %d index %d intr_index %d lif_index %d\n"
               "    ring_size 0x%x ring_base 0x%"PRIx64"\n",
               cmd->pid,
               cmd->index,
               cmd->intr_index,
               cmd->lif_index,
               cmd->ring_size,
               cmd->ring_base);

    comp->status = 0;
    comp->qid = ep->adq_qidbase + cmd->index;
    comp->db_type = ep->adq_type;
}

static void
devcmd_txq_init(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct txq_init_cmd *cmd = (void *)acmd;
    struct txq_init_comp *comp = (void *)acomp;
    simdev_t *sd = current_sd;
    ethparams_t *ep = sd->priv;
    qstate_t qs;
    qstate_app_ethtx_t *qsethtx;

    simdev_log("devcmd_txq_init: type %d index %d "
               "ring_base 0x%"PRIx64" ring_size %d %c%c\n",
               cmd->type,
               cmd->index,
               cmd->ring_base,
               cmd->ring_size,
               cmd->I ? 'I' : '-',
               cmd->E ? 'E' : '-');

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        simdev_error("devcmd_txq_init: bad ring_size %d\n", cmd->ring_size);
        comp->status = 1;
        return;
    }
    if (cmd->index >= ep->txq_count) {
        simdev_error("devcmd_txq_init: bad qid %d\n", cmd->index);
        comp->status = 1;
        return;
    }

    memset(&qs, 0, sizeof(qs));
    qs.host = 2;
    qs.total = 2;
    qs.pid = cmd->pid;
    qsethtx = (qstate_app_ethtx_t *)qs.app_data;
    qsethtx->ring_base = cmd->ring_base;
    qsethtx->ring_size = cmd->ring_size;
    qsethtx->cq_ring_base = roundup(cmd->ring_base + (1<<cmd->ring_size),4096);
    qsethtx->enable = cmd->E;

    if (eth_write_txqstate(sd, cmd->index, &qs) < 0) {
        simdev_error("devcmd_txq_init: write_txqstate %d failed\n",
                     cmd->index);
        comp->status = 1;
        return;
    }

    comp->status = 0;
    comp->qid = ep->txq_qidbase + cmd->index;
    comp->db_type = ep->txq_type;
}

static void
devcmd_rxq_init(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct rxq_init_cmd *cmd = (void *)acmd;
    struct rxq_init_comp *comp = (void *)acomp;
    simdev_t *sd = current_sd;
    ethparams_t *ep = sd->priv;
    qstate_t qs;
    qstate_app_ethrx_t *qsethrx;

    simdev_log("devcmd_rxq_init: type %d index %d "
               "ring_base 0x%"PRIx64" ring_size %d %c%c\n",
               cmd->type,
               cmd->index,
               cmd->ring_base,
               cmd->ring_size,
               cmd->I ? 'I' : '-',
               cmd->E ? 'E' : '-');

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        simdev_error("devcmd_rxq_init: bad ring_size %d\n", cmd->ring_size);
        comp->status = 1;
        return;
    }
    if (cmd->index >= ep->rxq_count) {
        simdev_error("devcmd_rxq_init: bad qid %d\n", cmd->index);
        comp->status = 1;
        return;
    }

    memset(&qs, 0, sizeof(qs));
    qs.host = 2;
    qs.total = 2;
    qs.pid = cmd->pid;
    qsethrx = (qstate_app_ethrx_t *)qs.app_data;
    qsethrx->ring_base = cmd->ring_base;
    qsethrx->ring_size = cmd->ring_size;
    qsethrx->cq_ring_base = roundup(cmd->ring_base + (1<<cmd->ring_size),4096);
    qsethrx->enable = 1;

    if (eth_write_rxqstate(sd, cmd->index, &qs) < 0) {
        simdev_error("devcmd_rxq_init: write_rxqstate %d failed\n",
                     cmd->index);
        comp->status = 1;
        return;
    }

    comp->status = 0;
    comp->qid = ep->rxq_qidbase + cmd->index;
    comp->db_type = ep->rxq_type;
}

static void
devcmd_q_enable(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct q_enable_cmd *cmd = (void *)acmd;
    struct q_enable_comp *comp = (void *)acomp;
    simdev_t *sd = current_sd;
    const int type = cmd->db_type;
    const int qid = cmd->qid;
    qstate_t qs;
    qstate_app_ethtx_t *qsethtx;

    simdev_log("devcmd_q_enable: type %d qid %d\n", type, qid);

    if (eth_read_qstate(sd, type, qid, &qs) < 0) {
        simdev_error("devcmd_q_enable: read_qstate %d failed\n", qid);
        comp->status = 1;
        return;
    }

    qsethtx = (qstate_app_ethtx_t *)qs.app_data;
    qsethtx->enable = 1;

    assert(offsetof(qstate_app_ethtx_t, enable) == 
           offsetof(qstate_app_ethrx_t, enable));

    if (eth_write_qstate(sd, type, qid, &qs) < 0) {
        simdev_error("devcmd_q_enable: write_qstate %d failed\n", qid);
        comp->status = 1;
        return;
    }
}

static void
devcmd_q_disable(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct q_disable_cmd *cmd = (void *)acmd;
    struct q_disable_comp *comp = (void *)acomp;
    simdev_t *sd = current_sd;
    const int type = cmd->db_type;
    const int qid = cmd->qid;
    qstate_t qs;
    qstate_app_ethtx_t *qsethtx;

    simdev_log("devcmd_q_disable: type %d qid %d\n", type, qid);

    if (eth_read_qstate(sd, type, qid, &qs) < 0) {
        simdev_error("devcmd_q_enable: read_qstate %d,%d failed\n", type, qid);
        comp->status = 1;
        return;
    }

    qsethtx = (qstate_app_ethtx_t *)qs.app_data;
    qsethtx->enable = 0;

    assert(offsetof(qstate_app_ethtx_t, enable) == 
           offsetof(qstate_app_ethrx_t, enable));

    if (eth_write_qstate(sd, type, qid, &qs) < 0) {
        simdev_error("devcmd_q_enable: write_qstate %d failed\n", qid);
        comp->status = 1;
        return;
    }
}

static void
devcmd_station_mac_addr_get(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct station_mac_addr_get_comp *comp = (void *)acomp;
    simdev_t *sd = current_sd;
    ethparams_t *ep = sd->priv;

    simdev_log("devcmd_station_mac_addr_get: %s\n", mac_to_str(&ep->mac));
    memcpy(&comp->addr, &ep->mac, 6);
}

static void
devcmd_debug_q_dump(struct admin_cmd *acmd, struct admin_comp *acomp)
{
    struct debug_q_dump_cmd *cmd = (void *)acmd;
    struct debug_q_dump_comp *comp = (void *)acomp;
    simdev_t *sd = current_sd;
    const int type = cmd->qtype;
    const int qid = cmd->qid;
    qstate_t qs;

    simdev_log("devcmd_debug_q_dump: type %d qid %d\n", type, qid);

    if (type >= 8) {
        simdev_error("devcmd_debug_q_dump: bad qtype %d\n", type);
        comp->status = 1;
        return;
    }

    if (eth_read_qstate(sd, type, qid, &qs) < 0) {
        simdev_error("devcmd_debug_q_dump: read_qstate %d,%d failed\n", 
                     type, qid);
        comp->status = 1;
        return;
    }

    comp->p_index0 = qs.p_index0;
    comp->c_index0 = qs.c_index0;
    comp->p_index1 = qs.p_index1;
    comp->c_index1 = qs.c_index1;
}

static void
devcmd(struct dev_cmd_regs *dc)
{
    struct admin_cmd *cmd = (struct admin_cmd *)&dc->cmd;
    struct admin_comp *comp = (struct admin_comp *)&dc->response;

    if (dc->done) {
        simdev_error("devcmd: done set at cmd start!\n");
        comp->status = -1;
        return;
    }

    memset(comp, 0, sizeof(*comp));

    switch (cmd->opcode) {
    case CMD_OPCODE_NOP:
        devcmd_nop(cmd, comp);
        break;
    case CMD_OPCODE_RESET:
        devcmd_reset(cmd, comp);
        break;
    case CMD_OPCODE_IDENTIFY:
        devcmd_identify(cmd, comp);
        break;
    case CMD_OPCODE_LIF_INIT:
        devcmd_lif_init(cmd, comp);
        break;
    case CMD_OPCODE_ADMINQ_INIT:
        devcmd_adminq_init(cmd, comp);
        break;
    case CMD_OPCODE_TXQ_INIT:
        devcmd_txq_init(cmd, comp);
        break;
    case CMD_OPCODE_RXQ_INIT:
        devcmd_rxq_init(cmd, comp);
        break;
    case CMD_OPCODE_Q_ENABLE:
        devcmd_q_enable(cmd, comp);
        break;
    case CMD_OPCODE_Q_DISABLE:
        devcmd_q_disable(cmd, comp);
        break;
    case CMD_OPCODE_STATION_MAC_ADDR_GET:
        devcmd_station_mac_addr_get(cmd, comp);
        break;
    case CMD_OPCODE_DEBUG_Q_DUMP:
        devcmd_debug_q_dump(cmd, comp);
        break;
    default:
        simdev_error("devcmd: unknown opcode %d\n", cmd->opcode);
        comp->status = -1;
        break;
    }

    dc->done = 1;
}

/*
 * ================================================================
 * bar region handlers
 * ----------------------------------------------------------------
 */

static int
bar_invalid_rd(int bar, int reg, 
               u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    simdev_error("invalid_rd: bar %d reg %d off 0x%"PRIx64" size %d\n",
                 bar, reg, offset, size);
    return -1;
}

static int
bar_invalid_wr(int bar, int reg, 
               u_int64_t offset, u_int8_t size, u_int64_t val)
{
    simdev_error("invalid_wr: bar %d reg %d off 0x%"PRIx64" "
                 "size %d = val 0x%"PRIx64"\n",
                 bar, reg, offset, size, val);
    return -1;
}

static int
bar_devcmd_rd(int bar, int reg, 
              u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    if (offset + size > sizeof(dev_cmd_regs)) {
        simdev_error("devcmd_rd: invalid offset 0x%"PRIx64" size 0x%x\n",
                     offset, size);
        return -1;
    }

    *valp = bar_mem_rd(offset, size, &dev_cmd_regs);

    return 0;
}

static int
bar_devcmd_wr(int bar, int reg, 
              u_int64_t offset, u_int8_t size, u_int64_t val)
{
    if (offset + size >= sizeof(dev_cmd_regs)) {
        simdev_error("devcmd_rd: invalid offset 0x%"PRIx64" size 0x%x\n",
                     offset, size);
        return -1;
    }

    bar_mem_wr(offset, size, &dev_cmd_regs, val);

    return 0;
}

static int
bar_devcmddb_rd(int bar, int reg, 
                u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    /* devcmddb is write-only */
    return bar_invalid_rd(bar, reg, offset, size, valp);
}

static int
bar_devcmddb_wr(int bar, int reg, 
                u_int64_t offset, u_int8_t size, u_int64_t val)
{
    if (size != 4) {
        simdev_error("doorbell: write size %d != 4, ignoring\n", size);
        return -1;
    }
    if (offset != 0) {
        simdev_error("doorbell: write offset 0x%x, ignoring\n", size);
        return -1;
    }
    if (val != 1) {
        simdev_error("doorbell: write data 0x%"PRIx64", ignoring\n", val);
        return -1;
    }

    devcmd(&dev_cmd_regs);
    return 0;
}

static int
bar_intrctrl_rd(int bar, int reg,
                u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    u_int32_t idx = eth_intrb(current_sd);
    u_int64_t base = intr_drvcfg_addr(idx);
    u_int32_t val;

    simdev_log("intrctrl read offset 0x%"PRIx64"\n", offset);
    if (size != 4) {
        simdev_error("intrctrl read size %d != 4, ignoring\n", size);
        return -1;
    }
    if (simdev_read_reg(base + offset, &val) < 0) {
        return -1;
    }
    *valp = val;
    return 0;
}

static int
bar_intrctrl_wr(int bar, int reg,
                u_int64_t offset, u_int8_t size, u_int64_t val)
{
    u_int32_t idx = eth_intrb(current_sd);
    u_int64_t base = intr_drvcfg_addr(idx);

    simdev_log("intrctrl write offset 0x%"PRIx64"\n", offset);
    if (size != 4) {
        simdev_error("intrctrl write size %d != 4, ignoring\n", size);
        return -1;
    }
    if (simdev_write_reg(base + offset, val) < 0) {
        return -1;
    }
    return 0;
}

static int
bar_intrstatus_rd(int bar, int reg,
                  u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    u_int32_t idx = eth_intrb(current_sd);
    u_int64_t base = intr_pba_addr(idx);
    u_int32_t val;

    simdev_log("intrstatus read offset 0x%"PRIx64"\n", offset);
    if (size != 4 && size != 8) {
        simdev_error("intrctrl read size %d invalid, ignoring\n", size);
        return -1;
    }
    if (size == 8) { /* XXX */
        simdev_error("intrctrl read size %d unimplemented\n", size);
        assert(0);
    }
    if (simdev_read_reg(base + offset, &val) < 0) {
        return -1;
    }
    *valp = val;
    return 0;
}

static int
bar_intrstatus_wr(int bar, int reg,
                  u_int64_t offset, u_int8_t size, u_int64_t val)
{
    /* intrstatus reg is read-only */
    return bar_invalid_wr(bar, reg, offset, size, val);
}

static int
bar_msixtbl_rd(int bar, int reg,
               u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    u_int32_t idx = eth_intrb(current_sd);
    u_int64_t base = intr_msixcfg_addr(idx);
    u_int32_t val;

    simdev_log("msixtbl read offset 0x%"PRIx64"\n", offset);
    if (size != 4 && size != 8) {
        simdev_error("msixtbl read size %d invalid, ignoring\n", size);
        return -1;
    }
    if (size == 8) { /* XXX */
        simdev_error("msixtbl read size %d unimplemented\n", size);
        assert(0);
    }
    if (simdev_read_reg(base + offset, &val) < 0) {
        return -1;
    }
    *valp = val;
    return 0;
}

static int
bar_msixtbl_wr(int bar, int reg,
               u_int64_t offset, u_int8_t size, u_int64_t val)
{
    u_int32_t idx = eth_intrb(current_sd);
    u_int64_t base = intr_msixcfg_addr(idx);

    simdev_log("msixtbl write offset 0x%"PRIx64"\n", offset);
    if (size != 4 && size != 8) {
        simdev_error("msixtbl write size %d invalid, ignoring\n", size);
        return -1;
    }
    if (size == 8) { /* XXX */
        simdev_error("msixtbl write size %d unimplemented\n", size);
        assert(0);
    }
    if (simdev_write_reg(base + offset, val) < 0) {
        return -1;
    }
    return 0;
}

static int
bar_msixpba_rd(int bar, int reg,
               u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    u_int32_t idx = eth_lif(current_sd);
    u_int64_t base = intr_pba_addr(idx);
    u_int32_t val;

    simdev_log("msixpba read offset 0x%"PRIx64" pba_base 0x%"PRIx64"\n",
               offset, base);
    if (size != 4 && size != 8) {
        simdev_error("msixpba read size %d invalid, ignoring\n", size);
        return -1;
    }
    if (size == 8) { /* XXX */
        simdev_error("msixpba read size %d unimplemented\n", size);
        assert(0);
    }
    if (simdev_read_reg(base + offset, &val) < 0) {
        return -1;
    }
    *valp = val;
    return 0;
}

static int
bar_msixpba_wr(int bar, int reg,
               u_int64_t offset, u_int8_t size, u_int64_t val)
{
    /* msixpba is read-only */
    return bar_invalid_wr(bar, reg, offset, size, val);
}

static int
bar_db_rd(int bar, int reg,
          u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    simdev_error("doorbell read: undefined!\n");
    return -1;
}

static int
bar_db_wr(int bar, int reg,
          u_int64_t offset, u_int8_t size, u_int64_t val)
{
    u_int32_t idx = eth_lif(current_sd);
    u_int64_t base = db_host_addr(idx);
    struct doorbell {
        u16 p_index;
        u8 ring:3;
        u8 rsvd:5;
        u8 qid_lo;
        u16 qid_hi;
        u16 rsvd2;
    } PACKED db;
    u_int32_t qid;

    if (size != 8) {
        simdev_error("doorbell: write size %d != 8, ignoring\n", size);
        return -1;
    }
    if (((offset & (8-1)) != 0) ||
        offset >= sizeof(db) * 8) {
        simdev_error("doorbell: write offset 0x%"PRIx64", ignoring\n", offset);
        return -1;
    }

    *(u64 *)&db = val;
    qid = (db.qid_hi << 8) | db.qid_lo;

    simdev_log("doorbell: pid %d qid %d ring %d index %d\n",
               reg, qid, db.ring, db.p_index);
    simdev_doorbell(base + offset, val);
    return 0;
}

#define NREGS_PER_BAR   16
#define NBARS           6

typedef struct barreg_handler_s {
    int (*rd)(int bar, int reg,
              u_int64_t offset, u_int8_t size, u_int64_t *valp);
    int (*wr)(int bar, int reg,
              u_int64_t offset, u_int8_t size, u_int64_t val);
} barreg_handler_t;

static barreg_handler_t invalid_reg = { bar_invalid_rd, bar_invalid_wr };
static barreg_handler_t devcmd_reg = { bar_devcmd_rd, bar_devcmd_wr };
static barreg_handler_t devcmddb_reg = { bar_devcmddb_rd, bar_devcmddb_wr };
static barreg_handler_t intrctrl_reg = { bar_intrctrl_rd, bar_intrctrl_wr };
static barreg_handler_t intrstatus_reg = { bar_intrstatus_rd,
                                           bar_intrstatus_wr };
static barreg_handler_t msixtbl_reg = { bar_msixtbl_rd, bar_msixtbl_wr };
static barreg_handler_t msixpba_reg = { bar_msixpba_rd, bar_msixpba_wr };
static barreg_handler_t db_reg = { bar_db_rd, bar_db_wr };

typedef struct bar_handler_s {
    u_int32_t regsz;
    barreg_handler_t *regs[NREGS_PER_BAR];
} bar_handler_t;

bar_handler_t invalid_bar = {
    .regs = {
        &invalid_reg,
    },
};

bar_handler_t bar0_handler = {
    .regsz = 4096,
    .regs = {
        &devcmd_reg,
        &devcmddb_reg,
        &intrctrl_reg,
        &intrstatus_reg,
        &invalid_reg,
        &invalid_reg,
        &msixtbl_reg,
        &msixpba_reg,
    },
};

bar_handler_t bar2_handler = {
    .regsz = 4096,
    .regs = {
        &db_reg,
    },
};

bar_handler_t *bar_handlers[NBARS] = {
    &bar0_handler,
    &invalid_bar,
    &bar2_handler,
};

static bar_handler_t *
bar_handler(int bar)
{
    return (bar < NBARS) ? bar_handlers[bar] : NULL;
}

static int
barreg(bar_handler_t *b, u_int64_t offset)
{
    return b->regsz ? offset / b->regsz : 0;
}

static int
barreg_offset(bar_handler_t *b, u_int64_t offset)
{
    return offset - (barreg(b, offset) * b->regsz);
}

static barreg_handler_t *
barreg_handler(bar_handler_t *b, int reg)
{
    return (b && reg < NREGS_PER_BAR) ? b->regs[reg] : NULL;
}

static int
bar_rd(int bar, u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    bar_handler_t *b;
    barreg_handler_t *breg;
    u_int64_t regoff;
    int reg;

    b = bar_handler(bar);
    if (b == NULL) {
        simdev_error("bar_rd: unhandled bar %d\n", bar);
        return -1;
    }

    reg = barreg(b, offset);
    regoff = barreg_offset(b, offset);
    breg = barreg_handler(b, reg);
    if (breg == NULL || breg->rd == NULL) {
        simdev_error("bar_rd: unhandled reg %d\n", reg);
        return -1;
    }

    return breg->rd(bar, reg, regoff, size, valp);
}

static int
bar_wr(int bar, u_int64_t offset, u_int8_t size, u_int64_t val)
{
    bar_handler_t *b;
    barreg_handler_t *breg;
    u_int64_t regoff;
    int reg;

    b = bar_handler(bar);
    if (b == NULL) {
        simdev_error("bar_rd: unhandled bar %d\n", bar);
        return -1;
    }

    reg = barreg(b, offset);
    regoff = barreg_offset(b, offset);
    breg = barreg_handler(b, reg);
    if (breg == NULL || breg->wr == NULL) {
        simdev_error("bar_wr: unhandled reg %d\n", reg);
        return -1;
    }

    return breg->wr(bar, reg, regoff, size, val);
}

/*
 * ================================================================
 * process messages
 * ----------------------------------------------------------------
 */

static int
eth_memrd(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;

    current_sd = sd;

    if (bar_rd(bar, addr, size, valp) < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EFAULT);
        return -1;
    }
    sims_readres(sd->fd, bdf, bar, addr, size, *valp, 0);
    return 0;
}

static void
eth_memwr(simdev_t *sd, simmsg_t *m)
{
    const u_int8_t  bar  = m->u.write.bar;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    current_sd = sd;

    bar_wr(bar, addr, size, val);
}

static int
eth_iord(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;

    current_sd = sd;

    if (bar_rd(bar, addr, size, valp) < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EINVAL);
        return -1;
    }
    sims_readres(sd->fd, bdf, bar, addr, size, *valp, 0);
    return 0;
}

static void
eth_iowr(simdev_t *sd, simmsg_t *m)
{
    const int       bar  = m->u.write.bar;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    current_sd = sd;

    bar_wr(bar, addr, size, val);
}

static void
eth_init_lif(simdev_t *sd)
{
    /* anything to do for lif? */
}

static void
eth_init_intr_pba_cfg(simdev_t *sd)
{
    const u_int32_t lif = eth_lif(sd);
    const u_int32_t intrb = eth_intrb(sd);
    const u_int32_t intrc = eth_intrc(sd);

    intr_pba_cfg(lif, intrb, intrc);
}

static void
eth_init_intr_fwcfg(simdev_t *sd)
{
    const int lif = eth_lif(sd);
    const u_int32_t intrb = eth_intrb(sd);
    const u_int32_t intrc = eth_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_fwcfg_msi(intr, lif, 0);
    }
}

static void
eth_init_intr_pba(simdev_t *sd)
{
    const u_int32_t intrb = eth_intrb(sd);
    const u_int32_t intrc = eth_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_pba_clear(intr);
    }
}

static void
eth_init_intr_drvcfg(simdev_t *sd)
{
    const u_int32_t intrb = eth_intrb(sd);
    const u_int32_t intrc = eth_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_drvcfg(intr);
    }
}

static void
eth_init_intr_msixcfg(simdev_t *sd)
{
    const u_int32_t intrb = eth_intrb(sd);
    const u_int32_t intrc = eth_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_msixcfg(intr, 0, 0, 1);
    }
}

static void
eth_init_intrs(simdev_t *sd)
{
    eth_init_intr_pba_cfg(sd);
    eth_init_intr_fwcfg(sd);
    eth_init_intr_pba(sd);
    eth_init_intr_drvcfg(sd);
    eth_init_intr_msixcfg(sd);
}

static void
eth_init_device(simdev_t *sd)
{
    eth_init_lif(sd);
    eth_init_intrs(sd);
}

static int
eth_init(simdev_t *sd, const char *devparams)
{
    ethparams_t *ep;
    char qidbase[80];

    if (0) eth_read_txqstate(NULL, 0, NULL);
    if (0) eth_read_rxqstate(NULL, 0, NULL);

    if (devparam_str(devparams, "help", NULL, 0) == 0) {
        simdev_error("eth params:\n"
                     "    lif=<lif>\n"
                     "    adq_type=<adq_type>\n"
                     "    adq_count=<adq_count>\n"
                     "    adq_qidbase=<adq_qidbase>\n"
                     "    txq_type=<txq_type>\n"
                     "    txq_count=<txq_count>\n"
                     "    txq_qidbase=<txq_qidbase>\n"
                     "    rxq_type=<rxq_type>\n"
                     "    rxq_count=<rxq_count>\n"
                     "    rxq_qidbase=<rxq_qidbase>\n"
                     "    intr_base=<intr_base>\n"
                     "    intr_count=<intr_count>\n");
        return -1;
    }

    ep = calloc(1, sizeof(ethparams_t));
    if (ep == NULL) {
        simdev_error("ethparams alloc failed: no mem\n");
        return -1;
    }
    sd->priv = ep;

#define GET_PARAM(P, TYP) \
    devparam_##TYP(devparams, #P, &ep->P)

    GET_PARAM(lif, int);
    GET_PARAM(adq_type, int);
    GET_PARAM(adq_count, int);
    GET_PARAM(adq_qidbase, int);
    GET_PARAM(txq_type, int);
    GET_PARAM(txq_count, int);
    GET_PARAM(txq_qidbase, int);
    GET_PARAM(rxq_type, int);
    GET_PARAM(rxq_count, int);
    GET_PARAM(rxq_qidbase, int);
    GET_PARAM(intr_base, int);
    GET_PARAM(intr_count, int);
    GET_PARAM(qstate_addr, u64);
    GET_PARAM(mac, mac);

    /*
     * qidbase=0:1:2:3:4:5:6:7
     */
    if (devparam_str(devparams, "qidbase", qidbase, sizeof(qidbase)) == 0) {
        char *p, *q, *sp;
        int i;

        q = qidbase;
        for (i = 0; i < 8 && (p = strtok_r(q, ":", &sp)) != NULL; i++) {
            ep->qidbase[i] = strtoul(p, NULL, 0);
            if (q != NULL) q = NULL;
        }
    }

    eth_init_device(sd);
    return 0;
}

static void
eth_free(simdev_t *sd)
{
    free(sd->priv);
    sd->priv = NULL;
}

dev_ops_t eth_ops = {
    .init  = eth_init,
    .free  = eth_free,
    .cfgrd = generic_cfgrd,
    .cfgwr = generic_cfgwr,
    .memrd = eth_memrd,
    .memwr = eth_memwr,
    .iord  = eth_iord,
    .iowr  = eth_iowr,
};
