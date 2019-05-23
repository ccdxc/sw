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

#if !defined(APOLLO) && !defined(ARTEMIS)
#include "gen/proto/device.pb.h"
#include "accel_dev.hpp"
#endif

#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/sdk/platform/misc/include/maclib.h"

#include "logger.hpp"

#include "pd_client.hpp"
#include "adminq.hpp"
#include "dev.hpp"
#include "eth_dev.hpp"
#include "nicmgr_init.hpp"
#include "nvme_dev.hpp"

using namespace std;

namespace pt = boost::property_tree;

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
                             platform_t platform, EV_P)
{
    NIC_HEADER_TRACE("Initializing DeviceManager");
    init_done = false;
    instance = this;
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) ==
               sdk::lib::PAL_RET_OK);
#elif __aarch64__
#if !defined(APOLLO) && !defined(ARTEMIS)
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

    NIC_LOG_DEBUG("Event loop: {:#x}", (uint64_t)this->loop);

    // Reserve all the LIF ids used by HAL
    NIC_LOG_DEBUG("Reserving HAL lifs {}-{}", HAL_LIF_ID_MIN, HAL_LIF_ID_MAX);
    // int ret = pd->lm_->LIFRangeAlloc(HAL_LIF_ID_MIN, HAL_LIF_ID_MAX);
    int ret = pd->lm_->reserve_id(HAL_LIF_ID_MIN, (HAL_LIF_ID_MAX - HAL_LIF_ID_MIN + 1));
    if (ret < 0) {
        throw runtime_error("Failed to reserve HAL LIFs");
    }

    upg_state = UNKNOWN_STATE;
}

string
DeviceManager::ParseDeviceConf(string filename)
{
#if !defined(APOLLO) && !defined(ARTEMIS)
    boost::property_tree::ptree spec;

    cout << "Parsing Device conf, input: " << filename << endl;
    if (filename.compare("none") == 0) {
        /* No device.conf file. Classic default mode */
        return string("/platform/etc/nicmgrd/device.json");
    }

    /* Parse the input device.conf json file */
    boost::property_tree::read_json(filename, spec);
    int fw_mode = spec.get<int>("forwarding-mode");
    int feature_profile = spec.get<int>("feature-profile");
    cout << "fw_mode: " <<
        device::ForwardingMode_Name(device::ForwardingMode(fw_mode)) << endl;
    cout << "feature_profile: " <<
        device::FeatureProfile_Name(device::FeatureProfile(feature_profile)) << endl;

    if ((fw_mode == device::FORWARDING_MODE_HOSTPIN) ||
        (fw_mode == device::FORWARDING_MODE_SWITCH)) {
        return string("/platform/etc/nicmgrd/eth_smart.json");
    } else if (fw_mode == device::FORWARDING_MODE_CLASSIC) {
        if (feature_profile == device::FEATURE_PROFILE_CLASSIC_DEFAULT) {
            return string("/platform/etc/nicmgrd/device.json");
        } else if (feature_profile == device::FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE) {
            return string("/platform/etc/nicmgrd/eth_scale.json");
        } else {
            cout << "Profile is none! Return classic default" << endl;
            return string("/platform/etc/nicmgrd/device.json");
        }
    } else {
        cout << "Unknown mode, returning classic default" << endl;
        return string("/platform/etc/nicmgrd/device.json");
    }
#endif
    return string("");
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
    uplink_t *up = NULL;
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
                NIC_LOG_DEBUG("Creating uplink: {}, oob: {}",
                             val.get<uint64_t>("id"),
                             val.get<bool>("oob", false));

                up = new uplink_t();
                up->id = val.get<uint64_t>("id");
                up->port = val.get<uint64_t>("port");
                up->is_oob = val.get<bool>("oob", false);
                uplinks[up->id] = up;
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

#endif //IRIS

    upg_state = DEVICES_ACTIVE_STATE;

    return 0;
}

Device *
DeviceManager::AddDevice(enum DeviceType type, void *dev_spec)
{
#ifdef IRIS
    AccelDev *accel_dev;
    NvmeDev *nvme_dev;
#endif //IRIS

    switch (type) {
    case MNIC:
        {
            //For mnic upgrade_mode is always false
            std::vector<Eth*> eth_devices = Eth::factory(type, dev_api, dev_spec, pd, EV_A_ false);
            for (std::size_t idx = 0; idx < eth_devices.size(); ++idx)
                devices[eth_devices[idx]->GetName()] = eth_devices[idx];
            return (Device *)eth_devices[0];
        }
    case DEBUG:
        NIC_LOG_ERR("Unsupported Device Type DEBUG");
        return NULL;
    case ETH:
        {
            std::vector<Eth*> eth_devices = Eth::factory(type, dev_api, dev_spec, pd, EV_A_ upgrade_mode);
            for (std::size_t idx = 0; idx < eth_devices.size(); ++idx) {
                //Create PCIe device for only PF
                if (!idx) {
                    // Create the device
                    if (eth_devices[idx]->GetType() == ETH_HOST_MGMT || eth_devices[idx]->GetType() == ETH_HOST) {
                        if (!eth_devices[idx]->CreateHostDevice()) {
                            NIC_LOG_ERR("CreateHostDevice() failed for {}", eth_devices[idx]->GetName());
                            return NULL;
                        }
                    }
                    else {
                        NIC_LOG_DEBUG("{}: Skipped creating host device", eth_devices[idx]->GetName());
                    }
                }
                NIC_LOG_DEBUG("{}: Adding device to dev manager", eth_devices[idx]->GetName());
                devices[eth_devices[idx]->GetName()] = eth_devices[idx];
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
#endif //IRIS
    case VIRTIO:
        NIC_LOG_ERR("Unsupported Device Type VIRTIO");
        return NULL;
    default:
        return NULL;
    }

    return NULL;
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
        for (auto it = uplinks.begin(); it != uplinks.end(); it++) {
            uplink_t *up = it->second;
            dev_api->uplink_create(up->id, up->port, up->is_oob);
        }

        // Setting hal clients in all devices
        SetHalClient(dev_api);

        init_done = true;
    }

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->HalEventHandler(status);
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
#endif //IRIS
    }
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
            NIC_LOG_DEBUG("Event {} not implemented", event);
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
            NIC_LOG_DEBUG("Unsupported state {}", upg_state);
    }

    return upg_state;
}
