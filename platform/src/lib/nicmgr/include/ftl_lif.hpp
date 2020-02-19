#ifndef __FTL_LIF_HPP__
#define __FTL_LIF_HPP__

#include <map>
#include "nic/p4/ftl_dev/include/ftl_dev_shared.h"

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
#ifdef ELBA
#include "elb_top_csr_defines.h"
#include "elb_wa_csr_define.h"
#include "elb_ms_csr_define.h"

#ifdef __aarch64__
#define HW_DB_ADDR_LOCAL_CSR            (ELB_ADDR_BASE_DB_WA_OFFSET + \
                                         ELB_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS)
#else
#define HW_DB_ADDR_LOCAL_CSR            (ELB_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS)
#endif  // __aarch64__
#else
#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"
#include "cap_ms_c_hdr.h"
#ifdef __aarch64__
#define HW_DB_ADDR_LOCAL_CSR            (CAP_ADDR_BASE_DB_WA_OFFSET + \
                                         CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS)
#else
#define HW_DB_ADDR_LOCAL_CSR            (CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS)
#endif // __aarch64__
#endif // ELBA

#define FTL_LIF_DBADDR_UPD              0x0b
#define FTL_LIF_DBADDR_UPD_INC          0x0f
#define FTL_LIF_DBADDR_UPD_CLR          0x02
#define DB_UPD_SHFT                     17
#define DB_LIF_SHFT                     6
#define DB_TYPE_SHFT                    3
#define DB_QID_SHFT                     24
#define FTL_LIF_DBADDR_SET(lif, qtype, upd)                         \
    (((uint64_t)(lif) << DB_LIF_SHFT) |                             \
    ((uint64_t)(qtype) << DB_TYPE_SHFT) |                           \
    ((uint64_t)(upd) << DB_UPD_SHFT))

#define FTL_LIF_LOCAL_DBADDR_SET(lif, qtype, upd)                   \
    (FTL_LIF_DBADDR_SET(lif, qtype, upd) | HW_DB_ADDR_LOCAL_CSR)

#define FTL_LIF_DBDATA_SET(qid, pndx)                               \
    (((uint64_t)(qid) << DB_QID_SHFT) | (pndx))

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
typedef struct {
    uint64_t                timestamp;
    uint64_t                expiry;
} ftl_timestamp_t;

static inline uint64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * USEC_PER_SEC + tv.tv_usec);
}

static inline void
time_expiry_set(ftl_timestamp_t& ts,
                uint64_t expiry)
{
    ts.timestamp = timestamp();
    ts.expiry = expiry;
}

static inline bool
time_expiry_check(const ftl_timestamp_t& ts)
{
    return (ts.expiry == 0) ||
           ((timestamp() - ts.timestamp) > ts.expiry);
}

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
    FTL_DEV_INDEX_STRINGIFY(FTL_LIF_EV_ACCEL_AGING_CONTROL),        \
 
typedef ftl_lif_event_t (FtlLif::*ftl_lif_action_t)(ftl_lif_event_t event);

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
    uint8_t                 qdepth_shft;
} poller_init_single_cmd_t;

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

private:
    ftl_status_code_t scanner_init_single(const scanner_init_single_cmd_t *cmd);
    ftl_status_code_t poller_init_single(const poller_init_single_cmd_t *cmd);
    ftl_status_code_t pgm_pc_offset_get(const char *pc_jump_label,
                                        uint8_t *pc_offset);

    FtlLif&                 lif;
    enum ftl_qtype          qtype_;
    uint64_t                wrings_base_addr;
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
 * LIF State Machine Context
 */
typedef struct {
    ftl_devcmd_t            *req;
    void                    *req_data;
    ftl_devcmd_cpl_t        *rsp;
    void                    *rsp_data;
    ftl_status_code_t       status;
} ftl_lif_devcmd_ctx_t;

typedef struct {
    ftl_lif_state_t         state;
    ftl_lif_state_t         enter_state;
    ftl_lif_devcmd_ctx_t    devcmd;
    ftl_timestamp_t         ts;
    uint32_t                reset               : 1,
                            reset_destroy       : 1;
} ftl_lif_fsm_ctx_t;

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

    lif_info_t                  hal_lif_info_;

    friend class ftl_lif_queues_ctl_t;

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

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    uint32_t                    index;
    uint8_t                     cosA, cosB, ctl_cosA, ctl_cosB;

    // Controller memory
    uint64_t                    cmb_age_tmo_addr;
    uint32_t                    cmb_age_tmo_size;

    // Other state
    ftl_lif_fsm_ctx_t           fsm_ctx;
    age_tmo_cb_t                normal_age_tmo_cb;
    age_tmo_cb_t                accel_age_tmo_cb;

    EV_P;

    // Reset command anchor for use by FTL_LIF_EV_RESET_DESTROY
    lif_reset_cmd_t             devcmd_reset;

    void ftl_lif_state_machine(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_null_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_eagain_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_reject_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_create_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_destroy_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_hal_up_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_init_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_setattr_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_getattr_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_identify_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_reset_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_pollers_init_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_pollers_flush_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_pollers_deq_burst_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_scanners_init_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_scanners_start_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_scanners_start_single_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_scanners_stop_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_scanners_quiesce_action(ftl_lif_event_t event);
    ftl_lif_event_t ftl_lif_accel_aging_ctl_action(ftl_lif_event_t event);

    void age_tmo_cb_init(age_tmo_cb_t *age_tmo_cb,
                         uint64_t cb_addr,
                         bool cb_select);
    void age_tmo_cb_set(const char *which,
                        age_tmo_cb_t *age_tmo_cb,
                        uint64_t cb_addr,
                        const lif_attr_age_tmo_t *attr_age_tmo);
    void age_tmo_cb_get(lif_attr_age_tmo_t *attr_age_tmo,
                        const age_tmo_cb_t *age_tmo_cb);
    void force_session_expired_ts_set(age_tmo_cb_t *age_tmo_cb,
                                      uint64_t cb_addr,
                                      uint8_t force_expired_ts);
    void force_conntrack_expired_ts_set(age_tmo_cb_t *age_tmo_cb,
                                        uint64_t cb_addr,
                                        uint8_t force_expired_ts);
    ftl_status_code_t normal_age_tmo_cb_select(void);
    ftl_status_code_t accel_age_tmo_cb_select(void);

    uint64_t normal_age_cb_addr(void) { return cmb_age_tmo_addr; }
    uint64_t accel_age_cb_addr(void) { return cmb_age_tmo_addr + sizeof(age_tmo_cb_t); }
};

#endif
