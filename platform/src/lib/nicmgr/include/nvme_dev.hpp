
#ifndef __NVME_DEV_HPP__
#define __NVME_DEV_HPP__

#include <ev++.h>

#include "nic/sdk/storage/storage_seq_common.h"
#include "nic/sdk/linkmgr/linkmgr_types.hpp"

#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/devapi/devapi.hpp"

#include "device.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"
#include "nvme_if.h"

/**
 * Nvme Device Spec
 */
typedef struct nvme_devspec {
    uint8_t  enable;
    std::string name;
    // RES
    uint32_t lif_count;
    uint32_t adminq_count;
    uint32_t sq_count;
    uint32_t cq_count;
    uint32_t intr_count;
    // PCIe
    uint8_t  pcie_port;
    uint32_t pcie_total_vfs;
} nvme_devspec_t;

/* forward declearation */
class NvmeLif;

typedef int nvme_status_code_t;

/**
 * NVME Device
 */
class NvmeDev : public Device {
public:
    NvmeDev(devapi *dev_api,
             void *dev_spec,
             PdClient *pd_client,
             EV_P);
    ~NvmeDev();

    void DevcmdHandler();
    void HalEventHandler(bool status);
    void SetHalClient(devapi *dapi);
    void DelphiMountEventHandler(bool mounted);

    void IntrReset(void);

    std::string GetName() { return spec->name; }
    const std::string& DevNameGet(void) { return spec->name; }
    const nvme_devspec_t *DevSpecGet(void) { return spec; }
    PdClient *PdClientGet(void) { return pd; }
    devapi *DevApiGet(void) { return dev_api; }
    uint64_t DevcmdPageGet(void) { return devcmd_mem_addr; }
    uint64_t DevcmddbPageGet(void) { return devcmddb_mem_addr; }

    nvme_dev_cmd_regs_t        *devcmd;

    static struct nvme_devspec *ParseConfig(boost::property_tree::ptree::value_type node);

private:
    // Device Spec
    const nvme_devspec_t       *spec;

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    // Resources
    NvmeLif                     *lif;

    uint32_t                    lif_base;
    uint32_t                    intr_base;
    // Devcmd
    uint64_t                    devcmd_mem_addr;
    uint64_t                    devcmddb_mem_addr;
    // CMB
    uint64_t                    cmb_mem_addr;
    uint32_t                    cmb_mem_size;

    bool                        cc_en;

    bool _CreateHostDevice(void);
    void _DestroyHostDevice(void);

    /* Device Commands */
    static void _DevcmdPoll(void *obj);

    // Tasks
    EV_P;
    evutil_timer                devcmd_timer;

    NvmeLif *_LifFind(uint64_t lif_id);

    void NvmeRegsInit();
};

#endif
