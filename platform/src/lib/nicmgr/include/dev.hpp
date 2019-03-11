/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __DEV_HPP__
#define __DEV_HPP__

#include <string>
#include <map>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "nic/include/globals.hpp"

#if 0
#include "platform/src/lib/hal_api/include/hal_common_client.hpp"
#include "platform/src/lib/hal_api/include/hal_grpc_client.hpp"
#endif
#include "platform/src/lib/devapi_iris/devapi_iris.hpp"

#include "platform/capri/capri_tbl_rw.hpp"

#include "pal_compat.hpp"

#include "nic/sdk/platform/evutils/include/evutils.h"
#ifdef __aarch64__
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#endif
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"
#include "nic/sdk/platform/devapi/devapi.hpp"
#include "devapi_types.hpp"
#include "pd_client.hpp"


#define VERSION_FILE        "/nic/etc/VERSION.json"

using std::string;

enum {
    NICMGR_THREAD_ID_MIN           = 0,
    NICMGR_THREAD_ID_DELPHI_CLIENT = 1,
    NICMGR_THREAD_ID_MAX           = 2,
};

enum {
    NICMGR_TIMER_ID_NONE                     = 0,
    NICMGR_TIMER_ID_MIN                      = 1,
    NICMGR_TIMER_ID_HEARTBEAT                = NICMGR_TIMER_ID_MIN,
    NICMGR_TIMER_ID_MAX                      = 2,
};

/**
 * Device Types
 */
enum DeviceType
{
    INVALID,
    MNIC,
    DEBUG,
    ETH,
    ACCEL,
    NVME,
    VIRTIO,
};

/**
 * OPROM
 */
typedef enum OpromType_s {
    OPROM_UNKNOWN,
    OPROM_LEGACY,
    OPROM_UEFI,
    OPROM_UNIFIED
} OpromType;

const char *oprom_type_to_str(OpromType_s);

/**
 * ETH Device type
 */
typedef enum EthDevType_s {
    ETH_UNKNOWN,
    ETH_HOST,
    ETH_HOST_MGMT,
    ETH_MNIC_OOB_MGMT,
    ETH_MNIC_INTERNAL_MGMT,
    ETH_MNIC_INBAND_MGMT,
} EthDevType;

typedef struct uplink_s {
    uint32_t id;
    uint32_t port;
    bool is_oob;
} uplink_t;

const char *eth_dev_type_to_str(EthDevType type);

/**
 * Eth Device Spec
 */
struct eth_devspec {
    // Delphi
    uint64_t dev_uuid;
    // Device
    EthDevType eth_type;
    std::string name;
    OpromType oprom;
    uint8_t pcie_port;
    bool host_dev;
    // Network
    uint32_t uplink_port_num;
    std::string qos_group;
    // RES
    uint32_t lif_count;
    uint32_t rxq_count;
    uint32_t txq_count;
    uint32_t eq_count;
    uint32_t adminq_count;
    uint32_t intr_count;
    uint64_t mac_addr;
    // RDMA
    bool enable_rdma;
    uint32_t pte_count;
    uint32_t key_count;
    uint32_t ah_count;
    uint32_t rdma_sq_count;
    uint32_t rdma_rq_count;
    uint32_t rdma_cq_count;
    uint32_t rdma_eq_count;
    uint32_t rdma_adminq_count;
    uint32_t rdma_pid_count;
    uint32_t barmap_size;    // in 8MB units
};

typedef struct dev_cmd_db_s {
    uint32_t    v;
} dev_cmd_db_t;

class PdClient;
class AdminQ;

/**
 * Base Class for devices
 */
class Device {
public:
    enum DeviceType GetType() { return type; }
    void SetType(enum DeviceType type) { this->type = type;}
    virtual void DelphiMountEventHandler(bool mounted) {}
private:
    enum DeviceType type;
};

/**
 * Device Manager
 */
class DeviceManager {
public:
    DeviceManager(std::string config_file, fwd_mode_t fwd_mode,
                  platform_t platform);

    int LoadConfig(std::string path);
    static DeviceManager *GetInstance() { return instance; }

    Device *GetDevice(std::string name);

    void HalEventHandler(bool is_up);
    void LinkEventHandler(port_status_t *evd);
    void DelphiMountEventHandler(bool mounted);

    void CreateUplinkVRFs();
    void SetHalClient(devapi *dev_api);

    int GenerateQstateInfoJson(std::string qstate_info_file);
    static string ParseDeviceConf(string input_arg);
    PdClient *GetPdClient(void) { return pd; }

private:
    static DeviceManager *instance;

    boost::property_tree::ptree spec;
    std::map<std::string, Device*> devices;

    devapi *dev_api;
    PdClient *pd;
    std::map<uint32_t, uplink_t*> uplinks;

    bool init_done;
    std::string config_file;
    fwd_mode_t fwd_mode;

    Device *AddDevice(enum DeviceType type, void *dev_spec);
};

#endif /* __DEV_HPP__ */
