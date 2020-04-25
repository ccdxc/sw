/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>
#include <sys/time.h>

// Tell accel_dev.hpp to emumerate definitions of all devcmds
#define ACCEL_DEV_CMD_ENUMERATE  1

#include "nic/include/base.hpp"
#include "nic/include/notify.hpp"

#ifdef NICMGR_DELPHI_METRICS_ENABLE
#include "gen/proto/nicmgr/accel_metrics.pb.h"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"
#endif

#include "logger.hpp"
#include "accel_dev.hpp"
#include "accel_lif.hpp"
#include "pd_client.hpp"

namespace AccelLifUtils {

// Detection log control
#define HW_MON_DETECT_LOG_SUPPRESS_US   (5 * 60 * USEC_PER_SEC)

typedef std::pair<uint32_t,uint32_t>    mon_err_mask_code_t;
typedef std::map<const std::string,mon_err_mask_code_t> mon_status_reg_map_t;

const static mon_status_reg_map_t       mon_crypto_status_reg_map = {
    {"xts_enc_status", {CRYPTO_SYM_ERR_UNRECOV_MASK, ACCEL_LIF_REASON_XTS_ENCR_ERR_RESET}},
    {"xts_status",     {CRYPTO_SYM_ERR_UNRECOV_MASK, ACCEL_LIF_REASON_XTS_DECR_ERR_RESET}},
    {"gcm0_status",    {CRYPTO_SYM_ERR_UNRECOV_MASK, ACCEL_LIF_REASON_GCM_ENCR_ERR_RESET}},
    {"gcm1_status",    {CRYPTO_SYM_ERR_UNRECOV_MASK, ACCEL_LIF_REASON_GCM_DECR_ERR_RESET}},
};

/*
 * Note that unlike crypto, CP/DC engines are designed not to lock up due to
 * any errors (i.e., a subsequent soft reset would likely make no difference).
 * Hence, mask values below are only used for logging purposes.
 */
const static mon_status_reg_map_t       mon_cpdc_status_reg_map = {
    {"cp_int",              {CPDC_INT_ERR_LOG_MASK,    ACCEL_LIF_REASON_CP_ERR_LOG}},
    {"cp_int_ecc_error",    {CPDC_INT_ECC_LOG_MASK,    ACCEL_LIF_REASON_CP_ERR_LOG}},
    {"cp_int_axi_error_w0", {CPDC_AXI_ERR_W0_LOG_MASK, ACCEL_LIF_REASON_CP_ERR_LOG}},
    {"cp_int_axi_error_w1", {CPDC_AXI_ERR_W1_LOG_MASK, ACCEL_LIF_REASON_CP_ERR_LOG}},
    {"dc_int",              {CPDC_INT_ERR_LOG_MASK,    ACCEL_LIF_REASON_DC_ERR_LOG}},
    {"dc_int_ecc_error",    {CPDC_INT_ECC_LOG_MASK,    ACCEL_LIF_REASON_DC_ERR_LOG}},
    {"dc_int_axi_error_w0", {CPDC_AXI_ERR_W0_LOG_MASK, ACCEL_LIF_REASON_DC_ERR_LOG}},
    {"dc_int_axi_error_w1", {CPDC_AXI_ERR_W1_LOG_MASK, ACCEL_LIF_REASON_DC_ERR_LOG}},
};

typedef std::map<uint32_t,const mon_status_reg_map_t&> mon_ring2status_reg_map_t;

const static mon_ring2status_reg_map_t  mon_ring2status_reg_map = {
   {ACCEL_RING_CP,      mon_cpdc_status_reg_map},
   {ACCEL_RING_CP_HOT,  mon_cpdc_status_reg_map},
   {ACCEL_RING_DC,      mon_cpdc_status_reg_map},
   {ACCEL_RING_DC_HOT,  mon_cpdc_status_reg_map},
   {ACCEL_RING_XTS0,    mon_crypto_status_reg_map},
   {ACCEL_RING_XTS1,    mon_crypto_status_reg_map},
   {ACCEL_RING_GCM0,    mon_crypto_status_reg_map},
   {ACCEL_RING_GCM1,    mon_crypto_status_reg_map},
};

static const mon_status_reg_map_t *
mon_ring2status_reg_map_find(uint32_t ring_handle);

static const mon_err_mask_code_t *
mon_status_mask_code_find(const mon_status_reg_map_t *status_reg_map,
                          const char *reg_name);

/*
 * Devapi availability check
 */
#define MON_CB_DEVAPI_CHECK_VOID()                                              \
    if (!mon->lif.dev_api) {                                                    \
        if (mon->detection_log.Request()) {                                     \
            NIC_LOG_ERR("{}: Uninitialized devapi", mon->lif.LifNameGet());     \
        }                                                                       \
        goto out;                                                               \
    }

/*
 * Monitor accel ring group API return error checking
 */
#define MON_CB_RGROUP_GET_NO_RET_CHECK(_ret_val, _num_rings, _name)             \
    do {                                                                        \
        if (_ret_val != SDK_RET_OK) {                                           \
            if (mon->detection_log.Request()) {                                 \
                NIC_LOG_ERR("{}: failed to get " _name " for ring group",       \
                            mon->lif.LifNameGet());                             \
            }                                                                   \
        } else if (_num_rings < accel_ring_vec.size()) {                        \
            if (mon->detection_log.Request()) {                                 \
                NIC_LOG_ERR("{}: too few num_rings {} expected at least {}",    \
                    mon->lif.LifNameGet(), _num_rings, accel_ring_vec.size());  \
            }                                                                   \
        }                                                                       \
    } while (false)


/**
 * FW-Driver notify queue
 */
NotifyQ::NotifyQ(const std::string& name,
                 PdClient *pd,
                 uint64_t lif_id,
                 uint32_t tx_qtype,
                 uint32_t tx_qid,
                 uint64_t intr_base,
                 uint8_t  admin_cosA,
                 uint8_t  admin_cosB,
                 bool host_dev) :
    name(name),
    pd(pd),
    lif_id(lif_id),
    tx_qtype(tx_qtype),
    tx_qid(tx_qid),
    intr_base(intr_base),
    admin_cosA(admin_cosA),
    admin_cosB(admin_cosB),
    host_dev(host_dev),
    tx_ring_size(0),
    tx_alloc_size(0)
{
}

NotifyQ::~NotifyQ()
{
    TxQReset();
}

bool
NotifyQ::TxQInit(const notifyq_init_cmd_t *cmd,
                 uint32_t desc_size)
{
    notify_qstate_t     qstate = {0};
    uint64_t            qstate_addr;
    uint32_t            alloc_size;
    uint8_t             pc_offs;

    NIC_LOG_DEBUG("{}: NOTIFYQ desc_size {}", name, desc_size);

    /*
     * Ensure sizes are a power of 2
     */
    if (!is_power_of_2(desc_size)) {
        NIC_LOG_ERR("{}: desc_size {} not a power of 2", name, desc_size);
        return false;
    }

    tx_ring_size = 1 << cmd->ring_size;
    alloc_size = desc_size << cmd->ring_size;
    if (tx_alloc_size) {
        if (tx_alloc_size < alloc_size) {
            NIC_LOG_ERR("{}: alloc_size {} exceeds previous allocation {}",
                        name, alloc_size, tx_alloc_size);
            return false;
        }
    } else {
        tx_ring_base = pd->nicmgr_mem_alloc(alloc_size);
        if (tx_ring_base == 0) {
            NIC_LOG_ERR("{}: Failed NOTIFYQ allocate tx queue size {}",
                        name, alloc_size);
            return false;
        }
        tx_alloc_size = alloc_size;
        NIC_LOG_DEBUG("{}: NOTIFYQ tx ring_size {} queue address {:#x}", name,
                      tx_ring_size, tx_ring_base);
    }

    tx_desc_size = desc_size;
    tx_head = 0;

    qstate_addr = pd->lm_->get_lif_qstate_addr(lif_id, tx_qtype, tx_qid);
    if (qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed NOTIFYQ get qstate address", name);
        return false;
    }

    if (pd->get_pc_offset("txdma_stage0.bin", "notify_stage0",
                          &pc_offs, NULL) < 0) {
        NIC_LOG_ERR("Failed to resolve program: txdma_stage0.bin notify_stage0");
        return false;
    }

    qstate.pc_offset = pc_offs;
    qstate.cosA = admin_cosA;
    qstate.cosB = admin_cosB;
    qstate.total = 1;
    qstate.cfg.enable = 1;
    qstate.cfg.host_queue = host_dev;
    qstate.cfg.intr_enable = 1;
    qstate.ring_base = tx_ring_base;
    qstate.ring_size = cmd->ring_size;

    qstate.host_ring_base = cmd->cq_ring_base;
    if (host_dev) {
        qstate.host_ring_base |= ACCEL_PHYS_ADDR_HOST_SET(1) |
                                 ACCEL_PHYS_ADDR_LIF_SET(lif_id);
    }
    NIC_LOG_DEBUG("{}: NOTIFYQ tx qstate {:#x} host_ring_base {:#x} ",
                  name, qstate_addr, qstate.host_ring_base);

    qstate.host_ring_size = cmd->ring_size;
    qstate.host_intr_assert_index = intr_base + cmd->intr_index;
    WRITE_MEM(qstate_addr, (uint8_t *)&qstate, sizeof(qstate), 0);
    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(qstate_addr, sizeof(qstate),
                                                  P4PLUS_CACHE_INVALIDATE_TXDMA);
    return true;
}

bool
NotifyQ::TxQPost(const void *desc)
{
    notify_qstate_t     qstate;
    asic_db_addr_t      db_addr;
    uint64_t            qstate_addr;
    uint64_t            tx_db_data;
    uint64_t            desc_addr;
    uint32_t            new_p_index0;

    if (!tx_ring_base || !tx_ring_size) {
        NIC_LOG_DEBUG("{}: Failed NOTIFYQ not yet initialized", name);
        return false;
    }

    /*
     * Only post if there's room in the queue
     */
    qstate_addr = pd->lm_->get_lif_qstate_addr(lif_id, tx_qtype, tx_qid);
    READ_MEM(qstate_addr + offsetof(notify_qstate_t, c_index0),
             (uint8_t *)&qstate.c_index0,
             sizeof(qstate.c_index0), 0);
    qstate.c_index0 &= (tx_ring_size - 1);
    new_p_index0 = (tx_head + 1) & (tx_ring_size - 1);
    if (new_p_index0 != qstate.c_index0) {
        desc_addr = tx_ring_base + (tx_desc_size * tx_head);
        WRITE_MEM(desc_addr, (uint8_t *)desc, tx_desc_size, 0);

        db_addr.lif_id = lif_id;
        db_addr.q_type = tx_qtype;
        db_addr.upd = ASIC_DB_ADDR_UPD_FILL(ASIC_DB_UPD_SCHED_SET,
                        ASIC_DB_UPD_INDEX_SET_PINDEX, false);

        tx_head = new_p_index0;
        tx_db_data = ACCEL_LIF_DBDATA_SET(tx_qid, tx_head);

        PAL_barrier();
        sdk::asic::pd::asic_ring_db(&db_addr, tx_db_data);

        return true;
    }

    return false;
}

bool
NotifyQ::TxQReset(void)
{
    uint64_t                    qstate_addr;
    struct notify_cfg_qstate    cfg = {0};
    uint8_t                     pc_offs = 0;

    /*
     * Nothing to do if ring never created.
     */
    if (!tx_ring_size) {
        return true;
    }

    qstate_addr = pd->lm_->get_lif_qstate_addr(lif_id, tx_qtype, tx_qid);
    if (qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed NOTIFYQ get qstate address", name);
        return false;
    }
    NIC_LOG_DEBUG("{}: NOTIFYQ resetting qstate {:#x}", name, qstate_addr);

    WRITE_MEM(qstate_addr + offsetof(notify_qstate_t, pc_offset), &pc_offs,
              sizeof(pc_offs), 0);
    WRITE_MEM(qstate_addr + offsetof(notify_qstate_t, cfg), (uint8_t *)&cfg,
              sizeof(cfg), 0);
    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(qstate_addr, sizeof(notify_qstate_t),
                                                  P4PLUS_CACHE_INVALIDATE_TXDMA);
    tx_head = 0;

    return true;
}

/**
 * Hardware Monitor
 */
HwMonitor::HwMonitor(AccelLif& lif,
                     EV_P) :
    lif(lif),
    detection_log(HW_MON_DETECT_LOG_SUPPRESS_US),
    reason_code(ACCEL_LIF_REASON_VOID),
    EV_A(EV_A),
    err_timer_started(false)
{
    memset(&err_timer, 0, sizeof(err_timer));
}

HwMonitor::~HwMonitor()
{
    ErrPollStop();
}

/*
 * Evaluate reset/log reasons
 */
void
HwMonitor::reset_log_reason_eval(const accel_rgroup_rmisc_rsp_t& misc,
                                 uint32_t *ret_reason_code)
{
    const mon_status_reg_map_t  *reg_map;
    const accel_ring_reg_val_t  *reg_val;
    const mon_err_mask_code_t   *mask_code;
    uint32_t                    num_regs;
    uint32_t                    i;

    reg_map = mon_ring2status_reg_map_find(misc.ring_handle);
    if (reg_map) {
        num_regs = std::min(misc.num_reg_vals,
                            (uint32_t)ACCEL_RING_NUM_REGS_MAX);
        for (i = 0, reg_val = &misc.reg_val[0]; i < num_regs; i++, reg_val++) {
            mask_code = mon_status_mask_code_find(reg_map, reg_val->name);
            if (mask_code && (reg_val->val & mask_code->first)) {
                if (detection_log.Request()) {
                    NIC_LOG_ERR("HwMonitor: error detect {} {:#x} mask {:#x}",
                                reg_val->name, reg_val->val, mask_code->first);
                }
                *ret_reason_code |= mask_code->second;
            }
        }
    }
}

/*
 * Accelerator ring group misc info response callback handler
 */
void
mon_rmisc_rsp_cb(void *user_ctx,
                 const accel_rgroup_rmisc_rsp_t& misc)
{
        HwMonitor *mon = (HwMonitor *)user_ctx;
        mon->reset_log_reason_eval(misc, &mon->reason_code);
}

/*
 * Periodic HW error poll function
 */
void
HwMonitor::ErrPoll(void *obj)
{
    HwMonitor           *mon = (HwMonitor *)obj;
    uint32_t            num_rings;
    int                 ret_val;

    mon->detection_log.Enter();

    MON_CB_DEVAPI_CHECK_VOID();
    mon->reason_code = ACCEL_LIF_REASON_VOID;
    ret_val = mon->lif.dev_api->accel_rgroup_misc_get(mon->lif.LifNameGet(),
                                ACCEL_SUB_RING_ALL, mon_rmisc_rsp_cb,
                                mon, &num_rings);
    MON_CB_RGROUP_GET_NO_RET_CHECK(ret_val, num_rings, "HwMonitor_rmisc");

    /*
     * HW unrecoverable errors detected, notify host driver so it can
     * issue LIF reset if applicable.
     */
    if ((mon->reason_code & ACCEL_LIF_REASON_ALL_ERR_RESET_MASK) &&
        mon->lif.notifyq) {

        reset_event_t reset_ev = {0};
        reset_ev.ecode = EVENT_OPCODE_RESET;
        reset_ev.reason_code = mon->reason_code;
        reset_ev.eid = mon->lif.next_event_id_get();
        if (mon->detection_log.Request()) {
            NIC_LOG_DEBUG("{}: sending EVENT_OPCODE_RESET eid {} reason_code {:#x}",
                    mon->lif.LifNameGet(), reset_ev.eid, reset_ev.reason_code);
        }
        mon->lif.notifyq->TxQPost((void *)&reset_ev);

        /*
         * Self stop monitoring; a LIF reset from host driver would eventually
         * restart monitoring. Otherwise, keep it disabled if driver decided
         * not to take any concrete recovery action.
         */
        mon->ErrPollStop();
    }

out:
    mon->detection_log.Leave();
}

/*
 * Start and stop HW error polling
 */
void
HwMonitor::ErrPollStart(void)
{
    if (!err_timer_started) {
        NIC_LOG_DEBUG("{}: starting ErrPoll", lif.LifNameGet());
        evutil_timer_start(EV_A_ &err_timer, &HwMonitor::ErrPoll,
                           this, 5.0, 5.0);
        err_timer_started = true;
    }
}

void
HwMonitor::ErrPollStop(void)
{
    if (err_timer_started) {
        NIC_LOG_DEBUG("{}: stopping ErrPoll", lif.LifNameGet());
        evutil_timer_stop(EV_A_ &err_timer);
        err_timer_started = false;
    }
}

static const mon_status_reg_map_t *
mon_ring2status_reg_map_find(uint32_t ring_handle)
{
    auto iter = mon_ring2status_reg_map.find(ring_handle);
    if (iter != mon_ring2status_reg_map.end()) {
        return &iter->second;
    }
    return nullptr;
}

static const mon_err_mask_code_t *
mon_status_mask_code_find(const mon_status_reg_map_t *status_reg_map,
                          const char *reg_name)
{
    std::string         name_str;

    name_str.assign(reg_name);
    auto iter = status_reg_map->find(name_str);
    if (iter != status_reg_map->end()) {
        return &iter->second;
    }
    return nullptr;
}

/*
 * Limit amount of monitor logging to prevent log flooding.
 */
DetectionLogControl::DetectionLogControl(uint64_t duration) :
    duration(duration),
    state(DETECTION_LOG_ST_INITIAL)
{
}

void
DetectionLogControl::Enter(void)
{
    switch (state) {
    case DETECTION_LOG_ST_INITIAL:
        state = DETECTION_LOG_ST_ENTER;
        break;

    default:
        break;
    }
}

/*
 * 'Request' should only be called if the user has intention to
 * actually log an error. If the user has never detected any
 * errors, it should avoid calling this function.
 */
bool
DetectionLogControl::Request(void)
{
    switch (state) {
    case DETECTION_LOG_ST_SUPPRESS:
        if (!time_expiry_check(ts)) {
            break;
        }

        /*
         * Fall through!!!
         */

    case DETECTION_LOG_ST_ENTER:
        time_expiry_set(ts, duration);

        /*
         * Fall through!!!
         */

    case DETECTION_LOG_ST_REQUEST:
        state = DETECTION_LOG_ST_REQUEST;
        return true;

    default:
        break;
    }
    return false;
}

void
DetectionLogControl::Leave(void)
{
    switch (state) {
    case DETECTION_LOG_ST_REQUEST:
        state = DETECTION_LOG_ST_SUPPRESS;
        break;

    case DETECTION_LOG_ST_SUPPRESS:
        if (!time_expiry_check(ts)) {
            break;
        }

        /*
         * Fall through!!!
         */

    default:
        state = DETECTION_LOG_ST_INITIAL;
        break;
    }
}

} // namespace AccelLifUtils
