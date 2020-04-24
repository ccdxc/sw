/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __DEV_HPP__
#define __DEV_HPP__

#include <map>
#include <string>
#include <vector>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "nic/include/globals.hpp"
#include "pal_compat.hpp"

#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/lib/shmmgr/shmmgr.hpp"
#include "nic/sdk/platform/devapi/devapi.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/sdk/lib/catalog/catalog.hpp"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"

#include "device.hpp"
#include "pd_client.hpp"
#include "ev.h"

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
    UPG_EVENT_DISABLEQ,
    UPG_EVENT_ENABLEQ,
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
    // pipeline string indicates which p4 pipine is loaded in the datapath
    std::string pipeline;
    // memory profile is used to pick mem json file, this can be for different
    // scale and/or different combination of features
    std::string memory_profile;
    // device profile is used to pick the right device json file, this is for
    // different scale and/or combination of device types
    std::string device_profile;
    // catalog file captures h/w specific attributes
    sdk::lib::catalog *catalog;
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
    // constructor and destructor
    DeviceManager(devicemgr_cfg_t *cfg);
    virtual ~DeviceManager();

    // get current instance
    static DeviceManager *GetInstance() { return instance; }

    // init functions
    void Init(devicemgr_cfg_t *cfg);
    void UpgradeGracefulInit(devicemgr_cfg_t *cfg);
    void UpgradeHitlessInit(devicemgr_cfg_t *cfg);

    // Load state from profile cfg
    int LoadProfile(std::string path, bool init_pci);
    void CreateUplink(uint32_t id, uint32_t port, bool is_oob);

    // device handling functions
    Device *GetDevice(std::string name);
    Device *GetDeviceByLif(uint32_t lif_id);
    void AddDevice(enum DeviceType type, void *dev_spec);
    void RestoreDevice(enum DeviceType type, void *dev_state);
    void DeleteDevices();

    // event handlers
    void HalEventHandler(bool is_up);
    int HandleUpgradeEvent(UpgradeEvent event);
    void UpgradeGracefulHalEventHandler(bool is_up);
    void UpgradeHitlessHalEventHandler(bool is_up);
    void LinkEventHandler(port_status_t *evd);
    void XcvrEventHandler(port_status_t *evd);
    void DelphiMountEventHandler(bool mounted);
    void DeviceResetEventHandler();
    void SystemSpecEventHandler(bool micro_seg_en);

    // set/get devapi
    void SetHalClient(devapi *dev_api);
    devapi *DevApi(void) { return dev_api; }

    // set/get pd client
    PdClient *GetPdClient(void) { return pd; }

    // config path and helper functions
    std::string CfgPath(void) { return cfg_path; };
    int GenerateQstateInfoJson(std::string qstate_info_file);
    void GetConfigFiles(devicemgr_cfg_t *cfg, std::string &hbm_mem_json_file,
                        std::string &device_json_file);


    // upgrade helper functions
    void SetUpgradeMode(UpgradeMode mode) { upgrade_mode = mode; }
    UpgradeMode GetUpgradeMode(void) { return upgrade_mode; }
    UpgradeState GetUpgradeState();
    bool UpgradeCompatCheck();
    void SetFwStatus(uint8_t fw_status);

    // device list
    std::vector<struct EthDevInfo *> GetEthDevStateInfo();
    std::map<uint32_t, uplink_t *> GetUplinks() { return uplinks; };

    // swm functions
    void swm_update(bool enable, uint32_t port_num, uint32_t vlan, mac_t mac);

    // pcie handler
    DevPcieEvHandler pcie_evhandler;

    // thread funtions
    sdk::lib::thread *Thread(void) { return thread; }
    void SetThread(sdk::lib::thread *thr) { thread = thr; }
    struct ev_loop *ev_loop(void) { return EV_A; }
    ev_timer heartbeat_timer = {0};

    // device mode
    bool IsHostManaged(void) { return fwd_mode == sdk::lib::FORWARDING_MODE_CLASSIC; }

private:
    static DeviceManager *instance;

    boost::property_tree::ptree spec;
    std::map<std::string, Device *> devices;

    EV_P;
    devapi *dev_api;
    PdClient *pd;
    sdk::lib::thread *thread;
    std::map<uint32_t, uplink_t *> uplinks;

    bool init_done;
    UpgradeMode upgrade_mode;
    sdk::platform::platform_type_t platform;
    sdk::lib::dev_forwarding_mode_t fwd_mode;
    bool micro_seg_en;
    std::string device_json_file;
    std::string hbm_mem_json_file;
    std::string cfg_path;

    std::vector<struct EthDevInfo *> eth_dev_info_list;

    UpgradeState upg_state;

    // init helper functions
    void PlatformInit(devicemgr_cfg_t *cfg);
    void PciemgrInit(devicemgr_cfg_t *cfg);
    void HalLifIDReserve(void);
    void LifsReset(void);

    // hal event handler helper functions
    void DevApiClientInit(void);
    void OOBCreate(void);
    void OOBBringup(bool status);
    void UplinkInit(void);
    void SwmInit(void);
    void DeviceCreate(bool status);
    void UpgradeDeviceCreate(bool status);

    // upgrade helper functions
    bool IsDataPathQuiesced(void);
    bool CheckAllDevsDisabled(void);
    int SendFWDownEvent(void);

    // heartbeat timer
    timespec_t hb_last;
    bool hb_timer_init_done = false;
    void HeartbeatStart(void);
    void HeartbeatStop(void);
    static void HeartbeatEventHandler(EV_P_ ev_timer *w, int events);
};

#endif /* __DEV_HPP__ */
