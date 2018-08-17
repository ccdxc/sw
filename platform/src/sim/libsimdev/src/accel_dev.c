/*
 * Copyright (c) 2018, Pensando Systems Inc.
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
#include <pthread.h>
#include <sys/types.h>
#include <sys/param.h>

#include "src/lib/misc/include/maclib.h"
#include "src/lib/misc/include/bdf.h"
#include "src/sim/libsimlib/include/simserver.h"
#include "src/sim/libsimdev/src/dev_utils.h"
#include "src/sim/libsimdev/src/simdev_impl.h"

/* Supply these for accel_dev_if.h */
#define dma_addr_t uint64_t

#include "storage_seq_common.h"
#include "accel_ring.h"
#include "accel_dev_if.h"

typedef struct accelparams_s {
    int lif;
    int adq_type;
    int adq_count;
    int adq_qidbase;
    int seq_queue_type;
    int seq_queue_count;
    int seq_queue_base;
    int intr_base;
    int intr_count;
    u_int64_t cmb_base;
    int upd[8];
} accelparams_t;

static simdev_t *current_sd;

static pthread_cond_t *accel_cond;
static pthread_mutex_t *accel_mutex;
static pthread_t *accel_thread;
static int accel_thread_has_work;
static int accel_thread_done;

static pthread_cond_t *
accel_cond_create(void)
{
    pthread_cond_t  *cond;

    cond = calloc(1, sizeof(*cond));
    if (pthread_cond_init(cond, NULL) == 0) {
        return cond;
    }

    free(cond);
    return NULL;
}

static void
accel_cond_wait(pthread_cond_t *cond,
                pthread_mutex_t *mutex)
{
    assert(cond);
    assert(pthread_cond_wait(cond, mutex) == 0);
}

static void
accel_cond_broadcast(pthread_cond_t *cond)
{
    assert(cond);
    assert(pthread_cond_broadcast(cond) == 0);
}

static void
accel_cond_destroy(pthread_cond_t *cond)
{
    if (cond) {
		pthread_cond_destroy(cond);
        free(cond);
    }
}

static pthread_mutex_t *
accel_mutex_create(void)
{
    pthread_mutex_t  *mutex;

    mutex = calloc(1, sizeof(*mutex));
    if (pthread_mutex_init(mutex, NULL) == 0) {
        return mutex;
    }

    free(mutex);
    return NULL;
}

static void
accel_mutex_destroy(pthread_mutex_t *mutex)
{
    if (mutex) {
		pthread_mutex_destroy(mutex);
        free(mutex);
    }
}

static void
accel_mutex_lock(pthread_mutex_t *mutex)
{
    assert(mutex);
	assert(pthread_mutex_lock(mutex) == 0);
}

static void
accel_mutex_unlock(pthread_mutex_t *mutex)
{
    assert(mutex);
	assert(pthread_mutex_unlock(mutex) == 0);
}

static pthread_t *
accel_thread_create(void *(*thread_fn)(void *),
                    void* arg) 
{
    pthread_t       *handle;

    handle = calloc(1, sizeof(*handle));
	if (pthread_create(handle, NULL, thread_fn, arg) == 0) {
        return handle;
    }

    free(handle);
    return NULL;
}

static void
accel_thread_destroy(pthread_t *handle)
{
    if (handle) {
		pthread_join(*handle, NULL);
        free(handle);
    }
}

static int
accel_lif(simdev_t *sd)
{
    accelparams_t *ep = sd->priv;
    return ep->lif;
}

static int
accel_intrb(simdev_t *sd)
{
    accelparams_t *ep = sd->priv;
    return ep->intr_base;
}

static int
accel_intrc(simdev_t *sd)
{
    accelparams_t *ep = sd->priv;
    return ep->intr_count;
}

static u_int64_t
accel_cmb_base(simdev_t *sd)
{
    accelparams_t *ep = sd->priv;
    return ep->cmb_base;
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

struct dev_cmd_regs_all {
    struct dev_cmd_regs regs;
    uint8_t data[2048] __attribute__((aligned (2048)));
} PACKED;

static struct dev_cmd_regs dev_cmd_regs = {
    .signature = DEV_CMD_SIGNATURE,
};

static struct dev_cmd_regs ret_dev_cmd;

static u_int64_t accel_devcmdpa;
static u_int64_t accel_devcmddbpa;

static void
accel_devcmdpa_init(void)
{
    uint32_t    total_size;

    if (!accel_devcmdpa) {
        if (simdev_alloc_hbm_address("storage_devcmd", &accel_devcmdpa,
                                     &total_size) == 0) {
            total_size *= 1024;
            accel_devcmddbpa = accel_devcmdpa + sizeof(struct dev_cmd_regs_all);
            simdev_log("accel_devcmdpa 0x%"PRIx64" accel_devcmddbpa 0x%"PRIx64
                       " total_size %u\n", accel_devcmdpa, accel_devcmddbpa,
                       total_size);
        }
    }
}

static int
accel_devcmdpa_done_poll(void)
{
    ret_dev_cmd.done = 0;
    int maxpolls = 1000;

    if (accel_devcmdpa) {
        while (maxpolls--) {
            if (simdev_read_mem(accel_devcmdpa, &ret_dev_cmd,
                                sizeof(ret_dev_cmd)) == 0) {
                if (ret_dev_cmd.done) {
                    return 0;
                }
            }
            usleep(10000);
        }
    }
    return -1;
}

static inline void *
accel_devcmdpa_response(void)
{
    return (void *)&ret_dev_cmd.response[0];
}

static void
devcmd_nop(admin_cmd_t *acmd, admin_cpl_t *acpl)
{
    simdev_log("devcmd_nop:\n");
}

static void
devcmd_reset(admin_cmd_t *acmd, admin_cpl_t *acpl)
{
    reset_cpl_t *cpl = (void *)acpl;

    simdev_log("devcmd_reset\n");
    if (accel_devcmdpa_done_poll()) {
        simdev_error("%s: timed out\n", __FUNCTION__);
        cpl->status = 1;
    } else {
        reset_cpl_t *ret_cpl = accel_devcmdpa_response();
        cpl->status = ret_cpl->status;
    }
}

static void
devcmd_identify(admin_cmd_t *acmd, admin_cpl_t *acpl)
{
    identify_cmd_t *cmd = (void *)acmd;
    identify_cpl_t *cpl = (void *)acpl;
    simdev_t *sd = current_sd;
    identity_t ident = {0};
    int status = -1;

    simdev_log("devcmd_identify: addr 0x%"PRIx64" size %ld\n", 
               cmd->addr, sizeof(ident));
    if (accel_devcmdpa_done_poll()) {
        simdev_error("%s: timed out\n", __FUNCTION__);
    } else {
        identify_cpl_t *ret_cpl = accel_devcmdpa_response();
        cpl->status = ret_cpl->status;
        status = simdev_read_mem(accel_devcmdpa +
                                 offsetof(struct dev_cmd_regs_all, data),
                                 &ident, sizeof(ident));
    }

    if (sims_memwr(sd->fd, sd->bdf, cmd->addr, sizeof(ident), &ident) < 0) {
        simdev_error("%s: sims_memwr failed\n", __FUNCTION__);
        status = -1;
    }

    if (status) {
        cpl->status = 1;
        cpl->ver = IDENTITY_VERSION_1;
    }
}

static void
devcmd_lif_init(admin_cmd_t *acmd, admin_cpl_t *acpl)
{
    lif_init_cmd_t *cmd = (void *)acmd;
    lif_init_cpl_t *cpl = (void *)acpl;

    simdev_log("devcmd_lif_init: lif %d\n", cmd->index);
    if (accel_devcmdpa_done_poll()) {
        simdev_error("%s: timed out\n", __FUNCTION__);
        cpl->status = 1;
    } else {
        lif_init_cpl_t *ret_cpl = accel_devcmdpa_response();
        cpl->status = ret_cpl->status;
    }
}

static void
devcmd_adminq_init(admin_cmd_t *acmd, admin_cpl_t *acpl)
{
    adminq_init_cmd_t *cmd = (void *)acmd;
    adminq_init_cpl_t *cpl = (void *)acpl;

    simdev_log("devcmd_adminq_init: "
               "pid %d index %d intr_index %d lif_index %d\n"
               " wring_size 0x%x ring_base 0x%"PRIx64"\n",
               cmd->pid,
               cmd->index,
               cmd->intr_index,
               cmd->lif_index,
               cmd->ring_size,
               cmd->ring_base);

    if (accel_devcmdpa_done_poll()) {
        simdev_error("%s: timed out\n", __FUNCTION__);
        cpl->status = 1;
    } else {
        adminq_init_cpl_t *ret_cpl = accel_devcmdpa_response();
        cpl->status = ret_cpl->status;
        cpl->qid = ret_cpl->qid;
        cpl->qtype = ret_cpl->qtype;
    }
}

static void
devcmd_seq_q_init(admin_cmd_t *acmd, admin_cpl_t *acpl)
{
    seq_queue_init_cmd_t *cmd = (void *)acmd;
    seq_queue_init_cpl_t *cpl = (void *)acpl;

    simdev_log("devcmd_seq_q_init: qgroup %d index %d "
               "wring_base 0x%"PRIx64" wring_size %d\n",
               cmd->qgroup, cmd->index,
               cmd->wring_base, cmd->wring_size);

    if (accel_devcmdpa_done_poll()) {
        simdev_error("%s: timed out\n", __FUNCTION__);
        cpl->status = 1;
    } else {
        seq_queue_init_cpl_t *ret_cpl = accel_devcmdpa_response();
        cpl->status = ret_cpl->status;
        cpl->qid = ret_cpl->qid;
        cpl->qtype = ret_cpl->qtype;
    }
}

static void
devcmd_seq_q_enable(admin_cmd_t *acmd, admin_cpl_t *acpl)
{
    seq_queue_control_cmd_t *cmd = (void *)acmd;
    seq_queue_control_cpl_t *cpl = (void *)acpl;

    simdev_log("devcmd_seq_q_enable: qtype %d qid %u\n",
               cmd->qtype, cmd->qid);

    if (accel_devcmdpa_done_poll()) {
        simdev_error("%s: timed out\n", __FUNCTION__);
        cpl->status = 1;
    } else {
        seq_queue_control_cpl_t *ret_cpl = accel_devcmdpa_response();
        cpl->status = ret_cpl->status;
    }
}

static void
devcmd_seq_q_disable(admin_cmd_t *acmd, admin_cpl_t *acpl)
{
    seq_queue_control_cmd_t *cmd = (void *)acmd;
    seq_queue_control_cpl_t *cpl = (void *)acpl;

    simdev_log("devcmd_seq_q_disable: qtype %d qid %u\n",
               cmd->qtype, cmd->qid);

    if (accel_devcmdpa_done_poll()) {
        simdev_error("%s: timed out\n", __FUNCTION__);
        cpl->status = 1;
    } else {
        seq_queue_control_cpl_t *ret_cpl = accel_devcmdpa_response();
        cpl->status = ret_cpl->status;
    }
}

static void
devcmd(struct dev_cmd_regs *dc)
{
    admin_cmd_t *cmd = (admin_cmd_t *)&dc->cmd;
    admin_cpl_t *cpl = (admin_cpl_t *)&dc->response;

    if (dc->done) {
        simdev_error("devcmd: done set at cmd start!\n");
        cpl->status = -1;
        return;
    }

    memset(cpl, 0, sizeof(*cpl));

    simdev_log("opcode %u\n", cmd->opcode);
    switch (cmd->opcode) {
    case CMD_OPCODE_NOP:
        devcmd_nop(cmd, cpl);
        break;
    case CMD_OPCODE_RESET:
        devcmd_reset(cmd, cpl);
        break;
    case CMD_OPCODE_IDENTIFY:
        devcmd_identify(cmd, cpl);
        break;
    case CMD_OPCODE_LIF_INIT:
        devcmd_lif_init(cmd, cpl);
        break;
    case CMD_OPCODE_ADMINQ_INIT:
        devcmd_adminq_init(cmd, cpl);
        break;
    case CMD_OPCODE_SEQ_QUEUE_INIT:
        devcmd_seq_q_init(cmd, cpl);
        break;
    case CMD_OPCODE_SEQ_QUEUE_ENABLE:
        devcmd_seq_q_enable(cmd, cpl);
        break;
    case CMD_OPCODE_SEQ_QUEUE_DISABLE:
        devcmd_seq_q_disable(cmd, cpl);
        break;
    default:
        simdev_error("devcmd: unknown opcode %d\n", cmd->opcode);
        cpl->status = -1;
        break;
    }

    dc->done = 1;
}

/*
 * Because libsimdev/model_server takes a lock when entered,
 * a thread is needed to handle the devcmd processing with the
 * lock released so that the nicmgr process (the real devcmd
 * processor) can enter model_server upon its devcmddb poll.
 */
static void *
devcmd_thread(void *arg)
{
    accel_mutex_lock(accel_mutex);
    while (1) {
        accel_cond_wait(accel_cond, accel_mutex);

        /*
         * Process the result from Accel_PF
         */
        if (accel_thread_done) {
            break;
        }
        if (accel_thread_has_work) {
            accel_thread_has_work = 0;
            devcmd(&dev_cmd_regs);
        }
    }
    accel_mutex_unlock(accel_mutex);

    return NULL;
}

static void
devcmd_thread_create(void)
{
    accel_mutex = accel_mutex_create();
    accel_cond = accel_cond_create();
    accel_thread = accel_thread_create(&devcmd_thread, NULL);
}

static void
devcmd_thread_destroy(void)
{
    accel_thread_done = 1;
    accel_cond_broadcast(accel_cond);
    accel_thread_destroy(accel_thread);
    accel_cond_destroy(accel_cond);
    accel_mutex_destroy(accel_mutex);
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

    accel_mutex_lock(accel_mutex);
    bar_mem_wr(offset, size, &dev_cmd_regs, val);
    accel_mutex_unlock(accel_mutex);

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

    /*
     * Send command to the devcmd area of the real Accel_PF device
     */
    accel_devcmdpa_init();
    if (accel_devcmdpa) {
        u_int32_t ring_db = 1;

        accel_mutex_lock(accel_mutex);
        simdev_write_mem(accel_devcmdpa, &dev_cmd_regs, sizeof(dev_cmd_regs));
        simdev_write_mem(accel_devcmddbpa, &ring_db, sizeof(ring_db));

        accel_thread_has_work = 1;
        accel_cond_broadcast(accel_cond);
        accel_mutex_unlock(accel_mutex);
    }
    return 0;
}

static int
bar_intrctrl_rd(int bar, int reg,
                u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    u_int32_t idx = accel_intrb(current_sd);
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
    u_int32_t idx = accel_intrb(current_sd);
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
    u_int32_t idx = accel_intrb(current_sd);
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
    u_int32_t idx = accel_intrb(current_sd);
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
    u_int32_t idx = accel_intrb(current_sd);
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
    u_int32_t idx = accel_lif(current_sd);
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
    simdev_t *sd = current_sd;
    accelparams_t *ep = sd->priv;
    u_int32_t idx = accel_lif(sd);
    u_int64_t base = db_host_addr(idx);
    struct doorbell {
        u_int16_t p_index;
        u_int8_t ring:3;
        u_int8_t rsvd:5;
        u_int8_t qid_lo;
        u_int16_t qid_hi;
        u_int16_t rsvd2;
    } PACKED db;
    u_int32_t qid;
    u_int8_t qtype, upd;
    u_int32_t pid = offset >> 12;
    
    if (size != 8) {
        simdev_error("doorbell: write size %d != 8, ignoring\n", size);
        return -1;
    }
    if (((offset & (8-1)) != 0) ||
        (offset & 0xFFF) >= sizeof(db) * 8) {
        simdev_error("doorbell: write offset 0x%"PRIx64", ignoring\n", offset);
        return -1;
    }

    *(u_int64_t *)&db = val;
    qid = (db.qid_hi << 8) | db.qid_lo;

    /* set UPD bits on doorbell based on qtype */
    qtype = (offset >> 3) & 0x7;
    upd = ep->upd[qtype];
    offset = (offset & 0xFFF)|(upd << 17);

    simdev_log("doorbell: offset %lx upd 0x%x pid %d qtype %d "
               "qid %d ring %d index %d\n",
               offset, upd, pid, qtype, qid, db.ring, db.p_index);
    simdev_doorbell(base + offset, val);
    return 0;
}

static int
bar4_rd(int bar, int reg,
        u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    const u_int64_t cmb_base = accel_cmb_base(current_sd);
    const u_int64_t addr = cmb_base + offset;

    *valp = 0;
    return simdev_read_mem(addr, valp, size);
}

static int
bar4_wr(int bar, int reg,
        u_int64_t offset, u_int8_t size, u_int64_t val)
{
    const u_int64_t cmb_base = accel_cmb_base(current_sd);
    const u_int64_t addr = cmb_base + offset;

    return simdev_write_mem(addr, &val, size);
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
static barreg_handler_t bar4_reg = { bar4_rd, bar4_wr };

typedef struct bar_handler_s {
    u_int32_t regsz;
    barreg_handler_t *regs[NREGS_PER_BAR];
} bar_handler_t;

static bar_handler_t invalid_bar = {
    .regs = {
        &invalid_reg,
    },
};

static bar_handler_t bar0_handler = {
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

static bar_handler_t bar2_handler = {
    .regsz = 0,
    .regs = {
        &db_reg,
    },
};

static bar_handler_t bar4_handler = {
    .regsz = 0,
    .regs = {
        &bar4_reg,
    },
};

static bar_handler_t *bar_handlers[NBARS] = {
    &bar0_handler,
    &invalid_bar,
    &bar2_handler,
    &invalid_bar,
    &bar4_handler,
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
accel_memrd(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
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
accel_memwr(simdev_t *sd, simmsg_t *m)
{
    const u_int16_t bdf  = m->u.write.bdf;
    const u_int8_t  bar  = m->u.write.bar;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    current_sd = sd;
    bar_wr(bar, addr, size, val);
    sims_writeres(sd->fd, bdf, bar, addr, size, 0);
}

static int
accel_iord(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
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
accel_iowr(simdev_t *sd, simmsg_t *m)
{
    const u_int16_t bdf  = m->u.write.bdf;
    const int       bar  = m->u.write.bar;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    current_sd = sd;

    bar_wr(bar, addr, size, val);
    sims_writeres(sd->fd, bdf, bar, addr, size, 0);
}

static void
accel_init_lif(simdev_t *sd)
{
    /* anything to do for lif? */
}

static void
accel_init_intr_pba_cfg(simdev_t *sd)
{
    const u_int32_t lif = accel_lif(sd);
    const u_int32_t intrb = accel_intrb(sd);
    const u_int32_t intrc = accel_intrc(sd);

    intr_pba_cfg(lif, intrb, intrc);
}

static void
accel_init_intr_fwcfg(simdev_t *sd)
{
    const int lif = accel_lif(sd);
    const u_int32_t intrb = accel_intrb(sd);
    const u_int32_t intrc = accel_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_fwcfg_msi(intr, lif, 0);
    }
}

static void
accel_init_intr_pba(simdev_t *sd)
{
    const u_int32_t intrb = accel_intrb(sd);
    const u_int32_t intrc = accel_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_pba_clear(intr);
    }
}

static void
accel_init_intr_drvcfg(simdev_t *sd)
{
    const u_int32_t intrb = accel_intrb(sd);
    const u_int32_t intrc = accel_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_drvcfg(intr);
    }
}

static void
accel_init_intr_msixcfg(simdev_t *sd)
{
    const u_int32_t intrb = accel_intrb(sd);
    const u_int32_t intrc = accel_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_msixcfg(intr, 0, 0, 1);
    }
}

static void
accel_init_intrs(simdev_t *sd)
{
    accel_init_intr_pba_cfg(sd);
    accel_init_intr_fwcfg(sd);
    accel_init_intr_pba(sd);
    accel_init_intr_drvcfg(sd);
    accel_init_intr_msixcfg(sd);
}

static void
accel_init_device(simdev_t *sd)
{
    accel_init_lif(sd);
    accel_init_intrs(sd);
}

static int
accel_init(simdev_t *sd, const char *devparams)
{
    accelparams_t *ep;
    char pbuf[80];

    if (devparam_str(devparams, "help", NULL, 0) == 0) {
        simdev_error("accel params:\n"
                     "    lif=<lif>\n"
                     "    adq_type=<adq_type>\n"
                     "    adq_count=<adq_count>\n"
                     "    seq_queue_type=<seq_queue_type>\n"
                     "    seq_queue_count=<seq_queue_count>\n"
                     "    intr_base=<intr_base>\n"
                     "    intr_count=<intr_count>\n"
                     "    cmb_base=<cmb_base>\n");
        return -1;
    }

    ep = calloc(1, sizeof(accelparams_t));
    if (ep == NULL) {
        simdev_error("accelparams alloc failed: no mem\n");
        return -1;
    }
    sd->priv = ep;

#define GET_PARAM(P, TYP) \
    devparam_##TYP(devparams, #P, &ep->P)

    GET_PARAM(lif, int);
    GET_PARAM(adq_type, int);
    GET_PARAM(adq_count, int);
    GET_PARAM(seq_queue_type, int);
    GET_PARAM(seq_queue_count, int);
    GET_PARAM(intr_base, int);
    GET_PARAM(intr_count, int);
    GET_PARAM(cmb_base, u64);

    /*
     * upd=0x8:0xb:0:0:0:0:0:0
     */
    if (devparam_str(devparams, "upd", pbuf, sizeof(pbuf)) == 0) {
        char *p, *q, *sp;
        int i;

        q = pbuf;
        for (i = 0; i < 8 && (p = strtok_r(q, ":", &sp)) != NULL; i++) {
            ep->upd[i] = strtoul(p, NULL, 0);
            if (q != NULL) q = NULL;
        }
    } else {
        /* UPD defaults */
        for (int i = 0; i < 8; i++) {
            ep->upd[i] = 0xb;
        }
    }

    accel_init_device(sd);
    simdev_set_lif(ep->lif);

    devcmd_thread_create();
    return 0;
}

static void
accel_free(simdev_t *sd)
{
    devcmd_thread_destroy();
    free(sd->priv);
    sd->priv = NULL;
}

dev_ops_t accel_ops = {
    .init  = accel_init,
    .free  = accel_free,
    .cfgrd = generic_cfgrd,
    .cfgwr = generic_cfgwr,
    .memrd = accel_memrd,
    .memwr = accel_memwr,
    .iord  = accel_iord,
    .iowr  = accel_iowr,
};
