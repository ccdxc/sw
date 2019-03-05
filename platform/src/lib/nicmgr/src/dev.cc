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

#include "gen/proto/device.pb.h"

#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/sdk/platform/misc/include/maclib.h"

#include "logger.hpp"

#include "pd_client.hpp"
#include "adminq.hpp"
#include "dev.hpp"
#include "eth_dev.hpp"
#include "accel_dev.hpp"


namespace pt = boost::property_tree;

DeviceManager *DeviceManager::instance;


EthDevType
eth_dev_type_str_to_type(std::string const& s)
{
    if (s == "host") {
        return ETH_HOST;
    } else if (s == "host_mgmt") {
        return ETH_HOST_MGMT;
    } else if (s == "oob_mgmt") {
        return ETH_MNIC_OOB_MGMT;
    } else if (s == "internal_mgmt") {
        return ETH_MNIC_INTERNAL_MGMT;
    } else if (s == "inband_mgmt") {
        return ETH_MNIC_INBAND_MGMT;
    } else {
        NIC_LOG_ERR("Unknown ETH dev type: {}", s);
        return ETH_UNKNOWN;
    }
}

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
        default: return "Unknown";
    }
}

OpromType
oprom_type_str_to_type(std::string const& s)
{
    if (s == "legacy") {
        return OPROM_LEGACY;
    } else if (s == "uefi") {
        return OPROM_UEFI;
    } else if (s == "unified") {
        return OPROM_UNIFIED;
    } else {
        NIC_LOG_ERR("Unknown OPROM type: {}", s);
        return OPROM_UNKNOWN;
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

DeviceManager::DeviceManager(std::string config_file, enum ForwardingMode fwd_mode,
                             platform_t platform)
{
    NIC_HEADER_TRACE("Initializing DeviceManager");
    init_done = false;
    instance = this;
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) ==
               sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) ==
               sdk::lib::PAL_RET_OK);
#endif
    this->fwd_mode = fwd_mode;
    this->config_file = config_file;
    pd = PdClient::factory(platform);
    assert(pd);

    // Reserve all the LIF ids used by HAL
    NIC_LOG_DEBUG("Reserving HAL lifs {}-{}", HAL_LIF_ID_MIN, HAL_LIF_ID_MAX);
    // int ret = pd->lm_->LIFRangeAlloc(HAL_LIF_ID_MIN, HAL_LIF_ID_MAX);
    int ret = pd->lm_->reserve_id(HAL_LIF_ID_MIN, (HAL_LIF_ID_MAX - HAL_LIF_ID_MIN + 1));
    if (ret < 0) {
        throw runtime_error("Failed to reserve HAL LIFs");
    }
}

string
DeviceManager::ParseDeviceConf(string filename)
{
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

    return string("");
}

int
DeviceManager::LoadConfig(string path)
{
    struct eth_devspec *eth_spec;
    struct accel_devspec *accel_spec;

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
    if (readKey(MACADDRESS_KEY, mac_str) == 0) {
        mac_from_str(&fru_mac, mac_str.c_str());
    } else {
        NIC_LOG_ERR("Failed to read MAC address from FRU");
    }
    if (readKey(NUMMACADDR_KEY, num_macs_str) == 0) {
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
#if 0
                Uplink::Factory(val.get<uint64_t>("id"),
                                val.get<uint64_t>("port"),
                                val.get<bool>("oob", false));
#endif
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
            eth_spec = new struct eth_devspec;
            memset(eth_spec, 0, sizeof(*eth_spec));

            auto val = node.second;

            eth_spec->name = val.get<string>("name");
            eth_spec->dev_uuid = val.get<uint64_t>("dev_uuid");
            eth_spec->lif_count = val.get<uint64_t>("lif_count");
            eth_spec->rxq_count = val.get<uint64_t>("rxq_count");
            eth_spec->txq_count = val.get<uint64_t>("txq_count");
            eth_spec->eq_count = val.get<uint64_t>("eq_count");
            eth_spec->adminq_count = val.get<uint64_t>("adminq_count");
            eth_spec->intr_count = val.get<uint64_t>("intr_count");
            eth_spec->mac_addr = mnic_mac_base--;

            if (val.get_optional<string>("network")) {
                eth_spec->uplink_port_num = val.get<uint64_t>("network.uplink");
            }

            if (val.get_optional<string>("type")) {
                eth_spec->eth_type = eth_dev_type_str_to_type(val.get<string>("type"));
            } else {
                eth_spec->eth_type = ETH_UNKNOWN;
            }

            eth_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
            NIC_LOG_DEBUG("Creating mnic device with name: {}, type: {}, "
                         " pinned_uplink: {}, intr_count: {}, qos_group {}",
                         eth_spec->name,
                         eth_dev_type_to_str(eth_spec->eth_type),
                         eth_spec->uplink_port_num,
                         eth_spec->intr_count,
                         eth_spec->qos_group);
            AddDevice(ETH, (void *)eth_spec);
        }
    }

    NIC_HEADER_TRACE("Loading Eth devices");
    // Create Ethernet devices
    if (spec.get_child_optional("eth_dev")) {
        for (const auto &node : spec.get_child("eth_dev")) {
            eth_spec = new struct eth_devspec;
            memset(eth_spec, 0, sizeof(*eth_spec));

            auto val = node.second;

            eth_spec->name = val.get<string>("name");
            eth_spec->dev_uuid = val.get<uint64_t>("dev_uuid");
            eth_spec->lif_count = val.get<uint64_t>("lif_count");
            eth_spec->rxq_count = val.get<uint64_t>("rxq_count");
            eth_spec->txq_count = val.get<uint64_t>("txq_count");
            eth_spec->eq_count = val.get<uint64_t>("eq_count");
            eth_spec->adminq_count = val.get<uint64_t>("adminq_count");
            eth_spec->intr_count = val.get<uint64_t>("intr_count");
            if (host_mac_base == mnic_mac_base) {
                NIC_LOG_ERR("Number of macs {} not enough for Host ifs and Mnic ifs.",
                            num_macs);
            }
            eth_spec->mac_addr = host_mac_base++;

            if (val.get_optional<string>("rdma")) {
                eth_spec->enable_rdma = true;
                eth_spec->rdma_sq_count = val.get<uint64_t>("rdma.sq_count");
                eth_spec->rdma_rq_count = val.get<uint64_t>("rdma.rq_count");
                eth_spec->rdma_cq_count = val.get<uint64_t>("rdma.cq_count");
                eth_spec->rdma_eq_count = val.get<uint64_t>("rdma.eq_count");
                eth_spec->rdma_adminq_count = val.get<uint64_t>("rdma.adminq_count");
                eth_spec->rdma_pid_count = val.get<uint64_t>("rdma.pid_count");
                eth_spec->key_count = val.get<uint64_t>("rdma.key_count");
                eth_spec->pte_count = val.get<uint64_t>("rdma.pte_count");
                eth_spec->ah_count = val.get<uint64_t>("rdma.ah_count");
                //eth_spec->barmap_size = val.get<uint64_t>("rdma.barmap_size");
                eth_spec->barmap_size = 1;
            }

            if (val.get_optional<string>("network")) {
                eth_spec->uplink_port_num = val.get<uint64_t>("network.uplink");
            }

            eth_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
            if (val.get_optional<string>("pcie.oprom")) {
                eth_spec->oprom = oprom_type_str_to_type(val.get<string>("pcie.oprom"));
            }

            eth_spec->host_dev = true;
            if (val.get_optional<string>("type")) {
                eth_spec->eth_type = eth_dev_type_str_to_type(val.get<string>("type"));
            } else {
                eth_spec->eth_type = ETH_UNKNOWN;
            }

            eth_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
            NIC_LOG_DEBUG("Creating eth device with name: {}, type: {}, "
                         "pinned_uplink: {}, qos_group {}",
                         eth_spec->name,
                         eth_dev_type_to_str(eth_spec->eth_type),
                         eth_spec->uplink_port_num,
                         eth_spec->qos_group);
            AddDevice(ETH, (void *)eth_spec);
        }
    }

    NIC_HEADER_TRACE("Loading Accel devices");
    // Create Accelerator devices
    if (spec.get_child_optional("accel_dev")) {
        for (const auto &node : spec.get_child("accel_dev")) {
            accel_spec = new struct accel_devspec;
            memset(accel_spec, 0, sizeof(*accel_spec));

            auto val = node.second;

            accel_spec->name = val.get<string>("name");
            accel_spec->lif_count    = val.get<uint64_t>("lif_count");
            accel_spec->seq_queue_count = val.get<uint32_t>("seq_queue_count");
            accel_spec->adminq_count = val.get<uint32_t>("adminq_count");
            accel_spec->intr_count = val.get<uint32_t>("intr_count");
            if (val.get_optional<string>("rate_limit")) {
                if (val.get_optional<string>("rate_limit.rx_limit_gbps")) {
                    accel_spec->rx_limit_gbps = val.get<uint32_t>("rate_limit.rx_limit_gbps");
                }
                if (val.get_optional<string>("rate_limit.rx_burst_gb")) {
                    accel_spec->rx_burst_gb = val.get<uint32_t>("rate_limit.rx_burst_gb");
                }
                if (val.get_optional<string>("rate_limit.tx_limit_gbps")) {
                    accel_spec->tx_limit_gbps = val.get<uint32_t>("rate_limit.tx_limit_gbps");
                }
                if (val.get_optional<string>("rate_limit.tx_burst_gb")) {
                    accel_spec->tx_burst_gb = val.get<uint32_t>("rate_limit.tx_burst_gb");
                }
            }

            accel_spec->pub_intv_frac = ACCEL_DEV_PUB_INTV_FRAC_DFLT;
            if (val.get_optional<string>("publish_interval")) {
                accel_spec->pub_intv_frac = val.get<uint32_t>("publish_interval.sec_fraction");
            }

            accel_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
            accel_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
            NIC_LOG_DEBUG("Creating accel device with name: {}, qos_group: {}",
                         accel_spec->name,
                         accel_spec->qos_group);
            AddDevice(ACCEL, (void *)accel_spec);
        }
    }

    return 0;
}

Device *
DeviceManager::AddDevice(enum DeviceType type, void *dev_spec)
{
    Eth *eth_dev;
    Accel_PF *accel_dev;

    switch (type) {
    case MNIC:
        eth_dev = new Eth(hal, dev_api, dev_spec, pd);
        eth_dev->SetType(MNIC);
        devices[eth_dev->GetName()] = eth_dev;
        return (Device *)eth_dev;
    case DEBUG:
        NIC_LOG_ERR("Unsupported Device Type DEBUG");
        return NULL;
    case ETH:
        eth_dev = new Eth(hal, dev_api, dev_spec, pd);
        eth_dev->SetType(type);
        devices[eth_dev->GetName()] = eth_dev;
        return (Device *)eth_dev;
    case ACCEL:
        accel_dev = new Accel_PF(hal, dev_api, dev_spec, pd);
        accel_dev->SetType(type);
        devices[accel_dev->GetName()] = accel_dev;
        return (Device *)accel_dev;
    case NVME:
        NIC_LOG_ERR("Unsupported Device Type NVME");
        return NULL;
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
DeviceManager::SetHalClient(HalClient *hal_client, devapi *dev_api)
{
    hal_client->set_devapi(dev_api);
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->SetHalClient(hal_client, dev_api);
        }
        if (dev->GetType() == ACCEL) {
            Accel_PF *accel_dev = (Accel_PF *)dev;
            accel_dev->SetHalClient(hal_client, dev_api);
        }
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
        hal = new HalClient(fwd_mode);
        dev_api = devapi_iris::factory();
        dev_api->set_fwd_mode((fwd_mode_t)fwd_mode);
#if 0
        hal_common_client = HalGRPCClient::Factory((HalForwardingMode)fwd_mode);
        // Setting up HAL container for all objects
        HalObject::PopulateHalCommonClient();
#endif
        pd->update();

        // Create uplinks
        for (auto it = uplinks.begin(); it != uplinks.end(); it++) {
            uplink_t *up = it->second;
            dev_api->uplink_create(up->id, up->port, up->is_oob);
        }

        // Create VRFs for uplinks
        // Uplink::CreateVrfs();

        // Setting hal clients in all devices
        SetHalClient(hal, dev_api);

        init_done = true;
    }

#if 0
    // If Sysmgr is able to update HAL Status to DOWN, we can handle it.
    // Hal DOWN
    if (!status) {
        if (hal_common_client) {
            NIC_LOG_DEBUG("Hal DOWN: Freeing up hal client");
            // Cleanup HAL client
            HalGRPCClient::Destroy(hal_common_client);
            hal_common_client = NULL;
            // Setting up HAL container for all objects
            HalObject::PopulateHalCommonClient();
        }
    }
#endif

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH || dev->GetType() == MNIC) {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->HalEventHandler(status);
        }
        if (dev->GetType() == ACCEL) {
            Accel_PF *accel_dev = (Accel_PF *)dev;
            accel_dev->HalEventHandler(status);
        }
    }
}

void
DeviceManager::LinkEventHandler(hal_port_status_t *evd)
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

void
DeviceManager::ThreadsWaitJoin(void)
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ACCEL) {
            Accel_PF *accel_dev = (Accel_PF *)dev;
            accel_dev->ThreadsWaitJoin();
        }
    }
}
