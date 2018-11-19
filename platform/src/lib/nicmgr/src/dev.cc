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

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"

#include "logger.hpp"

#include "dev.hpp"
#include "pd_client.hpp"
#include "eth_dev.hpp"
#include "accel_dev.hpp"

namespace pt = boost::property_tree;

sdk::lib::indexer *intr_allocator = sdk::lib::indexer::factory(4096);
DeviceManager *DeviceManager::instance;
nicmgr_req_qstate_t qstate_req = { 0 };
nicmgr_resp_qstate_t qstate_resp = { 0 };

struct queue_info DeviceManager::qinfo [NUM_QUEUE_TYPES] = {
    [NICMGR_QTYPE_REQ] = {
            .type_num = NICMGR_QTYPE_REQ,
            .size = 2,
            .entries = 0,
            .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
            .prog = "txdma_stage0.bin",
            .label = "nicmgr_req_stage0",
            .qstate = (const char *)&qstate_req
    },
    [NICMGR_QTYPE_RESP] = {
            .type_num = NICMGR_QTYPE_RESP,
            .size = 2,
            .entries = 0,
            .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
            .prog = "txdma_stage0.bin",
            .label = "nicmgr_resp_stage0",
            .qstate = (const char *)&qstate_resp
    },
};

uint64_t
mac_to_int(std::string const& s)
{
    unsigned char m[6];
    int rc = sscanf(s.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                    m + 0, m + 1, m + 2, m + 3, m + 4, m + 5);

    if (rc != 6) {
        throw std::runtime_error("invalid mac address " + s);
    }

    return (
            uint64_t(m[0]) << 40 |
            uint64_t(m[1]) << 32 |
            uint64_t(m[2]) << 24 |
            uint64_t(m[3]) << 16 |
            uint64_t(m[4]) << 8 |
            uint64_t(m[5])
        );
}

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

void
invalidate_rxdma_cacheline(uint64_t addr)
{
    WRITE_REG32(CAP_ADDR_BASE_RPC_PICS_OFFSET +
              CAP_PICS_CSR_PICC_DHS_CACHE_INVALIDATE_BYTE_ADDRESS,
              ((addr >> 6) << 1));
}

void
invalidate_txdma_cacheline(uint64_t addr)
{
    WRITE_REG32(CAP_ADDR_BASE_TPC_PICS_OFFSET +
              CAP_PICS_CSR_PICC_DHS_CACHE_INVALIDATE_BYTE_ADDRESS,
              ((addr >> 6) << 1));
}

int
DeviceManager::lifs_reservation(platform_t platform)
{
    int ret;

    // Reserve hw_lif_id for uplinks which HAL will use from 1 - 32.
    NIC_LOG_INFO("Reserving 1-{} hw_lif_ids for HAL(uplinks) to use. Nicmgr will start from {}.",
                 (HAL_LIF_ID_NICMGR_MIN - 1),
                 HAL_LIF_ID_NICMGR_MIN);
    ret = pd->lm_->LIFRangeAlloc(1, (HAL_LIF_ID_NICMGR_MIN - 1));
    if (ret <= 0) {
        NIC_LOG_ERR("Unable to reserve 1-32 lifs for uplinks");
        return -1;
    }

    /*
     * Even though (HAL_LIF_ID_NICMGR_MIN - 1)  IDs were reserved locally above,
     * HAL in DOL mode only does the same when platform is HW/HAPS.
     * So to ensure the nicmgr LIF is assigned the correct hw_lif_id,
     * we need to create a bunch of throw aways HAL LIFs in the DOL case.
     */
    if (platform_is_hw(platform)) {
        return 0;
    }

    for (uint32_t i = 1; i < HAL_LIF_ID_NICMGR_MIN; i++) {
#if 0
        if (lif_allocator->alloc(&lif_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate reserved lif_id");
            return -1;
        }
#endif


        // TODO: Fix for storage test cases
#if 0
        struct lif_info linfo;
        memset(&linfo, 0, sizeof(linfo));
        linfo.hw_lif_id = i;
        linfo.lif_id = info.hw_lif_id;
        if (hal->LifCreate(lif_id,  NULL, empty_qinfo, &linfo)) {
            NIC_LOG_ERR("Failed to reserve LIF {} thru HAL LifCreate", lif_id);
            return -1;
        }
#endif
    }
    return 0;
}

void DeviceManager::CreateUplinkVRFs()
{
    Uplink *uplink = NULL;
    for (auto it = uplinks.cbegin(); it != uplinks.cend(); it++) {
        uplink = (Uplink *)(it->second);
        NIC_LOG_INFO("Creating VRF for uplink: {}", uplink->GetId());
        uplink->CreateVrf();
    }
}

void
DeviceManager::SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client)
{
    Device *dev = NULL;
    Eth *eth_dev = NULL;
    Accel_PF *acc_dev = NULL;

    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        dev = (Device *)(it->second);
        if (dev->GetType() == ETH) {
            eth_dev = (Eth *)dev;
            eth_dev->SetHalClient(hal_client, hal_cmn_client);
        }
        if (dev->GetType() == ACCEL) {
            acc_dev = (Accel_PF *)dev;
            acc_dev->SetHalClient(hal_client);
            acc_dev->LifInit();
        }
    }
}

void DeviceManager::Update()
{
    int32_t     cosA = 1;
    int32_t     cosB = 0;

    if (init_done) {
        return;
    }

    // instantiate HAL client
    hal = new HalClient(fwd_mode);
    hal_common_client = HalGRPCClient::Factory((HalForwardingMode)fwd_mode);
    pd->update();

    // Setting hal clients in all devices
    SetHalClient(hal, hal_common_client);

    // Create VRFs for uplinks
    NIC_LOG_INFO("Creating VRFs for uplinks");
    CreateUplinkVRFs();

    uint64_t ret = hal->LifCreate(&hal_lif_info_);
    if (ret != 0) {
        NIC_LOG_ERR("Failed to create Nicmgr service LIF. ret: {}", ret);
        return;
    }

    cosB = HalClient::GetTxTrafficClassCos("DEFAULT", 0);
    if (cosB < 0) {
        NIC_LOG_ERR("Service Lif: Failed to get cosB for group default");
        throw runtime_error("Failed to get cosB for nicmgr LIF");
    }
    uint8_t coses = (((cosB & 0x0f) << 4) | (cosA & 0x0f));
    pd->program_qstate(qinfo, &hal_lif_info_, coses);

    // Init QState
    uint64_t hbm_base = NICMGR_BASE;
    uint8_t tmp[sizeof(struct nicmgr_req_desc)] = { 0 };

    ring_size = 4096;
    req_ring_base = hbm_base;
    resp_ring_base = hbm_base + (sizeof(struct nicmgr_req_desc) * ring_size);

    NIC_LOG_INFO("nicmgr req qstate address {:#x}", hal_lif_info_.qstate_addr[NICMGR_QTYPE_REQ]);
    NIC_LOG_INFO("nicmgr resp qstate address {:#x}", hal_lif_info_.qstate_addr[NICMGR_QTYPE_RESP]);
    NIC_LOG_INFO("nicmgr req queue address {:#x}", req_ring_base);
    NIC_LOG_INFO("nicmgr resp queue address {:#x}", resp_ring_base);

    req_head = ring_size - 1;
    req_tail = 0;
    resp_head = 0;
    resp_tail = 0;

    invalidate_txdma_cacheline(hal_lif_info_.qstate_addr[NICMGR_QTYPE_REQ]);
    READ_MEM(hal_lif_info_.qstate_addr[NICMGR_QTYPE_REQ], (uint8_t *)&qstate_req, sizeof(qstate_req), 0);

    qstate_req.host = 1;
    qstate_req.total = 1;
    qstate_req.p_index0 = req_head;
    qstate_req.c_index0 = req_tail;
    qstate_req.comp_index = 0;
    qstate_req.ci_fetch = 0;
    qstate_req.sta.color = 1;
    qstate_req.cfg.enable = 1;
    qstate_req.ring_base = req_ring_base;
    qstate_req.ring_size = log2(ring_size);

    for (int i = 0; i < ring_size; i++) {
        WRITE_MEM(req_ring_base + (sizeof(struct nicmgr_req_desc) * i),
                  (uint8_t *)tmp, sizeof(tmp), 0);
    }

    WRITE_MEM(hal_lif_info_.qstate_addr[NICMGR_QTYPE_REQ], (uint8_t *)&qstate_req, sizeof(qstate_req), 0);
    invalidate_txdma_cacheline(hal_lif_info_.qstate_addr[NICMGR_QTYPE_REQ]);


    invalidate_txdma_cacheline(hal_lif_info_.qstate_addr[NICMGR_QTYPE_RESP]);
    READ_MEM(hal_lif_info_.qstate_addr[NICMGR_QTYPE_RESP], (uint8_t *)&qstate_resp, sizeof(qstate_resp), 0);

    qstate_resp.host = 1;
    qstate_resp.total = 1;
    qstate_resp.p_index0 = resp_head;
    qstate_resp.c_index0 = resp_tail;
    qstate_resp.comp_index = 0;
    qstate_resp.ci_fetch = 0;
    qstate_resp.sta.color = 1;
    qstate_resp.cfg.enable = 1;
    qstate_resp.ring_base = resp_ring_base;
    qstate_resp.ring_size = log2(ring_size);

    for (int i = 0; i < ring_size; i++) {
        WRITE_MEM(resp_ring_base + (sizeof(struct nicmgr_resp_desc) * i),
                  (uint8_t *)tmp, sizeof(tmp), 0);
    }
    WRITE_MEM(hal_lif_info_.qstate_addr[NICMGR_QTYPE_RESP], (uint8_t *)&qstate_resp, sizeof(qstate_resp), 0);

    invalidate_txdma_cacheline(hal_lif_info_.qstate_addr[NICMGR_QTYPE_RESP]);

    init_done = true;
}

void
devicemanager_init (void)
{
    DeviceManager::GetInstance()->Update();
}

DeviceManager::DeviceManager(std::string config_file, enum ForwardingMode fwd_mode,
                             platform_t platform, bool dol_integ) :
    dol_integ(dol_integ)
{
    uint64_t    hw_lif_id;

    NIC_HEADER_TRACE("Initializing DeviceManager");
    instance = this;
#ifdef __x86_64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) ==
               sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HAPS) ==
               sdk::lib::PAL_RET_OK);
#endif
    this->fwd_mode = fwd_mode;
    this->config_file = config_file;
    pd = PdClient::factory(platform);
    assert(pd);

    if (lifs_reservation(platform)) {
        throw runtime_error("Failed to reserve LIFs");
    }

    NIC_HEADER_TRACE("Admin Lif creation");
    memset(&hal_lif_info_, 0, sizeof(hal_lif_info_));
    hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
    hal_lif_info_.id = hw_lif_id;
    if (dol_integ) {
        struct lif_info info;

        /*
         * For DOL integration, allocate nicmgr LIF fully with HAL, i.e.,
         * complete with qstate initialization by the HAL. Since HAL has
         * so far kept in lock step with nicmgr regarding hw_lif_id's,
         * the next id that HAL returns should match.
         */
        hal_lif_info_.hw_lif_id = 0;
        if (hal->LifCreate(hal_lif_info_.id, qinfo, &info,
                           0, false, 0, 0, 0, 0) == 0) {
            throw runtime_error("Failed to create HAL nicmgr LIF");
        }
        hal_lif_info_.hw_lif_id = info.hw_lif_id;
    } else {
        hal_lif_info_.hw_lif_id = hw_lif_id;
        hal_lif_info_.pinned_uplink = NULL;
        hal_lif_info_.enable_rdma = false;
        memcpy(hal_lif_info_.queue_info, qinfo,
               sizeof(hal_lif_info_.queue_info));
    }
    NIC_LOG_INFO("nicmgr lif id:{}, hw_lif_id: {}", hal_lif_info_.id, hal_lif_info_.hw_lif_id);
}

DeviceManager::~DeviceManager()
{
}

int
DeviceManager::LoadConfig(string path)
{
    struct eth_devspec *eth_spec;
    struct accel_devspec *accel_spec;
    uint32_t intr_base = 0;
    char *system_uuid = getenv("SYSUUID");
    uint64_t sys_mac_base;

    NIC_HEADER_TRACE("Loading Config");
    NIC_LOG_INFO("Json: {}", path);


    boost::property_tree::read_json(path, spec);
    if (!system_uuid || !strcmp(system_uuid, "")) {
        sys_mac_base = 0x00DEADBEEF00llu;
    } else {
        sys_mac_base = mac_to_int(system_uuid);
    }

    NIC_LOG_INFO("Entered SysUuid={} SysMacBase={}",
                 system_uuid, sys_mac_base);


    // Create Network
    if (spec.get_child_optional("network")) {
        // Create Uplinks
        if (spec.get_child_optional("network.uplink")) {
            for (const auto &node : spec.get_child("network.uplink")) {
                auto val = node.second;
                NIC_LOG_INFO("Creating uplink: {}, oob: {}",
                             val.get<uint64_t>("id"),
                             val.get<bool>("oob", false));
                Uplink *up1 = Uplink::Factory(val.get<uint64_t>("id"), val.get<bool>("oob", false));
                uplinks[val.get<uint64_t>("id")] = up1;
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

            eth_spec->dev_uuid     = val.get<uint64_t>("dev_uuid");
            eth_spec->rxq_count    = val.get<uint64_t>("rxq_count");
            eth_spec->txq_count    = val.get<uint64_t>("txq_count");
            eth_spec->eq_count     = val.get<uint64_t>("eq_count");
            eth_spec->adminq_count = val.get<uint64_t>("adminq_count");
            eth_spec->intr_count   = val.get<uint64_t>("intr_count");
            if (intr_allocator->alloc_block(&intr_base, eth_spec->intr_count) != sdk::lib::indexer::SUCCESS) {
                NIC_LOG_ERR("lif{}: Failed to allocate interrupts", hal_lif_info_.hw_lif_id);
                return -1;
            }
            eth_spec->intr_base    = intr_base;
            eth_spec->mac_addr     = sys_mac_base++;
            eth_spec->hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
            eth_spec->lif_id = eth_spec->hw_lif_id;
            if (val.get_optional<string>("network")) {
                eth_spec->uplink_id = val.get<uint64_t>("network.uplink");
                eth_spec->uplink = uplinks[eth_spec->uplink_id];
                if (eth_spec->uplink == NULL) {
                    NIC_LOG_ERR("Unable to find uplink for id: {}", eth_spec->uplink_id);
                }
            }

            eth_spec->host_dev = false;
            if (val.get_optional<string>("type")) {
                eth_spec->eth_type = eth_dev_type_str_to_type(val.get<string>("type"));
            } else {
                eth_spec->eth_type = ETH_UNKNOWN;
            }
            eth_spec->if_name = val.get<string>("name");
            eth_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
            NIC_LOG_INFO("Creating mnic device with name: {} type: {}, lif_id: {}, hw_lif_id: {},"
                         "pinned_uplink: {} intr_count: {} qos_group {}",
                         eth_spec->if_name,
                         eth_dev_type_to_str(eth_spec->eth_type),
                         eth_spec->lif_id,
                         eth_spec->hw_lif_id,
                         eth_spec->uplink_id,
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

            eth_spec->dev_uuid = val.get<uint64_t>("dev_uuid");
            eth_spec->rxq_count = val.get<uint64_t>("rxq_count");
            eth_spec->txq_count = val.get<uint64_t>("txq_count");
            eth_spec->eq_count = val.get<uint64_t>("eq_count");
            eth_spec->adminq_count = val.get<uint64_t>("adminq_count");
            eth_spec->intr_count = val.get<uint64_t>("intr_count");
            if (intr_allocator->alloc_block(&intr_base, eth_spec->intr_count) != sdk::lib::indexer::SUCCESS) {
                NIC_LOG_ERR("lif{}: Failed to allocate interrupts", hal_lif_info_.hw_lif_id);
                return -1;
            }
            eth_spec->intr_base = intr_base;
            eth_spec->mac_addr = sys_mac_base++;

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

            eth_spec->hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
            eth_spec->lif_id = eth_spec->hw_lif_id;
            if (val.get_optional<string>("network")) {
                eth_spec->uplink_id = val.get<uint64_t>("network.uplink");
                eth_spec->uplink = uplinks[eth_spec->uplink_id];
                if (eth_spec->uplink == NULL) {
                    NIC_LOG_ERR("Unable to find uplink for id: {}", eth_spec->uplink_id);
                }
            }

            eth_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
            eth_spec->host_dev = true;
            if (val.get_optional<string>("type")) {
                eth_spec->eth_type = eth_dev_type_str_to_type(val.get<string>("type"));
            } else {
                eth_spec->eth_type = ETH_UNKNOWN;
            }
            eth_spec->if_name = val.get<string>("name");
            eth_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
            NIC_LOG_INFO("Creating eth device with name: {}, type: {}, lif_id: {}, hw_lif_id: {}, "
                         "pinned_uplink: {}, qos_group {}",
                         eth_spec->if_name,
                         eth_dev_type_to_str(eth_spec->eth_type),
                         eth_spec->lif_id,
                         eth_spec->hw_lif_id,
                         eth_spec->uplink_id,
                         eth_spec->qos_group);
            AddDevice(ETH, (void *)eth_spec);
        }
    }

    NIC_HEADER_TRACE("Loading Accel devices");
    // Create Accelerator devices
    if (spec.get_child_optional("accel_dev")) {
        NIC_LOG_INFO("Creating accel device");
        for (const auto &node : spec.get_child("accel_dev")) {
            NIC_LOG_INFO("Creating accel device");
            accel_spec = new struct accel_devspec;
            memset(accel_spec, 0, sizeof(*accel_spec));

            auto val = node.second;

            accel_spec->hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
            if (dol_integ) {
                    accel_spec->lif_id = STORAGE_SEQ_SW_LIF_ID;
            } else {
                accel_spec->lif_id = accel_spec->hw_lif_id;
            }
            accel_spec->seq_queue_count = val.get<uint32_t>("seq_queue_count");
            accel_spec->adminq_count = val.get<uint32_t>("adminq_count");
            accel_spec->intr_count = val.get<uint32_t>("intr_count");
            if (intr_allocator->alloc_block(&intr_base, accel_spec->intr_count) != sdk::lib::indexer::SUCCESS) {
                NIC_LOG_ERR("Accel lif: Failed to allocate interrupts");
                return -1;
            }
            accel_spec->intr_base = intr_base;

            accel_spec->pub_intv_frac = ACCEL_DEV_PUB_INTV_FRAC_DFLT;
            if (val.get_optional<string>("publish_interval")) {
                accel_spec->pub_intv_frac = val.get<uint32_t>("publish_interval.sec_fraction");
            }

            accel_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
            accel_spec->qos_group = val.get<string>("qos_group", "DEFAULT");
            NIC_LOG_INFO("Creating accel device with lif_id: {}, hw_lif_id: {} "
                         "qos_group {}",
                         accel_spec->lif_id,
                         accel_spec->hw_lif_id,
                         accel_spec->qos_group);
            AddDevice(ACCEL, (void *)accel_spec);
        }
    }

    return 0;
}

Device *
DeviceManager::GetDevice(uint64_t id)
{
    return devices[id];
}

Device *
DeviceManager::AddDevice(enum DeviceType type, void *dev_spec)
{
    Eth *eth_dev;
    Accel_PF *accel_dev;

    switch (type) {
    case MNIC:
        NIC_LOG_ERR("Unsupported Device Type MNIC");
        return NULL;
    case DEBUG:
        NIC_LOG_ERR("Unsupported Device Type DEBUG");
        return NULL;
    case ETH:
        eth_dev = new Eth(hal, hal_common_client, dev_spec, &hal_lif_info_, pd);
        eth_dev->SetType(type);
        devices[eth_dev->GetHalLifInfo()->hw_lif_id] = (Device *)eth_dev;
        return (Device *)eth_dev;
    case ACCEL:
        accel_dev = new Accel_PF(hal, dev_spec, &hal_lif_info_, pd, dol_integ);
        accel_dev->SetType(type);
        devices[accel_dev->info.hw_lif_id] = (Device *)accel_dev;
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

void
DeviceManager::CreateMnets()
{
    Device *dev = NULL;
    Eth *eth_dev = NULL;

    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        dev = (Device *)(it->second);
        if (dev->GetType() == ETH) {
            eth_dev = (Eth *)dev;
            if (eth_dev->isMnic()) {
                eth_dev->CreateMnet();
            }
        }
    }
}

#ifdef __aarch64__
void
DeviceManager::PcieEventHandler(const pciehdev_eventdata_t *evd)
{
    Device *dev = (Device *)pciehdev_get_priv(evd->pdev);
    dev->DevcmdHandler();
}
#endif

void
DeviceManager::DevcmdPoll()
{
    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        it->second->DevcmdPoll();
    }
}

void
DeviceManager::DevLinkDownHandler(uint32_t port_num)
{
    Device *dev;
    Eth *eth_dev;

    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        dev = it->second;
        eth_dev = (Eth*) dev;
        if (eth_dev && eth_dev->uplink_id == port_num) {
            eth_dev->DevLinkDownHandler(port_num);
        }
    }
}


void
DeviceManager::DevLinkUpHandler(uint32_t port_num)
{
    Device *dev;
    Eth *eth_dev;

    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        dev = it->second;
        eth_dev = (Eth*) dev;
        if (eth_dev && eth_dev->uplink_id == port_num) {
            eth_dev->DevLinkUpHandler(port_num);
        }
    }
}

void
DeviceManager::AdminQPoll()
{
    uint16_t p_index0 = 0, c_index0 = 0;

    uint64_t req_desc_addr = 0, resp_desc_addr = 0;
    struct nicmgr_req_desc req_desc = { 0 };
    uint8_t req_data[4096] = { 0 };
    struct nicmgr_resp_desc resp_desc = { 0 };
    uint8_t resp_data[4096] = { 0 };

    uint64_t req_qstate_addr = hal_lif_info_.qstate_addr[NICMGR_QTYPE_REQ];
    uint64_t req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0x8 /* PI_UPD + UPD_NOP */ << 17) +
                (hal_lif_info_.hw_lif_id << 6) +
                (NICMGR_QTYPE_REQ << 3);
    uint64_t req_db_data = 0x0;

    uint64_t resp_qstate_addr = hal_lif_info_.qstate_addr[NICMGR_QTYPE_RESP];
    uint64_t resp_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0x9 /* PI_UPD + UPD_EVAL */ << 17) +
                (hal_lif_info_.hw_lif_id << 6) +
                (NICMGR_QTYPE_RESP << 3);
    uint64_t resp_db_data = 0x0;

    invalidate_txdma_cacheline(req_qstate_addr);

    READ_MEM(req_qstate_addr + offsetof(admin_qstate_t, p_index0),
             (uint8_t *)&p_index0, sizeof(p_index0), 0);

    READ_MEM(req_qstate_addr + offsetof(admin_qstate_t, c_index0),
             (uint8_t *)&c_index0, sizeof(c_index0), 0);

    if (req_tail != c_index0) {

        NIC_LOG_INFO("request: PRE: p_index0 {}, c_index0 {}, head {}, tail {}",
               p_index0, c_index0, req_head, req_tail);

        // Read nicmgr request descriptor
        req_desc_addr = req_ring_base + (sizeof(req_desc) * req_tail);
        READ_MEM(req_desc_addr, (uint8_t *)&req_desc, sizeof(req_desc), 0);

        NIC_LOG_INFO("request: lif {} qtype {} qid {} comp_index {}"
               " adminq_qstate_addr {:#x} desc_addr {:#x}",
               req_desc.lif, req_desc.qtype, req_desc.qid,
               req_desc.comp_index, req_desc.adminq_qstate_addr,
               req_desc_addr);

        // Process devcmd
        Device *dev = (Device *)devices[req_desc.lif];
        if (dev == NULL) {
            NIC_LOG_ERR("Invalid AdminQ request for lif {}!", req_desc.lif);
        } else {
            dev->CmdHandler(&req_desc.cmd, (void *)&req_data,
                &resp_desc.comp, (void *)&resp_data);
        }

        // Ring doorbell to update the PI
        req_head = (req_head + 1) & (ring_size - 1);
        req_tail = (req_tail + 1) & (ring_size - 1);
        req_db_data = req_head;
        NIC_LOG_INFO("req_db_addr {:#x} req_db_data {:#x}", req_db_addr, req_db_data);
        WRITE_DB64(req_db_addr, req_db_data);

        invalidate_txdma_cacheline(req_qstate_addr);

        READ_MEM(req_qstate_addr + offsetof(admin_qstate_t, p_index0),
                 (uint8_t *)&p_index0, sizeof(p_index0), 0);

        READ_MEM(req_qstate_addr + offsetof(admin_qstate_t, c_index0),
                 (uint8_t *)&c_index0, sizeof(c_index0), 0);

        NIC_LOG_INFO("request: POST: p_index0 {}, c_index0 {}, head {}, tail {}",
               p_index0, c_index0, req_head, req_tail);

        // Write nicmgr response descriptor
        invalidate_txdma_cacheline(resp_qstate_addr);

        READ_MEM(resp_qstate_addr + offsetof(admin_qstate_t, p_index0),
                 (uint8_t *)&p_index0, sizeof(p_index0), 0);

        READ_MEM(resp_qstate_addr + offsetof(admin_qstate_t, c_index0),
                 (uint8_t *)&c_index0, sizeof(c_index0), 0);

        NIC_LOG_INFO("response: PRE: p_index0 {}, c_index0 {}, head {}, tail {}",
               p_index0, c_index0, resp_head, resp_tail);

        resp_desc_addr = resp_ring_base + (sizeof(resp_desc) * resp_tail);

        resp_desc.lif = req_desc.lif;
        resp_desc.qtype = req_desc.qtype;
        resp_desc.qid = req_desc.qid;
        resp_desc.comp_index = req_desc.comp_index;
        resp_desc.adminq_qstate_addr = req_desc.adminq_qstate_addr;

        NIC_LOG_INFO("response: lif {} qtype {} qid {} comp_index {}"
               " adminq_qstate_addr {:#x} desc_addr {:#x}",
               resp_desc.lif, resp_desc.qtype, resp_desc.qid,
               resp_desc.comp_index, resp_desc.adminq_qstate_addr,
               resp_desc_addr);

        WRITE_MEM(resp_desc_addr, (uint8_t *)&resp_desc, sizeof(resp_desc), 0);

        // Ring doorbell to update the PI and run nicmgr response program
        resp_tail = (resp_tail + 1) & (ring_size - 1);
        resp_db_data = resp_tail;
        NIC_LOG_INFO("resp_db_addr {:#x} resp_db_data {:#x}", resp_db_addr, resp_db_data);
        WRITE_DB64(resp_db_addr, resp_db_data);

        invalidate_txdma_cacheline(resp_qstate_addr);

        READ_MEM(resp_qstate_addr + offsetof(admin_qstate_t, p_index0),
                 (uint8_t *)&p_index0, sizeof(p_index0), 0);

        READ_MEM(resp_qstate_addr + offsetof(admin_qstate_t, c_index0),
                 (uint8_t *)&c_index0, sizeof(c_index0), 0);

        NIC_LOG_INFO("response: POST: p_index0 {}, c_index0 {}, head {}, tail {}",
               p_index0, c_index0, resp_head, resp_tail);
    }
}

int
DeviceManager::DumpQstateInfo(pt::ptree &lifs)
{
    pt::ptree lif;
    pt::ptree qstates;

    NIC_LOG_INFO("lif-{}: Qstate Info to Json", hal_lif_info_.hw_lif_id);

    lif.put("lif_id", hal_lif_info_.id);
    lif.put("hw_lif_id", hal_lif_info_.hw_lif_id);

    for (int j = 0; j < NUM_QUEUE_TYPES; j++) {
        pt::ptree qs;
        char numbuf[32];

        if (qinfo[j].size < 1) continue;

        qs.put("qtype", qinfo[j].type_num);
        qs.put("qsize", qinfo[j].size);
        qs.put("qaddr", hal_lif_info_.qstate_addr[qinfo[j].type_num]);
        snprintf(numbuf, sizeof(numbuf), "0x%lx", hal_lif_info_.qstate_addr[qinfo[j].type_num]);
        qs.put("qaddr_hex", numbuf);
        qstates.push_back(std::make_pair("", qs));
        qs.clear();
    }

    lif.add_child("qstates", qstates);
    lifs.push_back(std::make_pair("", lif));
    qstates.clear();
    lif.clear();
    return 0;
}

int
DeviceManager::GenerateQstateInfoJson(std::string qstate_info_file)
{
    pt::ptree root, lifs;

    NIC_LOG_INFO("{}: file: {}", __FUNCTION__, qstate_info_file);

    DumpQstateInfo(lifs);
    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        Device *dev = it->second;
        Eth *eth_dev = (Eth *) dev;
        eth_dev->GenerateQstateInfoJson(lifs);
    }

    root.push_back(std::make_pair("lifs", lifs));
    pt::write_json(qstate_info_file, root);
    return 0;
}

const char *eth_dev_type_to_str(EthDevType type) {
    switch(type) {
        case ETH_UNKNOWN: return "ETH_UNKNOWN";
        case ETH_HOST: return "ETH_HOST";
        case ETH_HOST_MGMT: return "ETH_HOST_MGMT";
        case ETH_MNIC_OOB_MGMT: return "ETH_MNIC_OOB_MGMT";
        case ETH_MNIC_INTERNAL_MGMT: return "ETH_MNIC_INTERNAL_MGMT";
        case ETH_MNIC_INBAND_MGMT: return "ETH_MNIC_INBAND_MGMT";
        default: return "Unknown";
    }
}
