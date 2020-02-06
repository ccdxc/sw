/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "nic/sdk/lib/device/device.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/sdk/platform/misc/include/maclib.h"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"

#include "logger.hpp"

#include "dev.hpp"
#include "eth_dev.hpp"
#ifdef IRIS
#include "accel_dev.hpp"
#include "nvme_dev.hpp"
#include "virtio_dev.hpp"
#endif  // IRIS
#ifdef ATHENA
#include "ftl_dev.hpp"
#endif
#include "nicmgr_init.hpp"
#include "nicmgr_utils.hpp"
#include "pd_client.hpp"

using namespace std;

namespace pt = boost::property_tree;

#define HEARTBEAT_PERIOD_S       1 // seconds
#define HEARTBEAT_MAX_PERIOD_S   10 // seconds

DeviceManager *DeviceManager::instance;
pciemgr *pciemgr;

/*
 * DeviceManger's pcie event handlers
 */

void
DevPcieEvHandler::memrd(const int port, const uint32_t lif, const pciehdev_memrw_notify_t *n)
{
    NIC_LOG_INFO("memrd: port {} lif {}"
                 " bar {} baraddr {:#x} baroffset {:#x}"
                 " size {} localpa {:#x} data {:#x}",
                 port, lif, n->cfgidx, n->baraddr, n->baroffset, n->size, n->localpa, n->data);
}

void
DevPcieEvHandler::memwr(const int port, const uint32_t lif, const pciehdev_memrw_notify_t *n)
{
    NIC_LOG_INFO("memwr: port {} lif {}"
                 " bar {} baraddr {:#x} baroffset {:#x} "
                 " size {} localpa {:#x} data {:#x}",
                 port, lif, n->cfgidx, n->baraddr, n->baroffset, n->size, n->localpa, n->data);
}

void
DevPcieEvHandler::hostup(const int port)
{
    NIC_LOG_INFO("hostup: port {}", port);
}

void
DevPcieEvHandler::hostdn(const int port)
{
    NIC_LOG_INFO("hostdn: port {}", port);
}

void
DevPcieEvHandler::sriov_numvfs(const int port, const uint32_t lif, const uint16_t numvfs)
{
    NIC_LOG_INFO("sriov_numvfs: port {} lif {} numvfs {}", port, lif, numvfs);
}

void
DevPcieEvHandler::reset(const int port, uint32_t rsttype, const uint32_t lifb, const uint32_t lifc)
{
    DeviceManager *devmgr;
    Eth *eth_dev;

    NIC_LOG_DEBUG("reset: port {} rsttype {} lifb {} lifc {}", port, rsttype, lifb, lifc);

    devmgr = DeviceManager::GetInstance();
    if (devmgr == NULL) {
        NIC_LOG_ERR("{}: Devmgr instance not found", __func__);
        return;
    }

    eth_dev = (Eth *)devmgr->GetDeviceByLif(lifb);
    if (eth_dev == NULL) {
        NIC_LOG_ERR("{}: No Eth device found for lif {}", __func__, lifb);
        return;
    }

    eth_dev->PcieResetEventHandler(rsttype);
}

/**
 * Device Manager
 */
DeviceManager::DeviceManager(sdk::platform::platform_type_t platform,
                             std::string device_conf_file,
                             sdk::lib::dev_forwarding_mode_t fwd_mode,
                             bool micro_seg_en,
                             EV_P)
{
    string device_json_file, hbm_mem_json_file;

    NIC_HEADER_TRACE("Initializing DeviceManager");
    init_done = false;
    instance = this;

    this->platform = platform;
    NIC_LOG_DEBUG("Platform {}", platform);

    sdk::lib::pal_init(platform);

    GetConfigFiles(device_conf_file, hbm_mem_json_file, device_json_file);

    this->loop = (loop == NULL) ? EV_DEFAULT : loop;
    this->fwd_mode = fwd_mode;
    this->micro_seg_en = micro_seg_en;
    this->device_json_file = device_json_file;
    this->dev_api = NULL;

    pd = PdClient::factory(platform, hbm_mem_json_file);
    assert(pd);
    this->skip_hwinit = pd->is_dev_hwinit_done(NULL);

    // Reserve all the LIF ids used by HAL
    NIC_LOG_DEBUG("Reserving HAL lifs {}-{}", HAL_LIF_ID_MIN, HAL_LIF_ID_MAX);
    int ret = pd->lm_->reserve_id(HAL_LIF_ID_MIN, (HAL_LIF_ID_MAX - HAL_LIF_ID_MIN + 1));
    if (ret < 0) {
        throw runtime_error("Failed to reserve HAL LIFs");
    }
    NIC_LOG_DEBUG("Reserving NCSI lifs {}-{}", NICMGR_NCSI_LIF_MIN, NICMGR_NCSI_LIF_MAX);
    ret = pd->lm_->reserve_id(NICMGR_NCSI_LIF_MIN, 
                              (NICMGR_NCSI_LIF_MAX - NICMGR_NCSI_LIF_MIN + 1));
    if (ret < 0) {
        throw runtime_error("Failed to reserve NCSI LIFs");
    }
    if (!skip_hwinit) {
        ret = sdk::platform::utils::lif_mgr::lifs_reset(NICMGR_SVC_LIF, NICMGR_LIF_MAX);
        if (ret != sdk::SDK_RET_OK) {
            throw runtime_error("Failed to reset LIFs");
        }
    }
    upg_state = UNKNOWN_STATE;

    // initialize pciemgr
    if (platform_is_hw(platform)) {
        pciemgr = new class pciemgr("nicmgrd", pcie_evhandler, EV_A);
    }
}

static std::string
hal_cfg_path()
{
    std::string hal_cfg_path;
    if (std::getenv("HAL_CONFIG_PATH") == NULL) {
        hal_cfg_path = "/nic/conf/";
    } else {
        hal_cfg_path = std::string(std::getenv("HAL_CONFIG_PATH"));
    }

    return hal_cfg_path;
}

void
DeviceManager::GetConfigFiles(string device_conf_file, string &hbm_mem_json_file,
                              string &device_json_file)
{
    sdk::lib::device *device = NULL;
    sdk::lib::dev_feature_profile_t feature_profile;
    string profile_name;
    std::string hal_cfg_dir = hal_cfg_path();

    device = sdk::lib::device::factory(device_conf_file);
    feature_profile = device->get_feature_profile();

     profile_name = std::string(DEV_FEATURE_PROFILE_str(feature_profile));
     profile_name.replace(0, std::string("FEATURE_PROFILE").length(), "");
     std::transform(profile_name.begin(), profile_name.end(),
                    profile_name.begin(), ::tolower);

    // Handle for other pipelines
#if defined(APOLLO)
     hbm_mem_json_file = hal_cfg_dir + "/apollo/hbm_mem.json";
#elif defined(ARTEMIS)
     hbm_mem_json_file = hal_cfg_dir + "/artemis/hbm_mem.json";
#elif defined(APULU)
     hbm_mem_json_file = hal_cfg_dir + "/apulu/8g/hbm_mem.json";
#elif defined(ATHENA)
     hbm_mem_json_file = hal_cfg_dir + "/athena/hbm_mem.json";
#else
     hbm_mem_json_file =  hal_cfg_dir + "/" +
         "iris" + "/hbm_mem" + profile_name + ".json";
     device_json_file = string("/platform/etc/nicmgrd/device" + profile_name + ".json");
#endif

    NIC_LOG_DEBUG("HBM json file: {}, Device json file: {}",
                  hbm_mem_json_file, device_json_file);
}


void
DeviceManager::CreateUplink(uint32_t id, uint32_t port, bool is_oob)
{
    uplink_t *up = NULL;
    NIC_LOG_DEBUG("Creating uplink: id: {} port: {}, oob: {}", id, port, is_oob);

    up = new uplink_t();
    up->id = id;
    up->port = port;
    up->is_oob = is_oob;
    uplinks[up->id] = up;
}

static bool
valid_mac_base(string where, uint64_t mac_base, uint32_t &mac_count)
{
    char mac_str[32] = {0};
    if (!is_unicast_mac_addr(&mac_base)) {
        NIC_LOG_INFO("Invalid {} mac address {}", where, mac_to_str(&mac_base, mac_str, sizeof(mac_str)));
        return false;
    }
    NIC_LOG_INFO("Valid {} mac address {}", where, mac_to_str(&mac_base, mac_str, sizeof(mac_str)));

    if ((mac_base & NICMGR_MAC_DEV_MASK) + mac_count > NICMGR_MAC_DEV_CAP) {
        NIC_LOG_INFO("Invalid {} mac count {}", where, mac_count);
        mac_count = NICMGR_MAC_DEV_CAP - (mac_base & NICMGR_MAC_DEV_MASK);
        NIC_LOG_INFO("Clamped {} mac count {}", where, mac_count);
    }
    if (mac_count < NICMGR_MIN_MAC_COUNT) {
        NIC_LOG_INFO("Invalid {} mac count {} fewer than min {}", where, mac_count, NICMGR_MIN_MAC_COUNT);
        return false;
    }
    NIC_LOG_INFO("Valid {} mac count {}", where, mac_count);

    return true;
}

#ifdef __aarch64__
static bool
read_fru_mac_base(uint64_t &mac_base, uint32_t &mac_count)
{
    string tmp_str;

    if (sdk::platform::readFruKey(MACADDRESS_KEY, tmp_str) == 0) {
        mac_from_str(&mac_base, tmp_str.c_str());
    } else {
        NIC_LOG_ERR("Failed to read MAC address from FRU");
        return false;
    }

    if (sdk::platform::readFruKey(NUMMACADDR_KEY, tmp_str) == 0) {
        mac_count = std::stoi(tmp_str);
    } else {
        NIC_LOG_ERR("Failed to read MAC address count from FRU");
        mac_count = NICMGR_DEF_MAC_COUNT;
    }

    return valid_mac_base("FRU", mac_base, mac_count);
}
#endif

static bool
read_env_mac_base(uint64_t &mac_base, uint32_t &mac_count)
{
    if (getenv("SYSUUID") != NULL) {
        mac_from_str(&mac_base, getenv("SYSUUID"));
    } else {
        NIC_LOG_DEBUG("SYSUUID environment variable is not set");
        return false;
    }

    mac_count = NICMGR_DEF_MAC_COUNT;

    return valid_mac_base("SYSUUID", mac_base, mac_count);
}

static bool
read_spec_mac_base(uint64_t &mac_base, uint32_t &mac_count,
                   boost::property_tree::ptree &spec)
{
    if (!spec.get_child_optional("network") ||
        !spec.get_child_optional("network.mac_addr")) {
        return false;
    }

    auto base = spec.get_optional<string>("network.mac_addr.base");
    if (base) {
        mac_from_str(&mac_base, base->c_str());
    } else {
        NIC_LOG_DEBUG("DEVSPEC network.mac_addr.base is not set");
        return false;
    }

    auto count = spec.get_optional<uint32_t>("network.mac_addr.count");
    if (count) {
        mac_count = *count;
    } else {
        NIC_LOG_DEBUG("DEVSPEC network.mac_addr.count is not set");
        mac_count = NICMGR_DEF_MAC_COUNT;
    }

    return valid_mac_base("DEVSPEC", mac_base, mac_count);
}

static bool
deadbeef_mac_base(uint64_t &mac_base, uint32_t &mac_count)
{
    mac_from_str(&mac_base, "00:de:ad:be:ef:00");
    mac_count = NICMGR_DEF_MAC_COUNT;
    return true;
}

int
DeviceManager::LoadProfile(string device_json_file, bool init_pci)
{
    struct eth_devspec *eth_spec;

    NIC_HEADER_TRACE("Loading Config");
    if (!device_json_file.empty()) {
        this->device_json_file = device_json_file;
    }
    NIC_LOG_DEBUG("Json: {}", this->device_json_file);
    boost::property_tree::read_json(this->device_json_file, spec);

    // Determine the base mac address
    uint64_t host_mac_base = 0;
    uint64_t mnic_mac_base = 0;
    uint64_t sys_mac_base = 0;
    uint32_t sys_mac_count = 0;

    // Read & validate & set base mac address
    read_spec_mac_base(sys_mac_base, sys_mac_count, spec) ||
#ifdef __aarch64__
        read_fru_mac_base(sys_mac_base, sys_mac_count) ||
#endif
        read_env_mac_base(sys_mac_base, sys_mac_count) ||
        deadbeef_mac_base(sys_mac_base, sys_mac_count);

    /*
     * Host Ifs: <sys_mac_base .....
     * Mnic Ifs: ......mnic_mac_base>
     */
    host_mac_base = sys_mac_base;
    mnic_mac_base = sys_mac_base + sys_mac_count - 1;

    char sys_mac_str[32] = {0};
    NIC_LOG_INFO("Number of Macs: {}", sys_mac_count);
    NIC_LOG_INFO("Base mac address {}", mac_to_str(&sys_mac_base, sys_mac_str, sizeof(sys_mac_str)));
    NIC_LOG_INFO("Host Base: {}", mac_to_str(&host_mac_base, sys_mac_str, sizeof(sys_mac_str)));
    NIC_LOG_INFO("Mnic Base: {}", mac_to_str(&mnic_mac_base, sys_mac_str, sizeof(sys_mac_str)));

    //
    if (platform_is_hw(platform) && pciemgr && init_pci) {
        NIC_LOG_INFO("Initializing PCI configuration");
        pciemgr->initialize();
    }

    // Create Network
    if (spec.get_child_optional("network")) {
        // Create Uplinks
        if (spec.get_child_optional("network.uplink")) {
            for (const auto &node : spec.get_child("network.uplink")) {
                auto val = node.second;
                CreateUplink(val.get<uint64_t>("id"), val.get<uint64_t>("port"),
                             val.get<bool>("oob", false));
            }
        }
    }

    NIC_HEADER_TRACE("Loading Mnic devices");
    // Create MNICs
    if (spec.get_child_optional("mnic_dev")) {
        for (const auto &node : spec.get_child("mnic_dev")) {

            eth_spec = Eth::ParseConfig(node);

            if (eth_spec->mac_addr == 0) {
                eth_spec->mac_addr = mnic_mac_base--;
            }

            AddDevice(ETH, (void *)eth_spec);
        }
    }

    NIC_HEADER_TRACE("Loading Eth devices");
    // Create Ethernet devices
    if (spec.get_child_optional("eth_dev")) {
        for (const auto &node : spec.get_child("eth_dev")) {

            eth_spec = Eth::ParseConfig(node);

            if (eth_spec->mac_addr == 0) {
                if (host_mac_base >= mnic_mac_base) {
                    NIC_LOG_ERR("Number of macs {} not enough for Host ifs and Mnic ifs.",
                                sys_mac_count);
                }
                eth_spec->mac_addr = host_mac_base++;
            }

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
            if (0)
                Eth::ParseConfig(node); // XXX
            AddDevice(PCIESTRESS, NULL);
        }
    }
#endif // IRIS

#ifdef ATHENA
    NIC_HEADER_TRACE("Loading FTL devices");
    if (spec.get_child_optional("ftl_dev")) {
        struct ftl_devspec *ftl_spec;

        for (const auto &node : spec.get_child("ftl_dev")) {

            ftl_spec = FtlDev::ParseConfig(node);
            AddDevice(FTL, (void *)ftl_spec);
        }
    }
#endif

    if (platform_is_hw(platform) && pciemgr && init_pci) {
        NIC_LOG_INFO("Finalizing PCI configuration");
        pciemgr->finalize();
    }

    NIC_LOG_INFO("Starting Heartbeat timer");
    evutil_timer_start(EV_A_ &heartbeat_timer, DeviceManager::HeartbeatEventHandler, this, 0.0, HEARTBEAT_PERIOD_S);
    clock_gettime(CLOCK_MONOTONIC, &hb_last);

    upg_state = DEVICES_ACTIVE_STATE;

    return 0;
}

void
DeviceManager::AddDevice(enum DeviceType type, void *dev_spec)
{
    switch (type) {
    case DEBUG:
        NIC_LOG_ERR("Unsupported Device Type DEBUG");
        break;
    case ETH: {
        std::vector<Eth *> eth_devices = Eth::factory(dev_api, dev_spec, pd, EV_A);
        for (auto it = eth_devices.begin(); it != eth_devices.end(); it++) {
            Eth *eth_dev = *it;
            eth_dev->SetType(type);
            devices[eth_dev->GetName()] = eth_dev;
        }

        // Create the host device
        if (eth_devices[0]->GetEthType() == ETH_HOST_MGMT ||
            eth_devices[0]->GetEthType() == ETH_HOST) {
            if (!eth_devices[0]->CreateHostDevice()) {
                NIC_LOG_ERR("{}: CreateHostDevice() failed", eth_devices[0]->GetName());
            }
        }

        break;
    }
#ifdef IRIS
    case ACCEL: {
        AccelDev *accel_dev = new AccelDev(dev_api, dev_spec, pd, EV_A);
        accel_dev->SetType(type);
        devices[accel_dev->GetName()] = accel_dev;
        break;
    }
    case NVME: {
        NvmeDev *nvme_dev = new NvmeDev(dev_api, dev_spec, pd, EV_A);
        nvme_dev->SetType(type);
        devices[nvme_dev->GetName()] = nvme_dev;
        break;
    }
    case VIRTIO: {
        VirtIODev *virtio_dev = new VirtIODev(dev_api, dev_spec, pd, EV_A);
        virtio_dev->SetType(type);
        devices[virtio_dev->GetName()] = virtio_dev;
        break;
    }
    case PCIESTRESS: {
        if (pciemgr) {
            pciehdevice_resources_t pres;
            static int instance;

            memset(&pres, 0, sizeof(pres));
            pres.type = PCIEHDEVICE_PCIESTRESS;
            snprintf(pres.pfres.name, sizeof(pres.pfres.name), "pciestress%d", instance++);
            int ret = pciemgr->add_devres(&pres);
            if (ret != 0) {
                NIC_LOG_ERR("pciestress add failed {}", ret);
            }
        }
    }
#endif // IRIS
#ifdef ATHENA
    case FTL: {
        FtlDev *ftl_dev = new FtlDev(dev_api, dev_spec, pd, EV_A);
        ftl_dev->SetType(type);
        devices[ftl_dev->GetName()] = ftl_dev;
        break;
    }
#endif
    default:
        break;
    }
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
DeviceManager::RestoreDevice(enum DeviceType type, void *dev_state)
{
    switch (type) {
    case ETH: {
        Eth *eth_dev = new Eth(dev_api, (struct EthDevInfo *)dev_state, pd, EV_A);
        eth_dev->SetType(ETH);
        devices[eth_dev->GetName()] = eth_dev;
        break;
    }
    default:
        break;
    }

    evutil_timer_start(EV_A_ &heartbeat_timer, DeviceManager::HeartbeatEventHandler, this, 0.0, HEARTBEAT_PERIOD_S);
    clock_gettime(CLOCK_MONOTONIC, &hb_last);

    upg_state = DEVICES_ACTIVE_STATE;
}

Device *
DeviceManager::GetDevice(std::string name)
{
    return devices[name];
}

Device *
DeviceManager::GetDeviceByLif(uint32_t lif_id)
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
            if (eth_dev->IsDevLif(lif_id)) {
                return dev;
            }
        }
    }

    return NULL;
}

void
DeviceManager::SetHalClient(devapi *dev_api)
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        enum DeviceType type = dev->GetType(); 
        switch (type) {
        case ETH: {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->SetHalClient(dev_api);
            break;
        }
#ifdef IRIS
       case ACCEL: {
           AccelDev *accel_dev = (AccelDev *)dev;
           accel_dev->SetHalClient(dev_api);
           break;
       }
       case NVME: {
           NvmeDev *nvme_dev = (NvmeDev *)dev;
           nvme_dev->SetHalClient(dev_api);
           break;
       }
       case VIRTIO: {
           VirtIODev *virtio_dev = (VirtIODev *)dev;
           virtio_dev->SetHalClient(dev_api);
           break;
       }
#endif // IRIS
#ifdef ATHENA
       case FTL: {
           FtlDev *ftl_dev = (FtlDev *)dev;
           ftl_dev->SetHalClient(dev_api);
           break;
        }
#endif //ATHENA
       default:
           NIC_LOG_ERR("Invalid device type {} in SetHalClient",
                       type);
       }
    }
}

void
DeviceManager::HalEventHandler(bool status)
{
    NIC_HEADER_TRACE("HAL Event");

    if (init_done) {
        return;
    }

    // Hal UP
    if (status && !init_done) {
        NIC_LOG_DEBUG("Hal UP: Initializing hal client and creating VRFs.");

        // Instantiate HAL client
        dev_api = devapi_init();
        dev_api->set_micro_seg_en(micro_seg_en);
        pd->update();

        if (!skip_hwinit) {
            for (auto it = uplinks.begin(); it != uplinks.end(); it++) {
                uplink_t *up = it->second;
                if (up->is_oob) {
                    dev_api->uplink_create(up->id, up->port, up->is_oob);
                }
            }
        }

        // Setting hal clients in all devices
        SetHalClient(dev_api);
    }
       
    // OOB bring UP first for NCSI
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
            if (eth_dev->GetEthType() == ETH_MNIC_OOB_MGMT) {
                eth_dev->HalEventHandler(status);
                break;
            }
        }
    }

    // Initialize SWM
    if (status && !init_done) {
        // Create uplinks
        if (!skip_hwinit) {
            for (auto it = uplinks.begin(); it != uplinks.end(); it++) {
                uplink_t *up = it->second;
                if (!up->is_oob) {
                    dev_api->uplink_create(up->id, up->port, up->is_oob);
                }
            }
            dev_api->swm_enable();
            // Create NCSI Channels for non-oob uplinks
            int cid = 0;
            for (auto it = uplinks.begin(); it != uplinks.end(); it++) {
                uplink_t *up = it->second;
                if (!up->is_oob) {
                    dev_api->swm_create_channel(cid++, up->port);
                }
            }
        }
        init_done = true;
    }

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        enum DeviceType type = dev->GetType();
        switch (type) {
        case ETH: {
            Eth *eth_dev = (Eth *)dev;
            if (eth_dev->GetEthType() != ETH_MNIC_OOB_MGMT) {
                eth_dev->HalEventHandler(status);
            }
            break;
        }
#ifdef IRIS
        case ACCEL: {
            AccelDev *accel_dev = (AccelDev *)dev;
            accel_dev->HalEventHandler(status);
            break;
        }
        case NVME: {
            NvmeDev *nvme_dev = (NvmeDev *)dev;
            nvme_dev->HalEventHandler(status);
            break;
        }
        case VIRTIO: {
            VirtIODev *virtio_dev = (VirtIODev *)dev;
            virtio_dev->HalEventHandler(status);
            break;
        }
#endif // IRIS
#ifdef ATHENA
        case FTL: {
            FtlDev *ftl_dev = (FtlDev *)dev;
            ftl_dev->HalEventHandler(status);
            break;
        }
#endif // ATHENA
        default:
            NIC_LOG_ERR("Invalid device type {} in HalEventHandler",
                       type); 
        }
    }
}

void
DeviceManager::SystemSpecEventHandler(bool micro_seg_en)
{
    if (this->micro_seg_en != micro_seg_en) {
        NIC_LOG_ERR("Micro-segmentation mode changed: {} > {}",
            this->micro_seg_en, micro_seg_en);
    }

    this->micro_seg_en = micro_seg_en;
    dev_api->set_micro_seg_en(micro_seg_en);
}

void
DeviceManager::swm_update(bool enable,
                          uint32_t port_num, uint32_t vlan, mac_t mac)
{
#if 0
    dev_api->swm_enable();
    dev_api->swm_set_port(port_num);
    dev_api->swm_add_mac(mac);
    dev_api->swm_add_vlan(vlan);
#endif
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
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->LinkEventHandler(evd);
        }
    }
}

void
DeviceManager::XcvrEventHandler(port_status_t *evd)
{
    NIC_HEADER_TRACE("Xcvr Event");

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->XcvrEventHandler(evd);
        }
    }
}

void
DeviceManager::HeartbeatEventHandler(void *obj)
{
    DeviceManager *devmgr = (DeviceManager *)obj;
    if (devmgr->Thread()) {
        // call thread level heartbeat
        devmgr->Thread()->punch_heartbeat();
    }
    timespec_t now, hb_delta;

    clock_gettime(CLOCK_MONOTONIC, &now);
    hb_delta = sdk::timestamp_diff(&now, &devmgr->hb_last);
    if (hb_delta.tv_sec >= HEARTBEAT_MAX_PERIOD_S) {
        NIC_LOG_WARN("Missed heartbeat for {} seconds", hb_delta.tv_sec);
    }

    devmgr->hb_last = now;
    for (auto it = devmgr->devices.begin(); it != devmgr->devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
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
        enum DeviceType type = dev->GetType();
        switch (type) {
        case ETH: {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->DelphiMountEventHandler(mounted);
            break;
        }
#ifdef IRIS
        case ACCEL: {
            AccelDev *accel_dev = (AccelDev *)dev;
            accel_dev->DelphiMountEventHandler(mounted);
            break;
        }
        case NVME: {
            NvmeDev *nvme_dev = (NvmeDev *)dev;
            nvme_dev->DelphiMountEventHandler(mounted);
            break;
        }
        case VIRTIO: {
            VirtIODev *virtio_dev = (VirtIODev *)dev;
            virtio_dev->DelphiMountEventHandler(mounted);
            break;
        }
#endif // IRIS
        default:
            NIC_LOG_ERR("Invalid device type {} in DelphiMountEventHandler",
                            type);
        }
    }
}

int
DeviceManager::GenerateQstateInfoJson(std::string qstate_info_file)
{
    pt::ptree root, lifs;

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
            eth_dev->GenerateQstateInfoJson(lifs);
        }
    }

    root.push_back(std::make_pair("lifs", lifs));
    pt::write_json(qstate_info_file, root);
    return 0;
}

static const char *
upgrade_state_to_str(UpgradeState state)
{
    switch (state) {
        CASE(UNKNOWN_STATE);
        CASE(DEVICES_ACTIVE_STATE);
        CASE(DEVICES_QUIESCED_STATE);
        CASE(DEVICES_RESET_STATE);
        default: return "invalid";
    }
}

static const char *
upgrade_event_to_str(UpgradeEvent event)
{
    switch (event) {
        CASE(UPG_EVENT_QUIESCE);
        CASE(UPG_EVENT_DEVICE_RESET);
        default: return "invalid";
    }
}

int
DeviceManager::HandleUpgradeEvent(UpgradeEvent event)
{
    NIC_LOG_DEBUG(upgrade_event_to_str(event));

    switch (event) {
        case UPG_EVENT_QUIESCE:
            for (auto it = devices.begin(); it != devices.end(); it++) {
                Device *dev = it->second;
                if (dev->GetType() == ETH) {
                    Eth *eth_dev = (Eth *) dev;
                    eth_dev->QuiesceEventHandler(true);
                }
            }
            break;
        case UPG_EVENT_DEVICE_RESET:
            if (upg_state != DEVICES_QUIESCED_STATE) {
                NIC_LOG_ERR("Invalid event {} in state {}",
                    upgrade_event_to_str(event),
                    upgrade_state_to_str(upg_state));
                break;
            }
            SendFWDownEvent();
            break;
        default:
            break;
    }

    return 0;
}

bool
DeviceManager::IsDataPathQuiesced()
{
    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
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
        if (dev->GetType() == ETH) {
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
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *) dev;
            eth_dev->SendFWDownEvent();
        }
    }

    return 0;
}


UpgradeState
DeviceManager::GetUpgradeState()
{
    switch (upg_state) {
        case DEVICES_ACTIVE_STATE:
            if (IsDataPathQuiesced()) {
                upg_state = DEVICES_QUIESCED_STATE;
            }
            break;
        case DEVICES_QUIESCED_STATE:
            if (CheckAllDevsDisabled()) {
                upg_state = DEVICES_RESET_STATE;
            }
            break;
        case DEVICES_RESET_STATE:
            //nothing to be done here
            break;
        default:
            NIC_LOG_DEBUG("Unsupported upgrade state {}", upg_state);
    }

    return upg_state;
}

std::vector<struct EthDevInfo *>
DeviceManager::GetEthDevStateInfo()
{
    std::vector<struct EthDevInfo *> eth_dev_info_list;

    for (auto it = devices.begin(); it != devices.end(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
            Eth *eth_dev = (Eth *)dev;
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
            NIC_LOG_ERR("Upgrade is not possible with non-ETH device {}. "
                        "compat check failed for Upgrade",
                        it->first);
            return false;
        } else {
            // If RDMA is enabled in ETH device then also upgrade is not feasible
            Eth *eth_dev = (Eth *)dev;
            struct EthDevInfo *info = new EthDevInfo();

            eth_dev->GetEthDevInfo(info);
            if (info->eth_spec->enable_rdma) {
                NIC_LOG_WARN("RDMA enabled ETH device {} will not function "
                             "after upgrade",
                             it->first);
                /*
                 * TODO: as of now we need to ignore RDMA check since
                 * smart-nic profile has RDMA default enabled
                 */
                // return false;
            }
        }
    }

    return true;
}
