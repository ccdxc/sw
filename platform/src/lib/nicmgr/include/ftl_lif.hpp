#ifndef __FTL_LIF_HPP__
#define __FTL_LIF_HPP__

#include <map>
#include <utility>
#include "nic/p4/ftl_dev/include/ftl_dev_shared.h"
#include "nic/sdk/asic/pd/db.hpp"

#ifndef USEC_PER_SEC
#define USEC_PER_SEC    1000000L
#endif
#ifndef MSEC_PER_SEC
#define MSEC_PER_SEC    1000L
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#endif

/*
 * Local doorbell address formation
 */
#define DB_QID_SHFT                     24
#define DB32_QID_SHFT                   16

#define FTL_LIF_DBDATA_SET(qid, pndx)                               \
    (((uint64_t)(qid) << DB_QID_SHFT) | (pndx))

#define FTL_LIF_DBDATA32_SET(qid, pndx)                             \
    (((uint64_t)(qid) << DB32_QID_SHFT) | (pndx))

using namespace ftl_dev_if;

class FtlLif;

static inline bool
is_power_of_2(uint64_t n)
{
    return n && !(n & (n - 1));
}

/**
 * Generic timestamp and expiry interval
 */
static inline uint64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((uint64_t)tv.tv_sec * USEC_PER_SEC) +
            (uint64_t)tv.tv_usec);
}

class ftl_timestamp_t
{
public:
    ftl_timestamp_t() :
        ts(0),
        expiry(0) {}

    void time_expiry_set(uint64_t val)
    {
        ts = timestamp();
        expiry = val;
    }

    bool time_expiry_check(void)
    {
        return (expiry == 0) ||
               ((timestamp() - ts) > expiry);
    }

private:
    uint64_t                ts;
    uint64_t                expiry;
};

/**
 * Devcmd context
 */
class ftl_lif_devcmd_ctx_t {
public:
    ftl_lif_devcmd_ctx_t() :
        req(nullptr),
        req_data(nullptr),
        rsp(nullptr),
        rsp_data(nullptr),
        status(FTL_RC_SUCCESS)
    {
    }

    ftl_devcmd_t            *req;
    void                    *req_data;
    ftl_devcmd_cpl_t        *rsp;
    void                    *rsp_data;
    ftl_status_code_t       status;
};

/**
 * LIF State Machine
 */
typedef enum {
    FTL_LIF_ST_INITIAL,
    FTL_LIF_ST_WAIT_HAL,
    FTL_LIF_ST_PRE_INIT,
    FTL_LIF_ST_POST_INIT,
    FTL_LIF_ST_QUEUES_RESET,
    FTL_LIF_ST_QUEUES_PRE_INIT,
    FTL_LIF_ST_QUEUES_INIT_TRANSITION,
    FTL_LIF_ST_QUEUES_STOPPING,
    FTL_LIF_ST_QUEUES_STARTED,
    FTL_LIF_ST_MAX,

    FTL_LIF_ST_SAME
} ftl_lif_state_t;

#define FTL_LIF_STATE_STR_TABLE                                     \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_INITIAL),                    \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_WAIT_HAL),                   \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_PRE_INIT),                   \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_POST_INIT),                  \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_QUEUES_RESET),               \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_QUEUES_PRE_INIT),            \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_QUEUES_INIT_TRANSITION),     \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_QUEUES_STOPPING),            \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_ST_QUEUES_STARTED),             \

typedef enum {
    FTL_LIF_EV_NULL,
    FTL_LIF_EV_ANY,
    FTL_LIF_EV_CREATE,
    FTL_LIF_EV_DESTROY,
    FTL_LIF_EV_HAL_UP,
    FTL_LIF_EV_IDENTIFY,
    FTL_LIF_EV_INIT,
    FTL_LIF_EV_SETATTR,
    FTL_LIF_EV_GETATTR,
    FTL_LIF_EV_RESET,
    FTL_LIF_EV_RESET_DESTROY,
    FTL_LIF_EV_PRE_INIT,
    FTL_LIF_EV_SCANNERS_QUIESCE,
    FTL_LIF_EV_POLLERS_INIT,
    FTL_LIF_EV_SCANNERS_INIT,
    FTL_LIF_EV_SCANNERS_START,
    FTL_LIF_EV_SCANNERS_START_SINGLE,
    FTL_LIF_EV_SCANNERS_STOP,
    FTL_LIF_EV_QUEUES_STOP_COMPLETE,
    FTL_LIF_EV_POLLERS_FLUSH,
    FTL_LIF_EV_POLLERS_DEQ_BURST,
    FTL_LIF_EV_MPU_TIMESTAMP_INIT,
    FTL_LIF_EV_MPU_TIMESTAMP_START,
    FTL_LIF_EV_MPU_TIMESTAMP_STOP,
    FTL_LIF_EV_ACCEL_AGING_CONTROL,

    FTL_LIF_EV_MAX
} ftl_lif_event_t;

#define FTL_LIF_EVENT_STR_TABLE                                     \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_NULL),                       \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_ANY),                        \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_CREATE),                     \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_DESTROY),                    \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_HAL_UP),                     \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_IDENTIFY),                   \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_INIT),                       \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_SETATTR),                    \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_GETATTR),                    \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_RESET),                      \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_RESET_DESTROY),              \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_PRE_INIT),                   \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_SCANNERS_QUIESCE),           \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_POLLERS_INIT),               \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_SCANNERS_INIT),              \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_SCANNERS_START),             \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_SCANNERS_START_SINGLE),      \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_SCANNERS_STOP),              \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_QUEUES_STOP_COMPLETE),       \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_POLLERS_FLUSH),              \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_POLLERS_DEQ_BURST),          \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_MPU_TIMESTAMP_INIT),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_MPU_TIMESTAMP_START),        \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_MPU_TIMESTAMP_STOP),         \
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_ACCEL_AGING_CONTROL),        \
 
typedef ftl_lif_event_t (FtlLif::*ftl_lif_action_t)(ftl_lif_event_t event,
                                                    ftl_lif_devcmd_ctx_t& devcmd_ctx);

typedef struct {
    ftl_lif_event_t         event;
    ftl_lif_action_t        action;
    ftl_lif_state_t         next_state;
} ftl_lif_state_event_t;

typedef struct {
    ftl_lif_state_event_t *state_event;
} ftl_lif_fsm_t;

typedef struct {
    ftl_lif_action_t        action;
    ftl_lif_state_t         next_state;
} ftl_lif_ordered_event_t;

/**
 * HW Scanners init - single queue init
 */
typedef struct {
    uint8_t                 cos;
    uint8_t                 qtype;
    uint16_t                lif_index;
    uint16_t                pid;
    uint32_t                index;
    uint64_t                scan_addr_base;
    uint32_t                scan_id_base;
    uint32_t                scan_range_sz;
    uint32_t                scan_burst_sz;
    uint32_t                scan_resched_time;
    uint32_t                poller_lif;
    uint32_t                poller_qid;
    uint8_t                 poller_qdepth_shft;
    uint8_t                 poller_qtype;
    uint8_t                 cos_override;
} scanner_init_single_cmd_t;

/**
 * SW Pollers init - single queue init
 */
typedef struct {
    uint16_t                lif_index;
    uint16_t                pid;
    uint32_t                index;
    uint64_t                wring_base_addr;
    uint32_t                wring_sz;
    uint8_t                 qdepth_shft;
} poller_init_single_cmd_t;

/*
 * std::exchange() is available only in C++14 so
 * we use the cppreference implementation here.
 */
template<class T, class U = T>
T obj_xchg(T& obj, U&& new_value)
{
    T old_value = std::move(obj);
    obj = std::forward<U>(new_value);
    return old_value;
}

/*
 * Memory access wrapper
 */
class mem_access_t {
public:
    mem_access_t(FtlLif& lif) :
        lif(lif),
        paddr(0),
        vaddr(nullptr),
        total_sz(0)
    {
    }

    /*
     * Move constructor
     */
    mem_access_t(mem_access_t &&m) noexcept :
        lif(m.lif),
        paddr(m.paddr),
        vaddr(obj_xchg(m.vaddr, nullptr)),
        total_sz(m.total_sz)
    {
    }

    ~mem_access_t();

    void reset(int64_t paddr,
               uint32_t total_sz,
               bool mmap_requested = true);
    void small_read(uint32_t offset,
                    uint8_t *buf,
                    uint32_t read_sz) const;
    void small_write(uint32_t offset,
                     const uint8_t *buf,
                     uint32_t write_sz) const;
    void large_read(uint32_t offset,
                    uint8_t *buf,
                    uint32_t read_sz) const;
    void large_write(uint32_t offset,
                     const uint8_t *buf,
                     uint32_t write_sz) const;
    void cache_invalidate(uint32_t offset = 0,
                          uint32_t sz = 0) const;
    uint64_t pa(void) const { return paddr; }
    volatile uint8_t *va(void) const { return vaddr; }


private:
    FtlLif&                 lif;
    int64_t                 paddr;
    volatile uint8_t        *vaddr;
    uint32_t                total_sz;
};

/*
 * Doorbell access wrapper
 */
class db_access_t {
public:
    db_access_t(FtlLif& lif) :
        lif(lif),
        db_access(lif)
    {
        db_addr = {0};
    }

    void reset(enum ftl_qtype qtype,
               uint32_t upd);
    void write32(uint32_t data);

private:
    FtlLif&                 lif;
    asic_db_addr_t          db_addr;
    mem_access_t            db_access;
};

/**
 * Queues control class
 */
class ftl_lif_queues_ctl_t {
public:
    ftl_lif_queues_ctl_t(FtlLif& lif,
                        enum ftl_qtype qtype,
                        uint32_t qcount);
    ~ftl_lif_queues_ctl_t();

    ftl_status_code_t init(const scanners_init_cmd_t *cmd);
    ftl_status_code_t init(const pollers_init_cmd_t *cmd);
    ftl_status_code_t init(const mpu_timestamp_init_cmd_t *cmd);

    ftl_status_code_t start(void);
    ftl_status_code_t sched_start_single(uint32_t qid);
    ftl_status_code_t stop(void);
    ftl_status_code_t sched_stop_single(uint32_t qid);
    ftl_status_code_t dequeue_burst(uint32_t qid,
                                    uint32_t *burst_count,
                                    uint8_t *buf,
                                    uint32_t buf_sz);
    ftl_status_code_t metrics_get(lif_attr_metrics_t *metrics);

    bool quiesce(void);
    void quiesce_idle(void);

    enum ftl_qtype qtype(void) { return qtype_; }
    uint32_t qcount(void) { return qcount_; }
    uint32_t qid_high(void) { return qid_high_; }
    uint32_t quiesce_qid(void) { return quiesce_qid_; }
    int64_t qid_qstate_addr(uint32_t qid);
    const mem_access_t *qid_qstate_access(uint32_t qid);
    const mem_access_t *qid_wring_access(uint32_t qid);

    bool empty_qstate_access(void) { return qstate_access.empty(); }
    bool empty_wring_access(void) { return wring_access.empty(); }

private:
    ftl_status_code_t scanner_init_single(const scanner_init_single_cmd_t *cmd);
    ftl_status_code_t poller_init_single(const poller_init_single_cmd_t *cmd);
    ftl_status_code_t pgm_pc_offset_get(const char *pc_jump_label,
                                        uint8_t *pc_offset);

    FtlLif&                 lif;
    enum ftl_qtype          qtype_;
    std::vector<mem_access_t> qstate_access;
    std::vector<mem_access_t> wring_access;
    db_access_t             db_pndx_inc;
    db_access_t             db_shed_clr;
    uint64_t                wrings_base_addr;
    uint32_t                wring_single_sz;
    uint32_t                slot_data_sz;
    uint32_t                qcount_;
    uint32_t                qdepth;
    uint32_t                qdepth_mask;
    uint32_t                qid_high_;
    uint32_t                quiesce_qid_;
    uint32_t                quiescing   : 1,
                            unused      : 31;
};

/**
 * MPU timestamp access class
 */
class mpu_timestamp_access_t {
public:
    mpu_timestamp_access_t(FtlLif& lif) :
        lif(lif),
        v_qstate(nullptr)
    {
    }

    void reset(volatile uint8_t *qstate_vaddr)
    {
        v_qstate = (volatile mpu_timestamp_qstate_t *)qstate_vaddr;
    }

    uint64_t curr_timestamp(void)
    {
        /*
         * Only HW platform has VA access to qstate;
         * SIM would not, but then SIM doesn't require timestamp anyway.
         */
        return v_qstate ? v_qstate->timestamp : 0;
    }

private:
    FtlLif&                 lif;
    volatile mpu_timestamp_qstate_t *v_qstate;
};

/**
 * LIF State Machine Context
 */
class ftl_lif_fsm_ctx_t {
public:
    ftl_lif_fsm_ctx_t() :
        state(FTL_LIF_ST_INITIAL),
        enter_state(FTL_LIF_ST_INITIAL),
        reset(0),
        reset_destroy(0)
    {
    }

    ftl_lif_state_t         state;
    ftl_lif_state_t         enter_state;
    ftl_timestamp_t         ts;
    uint32_t                reset               : 1,
                            reset_destroy       : 1;
};

/**
 * LIF Resource structure
 */
typedef struct {
    uint64_t                lif_id;
    uint32_t                index;
} ftl_lif_res_t;

/**
 * Ftl LIF
 */
class FtlLif {
public:
    FtlLif(FtlDev& ftl_dev,
           ftl_lif_res_t& lif_res,
           EV_P);
    ~FtlLif();

    ftl_status_code_t CmdHandler(ftl_devcmd_t *req,
                                 void *req_data,
                                 ftl_devcmd_cpl_t *rsp,
                                 void *rsp_data);
    ftl_status_code_t reset(bool destroy);
    void SetHalClient(devapi *dapi);
    void HalEventHandler(bool status);


    uint64_t LifIdGet(void) { return hal_lif_info_.lif_id; }
    const std::string& LifNameGet(void) { return lif_name; }
    uint64_t mpu_timestamp(void) { return mpu_timestamp_access.curr_timestamp(); }

    lif_info_t                  hal_lif_info_;

    friend class ftl_lif_queues_ctl_t;
    friend class mem_access_t;

    static ftl_lif_state_event_t lif_initial_ev_table[];
    static ftl_lif_state_event_t lif_wait_hal_ev_table[];
    static ftl_lif_state_event_t lif_pre_init_ev_table[];
    static ftl_lif_state_event_t lif_post_init_ev_table[];
    static ftl_lif_state_event_t lif_queues_reset_ev_table[];
    static ftl_lif_state_event_t lif_queues_pre_init_ev_table[];
    static ftl_lif_state_event_t lif_queues_init_transition_ev_table[];
    static ftl_lif_state_event_t lif_queues_stopping_ev_table[];
    static ftl_lif_state_event_t lif_queues_started_ev_table[];

private:
    std::string                 lif_name;
    FtlDev&                     ftl_dev;
    const ftl_devspec_t         *spec;
    struct queue_info           pd_qinfo[NUM_QUEUE_TYPES];
    ftl_lif_queues_ctl_t        session_scanners_ctl;
    ftl_lif_queues_ctl_t        conntrack_scanners_ctl;
    ftl_lif_queues_ctl_t        pollers_ctl;
    ftl_lif_queues_ctl_t        mpu_timestamp_ctl;
    mpu_timestamp_access_t      mpu_timestamp_access;

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    uint32_t                    index;
    uint8_t                     cosA, cosB, ctl_cosA, ctl_cosB;

    // Other states
    ftl_lif_fsm_ctx_t           fsm_ctx;
    age_tmo_cb_t                normal_age_tmo_cb;
    age_tmo_cb_t                accel_age_tmo_cb;
    mem_access_t                normal_age_access_;
    mem_access_t                accel_age_access_;

    EV_P;

    void ftl_lif_state_machine(ftl_lif_event_t event,
                               ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_null_action(ftl_lif_event_t event,
                                        ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_null_no_log_action(ftl_lif_event_t event,
                                               ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_eagain_action(ftl_lif_event_t event,
                                          ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_reject_action(ftl_lif_event_t event,
                                          ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_create_action(ftl_lif_event_t event,
                                          ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_destroy_action(ftl_lif_event_t event,
                                           ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_hal_up_action(ftl_lif_event_t event,
                                          ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_init_action(ftl_lif_event_t event,
                                        ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_setattr_action(ftl_lif_event_t event,
                                           ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_getattr_action(ftl_lif_event_t event,
                                           ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_identify_action(ftl_lif_event_t event,
                                            ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_reset_action(ftl_lif_event_t event,
                                         ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_pollers_init_action(ftl_lif_event_t event,
                                                ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_pollers_flush_action(ftl_lif_event_t event,
                                                 ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_pollers_deq_burst_action(ftl_lif_event_t event,
                                                     ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_scanners_init_action(ftl_lif_event_t event,
                                                 ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_scanners_start_action(ftl_lif_event_t event,
                                                  ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_scanners_start_single_action(ftl_lif_event_t event,
                                                         ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_scanners_stop_action(ftl_lif_event_t event,
                                                 ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_scanners_quiesce_action(ftl_lif_event_t event,
                                                    ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_mpu_timestamp_init_action(ftl_lif_event_t event,
                                                      ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_mpu_timestamp_start_action(ftl_lif_event_t event,
                                                       ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_mpu_timestamp_stop_action(ftl_lif_event_t event,
                                                      ftl_lif_devcmd_ctx_t& devcmd_ctx);
    ftl_lif_event_t ftl_lif_accel_aging_ctl_action(ftl_lif_event_t event,
                                                   ftl_lif_devcmd_ctx_t& devcmd_ctx);

    void age_tmo_cb_init(age_tmo_cb_t *age_tmo_cb,
                         const mem_access_t& access,
                         bool cb_select);
    void age_tmo_cb_set(const char *which,
                        age_tmo_cb_t *age_tmo_cb,
                        const mem_access_t& access,
                        const lif_attr_age_tmo_t *attr_age_tmo);
    void age_tmo_cb_get(lif_attr_age_tmo_t *attr_age_tmo,
                        const age_tmo_cb_t *age_tmo_cb);
    void force_session_expired_ts_set(age_tmo_cb_t *age_tmo_cb,
                                      const mem_access_t& access,
                                      uint8_t force_expired_ts);
    void force_conntrack_expired_ts_set(age_tmo_cb_t *age_tmo_cb,
                                        const mem_access_t& access,
                                        uint8_t force_expired_ts);
    ftl_status_code_t normal_age_tmo_cb_select(void);
    ftl_status_code_t accel_age_tmo_cb_select(void);

    const mem_access_t& normal_age_access(void) { return normal_age_access_; }
    const mem_access_t& accel_age_access(void) { return accel_age_access_; }
};

#endif
