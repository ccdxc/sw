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

#include "pal_compat.hpp"

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"
#include "platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/sdk/platform/devapi/devapi.hpp"
#include "devapi_types.hpp"
#include "pd_client.hpp"

#include "device.hpp"
#include "eth_dev.hpp"

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

enum {
    NICMGR_MAC_DEV_CAP              = 0x1000000,
    NICMGR_MAC_DEV_MASK             = 0xffffff,
    NICMGR_DEF_MAC_COUNT            = 24,
    NICMGR_MIN_MAC_COUNT            = 8,
};


enum UpgradeState {
    UNKNOWN_STATE,
    DEVICES_ACTIVE_STATE,
    DEVICES_QUIESCED_STATE,
    DEVICES_RESET_STATE
};

enum UpgradeEvent {
    UPG_EVENT_QUIESCE,
    UPG_EVENT_DEVICE_RESET
};

enum UpgradeMode {
    FW_MODE_NORMAL_BOOT,
    FW_MODE_UPGRADE,
    FW_MODE_ROLLBACK
};

const char *oprom_type_to_str(OpromType_s);

typedef struct uplink_s {
    uint32_t id;
    uint32_t port;
    bool is_oob;
} uplink_t;

class PdClient;
class AdminQ;

class DevPcieEvHandler : public pciemgr::evhandler {
public:
    void memrd(const int port,
               const uint32_t lif, const pciehdev_memrw_notify_t *n);
    void memwr(const int port,
               const uint32_t lif, const pciehdev_memrw_notify_t *n);
    void hostup(const int port);
    void hostdn(const int port);
    void sriov_numvfs(const int port,
                      const uint32_t lif, const uint16_t numvfs);
    void reset(const int port,
               uint32_t rsttype, const uint32_t lifb, const uint32_t lifc);
};

/**
 * Device Manager
 */
class DeviceManager {
public:
    DeviceManager(std::string config_file, fwd_mode_t fwd_mode,
                  sdk::platform::platform_type_t platform, EV_P = NULL);

    int LoadConfig(std::string path);
    void CreateUplinks(uint32_t id, uint32_t port, bool is_oob);
    static DeviceManager *GetInstance() { return instance; }

    Device *GetDevice(std::string name);
    Eth *GetEthDeviceByLif(uint32_t lif_id);

    void DeleteDevice(std::string name);

    void HalEventHandler(bool is_up);
    void LinkEventHandler(port_status_t *evd);
    void XcvrEventHandler(port_status_t *evd);
    void DelphiMountEventHandler(bool mounted);
    void DeviceResetEventHandler();

    void CreateUplinkVRFs();
    void SetHalClient(devapi *dev_api);

    int GenerateQstateInfoJson(std::string qstate_info_file);
    static string ParseDeviceConf(string input_arg, fwd_mode_t *fw_mode);
    PdClient *GetPdClient(void) { return pd; }
    void SetUpgradeMode(UpgradeMode upg_mode) { upgrade_mode = upg_mode; };
    UpgradeMode GetUpgradeMode() { return upgrade_mode; };
    UpgradeState GetUpgradeState();
    int HandleUpgradeEvent(UpgradeEvent event);
    std::map<uint32_t, uplink_t*> GetUplinks() { return uplinks; };
    void SetFwStatus(uint8_t fw_status);
    std::vector <struct EthDevInfo *> GetEthDevStateInfo();
    void RestoreDevicesState(std::vector <struct EthDevInfo *> eth_dev_info_list);
    bool UpgradeCompatCheck();
    devapi *DevApi(void) { return dev_api; }
    void swm_update(bool enable,
                    uint32_t port_num, uint32_t vlan, mac_t mac);
    DevPcieEvHandler pcie_evhandler;
private:
    static DeviceManager *instance;

    boost::property_tree::ptree spec;
    std::map<std::string, Device*> devices;

    EV_P;
    devapi *dev_api;
    PdClient *pd;
    bool skip_hwinit;
    std::map<uint32_t, uplink_t*> uplinks;

    bool init_done;
    UpgradeMode upgrade_mode;
    std::string config_file;
    fwd_mode_t fwd_mode;
    UpgradeState upg_state;
    std::vector <struct EthDevInfo *> eth_dev_info_list;

    Device *AddDevice(enum DeviceType type, void *dev_spec);
    bool IsDataPathQuiesced();
    bool CheckAllDevsDisabled();
    int SendFWDownEvent();
    static void HeartbeatEventHandler(void* obj);
};

#endif /* __DEV_HPP__ */
