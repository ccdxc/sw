
#ifndef __ACCEL_DEV_HPP__
#define __ACCEL_DEV_HPP__

#include <map>
#include <ev++.h>

#include "nic/include/accel_ring.h"
#include "nic/sdk/storage/storage_seq_common.h"
#include "nic/sdk/linkmgr/linkmgr_types.hpp"

#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/devapi/devapi.hpp"
#include "nic/sdk/asic/asic.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

#include "device.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"
#include "nic/include/accel_dev_if.h"

#ifdef ELBA
#include "elb_top_csr_defines.h"
#include "elb_wa_csr_define.h"
#include "elb_ms_csr_define.h"
#include "nic/hal/pd/elba/elba_cpdc.hpp"
#include "nic/hal/pd/elba/elba_barco_crypto.hpp"
#include "nic/hal/pd/elba/elba_barco_rings.hpp"
#include "nic/hal/pd/elba/elba_hbm.hpp"

// Temporarily using CHIPID since VER is not yet exported from model
#define HW_CHIP_VER_CSR                 (ELB_ADDR_BASE_MS_MS_OFFSET + \
                                         ELB_MS_CSR_STA_TAP_CHIPID_BYTE_ADDRESS)
// Temporarily set ASIC type/rev to 0 since VER is not yet exported from model
#define HW_ASIC_TYPE_GET(ver)           ((ver) & 0)
#define HW_ASIC_REV_GET(ver)            ((ver) & 0)
#define CPDC_INT_ERR_LOG_MASK           ELBA_CPDC_INT_ERR_LOG_MASK
#define CPDC_INT_ECC_LOG_MASK           ELBA_CPDC_INT_ECC_LOG_MASK
#define CPDC_AXI_ERR_W0_LOG_MASK        ELBA_CPDC_AXI_ERR_W0_LOG_MASK
#define CPDC_AXI_ERR_W1_LOG_MASK        ELBA_CPDC_AXI_ERR_W1_LOG_MASK
#define CRYPTO_SYM_ERR_UNRECOV_MASK     ELBA_BARCO_SYM_ERR_UNRECOV_MASK
#else
#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"
#include "cap_ms_c_hdr.h"
#include "capri_cpdc.hpp"
#include "nic/sdk/platform/capri/capri_barco_crypto.hpp"
#include "nic/sdk/platform/capri/capri_barco_rings.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"

#define HW_CHIP_VER_CSR                 (CAP_ADDR_BASE_MS_MS_OFFSET + \
                                         CAP_MS_CSR_STA_VER_BYTE_ADDRESS)
#define HW_ASIC_TYPE_GET(ver)           CAP_MS_CSR_STA_VER_CHIP_TYPE_GET(ver)
#define HW_ASIC_REV_GET(ver)            CAP_MS_CSR_STA_VER_CHIP_VERSION_GET(ver)
#define CPDC_INT_ERR_LOG_MASK           CAPRI_CPDC_INT_ERR_LOG_MASK
#define CPDC_INT_ECC_LOG_MASK           CAPRI_CPDC_INT_ECC_LOG_MASK
#define CPDC_AXI_ERR_W0_LOG_MASK        CAPRI_CPDC_AXI_ERR_W0_LOG_MASK
#define CPDC_AXI_ERR_W1_LOG_MASK        CAPRI_CPDC_AXI_ERR_W1_LOG_MASK
#define CRYPTO_SYM_ERR_UNRECOV_MASK     CAPRI_BARCO_SYM_ERR_UNRECOV_MASK
#endif // ELBA

                        ;
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

/* forward declearation */
class AccelLif;

typedef int accel_status_code_t;

#include "gen/proto/nicmgr/accel_metrics.pb.h"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"

typedef struct seq_queue_init_cmd seq_queue_init_cmd_t;
typedef struct seq_queue_control_cmd seq_queue_control_cmd_t;
typedef struct dev_cmd_db_s {
    uint32_t    v;
} dev_cmd_db_t;

const char *accel_dev_opcode_str(uint32_t opcode);

/**
 * Accelerator Device
 */
class AccelDev : public Device {
public:
    AccelDev(devapi *dev_api,
             void *dev_spec,
             PdClient *pd_client,
             EV_P);
    ~AccelDev();

    void DevcmdHandler();
    void HalEventHandler(bool status);
    virtual void DelphiMountEventHandler(bool mounted);
    void SetHalClient(devapi *dapi);

    accel_status_code_t
    CmdHandler(void *req, void *req_data, void *resp, void *resp_data);

    void IntrClear(void);

    std::string GetName() { return spec->name; }
    const std::string& DevNameGet(void) { return spec->name; }
    const accel_devspec_t *DevSpecGet(void) { return spec; }
    PdClient *PdClientGet(void) { return pd; }
    devapi *DevApiGet(void) { return dev_api; }
    uint64_t DevcmdPageGet(void) { return devcmd_mem_addr; }
    uint32_t CryptoKeyIdxBaseGet(void) { return crypto_key_idx_base; }
    uint32_t NumCryptoKeysMaxGet(void) { return num_crypto_keys_max; }
    uint32_t NumLifsGet(void) { return lif_vec.size(); }

    static struct accel_devspec *ParseConfig(boost::property_tree::ptree::value_type node);

private:
    // Device Spec
    const accel_devspec_t       *spec;

    // Delphi
    delphi::objects::AccelPfInfoPtr delphi_pf;

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    // Resources
    std::vector<AccelLif *>     lif_vec;
    uint32_t                    lif_base;
    uint32_t                    intr_base;
    // Devcmd
    uint64_t                    regs_mem_addr;
    uint64_t                    devcmd_mem_addr;
    dev_regs_t                  *regs;
    dev_cmd_regs_t              *devcmd;
    // CMB
    uint64_t                    cmb_mem_addr;
    uint32_t                    cmb_mem_size;
    // Other state
    uint32_t                    crypto_key_idx_base;
    uint32_t                    num_crypto_keys_max;
    bool                        delphi_mounted;

    void _DevInfoRegsInit(void);
    bool _CreateHostDevice(void);
    void _DestroyHostDevice(void);

    /* Device Commands */
    static void _DevcmdPoll(void *obj);

    accel_status_code_t
    _AdminCmdHandler(uint32_t lif_index, void *req, void *req_data,
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
    EV_P;
    evutil_timer                devcmd_timer;

    void _MetricsInit(void);
    void _MetricsFini(void);

    AccelLif *_LifFind(uint32_t lif_index);
};

#endif
