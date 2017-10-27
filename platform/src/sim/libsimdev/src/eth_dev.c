/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
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

#include "src/lib/misc/include/maclib.h"
#include "src/lib/misc/include/bdf.h"
#include "src/sim/libsimlib/include/simserver.h"
#include "src/sim/libsimdev/src/dev_utils.h"
#include "src/sim/libsimdev/src/simdev_impl.h"

typedef struct ethparams_s {
    int lif;
    int intr_base;
    int intr_count;
    mac_t mac;
} ethparams_t;

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
    .signature = 0x44455643,
};

#define DEV_CMD_NOP             0
#define DEV_CMD_RESET           1
#define DEV_CMD_IDENTIFY        2
#define DEV_CMD_LIF_INIT        3
#define DEV_CMD_ADMINQ_INIT     4
#define DEV_CMD_TXQ_INIT        5

struct admin_cmd {
    u_int16_t opcode;
    u_int16_t cmd_data[31];
} PACKED;

struct admin_comp {
    u_int32_t status:8;
    u_int32_t comp_index:24;
    u_int8_t cmd_data[11];
    u_int8_t color:1;
} PACKED;

typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef u_int64_t dma_addr_t;

union dev_identify {
    struct {
        u8 asic_type;
        u8 asic_rev;
        u8 rsvd[2];
        char serial_num[20];
        char fw_version[20];
        u32 nlifs;
        u32 ndbpgs_per_lif;
        u32 ntxqs_per_lif;
        u32 nrxqs_per_lif;
        u32 ncqs_per_lif;
        u32 nrdmasqs_per_lif;
        u32 nrdmarqs_per_lif;
        u32 neqs_per_lif;
        u32 nintrs;
    } PACKED;
    u32 words[1024];
};

struct identify_cmd {
    u16 opcode;
    u16 rsvd;
    dma_addr_t addr;
    u32 rsvd2[13];
} PACKED;

struct lif_init_cmd {
    u16 opcode;
    u16 rsrv;
    u32 index:24;
    u32 rsrv2[16];
} PACKED;

struct adminq_init_cmd {
    u16 opcode;
    u16 pid;
    u16 index;
    u16 intr_index;
    u32 lif_index:24;
    u32 ring_size:8;
    dma_addr_t ring_base;
    u32 rsvd2[11];
} PACKED;

struct adminq_init_comp {
    u32 status:8;
    u32 rsvd:24;
    u32 qid:24;
    u32 db_type:8;
    u32 rsvd2[2];
} PACKED;

struct txq_init_cmd {
    u16 opcode;
    u8 I:1;
    u8 E:1;
    u8 rsvd;
    u16 pid;
    u16 intr_index;
    u32 type:8;
    u32 index:16;
    u32 rsvd2:8;
    u32 cos:3;
    u32 ring_size:8;
    u32 rsvd3:21;
    dma_addr_t ring_base;
    u32 rsvd4[10];
} PACKED;

struct txq_init_comp {
    u32 status:8;
    u32 comp_index:24;
    u32 qid:24;
    u32 db_type:8;
    u32 rsvd;
    u32 rsrv2:24;
    u32 color:1;
    u32 rsvd3:7;
} PACKED;

static void
devcmd_nop(struct admin_cmd *cmd, struct admin_comp *comp)
{
    simdev_log("devcmd_nop:\n");
}

static void
devcmd_reset(struct admin_cmd *cmd, struct admin_comp *comp)
{
    simdev_log("devcmd_reset:\n");
}

static void
devcmd_identify(struct admin_cmd *cmd, struct admin_comp *comp)
{
    simdev_t *sd = current_sd;
    struct identify_cmd *icmd = (void *)cmd;
    union dev_identify devid = {
        .asic_type = 0x00,
        .asic_rev = 0xa0,
        .serial_num = "serial_num0001",
        .fw_version = "fwvers0002",
        .nlifs = 1,
        .ndbpgs_per_lif = 1,
        .ntxqs_per_lif = 1,
        .nrxqs_per_lif = 1,
        .ncqs_per_lif = 2,
        .nrdmasqs_per_lif = 0,
        .nrdmarqs_per_lif = 0,
        .neqs_per_lif = 0,
        .nintrs = 4,
    };

    simdev_log("devcmd_identify: addr 0x%"PRIx64" size %ld\n", 
               icmd->addr, sizeof(devid));
    if (sims_memwr(sd->fd, sd->bdf,
                   icmd->addr, sizeof(devid), &devid) < 0) {
        simdev_error("devcmd_identify: sims_memwr failed\n");
        comp->status = 1;
    }
}

static void
devcmd_lif_init(struct admin_cmd *cmd, struct admin_comp *comp)
{
    struct lif_init_cmd *lcmd = (void *)cmd;

    simdev_log("devcmd_lif_init: lif %d\n", lcmd->index);
}

static void
devcmd_adminq_init(struct admin_cmd *cmd, struct admin_comp *comp)
{
    struct adminq_init_cmd *acmd = (void *)cmd;
    struct adminq_init_comp *acomp = (void *)comp;

    simdev_log("devcmd_adminq_init: "
               "pid %d index %d intr_index %d lif_index %d\n"
               "    ring_size 0x%x ring_base 0x%"PRIx64"\n",
               acmd->pid,
               acmd->index,
               acmd->intr_index,
               acmd->lif_index,
               acmd->ring_size,
               acmd->ring_base);

    acomp->status = 0;
    acomp->qid = 3;
    acomp->db_type = 2;
}

static void
devcmd_txq_init(struct admin_cmd *cmd, struct admin_comp *comp)
{
    struct txq_init_cmd *tcmd = (void *)cmd;

    simdev_log("devcmd_txq_init: type %d index %d ring_size %d\n",
               tcmd->type,
               tcmd->index,
               tcmd->ring_size);
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
    case DEV_CMD_NOP:
        devcmd_nop(cmd, comp);
        break;
    case DEV_CMD_RESET:
        devcmd_reset(cmd, comp);
        break;
    case DEV_CMD_IDENTIFY:
        devcmd_identify(cmd, comp);
        break;
    case DEV_CMD_LIF_INIT:
        devcmd_lif_init(cmd, comp);
        break;
    case DEV_CMD_ADMINQ_INIT:
        devcmd_adminq_init(cmd, comp);
        break;
    case DEV_CMD_TXQ_INIT:
        devcmd_txq_init(cmd, comp);
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

    ep = calloc(1, sizeof(ethparams_t));
    if (ep == NULL) {
        simdev_error("ethparams alloc failed: no mem\n");
        return -1;
    }
    sd->priv = ep;

    devparam_int(devparams, "lif", &ep->lif);
    devparam_int(devparams, "intr_base", &ep->intr_base);
    devparam_int(devparams, "intr_count", &ep->intr_count);
    devparam_mac(devparams, "mac", &ep->mac);

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
