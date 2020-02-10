/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __DEV_HPP__
#define __DEV_HPP__

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <map>
#include <string>
#include <vector>

#include "nic/include/globals.hpp"

#ifndef __FTL_DEV_IMPL_HPP__
#include "pal_compat.hpp"
#endif

#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/lib/shmmgr/shmmgr.hpp"
#include "nic/sdk/platform/devapi/devapi.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"

#include "device.hpp"
#include "pd_client.hpp"

using std::string;

enum {
    NICMGR_THREAD_ID_MIN = 0,
    NICMGR_THREAD_ID_DELPHI_CLIENT = 1,
    NICMGR_THREAD_ID_MAX = 2,
};

enum {
    NICMGR_TIMER_ID_NONE = 0,
    NICMGR_TIMER_ID_MIN = 1,
    NICMGR_TIMER_ID_HEARTBEAT = NICMGR_TIMER_ID_MIN,
    NICMGR_TIMER_ID_MAX = 2,
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

typedef struct uplink_s {
    uint32_t id;
    uint32_t port;
    bool is_oob;
} uplink_t;

typedef struct devicemgr_cfg_s {
    sdk::platform::platform_type_t platform_type;
    std::string cfg_path;
    std::string device_conf_file;
    sdk::lib::dev_forwarding_mode_t fwd_mode;
    bool micro_seg_en;
    sdk::lib::shmmgr *shm_mgr;
    EV_P;
} devicemgr_cfg_t;


class PdClient;
class AdminQ;

class DevPcieEvHandler : public pciemgr::evhandler
{
  public:
    void memrd(const int port, const uint32_t lif, const pciehdev_memrw_notify_t *n);
    void memwr(const int port, const uint32_t lif, const pciehdev_memrw_notify_t *n);
    void hostup(const int port);
    void hostdn(const int port);
    void sriov_numvfs(const int port, const uint32_t lif, const uint16_t numvfs);
    void reset(const int port, uint32_t rsttype, const uint32_t lifb, const uint32_t lifc);
};

/**
 * Device Manager
 */
class DeviceManager
{
public:
    DeviceManager(devicemgr_cfg_t *cfg);

    int LoadProfile(std::string path, bool init_pci);
    void LoadState(std::vector<struct EthDevInfo *> eth_dev_info_list);

    void CreateUplink(uint32_t id, uint32_t port, bool is_oob);
    static DeviceManager *GetInstance() { return instance; }

    Device *GetDevice(std::string name);
    Device *GetDeviceByLif(uint32_t lif_id);

    void AddDevice(enum DeviceType type, void *dev_spec);
    void RestoreDevice(enum DeviceType type, void *dev_state);
    void DeleteDevice(std::string name);

    void HalEventHandler(bool is_up);
    void LinkEventHandler(port_status_t *evd);
    void XcvrEventHandler(port_status_t *evd);
    void DelphiMountEventHandler(bool mounted);
    void DeviceResetEventHandler();
    void SystemSpecEventHandler(bool micro_seg_en);

    void SetHalClient(devapi *dev_api);

    int GenerateQstateInfoJson(std::string qstate_info_file);
    void GetConfigFiles(string device_conf_file, string &hbm_mem_json_file, string &device_json_file);
    PdClient *GetPdClient(void) { return pd; }
    devapi *DevApi(void) { return dev_api; }

    UpgradeState GetUpgradeState();
    int HandleUpgradeEvent(UpgradeEvent event);
    bool UpgradeCompatCheck();
    std::vector<struct EthDevInfo *> GetEthDevStateInfo();
    void SetFwStatus(uint8_t fw_status);

    std::map<uint32_t, uplink_t *> GetUplinks() { return uplinks; };

    void swm_update(bool enable, uint32_t port_num, uint32_t vlan, mac_t mac);

    DevPcieEvHandler pcie_evhandler;
    evutil_timer heartbeat_timer;
    sdk::lib::thread *Thread(void) { return thread; }
    void SetThread(sdk::lib::thread *thr) { thread = thr; }
    void SetUpgradeMode(UpgradeMode mode) { upgrade_mode = mode; }
    UpgradeMode GetUpgradeMode(void) { return upgrade_mode; }
    struct ev_loop *ev_loop(void) { return EV_A; }
    string CfgPath(void) { return cfg_path; };

private:
    static DeviceManager *instance;

    boost::property_tree::ptree spec;
    std::map<std::string, Device *> devices;

    EV_P;
    devapi *dev_api;
    PdClient *pd;
    sdk::lib::thread *thread;
    bool skip_hwinit;
    std::map<uint32_t, uplink_t *> uplinks;

    bool init_done;
    UpgradeMode upgrade_mode;
    sdk::platform::platform_type_t platform;
    sdk::lib::dev_forwarding_mode_t fwd_mode;
    bool micro_seg_en;
    string device_json_file;
    string cfg_path;

    std::vector<struct EthDevInfo *> eth_dev_info_list;

    UpgradeState upg_state;

    bool IsDataPathQuiesced();
    bool CheckAllDevsDisabled();
    int SendFWDownEvent();

    timespec_t hb_last;
    static void HeartbeatEventHandler(void* obj);
};

#endif /* __DEV_HPP__ */
