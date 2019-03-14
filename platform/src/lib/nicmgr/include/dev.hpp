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

#include "platform/src/lib/devapi_iris/devapi_iris.hpp"

#include "platform/capri/capri_tbl_rw.hpp"

#include "pal_compat.hpp"

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pciehdevices/include/pciehdevices.h"
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

const char *oprom_type_to_str(OpromType_s);

typedef struct uplink_s {
    uint32_t id;
    uint32_t port;
    bool is_oob;
} uplink_t;

class PdClient;
class AdminQ;

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
