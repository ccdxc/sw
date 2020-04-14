//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// FTE Flow Table Library device handling
///
//----------------------------------------------------------------------------

#include "ftl_dev_impl.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/athena/include/p4pd.h"
#include "nic/apollo/core/event.hpp"

namespace ftl_dev_impl {

static rte_atomic16_t       module_inited = RTE_ATOMIC16_INIT(0);
static rte_atomic16_t       lif_init_initiated = RTE_ATOMIC16_INIT(0);
static rte_atomic16_t       lif_init_completed = RTE_ATOMIC16_INIT(0);
static rte_spinlock_t       tmo_cfg_lock = RTE_SPINLOCK_INITIALIZER;

static FtlDev               *ftl_dev;
static FtlLif               *ftl_lif;
static dev_identity_t       dev_ident;
static lif_identity_t       lif_ident;
static lif_queues_ctl_t     *queues_ctl[FTL_QTYPE_MAX];

static platform_type_t      platform_type;

static inline queue_identity_t *
pollers_qident(void)
{
    return &lif_ident.base.qident[FTL_QTYPE_POLLER];
}

static inline queue_identity_t *
session_scanners_qident(void)
{
    return &lif_ident.base.qident[FTL_QTYPE_SCANNER_SESSION];
}

static inline queue_identity_t *
conntrack_scanners_qident(void)
{
    return &lif_ident.base.qident[FTL_QTYPE_SCANNER_CONNTRACK];
}

static inline queue_identity_t *
qtype_qident(enum ftl_qtype qtype)
{
    return qtype < FTL_QTYPE_MAX ?
           &lif_ident.base.qident[qtype] : nullptr;
}

static inline lif_queues_ctl_t *
pollers(void)
{
    return queues_ctl[FTL_QTYPE_POLLER];
}

static inline lif_queues_ctl_t *
session_scanners(void)
{
    return queues_ctl[FTL_QTYPE_SCANNER_SESSION];
}

static inline lif_queues_ctl_t *
conntrack_scanners(void)
{
    return queues_ctl[FTL_QTYPE_SCANNER_CONNTRACK];
}

static inline lif_queues_ctl_t *
mpu_timestamp_ctl(void)
{
    return queues_ctl[FTL_QTYPE_MPU_TIMESTAMP];
}

static inline lif_queues_ctl_t *
qtype_queue_ctl(enum ftl_qtype qtype)
{
    return qtype < FTL_QTYPE_MAX ?
           queues_ctl[qtype] : nullptr;
}


static pds_ret_t dev_identify(void);
static pds_ret_t lif_init(void);
static pds_ret_t attr_age_tmo_set(enum lif_attr attr,
                                  const pds_flow_age_timeouts_t *attr_age_tmo);
static pds_ret_t attr_age_tmo_get(enum lif_attr attr,
                                  pds_flow_age_timeouts_t *attr_age_tmo);
static pds_ret_t force_expired_ts_set(enum lif_attr attr,
                                      bool force_expired_ts);
static pds_ret_t metrics_get(enum ftl_qtype qtype,
                             lif_attr_metrics_t *metrics);
static pds_ret_t pollers_alloc(enum ftl_qtype qtype);
static pds_ret_t scanners_alloc(enum ftl_qtype qtype);
static pds_ret_t mpu_timestamp_ctl_alloc(enum ftl_qtype qtype);

static void queues_lock_all(void *user_arg,
                            uint32_t idx);
static void queues_unlock_all(void *user_arg,
                              uint32_t idx);
static void scanners_lock_all(void *user_arg,
                              uint32_t idx);
static void scanners_unlock_all(void *user_arg,
                                uint32_t idx);
static void scanners_lock_one(void *user_arg,
                              uint32_t idx);
static void scanners_unlock_one(void *user_arg,
                                uint32_t idx);
static void pollers_lock_all(void *user_arg,
                             uint32_t idx);
static void pollers_unlock_all(void *user_arg,
                               uint32_t idx);
static void pollers_lock_one(void *user_arg,
                             uint32_t idx);
static void pollers_unlock_one(void *user_arg,
                               uint32_t idx);
static void age_tmo_cfg_lock(void *user_arg,
                             uint32_t idx);
static void age_tmo_cfg_unlock(void *user_arg,
                               uint32_t idx);
static pds_ret_t ftl_status_2_pds_ret(ftl_status_code_t ftl_status);


pds_ret_t
init(void)
{
    DeviceManager       *devmgr_if;
    pds_ret_t           ret = PDS_RET_OK;

    if (rte_atomic16_test_and_set(&module_inited)) {
        platform_type = api::g_pds_state.platform_type();
        PDS_TRACE_DEBUG("One-time initialization platform_type %d",
                        platform_type);
        devmgr_if = nicmgr::nicmgrapi::devmgr_if();
        SDK_ASSERT_TRACE_RETURN(devmgr_if, PDS_RET_ENTRY_NOT_FOUND,
                                "failed to locate devmgr_if");

        ftl_dev = (FtlDev *)devmgr_if->GetDevice("ftl");
        SDK_ASSERT_TRACE_RETURN(ftl_dev, PDS_RET_ENTRY_NOT_FOUND,
                                "failed to locate ftl device");
        ret = dev_identify();
        if (ret == PDS_RET_OK) {
            ret = lif_init();
        }
    }
    return ret;
}

void
fini(void)
{
    if (lif_init_done()) {
        scanners_stop(true);
        if (mpu_timestamp_ctl()) {
            mpu_timestamp_ctl()->stop(true);
        }
        pollers_flush();
    }
}

pds_ret_t
scanners_start(void)
{
    pds_ret_t       ret = PDS_RET_RETRY;

    if (lif_init_done()) {

        /*
         * Scanners_start in nicmgr will start all scanner types (session and
         * conntrack scanners) regardless of how it is invoked below.
         */
        ret = PDS_RET_OK;
        if (session_scanners()) {
            ret = session_scanners()->start();
        } else if (conntrack_scanners()) {
            ret = conntrack_scanners()->start();
        }
    }
    return ret;
}

pds_ret_t
scanners_stop(bool quiesce_check)
{
    pds_ret_t       ret = PDS_RET_RETRY;

    if (lif_init_done()) {

        /*
         * Scanners_stop in nicmgr will stop all scanner types (session and
         * conntrack scanners) regardless of how it is invoked below.
         */
        ret = PDS_RET_OK;
        if (session_scanners()) {
            ret = session_scanners()->stop(quiesce_check);
        } else if (conntrack_scanners()) {
            ret = conntrack_scanners()->stop(quiesce_check);
        }
    }
    return ret;
}

pds_ret_t
scanners_start_single(enum ftl_qtype qtype,
                      uint32_t qid)
{
    lif_queues_ctl_t    *qctl;
    pds_ret_t           ret = PDS_RET_INVALID_ARG;

    /*
     * start-single is expected to be called well after init time
     * (since it would only be called from an active poller)
     * so no need to incur the lif_init_done test.
     */
    qctl = qtype_queue_ctl(qtype);
    if (qctl) {
        ret = qctl->start_single(qid);
    }
    return ret;
}

pds_ret_t
pollers_qcount_get(uint32_t *ret_qcount)
{
    *ret_qcount = 0;
    if (!lif_init_done()) {
        return PDS_RET_RETRY;
    }
    if (pollers()) {
        *ret_qcount = pollers()->qcount_get();
    }
    return PDS_RET_OK;
}

pds_ret_t
pollers_flush(void)
{
    pds_ret_t       ret = PDS_RET_OK;

    if (!lif_init_done()) {
        return PDS_RET_RETRY;
    }
    if (pollers()) {
        ret = pollers()->flush();
    }
    return ret;
}

pds_ret_t
pollers_dequeue_burst(uint32_t qid,
                      poller_slot_data_t *slot_data_buf,
                      uint32_t slot_data_buf_sz,
                      uint32_t *burst_count)
{
    pds_ret_t       ret = PDS_RET_NOOP;

    if (!lif_init_done()) {
        return PDS_RET_RETRY;
    }
    if (pollers()) {
        ret = pollers()->dequeue_burst(qid, slot_data_buf,
                                       slot_data_buf_sz, burst_count);
    }
    return ret;
}

pds_ret_t
normal_timeouts_set(const pds_flow_age_timeouts_t *age_tmo)
{
    return attr_age_tmo_set(FTL_LIF_ATTR_NORMAL_AGE_TMO, age_tmo);
}

pds_ret_t
normal_timeouts_get(pds_flow_age_timeouts_t *age_tmo)
{
    return attr_age_tmo_get(FTL_LIF_ATTR_NORMAL_AGE_TMO, age_tmo);
}

pds_ret_t
accel_timeouts_set(const pds_flow_age_timeouts_t *age_tmo)
{
    return attr_age_tmo_set(FTL_LIF_ATTR_ACCEL_AGE_TMO, age_tmo);
}

pds_ret_t
accel_timeouts_get(pds_flow_age_timeouts_t *age_tmo)
{
    return attr_age_tmo_get(FTL_LIF_ATTR_ACCEL_AGE_TMO, age_tmo);
}

pds_ret_t
accel_aging_control(bool enable_sense)
{
    devcmd_t        devcmd(ftl_lif, age_tmo_cfg_lock, age_tmo_cfg_unlock);

    devcmd.req().accel_aging_ctl.opcode = FTL_DEVCMD_OPCODE_ACCEL_AGING_CONTROL;
    devcmd.req().accel_aging_ctl.enable_sense = enable_sense;
    return devcmd.submit();
}

pds_ret_t
force_session_expired_ts_set(bool force_expired_ts)
{
    return force_expired_ts_set(FTL_LIF_ATTR_FORCE_SESSION_EXPIRED_TS,
                                force_expired_ts);
}

pds_ret_t
force_conntrack_expired_ts_set(bool force_expired_ts)
{
    return force_expired_ts_set(FTL_LIF_ATTR_FORCE_CONNTRACK_EXPIRED_TS,
                                force_expired_ts);
}

pds_ret_t
session_scanners_metrics_get(lif_attr_metrics_t *metrics)
{
    return metrics_get(FTL_QTYPE_SCANNER_SESSION, metrics);
}

pds_ret_t
conntrack_scanners_metrics_get(lif_attr_metrics_t *metrics)
{
    return metrics_get(FTL_QTYPE_SCANNER_CONNTRACK, metrics);
}

pds_ret_t
pollers_metrics_get(lif_attr_metrics_t *metrics)
{
    return metrics_get(FTL_QTYPE_POLLER, metrics);
}

pds_ret_t
timestamp_metrics_get(lif_attr_metrics_t *metrics)
{
    return metrics_get(FTL_QTYPE_MPU_TIMESTAMP, metrics);
}

pds_ret_t
session_table_depth_get(uint32_t *ret_table_depth)
{
    *ret_table_depth = session_scanners() ? 
                       session_scanners()->table_depth() : 0;
    return PDS_RET_OK;
}

pds_ret_t
conntrack_table_depth_get(uint32_t *ret_table_depth)
{
    *ret_table_depth = conntrack_scanners() ? 
                       conntrack_scanners()->table_depth() : 0;
    return PDS_RET_OK;
}

bool
lif_init_done(void)
{
    return !!rte_atomic16_read(&lif_init_completed);
}

uint64_t
mpu_timestamp(void)
{
    return ftl_lif ? ftl_lif->mpu_timestamp() : 0;
}

static pds_ret_t
dev_identify(void)
{
    devcmd_t        devcmd(ftl_dev);
    pds_ret_t       ret;

    devcmd.req().dev_identify.opcode = FTL_DEVCMD_OPCODE_IDENTIFY;
    devcmd.req().dev_identify.type = FTL_LIF_TYPE_BASE;
    devcmd.req().dev_identify.ver = IDENTITY_VERSION_1;

    ret = devcmd.submit(nullptr, &dev_ident);
    SDK_ASSERT_TRACE_RETURN(ret == PDS_RET_OK, ret,
                            "failed device identify");
    SDK_ASSERT_TRACE_RETURN(dev_ident.base.version == IDENTITY_VERSION_1,
                            PDS_RET_ERR, "unexpected device version %d",
                            dev_ident.base.version);
    SDK_ASSERT_TRACE_RETURN(dev_ident.base.nlifs, PDS_RET_ERR,
                            "no device LIFs");
    ftl_lif = ftl_dev->LifFind(0);
    SDK_ASSERT_TRACE_RETURN(ftl_lif, PDS_RET_ERR,
                            "LIF at index 0 not found");
    return ret;
}

static pds_ret_t
lif_init(void)
{
    devcmd_t        devcmd(ftl_lif);
    pds_ret_t       ret = PDS_RET_OK;

    if (rte_atomic16_test_and_set(&lif_init_initiated)) {
        PDS_TRACE_DEBUG("One-time LIF initialization");

        devcmd.req().lif_identify.opcode = FTL_DEVCMD_OPCODE_LIF_IDENTIFY;
        devcmd.req().lif_identify.type = FTL_LIF_TYPE_BASE;
        devcmd.req().lif_identify.ver = IDENTITY_VERSION_1;
        ret = devcmd.submit_with_retry(nullptr, &lif_ident);
        SDK_ASSERT_TRACE_RETURN(ret == PDS_RET_OK, ret,
                                "failed LIF identify");
        SDK_ASSERT_TRACE_RETURN(lif_ident.base.version == IDENTITY_VERSION_1,
                                PDS_RET_ERR, "unexpected LIF version %d",
                                lif_ident.base.version);
        devcmd.req_clr();
        devcmd.req().lif_init.opcode = FTL_DEVCMD_OPCODE_LIF_INIT;
        ret = devcmd.submit_with_retry();
        SDK_ASSERT_TRACE_RETURN(ret == PDS_RET_OK, ret,
                                "failed LIF init");
        devcmd.req_clr();
        devcmd.req().lif_reset.opcode = FTL_DEVCMD_OPCODE_LIF_RESET;
        ret = devcmd.submit_with_retry();
        SDK_ASSERT_TRACE_RETURN(ret == PDS_RET_OK, ret,
                                "failed LIF reset");
        /*
         * Allocate poller/scanners before initializing them
         * (in order to provide proper devcmd locking)
         */
        ret = pollers_alloc(FTL_QTYPE_POLLER);
        if (ret == PDS_RET_OK) {
            ret = scanners_alloc(FTL_QTYPE_SCANNER_SESSION);
        }
        if (ret == PDS_RET_OK) {
            ret = scanners_alloc(FTL_QTYPE_SCANNER_CONNTRACK);
        }
        if (ret == PDS_RET_OK) {
            ret = mpu_timestamp_ctl_alloc(FTL_QTYPE_MPU_TIMESTAMP);
        }

        /*
         * Pre-lock and share the same devcmd block across all queue inits.
         */
        if (ret == PDS_RET_OK) {
            devcmd_t devcmd_qinit(ftl_lif, queues_lock_all, queues_unlock_all);

            devcmd_qinit.owner_pre_lock();
            if (pollers() && (ret == PDS_RET_OK)) {
                ret = pollers()->init(&devcmd_qinit);
            }
            if (session_scanners() && (ret == PDS_RET_OK)) {
                ret = session_scanners()->init(&devcmd_qinit);
            }
            if (conntrack_scanners() && (ret == PDS_RET_OK)) {
                ret = conntrack_scanners()->init(&devcmd_qinit);
            }
            if (mpu_timestamp_ctl() && (ret == PDS_RET_OK)) {
                ret = mpu_timestamp_ctl()->init(&devcmd_qinit);
            }

            /*
             * Scanners are always started by default unless we're in
             * SIM mode, in which case, a test program will start them.
             */
            if (platform_is_hw(platform_type)) {
                if (mpu_timestamp_ctl() && (ret == PDS_RET_OK)) {
                    ret = mpu_timestamp_ctl()->start(&devcmd_qinit);
                }
#if 0
                /*
                 * Keep disabled for now until flow cache (hash) deletion
                 * is implemented for aging.
                 */
                if (session_scanners() && (ret == PDS_RET_OK)) {
                    ret = session_scanners()->start(&devcmd_qinit);
                }
                if (conntrack_scanners() && (ret == PDS_RET_OK)) {
                    ret = conntrack_scanners()->start(&devcmd_qinit);
                }
#endif
            }
            devcmd_qinit.owner_pre_unlock();
        }
        rte_atomic16_set(&lif_init_completed, 1);
    }
    return ret;
}

static pds_ret_t
attr_age_tmo_set(enum lif_attr attr,
                 const pds_flow_age_timeouts_t *attr_age_tmo)
{
    devcmd_t        devcmd(ftl_lif, age_tmo_cfg_lock, age_tmo_cfg_unlock);

    devcmd.req().lif_setattr.opcode = FTL_DEVCMD_OPCODE_LIF_SETATTR;
    devcmd.req().lif_setattr.attr = attr;
    devcmd.req().lif_setattr.age_tmo = *attr_age_tmo;
    return devcmd.submit();
}

static pds_ret_t
attr_age_tmo_get(enum lif_attr attr,
                 pds_flow_age_timeouts_t *attr_age_tmo)
{
    devcmd_t        devcmd(ftl_lif, age_tmo_cfg_lock, age_tmo_cfg_unlock);

    devcmd.req().lif_getattr.opcode = FTL_DEVCMD_OPCODE_LIF_GETATTR;
    devcmd.req().lif_getattr.attr = attr;
    return devcmd.submit(nullptr, (void *)attr_age_tmo);
}

static pds_ret_t
force_expired_ts_set(enum lif_attr attr,
                     bool force_expired_ts)
{
    devcmd_t        devcmd(ftl_lif, age_tmo_cfg_lock, age_tmo_cfg_unlock);

    devcmd.req().lif_setattr.opcode = FTL_DEVCMD_OPCODE_LIF_SETATTR;
    devcmd.req().lif_setattr.attr = attr;
    devcmd.req().lif_setattr.force_expired_ts = force_expired_ts;
    return devcmd.submit();
}

static pds_ret_t
metrics_get(enum ftl_qtype qtype,
            lif_attr_metrics_t *metrics)
{
    devcmd_t        devcmd(ftl_lif);

    devcmd.req().lif_getattr.opcode = FTL_DEVCMD_OPCODE_LIF_GETATTR;
    devcmd.req().lif_getattr.attr = FTL_LIF_ATTR_METRICS;
    devcmd.req().lif_getattr.qtype = qtype;
    return devcmd.submit(nullptr, metrics);
}

static pds_ret_t
pollers_alloc(enum ftl_qtype qtype)
{
    queue_identity_t    *qident = qtype_qident(qtype);

    SDK_ASSERT_TRACE_RETURN(qtype == FTL_QTYPE_POLLER, PDS_RET_INVALID_ARG,
                            "invalid qtype %d", qtype);
    PDS_TRACE_DEBUG("qtype %d qcount %d qdepth %d", qtype,
                    qident->qcount, qident->qdepth);
    if (qident->qcount) {
        queues_ctl[qtype] = new lif_queues_ctl_t(qtype, qident->qcount,
                                                 qident->qdepth);
        SDK_ASSERT_TRACE_RETURN(queues_ctl[qtype], PDS_RET_OOM,
                                "failed to allocate pollers for qtype %d",
                                qtype);
    }
    return PDS_RET_OK;
}

static pds_ret_t
scanners_alloc(enum ftl_qtype qtype)
{
    queue_identity_t    *qident = qtype_qident(qtype);

    SDK_ASSERT_TRACE_RETURN((qtype == FTL_QTYPE_SCANNER_SESSION) ||
                            (qtype == FTL_QTYPE_SCANNER_CONNTRACK),
                            PDS_RET_INVALID_ARG,
                            "invalid qtype %d", qtype);
    PDS_TRACE_DEBUG("qtype %d qcount %d qdepth %d", qtype,
                    qident->qcount, qident->qdepth);
    if (qident->qcount) {
        queues_ctl[qtype] = new lif_queues_ctl_t(qtype, qident->qcount,
                                                 qident->qdepth);
        SDK_ASSERT_TRACE_RETURN(queues_ctl[qtype], PDS_RET_OOM,
                                "failed to allocate scanners for qtype %d",
                                qtype);
    }
    return PDS_RET_OK;
}

static pds_ret_t
mpu_timestamp_ctl_alloc(enum ftl_qtype qtype)
{
    queue_identity_t    *qident = qtype_qident(qtype);

    SDK_ASSERT_TRACE_RETURN(qtype == FTL_QTYPE_MPU_TIMESTAMP,
                            PDS_RET_INVALID_ARG,
                            "invalid qtype %d", qtype);
    PDS_TRACE_DEBUG("qtype %d qcount %d qdepth %d", qtype,
                    qident->qcount, qident->qdepth);
    if (qident->qcount) {
        queues_ctl[qtype] = new lif_queues_ctl_t(qtype, qident->qcount,
                                                 qident->qdepth);
        SDK_ASSERT_TRACE_RETURN(queues_ctl[qtype], PDS_RET_OOM,
                                "failed to allocate MPU timestamp for qtype %d",
                                qtype);
    }
    return PDS_RET_OK;
}

/**
 * Queues control class
 */
lif_queues_ctl_t::lif_queues_ctl_t(enum ftl_qtype qtype,
                                   uint32_t qcount,
                                   uint32_t qdepth) :
    qtype(qtype),
    qcount(qcount),
    qcount_actual(0),
    qdepth(qdepth),
    table_sz(0)
{
    spinlocks = (rte_spinlock_t *)SDK_MALLOC(SDK_MEM_ALLOC_FTL_DEV_IMPL_LOCKS,
                                             sizeof(rte_spinlock_t) * qcount);
    SDK_ASSERT_RETURN_VOID(spinlocks);

    for (uint32_t qid = 0; qid < qcount; qid++) {
        rte_spinlock_init(&spinlocks[qid]);
    }
}

lif_queues_ctl_t::~lif_queues_ctl_t()
{
    SDK_FREE(SDK_MEM_ALLOC_FTL_DEV_IMPL_LOCKS, spinlocks);
}

pds_ret_t
lif_queues_ctl_t::init(devcmd_t *owner_devcmd)
{
    devcmd_t        local_devcmd(ftl_lif, queues_lock_all, queues_unlock_all);
    devcmd_t        *devcmd;
    pds_ret_t       ret = PDS_RET_OK;

    /*
     * Caller may have supplied a devcmd_t struct which indicates
     * a desire to enforce the same locking across multiple commands.
     * Otherwise, we use our own just for the current command.
     */
    devcmd = owner_devcmd ? owner_devcmd : &local_devcmd;
    switch (qtype) {

    case FTL_QTYPE_POLLER:
        ret = pollers_init(devcmd);
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
        ret = scanners_init(devcmd);
        break;

    case FTL_QTYPE_MPU_TIMESTAMP:
        ret = mpu_timestamp_init(devcmd);
        break;

    default:
        ret = PDS_RET_ERR;
        break;
    }
    return ret;
}

pds_ret_t
lif_queues_ctl_t::start(devcmd_t *owner_devcmd)
{
    devcmd_t        local_devcmd(ftl_lif, scanners_lock_all, scanners_unlock_all);
    devcmd_t        *devcmd;
    pds_ret_t       ret = PDS_RET_OK;

    /*
     * Caller may have supplied a devcmd_t struct which indicates
     * a desire to enforce the same locking across multiple commands.
     * Otherwise, we use our own just for the current command.
     */
    devcmd = owner_devcmd ? owner_devcmd : &local_devcmd;
    switch (qtype) {

    case FTL_QTYPE_POLLER:

        /*
         * SW queues don't use scheduler
         */
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
        devcmd->req_clr();
        devcmd->rsp_clr();
        devcmd->req().scanners_start.opcode = FTL_DEVCMD_OPCODE_SCANNERS_START;
        ret = devcmd->submit();
        if (ret != PDS_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    case FTL_QTYPE_MPU_TIMESTAMP:
        devcmd->req_clr();
        devcmd->rsp_clr();
        devcmd->req().mpu_timestamp_start.opcode = FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_START;
        ret = devcmd->submit();
        if (ret != PDS_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    default:
        ret = PDS_RET_ERR;
        break;
    }
    return ret;
}

pds_ret_t
lif_queues_ctl_t::stop(bool quiesce_check,
                       devcmd_t *owner_devcmd)
{
    devcmd_t        local_devcmd(ftl_lif, scanners_lock_all, scanners_unlock_all);
    devcmd_t        *devcmd;
    pds_ret_t       ret = PDS_RET_OK;

    /*
     * Caller may have supplied a devcmd_t struct which indicates
     * a desire to enforce the same locking across multiple commands.
     * Otherwise, we use our own just for the current command.
     */
    devcmd = owner_devcmd ? owner_devcmd : &local_devcmd;
    switch (qtype) {

    case FTL_QTYPE_POLLER:

        /*
         * SW queues don't use scheduler
         */
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
        devcmd->req_clr();
        devcmd->rsp_clr();
        devcmd->req().scanners_stop.opcode = FTL_DEVCMD_OPCODE_SCANNERS_STOP;
        devcmd->req().scanners_stop.quiesce_check = quiesce_check;
        ret = devcmd->submit_with_retry();
        if (ret != PDS_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    case FTL_QTYPE_MPU_TIMESTAMP:
        devcmd->req_clr();
        devcmd->rsp_clr();
        devcmd->req().mpu_timestamp_stop.opcode = FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_STOP;
        ret = devcmd->submit();
        if (ret != PDS_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    default:
        ret = PDS_RET_ERR;
        break;
    }
    return ret;
}

pds_ret_t
lif_queues_ctl_t::start_single(uint32_t qid)
{
    devcmd_t        devcmd(ftl_lif, scanners_lock_one, scanners_unlock_one,
                           this, qid);
    pds_ret_t       ret = PDS_RET_OK;

    switch (qtype) {

    case FTL_QTYPE_POLLER:

        /*
         * SW queues don't use scheduler
         */
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
        devcmd.req().scanners_start_single.opcode = FTL_DEVCMD_OPCODE_SCANNERS_START_SINGLE;
        devcmd.req().scanners_start_single.qtype = qtype;
        devcmd.req().scanners_start_single.index = qid;
        ret = devcmd.submit();
        if (ret != PDS_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    case FTL_QTYPE_MPU_TIMESTAMP:
        break;

    default:
        PDS_TRACE_ERR("unsupported qtype %d", qtype);
        ret = PDS_RET_ERR;
        break;
    }
    return ret;
}

pds_ret_t
lif_queues_ctl_t::flush(void)
{
    devcmd_t        devcmd(ftl_lif, pollers_lock_all, pollers_unlock_all);
    pds_ret_t       ret = PDS_RET_OK;

    switch (qtype) {

    case FTL_QTYPE_POLLER:
        devcmd.req().pollers_flush.opcode = FTL_DEVCMD_OPCODE_POLLERS_FLUSH;
        devcmd.req().pollers_flush.qtype = FTL_QTYPE_POLLER;
        ret = devcmd.submit();
        if (ret != PDS_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
    case FTL_QTYPE_MPU_TIMESTAMP:

        /*
         * HW queues support start/stop rather than flush.
         */
        break;

    default:
        ret = PDS_RET_ERR;
        break;
    }
    return ret;
}

pds_ret_t
lif_queues_ctl_t::dequeue_burst(uint32_t qid,
                                poller_slot_data_t *slot_data_buf,
                                uint32_t slot_data_buf_sz,
                                uint32_t *burst_count,
                                devcmd_t *owner_devcmd)
{
    devcmd_t        *devcmd;
    pds_ret_t       ret = PDS_RET_OK;

    switch (qtype) {

    case FTL_QTYPE_POLLER:

        if ((qid < qcount) && slot_data_buf_sz && *burst_count) {
            devcmd_t local_devcmd(ftl_lif, pollers_lock_one, pollers_unlock_one,
                                  this, qid);
            devcmd = owner_devcmd ? owner_devcmd : &local_devcmd;
            devcmd->req().pollers_deq_burst.opcode = FTL_DEVCMD_OPCODE_POLLERS_DEQ_BURST;
            devcmd->req().pollers_deq_burst.qtype = qtype;
            devcmd->req().pollers_deq_burst.index = qid;
            devcmd->req().pollers_deq_burst.burst_count = *burst_count;
            devcmd->req().pollers_deq_burst.buf_sz = slot_data_buf_sz;

            ret = devcmd->submit(nullptr, slot_data_buf);
            if ((ret != PDS_RET_OK) && (ret != PDS_RET_RETRY)) {
                PDS_TRACE_ERR("failed devcmd: error %d", ret);
            }

            *burst_count = devcmd->rsp().pollers_deq_burst.read_count;
            break;
        }

        ret = PDS_RET_INVALID_ARG;
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
    case FTL_QTYPE_MPU_TIMESTAMP:

        /*
         * HW queues have nothing to dequeue
         */
        *burst_count = 0;
        break;

    default:
        ret = PDS_RET_ERR;
        break;
    }
    return ret;
}

pds_ret_t
lif_queues_ctl_t::pollers_init(devcmd_t *devcmd)
{
    uint64_t        wrings_base_addr;
    uint32_t        wrings_total_sz;
    pds_ret_t       ret;

    devcmd->req_clr();
    devcmd->rsp_clr();

    devcmd->req().pollers_init.opcode = FTL_DEVCMD_OPCODE_POLLERS_INIT;
    devcmd->req().pollers_init.qtype = qtype;
    devcmd->req().pollers_init.qcount = qcount;
    devcmd->req().pollers_init.qdepth = qdepth;
    wrings_base_addr = api::g_pds_state.mempartition()->start_addr(
                                        FTL_DEV_POLLER_RINGS_HBM_HANDLE);
    wrings_total_sz = api::g_pds_state.mempartition()->size(
                                        FTL_DEV_POLLER_RINGS_HBM_HANDLE);
    SDK_ASSERT_TRACE_RETURN((wrings_base_addr != INVALID_MEM_ADDRESS) &&
                            wrings_total_sz, PDS_RET_NO_RESOURCE,
                            "HBM memory error for %s"
                            FTL_DEV_POLLER_RINGS_HBM_HANDLE);
    devcmd->req().pollers_init.wrings_base_addr = wrings_base_addr;
    devcmd->req().pollers_init.wrings_total_sz = wrings_total_sz;

    /*
     * In SIM mode artificially reduce the number of poller queues.
     */
    if (!platform_is_hw(platform_type)) {
        devcmd->req().pollers_init.qcount = std::min((uint32_t)2, qcount);
    }
    qcount_actual = devcmd->req().pollers_init.qcount;

    ret = devcmd->submit();
    if (ret != PDS_RET_OK) {
        PDS_TRACE_ERR("failed devcmd: error %d", ret);
    }
    return ret;
}

pds_ret_t
lif_queues_ctl_t::scanners_init(devcmd_t *devcmd)
{
    queue_identity_t        *qident = &lif_ident.base.qident[qtype];
    queue_identity_t        *pollers_qident = &lif_ident.base.qident[FTL_QTYPE_POLLER];
    p4pd_table_properties_t tprop = {0};
    uint32_t                tableid;
    pds_ret_t               ret;

    devcmd->req_clr();
    devcmd->rsp_clr();

    devcmd->req().scanners_init.opcode = FTL_DEVCMD_OPCODE_SCANNERS_INIT;
    devcmd->req().scanners_init.qtype = qtype;
    devcmd->req().scanners_init.qcount = qcount;

    tableid = qtype == FTL_QTYPE_SCANNER_SESSION ?
              P4TBL_ID_SESSION_INFO : P4TBL_ID_CONNTRACK;
    p4pd_error_t p4pd_error = p4pd_global_table_properties_get(tableid, &tprop);
    if (p4pd_error != P4PD_SUCCESS) {
        PDS_TRACE_ERR("failed to obtain properties for tableid %u: "
                      "error %d", tableid, p4pd_error);
        return PDS_RET_HW_PROGRAM_ERR;
    }

    devcmd->req().scanners_init.scan_addr_base = tprop.base_mem_pa;
    devcmd->req().scanners_init.scan_id_base = 0;
    devcmd->req().scanners_init.scan_table_sz = tprop.tabledepth;
    PDS_TRACE_DEBUG("qtype %d scan_addr_base 0x%" PRIx64 " scan_table_sz %u",
                    qtype, tprop.base_mem_pa, tprop.tabledepth);

    devcmd->req().scanners_init.scan_burst_sz = qident->burst_sz;
    devcmd->req().scanners_init.scan_resched_time = qident->burst_resched_time_us;
    devcmd->req().scanners_init.poller_lif = lif_ident.base.hw_index;
    devcmd->req().scanners_init.poller_qcount = pollers_qident->qcount;
    devcmd->req().scanners_init.poller_qdepth = pollers_qident->qdepth;
    devcmd->req().scanners_init.poller_qtype = FTL_QTYPE_POLLER;

    /*
     * In SIM mode artificially reduce the number of scanner queues
     * and scan table size.
     */
    if (!platform_is_hw(platform_type)) {
        devcmd->req().scanners_init.qcount =  std::min((uint32_t)2, qcount);
        devcmd->req().scanners_init.scan_table_sz = 1024;
        devcmd->req().scanners_init.poller_qcount =
                      std::min((uint32_t)2, pollers_qident->qcount);
    }
    table_sz = devcmd->req().scanners_init.scan_table_sz;
    qcount_actual = devcmd->req().scanners_init.qcount;

    ret = devcmd->submit();
    if (ret != PDS_RET_OK) {
        PDS_TRACE_ERR("failed devcmd: error %d", ret);
    }
    return ret;
}

pds_ret_t
lif_queues_ctl_t::mpu_timestamp_init(devcmd_t *devcmd)
{
    pds_ret_t               ret;

    devcmd->req_clr();
    devcmd->rsp_clr();
    devcmd->req().mpu_timestamp_init.opcode = FTL_DEVCMD_OPCODE_MPU_TIMESTAMP_INIT;
    devcmd->req().mpu_timestamp_init.qtype = qtype;

    ret = devcmd->submit();
    if (ret != PDS_RET_OK) {
        PDS_TRACE_ERR("failed devcmd: error %d", ret);
    }
    return ret;
}

void
lif_queues_ctl_t::lock(uint32_t qid)
{
    SDK_ASSERT_RETURN_VOID(qid < qcount);
    rte_spinlock_lock(&spinlocks[qid]);
}

void
lif_queues_ctl_t::unlock(uint32_t qid)
{
    SDK_ASSERT_RETURN_VOID(qid < qcount);
    rte_spinlock_unlock(&spinlocks[qid]);
}

void
lif_queues_ctl_t::lock_all(void)
{
    /*
     * NOTE: this code gives the appearance that it would be inefficient
     * having to lock all queues. However, commands that end up using this
     * function occur very infrequently, i.e., only once or twice 
     * during device bring up and bring down.
     *
     * Other more frequent commands would only need to acquire a single queue 
     * lock which is suitably fine-grained and less contentious.
     */
    for (uint32_t qid = 0; qid < qcount; qid++) {
        rte_spinlock_lock(&spinlocks[qid]);
    }
}

void
lif_queues_ctl_t::unlock_all(void)
{
    /*
     * See comments in lock_all().
     */
    for (uint32_t qid = 0; qid < qcount; qid++) {
        rte_spinlock_unlock(&spinlocks[qid]);
    }
}

/**
 * devcmd interface wrapper
 */
devcmd_t::devcmd_t(FtlDev *ftl_dev,
                   spinlock_enter_t spinlock_enter,
                   spinlock_leave_t spinlock_leave,
                   void *user_arg,
                   uint32_t user_idx) :
    ftl_dev(ftl_dev),
    ftl_lif(nullptr),
    spinlock_enter(spinlock_enter),
    spinlock_leave(spinlock_leave),
    user_arg(user_arg),
    user_idx(user_idx)
{
    SDK_ASSERT_RETURN_VOID((spinlock_enter  && spinlock_leave) ||
                           (!spinlock_enter && !spinlock_leave));
    req_clr();
    rsp_clr();
}

devcmd_t::devcmd_t(FtlLif *ftl_lif,
                   spinlock_enter_t spinlock_enter,
                   spinlock_leave_t spinlock_leave,
                   void *user_arg,
                   uint32_t user_idx) :
    ftl_dev(nullptr),
    ftl_lif(ftl_lif),
    spinlock_enter(spinlock_enter),
    spinlock_leave(spinlock_leave),
    user_arg(user_arg),
    user_idx(user_idx),
    pre_locked(false)
{
    SDK_ASSERT_RETURN_VOID((spinlock_enter  && spinlock_leave) ||
                           (!spinlock_enter && !spinlock_leave));
    req_clr();
    rsp_clr();
}

pds_ret_t
devcmd_t::owner_pre_lock(void)
{
    if (spinlock_enter) {
        (*spinlock_enter)(user_arg, user_idx);
        pre_locked = true;
        return PDS_RET_OK;
    }
    return PDS_RET_NOOP;
}

pds_ret_t
devcmd_t::owner_pre_unlock(void)
{
    if (pre_locked && spinlock_leave) {
        pre_locked = false;
        (*spinlock_leave)(user_arg, user_idx);
        return PDS_RET_OK;
    }
    return PDS_RET_NOOP;
}

pds_ret_t
devcmd_t::submit(void *req_data,
                 void *rsp_data)
{
    pds_ret_t           ret;

    /*
     * Take lock only if not already pre-locked by owener.
     */
    if (!pre_locked && spinlock_enter) {
        (*spinlock_enter)(user_arg, user_idx);
    }

    ret = cmd_handler(req_data, rsp_data);

    if (!pre_locked && spinlock_leave) {
        (*spinlock_leave)(user_arg, user_idx);
    }
    if (ret == PDS_RET_RETRY) {
        PDS_TRACE_DEBUG("retry needed for command %s",
                        ftl_dev_opcode_str(req_.cmd.opcode));
    }
    return ret;
}

pds_ret_t
devcmd_t::submit_with_retry(void *req_data,
                            void *rsp_data)
{
    ftl_timestamp_t     ts;
    pds_ret_t           ret;

    ts.time_expiry_set(FTL_DEVCMD_RETRY_TMO_US);

    /*
     * Take lock only if not already pre-locked by owener.
     */
    if (!pre_locked && spinlock_enter) {
        (*spinlock_enter)(user_arg, user_idx);
    }

    for ( ;; ) {
        ret = cmd_handler(req_data, rsp_data);
        if ((ret != PDS_RET_RETRY) || ts.time_expiry_check()) {
            break;
        }
        usleep(10000);
    }

    if (!pre_locked && spinlock_leave) {
        (*spinlock_leave)(user_arg, user_idx);
    }
    return ret == PDS_RET_RETRY ? PDS_RET_HW_PROGRAM_ERR : ret;
}

pds_ret_t
devcmd_t::cmd_handler(void *req_data,
                      void *rsp_data)
{
    ftl_status_code_t   status;

    status = ftl_lif ?
             ftl_lif->CmdHandler(&req_, req_data, &rsp_, rsp_data) :
             ftl_dev->CmdHandler(&req_, req_data, &rsp_, rsp_data);
    return ftl_status_2_pds_ret(status);
}

/**
 * Miscellaneous utility functions
 */
static void
scanners_lock_one(void *user_arg,
                  uint32_t idx)
{
    auto arg_scanners = static_cast<lif_queues_ctl_t *>(user_arg);
    arg_scanners->lock(idx);
}

static void
scanners_unlock_one(void *user_arg,
                    uint32_t idx)
{
    auto arg_scanners = static_cast<lif_queues_ctl_t *>(user_arg);
    arg_scanners->unlock(idx);
}

static void
pollers_lock_one(void *user_arg,
                 uint32_t idx)
{
    auto arg_pollers = static_cast<lif_queues_ctl_t *>(user_arg);
    arg_pollers->lock(idx);
}

static void
pollers_unlock_one(void *user_arg,
                   uint32_t idx)
{
    auto arg_pollers = static_cast<lif_queues_ctl_t *>(user_arg);
    arg_pollers->unlock(idx);
}

static void
age_tmo_cfg_lock(void *user_arg,
                 uint32_t idx)
{
    rte_spinlock_lock(&tmo_cfg_lock);
}

static void
age_tmo_cfg_unlock(void *user_arg,
                   uint32_t idx)
{
    rte_spinlock_unlock(&tmo_cfg_lock);
}

/*
 * The lock-all varieties below are expected to be used only by operations
 * that are very infrequent, such as device bringup/bringdown. The more
 * common operations would use the more fine grained (less contentious)
 * lock-one functions.
 */
static void
scanners_lock_all(void *user_arg,
                  uint32_t idx)
{
    if (session_scanners()) {
        session_scanners()->lock_all();
    }
    if (conntrack_scanners()) {
        conntrack_scanners()->lock_all();
    }
}

static void
scanners_unlock_all(void *user_arg,
                    uint32_t idx)
{
    if (conntrack_scanners()) {
        conntrack_scanners()->unlock_all();
    }
    if (session_scanners()) {
        session_scanners()->unlock_all();
    }
}

static void
pollers_lock_all(void *user_arg,
                 uint32_t idx)
{
    if (pollers()) {
        pollers()->lock_all();
    }
}

static void
pollers_unlock_all(void *user_arg,
                   uint32_t idx)
{
    if (pollers()) {
        pollers()->unlock_all();
    }
}

static void
queues_lock_all(void *user_arg,
                 uint32_t idx)
{
    pollers_lock_all(user_arg, idx);
    scanners_lock_all(user_arg, idx);
}

static void
queues_unlock_all(void *user_arg,
                  uint32_t idx)
{
    scanners_unlock_all(user_arg, idx);
    pollers_unlock_all(user_arg, idx);
}

static pds_ret_t
ftl_status_2_pds_ret(ftl_status_code_t ftl_status)
{
    switch (ftl_status) {

#define FTL_STATUS_CASE_PDS_RET(x, y)      case x: return y

    FTL_STATUS_CASE_PDS_RET(FTL_RC_SUCCESS,  PDS_RET_OK); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EVERSION, PDS_RET_HW_SW_OO_SYNC); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EOPCODE,  PDS_RET_INVALID_OP); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EIO,      PDS_RET_HW_PROGRAM_ERR); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EPERM,    PDS_RET_ERR); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EQID,     PDS_RET_INVALID_ARG); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EQTYPE,   PDS_RET_INVALID_ARG); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_ENOENT,   PDS_RET_ENTRY_NOT_FOUND); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EINTR,    PDS_RET_RETRY); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EAGAIN,   PDS_RET_RETRY); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_ENOMEM,   PDS_RET_OOM); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EFAULT,   PDS_RET_INVALID_ARG); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EBUSY,    PDS_RET_IN_PROGRESS); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EEXIST,   PDS_RET_ENTRY_EXISTS); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_EINVAL,   PDS_RET_INVALID_ARG); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_ENOSPC,   PDS_RET_NO_RESOURCE); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_ERANGE,   PDS_RET_OOB); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_BAD_ADDR, PDS_RET_OOB); 
    FTL_STATUS_CASE_PDS_RET(FTL_RC_ERROR,    PDS_RET_ERR); 

    default: return PDS_RET_ERR;
    }
}

} // namespace ftl_dev_impl

