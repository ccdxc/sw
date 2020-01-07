#ifndef __ACCEL_LIF_HPP__
#define __ACCEL_LIF_HPP__

#include <map>
#include "accel_lif_utils.hpp"

/*
 * Queue info
 */
#define ACCEL_ADMINQ_REQ_QTYPE            STORAGE_SEQ_QTYPE_ADMIN
#define ACCEL_ADMINQ_ADMIN_QID            0
#define ACCEL_ADMINQ_REQ_QID              1
#define ACCEL_ADMINQ_REQ_RING_SIZE        16

#define ACCEL_ADMINQ_RESP_QTYPE           STORAGE_SEQ_QTYPE_ADMIN
#define ACCEL_ADMINQ_RESP_QID             2
#define ACCEL_ADMINQ_RESP_RING_SIZE       16

#define ACCEL_NOTIFYQ_TX_QTYPE            STORAGE_SEQ_QTYPE_NOTIFY
#define ACCEL_NOTIFYQ_TX_QID              0

/*
 * Physical host address bit manipulation
 */
#define ACCEL_PHYS_ADDR_HOST_POS        63
#define ACCEL_PHYS_ADDR_HOST_MASK       0x1
#define ACCEL_PHYS_ADDR_LIF_POS         52
#define ACCEL_PHYS_ADDR_LIF_MASK        0x7ff

#define ACCEL_PHYS_ADDR_FIELD_GET(addr, pos, mask)      \
    (((addr) >> (pos)) & (mask))
#define ACCEL_PHYS_ADDR_FIELD_SET(pos, mask, val)       \
    (((uint64_t)((val) & (mask))) << (pos))

#define ACCEL_PHYS_ADDR_HOST_GET(addr)                  \
    ACCEL_PHYS_ADDR_FIELD_GET(addr, ACCEL_PHYS_ADDR_HOST_POS, ACCEL_PHYS_ADDR_HOST_MASK)
#define ACCEL_PHYS_ADDR_HOST_SET(host)                  \
    ACCEL_PHYS_ADDR_FIELD_SET(ACCEL_PHYS_ADDR_HOST_POS, ACCEL_PHYS_ADDR_HOST_MASK, host)

#define ACCEL_PHYS_ADDR_LIF_GET(addr)                   \
    ACCEL_PHYS_ADDR_FIELD_GET(addr, ACCEL_PHYS_ADDR_LIF_POS, ACCEL_PHYS_ADDR_LIF_MASK)
#define ACCEL_PHYS_ADDR_LIF_SET(lif)                    \
    ACCEL_PHYS_ADDR_FIELD_SET(ACCEL_PHYS_ADDR_LIF_POS, ACCEL_PHYS_ADDR_LIF_MASK, lif)

/*
 * Local doorbell address formation
 */
#define ACCEL_LIF_DBADDR_UPD            0x0b
#define DB_UPD_SHFT                     17
#define DB_LIF_SHFT                     6
#define DB_TYPE_SHFT                    3
#define DB_QID_SHFT                     24
#define ACCEL_LIF_DBADDR_SET(lif, qtype)                \
    (((uint64_t)(lif) << DB_LIF_SHFT) |                 \
    ((uint64_t)(qtype) << DB_TYPE_SHFT) |               \
    ((uint64_t)(ACCEL_LIF_DBADDR_UPD) << DB_UPD_SHFT))

#define ACCEL_LIF_LOCAL_DBADDR_SET(lif, qtype)          \
    (ACCEL_LIF_DBADDR_SET(lif, qtype) | HW_DB_ADDR_LOCAL_CSR)

#define ACCEL_LIF_DBDATA_SET(qid, pndx)                 \
    (((uint64_t)(qid) << DB_QID_SHFT) | (pndx))

class AccelLif;

extern std::vector<std::pair<const std::string,uint32_t>> accel_ring_vec;

/**
 * Accelerator device ring group ring info
 */
typedef struct {
#ifdef NICMGR_DELPHI_METRICS_ENABLE
    delphi::objects::AccelHwRingMetricsPtr delphi_metrics;
#endif
    accel_rgroup_rinfo_rsp_t    info;
    accel_rgroup_rindices_rsp_t indices;
    accel_rgroup_rmetrics_rsp_t metrics;
} accel_rgroup_ring_t;

static inline bool
is_power_of_2(uint64_t n)
{
    return n && !(n & (n - 1));
}

/*
 * Ring group map:
 * key = {ring_handle, sub_ring}
 * value = accel_rgroup_ring_t
 */
typedef uint64_t                         accel_rgroup_ring_key_t;
typedef std::map<accel_rgroup_ring_key_t,accel_rgroup_ring_t> accel_rgroup_map_t;

static inline accel_rgroup_ring_key_t
accel_rgroup_ring_key_make(uint32_t ring_handle,
                           uint32_t sub_ring)
{
    return ((accel_rgroup_ring_key_t)(ring_handle) << 32) | sub_ring;
}

static inline void
accel_rgroup_ring_key_extract(accel_rgroup_ring_key_t key,
                              uint32_t& ring_handle,
                              uint32_t& sub_ring)
{
    ring_handle = key >> 32;
    sub_ring = key & 0xffffffff;
}

/**
 * Sequencer queue info metrics layout for Delphi
 */
typedef struct {
    uint64_t    qstate_addr;
    uint64_t    qgroup;
    uint64_t    core_id;
} __attribute__((packed)) seq_queue_info_metrics_t;

/**
 * LIF State Machine
 */
typedef enum {
    ACCEL_LIF_ST_INITIAL,
    ACCEL_LIF_ST_WAIT_HAL,
    ACCEL_LIF_ST_PRE_INIT,
    ACCEL_LIF_ST_POST_INIT,
    ACCEL_LIF_ST_SEQ_QUEUE_RESET,
    ACCEL_LIF_ST_RGROUP_QUIESCE,
    ACCEL_LIF_ST_RGROUP_RESET,
    ACCEL_LIF_ST_POST_INIT_POST_RESET,
    ACCEL_LIF_ST_SEQ_QUEUE_PRE_INIT,
    ACCEL_LIF_ST_SEQ_QUEUE_INIT,
    ACCEL_LIF_ST_IDLE,
    ACCEL_LIF_ST_MAX,

    ACCEL_LIF_ST_SAME
} accel_lif_state_t;

#define ACCEL_LIF_STATE_STR_TABLE                                     \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_INITIAL),                  \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_WAIT_HAL),                 \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_PRE_INIT),                 \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_POST_INIT),                \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_SEQ_QUEUE_RESET),          \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_RGROUP_QUIESCE),           \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_RGROUP_RESET),             \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_POST_INIT_POST_RESET),     \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_SEQ_QUEUE_PRE_INIT),       \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_SEQ_QUEUE_INIT),           \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_ST_IDLE),                     \

typedef enum {
    ACCEL_LIF_EV_NULL,
    ACCEL_LIF_EV_ANY,
    ACCEL_LIF_EV_CREATE,
    ACCEL_LIF_EV_DESTROY,
    ACCEL_LIF_EV_HAL_UP,
    ACCEL_LIF_EV_IDENTIFY,
    ACCEL_LIF_EV_INIT,
    ACCEL_LIF_EV_SETATTR,
    ACCEL_LIF_EV_GETATTR,
    ACCEL_LIF_EV_RESET,
    ACCEL_LIF_EV_RESET_DESTROY,
    ACCEL_LIF_EV_WAIT_SEQ_QUEUE_QUIESCE,
    ACCEL_LIF_EV_WAIT_RGROUP_QUIESCE,
    ACCEL_LIF_EV_RGROUP_RESET,
    ACCEL_LIF_EV_SEQ_QUEUE_PRE_INIT,
    ACCEL_LIF_EV_PRE_INIT,
    ACCEL_LIF_EV_ADMINQ_INIT,
    ACCEL_LIF_EV_NOTIFYQ_INIT,
    ACCEL_LIF_EV_SEQ_QUEUE_INIT,
    ACCEL_LIF_EV_SEQ_QUEUE_BATCH_INIT,
    ACCEL_LIF_EV_SEQ_QUEUE_INIT_COMPLETE,
    ACCEL_LIF_EV_SEQ_QUEUE_ENABLE,
    ACCEL_LIF_EV_SEQ_QUEUE_DISABLE,
    ACCEL_LIF_EV_SEQ_QUEUE_BATCH_ENABLE,
    ACCEL_LIF_EV_SEQ_QUEUE_BATCH_DISABLE,
    ACCEL_LIF_EV_CRYPTO_KEY_UPDATE,
    ACCEL_LIF_EV_HANG_NOTIFY,

    ACCEL_LIF_EV_MAX
} accel_lif_event_t;

#define ACCEL_LIF_EVENT_STR_TABLE                                     \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_NULL),                     \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_ANY),                      \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_CREATE),                   \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_DESTROY),                  \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_HAL_UP),                   \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_IDENTIFY),                 \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_INIT),                     \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SETATTR),                  \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_GETATTR),                  \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_RESET),                    \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_RESET_DESTROY),            \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_WAIT_SEQ_QUEUE_QUIESCE),   \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_WAIT_RGROUP_QUIESCE),      \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_RGROUP_RESET),             \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SEQ_QUEUE_PRE_INIT),       \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_PRE_INIT),                 \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_ADMINQ_INIT),              \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_NOTIFYQ_INIT),             \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SEQ_QUEUE_INIT),           \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SEQ_QUEUE_BATCH_INIT),     \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SEQ_QUEUE_INIT_COMPLETE),  \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SEQ_QUEUE_ENABLE),         \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SEQ_QUEUE_DISABLE),        \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SEQ_QUEUE_BATCH_ENABLE),   \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_SEQ_QUEUE_BATCH_DISABLE),  \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_CRYPTO_KEY_UPDATE),        \
    ACCEL_DEV_INDEX_STRINGIFY(ACCEL_LIF_EV_HANG_NOTIFY),              \

typedef accel_lif_event_t (AccelLif::*accel_lif_action_t)(accel_lif_event_t event);

typedef struct {
    accel_lif_event_t       event;
    accel_lif_action_t      action;
    accel_lif_state_t       next_state;
} accel_lif_state_event_t;

typedef struct {
    accel_lif_state_event_t *state_event;
} accel_lif_fsm_t;

typedef struct {
    accel_lif_action_t      action;
    accel_lif_state_t       next_state;
} accel_lif_ordered_event_t;

/**
 * LIF State Machine Context
 */
typedef struct {
    void                    *req;
    void                    *req_data;
    void                    *resp;
    void                    *resp_data;
    accel_status_code_t     status;
} accel_lif_devcmd_ctx_t;

typedef struct {
    accel_lif_state_t       state;
    accel_lif_state_t       enter_state;
    accel_lif_devcmd_ctx_t  devcmd;
    AccelLifUtils::accel_timestamp_t ts;
    uint32_t                quiesce_qid;
    uint32_t                reset_destroy      : 1,
                            info_dump          : 1,
                            uncond_desc_notify : 1;
    enum desc_notify_type   desc_notify_type;    
} accel_lif_fsm_ctx_t;

/**
 * LIF Resource structure
 */
typedef struct eth_lif_res_s {
    uint64_t lif_id;
    uint64_t intr_base;
    uint64_t cmb_mem_addr;
    uint64_t cmb_mem_size;
    uint32_t index;
} accel_lif_res_t;

/**
 * Map many ring_handle types to fewer descriptor types
 */
typedef enum {
    ACCEL_RING_DESC_VOID,
    ACCEL_RING_DESC_CPDC,
    ACCEL_RING_DESC_CRYPTO_SYMM,
} accel_ring_desc_type_t;

/**
 * Accelerator LIF
 */
class AccelLif {
public:
    AccelLif(AccelDev& accel_dev,
             accel_lif_res_t& lif_res,
             EV_P);
    ~AccelLif();

    accel_status_code_t
    CmdHandler(void *req, void *req_data, void *resp, void *resp_data);

    accel_status_code_t reset(bool destroy);
    void SetHalClient(devapi *dapi);
    void HalEventHandler(bool status);

    uint64_t LifIdGet(void) { return hal_lif_info_.lif_id; }
    const std::string& LifNameGet(void) { return lif_name; }
    uint32_t SeqCreatedCountGet(void) { return seq_created_count; }
    const accel_ring_t *AccelRingTableGet(void) { return accel_ring_tbl; }

    lif_info_t                  hal_lif_info_;

    friend void AdminCmdHandler(void *obj, void *req, void *req_data,
                                void *resp, void *resp_data);
    friend void accel_rgroup_rinfo_rsp_cb(void *user_ctx,
                             const accel_rgroup_rinfo_rsp_t& info);
    friend void accel_rgroup_rindices_rsp_cb(void *user_ctx,
                             const accel_rgroup_rindices_rsp_t& indices);
    friend void accel_rgroup_rmetrics_rsp_cb(void *user_ctx,
                             const accel_rgroup_rmetrics_rsp_t& metrics);
    friend void accel_rgroup_rmisc_rsp_cb(void *user_ctx,
                             const accel_rgroup_rmisc_rsp_t& misc);
    friend void AccelLifUtils::HwMonitor::ErrPoll(void *obj);

    accel_lif_event_t accel_lif_null_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_eagain_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_reject_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_create_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_destroy_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_hal_up_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_ring_info_get_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_init_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_setattr_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_getattr_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_identify_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_reset_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_seq_quiesce_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_rgroup_quiesce_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_rgroup_reset_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_adminq_init_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_notifyq_init_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_seq_queue_init_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_seq_queue_control_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_seq_queue_batch_init_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_seq_queue_batch_control_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_seq_queue_init_cpl_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_crypto_key_update_action(accel_lif_event_t event);
    accel_lif_event_t accel_lif_hang_notify_action(accel_lif_event_t event);

private:
    std::string                 lif_name;
    AccelDev&                   accel_dev;
    const accel_devspec_t       *spec;
    struct queue_info           qinfo[NUM_QUEUE_TYPES];
    accel_rgroup_map_t          rgroup_map;

#ifdef NICMGR_DELPHI_METRICS_ENABLE
    // Delphi
    std::vector<delphi::objects::AccelSeqQueueInfoMetricsPtr> delphi_qinfo_vec;
    std::vector<delphi::objects::AccelSeqQueueMetricsPtr> delphi_qmetrics_vec;
#endif

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    uint32_t                    index;
    uint8_t                     cosA, cosB, admin_cosA, admin_cosB;

    // Resources
    AccelLifUtils::DetectionLogControl hang_detect_log;
    uint32_t                    intr_base;
    uint32_t                    crypto_key_idx_base;
    uint32_t                    num_crypto_keys_max;
    // CMB
    uint64_t                    cmb_qinfo_addr;
    uint64_t                    cmb_rmetrics_addr;
    uint32_t                    cmb_qinfo_size;
    uint32_t                    cmb_rmetrics_size;
    // Other state
    uint32_t                    seq_created_count;
    uint32_t                    seq_qid_init_high;
    uint32_t                    reason_code;
    accel_lif_fsm_ctx_t         fsm_ctx;

    EV_P;
    AccelLifUtils::HwMonitor    *hw_mon;

    // HW rings
    accel_ring_t                accel_ring_tbl[ACCEL_RING_ID_MAX];

    accel_rgroup_ring_t&        accel_rgroup_find_create(uint32_t ring_handle,
                                                         uint32_t sub_ring);
    /* AdminQ Commands */
    AdminQ                      *adminq;
    AccelLifUtils::NotifyQ      *notifyq;
    uint64_t                    event_id;

    uint64_t next_event_id_get(void) { return ++event_id; }

    accel_status_code_t
    _DevcmdSeqQueueSingleInit(const seq_queue_init_cmd_t *cmd);

    accel_status_code_t
    _DevcmdSeqQueueSingleControl(const seq_queue_control_cmd_t *cmd,
                                 bool enable);

    accel_ring_t *accel_ring_get(uint32_t ring_handle,
                                 uint32_t sub_ring);
    int accel_ring_info_get_all(void);
    void accel_ring_info_del_all(void);
    int accel_rgroup_add(void);
    void accel_rgroup_del(void);
    int accel_rgroup_rings_add(void);
    void accel_rgroup_rings_del(void);
    int accel_rgroup_reset_set(bool reset_sense);
    int accel_rgroup_enable_set(bool enable_sense);
    int accel_rgroup_pndx_set(uint32_t val,
                              bool conditional);
    int accel_rgroup_rinfo_get(void);
    int accel_rgroup_rindices_get(void);
    int accel_rgroup_rmetrics_get(void);
    int accel_rgroup_rmisc_get(void);
    uint32_t accel_ring_num_pendings_get(const accel_rgroup_ring_t& rgroup_ring);
    int accel_ring_max_pendings_get(uint32_t& max_pendings);
    void accel_ring_desc_notify(const accel_ring_t *ring,
                                uint32_t desc_idx);
    void accel_ring_desc_notify(const accel_ring_t *ring,
                                uint32_t desc_idx,
                                const uint8_t *desc,
                                uint32_t desc_size);
    uint32_t accel_ring_desc_read(const accel_ring_t *ring,
                                  uint32_t desc_idx,
                                  uint8_t *desc,
                                  uint32_t desc_size);
    bool accel_ring_desc_valid(const accel_ring_t *ring,
                               const uint8_t *desc,
                               uint32_t desc_size,
                               accel_ring_desc_type_t *ret_desc_type);
    void accel_ring_desc_log(const accel_ring_t *ring,
                             uint32_t desc_idx,
                             const uint8_t *desc,
                             accel_ring_desc_type_t desc_type);
    void accel_ring_desc_data_notify(const accel_ring_t *ring,
                                     uint32_t desc_idx,
                                     const uint8_t *desc,
                                     uint32_t desc_size);
    void accel_ring_desc_addr_notify(const accel_ring_t *ring,
                                     uint32_t desc_idx,
                                     const uint8_t *desc,
                                     accel_ring_desc_type_t desc_type);
    bool accel_ring_desc_idx_delta(const accel_ring_t *ring,
                                   uint32_t desc_idx,
                                   int delta,
                                   uint32_t *ret_idx);
    int qmetrics_init(void);
    void qmetrics_fini(void);
    uint64_t rmetrics_addr_get(uint32_t ring_handle,
                               uint32_t sub_ring);
    uint64_t qinfo_metrics_addr_get(uint32_t qid);
    void qinfo_metrics_update(uint32_t qid,
                              uint64_t qstate_addr,
                              const storage_seq_qstate_t& qstate);
    void accel_lif_state_machine(accel_lif_event_t event);
};

/**
 * Some well known ring constants
 */
#define ACCEL_RING_MAX_DESC_SIZE                                        \
    ((sizeof(barco_symm_req_descriptor_t) >                    \
                            sizeof(hal::pd::cpdc_descriptor_t)) ?       \
    sizeof(barco_symm_req_descriptor_t) :                      \
    sizeof(hal::pd::cpdc_descriptor_t))

#define ACCEL_RING_MAX_PREFETCH_DESCS   16

static inline accel_ring_desc_type_t
accel_ring_desc_type_get(const accel_ring_t *ring)
{
    if (ring) {
        switch (ring->ring_id) {

        case ACCEL_RING_XTS0:
        case ACCEL_RING_XTS1:
        case ACCEL_RING_GCM0:
        case ACCEL_RING_GCM1:
            return ACCEL_RING_DESC_CRYPTO_SYMM;

        case ACCEL_RING_CP:
        case ACCEL_RING_CP_HOT:
        case ACCEL_RING_DC:
        case ACCEL_RING_DC_HOT:
            return ACCEL_RING_DESC_CPDC;

        default:
            break;
        }
    }

    return ACCEL_RING_DESC_VOID;
}
    
static inline bool
accel_ring_config_valid(const accel_ring_t *ring)
{
    return ring && ring->ring_size && ring->ring_desc_size;
}


#endif
