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
qtype_queue_ctl(enum ftl_qtype qtype)
{
    return qtype < FTL_QTYPE_MAX ?
           queues_ctl[qtype] : nullptr;
}


static sdk_ret_t dev_identify(void);
static sdk_ret_t lif_init(void);
static sdk_ret_t attr_age_tmo_set(enum lif_attr attr,
                                  const pds_flow_age_timeouts_t *attr_age_tmo);
static sdk_ret_t force_expired_ts_set(enum lif_attr attr,
                                      bool force_expired_ts);
static sdk_ret_t pollers_alloc(enum ftl_qtype qtype);
static sdk_ret_t scanners_alloc(enum ftl_qtype qtype);

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
static sdk_ret_t ftl_status_2_sdk_ret(ftl_status_code_t ftl_status);


sdk_ret_t
init(void)
{
    DeviceManager       *devmgr_if;
    sdk_ret_t           ret = SDK_RET_OK;

    if (rte_atomic16_test_and_set(&module_inited)) {
        platform_type = api::g_pds_state.platform_type();
        PDS_TRACE_DEBUG("One-time initialization platform_type %d",
                        platform_type);
        devmgr_if = nicmgr::nicmgrapi::devmgr_if();
        SDK_ASSERT_TRACE_RETURN(devmgr_if, SDK_RET_ENTRY_NOT_FOUND,
                                "failed to locate devmgr_if");

        ftl_dev = (FtlDev *)devmgr_if->GetDevice("ftl");
        SDK_ASSERT_TRACE_RETURN(ftl_dev, SDK_RET_ENTRY_NOT_FOUND,
                                "failed to locate ftl device");
        ret = dev_identify();
        if (ret == SDK_RET_OK) {
            ret = lif_init();
        }
    }
    return ret;
}

sdk_ret_t
scanners_start(void)
{
    sdk_ret_t       ret = SDK_RET_OK;

    if (!lif_init_done()) {
        return SDK_RET_RETRY;
    }

    if (session_scanners()) {
        ret = session_scanners()->start();
    }
    if (conntrack_scanners() && (ret == SDK_RET_OK)) {
        ret = conntrack_scanners()->start();
    }
    return ret;
}

sdk_ret_t
scanners_stop(bool quiesce_check)
{
    sdk_ret_t       ret = SDK_RET_OK;

    if (!lif_init_done()) {
        return SDK_RET_RETRY;
    }
    if (session_scanners()) {
        ret = session_scanners()->stop(quiesce_check);
    }
    if (conntrack_scanners() && (ret == SDK_RET_OK)) {
        ret = conntrack_scanners()->stop(quiesce_check);
    }
    return ret;
}

sdk_ret_t
scanners_start_single(enum ftl_qtype qtype,
                      uint32_t qid)
{
    lif_queues_ctl_t    *qctl;
    sdk_ret_t           ret = SDK_RET_INVALID_ARG;

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

sdk_ret_t
pollers_qcount_get(uint32_t *ret_qcount)
{
    *ret_qcount = 0;
    if (!lif_init_done()) {
        return SDK_RET_RETRY;
    }
    if (pollers()) {
        *ret_qcount = pollers()->qcount_get();
    }
    return SDK_RET_OK;
}

sdk_ret_t
pollers_flush(void)
{
    sdk_ret_t       ret = SDK_RET_OK;

    if (!lif_init_done()) {
        return SDK_RET_RETRY;
    }
    if (pollers()) {
        ret = pollers()->flush();
    }
    return ret;
}

sdk_ret_t
pollers_dequeue_burst(uint32_t qid,
                      poller_slot_data_t *slot_data_buf,
                      uint32_t slot_data_buf_sz,
                      uint32_t *burst_count)
{
    sdk_ret_t       ret = SDK_RET_NOOP;

    if (!lif_init_done()) {
        return SDK_RET_RETRY;
    }
    if (pollers()) {
        ret = pollers()->dequeue_burst(qid, slot_data_buf,
                                       slot_data_buf_sz, burst_count);
    }
    return ret;
}

sdk_ret_t
normal_timeouts_set(const pds_flow_age_timeouts_t *age_tmo)
{
    return attr_age_tmo_set(FTL_LIF_ATTR_NORMAL_AGE_TMO, age_tmo);
}

sdk_ret_t
accel_timeouts_set(const pds_flow_age_timeouts_t *age_tmo)
{
    return attr_age_tmo_set(FTL_LIF_ATTR_ACCEL_AGE_TMO, age_tmo);
}

sdk_ret_t
accel_aging_control(bool enable_sense)
{
    devcmd_t        devcmd(ftl_lif, age_tmo_cfg_lock, age_tmo_cfg_unlock);
    sdk_ret_t       ret;

    devcmd.req().accel_aging_ctl.opcode = FTL_DEVCMD_OPCODE_ACCEL_AGING_CONTROL;
    devcmd.req().accel_aging_ctl.enable_sense = enable_sense;

    ret = devcmd.submit();
    if ((ret != SDK_RET_OK) && (ret != SDK_RET_RETRY)) {
        PDS_TRACE_ERR("failed devcmd: error %d", ret);
    }
    return ret;
}

sdk_ret_t
force_session_expired_ts_set(bool force_expired_ts)
{
    return force_expired_ts_set(FTL_LIF_ATTR_FORCE_SESSION_EXPIRED_TS,
                                force_expired_ts);
}

sdk_ret_t
force_conntrack_expired_ts_set(bool force_expired_ts)
{
    return force_expired_ts_set(FTL_LIF_ATTR_FORCE_CONNTRACK_EXPIRED_TS,
                                force_expired_ts);
}

bool
lif_init_done(void)
{
    return !!rte_atomic16_read(&lif_init_completed);
}

static sdk_ret_t
dev_identify(void)
{
    devcmd_t        devcmd(ftl_dev);
    sdk_ret_t       ret;

    devcmd.req().dev_identify.opcode = FTL_DEVCMD_OPCODE_IDENTIFY;
    devcmd.req().dev_identify.type = FTL_LIF_TYPE_BASE;
    devcmd.req().dev_identify.ver = IDENTITY_VERSION_1;

    ret = devcmd.submit(nullptr, &dev_ident);
    SDK_ASSERT_TRACE_RETURN(ret == SDK_RET_OK, ret,
                            "failed device identify");
    SDK_ASSERT_TRACE_RETURN(dev_ident.base.version == IDENTITY_VERSION_1,
                            SDK_RET_ERR, "unexpected device version %d",
                            dev_ident.base.version);
    SDK_ASSERT_TRACE_RETURN(dev_ident.base.nlifs, SDK_RET_ERR,
                            "no device LIFs");
    ftl_lif = ftl_dev->LifFind(0);
    SDK_ASSERT_TRACE_RETURN(ftl_lif, SDK_RET_ERR,
                            "LIF at index 0 not found");
    return ret;
}

static sdk_ret_t
lif_init(void)
{
    devcmd_t        devcmd(ftl_lif);
    sdk_ret_t       ret = SDK_RET_OK;

    if (rte_atomic16_test_and_set(&lif_init_initiated)) {
        PDS_TRACE_DEBUG("One-time LIF initialization");

        devcmd.req().lif_identify.opcode = FTL_DEVCMD_OPCODE_LIF_IDENTIFY;
        devcmd.req().lif_identify.type = FTL_LIF_TYPE_BASE;
        devcmd.req().lif_identify.ver = IDENTITY_VERSION_1;
        ret = devcmd.submit_with_retry(nullptr, &lif_ident);
        SDK_ASSERT_TRACE_RETURN(ret == SDK_RET_OK, ret,
                                "failed LIF identify");
        SDK_ASSERT_TRACE_RETURN(lif_ident.base.version == IDENTITY_VERSION_1,
                                SDK_RET_ERR, "unexpected LIF version %d",
                                lif_ident.base.version);
        devcmd.req_clr();
        devcmd.req().lif_init.opcode = FTL_DEVCMD_OPCODE_LIF_INIT;
        ret = devcmd.submit_with_retry();
        SDK_ASSERT_TRACE_RETURN(ret == SDK_RET_OK, ret,
                                "failed LIF init");
        devcmd.req_clr();
        devcmd.req().lif_reset.opcode = FTL_DEVCMD_OPCODE_LIF_RESET;
        ret = devcmd.submit_with_retry();
        SDK_ASSERT_TRACE_RETURN(ret == SDK_RET_OK, ret,
                                "failed LIF reset");
        /*
         * Allocate poller/scanners before initializing them
         * (in order to provide proper devcmd locking)
         */
        ret = pollers_alloc(FTL_QTYPE_POLLER);
        if (ret == SDK_RET_OK) {
            ret = scanners_alloc(FTL_QTYPE_SCANNER_SESSION);
        }
        if (ret == SDK_RET_OK) {
            ret = scanners_alloc(FTL_QTYPE_SCANNER_CONNTRACK);
        }

        /*
         * Pre-lock and share the same devcmd block across all queue inits.
         */
        if (ret == SDK_RET_OK) {
            devcmd_t devcmd_qinit(ftl_lif, queues_lock_all, queues_unlock_all);

            devcmd_qinit.owner_pre_lock();
            if (pollers() && (ret == SDK_RET_OK)) {
                ret = pollers()->init(&devcmd_qinit);
            }
            if (session_scanners() && (ret == SDK_RET_OK)) {
                ret = session_scanners()->init(&devcmd_qinit);
            }
            if (conntrack_scanners() && (ret == SDK_RET_OK)) {
                ret = conntrack_scanners()->init(&devcmd_qinit);
            }

            /*
             * Scanners are always started by default unless we're in
             * SIM mode, in which case, a test program will start them.
             * (For now keep scanners disabled until fully brought up.)
             */
            if (0 /*platform_is_hw(platform_type)*/) {

                if (session_scanners() && (ret == SDK_RET_OK)) {
                    ret = session_scanners()->start(&devcmd_qinit);
                }
                if (conntrack_scanners() && (ret == SDK_RET_OK)) {
                    ret = conntrack_scanners()->start(&devcmd_qinit);
                }
            }
            devcmd_qinit.owner_pre_unlock();
        }
        rte_atomic16_set(&lif_init_completed, 1);
    }
    return ret;
}

static sdk_ret_t
attr_age_tmo_set(enum lif_attr attr,
                 const pds_flow_age_timeouts_t *attr_age_tmo)
{
    devcmd_t        devcmd(ftl_lif, age_tmo_cfg_lock, age_tmo_cfg_unlock);
    sdk_ret_t       ret;

    devcmd.req().lif_setattr.opcode = FTL_DEVCMD_OPCODE_LIF_SETATTR;
    devcmd.req().lif_setattr.attr = attr;
    devcmd.req().lif_setattr.age_tmo = *attr_age_tmo;

    ret = devcmd.submit();
    if ((ret != SDK_RET_OK) && (ret != SDK_RET_RETRY)) {
        PDS_TRACE_ERR("failed devcmd: error %d", ret);
    }
    return ret;
}

static sdk_ret_t
force_expired_ts_set(enum lif_attr attr,
                     bool force_expired_ts)
{
    /*
     * force_expiry is only for debugging and can be dynamically set
     * (no need for any lock)
     */
    devcmd_t        devcmd(ftl_lif);
    sdk_ret_t       ret;

    devcmd.req().lif_setattr.opcode = FTL_DEVCMD_OPCODE_LIF_SETATTR;
    devcmd.req().lif_setattr.attr = attr;
    devcmd.req().lif_setattr.force_expired_ts = force_expired_ts;

    ret = devcmd.submit();
    if ((ret != SDK_RET_OK) && (ret != SDK_RET_RETRY)) {
        PDS_TRACE_ERR("failed devcmd: error %d", ret);
    }
    return ret;
}

static sdk_ret_t
pollers_alloc(enum ftl_qtype qtype)
{
    queue_identity_t    *qident = qtype_qident(qtype);

    SDK_ASSERT_TRACE_RETURN(qtype == FTL_QTYPE_POLLER, SDK_RET_INVALID_ARG,
                            "invalid qtype %d", qtype);
    PDS_TRACE_DEBUG("qtype %d qcount %d qdepth %d", qtype,
                    qident->qcount, qident->qdepth);
    if (qident->qcount) {
        queues_ctl[qtype] = new lif_queues_ctl_t(qtype, qident->qcount,
                                                 qident->qdepth);
        SDK_ASSERT_TRACE_RETURN(queues_ctl[qtype], SDK_RET_OOM,
                                "failed to allocate pollers for qtype %d",
                                qtype);
    }
    return SDK_RET_OK;
}

static sdk_ret_t
scanners_alloc(enum ftl_qtype qtype)
{
    queue_identity_t    *qident = qtype_qident(qtype);

    SDK_ASSERT_TRACE_RETURN((qtype == FTL_QTYPE_SCANNER_SESSION) ||
                            (qtype == FTL_QTYPE_SCANNER_CONNTRACK),
                            SDK_RET_INVALID_ARG,
                            "invalid qtype %d", qtype);
    PDS_TRACE_DEBUG("qtype %d qcount %d qdepth %d", qtype,
                    qident->qcount, qident->qdepth);
    if (qident->qcount) {
        queues_ctl[qtype] = new lif_queues_ctl_t(qtype, qident->qcount,
                                                 qident->qdepth);
        SDK_ASSERT_TRACE_RETURN(queues_ctl[qtype], SDK_RET_OOM,
                                "failed to allocate scanners for qtype %d",
                                qtype);
    }
    return SDK_RET_OK;
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
    qdepth(qdepth)
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

sdk_ret_t
lif_queues_ctl_t::init(devcmd_t *owner_devcmd)
{
    devcmd_t        local_devcmd(ftl_lif, queues_lock_all, queues_unlock_all);
    devcmd_t        *devcmd;
    sdk_ret_t       ret;

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

    default:
        ret = SDK_RET_ERR;
        break;
    }
    return ret;
}

sdk_ret_t
lif_queues_ctl_t::start(devcmd_t *owner_devcmd)
{
    devcmd_t        local_devcmd(ftl_lif, scanners_lock_all, scanners_unlock_all);
    devcmd_t        *devcmd;
    sdk_ret_t       ret;

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
        ret = SDK_RET_OK;
        break;

    case FTL_QTYPE_SCANNER_SESSION:
        devcmd->req_clr();
        devcmd->rsp_clr();
        devcmd->req().scanners_start.opcode = FTL_DEVCMD_OPCODE_SCANNERS_START;
        ret = devcmd->submit();
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    case FTL_QTYPE_SCANNER_CONNTRACK:

        /*
         * NOTE: FTL_DEVCMD_OPCODE_SCANNERS_START above would start all scanners
         * (of all qtypes) so no need to run it separately for conntrack.
         */
        ret = SDK_RET_OK;
        break;

    default:
        ret = SDK_RET_ERR;
        break;
    }
    return ret;
}

sdk_ret_t
lif_queues_ctl_t::stop(bool quiesce_check,
                       devcmd_t *owner_devcmd)
{
    devcmd_t        local_devcmd(ftl_lif, scanners_lock_all, scanners_unlock_all);
    devcmd_t        *devcmd;
    sdk_ret_t       ret;

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
        ret = SDK_RET_OK;
        break;

    case FTL_QTYPE_SCANNER_SESSION:
        devcmd->req_clr();
        devcmd->rsp_clr();
        devcmd->req().scanners_stop.opcode = FTL_DEVCMD_OPCODE_SCANNERS_STOP;
        devcmd->req().scanners_stop.quiesce_check = quiesce_check;
        ret = devcmd->submit_with_retry();
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    case FTL_QTYPE_SCANNER_CONNTRACK:

        /*
         * NOTE: FTL_DEVCMD_OPCODE_SCANNERS_STOP above would stop all scanners
         * (of all qtypes) so no need to run it separately for conntrack.
         */
        ret = SDK_RET_OK;
        break;

    default:
        ret = SDK_RET_ERR;
        break;
    }
    return ret;
}

sdk_ret_t
lif_queues_ctl_t::start_single(uint32_t qid)
{
    devcmd_t        devcmd(ftl_lif, scanners_lock_one, scanners_unlock_one,
                           this, qid);
    sdk_ret_t       ret;

    switch (qtype) {

    case FTL_QTYPE_POLLER:

        /*
         * SW queues don't use scheduler
         */
        ret = SDK_RET_OK;
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
        devcmd.req().scanners_start_single.opcode = FTL_DEVCMD_OPCODE_SCANNERS_START_SINGLE;
        devcmd.req().scanners_start_single.qtype = qtype;
        devcmd.req().scanners_start_single.index = qid;
        ret = devcmd.submit();
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    default:
        PDS_TRACE_ERR("unsupported qtype %d", qtype);
        ret = SDK_RET_ERR;
        break;
    }
    return ret;
}

sdk_ret_t
lif_queues_ctl_t::flush(void)
{
    devcmd_t        devcmd(ftl_lif, pollers_lock_all, pollers_unlock_all);
    sdk_ret_t       ret;

    switch (qtype) {

    case FTL_QTYPE_POLLER:
        devcmd.req().pollers_flush.opcode = FTL_DEVCMD_OPCODE_POLLERS_FLUSH;
        devcmd.req().pollers_flush.qtype = FTL_QTYPE_POLLER;
        ret = devcmd.submit();
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("failed devcmd: error %d", ret);
        }
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:

        /*
         * HW queues support start/stop rather than flush.
         */
        ret = SDK_RET_OK;
        break;

    default:
        ret = SDK_RET_ERR;
        break;
    }
    return ret;
}

sdk_ret_t
lif_queues_ctl_t::dequeue_burst(uint32_t qid,
                                poller_slot_data_t *slot_data_buf,
                                uint32_t slot_data_buf_sz,
                                uint32_t *burst_count,
                                devcmd_t *owner_devcmd)
{
    devcmd_t        *devcmd;
    sdk_ret_t       ret;

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
            if ((ret != SDK_RET_OK) && (ret != SDK_RET_RETRY)) {
                PDS_TRACE_ERR("failed devcmd: error %d", ret);
            }

            *burst_count = devcmd->rsp().pollers_deq_burst.read_count;
            break;
        }

        ret = SDK_RET_INVALID_ARG;
        break;

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:

        /*
         * HW queues have nothing to dequeue
         */
        *burst_count = 0;
        ret = SDK_RET_OK;
        break;

    default:
        ret = SDK_RET_ERR;
        break;
    }
    return ret;
}

sdk_ret_t
lif_queues_ctl_t::pollers_init(devcmd_t *devcmd)
{
    uint64_t        wrings_base_addr;
    uint32_t        wrings_total_sz;
    sdk_ret_t       ret;

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
                            wrings_total_sz, SDK_RET_NO_RESOURCE,
                            "HBM memory error for %s"
                            FTL_DEV_POLLER_RINGS_HBM_HANDLE);
    devcmd->req().pollers_init.wrings_base_addr = wrings_base_addr;
    devcmd->req().pollers_init.wrings_total_sz = wrings_total_sz;

    /*
     * In SIM mode artificially reduce the number of poller queues.
     */
    if (!platform_is_hw(platform_type)) {
        devcmd->req().pollers_init.qcount = 1;
    }
    qcount_actual = devcmd->req().pollers_init.qcount;

    ret = devcmd->submit();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("failed devcmd: error %d", ret);
    }
    return ret;
}

sdk_ret_t
lif_queues_ctl_t::scanners_init(devcmd_t *devcmd)
{
    queue_identity_t        *qident = &lif_ident.base.qident[qtype];
    queue_identity_t        *pollers_qident = &lif_ident.base.qident[FTL_QTYPE_POLLER];
    p4pd_table_properties_t tprop = {0};
    uint32_t                tableid;
    sdk_ret_t               ret;

    devcmd->req_clr();
    devcmd->rsp_clr();

    devcmd->req().scanners_init.opcode = FTL_DEVCMD_OPCODE_SCANNERS_INIT;
    devcmd->req().scanners_init.qtype = qtype;
    devcmd->req().scanners_init.qcount = qcount;

    tableid = qtype == FTL_QTYPE_SCANNER_SESSION ?
              P4TBL_ID_SESSION_INFO_COMMON : P4TBL_ID_CONNTRACK;
    p4pd_error_t p4pd_error = p4pd_global_table_properties_get(tableid, &tprop);
    if (p4pd_error != P4PD_SUCCESS) {
        PDS_TRACE_ERR("failed to obtain properties for tableid %u: "
                      "error %d", tableid, p4pd_error);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    devcmd->req().scanners_init.scan_addr_base = tprop.base_mem_pa;
    devcmd->req().scanners_init.scan_id_base = 0;
    devcmd->req().scanners_init.scan_table_sz = tprop.tabledepth;
    PDS_TRACE_DEBUG("qtype %d scan_addr_base 0x%llx scan_table_sz %u",
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
        devcmd->req().scanners_init.qcount = 1;
        devcmd->req().scanners_init.scan_table_sz = 640;
        devcmd->req().scanners_init.poller_qcount = 1;
    }
    qcount_actual = devcmd->req().scanners_init.qcount;

    ret = devcmd->submit();
    if (ret != SDK_RET_OK) {
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

sdk_ret_t
devcmd_t::owner_pre_lock(void)
{
    if (spinlock_enter) {
        (*spinlock_enter)(user_arg, user_idx);
        pre_locked = true;
        return SDK_RET_OK;
    }
    return SDK_RET_NOOP;
}

sdk_ret_t
devcmd_t::owner_pre_unlock(void)
{
    if (pre_locked && spinlock_leave) {
        pre_locked = false;
        (*spinlock_leave)(user_arg, user_idx);
        return SDK_RET_OK;
    }
    return SDK_RET_NOOP;
}

sdk_ret_t
devcmd_t::submit(void *req_data,
                 void *rsp_data)
{
    sdk_ret_t           ret;

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
    if (ret == SDK_RET_RETRY) {
        PDS_TRACE_DEBUG("retry needed for command %s",
                        ftl_dev_opcode_str(req_.cmd.opcode));
    }
    return ret;
}

sdk_ret_t
devcmd_t::submit_with_retry(void *req_data,
                            void *rsp_data)
{
    ftl_timestamp_t     ts;
    sdk_ret_t           ret;

    time_expiry_set(ts, FTL_DEVCMD_RETRY_TMO_US);

    /*
     * Take lock only if not already pre-locked by owener.
     */
    if (!pre_locked && spinlock_enter) {
        (*spinlock_enter)(user_arg, user_idx);
    }

    for ( ;; ) {
        ret = cmd_handler(req_data, rsp_data);
        if ((ret != SDK_RET_RETRY) || time_expiry_check(ts)) {
            break;
        }
        usleep(10000);
    }

    if (!pre_locked && spinlock_leave) {
        (*spinlock_leave)(user_arg, user_idx);
    }
    return ret == SDK_RET_RETRY ? SDK_RET_HW_PROGRAM_ERR : ret;
}

sdk_ret_t
devcmd_t::cmd_handler(void *req_data,
                      void *rsp_data)
{
    ftl_status_code_t   status;

    status = ftl_lif ?
             ftl_lif->CmdHandler(&req_, req_data, &rsp_, rsp_data) :
             ftl_dev->CmdHandler(&req_, req_data, &rsp_, rsp_data);
    return ftl_status_2_sdk_ret(status);
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

static sdk_ret_t
ftl_status_2_sdk_ret(ftl_status_code_t ftl_status)
{
    switch (ftl_status) {

#define FTL_STATUS_CASE_SDK_RET(x, y)      case x: return y

    FTL_STATUS_CASE_SDK_RET(FTL_RC_SUCCESS,  SDK_RET_OK); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EVERSION, SDK_RET_HW_SW_OO_SYNC); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EOPCODE,  SDK_RET_INVALID_OP); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EIO,      SDK_RET_HW_PROGRAM_ERR); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EPERM,    SDK_RET_ERR); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EQID,     SDK_RET_INVALID_ARG); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EQTYPE,   SDK_RET_INVALID_ARG); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_ENOENT,   SDK_RET_ENTRY_NOT_FOUND); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EINTR,    SDK_RET_RETRY); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EAGAIN,   SDK_RET_RETRY); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_ENOMEM,   SDK_RET_OOM); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EFAULT,   SDK_RET_INVALID_ARG); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EBUSY,    SDK_RET_IN_PROGRESS); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EEXIST,   SDK_RET_ENTRY_EXISTS); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_EINVAL,   SDK_RET_INVALID_ARG); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_ENOSPC,   SDK_RET_NO_RESOURCE); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_ERANGE,   SDK_RET_OOB); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_BAD_ADDR, SDK_RET_OOB); 
    FTL_STATUS_CASE_SDK_RET(FTL_RC_ERROR,    SDK_RET_ERR); 

    default: return SDK_RET_ERR;
    }
}

} // namespace ftl_dev_impl

