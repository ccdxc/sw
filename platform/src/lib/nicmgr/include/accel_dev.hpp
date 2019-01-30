
#ifndef __ACCEL_DEV_HPP__
#define __ACCEL_DEV_HPP__

#include <map>
#include <ev++.h>

#include "nic/include/accel_ring.h"
#include "nic/include/storage_seq_common.h"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/linkmgr/linkmgr_types.hpp"

#include "platform/src/lib/hal_api/include/hal_types.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"

#include "dev.hpp"
#include "hal_client.hpp"

#define ACCEL_DEV_CONTROL_CORES_MASK    (1 << CONTROL_CORE_ID)
#define ACCEL_DEV_PAGE_SIZE             4096
#define ACCEL_DEV_PAGE_MASK             (ACCEL_DEV_PAGE_SIZE - 1)

#define ACCEL_DEV_ADDR_ALIGN(addr, sz)  \
    (((addr) + ((uint64_t)(sz) - 1)) & ~((uint64_t)(sz) - 1))

#define ACCEL_DEV_PAGE_ALIGN(addr)      \
    ACCEL_DEV_ADDR_ALIGN(addr, ACCEL_DEV_PAGE_SIZE)

enum {
    ACCEL_DEV_BAR0_DEV_CMD_REGS_PAGE    = 0,
    ACCEL_DEV_BAR0_DEV_CMD_DB_PAGE      = 1,
    ACCEL_DEV_BAR0_INTR_CTRL_PAGE       = 2,
    ACCEL_DEV_BAR0_INTR_STATUS_PAGE     = 3,

    ACCEL_DEV_BAR0_NUM_PAGES_MAX        = 8
};

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
#define DB_ADDR_BASE_LOCAL              0x8800000
#define ACCEL_LIF_DBADDR_UPD            0x0b
#define DB_UPD_SHFT                     17
#define DB_LIF_SHFT                     6
#define DB_TYPE_SHFT                    3
#define ACCEL_LIF_LOCAL_DBADDR_SET(lif, qtype)          \
    (((uint64_t)(lif) << DB_LIF_SHFT) |                 \
    ((uint64_t)(qtype) << DB_TYPE_SHFT) |               \
    ((uint64_t)(ACCEL_LIF_DBADDR_UPD) << DB_UPD_SHFT) | \
    DB_ADDR_BASE_LOCAL)

/*
 * Default publish interval fraction
 * e.g., 2 means 1/2 of a second
 */
#define ACCEL_DEV_PUB_INTV_FRAC_DFLT    2

/**
 * Accelerator Device Spec
 */
typedef struct accel_devspec {
    std::string name;
    // RES
    uint32_t lif_count;
    uint32_t seq_queue_count;
    uint32_t adminq_count;
    uint32_t intr_count;
    uint32_t pub_intv_frac; // publishing interval in fraction of second
    // PCIe
    uint8_t  pcie_port;
    std::string qos_group;
} accel_devspec_t;


typedef struct dev_cmd_regs dev_cmd_regs_t;

#ifdef ACCEL_DEV_CMD_ENUMERATE

/* Supply these for accel_dev_if.h */
#define dma_addr_t uint64_t

#include "nic/include/accel_dev_if.h"

#pragma pack(push, 1)

/**
 * Accelerator PF Devcmd Region
 */

typedef union dev_cmd {
    uint32_t                    words[16];
    admin_cmd_t                 cmd;
    nop_cmd_t                   nop;
    reset_cmd_t                 reset;
    identify_cmd_t              identify;
    lif_init_cmd_t              lif_init;
    adminq_init_cmd_t           adminq_init;
    seq_queue_init_cmd_t        seq_q_init;
    seq_queue_batch_init_cmd_t  seq_q_batch_init;
    seq_queue_control_cmd_t     q_control;
    seq_queue_batch_control_cmd_t q_batch_control;
} dev_cmd_t;

typedef union dev_cmd_cpl {
    uint32_t                    words[4];
    uint8_t                     status;
    admin_cpl_t                 cpl;
    nop_cpl_t                   nop;
    reset_cpl_t                 reset;
    identify_cpl_t              identify;
    lif_init_cpl_t              lif_init;
    adminq_init_cpl_t           adminq_init;
    seq_queue_init_cpl_t        seq_q_init;
    seq_queue_batch_init_cpl_t  seq_q_batch_init;
    seq_queue_control_cpl_t     q_control;
    seq_queue_batch_control_cpl_t q_batch_control;
} dev_cmd_cpl_t;
#pragma pack(pop)

typedef struct dev_cmd_regs {
    uint32_t                    signature;
    uint32_t                    done;
    dev_cmd_t                   cmd;
    dev_cmd_cpl_t               cpl;
    uint8_t data[2048] __attribute__((aligned (2048)));
} dev_cmd_regs_t;

static_assert(sizeof(dev_cmd_regs_t) == ACCEL_DEV_PAGE_SIZE);
static_assert((offsetof(dev_cmd_regs_t, cmd) % 4) == 0);
static_assert(sizeof(union dev_cmd) == 64);
static_assert((offsetof(dev_cmd_regs_t, cpl) % 4) == 0);
static_assert(sizeof(union dev_cmd_cpl) == 16);
static_assert((offsetof(dev_cmd_regs_t, data) % 4) == 0);

#endif /* ACCEL_DEV_CMD_ENUMERATE */

#ifndef _NICMGR_IF_HPP_

#include "gen/proto/nicmgr/accel_metrics.pb.h"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"

/**
 * Accelerator device ring group ring info
 */
typedef struct {
    delphi::objects::AccelHwRingInfoPtr delphi_ring;
    delphi::objects::AccelHwRingMetricsPtr delphi_metrics;
    accel_rgroup_rinfo_rsp_t    info;
    accel_rgroup_rindices_rsp_t indices;
    accel_rgroup_rmetrics_rsp_t metrics;
    uint64_t                    soft_resets;
} accel_rgroup_ring_t;

/*
 * Ring group map:
 * key = {ring_handle, sub_ring}
 * value = accel_rgroup_ring_t
 */
typedef uint64_t                            accel_rgroup_ring_key_t;
typedef std::map<accel_rgroup_ring_key_t,accel_rgroup_ring_t>  accel_rgroup_map_t;
typedef accel_rgroup_map_t::iterator        accel_rgroup_iter_t;
typedef accel_rgroup_map_t::const_iterator  accel_rgroup_iter_c;

static inline accel_rgroup_ring_key_t
accel_rgroup_ring_key_make(uint32_t ring_handle,
                           uint32_t sub_ring)
{
    return ((accel_rgroup_ring_key_t)(ring_handle) << 32) | sub_ring;
}

/*
 * Poll with timeout
 */
class Poller {

public:
  Poller(uint64_t timeout_us) : timeout_us(timeout_us) {}
  int operator()(std::function<int(void)> poll_func);

private:
  uint64_t  timeout_us;
};

typedef struct seq_queue_init_cmd seq_queue_init_cmd_t;
typedef struct seq_queue_control_cmd seq_queue_control_cmd_t;

/**
 * Accelerator work thread
 */
class AccelWorkThread {
public:
    AccelWorkThread(std::string thread_name,
                    uint64_t cores_mask,
                    void *user_ctx,
                    sdk::lib::thread_entry_func_t entry_func);
    ~AccelWorkThread();

    void *user_ctx_get(void) { return user_ctx; }
    void work_lock(void);
    void work_unlock(void);
    void work_notify(void);
    void wait_for_work(void);
    bool terminate_get(void) { return terminate; }
    void terminate_set(bool sense) { terminate = sense; }
    bool stop_work_get(void) { return stop_work; }
    void stop_work_set(bool sense) { stop_work = sense; }
    void yield(void) { if (thread->can_yield()) pthread_yield(); else usleep(5000); }
    void join(void);

private:
    std::string         thread_name;
    sdk::lib::thread    *thread;
    void                *user_ctx;
    pthread_mutex_t     work_mutex;
    pthread_cond_t      work_cond;
    bool                stop_work;
    bool                terminate;
};

/**
 * Devcmd Status codes
 */
enum DevcmdStatus
{
    DEVCMD_SUCCESS,
    DEVCMD_BUSY,
    DEVCMD_ERROR,
    DEVCMD_UNKNOWN,
};

/**
 * Accelerator PF Device
 */
class Accel_PF : public Device {
public:
    Accel_PF(HalClient *hal_client, void *dev_spec,
             const hal_lif_info_t *nicmgr_lif_info,
             PdClient *pd_client);

    void DevcmdHandler();
    enum DevcmdStatus CmdHandler(void *req, void *req_data,
                                 void *resp, void *resp_data);
    enum DevcmdStatus AdminCmdHandler(uint64_t lif_id,
                                      void *req, void *req_data,
                                      void *resp, void *resp_data);

    void HalEventHandler(bool status);
    void SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client);
    hal_lif_info_t *GetHalLifInfo(void) { return &hal_lif_info_; }
    virtual void ThreadsWaitJoin(void);
    uint32_t GetHalLifCount() { return spec->lif_count; }

    dev_cmd_regs_t              *devcmd;

    friend void *seq_queue_info_publish_thread(void *ctx);

private:
    // Device Spec
    const accel_devspec_t       *spec;
    struct queue_info           qinfo[NUM_QUEUE_TYPES];
    // Delphi
    delphi::objects::AccelPfInfoPtr delphi_pf;
    std::vector<delphi::objects::AccelSeqQueueInfoPtr> delphi_qinfo_vec;
    std::vector<delphi::objects::AccelSeqQueueMetricsPtr> delphi_qmetrics_vec;

    // PD Info
    PdClient *pd;
    // HAL Info
    HalClient                   *hal;
    HalCommonClient             *hal_common_client;
    hal_lif_info_t              hal_lif_info_;
    const hal_lif_info_t        *nicmgr_lif_info;
    uint8_t cosA, cosB;
    // PCIe info
    pciehdev_t                  *pdev;
    // Resources
    int32_t                     lif_base;
    uint32_t                    intr_base;
    // Devcmd
    uint64_t                    devcmd_mem_addr;
    uint64_t                    devcmddb_mem_addr;
    // CMB
    uint64_t                    cmb_mem_addr;
    uint32_t                    cmb_mem_size;
    // Other state
    uint32_t                    crypto_key_idx_base;
    uint32_t                    num_crypto_keys_max;
    uint32_t                    seq_created_count;
    uint32_t                    seq_qid_init_high;
    bool                        rgroup_indices_chg;
    bool                        rgroup_metrics_chg;
    // HW rings
    accel_ring_t                accel_ring_tbl[ACCEL_RING_ID_MAX];

    friend void accel_rgroup_rindices_rsp_cb(void *user_ctx,
                             const accel_rgroup_rindices_rsp_t& indices);
    friend void accel_rgroup_rmetrics_rsp_cb(void *user_ctx,
                             const accel_rgroup_rmetrics_rsp_t& metrics);

    bool CreateHostDevice();

    /* Methods */
    static void DevcmdPoll(void *obj);
    enum DevcmdStatus _DevcmdReset(void *req, void *req_data,
                                   void *resp, void *resp_data);
    enum DevcmdStatus _DevcmdIdentify(void *req, void *req_data,
                                      void *resp, void *resp_data);
    enum DevcmdStatus _DevcmdLifInit(void *req, void *req_data,
                                     void *resp, void *resp_data);
    enum DevcmdStatus _DevcmdAdminQueueInit(void *req, void *req_data,
                                            void *resp, void *resp_data);
    enum DevcmdStatus _DevcmdSeqQueueInit(void *req, void *req_data,
                                          void *resp, void *resp_data);
    enum DevcmdStatus _DevcmdSeqQueueInitComplete(void *req, void *req_data,
                                                  void *resp, void *resp_data);
    enum DevcmdStatus _DevcmdSeqQueueBatchInit(void *req, void *req_data,
                                               void *resp, void *resp_data);
    enum DevcmdStatus _DevcmdSeqQueueControl(void *req, void *req_data,
                                             void *resp, void *resp_data,
                                             bool enable);
    enum DevcmdStatus _DevcmdSeqQueueBatchControl(void *req, void *req_data,
                                                  void *resp, void *resp_data,
                                                  bool enable);
    enum DevcmdStatus _DevcmdCryptoKeyUpdate(void *req, void *req_data,
                                             void *resp, void *resp_data);
    enum DevcmdStatus _DevcmdSeqQueueSingleInit(const seq_queue_init_cmd_t *cmd);
    enum DevcmdStatus _DevcmdSeqQueueSingleControl(const seq_queue_control_cmd_t *cmd,
                                                   bool enable);

    // Tasks
    ev::timer                   sync_timer;     // timer to sync to hub
    evutil_timer                devcmd_timer;

    int DelphiDeviceInit(void);

    int accel_ring_info_get_all(void);
    int accel_ring_reset_all(void);
    int accel_ring_wait_quiesce_all(void);
    int accel_rgroup_add(void);
    int accel_rgroup_rings_add(void);
    int accel_rgroup_reset_set(bool reset_sense);
    int accel_rgroup_enable_set(bool enable_sense);
    int accel_rgroup_pndx_set(uint32_t val,
                              bool conditional);
    int accel_rgroup_rinfo_get(void);
    int accel_rgroup_rindices_get(void);
    int accel_rgroup_rmetrics_get(void);
    uint32_t accel_ring_num_pendings_get(const accel_rgroup_ring_t& rgroup_ring);
    int accel_ring_max_pendings_get(uint32_t& max_pendings);

    void periodic_sync(ev::timer &watcher, int revents);
    void delphi_update(void);
    const char*opcode_to_str(enum cmd_opcode opcode);
};

#endif /* _NICMGR_IF_HPP_ */

#endif
