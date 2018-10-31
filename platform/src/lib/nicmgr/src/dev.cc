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

#include "dev.hpp"
#include "pd_client.hpp"
#include "eth_dev.hpp"
#include "accel_dev.hpp"
#include "logger.hpp"
#include "sdk/pal.hpp"
#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"
#include "adminq.h"

namespace pt = boost::property_tree;

#define QSTATE_INFO_FILE_NAME  "lif_qstate_info.json"

sdk::lib::indexer *intr_allocator = sdk::lib::indexer::factory(4096);

#define LIF_ID_BASE     (100)
sdk::lib::indexer *lif_allocator = sdk::lib::indexer::factory(1024);

// #define ENIC_ID_BASE    (100)
// sdk::lib::indexer *enic_allocator = sdk::lib::indexer::factory(4096);

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
    struct queue_info empty_qinfo[NUM_QUEUE_TYPES] = {0};
    struct lif_info linfo;
    uint32_t lif_id;
    int ret;

    // Reserve hw_lif_id for uplinks which HAL will use from 1 - 32.
    ret = pd->lm_->LIFRangeAlloc(1, HAL_HW_LIF_ID_MAX);
    if (ret <= 0) {
        NIC_LOG_ERR("Unable to reserve 1-32 lifs for uplinks");
        return -1;
    }

    /*
     * Even though HAL_HW_LIF_ID_MAX IDs were reserved locally above,
     * HAL in DOL mode only does the same when platform is HW/HAPS.
     * So to ensure the nicmgr LIF is assigned the correct hw_lif_id,
     * we need to create a bunch of throw aways HAL LIFs in the DOL case.
     */
    if (platform_is_hw(platform)) {
        return 0;
    }

    /*
     * Note that hal->LifCreate() starts its hw_lif_id range from 0,
     * so the loop needs to go past HAL_HW_LIF_ID_MAX.
     */
    for (uint32_t i = 0; i <= HAL_HW_LIF_ID_MAX; i++) {
        if (lif_allocator->alloc(&lif_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate reserved lif_id");
            return -1;
        }
        memset(&linfo, 0, sizeof(linfo));
        linfo.lif_id = lif_id;
        if (hal->LifCreate(lif_id,  NULL, empty_qinfo, &linfo)) {
            NIC_LOG_ERR("Failed to reserve LIF {} thru HAL LifCreate", lif_id);
            return -1;
        }
    }
    return 0;
}

DeviceManager::DeviceManager(enum ForwardingMode fwd_mode, platform_t platform)
{
    hal = new HalClient(fwd_mode);
    hal_common_client = HalGRPCClient::Factory((HalForwardingMode)fwd_mode);
    uint8_t     cosA = 1;
    uint8_t     cosB = 0;
    const char  *dol_integ_str;
    uint32_t    lif_id;

    utils::logger::init();

#ifdef __x86_64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    pd = PdClient::factory(platform);
    assert(pd);

    // Create nicmgr service lif
    nicmgr_req_qstate_t qstate_req = { 0 };
    nicmgr_resp_qstate_t qstate_resp = { 0 };

    struct queue_info qinfo [NUM_QUEUE_TYPES] = {
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

    if (lifs_reservation(platform)) {
        throw runtime_error("Failed to reserve LIFs");
    }

    /*
     * Allocate sw_lif_id for nicmgr LIF
     */
    if (lif_allocator->alloc(&lif_id) != sdk::lib::indexer::SUCCESS) {
        throw runtime_error("Failed to allocate nicmgr sw_lif_id");
    }
    memset(&info, 0, sizeof(info));
    info.lif_id = lif_id;

    dol_integ_str = std::getenv("DOL");
    dol_integ = dol_integ_str ? !!atoi(dol_integ_str) : false;
    if (dol_integ) {

        /*
         * For DOL integration, allocate nicmgr LIF fully with HAL, i.e.,
         * complete with qstate initialization by the HAL>
         */
        if (hal->LifCreate(info.lif_id, qinfo, &info,
                           0, false, 0, 0, 0, 0) == 0) {
            throw runtime_error("Failed to create HAL nicmgr LIF");
        }

    } else {

        /*
         * HAL in non-HW platforms (x86) will fail LifCreate() with
         * non-zero hw_lif_id. One good thing is such a call isn't
         * needed on thos platforms.
         */
        info.hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
        if (platform_is_hw(platform)) {
            if (hal->LifCreate(info.lif_id,  NULL, qinfo, &info)) {
                throw runtime_error("Failed to create HAL nicmgr LIF");
            }
        }
        uint8_t coses = (((cosB & 0x0f) << 4) | (cosA & 0x0f));
        pd->program_qstate(qinfo, &info, coses);
    }
    NIC_LOG_INFO("nicmgr lif id:{}, hw_lif_id: {}", info.lif_id, info.hw_lif_id);

    // Init QState
    uint64_t hbm_base = NICMGR_BASE;
    uint8_t tmp[sizeof(struct nicmgr_req_desc)] = { 0 };
    NIC_LOG_INFO("nicmgr hbm {:#x}", hbm_base);

    ring_size = 4096;
    req_ring_base = hbm_base;
    resp_ring_base = hbm_base + (sizeof(struct nicmgr_req_desc) * ring_size);

    NIC_LOG_INFO("nicmgr req qstate address {:#x}", info.qstate_addr[NICMGR_QTYPE_REQ]);
    NIC_LOG_INFO("nicmgr resp qstate address {:#x}", info.qstate_addr[NICMGR_QTYPE_RESP]);
    NIC_LOG_INFO("nicmgr req queue address {:#x}", req_ring_base);
    NIC_LOG_INFO("nicmgr resp queue address {:#x}", resp_ring_base);

    req_head = ring_size - 1;
    req_tail = 0;
    resp_head = 0;
    resp_tail = 0;

    NIC_LOG_INFO("About to invalidate txdma cacheline at: {}", info.qstate_addr[NICMGR_QTYPE_REQ]);
    invalidate_txdma_cacheline(info.qstate_addr[NICMGR_QTYPE_REQ]);
    NIC_LOG_INFO("Invalidated txdma cacheline at: {}", info.qstate_addr[NICMGR_QTYPE_REQ]);
    READ_MEM(info.qstate_addr[NICMGR_QTYPE_REQ], (uint8_t *)&qstate_req, sizeof(qstate_req), 0);
    NIC_LOG_INFO("Reading txdma cacheline at: {}", info.qstate_addr[NICMGR_QTYPE_REQ]);

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

    NIC_LOG_INFO("About to write to nicmgr qtype req");

    WRITE_MEM(info.qstate_addr[NICMGR_QTYPE_REQ], (uint8_t *)&qstate_req, sizeof(qstate_req), 0);
    invalidate_txdma_cacheline(info.qstate_addr[NICMGR_QTYPE_REQ]);


    invalidate_txdma_cacheline(info.qstate_addr[NICMGR_QTYPE_RESP]);
    READ_MEM(info.qstate_addr[NICMGR_QTYPE_RESP], (uint8_t *)&qstate_resp, sizeof(qstate_resp), 0);

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

    NIC_LOG_INFO("About to write to nicmgr qtype resp");
    WRITE_MEM(info.qstate_addr[NICMGR_QTYPE_RESP], (uint8_t *)&qstate_resp, sizeof(qstate_resp), 0);

    NIC_LOG_INFO("About to invalidate txdma cache line");
    invalidate_txdma_cacheline(info.qstate_addr[NICMGR_QTYPE_RESP]);
}

DeviceManager::~DeviceManager()
{
}

int
DeviceManager::LoadConfig(string path)
{
    boost::property_tree::read_json(path, spec);
    struct eth_devspec *eth_spec;
    struct accel_devspec *accel_spec;
    uint32_t intr_base = 0;
    char *system_uuid = getenv("SYSUUID");
    uint64_t sys_mac_base;

    if (!strcmp(system_uuid, "")) {
        sys_mac_base = 0x00DEADBEEF00llu;
    } else {
        sys_mac_base = mac_to_int(system_uuid);
    }

    NIC_LOG_INFO("{}: Entered SysUuid={} SysMacBase={}\n", __FUNCTION__, system_uuid, sys_mac_base);

#if 0
    // Discover existing configuration
    hal->VrfProbe();
    hal->L2SegmentProbe();
    hal->InterfaceProbe();
    hal->LifProbe();
    hal->EndpointProbe();
    hal->MulticastProbe();
#endif

    // Create Network
    if (spec.get_child_optional("network")) {
        // Create Uplinks
        if (spec.get_child_optional("network.uplink")) {
            for (const auto &node : spec.get_child("network.uplink")) {
                auto val = node.second;
                Uplink *up1 = Uplink::Factory(val.get<uint64_t>("id"), val.get<bool>("oob", false));
                uplinks[val.get<uint64_t>("id")] = up1;
            }
        }
#if 0
        // Create VRFs
        if (spec.get_child_optional("network.vrf")) {
            for (const auto &node : spec.get_child("network.vrf")) {
                auto val = node.second;
                auto vrf_handle = hal->VrfCreate(val.get<uint32_t>("id"));
                if (vrf_handle == 0) {
                    NIC_LOG_ERR("Failed to create vrf");
                    return -1;
                }
            } // foreach vrf
        } // Create VRFs

        // Create L2Segments
        if (spec.get_child_optional("network.l2seg")) {
            for (const auto &node : spec.get_child("network.l2seg")) {
                auto val = node.second;
                auto l2seg_handle = hal->L2SegmentCreate(val.get<uint64_t>("vrf"),
                                                    val.get<uint64_t>("id"),
                                                    val.get<uint16_t>("vlan"));
                if (l2seg_handle == 0) {
                    NIC_LOG_ERR("Failed to create l2segment for vlan");
                    return -1;
                }
            } // foreach l2seg
        } // Create l2segs

        if (hal->get_fwd_mode() == FWD_MODE_CLASSIC_NIC) {
            // Create Uplinks
            if (spec.get_child_optional("network.uplink")) {
                for (const auto &node : spec.get_child("network.uplink")) {
                    auto val = node.second;
                    auto uplink_handle = hal->UplinkCreate(val.get<uint64_t>("id"),
                                                    val.get<uint64_t>("port"),
                                                    val.get<uint64_t>("native_l2seg", 0));
                    if (uplink_handle == 0) {
                        NIC_LOG_ERR("Failed to create uplink interface");
                        return -1;
                    }

                    // Add native l2segment on uplink
                    if (hal->AddL2SegmentOnUplink(val.get<uint64_t>("id"),
                        val.get<uint64_t>("native_l2seg", 0))) {
                        NIC_LOG_ERR("Failed to add vlan on uplink");
                        // TODO: Currently hal returns incorrect status, when
                        // vl2seg is already added on uplink.
                    }

                    // Add vlans to uplink
                    if (val.get_optional<string>("nonnative_l2seg")) {
                        for (const auto &l2seg : val.get_child("nonnative_l2seg")) {
                            if (hal->AddL2SegmentOnUplink(val.get<uint64_t>("id"),
                                boost::lexical_cast<uint64_t>(l2seg.second.data()))) {
                                NIC_LOG_ERR("Failed to add vlan on uplink");
                                // TODO: Currently hal returns incorrect status, when
                                // vl2seg is already added on uplink.
                            }
                        }
                    } // Add vlans to uplink
                } // foreach uplink
            } // Create uplinks
        }
#endif
    }

    // Create MNICs
    if (spec.get_child_optional("mnic_dev")) {
        for (const auto &node : spec.get_child("mnic_dev")) {
            eth_spec = new struct eth_devspec;
            memset(eth_spec, 0, sizeof(*eth_spec));

            auto val = node.second;

            // TODO: Refactor into resource allocator.
            // TODO: For now interrupts must be 256 aligned. The allocator does not support
            //       aligning resource ids, so allocate 256.
            if (intr_allocator->alloc_block(&intr_base, 256) != sdk::lib::indexer::SUCCESS) {
                NIC_LOG_ERR("lif{}: Failed to allocate interrupts", info.hw_lif_id);
                return -1;
            }
            eth_spec->dev_uuid = val.get<uint64_t>("dev_uuid");
            eth_spec->rxq_count = val.get<uint64_t>("rxq_count");
            eth_spec->txq_count = val.get<uint64_t>("txq_count");
            eth_spec->eq_count = val.get<uint64_t>("eq_count");
            eth_spec->adminq_count = val.get<uint64_t>("adminq_count");
            eth_spec->intr_base = intr_base;
            eth_spec->intr_count = val.get<uint64_t>("intr_count");
            eth_spec->mac_addr = sys_mac_base++;

            eth_spec->lif_id = val.get<uint64_t>("lif_id", 0);
            if (eth_spec->lif_id == 0) {
                if (lif_allocator->alloc(&lif_id) != sdk::lib::indexer::SUCCESS) {
                    NIC_LOG_ERR("Failed to allocate lif");
                    return -1;
                }
                eth_spec->lif_id = LIF_ID_BASE + lif_id;
            }

            eth_spec->hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
            if (val.get_optional<string>("network")) {
                // eth_spec->vrf_id = val.get<uint64_t>("network.vrf");
                eth_spec->uplink_id = val.get<uint64_t>("network.uplink");
                eth_spec->uplink = uplinks[eth_spec->uplink_id];
                if (eth_spec->uplink == NULL) {
                    NIC_LOG_ERR("Unable to find uplink for id: {}", eth_spec->uplink_id);
                }
                // eth_spec->native_l2seg_id = val.get<uint32_t>("network.native_l2seg");
            }

#if 0
            eth_spec->enic_id = val.get<uint64_t>("network.enic", 0);
            if (eth_spec->enic_id == 0) {
                if (enic_allocator->alloc(&enic_id) != sdk::lib::indexer::SUCCESS) {
                    NIC_LOG_ERR("Failed to allocate enic");
                    return -1;
                }
                eth_spec->enic_id = ENIC_ID_BASE + enic_id;
            }
#endif

            eth_spec->host_dev = false;
            NIC_LOG_INFO("Adding mnic device with lif_id: {}, hw_lif_id: {}",
                         eth_spec->lif_id,
                         eth_spec->hw_lif_id);
            AddDevice(ETH, (void *)eth_spec);
        }
    }

    // Create Ethernet devices
    if (spec.get_child_optional("eth_dev")) {
        for (const auto &node : spec.get_child("eth_dev")) {
            eth_spec = new struct eth_devspec;
            memset(eth_spec, 0, sizeof(*eth_spec));

            auto val = node.second;

            // TODO: Refactor into resource allocator
            // TODO: For now interrupts must be 256 aligned. The allocator does not support
            //       aligning resource ids, so allocate 256.
            if (intr_allocator->alloc_block(&intr_base, 256) != sdk::lib::indexer::SUCCESS) {
                NIC_LOG_ERR("lif{}: Failed to allocate interrupts", info.hw_lif_id);
                return -1;
            }

            eth_spec->dev_uuid = val.get<uint64_t>("dev_uuid");
            eth_spec->rxq_count = val.get<uint64_t>("rxq_count");
            eth_spec->txq_count = val.get<uint64_t>("txq_count");
            eth_spec->eq_count = val.get<uint64_t>("eq_count");
            eth_spec->adminq_count = val.get<uint64_t>("adminq_count");
            eth_spec->intr_base = intr_base;
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

            eth_spec->lif_id = val.get<uint64_t>("lif_id", 0);
            if (eth_spec->lif_id == 0) {
                if (lif_allocator->alloc(&lif_id) != sdk::lib::indexer::SUCCESS) {
                    NIC_LOG_ERR("Failed to allocate lif");
                    return -1;
                }
                eth_spec->lif_id = LIF_ID_BASE + lif_id;
            }

            eth_spec->hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
            // eth_spec->vrf_id = val.get<uint64_t>("network.vrf", 0);
            eth_spec->uplink_id = val.get<uint64_t>("network.uplink");
            eth_spec->uplink = uplinks[eth_spec->uplink_id];
            if (eth_spec->uplink == NULL) {
                NIC_LOG_ERR("Unable to find uplink for id: {}", eth_spec->uplink_id);
            }

#if 0
            eth_spec->native_l2seg_id = val.get<uint32_t>("network.native_l2seg", 0);
            eth_spec->enic_id = val.get<uint64_t>("network.enic", 0);
            if (eth_spec->enic_id == 0) {
                if (enic_allocator->alloc(&enic_id) != sdk::lib::indexer::SUCCESS) {
                    NIC_LOG_ERR("Failed to allocate enic");
                    return -1;
                }
                eth_spec->enic_id = ENIC_ID_BASE + enic_id;
            }
#endif

            eth_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
            eth_spec->host_dev = true;
            NIC_LOG_INFO("Adding eth device with lif_id: {}, hw_lif_id: {}",
                         eth_spec->lif_id,
                         eth_spec->hw_lif_id);
            AddDevice(ETH, (void *)eth_spec);
        }
    }

    // Create Accelerator devices
    if (spec.get_child_optional("accel_dev")) {
        NIC_LOG_INFO("Creating accel device");
        for (const auto &node : spec.get_child("accel_dev")) {
            NIC_LOG_INFO("Creating accel device");
            accel_spec = new struct accel_devspec;
            memset(accel_spec, 0, sizeof(*accel_spec));

            auto val = node.second;

            // TODO: Refactor into resource allocator
            // TODO: For now interrupts must be 256 aligned. The allocator does not support
            //       aligning resource ids, so allocate 256.
            if (intr_allocator->alloc_block(&intr_base, 256) != sdk::lib::indexer::SUCCESS) {
                NIC_LOG_ERR("lif{}: Failed to allocate interrupts", info.hw_lif_id);
                return -1;
            }

            accel_spec->hw_lif_id = pd->lm_->LIFRangeAlloc(-1, 1);
            if (dol_integ) {
                    accel_spec->lif_id = STORAGE_SEQ_SW_LIF_ID;
            } else {
                if (lif_allocator->alloc(&lif_id) != sdk::lib::indexer::SUCCESS) {
                    NIC_LOG_ERR("Failed to allocate accel_dev lif_id");
                    return -1;
                }
                accel_spec->lif_id =  LIF_ID_BASE + lif_id;
            }
            accel_spec->seq_queue_count = val.get<uint32_t>("seq_queue_count");
            accel_spec->adminq_count = val.get<uint32_t>("adminq_count");
            accel_spec->intr_base = intr_base;
            accel_spec->intr_count = val.get<uint32_t>("intr_count");

            accel_spec->pcie_port = val.get<uint8_t>("pcie.port", 0);
            AddDevice(ACCEL, (void *)accel_spec);
        }
    }

    NIC_LOG_INFO("Calling GenerateQstateInfoJson for: {}...",
            pd->gen_dir_path_ + "/" + QSTATE_INFO_FILE_NAME);
    GenerateQstateInfoJson(pd->gen_dir_path_ + "/" + QSTATE_INFO_FILE_NAME);

    NIC_LOG_INFO("{}: Exited\n", __FUNCTION__);
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

    NIC_LOG_INFO("{}: Entered\n", __FUNCTION__);

    switch (type) {
    case MNIC:
        NIC_LOG_ERR("Unsupported Device Type MNIC");
        return NULL;
    case DEBUG:
        NIC_LOG_ERR("Unsupported Device Type DEBUG");
        return NULL;
    case ETH:
        eth_dev = new Eth(hal, hal_common_client, dev_spec, pd);
        devices[eth_dev->info.hw_lif_id] = (Device *)eth_dev;
        NIC_LOG_INFO("Eth Device lifid: {}\n", eth_dev->info.hw_lif_id);
        return (Device *)eth_dev;
    case ACCEL:
        accel_dev = new Accel_PF(hal, dev_spec, &info, pd, dol_integ);
        devices[accel_dev->info.hw_lif_id] = (Device *)accel_dev;
        NIC_LOG_INFO("Acc Device lifid: {}\n", accel_dev->info.hw_lif_id);
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

    uint64_t req_qstate_addr = info.qstate_addr[NICMGR_QTYPE_REQ];
    uint64_t req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0x8 /* PI_UPD + UPD_NOP */ << 17) +
                (info.hw_lif_id << 6) +
                (NICMGR_QTYPE_REQ << 3);
    uint64_t req_db_data = 0x0;

    uint64_t resp_qstate_addr = info.qstate_addr[NICMGR_QTYPE_RESP];
    uint64_t resp_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0x9 /* PI_UPD + UPD_EVAL */ << 17) +
                (info.hw_lif_id << 6) +
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
DeviceManager::GenerateQstateInfoJson(std::string qstate_info_file)
{
    pt::ptree root, lifs;

    NIC_LOG_INFO("{}: Entered with filename: {}\n", __FUNCTION__, qstate_info_file);

    for (auto it = devices.cbegin(); it != devices.cend(); it++) {
        Device *dev = it->second;
        Eth *eth_dev = (Eth *) dev;
        eth_dev->GenerateQstateInfoJson(lifs);
    }

    root.push_back(std::make_pair("lifs", lifs));
    pt::write_json(qstate_info_file, root);
    NIC_LOG_INFO("{}: Exited\n", __FUNCTION__);
    return 0;
}
