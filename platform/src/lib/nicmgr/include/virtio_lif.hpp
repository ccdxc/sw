#ifndef __VIRTIO_LIF_HPP__
#define __VIRTIO_LIF_HPP__

#include <map>

/*
 * Queue info
 */

enum virtio_qtype {
    VIRTIO_QTYPE_RX = 0,
    VIRTIO_QTYPE_TX = 1,
};

#define VIRTIO_RXQ(x)                   ((x) << 1)
#define VIRTIO_TXQ(x)                   (VIRTIO_RXQ(x) + 1)
#define VIRTIO_CTRLQ                    VIRTIO_RXQ

#define VIRTIO_EDMAQ_COMP_POLL_US       (1000)
#define VIRTIO_EDMAQ_COMP_POLL_MAX      (10)

/*
 * Physical host address bit manipulation
 */
#define VIRTIO_PHYS_ADDR_HOST_POS       63
#define VIRTIO_PHYS_ADDR_HOST_MASK      0x1
#define VIRTIO_PHYS_ADDR_LIF_POS        52
#define VIRTIO_PHYS_ADDR_LIF_MASK       0x7ff

#define VIRTIO_PHYS_ADDR_FIELD_GET(addr, pos, mask)      \
    (((addr) >> (pos)) & (mask))
#define VIRTIO_PHYS_ADDR_FIELD_SET(pos, mask, val)       \
    (((uint64_t)((val) & (mask))) << (pos))

#define VIRTIO_PHYS_ADDR_HOST_GET(addr)                  \
    VIRTIO_PHYS_ADDR_FIELD_GET(addr, VIRTIO_PHYS_ADDR_HOST_POS, VIRTIO_PHYS_ADDR_HOST_MASK)
#define VIRTIO_PHYS_ADDR_HOST_SET(host)                  \
    VIRTIO_PHYS_ADDR_FIELD_SET(VIRTIO_PHYS_ADDR_HOST_POS, VIRTIO_PHYS_ADDR_HOST_MASK, host)

#define VIRTIO_PHYS_ADDR_LIF_GET(addr)                   \
    VIRTIO_PHYS_ADDR_FIELD_GET(addr, VIRTIO_PHYS_ADDR_LIF_POS, VIRTIO_PHYS_ADDR_LIF_MASK)
#define VIRTIO_PHYS_ADDR_LIF_SET(lif)                    \
    VIRTIO_PHYS_ADDR_FIELD_SET(VIRTIO_PHYS_ADDR_LIF_POS, VIRTIO_PHYS_ADDR_LIF_MASK, lif)

#define VIRTIO_PHYS_ADDR_HOST_LIF_SET(host, lif)         \
    (VIRTIO_PHYS_ADDR_HOST_SET(host) | VIRTIO_PHYS_ADDR_LIF_SET(lif))

/*
 *  Local doorbell data formation
 */
#define VIRTIO_DB_DATA_QID_SHIFT                24
#define VIRTIO_DB_DATA_RING_SHIFT               16
#define VIRTIO_DB_DATA_PINDEX_SHIFT             0
#define VIRTIO_LIF_LOCAL_DBDATA_SET(qid, ring, pindex) \
    (((uint64_t)(qid) << VIRTIO_DB_DATA_QID_SHIFT) |  \
     ((uint64_t)(ring) << VIRTIO_DB_DATA_RING_SHIFT) |  \
     ((uint64_t)(pindex) << VIRTIO_DB_DATA_PINDEX_SHIFT))

class VirtIOLif;

/**
 * Generic timestamp and expiry interval
 */
typedef struct {
    uint64_t    timestamp;
    uint64_t    expiry;
} virtio_timestamp_t;

/**
 * LIF State Machine
 */
typedef enum {
    VIRTIO_LIF_ST_INITIAL,
    VIRTIO_LIF_ST_WAIT_HAL,
    VIRTIO_LIF_ST_PRE_INIT,
    VIRTIO_LIF_ST_POST_INIT,
    VIRTIO_LIF_ST_MAX,

    VIRTIO_LIF_ST_SAME
} virtio_lif_state_t;

#define VIRTIO_LIF_STATE_STR_TABLE                                     \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_ST_INITIAL),                  \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_ST_WAIT_HAL),                 \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_ST_PRE_INIT),                 \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_ST_POST_INIT),                \

typedef enum {
    VIRTIO_LIF_EV_NULL,
    VIRTIO_LIF_EV_ANY,
    VIRTIO_LIF_EV_CREATE,
    VIRTIO_LIF_EV_DESTROY,
    VIRTIO_LIF_EV_HAL_UP,
    VIRTIO_LIF_EV_INIT,
    VIRTIO_LIF_EV_MAX
} virtio_lif_event_t;

#define VIRTIO_LIF_EVENT_STR_TABLE                                     \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_EV_NULL),                     \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_EV_ANY),                      \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_EV_CREATE),                   \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_EV_DESTROY),                  \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_EV_HAL_UP),                   \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_LIF_EV_INIT),                     \

typedef virtio_lif_event_t (VirtIOLif::*virtio_lif_action_t)(virtio_lif_event_t event);

typedef struct {
    virtio_lif_event_t       event;
    virtio_lif_action_t      action;
    virtio_lif_state_t       next_state;
} virtio_lif_state_event_t;

typedef struct {
    virtio_lif_state_event_t *state_event;
} virtio_lif_fsm_t;

typedef struct {
    virtio_lif_action_t      action;
    virtio_lif_state_t       next_state;
} virtio_lif_ordered_event_t;

/**
 * LIF State Machine Context
 */
typedef struct {
    void                    *req;
    void                    *req_data;
    void                    *resp;
    void                    *resp_data;
    virtio_status_code_t     status;
} virtio_lif_devcmd_ctx_t;

typedef struct {
    virtio_lif_state_t       state;
    virtio_lif_state_t       enter_state;
    virtio_lif_devcmd_ctx_t  devcmd;
    virtio_timestamp_t       ts;
    uint32_t                quiesce_qid;
    bool                    reset_destroy;
} virtio_lif_fsm_ctx_t;

/**
 * LIF Resource structure
 */
typedef struct virtio_lif_res_s {
    uint64_t lif_id;
    uint64_t intr_base;
    uint64_t cmb_mem_addr;
    uint64_t cmb_mem_size;
} virtio_lif_res_t;

/**
 * VirtIOerator LIF
 */
class VirtIOLif {
public:
    VirtIOLif(VirtIODev& virtio_dev,
             virtio_lif_res_t& lif_res,
             EV_P);
    ~VirtIOLif();

    void SetHalClient(devapi *dapi);
    void HalEventHandler(bool status);

    virtio_status_code_t
    Enable(virtio_dev_cmd_regs_t *regs_p);

    virtio_status_code_t
    Disable(virtio_dev_cmd_regs_t *regs_p);

    uint64_t LifIdGet(void) { return hal_lif_info_.lif_id; }
    const std::string& LifNameGet(void) { return lif_name; }

    virtio_dev_cmd_regs_t         *devcmd;
    lif_info_t                  hal_lif_info_;

    void ResetRxQstate(int rx_qid);

    void SetupRxQstate(int rx_qid,
                       uint64_t features,
                       uint64_t desc_addr,
                       uint64_t avail_addr,
                       uint64_t used_addr,
                       uint16_t intr,
                       uint16_t size);

    void ResetTxQstate(int tx_qid);


    void SetupTxQstate(int tx_qid,
                       uint64_t features,
                       uint64_t desc_addr,
                       uint64_t avail_addr,
                       uint64_t used_addr,
                       uint16_t intr,
                       uint16_t size);

    void NotifyTxQueue(int tx_qid);


    virtio_lif_event_t virtio_lif_null_action(virtio_lif_event_t event);
    virtio_lif_event_t virtio_lif_eagain_action(virtio_lif_event_t event);
    virtio_lif_event_t virtio_lif_reject_action(virtio_lif_event_t event);
    virtio_lif_event_t virtio_lif_create_action(virtio_lif_event_t event);
    virtio_lif_event_t virtio_lif_destroy_action(virtio_lif_event_t event);
    virtio_lif_event_t virtio_lif_hal_up_action(virtio_lif_event_t event);
    virtio_lif_event_t virtio_lif_init_action(virtio_lif_event_t event);

private:
    std::string                 lif_name;
    VirtIODev&                   virtio_dev;
    const virtio_devspec_t       *spec;
    struct queue_info           qinfo[NUM_QUEUE_TYPES];

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    uint8_t                     cosA, cosB, admin_cosA, admin_cosB, pc_rx, pc_tx;
    // Resources
    uint32_t                    intr_base;
    virtio_lif_fsm_ctx_t         fsm_ctx;

    EV_P;

    uint64_t GetQstateAddr(int qtype, int qid);

    void virtio_lif_state_machine(virtio_lif_event_t event);
};

#endif
