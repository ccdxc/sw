
#ifndef __ACCEL_DEV_HPP__
#define __ACCEL_DEV_HPP__

#include <map>
#include <ev++.h>

#include "nic/include/accel_ring.h"
#include "nic/sdk/storage/storage_seq_common.h"
#include "nic/sdk/linkmgr/linkmgr_types.hpp"

// #include "platform/src/lib/hal_api/include/hal_types.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"

#include "dev.hpp"
// #include "hal_client.hpp"

#define ACCEL_DEV_PAGE_SIZE             4096
#define ACCEL_DEV_PAGE_MASK             (ACCEL_DEV_PAGE_SIZE - 1)

#define ACCEL_DEV_ADDR_ALIGN(addr, sz)  \
    (((addr) + ((uint64_t)(sz) - 1)) & ~((uint64_t)(sz) - 1))

#define ACCEL_DEV_PAGE_ALIGN(addr)      \
    ACCEL_DEV_ADDR_ALIGN(addr, ACCEL_DEV_PAGE_SIZE)

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
    uint32_t rx_limit_gbps;
    uint32_t rx_burst_gb;
    uint32_t tx_limit_gbps;
    uint32_t tx_burst_gb;
    uint32_t pub_intv_frac; // publishing interval in fraction of second
    // PCIe
    uint8_t  pcie_port;
    std::string qos_group;
} accel_devspec_t;

class AccelLif;

typedef int accel_status_code_t;
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
    lif_reset_cmd_t             lif_reset;
    adminq_init_cmd_t           adminq_init;
    seq_queue_init_cmd_t        seq_q_init;
    seq_queue_init_complete_cmd_t seq_q_init_complete;
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
    lif_reset_cpl_t             lif_reset;
    adminq_init_cpl_t           adminq_init;
    seq_queue_init_cpl_t        seq_q_init;
    seq_queue_init_complete_cpl_t seq_q_init_complete;
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

static_assert(sizeof(dev_cmd_regs_t) == ACCEL_DEV_PAGE_SIZE, "");
static_assert((offsetof(dev_cmd_regs_t, cmd) % 4) == 0, "");
static_assert(sizeof(union dev_cmd) == 64, "");
static_assert((offsetof(dev_cmd_regs_t, cpl) % 4) == 0, "");
static_assert(sizeof(union dev_cmd_cpl) == 16, "");
static_assert((offsetof(dev_cmd_regs_t, data) % 4) == 0, "");

#endif /* ACCEL_DEV_CMD_ENUMERATE */

#include "gen/proto/nicmgr/accel_metrics.pb.h"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"

typedef struct seq_queue_init_cmd seq_queue_init_cmd_t;
typedef struct seq_queue_control_cmd seq_queue_control_cmd_t;

const char *accel_dev_opcode_str(uint32_t opcode);

/**
 * Accelerator Device
 */
class AccelDev : public Device {
public:
    AccelDev(devapi *dev_api,
             void *dev_spec,
             PdClient *pd_client);

    std::string GetName() { return spec->name; }

    void DevcmdHandler();
    void HalEventHandler(bool status);
    virtual void DelphiMountEventHandler(bool mounted);
    void SetHalClient(devapi *dapi);

    accel_status_code_t
    CmdHandler(void *req, void *req_data, void *resp, void *resp_data);

    void IntrClear(void);

    const std::string& DevNameGet(void) { return spec->name; }
    const accel_devspec_t *DevSpecGet(void) { return spec; }
    PdClient *PdClientGet(void) { return pd; }
    devapi *DevApiGet(void) { return dev_api; }
    uint64_t DevcmdPageGet(void) { return devcmd_mem_addr; }
    uint64_t DevcmddbPageGet(void) { return devcmddb_mem_addr; }

    dev_cmd_regs_t              *devcmd;

private:
    // Device Spec
    const accel_devspec_t       *spec;

    // Delphi
    delphi::objects::AccelPfInfoPtr delphi_pf;

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    // PCIe info
    pciehdev_t                  *pdev;
    // Resources
    std::map<uint64_t, AccelLif *> lif_map;
    uint32_t                    lif_base;
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
    bool                        delphi_mounted;

    bool _CreateHostDevice();

    /* Device Commands */
    static void _DevcmdPoll(void *obj);

    accel_status_code_t
    _AdminCmdHandler(uint64_t lif_id, void *req, void *req_data,
                     void *resp, void *resp_data);
    accel_status_code_t
    _DevcmdReset(void *req, void *req_data, void *resp, void *resp_data);

    accel_status_code_t
    _DevcmdIdentify(void *req, void *req_data, void *resp, void *resp_data);

    accel_status_code_t
    _DevcmdAdminQueueInit(void *req, void *req_data, void *resp, void *resp_data);

    accel_status_code_t
    _DevcmdCryptoKeyUpdate(void *req, void *req_data, void *resp, void *resp_data);

    // Tasks
    evutil_timer                devcmd_timer;

    void _DelphiInit(void);

    AccelLif *_LifFind(uint64_t lif_id);
};

#endif
