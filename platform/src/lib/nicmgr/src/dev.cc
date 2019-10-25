/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#if !defined(APOLLO) && !defined(ARTEMIS) && !defined(APULU)
#include "gen/proto/device.pb.h"
#include "accel_dev.hpp"
#endif

#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/sdk/platform/misc/include/maclib.h"
#include "nic/sdk/platform/utils/lif_mgr/lif_mgr.hpp"
#include "nic/sdk/lib/device/device.hpp"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"

#include "logger.hpp"

#include "pd_client.hpp"
#include "adminq.hpp"
#include "dev.hpp"
#include "eth_dev.hpp"
#include "nicmgr_init.hpp"
#include "nvme_dev.hpp"
#include "virtio_dev.hpp"

using namespace std;

namespace pt = boost::property_tree;

evutil_timer heartbeat_timer;
DeviceManager *DeviceManager::instance;

#define CASE(type) case type: return #type

const char *
eth_dev_type_to_str(EthDevType type)
{
    switch(type) {
        CASE(ETH_UNKNOWN);
        CASE(ETH_HOST);
        CASE(ETH_HOST_MGMT);
        CASE(ETH_MNIC_OOB_MGMT);
        CASE(ETH_MNIC_INTERNAL_MGMT);
        CASE(ETH_MNIC_INBAND_MGMT);
        CASE(ETH_MNIC_CPU);
        default: return "Unknown";
    }
}

#define CASE(type) case type: return #type

const char *
oprom_type_to_str(OpromType type)
{
    switch(type) {
        CASE(OPROM_UNKNOWN);
        CASE(OPROM_LEGACY);
        CASE(OPROM_UEFI);
        CASE(OPROM_UNIFIED);
        default: return "unknown";
    }
}

DeviceManager::DeviceManager(std::string config_file, fwd_mode_t fwd_mode,
                            sdk::platform::platform_type_t platform, EV_P)
{
    NIC_HEADER_TRACE("Initializing DeviceManager");
    init_done = false;
    instance = this;
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) ==
                sdk::lib::PAL_RET_OK);
#elif __aarch64__
#if !defined(APOLLO) && !defined(ARTEMIS) && !defined(APULU)
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) ==
                sdk::lib::PAL_RET_OK);
#endif
#endif
    if (loop == NULL) {
        this->loop = EV_DEFAULT;
    } else {
        this->loop = loop;
    }
    this->fwd_mode = fwd_mode;
    this->config_file = config_file;
    pd = PdClient::factory(platform, fwd_mode);
    assert(pd);
    this->skip_hwinit = pd->is_dev_hwinit_done(NULL);

    printf("Nicmgr forwarding mode: %s\n", FWD_MODE_TYPES_str(fwd_mode));
    NIC_LOG_DEBUG("Event loop: {:#x}", (uint64_t)this->loop);

    // Reserve all the LIF ids used by HAL
    NIC_LOG_DEBUG("Reserving HAL lifs {}-{}", HAL_LIF_ID_MIN, HAL_LIF_ID_MAX);
    // int ret = pd->lm_->LIFRangeAlloc(HAL_LIF_ID_MIN, HAL_LIF_ID_MAX);
    int ret = pd->lm_->reserve_id(HAL_LIF_ID_MIN,
                                  (HAL_LIF_ID_MAX - HAL_LIF_ID_MIN + 1));
    if (ret < 0) {
        throw runtime_error("Failed to reserve HAL LIFs");
    }
    if (!skip_hwinit) {
        ret = sdk::platform::utils::lif_mgr::lifs_reset(NICMGR_SVC_LIF,
                                                    NICMGR_LIF_MAX);
        if (ret != sdk::SDK_RET_OK) {
            throw runtime_error("Failed to reset LIFs");
        }
    }
    upg_state = UNKNOWN_STATE;
}

string
DeviceManager::ParseDeviceConf(string filename, fwd_mode_t *fw_mode)
{
#if !defined(APOLLO) && !defined(ARTEMIS) && !defined(APULU)
    sdk::lib::device *device = NULL;
    sdk::lib::dev_forwarding_mode_t fwd_mode;
    sdk::lib::dev_feature_profile_t feature_profile;

    cout << "Parsing Device conf file: " << filename << endl;
    device = sdk::lib::device::factory(filename);

    fwd_mode = device->get_forwarding_mode();
    feature_profile = device->get_feature_profile();

    printf("forwarding mode: %s, feature_profile: %d\n",
           FWD_MODE_TYPES_str(*fw_mode),
           feature_profile);

    if (fwd_mode == sdk::lib::FORWARDING_MODE_HOSTPIN ||
        fwd_mode == sdk::lib::FORWARDING_MODE_SWITCH) {
        *fw_mode = sdk::platform::FWD_MODE_SMART;
        return string("/platform/etc/nicmgrd/eth_smart.json");
    } else if (fwd_mode == sdk::lib::FORWARDING_MODE_CLASSIC) {
        *fw_mode = sdk::platform::FWD_MODE_CLASSIC;
        if (feature_profile == sdk::lib::FEATURE_PROFILE_CLASSIC_DEFAULT) {
            return string("/platform/etc/nicmgrd/device.json");
        } else if (feature_profile == sdk::lib::FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE) {
            return string("/platform/etc/nicmgrd/eth_scale.json");
        } else {
            return string("/platform/etc/nicmgrd/device.json");
        }
    } else {
        cout << "Unknown mode, returning classic default" << endl;
        *fw_mode = sdk::platform::FWD_MODE_CLASSIC;
        return string("/platform/etc/nicmgrd/device.json");
    }

#endif
    return string("");
}

void
DeviceManager::CreateUplinks(uint32_t id, uint32_t port, bool is_oob)
{
    uplink_t *up = NULL;
    NIC_LOG_DEBUG("Creating uplink: id: {} port: {}, oob: {}", id, port, is_oob);

    up = new uplink_t();
    up->id = id;
    up->port = port;
    up->is_oob = is_oob;
    uplinks[up->id] = up;

}

int
DeviceManager::LoadConfig(string path)
{
    struct eth_devspec *eth_spec;

    NIC_HEADER_TRACE("Loading Config");
    NIC_LOG_DEBUG("Json: {}", path);
    boost::property_tree::read_json(path, spec);

    // Determine the base mac address
    uint64_t sys_mac_base = 0;
    uint64_t host_mac_base = 0;
    uint64_t mnic_mac_base = 0;
    uint64_t fru_mac = 0;
    uint64_t cfg_mac = 0;
    uint32_t num_macs = 24;
    string mac_str;
    string num_macs_str;
#ifdef __aarch64__
    if (sdk::platform::readFruKey(MACADDRESS_KEY, mac_str) == 0) {
        mac_from_str(&fru_mac, mac_str.c_str());
    } else {
        NIC_LOG_ERR("Failed to read MAC address from FRU");
    }
    if (sdk::platform::readFruKey(NUMMACADDR_KEY, num_macs_str) == 0) {
        num_macs = std::stoi(num_macs_str);
    } else {
        NIC_LOG_ERR("Failed to read MAC address from FRU");
    }
#endif
    if (getenv("SYSUUID") != NULL) {
        mac_from_str(&cfg_mac, getenv("SYSUUID"));
    } else {
        NIC_LOG_DEBUG("MAC address environment variable is not set");
    }

    // Validate & set base mac address
    if (is_unicast_mac_addr(&fru_mac)) {
        NIC_LOG_INFO("FRU mac address {:#x}", fru_mac);
        sys_mac_base = fru_mac;
    } else if (is_unicast_mac_addr(&cfg_mac)) {
        NIC_LOG_INFO("Configured mac address {:#x}", cfg_mac);
        sys_mac_base = cfg_mac;
    } else {
        NIC_LOG_DEBUG("Invalid mac addresses: FRU {:#x} and config {:#x}",
            fru_mac, cfg_mac);
        mac_from_str(&sys_mac_base, "00:de:ad:be:ef:00");
    }

    /*
     * Host Ifs: <sys_mac_base .....
     * Mnic Ifs: ......mnic_mac_base>
     */
    host_mac_base = sys_mac_base;
    mnic_mac_base = host_mac_base + num_macs - 1;

    char sys_mac_str[32] = {0};
    NIC_LOG_INFO("Number of Macs: {}", num_macs);
    NIC_LOG_INFO("Base mac address {}", mac_to_str(&sys_mac_base, sys_mac_str, sizeof(sys_mac_str)));
    NIC_LOG_INFO("Host Base: {}", mac_to_str(&host_mac_base, sys_mac_str, sizeof(sys_mac_str)));
    NIC_LOG_INFO("Mnic Base: {}",mac_to_str(&mnic_mac_base, sys_mac_str, sizeof(sys_mac_str)));

    // Create Network
    if (spec.get_child_optional("network")) {
        // Create Uplinks
        if (spec.get_child_optional("network.uplink")) {
            for (const auto &node : spec.get_child("network.uplink")) {
                auto val = node.second;

                CreateUplinks(val.get<uint64_t>("id"), val.get<uint64_t>("port"), val.get<bool>("oob", false));
            }
        }
    }

    NIC_HEADER_TRACE("Loading Mnic devices");
    // Create MNICs
    if (spec.get_child_optional("mnic_dev")) {
        for (const auto &node : spec.get_child("mnic_dev")) {

            eth_spec = Eth::ParseConfig(node);
            eth_spec->mac_addr = mnic_mac_base--;

            AddDevice(ETH, (void *)eth_spec);
        }
    }

    NIC_HEADER_TRACE("Loading Eth devices");
    // Create Ethernet devices
    if (spec.get_child_optional("eth_dev")) {
        for (const auto &node : spec.get_child("eth_dev")) {

            eth_spec = Eth::ParseConfig(node);
            if (host_mac_base == mnic_mac_base) {
                NIC_LOG_ERR("Number of macs {} not enough for Host ifs and Mnic ifs.",
                        num_macs);
            }
            eth_spec->mac_addr = host_mac_base++;
            eth_spec->host_dev = true;

            AddDevice(ETH, (void *)eth_spec);
        }
    }

#ifdef IRIS
    NIC_HEADER_TRACE("Loading Accel devices");
    // Create Accelerator devices
    if (spec.get_child_optional("accel_dev")) {
        struct accel_devspec *accel_spec;

        for (const auto &node : spec.get_child("accel_dev")) {

            accel_spec = AccelDev::ParseConfig(node);

            AddDevice(ACCEL, (void *)accel_spec);
        }
    }

    NIC_HEADER_TRACE("Loading NVME devices");
    // Create NVME devices
    if (spec.get_child_optional("nvme_dev")) {
        struct nvme_devspec *nvme_spec;

        for (const auto &node : spec.get_child("nvme_dev")) {

            nvme_spec = NvmeDev::ParseConfig(node);

            if (nvme_spec->enable) {
                AddDevice(NVME, (void *)nvme_spec);
            }
        }
    }

    NIC_HEADER_TRACE("Loading VirtIO devices");
    // Create VirtIO devices
    if (spec.get_child_optional("virtio_dev")) {
        struct virtio_devspec *virtio_spec;

        for (const auto &node : spec.get_child("virtio_dev")) {

            virtio_spec = VirtIODev::ParseConfig(node);

            if (virtio_spec->enable) {
                AddDevice(VIRTIO, (void *)virtio_spec);
            }
        }
    }

    NIC_HEADER_TRACE("Loading Pciestress devices");
    // Create Pciestress devices
    if (spec.get_child_optional("pciestress")) {
        for (const auto &node : spec.get_child("pciestress")) {
            if (0) Eth::ParseConfig(node); // XXX
            AddDevice(PCIESTRESS, NULL);
        }
    }
#endif //IRIS

    evutil_timer_start(EV_A_ &heartbeat_timer, DeviceManager::HeartbeatEventHandler, this, 0.0, 1);
    upg_state = DEVICES_ACTIVE_STATE;

    return 0;
}

Device *
DeviceManager::AddDevice(enum DeviceType type, void *dev_spec)
{
#ifdef IRIS
    AccelDev *accel_dev;
    NvmeDev *nvme_dev;
    VirtIODev *virtio_dev;
#endif //IRIS

    switch (type) {
        case MNIC:
            {
                std::vector<Eth*> eth_devices = Eth::factory(type, dev_api, dev_spec, pd, EV_A);
                for (std::size_t idx = 0; idx < eth_devices.size(); ++idx)
                    devices[eth_devices[idx]->GetName()] = eth_devices[idx];
                return (Device *)eth_devices[0];
            }
        case DEBUG:
            NIC_LOG_ERR("Unsupported Device Type DEBUG");
            return NULL;
        case ETH:
            {
                std::vector<Eth*> eth_devices = Eth::factory(type, dev_api, dev_spec, pd, EV_A);

                for (std::size_t idx = 0; idx < eth_devices.size(); ++idx) {
                    devices[eth_devices[idx]->GetName()] = eth_devices[idx];
                }

                if (upgrade_mode == FW_MODE_NORMAL_BOOT) {
                    // Create the host device
                    if (eth_devices[0]->GetType() == ETH_HOST_MGMT || eth_devices[0]->GetType() == ETH_HOST) {
                        if (!eth_devices[0]->CreateHostDevice()) {
                            NIC_LOG_ERR("{}: CreateHostDevice() failed", eth_devices[0]->GetName());
                            return NULL;
                        }
                    }
                    else {
                        NIC_LOG_DEBUG("{}: Skipped creating host device", eth_devices[0]->GetName());
                    }
                }

                return (Device *)eth_devices[0];
            }
#ifdef IRIS
        case ACCEL:
            accel_dev = new AccelDev(dev_api, dev_spec, pd, EV_A);
            accel_dev->SetType(type);
            devices[accel_dev->GetName()] = accel_dev;
            return (Device *)accel_dev;
        case NVME:
            nvme_dev = new NvmeDev(dev_api, dev_spec, pd, EV_A);
            nvme_dev->SetType(type);
            devices[nvme_dev->GetName()] = nvme_dev;
            return (Device *)nvme_dev;
        case VIRTIO:
            virtio_dev = new VirtIODev(dev_api, dev_spec, pd, EV_A);
            virtio_dev->SetType(type);
            devices[virtio_dev->GetName()] = virtio_dev;
            return (Device *)virtio_dev;
        case PCIESTRESS:
            extern class pciemgr *pciemgr;

            if (pciemgr) {
                pciehdevice_resources_t pres;
                static int instance;

                memset(&pres, 0, sizeof(pres));
                pres.type = PCIEHDEVICE_PCIESTRESS;
                snprintf(pres.pfres.name, sizeof(pres.pfres.name),
                         "pciestress%d", instance++);
                int ret = pciemgr->add_devres(&pres);
                if (ret != 0) {
                    NIC_LOG_ERR("pciestress add failed {}", ret);
                    return NULL;
                }
            }
            return NULL;
#endif //IRIS
        default:
            return NULL;
    }

    return NULL;
}

void
DeviceManager::RestoreDevicesState(std::vector <struct EthDevInfo *> eth_dev_list)
{

    NIC_LOG_DEBUG("Restoring total {} ETH devices after upgrade", eth_dev_list.size());
    for (uint32_t idx = 0; idx < eth_dev_list.size(); idx++) {
        Eth *eth_dev = new Eth(dev_api, eth_dev_list[idx], pd, EV_A);
        eth_dev->SetType(ETH);
        devices[eth_dev->GetName()] = eth_dev;
    }

    upg_state = DEVICES_ACTIVE_STATE;
}

Device *
DeviceManager::GetDevice(std::string name)
{
    return devices[name];
}

void
DeviceManager::DeleteDevice(std::string name)
{
    auto iter = devices.find(name);
    if (iter != devices.end()) {
        delete iter->second;
        devices.erase(iter);
    }
}

void
DeviceManager::SetHalClient(devapi *dev_api)
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->SetHalClient(dev_api);
        }

#ifdef IRIS
        if (dev->GetType() == ACCEL) {
            AccelDev *accel_dev = (AccelDev *)dev;
            accel_dev->SetHalClient(dev_api);
        }
        if (dev->GetType() == NVME) {
            NvmeDev *nvme_dev = (NvmeDev *)dev;
            nvme_dev->SetHalClient(dev_api);
        }
        if (dev->GetType() == VIRTIO) {
            VirtIODev *virtio_dev = (VirtIODev *)dev;
            virtio_dev->SetHalClient(dev_api);
        }
#endif //IRIS

    }
}

void
DeviceManager::HalEventHandler(bool status)
{
    NIC_HEADER_TRACE("HAL Event");

    // Hal UP
    if (status && !init_done) {
        NIC_LOG_DEBUG("Hal UP: Initializing hal client and creating VRFs.");
        // Instantiate HAL client
        dev_api = devapi_init();
        dev_api->set_fwd_mode(fwd_mode);
        pd->update();

        // Create uplinks
        if (!skip_hwinit) {
            for (auto it = uplinks.begin(); it != uplinks.end(); it++) {
                uplink_t *up = it->second;
                dev_api->uplink_create(up->id, up->port, up->is_oob);
            }
        }
        // Setting hal clients in all devices
        SetHalClient(dev_api);

        init_done = true;

        // Setup swm on native vlan
        // swm_update(true, 1, 10, 0x00AECD00113F);
    }

    // Bringup OOB first
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        NIC_LOG_DEBUG("dev: {}, type: {}", it->first, dev->GetType());
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
            NIC_LOG_DEBUG("eth dev type: {}", eth_dev->GetType());
            if (eth_dev->GetType() == ETH_MNIC_OOB_MGMT) {
                eth_dev->HalEventHandler(status);
                break;
            }
        }
    }

    // Bringup other devices
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *)dev;
            if (eth_dev->GetType() != ETH_MNIC_OOB_MGMT) {
                eth_dev->HalEventHandler(status);
            }
        }

#ifdef IRIS
        if (dev->GetType() == ACCEL) {
            AccelDev *accel_dev = (AccelDev *)dev;
            accel_dev->HalEventHandler(status);
        }
        if (dev->GetType() == NVME) {
            NvmeDev *nvme_dev = (NvmeDev *)dev;
            nvme_dev->HalEventHandler(status);
        }
        if (dev->GetType() == VIRTIO) {
            VirtIODev *virtio_dev = (VirtIODev *)dev;
            virtio_dev->HalEventHandler(status);
        }
#endif //IRIS
    }
}

void
DeviceManager::swm_update(bool enable,
                          uint32_t port_num, uint32_t vlan, mac_t mac)
{
    dev_api->swm_enable();
    dev_api->swm_set_port(port_num);
    dev_api->swm_add_mac(mac);
    dev_api->swm_add_vlan(vlan);
}

void
DeviceManager::SetFwStatus(uint8_t fw_status)
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->SetFwStatus(fw_status);
        }
    }
}

void
DeviceManager::LinkEventHandler(port_status_t *evd)
{
    NIC_HEADER_TRACE("Link Event");

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *) dev;
            eth_dev->LinkEventHandler(evd);
        }
    }
}

bool
DeviceManager::IsDataPathQuiesced()
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *) dev;
            if (!eth_dev->IsDevQuiesced())
                return false;
        }
    }

    return true;
}

bool
DeviceManager::CheckAllDevsDisabled()
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *) dev;
            if (!eth_dev->IsDevReset())
                return false;
        }
    }

    return true;
}

int
DeviceManager::SendFWDownEvent()
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *) dev;
            eth_dev->SendFWDownEvent();
        }
    }

    return 0;
}

void
DeviceManager::XcvrEventHandler(port_status_t *evd)
{
    NIC_HEADER_TRACE("Xcvr Event");

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *) dev;
            eth_dev->XcvrEventHandler(evd);
        }
    }
}

void
DeviceManager::HeartbeatEventHandler(void *obj)
{
    DeviceManager *devmgr = (DeviceManager *)obj;

    for (auto it = devmgr->devices.begin(); it != devmgr->devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *) dev;
            eth_dev->HeartbeatEventHandler();
        }
    }
}

void
DeviceManager::DelphiMountEventHandler(bool mounted)
{
    NIC_HEADER_TRACE("Mount Event");

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        dev->DelphiMountEventHandler(mounted);
    }
}

int
DeviceManager::GenerateQstateInfoJson(std::string qstate_info_file)
{
    pt::ptree root, lifs;

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *) dev;
            eth_dev->GenerateQstateInfoJson(lifs);
        }
    }

    root.push_back(std::make_pair("lifs", lifs));
    pt::write_json(qstate_info_file, root);
    return 0;
}

int
DeviceManager::HandleUpgradeEvent(UpgradeEvent event)
{
    port_status_t st = {0};

    switch(event) {
        case UPG_EVENT_QUIESCE:
            //send link down event for all the uplinks
            for (auto it = uplinks.begin(); it != uplinks.end(); it++) {
                uplink_t *up = it->second;
                st.id = up->port;
                LinkEventHandler(&st);
            }

            //send link down event to mgmt port
            st.id = 0;
            LinkEventHandler(&st);

            break;
        case UPG_EVENT_DEVICE_RESET:
            //Send fw_down event
            SendFWDownEvent();

            break;
        default:
            NIC_LOG_DEBUG("Upgrade Event {} not implemented", event);
    }

    return 0;
}

UpgradeState
DeviceManager::GetUpgradeState()
{
    switch (upg_state) {
        case DEVICES_ACTIVE_STATE:
            if(IsDataPathQuiesced())
                upg_state = DEVICES_QUIESCED_STATE;

            break;
        case DEVICES_QUIESCED_STATE:
            if(CheckAllDevsDisabled())
                upg_state = DEVICES_RESET_STATE;

            break;
        case DEVICES_RESET_STATE:
            //nothing to be done here

            break;
        default:
            NIC_LOG_DEBUG("Unsupported upgrade state {}", upg_state);
    }

    return upg_state;
}

std::vector <struct EthDevInfo *>
DeviceManager::GetEthDevStateInfo()
{
    std::vector <struct EthDevInfo *> eth_dev_info_list;

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *) dev;
            struct EthDevInfo *info = new EthDevInfo();

            eth_dev->GetEthDevInfo(info);
            NIC_LOG_DEBUG("adding {} to save list", info->eth_spec->name);
            eth_dev_info_list.push_back(info);
        }
    }

    return eth_dev_info_list;
}

bool
DeviceManager::UpgradeCompatCheck()
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        // Upgrade is not possible for non-ETH devices
        if (dev->GetType() != ETH) {
            NIC_LOG_DEBUG("Upgrade is not possible with non-ETH device {}. "
                    "compat check failed for Upgrade", it->first);
            return false;
        }
        else {
            // If RDMA is enabled in ETH device then also upgrade is not feasible
            Eth *eth_dev = (Eth *) dev;
            struct EthDevInfo *info = new EthDevInfo();

            eth_dev->GetEthDevInfo(info);
            if (info->eth_spec->enable_rdma) {
                NIC_LOG_WARN("RDMA enabled ETH device {} will not function "
                        "after upgrade", it->first);
                /*
                 * TODO: as of now we need to ignore RDMA check since
                 * smart-nic profile has RDMA default enabled
                 */
                //return false;
            }
        }
    }

    return true;
}

