#ifndef __NVME_LIF_HPP__
#define __NVME_LIF_HPP__

#include <map>

/*
 * Queue info
 */
// NVME Admin SQ/CQ are QID 0 in their respective id space.
#define NVME_ASQ_QID                     0
#define NVME_ACQ_QID                     0

#define NVME_ARMQ_ASQ_QID                0
#define NVME_ARMQ_ASQ_RING_SIZE         16

#define NVME_ARMQ_ACQ_QID                1
#define NVME_ARMQ_ACQ_RING_SIZE         16

#define NVME_ARMQ_NOTIFY_QID             2

#define NVME_ARMQ_EDMAQ_QID              3
#define LG2_NVME_ARMQ_EDMA_RING_SIZE     4
#define NVME_ARMQ_EDMA_RING_SIZE        (1 << LG2_NVME_ARMQ_EDMA_RING_SIZE)
#define NVME_EDMAQ_COMP_POLL_US         (1000)
#define NVME_EDMAQ_COMP_POLL_MAX        (10)

/*
 * Interrupt info
 */
#define NVME_ACQ_INTR_NUM               0

/*
 * Physical host address bit manipulation
 */
#define NVME_PHYS_ADDR_HOST_POS        63
#define NVME_PHYS_ADDR_HOST_MASK       0x1
#define NVME_PHYS_ADDR_LIF_POS         52
#define NVME_PHYS_ADDR_LIF_MASK        0x7ff

#define NVME_PHYS_ADDR_FIELD_GET(addr, pos, mask)      \
    (((addr) >> (pos)) & (mask))
#define NVME_PHYS_ADDR_FIELD_SET(pos, mask, val)       \
    (((uint64_t)((val) & (mask))) << (pos))

#define NVME_PHYS_ADDR_HOST_GET(addr)                  \
    NVME_PHYS_ADDR_FIELD_GET(addr, NVME_PHYS_ADDR_HOST_POS, NVME_PHYS_ADDR_HOST_MASK)
#define NVME_PHYS_ADDR_HOST_SET(host)                  \
    NVME_PHYS_ADDR_FIELD_SET(NVME_PHYS_ADDR_HOST_POS, NVME_PHYS_ADDR_HOST_MASK, host)

#define NVME_PHYS_ADDR_LIF_GET(addr)                   \
    NVME_PHYS_ADDR_FIELD_GET(addr, NVME_PHYS_ADDR_LIF_POS, NVME_PHYS_ADDR_LIF_MASK)
#define NVME_PHYS_ADDR_LIF_SET(lif)                    \
    NVME_PHYS_ADDR_FIELD_SET(NVME_PHYS_ADDR_LIF_POS, NVME_PHYS_ADDR_LIF_MASK, lif)


/*
 *  Local doorbell address formation
 */
#define NVME_DB_ADDR_BASE_LOCAL             0x8800000
#define NVME_LIF_DBADDR_UPD                 0x0b
#define NVME_DB_UPD_SHFT                     17
#define NVME_DB_LIF_SHFT                     6
#define NVME_DB_TYPE_SHFT                    3
#define NVME_LIF_LOCAL_DBADDR_SET(lif, qtype)          \
    (((uint64_t)(lif) << NVME_DB_LIF_SHFT) |                \
    ((uint64_t)(qtype) << NVME_DB_TYPE_SHFT) |              \
    ((uint64_t)(NVME_LIF_DBADDR_UPD) << NVME_DB_UPD_SHFT) | \
    NVME_DB_ADDR_BASE_LOCAL)

/*
 *  Local doorbell data formation
 */
#define NVME_DB_DATA_QID_SHIFT               24
#define NVME_DB_DATA_RING_SHIFT              16
#define NVME_DB_DATA_PINDEX_SHIFT            0
#define NVME_LIF_LOCAL_DBDATA_SET(qid, ring, pindex) \
    (((uint64_t)(qid) << NVME_DB_DATA_QID_SHIFT) |  \
     ((uint64_t)(ring) << NVME_DB_DATA_RING_SHIFT) |  \
     ((uint64_t)(pindex) << NVME_DB_DATA_PINDEX_SHIFT))

class NvmeLif;

/**
 * Generic timestamp and expiry interval
 */
typedef struct {
    uint64_t    timestamp;
    uint64_t    expiry;
} nvme_timestamp_t;

/**
 * LIF State Machine
 */
typedef enum {
    NVME_LIF_ST_INITIAL,
    NVME_LIF_ST_WAIT_HAL,
    NVME_LIF_ST_PRE_INIT,
    NVME_LIF_ST_POST_INIT,
    NVME_LIF_ST_MAX,

    NVME_LIF_ST_SAME
} nvme_lif_state_t;

#define NVME_LIF_STATE_STR_TABLE                                     \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_ST_INITIAL),                  \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_ST_WAIT_HAL),                 \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_ST_PRE_INIT),                 \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_ST_POST_INIT),                \

typedef enum {
    NVME_LIF_EV_NULL,
    NVME_LIF_EV_ANY,
    NVME_LIF_EV_CREATE,
    NVME_LIF_EV_DESTROY,
    NVME_LIF_EV_HAL_UP,
    NVME_LIF_EV_INIT,
    NVME_LIF_EV_MAX
} nvme_lif_event_t;

#define NVME_LIF_EVENT_STR_TABLE                                     \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_EV_NULL),                     \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_EV_ANY),                      \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_EV_CREATE),                   \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_EV_DESTROY),                  \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_EV_HAL_UP),                   \
    NVME_DEV_INDEX_STRINGIFY(NVME_LIF_EV_INIT),                     \

typedef nvme_lif_event_t (NvmeLif::*nvme_lif_action_t)(nvme_lif_event_t event);

typedef struct {
    nvme_lif_event_t       event;
    nvme_lif_action_t      action;
    nvme_lif_state_t       next_state;
} nvme_lif_state_event_t;

typedef struct {
    nvme_lif_state_event_t *state_event;
} nvme_lif_fsm_t;

typedef struct {
    nvme_lif_action_t      action;
    nvme_lif_state_t       next_state;
} nvme_lif_ordered_event_t;

/**
 * LIF State Machine Context
 */
typedef struct {
    void                    *req;
    void                    *req_data;
    void                    *resp;
    void                    *resp_data;
    nvme_status_code_t     status;
} nvme_lif_devcmd_ctx_t;

typedef struct {
    nvme_lif_state_t       state;
    nvme_lif_state_t       enter_state;
    nvme_lif_devcmd_ctx_t  devcmd;
    nvme_timestamp_t       ts;
    uint32_t                quiesce_qid;
    bool                    reset_destroy;
} nvme_lif_fsm_ctx_t;

/**
 * LIF Resource structure
 */
typedef struct nvme_lif_res_s {
    uint64_t lif_id;
    uint64_t intr_base;
    uint64_t cmb_mem_addr;
    uint64_t cmb_mem_size;
} nvme_lif_res_t;

/**
 * Nvmeerator LIF
 */
class NvmeLif {
public:
    NvmeLif(NvmeDev& nvme_dev,
             nvme_lif_res_t& lif_res,
             EV_P);
    ~NvmeLif();

    nvme_status_code_t
    CmdHandler(void *req, void *req_data, void *resp, void *resp_data);

    nvme_status_code_t
    Enable(nvme_dev_cmd_regs_t *regs_p);

    nvme_status_code_t
    Disable(nvme_dev_cmd_regs_t *regs_p);

    void SetHalClient(devapi *dapi);
    void HalEventHandler(bool status);

    uint64_t LifIdGet(void) { return hal_lif_info_.lif_id; }
    const std::string& LifNameGet(void) { return lif_name; }

    nvme_dev_cmd_regs_t         *devcmd;
    lif_info_t                  hal_lif_info_;

    friend void NvmeAdminCmdHandler(void *obj, void *req, void *req_data,
                                    void *resp, void *resp_data);

    nvme_lif_event_t nvme_lif_null_action(nvme_lif_event_t event);
    nvme_lif_event_t nvme_lif_eagain_action(nvme_lif_event_t event);
    nvme_lif_event_t nvme_lif_reject_action(nvme_lif_event_t event);
    nvme_lif_event_t nvme_lif_create_action(nvme_lif_event_t event);
    nvme_lif_event_t nvme_lif_destroy_action(nvme_lif_event_t event);
    nvme_lif_event_t nvme_lif_hal_up_action(nvme_lif_event_t event);
    nvme_lif_event_t nvme_lif_init_action(nvme_lif_event_t event);
    nvme_lif_event_t nvme_lif_edmaq_init_action(nvme_lif_event_t event);
    nvme_lif_event_t nvme_lif_adminq_init_action(nvme_lif_event_t event);

private:
    std::string                 lif_name;
    NvmeDev&                   nvme_dev;
    const nvme_devspec_t       *spec;
    struct queue_info           qinfo[NUM_QUEUE_TYPES];

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    uint8_t                     cosA, cosB, admin_cosA, admin_cosB;
    // Resources
    uint32_t                    intr_base;
    nvme_lif_fsm_ctx_t         fsm_ctx;

    uint64_t cq_ring_base;
    uint64_t aq_ring_base;
    uint16_t sq_head;
    uint16_t cq_ring_size;
    uint16_t cq_head;
    uint16_t cq_color;
    uint64_t cq_intr_assert_addr;

    // EdmaQ
    uint16_t edma_ring_head;
    uint16_t edma_comp_tail;
    uint16_t edma_exp_color;
    uint64_t edma_ring_base;
    uint64_t edma_comp_base;
    uint64_t edma_buf_base;
    uint64_t edma_buf_base2;

    /* AdminQ Commands */
    AdminQ                      *adminq;

    EV_P;

    void nvme_lif_state_machine(nvme_lif_event_t event);
};

#endif
