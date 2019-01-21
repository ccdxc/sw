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

#include "nic/sdk/platform/fru/fru.hpp"
#include "platform/src/lib/misc/include/maclib.h"

#include "logger.hpp"

#include "dev.hpp"
#include "pd_client.hpp"
#include "eth_dev.hpp"
#include "accel_dev.hpp"

namespace pt = boost::property_tree;

DeviceManager *DeviceManager::instance;

struct queue_info DeviceManager::qinfo [NUM_QUEUE_TYPES] = {
    [NICMGR_QTYPE_REQ] = {
        .type_num = NICMGR_QTYPE_REQ,
        .size = 2,
        .entries = 0,
    },
    [NICMGR_QTYPE_RESP] = {
        .type_num = NICMGR_QTYPE_RESP,
        .size = 2,
        .entries = 0,
    },
};

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

const char
*eth_dev_type_to_str(EthDevType type)
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

const char *oprom_type_to_str(OpromType type)
{
    switch(type) {
        CASE(OPROM_UNKNOWN);
        CASE(OPROM_LEGACY);
        CASE(OPROM_UEFI);
        CASE(OPROM_UNIFIED);
        default: return "unknown";
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

void DeviceManager::CreateUplinkVRFs()
{
    Uplink::CreateVrfs();
}

void
DeviceManager::SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client)
{
    Device *dev = NULL;
    Eth *eth_dev = NULL;
    Accel_PF *accel_dev = NULL;

    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        dev = (Device *)(it->second);
        if (dev->GetType() == ETH) {
            eth_dev = (Eth *)dev;
            eth_dev->SetHalClient(hal_client, hal_cmn_client);
            eth_dev->HalEventHandler(true);
        }
        if (dev->GetType() == ACCEL) {
            accel_dev = (Accel_PF *)dev;
            accel_dev->SetHalClient(hal_client, hal_cmn_client);
            accel_dev->HalEventHandler(true);
        }
    }
}

void DeviceManager::HalEventHandler(bool is_up)
{
    if (!is_up) {
        NIC_LOG_INFO("HAL is {} UP", is_up ? "" : "not");
        return;
    }

    NIC_HEADER_TRACE("HAL Event");

    int32_t     cosA = 1;
    int32_t     cosB = 0;
    uint8_t     off;

    if (init_done) {
        return;
    }

    // instantiate HAL client
    hal = new HalClient(fwd_mode);
    hal_common_client = HalGRPCClient::Factory((HalForwardingMode)fwd_mode);
    pd->update();

    // Create VRFs for uplinks
    NIC_LOG_DEBUG("Creating VRFs for uplinks");
    CreateUplinkVRFs();

    uint64_t ret = hal->LifCreate(&hal_lif_info_);
    if (ret != 0) {
        NIC_LOG_ERR("Failed to create Nicmgr service LIF. ret: {}", ret);
        return;
    }

    cosB = QosClass::GetTxTrafficClassCos("DEFAULT", 0);
    if (cosB < 0) {
        NIC_LOG_ERR("lif{}: Failed to get cosB for group default",
            hal_lif_info_.hw_lif_id);
        throw runtime_error("Failed to get cosB for nicmgr LIF");
    }
    uint8_t coses = (((cosB & 0x0f) << 4) | (cosA & 0x0f));
    pd->program_qstate(qinfo, &hal_lif_info_, coses);

    // Init QState
    uint8_t tmp[sizeof(struct nicmgr_req_desc)] = { 0 };

    ring_size = 4096;
    req_ring_base = pd->nicmgr_mem_alloc(sizeof(struct nicmgr_req_desc) * ring_size);
    resp_ring_base = pd->nicmgr_mem_alloc(sizeof(struct nicmgr_resp_desc) * ring_size);

    NIC_LOG_DEBUG("nicmgr req qstate address {:#x}", hal_lif_info_.qstate_addr[NICMGR_QTYPE_REQ]);
    NIC_LOG_DEBUG("nicmgr resp qstate address {:#x}", hal_lif_info_.qstate_addr[NICMGR_QTYPE_RESP]);
    NIC_LOG_DEBUG("nicmgr req queue address {:#x}", req_ring_base);
    NIC_LOG_DEBUG("nicmgr resp queue address {:#x}", resp_ring_base);

    // Init Request Queue
    nicmgr_req_qstate_t qstate_req = {0};
    req_head = ring_size - 1;
    req_tail = 0;

    if (pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "nicmgr_req_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: nicmgr_req_stage0");
        throw runtime_error("Failed to resolve program/label");
    }
    qstate_req.pc_offset = off;
    qstate_req.cos_sel = 0;
    qstate_req.cosA = 0;
    qstate_req.cosB = cosB;
    qstate_req.host = 0;
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

    // Init Response Queue
    nicmgr_resp_qstate_t qstate_resp = {0};
    resp_head = 0;
    resp_tail = 0;

    if (pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "nicmgr_resp_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: nicmgr_resp_stage0");
        throw runtime_error("Failed to resolve program/label");
    }
    qstate_resp.pc_offset = off;
    qstate_resp.cos_sel = 0;
    qstate_resp.cosA = 0;
    qstate_resp.cosB = cosB;
    qstate_resp.host = 0;
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

    // Setting hal clients in all devices
    SetHalClient(hal, hal_common_client);

    evutil_timer_start(&adminq_timer, DeviceManager::AdminQPoll, this, 0.01, 0.01);

    init_done = true;
}

DeviceManager::DeviceManager(std::string config_file, enum ForwardingMode fwd_mode,
                             platform_t platform)
{
    uint64_t    hw_lif_id;

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
    int ret = pd->lm_->LIFRangeAlloc(HAL_LIF_ID_MIN, HAL_LIF_ID_MAX);
    if (ret < 0) {
        throw runtime_error("Failed to reserve HAL LIFs");
    }

    NIC_HEADER_TRACE("Admin Lif creation");
    memset(&hal_lif_info_, 0, sizeof(hal_lif_info_));
    hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
    hal_lif_info_.name = "admin";
    hal_lif_info_.hw_lif_id = hw_lif_id;
    hal_lif_info_.pinned_uplink_port_num = 0;
    hal_lif_info_.enable_rdma = false;
    memcpy(hal_lif_info_.queue_info, qinfo, sizeof(hal_lif_info_.queue_info));
    NIC_LOG_DEBUG("nicmgr hw_lif_id: {}", hal_lif_info_.hw_lif_id);
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
    uint64_t fru_mac = 0;
    uint64_t cfg_mac = 0;
    string mac_str;
#ifdef __aarch64__
    if (readKey(MACADDRESS_KEY, mac_str) == 0) {
        mac_from_str(&fru_mac, mac_str.c_str());
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

    char sys_mac_str[32] = {0};
    NIC_LOG_INFO("Base mac address {} {}", sys_mac_str, mac_to_str(&sys_mac_base, sys_mac_str, sizeof(sys_mac_str)));

    // Create Network
    if (spec.get_child_optional("network")) {
        // Create Uplinks
        if (spec.get_child_optional("network.uplink")) {
            for (const auto &node : spec.get_child("network.uplink")) {
                auto val = node.second;
                NIC_LOG_DEBUG("Creating uplink: {}, oob: {}",
                             val.get<uint64_t>("id"),
                             val.get<bool>("oob", false));
                Uplink::Factory(val.get<uint64_t>("id"),
                                val.get<uint64_t>("port"),
                                val.get<bool>("oob", false));
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
            eth_spec->mac_addr = sys_mac_base++;

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
        accel_dev = new Accel_PF(hal, dev_spec, &hal_lif_info_, pd);
        accel_dev->SetType(type);
        devices[accel_dev->GetHalLifInfo()->hw_lif_id] = (Device *)accel_dev;
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
DeviceManager::LinkEventHandler(port_status_t *evd)
{
    Device *dev;
    Eth *eth_dev;

    NIC_HEADER_TRACE("Link Event");
    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        dev = it->second;
        if (dev->GetType() == ETH) {
            eth_dev = (Eth*) dev;
            eth_dev->LinkEventHandler(evd);
        }
    }
}

void
DeviceManager::AdminQPoll(void *obj)
{
    DeviceManager *devmgr = (DeviceManager *)obj;
    Device *dev = NULL;
    bool req_error = false;

    uint16_t p_index0 = 0, c_index0 = 0;

    uint64_t req_desc_addr = 0, resp_desc_addr = 0;
    struct nicmgr_req_desc req_desc = { 0 };
    uint8_t req_data[4096] = { 0 };
    struct nicmgr_resp_desc resp_desc = { 0 };
    uint8_t resp_data[4096] = { 0 };

    uint64_t req_qstate_addr = devmgr->hal_lif_info_.qstate_addr[NICMGR_QTYPE_REQ];
    uint64_t req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1000 /* PI_UPD + NO_SCHED */ << 17) +
                (devmgr->hal_lif_info_.hw_lif_id << 6) +
                (NICMGR_QTYPE_REQ << 3);
    uint64_t req_db_data = 0x0;

    invalidate_txdma_cacheline(req_qstate_addr);

    READ_MEM(req_qstate_addr + offsetof(admin_qstate_t, p_index0),
             (uint8_t *)&p_index0, sizeof(p_index0), 0);

    READ_MEM(req_qstate_addr + offsetof(admin_qstate_t, c_index0),
             (uint8_t *)&c_index0, sizeof(c_index0), 0);

    if (devmgr->req_tail != c_index0) {

        NIC_HEADER_TRACE("AdminCmd");

        NIC_LOG_DEBUG("request: PRE: p_index0 {}, c_index0 {}, head {}, tail {}",
               p_index0, c_index0, devmgr->req_head, devmgr->req_tail);

        // Read nicmgr request descriptor
        req_desc_addr = devmgr->req_ring_base + (sizeof(req_desc) * devmgr->req_tail);
        READ_MEM(req_desc_addr, (uint8_t *)&req_desc, sizeof(req_desc), 0);

        NIC_LOG_DEBUG("request: lif {} qtype {} qid {} comp_index {}"
               " adminq_qstate_addr {:#x} desc_addr {:#x}",
               req_desc.lif, req_desc.qtype, req_desc.qid,
               req_desc.comp_index, req_desc.adminq_qstate_addr,
               req_desc_addr);

        // Process devcmd
        if (devmgr->devices.find(req_desc.lif) == devmgr->devices.cend()) {
            req_error = true;
            NIC_LOG_ERR("Invalid AdminQ request! lif {} qtype {} qid {}",
                req_desc.lif, req_desc.qtype, req_desc.qid);
        } else {
            req_error = false;
            dev = devmgr->devices[req_desc.lif];
            dev->CmdHandler(&req_desc.cmd, (void *)&req_data,
                &resp_desc.comp, (void *)&resp_data);
        }

        // Ring doorbell to update the PI
        devmgr->req_head = (devmgr->req_head + 1) & (devmgr->ring_size - 1);
        devmgr->req_tail = (devmgr->req_tail + 1) & (devmgr->ring_size - 1);
        req_db_data = devmgr->req_head;
        NIC_LOG_DEBUG("req_db_addr {:#x} req_db_data {:#x}", req_db_addr, req_db_data);
        WRITE_DB64(req_db_addr, req_db_data);

        invalidate_txdma_cacheline(req_qstate_addr);

        READ_MEM(req_qstate_addr + offsetof(admin_qstate_t, p_index0),
                 (uint8_t *)&p_index0, sizeof(p_index0), 0);

        READ_MEM(req_qstate_addr + offsetof(admin_qstate_t, c_index0),
                 (uint8_t *)&c_index0, sizeof(c_index0), 0);

        NIC_LOG_DEBUG("request: POST: p_index0 {}, c_index0 {}, head {}, tail {}",
               p_index0, c_index0, devmgr->req_head, devmgr->req_tail);

        if (!req_error) {

            uint64_t resp_qstate_addr = devmgr->hal_lif_info_.qstate_addr[NICMGR_QTYPE_RESP];
            uint64_t resp_db_addr =
        #ifdef __aarch64__
                        CAP_ADDR_BASE_DB_WA_OFFSET +
        #endif
                        CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                        (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                        (devmgr->hal_lif_info_.hw_lif_id << 6) +
                        (NICMGR_QTYPE_RESP << 3);
            uint64_t resp_db_data = 0x0;

            invalidate_txdma_cacheline(resp_qstate_addr);

            // Write nicmgr response descriptor
            READ_MEM(resp_qstate_addr + offsetof(admin_qstate_t, p_index0),
                    (uint8_t *)&p_index0, sizeof(p_index0), 0);

            READ_MEM(resp_qstate_addr + offsetof(admin_qstate_t, c_index0),
                    (uint8_t *)&c_index0, sizeof(c_index0), 0);

            NIC_LOG_DEBUG("response: PRE: p_index0 {}, c_index0 {}, head {}, tail {}",
                p_index0, c_index0, devmgr->resp_head, devmgr->resp_tail);

            resp_desc_addr = devmgr->resp_ring_base + (sizeof(resp_desc) * devmgr->resp_tail);

            resp_desc.lif = req_desc.lif;
            resp_desc.qtype = req_desc.qtype;
            resp_desc.qid = req_desc.qid;
            resp_desc.comp_index = req_desc.comp_index;
            resp_desc.adminq_qstate_addr = req_desc.adminq_qstate_addr;

            NIC_LOG_DEBUG("response: lif {} qtype {} qid {} comp_index {}"
                " adminq_qstate_addr {:#x} desc_addr {:#x}",
                resp_desc.lif, resp_desc.qtype, resp_desc.qid,
                resp_desc.comp_index, resp_desc.adminq_qstate_addr,
                resp_desc_addr);

            WRITE_MEM(resp_desc_addr, (uint8_t *)&resp_desc, sizeof(resp_desc), 0);

            // Ring doorbell to update the PI and run nicmgr response program
            devmgr->resp_tail = (devmgr->resp_tail + 1) & (devmgr->ring_size - 1);
            resp_db_data = devmgr->resp_tail;
            NIC_LOG_DEBUG("resp_db_addr {:#x} resp_db_data {:#x}", resp_db_addr, resp_db_data);
            WRITE_DB64(resp_db_addr, resp_db_data);

            invalidate_txdma_cacheline(resp_qstate_addr);

            READ_MEM(resp_qstate_addr + offsetof(admin_qstate_t, p_index0),
                    (uint8_t *)&p_index0, sizeof(p_index0), 0);

            READ_MEM(resp_qstate_addr + offsetof(admin_qstate_t, c_index0),
                    (uint8_t *)&c_index0, sizeof(c_index0), 0);

            NIC_LOG_DEBUG("response: POST: p_index0 {}, c_index0 {}, head {}, tail {}",
                p_index0, c_index0, devmgr->resp_head, devmgr->resp_tail);
        }
    }
}

int
DeviceManager::DumpQstateInfo(pt::ptree &lifs)
{
    pt::ptree lif;
    pt::ptree qstates;

    NIC_LOG_DEBUG("lif-{}: Qstate Info to Json", hal_lif_info_.hw_lif_id);

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

    NIC_LOG_DEBUG("{}: file: {}", __FUNCTION__, qstate_info_file);

    DumpQstateInfo(lifs);
    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        Device *dev = it->second;
        if (dev->GetType() == ETH) {
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
    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        Device *dev = it->second;
        dev->ThreadsWaitJoin();
    }
}
