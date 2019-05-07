
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

#include "device.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"
#include "accel_if.h"

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
    uint64_t DevcmddbPageGet(void) { return devcmddb_mem_addr; }

    accel_dev_cmd_regs_t        *devcmd;

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

    bool _CreateHostDevice(void);
    void _DestroyHostDevice(void);

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
    EV_P;
    evutil_timer                devcmd_timer;

    void _MetricsInit(void);
    void _MetricsFini(void);

    AccelLif *_LifFind(uint64_t lif_id);
};

#endif
