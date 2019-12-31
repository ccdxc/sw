
#ifndef __VIRTIO_DEV_HPP__
#define __VIRTIO_DEV_HPP__

#include <ev++.h>

#include "nic/sdk/storage/storage_seq_common.h"
#include "nic/sdk/linkmgr/linkmgr_types.hpp"

#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/devapi/devapi.hpp"

#include "device.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"
#include "virtio_if.h"

/**
 * VirtIO Device Spec
 */
typedef struct virtio_devspec {
    bool  enable;
    std::string name;

    // LIF
    sdk::platform::lif_type_t lif_type;
    int lif_count;
    int intr_count;
    int txrx_count;

    // PCIe
    int pcie_port;

    // Network
    int uplink_port_num;
} virtio_devspec_t;

/* forward declearation */
class VirtIOLif;

typedef int virtio_status_code_t;

/**
 * VirtIO Device
 */
class VirtIODev : public Device {
public:
    VirtIODev(devapi *dev_api,
             void *dev_spec,
             PdClient *pd_client,
             EV_P);
    ~VirtIODev();

    void DevcmdHandler();
    void HalEventHandler(bool status);
    void SetHalClient(devapi *dapi);
    void DelphiMountEventHandler(bool mounted);

    std::string GetName() { return spec->name; }
    const std::string& DevNameGet(void) { return spec->name; }
    const virtio_devspec_t *DevSpecGet(void) { return spec; }
    PdClient *PdClientGet(void) { return pd; }
    devapi *DevApiGet(void) { return dev_api; }
    uint64_t DevcmdPageGet(void) { return devcmd_mem_addr; }
    uint64_t DevcmddbPageGet(void) { return devcmddb_mem_addr; }

    static struct virtio_devspec *ParseConfig(boost::property_tree::ptree::value_type node);

private:
    // Device Spec
    const virtio_devspec_t       *spec;

    // PD Info
    PdClient                    *pd;
    // HAL Info
    devapi                      *dev_api;
    // Resources
    VirtIOLif                     *lif;

    uint32_t                    lif_base;
    uint32_t                    intr_base;
    // Devcmd
    uint64_t                    devcmd_mem_addr;
    uint64_t                    devcmddb_mem_addr;
    // CMB
    uint64_t                    cmb_mem_addr;
    uint32_t                    cmb_mem_size;

    bool _CreateHostDevice(void);
    void _DestroyHostDevice(void);

    /* Device Commands */
    static void _DevcmdPoll(void *obj);
    virtio_dev_cmd_regs_t        devcmd;
    virtio_dev_cmd_regs_t        devcmd_old;

    // Tasks
    EV_P;
    evutil_timer                devcmd_timer;

    VirtIOLif *_LifFind(uint64_t lif_id);

    void VirtIORegsInit();
};

#endif
