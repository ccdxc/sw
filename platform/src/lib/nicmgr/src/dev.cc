/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

#include "dev.hpp"
#include "eth_dev.hpp"
#include "accel_dev.hpp"
#include "sdk/pal.hpp"
#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"

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

DeviceManager::DeviceManager(enum ForwardingMode fwd_mode)
{
    hal = new HalClient(fwd_mode);

#ifdef __x86_64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    //assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif
}

DeviceManager::~DeviceManager()
{
}

int
DeviceManager::LoadConfig(string path)
{
    // Discover all VRFs
    hal->VrfProbe();

    // Discover all L2 Segments
    hal->L2SegmentProbe();

    // Discover all Lifs
    hal->LifProbe();

    // Discover all Uplink Interfaces, ENICs
    hal->InterfaceProbe();

    // Discover all Endpoints
    hal->EndpointProbe();

    // Discover all Multicast groups
    hal->MulticastProbe();

    // Create topology
    uint64_t vrf_id = 1, vrf_handle = 0;
    uint64_t uplink_if_id = 1, uplink_if_handle = 0;
    uint8_t port_num = 1, num_uplinks = 2;
    vector<uint16_t> vlans = {1, 10, 16};
    uint64_t native_l2seg_id = 1; /* l2seg_id = vlan_id */
    uint64_t l2seg_handle = 0;

    // Create VRF
    if (hal->vrf_id2handle.find(vrf_id) == hal->vrf_id2handle.end()) {
        vrf_handle = hal->VrfCreate(vrf_id);
        if (vrf_handle == 0) {
            return -1;
        }
    }

    // Create VLANs
    for (auto it = vlans.cbegin(); it != vlans.cend(); it++) {
        if (hal->vlan2seg_map.find(*it) == hal->vlan2seg_map.end()) {
            l2seg_handle = hal->L2SegmentCreate(vrf_id,
                                                *it, /* l2seg_id = vlan_id */
                                                ::l2segment::BROADCAST_FWD_POLICY_FLOOD,
                                                ::l2segment::MULTICAST_FWD_POLICY_FLOOD,
                                                *it);
            if (l2seg_handle == 0) {
                return -1;
            }
        }
    }

    // Create uplinks
    for (; port_num < num_uplinks + 1; port_num++, uplink_if_id++) {
        if (hal->uplink_map.find(port_num) == hal->uplink_map.end()) {
            // Create uplink interface
            uplink_if_handle = hal->UplinkCreate(uplink_if_id, port_num, native_l2seg_id);
            if (uplink_if_handle == 0) {
                return -1;
            }

            // Add vlans to uplink
            for (auto it = hal->vlan2seg_map.cbegin(); it != hal->vlan2seg_map.cend(); it++) {
                if (hal->AddL2SegmentOnUplink(uplink_if_id, it->second)) {
                    return -1;
                }
            }
        }
    }

    // Create nicmgr service lif
    struct eth_admin_qstate qstate_req = { 0 };
    struct eth_admin_qstate qstate_resp = { 0 };

    struct queue_info qinfo [NUM_QUEUE_TYPES] = {
        [NICMGR_QTYPE_REQ] = {
            .type_num = NICMGR_QTYPE_REQ,
            .size = 1,
            .entries = 0,
            .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
            .prog = "txdma_stage0.bin",
            .label = "nicmgr_req_stage0",
            .qstate = (const char *)&qstate_req
        },
        [NICMGR_QTYPE_RESP] = {
            .type_num = NICMGR_QTYPE_RESP,
            .size = 1,
            .entries = 0,
            .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
            .prog = "txdma_stage0.bin",
            .label = "nicmgr_resp_stage0",
            .qstate = (const char *)&qstate_resp
        },
    };

    if (hal->lif_map.find(1) == hal->lif_map.end()) {
        lif_handle = hal->LifCreate(1, qinfo, &info, false, 0, 0);
        if (lif_handle == 0) {
            return -1;
        }
    } else {
        lif_handle = hal->LifGet(1, &info);
        if (lif_handle == 0) {
            return -1;
        }
    };

    // Init QState
    uint64_t hbm_base = NICMGR_BASE;
    uint8_t tmp[sizeof(struct nicmgr_req_desc)] = { 0 };
    printf("[INFO] nicmgr hbm 0x%lx\n", hbm_base);

    ring_size = 4096;
    req_ring_base = hbm_base;
    resp_ring_base = hbm_base + (sizeof(struct nicmgr_req_desc) * ring_size);

    printf("[INFO] nicmgr req qstate address 0x%lx\n", info.qstate_addr[NICMGR_QTYPE_REQ]);
    printf("[INFO] nicmgr resp qstate address 0x%lx\n", info.qstate_addr[NICMGR_QTYPE_RESP]);
    printf("[INFO] nicmgr req queue address 0x%lx\n", req_ring_base);
    printf("[INFO] nicmgr resp queue address 0x%lx\n", resp_ring_base);

    req_head = ring_size - 1;
    req_tail = 0;
    resp_head = 0;
    resp_tail = 0;

    invalidate_txdma_cacheline(info.qstate_addr[NICMGR_QTYPE_REQ]);
    READ_MEM(info.qstate_addr[NICMGR_QTYPE_REQ], (uint8_t *)&qstate_req, sizeof(qstate_req));

    qstate_req.p_index0 = req_head;
    qstate_req.c_index0 = req_tail;
    qstate_req.comp_index = 0;
    qstate_req.ci_fetch = 0;
    qstate_req.enable = 1;
    qstate_req.host = 1;
    qstate_req.total = 1;
    qstate_req.color = 1;
    qstate_req.rsvd1 = 0x1f;
    qstate_req.ring_base = req_ring_base;
    qstate_req.ring_size = log2(ring_size);

    for (int i = 0; i < ring_size; i++) {
        WRITE_MEM(req_ring_base + (sizeof(struct nicmgr_req_desc) * i),
             (uint8_t *)tmp, sizeof(tmp));
    }

    WRITE_MEM(info.qstate_addr[NICMGR_QTYPE_REQ], (uint8_t *)&qstate_req, sizeof(qstate_req));
    invalidate_txdma_cacheline(info.qstate_addr[NICMGR_QTYPE_REQ]);


    invalidate_txdma_cacheline(info.qstate_addr[NICMGR_QTYPE_RESP]);
    READ_MEM(info.qstate_addr[NICMGR_QTYPE_RESP], (uint8_t *)&qstate_resp, sizeof(qstate_resp));

    qstate_resp.p_index0 = resp_head;
    qstate_resp.c_index0 = resp_tail;
    qstate_resp.comp_index = 0;
    qstate_resp.ci_fetch = 0;
    qstate_resp.enable = 1;
    qstate_resp.host = 1;
    qstate_resp.total = 1;
    qstate_resp.color = 1;
    qstate_resp.rsvd1 = 0x1f;
    qstate_resp.ring_base = resp_ring_base;
    qstate_resp.ring_size = log2(ring_size);

    for (int i = 0; i < ring_size; i++) {
        WRITE_MEM(resp_ring_base + (sizeof(struct nicmgr_resp_desc) * i),
            (uint8_t *)tmp, sizeof(tmp));
    }

    WRITE_MEM(info.qstate_addr[NICMGR_QTYPE_RESP], (uint8_t *)&qstate_resp, sizeof(qstate_resp));
    invalidate_txdma_cacheline(info.qstate_addr[NICMGR_QTYPE_RESP]);

    return 0;
}

Device *
DeviceManager::AddDevice(enum DeviceType type, void *dev_spec)
{
    Eth_PF *eth_dev;
    Accel_PF *accel_dev;

    switch (type) {
    case ETH_PF:
        eth_dev = new Eth_PF(hal, dev_spec);
        devices[eth_dev->info.hw_lif_id] = (Device *)eth_dev;
        return (Device *)eth_dev;
    case ETH_VF:
        cerr << "[ERROR] : Unsupported Device Type ETH_VF" << endl;
        return NULL;
    case NVME:
        cerr << "[ERROR] : Unsupported Device Type NVME" << endl;
        return NULL;
    case ACCEL:
        accel_dev = new Accel_PF(hal, dev_spec, &info);
        devices[accel_dev->info.hw_lif_id] = (Device *)accel_dev;
        return (Device *)accel_dev;
    case VIRTIO:
        cerr << "[ERROR] : Unsupported Device Type VIRTIO" << endl;
        return NULL;
    case DEBUG:
        cerr << "[ERROR] : Unsupported Device Type DEBUG" << endl;
        return NULL;
    default:
        return NULL;
    }

    return NULL;
}

#ifdef __aarch64__
void
DeviceManager::PcieEventHandler(pciehdev_t *pdev, const pciehdev_eventdata_t *evd)
{
    Device *obj = (Device *)pciehdev_get_priv(pdev);
    obj->DevcmdHandler();
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

    READ_MEM(req_qstate_addr + offsetof(struct eth_admin_qstate, p_index0),
             (uint8_t *)&p_index0, sizeof(p_index0));

    READ_MEM(req_qstate_addr + offsetof(struct eth_admin_qstate, c_index0),
             (uint8_t *)&c_index0, sizeof(c_index0));

    if (req_tail != c_index0) {

        printf("[INFO] request: PRE: p_index0 %d, c_index0 %d, head %d, tail %d\n",
               p_index0, c_index0, req_head, req_tail);

        // Read nicmgr request descriptor
        req_desc_addr = req_ring_base + (sizeof(req_desc) * req_tail); 
        READ_MEM(req_desc_addr, (uint8_t *)&req_desc, sizeof(req_desc));

        // printf("[DEBUG] request:");
        // for (uint32_t i = 0; i < sizeof(req_desc); i++) {
        //     printf(" %02x", ((uint8_t *)&req_desc)[i]);
        // }
        // printf("\n");

        printf("[INFO] request: lif %u qtype %u qid %u comp_index %u"
               " adminq_qstate_addr 0x%lx desc_addr 0x%lx\n",
               req_desc.lif, req_desc.qtype, req_desc.qid,
               req_desc.comp_index, req_desc.adminq_qstate_addr,
               req_desc_addr);

        // Process devcmd
        Device *dev = (Device *)devices[req_desc.lif];
        if (dev == NULL) {
            printf("[ERROR] Invalid AdminQ request for lif %u!\n", req_desc.lif);
        } else {
            dev->CmdHandler(&req_desc.cmd, (void *)&req_data,
                &resp_desc.comp, (void *)&resp_data);
        }

        // Ring doorbell to update the PI
        req_head = (req_head + 1) & (ring_size - 1);
        req_tail = (req_tail + 1) & (ring_size - 1);
        req_db_data = req_head;
        printf("[INFO] req_db_addr %lx req_db_data %lx\n", req_db_addr, req_db_data);
        WRITE_DB64(req_db_addr, req_db_data);

        invalidate_txdma_cacheline(req_qstate_addr);

        READ_MEM(req_qstate_addr + offsetof(struct eth_admin_qstate, p_index0),
                 (uint8_t *)&p_index0, sizeof(p_index0));

        READ_MEM(req_qstate_addr + offsetof(struct eth_admin_qstate, c_index0),
                 (uint8_t *)&c_index0, sizeof(c_index0));

        printf("[INFO] request: POST: p_index0 %d, c_index0 %d, head %d, tail %d\n",
               p_index0, c_index0, req_head, req_tail);

        // Write nicmgr response descriptor
        invalidate_txdma_cacheline(resp_qstate_addr);

        READ_MEM(resp_qstate_addr + offsetof(struct eth_admin_qstate, p_index0),
                 (uint8_t *)&p_index0, sizeof(p_index0));

        READ_MEM(resp_qstate_addr + offsetof(struct eth_admin_qstate, c_index0),
                 (uint8_t *)&c_index0, sizeof(c_index0));

        printf("[INFO] response: PRE: p_index0 %d, c_index0 %d, head %d, tail %d\n",
               p_index0, c_index0, resp_head, resp_tail);

        resp_desc_addr = resp_ring_base + (sizeof(resp_desc) * resp_tail);

        resp_desc.lif = req_desc.lif;
        resp_desc.qtype = req_desc.qtype;
        resp_desc.qid = req_desc.qid;
        resp_desc.comp_index = req_desc.comp_index;
        resp_desc.adminq_qstate_addr = req_desc.adminq_qstate_addr;

        // printf("[DEBUG] response:");
        // for (uint32_t i = 0; i < sizeof(resp_desc); i++) {
        //     printf(" %02x", ((uint8_t *)&resp_desc)[i]);
        // }
        // printf("\n");

        printf("[INFO] response: lif %u qtype %u qid %u comp_index %u"
               " adminq_qstate_addr 0x%lx desc_addr 0x%lx\n",
               resp_desc.lif, resp_desc.qtype, resp_desc.qid,
               resp_desc.comp_index, resp_desc.adminq_qstate_addr,
               resp_desc_addr);

        WRITE_MEM(resp_desc_addr, (uint8_t *)&resp_desc, sizeof(resp_desc));

        // Ring doorbell to update the PI and run nicmgr response program
        resp_tail = (resp_tail + 1) & (ring_size - 1);
        resp_db_data = resp_tail;
        printf("[INFO] resp_db_addr %lx resp_db_data %lx\n", resp_db_addr, resp_db_data);
        WRITE_DB64(resp_db_addr, resp_db_data);

        invalidate_txdma_cacheline(resp_qstate_addr);

        READ_MEM(resp_qstate_addr + offsetof(struct eth_admin_qstate, p_index0),
                 (uint8_t *)&p_index0, sizeof(p_index0));

        READ_MEM(resp_qstate_addr + offsetof(struct eth_admin_qstate, c_index0),
                 (uint8_t *)&c_index0, sizeof(c_index0));

        printf("[INFO] response: POST: p_index0 %d, c_index0 %d, head %d, tail %d\n",
               p_index0, c_index0, resp_head, resp_tail);
    }
}
