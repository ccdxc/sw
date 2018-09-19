
#ifndef __ACCEL_DEV_HPP__
#define __ACCEL_DEV_HPP__

#include <map>
#include <unordered_map>

#include "dev.hpp"
#include "pci_ids.h"
#include "misc.h"
#include "bdf.h"
#include "cfgspace.h"
#include "pciehost.h"
#include "pciehdevices.h"
#include "pciehw.h"
#include "pcieport.h"
#include "hal_client.hpp"
#include "accel_ring.h"
#include "storage_seq_common.h"

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
#define ACCEL_PHYS_ADDR_FIELD_SET(addr, pos, mask, val) \
    addr |= (((uint64_t)((val) & (mask))) << (pos))
    
#define ACCEL_PHYS_ADDR_HOST_GET(addr)                  \
    ACCEL_PHYS_ADDR_FIELD_GET(addr, ACCEL_PHYS_ADDR_HOST_POS, ACCEL_PHYS_ADDR_HOST_MASK)
#define ACCEL_PHYS_ADDR_HOST_SET(addr, host)            \
    ACCEL_PHYS_ADDR_FIELD_SET(addr, ACCEL_PHYS_ADDR_HOST_POS, ACCEL_PHYS_ADDR_HOST_MASK, host)
    
#define ACCEL_PHYS_ADDR_LIF_GET(addr)                   \
    ACCEL_PHYS_ADDR_FIELD_GET(addr, ACCEL_PHYS_ADDR_LIF_POS, ACCEL_PHYS_ADDR_LIF_MASK)
#define ACCEL_PHYS_ADDR_LIF_SET(addr, lif)              \
    ACCEL_PHYS_ADDR_FIELD_SET(addr, ACCEL_PHYS_ADDR_LIF_POS, ACCEL_PHYS_ADDR_LIF_MASK, lif)
    
/**
 * Accelerator Device Spec
 */
typedef struct accel_devspec {
    // RES
    uint64_t lif_id;
    uint32_t seq_queue_base;
    uint32_t seq_queue_count;
    uint32_t seq_created_count;
    uint32_t adminq_base;
    uint32_t adminq_count;
    uint32_t intr_base;
    uint32_t intr_count;
    // PCIe
    uint8_t  pcie_port;

    // HW rings
    accel_ring_t accel_ring_tbl[ACCEL_RING_ID_MAX];
} accel_devspec_t;


typedef struct dev_cmd_regs dev_cmd_regs_t;

#ifdef ACCEL_DEV_CMD_ENUMERATE

/* Supply these for accel_dev_if.h */
#define dma_addr_t uint64_t

#include "accel_dev_if.h"

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
    seq_queue_control_cmd_t     q_control;
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
    seq_queue_control_cpl_t     q_control;
} dev_cmd_cpl_t;
#pragma pack(pop)

typedef struct dev_cmd_regs {
    uint32_t                    signature;
    uint32_t                    done;
    dev_cmd_t                   cmd;
    dev_cmd_cpl_t               cpl;
    uint8_t data[2048] __attribute__((aligned (2048)));
} dev_cmd_regs_t;

#endif /* ACCEL_DEV_CMD_ENUMERATE */

/**
 * Accelerator device ring group ring info
 */
typedef struct {
    accel_rgroup_rinfo_rsp_t    info;
    accel_rgroup_rindices_rsp_t indices;
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

/**
 * Accelerator PF Device
 */
class Accel_PF : public Device {
public:
    Accel_PF(HalClient *hal_client, void *dev_spec,
             const struct lif_info *nicmgr_lif_info);

    void DevcmdHandler();
    void DevcmdPoll();
    enum DevcmdStatus CmdHandler(void *req, void *req_data,
                                 void *resp, void *resp_data);

    struct lif_info             info;
    dev_cmd_regs_t              *devcmd;

private:

    /* Members */
    string                      name;
    accel_devspec_t             *spec;
    // Hardware Info
    static struct queue_info    qinfo[NUM_QUEUE_TYPES];

    // HAL Info
    HalClient                   *hal;
    uint64_t                    lif_handle;
    // PCIe info
    pciehdev_t                  *pdev;
    pciehdevice_resources_t     pci_resources;

    // Oher states
    uint32_t                    seq_qid_init_high;  // highest seq qid initialized

    const struct lif_info       *nicmgr_lif_info;

    /* Methods */
    void _PostDevcmdDone(enum DevcmdStatus status);
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
    enum DevcmdStatus _DevcmdSeqQueueControl(void *req, void *req_data,
                                             void *resp, void *resp_data,
                                             bool enable);

    uint64_t GetQstateAddr(uint8_t qtype, uint32_t qid);

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
    uint32_t accel_ring_num_pendings_get(const accel_rgroup_ring_t& rgroup_ring);
    int accel_ring_max_pendings_get(uint32_t& max_pendings);

    friend ostream &operator<<(ostream&, const Accel_PF&);
};

#endif
