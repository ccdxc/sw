/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>

#include "gen/proto/nicmgr/nicmgr.pb.h"
#include "gen/proto/nicmgr/metrics.delphi.hpp"
#include "gen/proto/common/nicmgr_status_msgs.pb.h"
#include "gen/proto/common/nicmgr_status_msgs.delphi.hpp"

#include "nic/p4/common/defines.h"
#include "platform/src/lib/intrutils/include/intrutils.h"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

#include "logger.hpp"
#include "eth_dev.hpp"
#include "rdma_dev.hpp"
#include "hal_client.hpp"
#include "pd_client.hpp"
#include "platform/src/lib/hal_api/include/print.hpp"

using namespace nicmgr;
using namespace nicmgr_status_msgs;

using nicmgr::EthDeviceInfo;
using nicmgr::MacAddrVlans;
using nicmgr::MacAddrs;
using nicmgr::Vlans;
using nicmgr::LifInfo;
using nicmgr::QstateAddr;
using nicmgr_status_msgs::EthDeviceHostDownStatusMsg;
using nicmgr_status_msgs::EthDeviceHostUpStatusMsg;

extern class pciemgr *pciemgr;

static uint8_t *
memrev (uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

template<typename ... Args>
string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1 /* for '\0' */;
    unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return string( buf.get(), buf.get() + size - 1 ); // ignore '\0'
}

const char qstate[64] = { 0 };
const char admin_qstate[128] = { 0 };
const char rdma_qstate_512[512] = { 0 };
const char rdma_qstate_32[32]     = { 0 };
const char rdma_qstate_64[32]     = { 0 };

sdk::lib::indexer *Eth::fltr_allocator = sdk::lib::indexer::factory(4096);
sdk::lib::indexer *Eth::mnic_allocator = sdk::lib::indexer::factory(32, true /*thread_safe*/, true /*skip_zero*/);

struct queue_info Eth::qinfo [NUM_QUEUE_TYPES] = {
    [ETH_QTYPE_RX] = {
        .type_num = ETH_QTYPE_RX,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_RX,
        .prog = "rxdma_stage0.bin",
        .label = "eth_rx_stage0",
        .qstate = qstate
    },
    [ETH_QTYPE_TX] = {
        .type_num = ETH_QTYPE_TX,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_TX,
        .prog = "txdma_stage0.bin",
        .label = "eth_tx_stage0",
        .qstate = qstate
    },
    [ETH_QTYPE_ADMIN] = {
        .type_num = ETH_QTYPE_ADMIN,
        .size = 2,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
        .prog = "txdma_stage0.bin",
        .label = "adminq_stage0",
        .qstate = admin_qstate
    },
    [ETH_QTYPE_SQ] = {
        .type_num = ETH_QTYPE_SQ,
        .size = 4,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_RDMA_SEND,
        .prog = "txdma_stage0.bin",
        .label = "rdma_req_tx_stage0",
        .qstate = rdma_qstate_512
    },
    [ETH_QTYPE_RQ] = {
        .type_num = ETH_QTYPE_RQ,
        .size = 4,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_RDMA_SEND,
        .prog = "rxdma_stage0.bin",
        .label = "rdma_resp_rx_stage0",
        .qstate = rdma_qstate_512
    },
    [ETH_QTYPE_CQ] = {
        .type_num = ETH_QTYPE_CQ,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_CQ,
        .prog = "txdma_stage0.bin",
        .label = "rdma_req_tx_stage0",
        .qstate = rdma_qstate_64
    },
    [ETH_QTYPE_EQ] = {
        .type_num = ETH_QTYPE_EQ,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_EQ,
        .prog = "txdma_stage0.bin",
        .label = "rdma_req_tx_stage0",
        .qstate = rdma_qstate_32
    },
    [ETH_QTYPE_SVC] = {
        .type_num = ETH_QTYPE_SVC,
        .size = 1,
        .entries = 1,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_SVC,
        .prog = "txdma_stage0.bin",
        .label = "notify_stage0",
        .qstate = qstate
    },
};

void Eth::Update()
{
    int32_t     cosA = 1;
    int32_t     cosB = 0;
    cosB = HalClient::GetTxTrafficClassCos(spec->qos_group,
                                           spec->uplink ? spec->uplink->GetPortNum() : 0);
    if (cosB < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get cosB for group {}, uplink {}",
                    hal_lif_info_.hw_lif_id, spec->qos_group,
                    spec->uplink ? spec->uplink->GetPortNum() : 0);
        return;
    }
    uint8_t coses = (((cosB & 0x0f) << 4) | (cosA & 0x0f));
    uint64_t hbm_addr;
    uint32_t hbm_size;

    pd->program_qstate((struct queue_info*)hal_lif_info_.queue_info, &hal_lif_info_, coses);

    if (spec->enable_rdma) {

        //Request in 8MB units of CMB for per LIF
        hbm_size = spec->barmap_size << 23;

        pd->rdma_lif_init(hal_lif_info_.hw_lif_id, spec->key_count,
                          spec->ah_count, spec->pte_count,
                          &hbm_addr, &hbm_size);

        /*
         * Create the HBM resident queue memory region for RDMA and
         * if successful, register that memory with PCIe BAR2
         */
        if (hbm_size != 0) {
            pci_resources.cmbpa = hbm_addr;
            pci_resources.cmbsz = hbm_size;
            NIC_LOG_DEBUG("HBM address for RDMA LLQ memory {:#x} size {} bytes", pci_resources.cmbpa, pci_resources.cmbsz);
        }
    }
}

Eth::Eth(HalClient *hal_client,
         HalCommonClient *hal_common_client,
         void *dev_spec,
         hal_lif_info_t *nicmgr_lif_info,
         PdClient *pd_client)
{
    lif_state = LIF_STATE_CREATING;

    hal = hal_client;
    hal_common_client = hal_common_client;
    spec = (struct eth_devspec *)dev_spec;
    Eth::nicmgr_lif_info = nicmgr_lif_info;
    pd = pd_client;

    memset(&pci_resources, 0, sizeof(pci_resources));
    memset(&mnet_req, 0, sizeof(mnet_dev_create_req_t));

    // Create LIF
    qinfo[ETH_QTYPE_RX].entries = (uint32_t)log2(spec->rxq_count);
    qinfo[ETH_QTYPE_TX].entries = (uint32_t)log2(spec->txq_count);
    qinfo[ETH_QTYPE_ADMIN].entries = (uint32_t)log2(spec->adminq_count + spec->rdma_adminq_count);
    qinfo[ETH_QTYPE_SQ].entries = (uint32_t)log2(spec->rdma_sq_count);
    qinfo[ETH_QTYPE_RQ].entries = (uint32_t)log2(spec->rdma_rq_count);
    qinfo[ETH_QTYPE_CQ].entries = (uint32_t)log2(spec->rdma_cq_count);
    qinfo[ETH_QTYPE_EQ].entries = (uint32_t)log2(spec->eq_count + spec->rdma_eq_count);

    memset(&hal_lif_info_, 0, sizeof(hal_lif_info_t));
    hal_lif_info_.id = spec->lif_id;
    hal_lif_info_.name = spec->if_name;
    hal_lif_info_.type = ConvertDevTypeToLifType(spec->eth_type);
    hal_lif_info_.pinned_uplink = spec->uplink;
    hal_lif_info_.hw_lif_id = spec->hw_lif_id;
    hal_lif_info_.enable_rdma = spec->enable_rdma;
    hal_lif_info_.pushed_to_hal = false;

    memcpy(hal_lif_info_.queue_info, qinfo, sizeof(hal_lif_info_.queue_info));

    NIC_LOG_INFO("lif created: name: {}, id:{}, type: {},"
                 " hw_lif_id: {}, mac:{},",
                 hal_lif_info_.name,
                 hal_lif_info_.id,
                 eth_dev_type_to_str(spec->eth_type),
                 hal_lif_info_.hw_lif_id,
                 macaddr2str(spec->mac_addr));

    // Configure PCI resources
    pci_resources.lif_valid = 1;
    pci_resources.lif = hal_lif_info_.hw_lif_id;
    pci_resources.intrb = spec->intr_base;
    pci_resources.intrc = spec->intr_count;
    pci_resources.port = spec->pcie_port;
    pci_resources.npids = spec->rdma_pid_count;
    // TODO: Need an allocator for this
    pci_resources.devcmdpa = DEVCMD_BASE + (hal_lif_info_.hw_lif_id * 4096 * 2);
    pci_resources.devcmddbpa = pci_resources.devcmdpa + 4096;
    MEM_SET(pci_resources.devcmdpa, 0, 4096, 0);
    MEM_SET(pci_resources.devcmddbpa, 0, 4096, 0);

    NIC_LOG_DEBUG("lif-{}: devcmd_addr {:#x} devcmddb_addr {:#x}",
            hal_lif_info_.hw_lif_id,
            pci_resources.devcmdpa, pci_resources.devcmddbpa);

    // Init Devcmd Region
    // TODO: mmap instead of calloc after porting to real pal
    devcmd = (struct dev_cmd_regs *)calloc(1, sizeof(struct dev_cmd_regs));
    assert (devcmd != NULL);
    devcmd->signature = DEV_CMD_SIGNATURE;
    WRITE_MEM(pci_resources.devcmdpa, (uint8_t *)devcmd, sizeof(*devcmd), 0);

    // Stats
    stats_mem_addr = pd->mem_start_addr(CAPRI_HBM_REG_LIF_STATS);
    if (stats_mem_addr == INVALID_MEM_ADDRESS) {
        NIC_LOG_ERR("Failed to get stats address lif {}", spec->lif_id);
        return;
    }
    stats_mem_addr += (hal_lif_info_.hw_lif_id << LG2_LIF_STATS_SIZE);
    host_stats_mem_addr = 0;

    NIC_LOG_INFO("lif-{}: stats_mem_addr: {:#x}",
        hal_lif_info_.hw_lif_id, stats_mem_addr);

    auto lif_stats =
        delphi::objects::LifMetrics::NewLifMetrics(spec->lif_id, stats_mem_addr);
    if (lif_stats == NULL) {
        NIC_LOG_ERR("Failed lif metrics registration with delphi lif {}",
                    spec->lif_id);
        return;
    }

    // Notify Queue
    notify_block = (struct notify_block *)calloc(1, sizeof(struct notify_block));
    assert (notify_block != NULL);
    notify_block_addr = pd->nicmgr_mem_alloc(sizeof(struct notify_block));
    host_notify_block_addr = 0;
    MEM_SET(notify_block_addr, 0, sizeof(struct notify_block), 0);

    NIC_LOG_INFO("lif-{}: notify_block_addr {:#x}",
        hal_lif_info_.hw_lif_id, notify_block_addr);

    // Notify Block
    notify_ring_head = 0;
    notify_ring_base = pd->nicmgr_mem_alloc(4096 + (sizeof(union notifyq_comp) * ETH_NOTIFYQ_RING_SIZE));
    MEM_SET(notify_ring_base, 0, 4096 + (sizeof(union notifyq_comp) * ETH_NOTIFYQ_RING_SIZE), 0);

    NIC_LOG_INFO("lif-{}: notify_ring_base {:#x}",
        hal_lif_info_.hw_lif_id, notify_ring_base);

    // Edma Queue
    edma_ring_head = 0;
    edma_ring_base = pd->nicmgr_mem_alloc(4096 + (sizeof(struct edma_cmd_desc) * ETH_EDMAQ_RING_SIZE));
    MEM_SET(edma_ring_base, 0, 4096 + (sizeof(struct edma_cmd_desc) * ETH_EDMAQ_RING_SIZE), 0);
    edma_comp_base = pd->nicmgr_mem_alloc(4096 + (sizeof(struct edma_comp_desc) * ETH_EDMAQ_RING_SIZE));
    MEM_SET(edma_comp_base, 0, 4096 + (sizeof(struct edma_comp_desc) * ETH_EDMAQ_RING_SIZE), 0);

    NIC_LOG_INFO("lif-{}: edma_ring_base {:#x} edma_comp_base {:#x}",
        hal_lif_info_.hw_lif_id, edma_ring_base, edma_comp_base);

    // Create the real device
    if (isMnic()) {
        mnet_req.devcmd_pa = pci_resources.devcmdpa;
        mnet_req.devcmd_db_pa = pci_resources.devcmddbpa;
        mnet_req.doorbell_pa = DOORBELL_ADDR(hal_lif_info_.hw_lif_id);
        mnet_req.drvcfg_pa = intr_drvcfg_addr(spec->intr_base);
        mnet_req.msixcfg_pa = intr_msixcfg_addr(spec->intr_base);
        strcpy(mnet_req.iface_name, spec->if_name.c_str());

        NIC_LOG_DEBUG("lif-{}: Mnet devcmd_pa: {:#x}, devcmd_db_pa: {:#x}, "
                     "doorbell_pa: {:#x}, drvcfg_pa: {:#x}, msixcfg_pa: {:#x}, "
                     "iface_name: {}, intr_base: {}, intr_count: {}",
                     hal_lif_info_.hw_lif_id,
                     mnet_req.devcmd_pa,
                     mnet_req.devcmd_db_pa,
                     mnet_req.doorbell_pa,
                     mnet_req.drvcfg_pa,
                     mnet_req.msixcfg_pa,
                     mnet_req.iface_name,
                     spec->intr_base,
                     spec->intr_count);

        for (uint32_t fw_cfg_entry = spec->intr_base; fw_cfg_entry < (spec->intr_base + spec->intr_count); fw_cfg_entry++) {
            intr_fwcfg(fw_cfg_entry, hal_lif_info_.hw_lif_id, 0, 0, 0, 0);
            intr_fwcfg_local(fw_cfg_entry, 1);
        }
    } else if (isHostManagement()) {
        NIC_LOG_DEBUG("lif-{}: name: {} Host Management device call to pcie",
                     hal_lif_info_.hw_lif_id, spec->if_name);
        // Create PCI device
        pdev = pciehdev_mgmteth_new(spec->if_name.c_str(), &pci_resources);
        if (pdev == NULL) {
            NIC_LOG_ERR("lif-{}: Failed to create Eth PCI device",
                        hal_lif_info_.hw_lif_id);
            return;
        }
        pciehdev_set_priv(pdev, (void *)this);

        if (pciemgr) {
            // Add device to PCI topology
            int ret = pciemgr->add_device(pdev);
            if (ret != 0) {
                NIC_LOG_ERR("lif-{}: Failed to add Host mgmt PCI device to topology",
                            hal_lif_info_.hw_lif_id);
                return;
            }
        }
    } else if (isHost()) {
        // Create PCI device
        pdev = pciehdev_eth_new(spec->if_name.c_str(), &pci_resources);
        if (pdev == NULL) {
            NIC_LOG_ERR("lif-{}: Failed to create Eth PCI device",
                hal_lif_info_.hw_lif_id);
            return;
        }
        pciehdev_set_priv(pdev, (void *)this);

        // Add device to PCI topology
        if (pciemgr) {
            int ret = pciemgr->add_device(pdev);
            if (ret != 0) {
                NIC_LOG_ERR("lif-{}: Failed to add Eth PCI device to topology",
                    hal_lif_info_.hw_lif_id);
                return;
            }
        }
    } else {
        NIC_LOG_DEBUG("lif-{}: Skipped creating PCI device, pcie_port {}",
                hal_lif_info_.hw_lif_id, spec->pcie_port);
    }

    lif_state = LIF_STATE_CREATED;
}

void
Eth::CreateMnet()
{
    NIC_LOG_DEBUG("lif-{}: Calling Mnet lib.", hal_lif_info_.hw_lif_id);
    int rs = create_mnet(&mnet_req);
    if (rs) {
        NIC_LOG_ERR("lif-{}: Failed to create mnet. ret: {}",
                    hal_lif_info_.hw_lif_id, rs);
        return;
    }
}

bool
Eth::isHost()
{
    return (spec->eth_type == ETH_HOST);
}

bool
Eth::isHostManagement()
{
    return (spec->eth_type == ETH_HOST_MGMT);
}

bool
Eth::isMnic()
{
    if (spec->eth_type == ETH_MNIC_OOB_MGMT ||
        spec->eth_type == ETH_MNIC_INTERNAL_MGMT ||
        spec->eth_type == ETH_MNIC_INBAND_MGMT) {
        return true;
    }
    return false;
}

uint64_t
Eth::GetQstateAddr(uint8_t qtype, uint32_t qid)
{
    uint32_t cnt, sz;

    if (qtype >= NUM_QUEUE_TYPES) {
        NIC_LOG_ERR("lif-{}: Invalid qtype {}", hal_lif_info_.hw_lif_id, qtype);
        return 0;
    }

    cnt = 1 << this->qinfo[qtype].entries;
    sz = 1 << (5 + this->qinfo[qtype].size);

    if (qid >= cnt) {
        NIC_LOG_ERR("lif-{}: Invalid qid {}", hal_lif_info_.hw_lif_id, qid);
        return 0;
    }

    return hal_lif_info_.qstate_addr[qtype] + (qid * sz);
}

void
Eth::DevcmdPoll()
{
    dev_cmd_db_t    db = {0};
    dev_cmd_db_t    db_clear = {0};

    READ_MEM(pci_resources.devcmddbpa, (uint8_t *)&db, sizeof(db), 0);
    if (db.v) {
        WRITE_MEM(pci_resources.devcmddbpa, (uint8_t *)&db_clear,
                    sizeof(db_clear), 0);
        NIC_LOG_DEBUG("{} lif-{} active", __FUNCTION__, hal_lif_info_.hw_lif_id);
        DevcmdHandler();
    }
}

void
Eth::StatsUpdateHandler(void *obj)
{
    Eth *eth = (Eth *)obj;

    if (eth->host_stats_mem_addr == 0) {
        NIC_LOG_WARN("lif-{}: Host stats region is not created!", eth->hal_lif_info_.hw_lif_id);
        return;
    }

    uint64_t addr, req_db_addr;

    struct edma_cmd_desc cmd = {
        .opcode = EDMA_OPCODE_LOCAL_TO_HOST,
        .len = sizeof(struct ionic_lif_stats),
        .src_lif = (uint16_t)eth->hal_lif_info_.hw_lif_id,
        .src_addr = eth->stats_mem_addr,
        .dst_lif = (uint16_t)eth->hal_lif_info_.hw_lif_id,
        .dst_addr = eth->host_stats_mem_addr,
    };

    // Update stats
    addr = eth->edma_ring_base + eth->edma_ring_head * sizeof(struct edma_cmd_desc);
    WRITE_MEM(addr, (uint8_t *)&cmd, sizeof(struct edma_cmd_desc), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1001 /* PI_UPD + SCHED_EVAL */ << 17) +
                (eth->hal_lif_info_.hw_lif_id << 6) +
                (ETH_QTYPE_SVC << 3);

    // NIC_LOG_INFO("lif-{}: Updating stats, edma_idx {} edma_desc_addr {:#x}",
    //     eth->hal_lif_info_.hw_lif_id, eth->edma_ring_head, addr);
    eth->edma_ring_head = (eth->edma_ring_head + 1) % ETH_EDMAQ_RING_SIZE;
    WRITE_DB64(req_db_addr, (ETH_EDMAQ_ID << 24) | eth->edma_ring_head);

    // FIXME: Wait for completion
}

void
Eth::LinkEventHandler(link_eventdata_t *evd)
{
    if (lif_state != LIF_STATE_INITED &&
        lif_state != LIF_STATE_UP &&
        lif_state != LIF_STATE_DOWN) {
        NIC_LOG_INFO("lif-{}: {} + {} => {}",
            hal_lif_info_.hw_lif_id,
            lif_state_to_str(lif_state),
            evd->oper_status ? "LINK_UP" : "LINK_DN",
            lif_state_to_str(lif_state));
        return;
    }

    if (host_notify_block_addr == 0) {
        NIC_LOG_WARN("lif-{}: Host notify block is not created!", hal_lif_info_.hw_lif_id);
        return;
    }

    uint64_t addr, req_db_addr;

    notify_block->eid = ++eid;
    notify_block->link_status = evd->oper_status;
    notify_block->link_error_bits = 0;
    notify_block->phy_type = 0;
    notify_block->link_speed = evd->port_speed;
    notify_block->autoneg_status = 0;
    notify_block->link_flap_count = ++link_flap_count;

    WRITE_MEM(notify_block_addr, (uint8_t *)notify_block, sizeof(struct notify_block), 0);

    struct edma_cmd_desc cmd = {
        .opcode = EDMA_OPCODE_LOCAL_TO_HOST,
        .len = sizeof(struct notify_block),
        .src_lif = (uint16_t)hal_lif_info_.hw_lif_id,
        .src_addr = notify_block_addr,
        .dst_lif = (uint16_t)hal_lif_info_.hw_lif_id,
        .dst_addr = host_notify_block_addr,
    };

    NIC_LOG_INFO("lif-{}: notify_block_addr local {:#x} host {:#x}",
        hal_lif_info_.hw_lif_id, notify_block_addr, host_notify_block_addr);

    // Update the notify block
    addr = edma_ring_base + edma_ring_head * sizeof(struct edma_cmd_desc);
    WRITE_MEM(addr, (uint8_t *)&cmd, sizeof(struct edma_cmd_desc), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1001 /* PI_UPD + SCHED_EVAL */ << 17) +
                (hal_lif_info_.hw_lif_id << 6) +
                (ETH_QTYPE_SVC << 3);

    NIC_LOG_INFO("lif-{}: Updating notify block, eid {} edma_idx {} edma_desc_addr {:#x}",
        hal_lif_info_.hw_lif_id, eid, edma_ring_head, addr);
    edma_ring_head = (edma_ring_head + 1) % ETH_EDMAQ_RING_SIZE;
    WRITE_DB64(req_db_addr, (ETH_EDMAQ_ID << 24) | edma_ring_head);

    // FIXME: Wait for completion

    // Send the link event notification
    struct link_change_event msg = {
        .eid = eid,
        .ecode = EVENT_OPCODE_LINK_CHANGE,
        .link_status = evd->oper_status,
        .link_error_bits = 0,
        .phy_type = 0,
        .link_speed = evd->port_speed,
        .autoneg_status = 0,
    };

    addr = notify_ring_base + notify_ring_head * sizeof(union notifyq_comp);
    WRITE_MEM(addr, (uint8_t *)&msg, sizeof(union notifyq_comp), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1001 /* PI_UPD + SCHED_EVAL */ << 17) +
                (hal_lif_info_.hw_lif_id << 6) +
                (ETH_QTYPE_SVC << 3);

    NIC_LOG_INFO("lif-{}: Sending notify event, eid {} notify_idx {} notify_desc_addr {:#x}",
        hal_lif_info_.hw_lif_id, eid, notify_ring_head, addr);
    notify_ring_head = (notify_ring_head + 1) % ETH_NOTIFYQ_RING_SIZE;
    WRITE_DB64(req_db_addr, (ETH_NOTIFYQ_ID << 24) | notify_ring_head);

    // FIXME: Wait for completion

    NIC_LOG_INFO("lif-{}: {} + {} => {}",
        hal_lif_info_.hw_lif_id,
        lif_state_to_str(lif_state),
        evd->oper_status ? "LINK_UP" : "LINK_DN",
        evd->oper_status ? lif_state_to_str(LIF_STATE_UP) : lif_state_to_str(LIF_STATE_DOWN));

    lif_state = evd->oper_status ? LIF_STATE_UP : LIF_STATE_DOWN;
}

void Eth::DevObjSave() {
    auto eth_dev_obj_ptr = make_shared<delphi::objects::EthDeviceInfo>();

    uint64_t mac_addr;
    uint16_t vlan;

    eth_dev_obj_ptr->set_key(spec->dev_uuid);
    eth_dev_obj_ptr->mutable_lif()->set_hw_lif_id(hal_lif_info_.hw_lif_id);
    for (int i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto qstate_addr = eth_dev_obj_ptr->mutable_lif()->add_qstate_addr();
        qstate_addr->set_qstate_addr(hal_lif_info_.qstate_addr[i]);
    }
    eth_dev_obj_ptr->set_rss_type(rss_type);
    eth_dev_obj_ptr->set_rss_key((const char*) rss_key);
    eth_dev_obj_ptr->set_rss_indir((const char*) rss_indir);
    for (auto it = vlans.cbegin(); it != vlans.cend(); it++) {
        vlan = it->second;
        auto vlans = eth_dev_obj_ptr->add_vlans();
        vlans->set_vlan(vlan);
    }
    for (auto it = mac_addrs.cbegin(); it != mac_addrs.cend(); it++) {
        mac_addr = it->second;
        auto mac_addresses = eth_dev_obj_ptr->add_mac_addrs();
        mac_addresses->set_mac_addr(mac_addr);
    }
    g_nicmgr_svc->sdk()->SetObject(eth_dev_obj_ptr);
}

void
Eth::DevcmdHandler()
{
    enum DevcmdStatus status;

    // read devcmd region
    READ_MEM(pci_resources.devcmdpa, (uint8_t *)devcmd,
             sizeof(struct dev_cmd_regs), 0);

    if (devcmd->done != 0) {
        NIC_LOG_ERR("lif-{}: Devcmd done is set before processing command, opcode = {}",
               hal_lif_info_.hw_lif_id, devcmd->cmd.cmd.opcode);
        status = DEVCMD_ERROR;
        goto devcmd_done;
    }

    if (devcmd->signature != DEV_CMD_SIGNATURE) {
        NIC_LOG_ERR("lif-{}: Devcmd signature mismatch, opcode = {}",
               hal_lif_info_.hw_lif_id, devcmd->cmd.cmd.opcode);
        status = DEVCMD_ERROR;
        goto devcmd_done;
    }

    status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->comp, &devcmd->data);

    // write data
    if (status == DEVCMD_SUCCESS) {
        WRITE_MEM(pci_resources.devcmdpa + offsetof(struct dev_cmd_regs, data),
                  (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, data),
                  sizeof(devcmd->data), 0);
    }

devcmd_done:
    devcmd->comp.comp.status = status;
    devcmd->done = 1;

    // write completion
    WRITE_MEM(pci_resources.devcmdpa + offsetof(struct dev_cmd_regs, comp),
              (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, comp),
              sizeof(devcmd->comp), 0);

    // write done
    WRITE_MEM(pci_resources.devcmdpa + offsetof(struct dev_cmd_regs, done),
              (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, done),
              sizeof(devcmd->done), 0);
}

#define CASE(opcode) case opcode: return #opcode

const char*
Eth::opcode_to_str(enum cmd_opcode opcode)
{
    switch(opcode) {
        CASE(CMD_OPCODE_NOP);
        CASE(CMD_OPCODE_RESET);
        CASE(CMD_OPCODE_IDENTIFY);
        CASE(CMD_OPCODE_LIF_INIT);
        CASE(CMD_OPCODE_ADMINQ_INIT);
        CASE(CMD_OPCODE_TXQ_INIT);
        CASE(CMD_OPCODE_RXQ_INIT);
        CASE(CMD_OPCODE_FEATURES);
        CASE(CMD_OPCODE_HANG_NOTIFY);
        CASE(CMD_OPCODE_Q_ENABLE);
        CASE(CMD_OPCODE_Q_DISABLE);
        CASE(CMD_OPCODE_NOTIFYQ_INIT);
        CASE(CMD_OPCODE_STATION_MAC_ADDR_GET);
        CASE(CMD_OPCODE_MTU_SET);
        CASE(CMD_OPCODE_RX_MODE_SET);
        CASE(CMD_OPCODE_RX_FILTER_ADD);
        CASE(CMD_OPCODE_RX_FILTER_DEL);
        CASE(CMD_OPCODE_STATS_DUMP_START);
        CASE(CMD_OPCODE_STATS_DUMP_STOP);
        CASE(CMD_OPCODE_DEBUG_Q_DUMP);
        CASE(CMD_OPCODE_RSS_HASH_SET);
        CASE(CMD_OPCODE_RSS_INDIR_SET);
        CASE(CMD_OPCODE_RDMA_RESET_LIF);
        CASE(CMD_OPCODE_RDMA_CREATE_EQ);
        CASE(CMD_OPCODE_RDMA_CREATE_CQ);
        CASE(CMD_OPCODE_RDMA_CREATE_ADMINQ);
        CASE(CMD_OPCODE_V0_RDMA_CREATE_MR);
        CASE(CMD_OPCODE_V0_RDMA_CREATE_CQ);
        CASE(CMD_OPCODE_V0_RDMA_CREATE_QP);
        CASE(CMD_OPCODE_V0_RDMA_MODIFY_QP);
        CASE(CMD_OPCODE_V0_RDMA_CREATE_AH);
        default: return "DEVCMD_UNKNOWN";
    }
}

const char*
Eth::lif_state_to_str(enum lif_state state)
{
    switch(state) {
        CASE(LIF_STATE_RESET);
        CASE(LIF_STATE_CREATING);
        CASE(LIF_STATE_CREATED);
        CASE(LIF_STATE_INITING);
        CASE(LIF_STATE_INITED);
        CASE(LIF_STATE_UP);
        CASE(LIF_STATE_DOWN);
        default: return "LIF_STATE_INVALID";
    }
}

enum DevcmdStatus
Eth::CmdHandler(void *req, void *req_data,
    void *resp, void *resp_data)
{
    union dev_cmd *cmd = (union dev_cmd *)req;
    union dev_cmd_comp *comp = (union dev_cmd_comp *)resp;
    enum DevcmdStatus status;

    NIC_LOG_DEBUG("lif-{}: Handling cmd: {}", hal_lif_info_.hw_lif_id,
                 opcode_to_str((enum cmd_opcode)cmd->cmd.opcode));

    switch (cmd->cmd.opcode) {

    case CMD_OPCODE_NOP:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RESET:
        status = this->_CmdReset(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_IDENTIFY:
        status = this->_CmdIdentify(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_LIF_INIT:
        status = this->_CmdLifInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_ADMINQ_INIT:
        status = this->_CmdAdminQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_TXQ_INIT:
        status = this->_CmdTxQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RXQ_INIT:
        status = this->_CmdRxQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_FEATURES:
        status = this->_CmdFeatures(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_HANG_NOTIFY:
        status = this->_CmdHangNotify(req, req_data, resp, resp_data);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_Q_ENABLE:
        status = this->_CmdQEnable(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_Q_DISABLE:
        status = this->_CmdQDisable(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_NOTIFYQ_INIT:
        status = this->_CmdNotifyQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_STATION_MAC_ADDR_GET:
        status = this->_CmdMacAddrGet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_MTU_SET:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RX_MODE_SET:
        status = this->_CmdSetMode(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RX_FILTER_ADD:
        status = this->_CmdRxFilterAdd(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RX_FILTER_DEL:
        status = this->_CmdRxFilterDel(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_STATS_DUMP_START:
        status = this->_CmdStatsDumpStart(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_STATS_DUMP_STOP:
        status = this->_CmdStatsDumpStop(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_DEBUG_Q_DUMP:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RSS_HASH_SET:
        status = this->_CmdRssHashSet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RSS_INDIR_SET:
        status = this->_CmdRssIndirSet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RDMA_RESET_LIF:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RDMA_CREATE_EQ:
        status = this->_CmdRDMACreateEQ(req, req_data, resp, resp_data);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RDMA_CREATE_CQ:
        status = this->_CmdRDMACreateCQ(req, req_data, resp, resp_data);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RDMA_CREATE_ADMINQ:
        status = this->_CmdRDMACreateAdminQ(req, req_data, resp, resp_data);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_V0_RDMA_CREATE_MR:
        status = this->_CmdCreateMR(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_V0_RDMA_CREATE_CQ:
        status = this->_CmdCreateCQ(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_V0_RDMA_CREATE_QP:
        status = this->_CmdCreateQP(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_V0_RDMA_MODIFY_QP:
        status = this->_CmdModifyQP(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_V0_RDMA_CREATE_AH:
        status = this->_CmdCreateAH(req, req_data, resp, resp_data);
        break;

    default:
        NIC_LOG_ERR("lif-{}: Unknown Opcode {}", hal_lif_info_.hw_lif_id,
            cmd->cmd.opcode);
        status = DEVCMD_UNKNOWN;
        break;
    }

    comp->comp.status = status;
    comp->comp.rsvd = 0xff;
    NIC_LOG_DEBUG("lif-{}-{}: Done cmd: {}, status: {}", hal_lif_info_.hw_lif_id,
                 spec->if_name,
                 opcode_to_str((enum cmd_opcode)cmd->cmd.opcode),
                 status);

    return (status);
}

enum DevcmdStatus
Eth::_CmdIdentify(void *req, void *req_data, void *resp, void *resp_data)
{
    union identity *rsp = (union identity *)resp_data;
    struct identify_comp *comp = (struct identify_comp *)resp;

    // TODO: Get these from hw
    rsp->dev.asic_type = 0x00;
    rsp->dev.asic_rev = 0xA0;
    sprintf((char *)&rsp->dev.serial_num, "naples");
    // TODO: Get this from sw
    sprintf((char *)&rsp->dev.fw_version, "v0.0.1");
    rsp->dev.nlifs = 1;
    // At least one ndbpgs_per_lif
    rsp->dev.ndbpgs_per_lif = MAX(spec->rdma_pid_count, 1);
    rsp->dev.ntxqs_per_lif = spec->txq_count;
    rsp->dev.nrxqs_per_lif = spec->rxq_count;
    rsp->dev.nintrs = spec->intr_count;
    rsp->dev.intr_coal_mult = 1;
    rsp->dev.intr_coal_div = 10;
    rsp->dev.nucasts_per_lif = 32;
    rsp->dev.nmcasts_per_lif = 32;

    // TODO: Split these into ethernet & rdma
    rsp->dev.nadminqs_per_lif = spec->adminq_count + spec->rdma_adminq_count;
    rsp->dev.neqs_per_lif = spec->eq_count + spec->rdma_eq_count;

    rsp->dev.nrdmasqs_per_lif = spec->rdma_sq_count;
    rsp->dev.nrdmarqs_per_lif = spec->rdma_rq_count;
    rsp->dev.nrdma_mrs_per_lif = spec->key_count;
    rsp->dev.nrdma_pts_per_lif = spec->pte_count;
    rsp->dev.nrdma_ahs_per_lif = spec->ah_count;
    rsp->dev.ncqs_per_lif = spec->rdma_cq_count;

    rsp->dev.rdma_version = 1;
    rsp->dev.rdma_qp_opcodes[0] = 27;
    rsp->dev.rdma_admin_opcodes[0] = 50;

    comp->ver = IDENTITY_VERSION_1;

    NIC_LOG_DEBUG("lif-{} asic_type {} asic_rev {} serial_num {} fw_version {} "
                 "ndbpgs_per_lif {} ntxqs_per_lif {} nrxqs_per_lif {} "
                 "nintrs {} intr_coal_mult {} intr_coal_div {} "
                 "nucasts_per_lif {} nmcasts_per_lif {} nadminqs_per_lif {} "
                 "neqs_per_lif {} nrdmasqs_per_lif {} nrdmarqs_per_lif {} "
                 "nrdma_mrs_per_lif {} nrdma_pts_per_lif {} "
                 "nrdma_ahs_per_lif {} ncqs_per_lif {} rdma_version {} "
                 "rdma_admin_opcodes {}",
                 hal_lif_info_.hw_lif_id,
                 rsp->dev.asic_type, rsp->dev.asic_rev, rsp->dev.serial_num,
                 rsp->dev.fw_version, rsp->dev.ndbpgs_per_lif,
                 rsp->dev.ntxqs_per_lif, rsp->dev.nrxqs_per_lif,
                 rsp->dev.nintrs, rsp->dev.intr_coal_mult,
                 rsp->dev.intr_coal_div, rsp->dev.nucasts_per_lif,
                 rsp->dev.nmcasts_per_lif, rsp->dev.nadminqs_per_lif,
                 rsp->dev.neqs_per_lif, rsp->dev.nrdmasqs_per_lif,
                 rsp->dev.nrdmarqs_per_lif, rsp->dev.nrdma_mrs_per_lif,
                 rsp->dev.nrdma_pts_per_lif, rsp->dev.nrdma_ahs_per_lif,
                 rsp->dev.ncqs_per_lif, rsp->dev.rdma_version,
                 rsp->dev.rdma_admin_opcodes[0]);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdReset(void *req, void *req_data, void *resp, void *resp_data)
{
    EthLif *eth_lif = NULL;

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_pba_clear(pci_resources.intrb + intr);
        intr_drvcfg(pci_resources.intrb + intr);
    }

    if (hal_lif_info_.pushed_to_hal) {
        // Clean up filters, Reset rx-modes
        eth_lif = hal->eth_lif_map[hal_lif_info_.id];
        if (!eth_lif) {
            NIC_LOG_ERR("lif-{}: Unable to find eth_lif", hal_lif_info_.hw_lif_id);
            return (DEVCMD_ERROR);
        }
        eth_lif->Reset();
    }

    // RSS configuration
    rss_type = LifRssType::RSS_TYPE_NONE;
    memset(rss_key, 0x00, RSS_HASH_KEY_SIZE);
    memset(rss_indir, 0x00, RSS_IND_TBL_SIZE);

    lif_state = LIF_STATE_RESET;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdHangNotify(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    eth_rx_qstate_t rx_qstate;
    eth_tx_qstate_t tx_qstate;
    admin_qstate_t adminq_qstate;
    intr_state_t intr_st;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_HANG_NOTIFY", hal_lif_info_.hw_lif_id);

    for (uint32_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_RX, qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&rx_qstate), sizeof(rx_qstate), 0);
        NIC_LOG_DEBUG("lif-{}: rxq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.hw_lif_id, qid,
            rx_qstate.p_index0, rx_qstate.c_index0, rx_qstate.comp_index,
            rx_qstate.intr_assert_index);
    }

    for (uint32_t qid = 0; qid < spec->txq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_TX, qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&tx_qstate), sizeof(tx_qstate), 0);
        NIC_LOG_DEBUG("lif-{}: txq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.hw_lif_id, qid,
            tx_qstate.p_index0, tx_qstate.c_index0, tx_qstate.comp_index,
            tx_qstate.intr_assert_index);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_ADMIN, qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&adminq_qstate), sizeof(adminq_qstate), 0);
        NIC_LOG_DEBUG("lif-{}: adminq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.hw_lif_id, qid,
            adminq_qstate.p_index0, adminq_qstate.c_index0, adminq_qstate.comp_index,
            adminq_qstate.intr_assert_index);
    }

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_state(pci_resources.intrb + intr, &intr_st);
        NIC_LOG_DEBUG("lif-{}: intr{}: fwcfg_lif {} fwcfg_function_mask {}"
            " drvcfg_mask {} drvcfg_int_credits {} drvcfg_mask_on_assert {}",
            hal_lif_info_.hw_lif_id, pci_resources.intrb + intr,
            intr_st.fwcfg_lif, intr_st.fwcfg_function_mask,
            intr_st.drvcfg_mask, intr_st.drvcfg_int_credits,
            intr_st.drvcfg_mask_on_assert);
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdLifInit(void *req, void *req_data, void *resp, void *resp_data)
{
    struct lif_init_cmd *cmd = (struct lif_init_cmd *)req;
    uint64_t addr;
    edma_qstate_t qstate;

    lif_state = LIF_STATE_INITING;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_LIF_INIT: lif_index {}, pushed_to_hal: {}",
                 hal_lif_info_.hw_lif_id,
                 cmd->index,
                 hal_lif_info_.pushed_to_hal);

    eid = 0;
    link_flap_count = 0;

    // Trigger Hal for Lif create if this is the first time
    if (!hal_lif_info_.pushed_to_hal) {
        uint64_t ret = hal->LifCreate(&hal_lif_info_);
        if (ret != 0) {
            NIC_LOG_ERR("Failed to create LIF");
            return (DEVCMD_ERROR);
        }
    }
    Update();

    // Clear all non-intrinsic fields
    for (uint32_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_RX, qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(eth_rx_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(eth_rx_qstate_t, p_index0), 0);
        invalidate_rxdma_cacheline(addr);
    }

    for (uint32_t qid = 0; qid < spec->txq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_TX, qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        WRITE_MEM(addr + offsetof(eth_tx_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(eth_tx_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(eth_tx_qstate_t, p_index0), 0);
        invalidate_txdma_cacheline(addr);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_ADMIN, qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        WRITE_MEM(addr + offsetof(admin_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(admin_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(admin_qstate_t, p_index0), 0);
        invalidate_txdma_cacheline(addr);
    }

    addr = GetQstateAddr(ETH_QTYPE_SVC, ETH_EDMAQ_ID);
    if (addr == 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for SVC qid {}",
            hal_lif_info_.hw_lif_id, ETH_EDMAQ_ID);
        return (DEVCMD_ERROR);
    }

    READ_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);
    uint8_t off;
    pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "edma_stage0", &off);
    qstate.pc_offset = off;
    qstate.cosA = 0;
    qstate.cosB = 0;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = 0;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.sta.color = 1;
    qstate.cfg.enable = 1;
    qstate.ring_base = edma_ring_base;
    qstate.ring_size = LG2_ETH_EDMAQ_RING_SIZE;
    qstate.cq_ring_base = edma_comp_base;
    qstate.cfg.intr_enable = 0;
    qstate.intr_assert_index = 0;
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    invalidate_txdma_cacheline(addr);

    lif_state = LIF_STATE_INITED;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdAdminQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct adminq_init_cmd *cmd = (struct adminq_init_cmd *)req;
    struct adminq_init_comp *comp = (struct adminq_init_comp *)resp;
    admin_qstate_t admin_qstate;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_ADMINQ_INIT: "
        "queue_index {} ring_base {} ring_size {} intr_index {}",
        hal_lif_info_.hw_lif_id,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index);

    if (cmd->index >= spec->adminq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}", hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", hal_lif_info_.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring size {}", hal_lif_info_.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = GetQstateAddr(ETH_QTYPE_ADMIN, cmd->index);
    if (addr == 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    READ_MEM(addr, (uint8_t *)&admin_qstate, sizeof(admin_qstate), 0);
    //NOTE: admin_qstate.cosA is ignored for Admin Queues. Db should ring on cosB.
    admin_qstate.cosA = 0;
    //NOTE: admin_qstate.cosB is set by HAL Lif create
    admin_qstate.host = 1;
    admin_qstate.total = 1;
    admin_qstate.pid = cmd->pid;
    admin_qstate.p_index0 = 0;
    admin_qstate.c_index0 = 0;
    admin_qstate.comp_index = 0;
    admin_qstate.ci_fetch = 0;
    admin_qstate.sta.color = 1;
    admin_qstate.cfg.enable = 1;
    admin_qstate.cfg.host_queue = spec->host_dev;
    admin_qstate.cfg.intr_enable = 1;
    if (spec->host_dev)
        admin_qstate.ring_base = (1ULL << 63) | (hal_lif_info_.hw_lif_id << 52) | cmd->ring_base;
    else
        admin_qstate.ring_base = cmd->ring_base;
    admin_qstate.ring_size = cmd->ring_size;
    admin_qstate.cq_ring_base = roundup(admin_qstate.ring_base + (sizeof(union adminq_cmd) << cmd->ring_size), 4096);
    admin_qstate.intr_assert_index = pci_resources.intrb + cmd->intr_index;
    if (nicmgr_lif_info) {
        admin_qstate.nicmgr_qstate_addr = nicmgr_lif_info->qstate_addr[NICMGR_QTYPE_REQ];
    }
    WRITE_MEM(addr, (uint8_t *)&admin_qstate, sizeof(admin_qstate), 0);

    invalidate_txdma_cacheline(addr);

    comp->qid = cmd->index;
    comp->qtype = ETH_QTYPE_ADMIN;

    NIC_LOG_DEBUG("lif-{}: qid {} qtype {}",
                 hal_lif_info_.hw_lif_id, comp->qid, comp->qtype);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdTxQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct txq_init_cmd *cmd = (struct txq_init_cmd *)req;
    struct txq_init_comp *comp = (struct txq_init_comp *)resp;
    eth_tx_qstate_t tx_qstate;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_TXQ_INIT: "
        "queue_index {} cos {} ring_base {} ring_size {} intr_index {} {}{}",
        hal_lif_info_.hw_lif_id,
        cmd->index,
        cmd->cos,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        cmd->I ? 'I' : '-',
        cmd->E ? 'E' : '-');

    if (cmd->index >= spec->txq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}", hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", hal_lif_info_.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring_size {}", hal_lif_info_.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = GetQstateAddr(ETH_QTYPE_TX, cmd->index);
    if (addr == 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    READ_MEM(addr, (uint8_t *)&tx_qstate, sizeof(tx_qstate), 0);
    tx_qstate.cosA = cmd->cos;
    //NOTE: tx_qstate.cosB is ignored for TX queues.
    tx_qstate.host = 1;
    tx_qstate.total = 1;
    tx_qstate.pid = cmd->pid;
    tx_qstate.p_index0 = 0;
    tx_qstate.c_index0 = 0;
    tx_qstate.comp_index = 0;
    tx_qstate.ci_fetch = 0;
    tx_qstate.ci_miss = 0;
    tx_qstate.sta.color = 1;
    tx_qstate.sta.spec_miss = 0;
    tx_qstate.cfg.enable = cmd->E;
    tx_qstate.cfg.host_queue = spec->host_dev;
    tx_qstate.cfg.intr_enable = cmd->E;
    if (spec->host_dev)
        tx_qstate.ring_base = (1ULL << 63) | (hal_lif_info_.hw_lif_id << 52) | cmd->ring_base;
    else
        tx_qstate.ring_base = cmd->ring_base;
    tx_qstate.ring_size = cmd->ring_size;
    tx_qstate.cq_ring_base = roundup(tx_qstate.ring_base + (sizeof(struct txq_desc) << cmd->ring_size), 4096);
    tx_qstate.intr_assert_index = pci_resources.intrb + cmd->intr_index;
    tx_qstate.sg_ring_base = roundup(tx_qstate.cq_ring_base + (sizeof(struct txq_comp) << cmd->ring_size), 4096);
    tx_qstate.spurious_db_cnt = 0;
    WRITE_MEM(addr, (uint8_t *)&tx_qstate, sizeof(tx_qstate), 0);

    invalidate_txdma_cacheline(addr);

    comp->qid = cmd->index;
    comp->qtype = ETH_QTYPE_TX;

    NIC_LOG_DEBUG("lif-{}: qid {} qtype {}",
                 hal_lif_info_.hw_lif_id, comp->qid, comp->qtype);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRxQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct rxq_init_cmd *cmd = (struct rxq_init_cmd *)req;
    struct rxq_init_comp *comp = (struct rxq_init_comp *)resp;
    eth_rx_qstate_t rx_qstate;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RXQ_INIT: "
        "queue_index {} ring_base {} ring_size {} intr_index {} {}{}",
        hal_lif_info_.hw_lif_id,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        cmd->I ? 'I' : '-',
        cmd->E ? 'E' : '-');

    if (cmd->index >= spec->rxq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}", hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", hal_lif_info_.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring_size {}", hal_lif_info_.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = GetQstateAddr(ETH_QTYPE_RX, cmd->index);
    if (addr == 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    READ_MEM(addr, (uint8_t *)&rx_qstate, sizeof(rx_qstate), 0);
    rx_qstate.cosA = 0;
    rx_qstate.cosB = 0;
    rx_qstate.host = 1;
    rx_qstate.total = 1;
    rx_qstate.pid = cmd->pid;
    rx_qstate.p_index0 = 0;
    rx_qstate.c_index0 = 0;
    rx_qstate.comp_index = 0;
    rx_qstate.sta.color = 1;
    rx_qstate.cfg.enable = cmd->E;
    rx_qstate.cfg.host_queue = spec->host_dev;
    rx_qstate.cfg.intr_enable = cmd->E;
    if (spec->host_dev)
        rx_qstate.ring_base = (1ULL << 63) | (hal_lif_info_.hw_lif_id << 52) | cmd->ring_base;
    else
        rx_qstate.ring_base = cmd->ring_base;
    rx_qstate.ring_size = cmd->ring_size;
    rx_qstate.cq_ring_base = roundup(rx_qstate.ring_base + (sizeof(struct rxq_desc) << cmd->ring_size), 4096);
    rx_qstate.intr_assert_index = pci_resources.intrb + cmd->intr_index;
    WRITE_MEM(addr, (uint8_t *)&rx_qstate, sizeof(rx_qstate), 0);

    invalidate_rxdma_cacheline(addr);

    comp->qid = cmd->index;
    comp->qtype = ETH_QTYPE_RX;

    NIC_LOG_DEBUG("lif-{}: qid {} qtype {}",
                 hal_lif_info_.hw_lif_id, comp->qid, comp->qtype);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdNotifyQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr, host_ring_base;
    notify_qstate_t qstate;
    struct notifyq_init_cmd *cmd = (struct notifyq_init_cmd *)req;
    struct notifyq_init_comp *comp = (struct notifyq_init_comp *)resp;

    NIC_LOG_INFO("lif-{}: CMD_OPCODE_NOTIFYQ_INIT: "
        "queue_index {} ring_base {} ring_size {} intr_index {} notify_base {:#x}",
        hal_lif_info_.hw_lif_id,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        cmd->notify_base);

    if (cmd->index >= spec->eq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}", hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", hal_lif_info_.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring_size {}", hal_lif_info_.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = GetQstateAddr(ETH_QTYPE_SVC, cmd->index);
    if (addr == 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for SVC qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    READ_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);
    qstate.cosA = 0;
    qstate.cosB = 0;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.host_pindex = 0;
    qstate.sta = {0};
    qstate.cfg.enable = 1;
    qstate.cfg.host_queue = spec->host_dev;
    qstate.cfg.intr_enable = 1;
    qstate.ring_base = notify_ring_base;
    qstate.ring_size = LG2_ETH_NOTIFYQ_RING_SIZE;
    if (spec->host_dev)
        host_ring_base = (1ULL << 63) | (hal_lif_info_.hw_lif_id << 52) | cmd->ring_base;
    else
        host_ring_base = cmd->ring_base;
    qstate.host_ring_base = roundup(host_ring_base + (sizeof(notifyq_comp) << cmd->ring_size), 4096);
    qstate.host_ring_size = cmd->ring_size;
    qstate.host_intr_assert_index = pci_resources.intrb + cmd->intr_index;
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    host_notify_block_addr = cmd->notify_base;

    invalidate_txdma_cacheline(addr);

    comp->qid = cmd->index;
    comp->qtype = ETH_QTYPE_SVC;

    NIC_LOG_INFO("lif-{}: qid {} qtype {}",
                 hal_lif_info_.hw_lif_id, comp->qid, comp->qtype);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdFeatures(void *req, void *req_data, void *resp, void *resp_data)
{
    struct features_cmd *cmd = (struct features_cmd *)req;
    struct features_comp *comp = (struct features_comp *)resp;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_FEATURES: wanted "
        "vlan_strip {} vlan_insert {} rx_csum {} tx_csum {} rx_hash {} sg {}",
        hal_lif_info_.hw_lif_id,
        (cmd->wanted & ETH_HW_VLAN_RX_STRIP) ? 1 : 0,
        (cmd->wanted & ETH_HW_VLAN_TX_TAG) ? 1 : 0,
        (cmd->wanted & ETH_HW_RX_CSUM) ? 1 : 0,
        (cmd->wanted & ETH_HW_TX_CSUM) ? 1 : 0,
        (cmd->wanted & ETH_HW_RX_HASH) ? 1 : 0,
        (cmd->wanted & ETH_HW_TX_SG)  ? 1 : 0
    );

    comp->status = 0;
    if (hal->fwd_mode == FWD_MODE_SMART_NIC) {
        comp->supported = (
            //ETH_HW_VLAN_RX_STRIP |
            //ETH_HW_VLAN_TX_TAG |
            ETH_HW_RX_CSUM |
            ETH_HW_TX_CSUM |
            ETH_HW_RX_HASH |
            ETH_HW_TX_SG |
            ETH_HW_TSO |
            ETH_HW_TSO_IPV6
        );
    } else {
        comp->supported = (
            ETH_HW_VLAN_RX_STRIP |
            ETH_HW_VLAN_TX_TAG |
            ETH_HW_VLAN_RX_FILTER |
            ETH_HW_RX_CSUM |
            ETH_HW_TX_CSUM |
            ETH_HW_RX_HASH |
            ETH_HW_TX_SG |
            ETH_HW_TSO |
            ETH_HW_TSO_IPV6
        );
    }

    hal->LifSetVlanOffload(spec->lif_id,
        cmd->wanted & comp->supported & ETH_HW_VLAN_RX_STRIP,
        cmd->wanted & comp->supported & ETH_HW_VLAN_TX_TAG);
    NIC_LOG_DEBUG("lif-{}: supported {}",
                 hal_lif_info_.hw_lif_id, comp->supported);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdQEnable(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct q_enable_cmd *cmd = (struct q_enable_cmd *)req;
    // q_enable_comp *comp = (q_enable_comp *)resp;
    struct eth_rx_cfg_qstate rx_cfg = {0};
    struct eth_tx_cfg_qstate tx_cfg = {0};
    struct admin_cfg_qstate admin_cfg = {0};

    if (cmd->qtype >= 8) {
        NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qtype {}",
        hal_lif_info_.hw_lif_id, cmd->qtype);
        return (DEVCMD_ERROR);
    }

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_Q_ENABLE: type {} qid {}",
        hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);

    switch (cmd->qtype) {
    case ETH_QTYPE_RX:
        if (cmd->qid >= spec->rxq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
            hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_RX, cmd->qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        rx_cfg.enable = 0x1;
        rx_cfg.host_queue = spec->host_dev;
        rx_cfg.intr_enable = 0x1;
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, cfg), (uint8_t *)&rx_cfg, sizeof(rx_cfg), 0);
        invalidate_rxdma_cacheline(addr);
        break;
    case ETH_QTYPE_TX:
        if (cmd->qid >= spec->txq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
                   hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_TX, cmd->qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        tx_cfg.enable = 0x1;
        tx_cfg.host_queue = spec->host_dev;
        tx_cfg.intr_enable = 0x1;
        WRITE_MEM(addr + offsetof(eth_tx_qstate_t, cfg), (uint8_t *)&tx_cfg, sizeof(tx_cfg), 0);
        invalidate_txdma_cacheline(addr);
        break;
    case ETH_QTYPE_ADMIN:
        if (cmd->qid >= spec->adminq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
                   hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_ADMIN, cmd->qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        admin_cfg.enable = 0x1;
        admin_cfg.host_queue = spec->host_dev;
        admin_cfg.intr_enable = 0x1;
        WRITE_MEM(addr + offsetof(admin_qstate_t, cfg), (uint8_t *)&admin_cfg, sizeof(admin_cfg), 0);
        invalidate_txdma_cacheline(addr);
        break;
    default:
        return (DEVCMD_ERROR);
        break;
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdQDisable(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct q_disable_cmd *cmd = (struct q_disable_cmd *)req;
    // q_disable_comp *comp = (q_disable_comp *)resp;
    struct eth_rx_cfg_qstate rx_cfg = {0};
    struct eth_tx_cfg_qstate tx_cfg = {0};
    struct admin_cfg_qstate admin_cfg = {0};

    if (cmd->qtype >= 8) {
        NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_DISABLE: bad qtype {}",
        hal_lif_info_.hw_lif_id, cmd->qtype);
        return (DEVCMD_ERROR);
    }

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_Q_DISABLE: type {} qid {}",
        hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);

    switch (cmd->qtype) {
    case ETH_QTYPE_RX:
        if (cmd->qid >= spec->rxq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
            hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_RX, cmd->qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        rx_cfg.enable = 0x0;
        rx_cfg.host_queue = spec->host_dev;
        rx_cfg.intr_enable = 0x0;
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, cfg), (uint8_t *)&rx_cfg, sizeof(rx_cfg), 0);
        invalidate_rxdma_cacheline(addr);
        break;
    case ETH_QTYPE_TX:
        if (cmd->qid >= spec->txq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
                   hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_TX, cmd->qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        tx_cfg.enable = 0x0;
        tx_cfg.host_queue = spec->host_dev;
        tx_cfg.intr_enable = 0x0;
        WRITE_MEM(addr + offsetof(eth_tx_qstate_t, cfg), (uint8_t *)&tx_cfg, sizeof(tx_cfg), 0);
        invalidate_txdma_cacheline(addr);
        break;
    case ETH_QTYPE_ADMIN:
        if (cmd->qid >= spec->adminq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
                   hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_ADMIN, cmd->qid);
        if (addr == 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        admin_cfg.enable = 0x0;
        admin_cfg.host_queue = spec->host_dev;
        admin_cfg.intr_enable = 0x0;
        WRITE_MEM(addr + offsetof(admin_qstate_t, cfg), (uint8_t *)&admin_cfg, sizeof(admin_cfg), 0);
        invalidate_txdma_cacheline(addr);
        break;
    default:
        return (DEVCMD_ERROR);
        break;
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdSetMode(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rx_mode_set_cmd *cmd = (struct rx_mode_set_cmd *)req;
    // rx_mode_set_comp *comp = (rx_mode_set_comp *)resp;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RX_MODE_SET: rx_mode {} {}{}{}{}{}",
            hal_lif_info_.hw_lif_id,
            cmd->rx_mode,
            cmd->rx_mode & RX_MODE_F_UNICAST   ? 'u' : '-',
            cmd->rx_mode & RX_MODE_F_MULTICAST ? 'm' : '-',
            cmd->rx_mode & RX_MODE_F_BROADCAST ? 'b' : '-',
            cmd->rx_mode & RX_MODE_F_PROMISC   ? 'p' : '-',
            cmd->rx_mode & RX_MODE_F_ALLMULTI  ? 'a' : '-');

    hal->LifSetFilterMode(spec->lif_id,
        cmd->rx_mode & RX_MODE_F_BROADCAST,
        cmd->rx_mode & RX_MODE_F_ALLMULTI,
        cmd->rx_mode & RX_MODE_F_PROMISC);

    return (DEVCMD_SUCCESS);
}
enum DevcmdStatus
Eth::_CmdRxFilterAdd(void *req, void *req_data, void *resp, void *resp_data)
{
    //int status;
    uint64_t mac_addr;
    uint16_t vlan;
    uint32_t filter_id = 0;
    struct rx_filter_add_cmd *cmd = (struct rx_filter_add_cmd *)req;
    struct rx_filter_add_comp *comp = (struct rx_filter_add_comp *)resp;
    EthLif *eth_lif = NULL;

    eth_lif = hal->eth_lif_map[hal_lif_info_.id];
    if (cmd->match == RX_FILTER_MATCH_MAC) {

        memcpy((uint8_t *)&mac_addr, (uint8_t *)&cmd->mac.addr, sizeof(cmd->mac.addr));
        mac_addr = be64toh(mac_addr) >> (8 * sizeof(mac_addr) - 8 * sizeof(cmd->mac.addr));

        NIC_LOG_DEBUG("lif-{}: Add RX_FILTER_MATCH_MAC mac:{}",
                hal_lif_info_.id, macaddr2str(mac_addr));

        eth_lif->AddMac(mac_addr);

        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate MAC address filter");
            return (DEVCMD_ERROR);
        }
        mac_addrs[filter_id] = mac_addr;
    } else if (cmd->match == RX_FILTER_MATCH_VLAN) {
        vlan = cmd->vlan.vlan;

        NIC_LOG_DEBUG("lif-{}: Add RX_FILTER_MATCH_VLAN vlan {}",
                      hal_lif_info_.id, vlan);
        eth_lif->AddVlan(vlan);

        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate VLAN filter");
            return (DEVCMD_ERROR);
        }
        vlans[filter_id] = vlan;
    } else {
        memcpy((uint8_t *)&mac_addr, (uint8_t *)&cmd->mac_vlan.addr, sizeof(cmd->mac_vlan.addr));
        mac_addr = be64toh(mac_addr) >> (8 * sizeof(mac_addr) - 8 * sizeof(cmd->mac_vlan.addr));
        vlan = cmd->mac_vlan.vlan;

        NIC_LOG_DEBUG("lif-{}: Add RX_FILTER_MATCH_MAC_VLAN mac: {}, vlan: {}",
                      hal_lif_info_.id, macaddr2str(mac_addr), vlan);

        eth_lif->AddMacVlan(mac_addr, vlan);

        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate VLAN filter");
            return (DEVCMD_ERROR);
        }
        mac_vlans[filter_id] = std::make_tuple(mac_addr, vlan);
    }

    comp->filter_id = filter_id;
    NIC_LOG_DEBUG("lif-{}: filter_id {}",
                 hal_lif_info_.hw_lif_id, comp->filter_id);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRxFilterDel(void *req, void *req_data, void *resp, void *resp_data)
{
    //int status;
    uint64_t mac_addr;
    uint16_t vlan;
    struct rx_filter_del_cmd *cmd = (struct rx_filter_del_cmd *)req;
    //struct rx_filter_del_comp *comp = (struct rx_filter_del_comp *)resp;
    EthLif *eth_lif = NULL;

    eth_lif = hal->eth_lif_map[hal_lif_info_.id];
    if (eth_lif == NULL) {
        NIC_LOG_ERR("Unable to find eth_lif for id: {}", hal_lif_info_.id);
        return DEVCMD_ERROR;
    }

    if (mac_addrs.find(cmd->filter_id) != mac_addrs.end()) {
        mac_addr = mac_addrs[cmd->filter_id];
        NIC_LOG_DEBUG("lif-{}: Del RX_FILTER_MATCH_MAC mac:{}",
                      hal_lif_info_.hw_lif_id,
                      macaddr2str(mac_addr));
        eth_lif->DelMac(mac_addr);
        mac_addrs.erase(cmd->filter_id);
    } else if (vlans.find(cmd->filter_id) != vlans.end()) {
        vlan = vlans[cmd->filter_id];
        NIC_LOG_DEBUG("lif-{}: Del RX_FILTER_MATCH_VLAN vlan {}",
                     hal_lif_info_.hw_lif_id, vlan);
        eth_lif->DelVlan(vlan);
        vlans.erase(cmd->filter_id);
    } else if (mac_vlans.find(cmd->filter_id) != mac_vlans.end()) {
        auto mac_vlan = mac_vlans[cmd->filter_id];
        mac_addr = std::get<0>(mac_vlan);
        vlan = std::get<1>(mac_vlan);
        NIC_LOG_DEBUG("lif-{}: Del RX_FILTER_MATCH_MAC_VLAN mac: {}, vlan: {}",
                     hal_lif_info_.hw_lif_id, macaddr2str(mac_addr), vlan);
        eth_lif->DelMacVlan(mac_addr, vlan);
        mac_vlans.erase(cmd->filter_id);
    } else {
        NIC_LOG_ERR("Invalid filter id {}", cmd->filter_id);
        return (DEVCMD_ERROR);
    }

    return (DEVCMD_SUCCESS);
}


enum DevcmdStatus
Eth::_CmdMacAddrGet(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t mac_addr;

    //struct station_mac_addr_get_cmd *cmd = (struct station_mac_addr_get_cmd *)req;
    struct station_mac_addr_get_comp *comp = (struct station_mac_addr_get_comp *)resp;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_STATION_MAC_ADDR_GET", hal_lif_info_.hw_lif_id);

    mac_addr = be64toh(spec->mac_addr) >> (8 * sizeof(spec->mac_addr) - 8 * sizeof(uint8_t[6]));
    memcpy((uint8_t *)comp->addr, (uint8_t *)&mac_addr, sizeof(comp->addr));

    NIC_LOG_DEBUG("lif-{}: station mac address {:#x}", hal_lif_info_.hw_lif_id, mac_addr);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdStatsDumpStart(void *req, void *req_data, void *resp, void *resp_data)
{
    struct stats_dump_cmd *cmd = (struct stats_dump_cmd *)req;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_STATS_DUMP_START: host_stats_mem_addr {:#x}",
        hal_lif_info_.hw_lif_id, host_stats_mem_addr);

    host_stats_mem_addr = cmd->addr;

    Eth::StatsUpdateHandler(this);

    evutil_timer_start(&stats_timer, &Eth::StatsUpdateHandler, this, 1, 1);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdStatsDumpStop(void *req, void *req_data, void *resp, void *resp_data)
{
    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_STATS_DUMP_STOP", hal_lif_info_.hw_lif_id);

    if (host_stats_mem_addr == 0) {
        return (DEVCMD_SUCCESS);
    }

    evutil_timer_stop(&stats_timer);

    host_stats_mem_addr = 0;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRssHashSet(void *req, void *req_data, void *resp, void *resp_data)
{
    int ret;
    struct rss_hash_set_cmd *cmd = (struct rss_hash_set_cmd *)req;
    //rss_hash_set_comp *comp = (struct rss_hash_set_comp *)resp;

    rss_type = cmd->types;
    memcpy(rss_key, cmd->key, RSS_HASH_KEY_SIZE);

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RSS_HASH_SET: type {:#x} key {} table {}",
        hal_lif_info_.hw_lif_id, rss_type, rss_key, rss_indir);

    ret = pd->eth_program_rss(hal_lif_info_.hw_lif_id, rss_type, rss_key, rss_indir,
                              spec->rxq_count);
    if (ret != 0) {
        NIC_LOG_DEBUG("_CmdRssHashSet:{}: Unable to program hw for RSS HASH", ret);
        return (DEVCMD_ERROR);
    }

    return DEVCMD_SUCCESS;
}

enum DevcmdStatus
Eth::_CmdRssIndirSet(void *req, void *req_data, void *resp, void *resp_data)
{
    int ret;
    //struct rss_indir_set_cmd *cmd = (struct rss_indir_set_cmd *)req;
    //rss_indir_set_comp *comp = (struct rss_indir_set_comp *)resp;

    memcpy(rss_indir, req_data, RSS_IND_TBL_SIZE);
    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RSS_INDIR_SET: type {:#x} key {} table {}",
        hal_lif_info_.hw_lif_id, rss_type, rss_key, rss_indir);

    for (int i = 0; i < RSS_IND_TBL_SIZE; i++) {
        if (((uint8_t *)req_data)[i] > spec->rxq_count) {
            NIC_LOG_ERR("lif-{}: Invalid indirection table entry index %d qid %d",
                i, ((uint8_t *)req_data)[i]);
            return (DEVCMD_ERROR);
        }
    }

    ret = pd->eth_program_rss(hal_lif_info_.hw_lif_id, rss_type, rss_key, rss_indir,
                          spec->rxq_count);
    if (ret != 0) {
        NIC_LOG_ERR("_CmdRssIndirSet:{}: Unable to program hw for RSS INDIR", ret);
        return (DEVCMD_ERROR);
    }

    return (DEVCMD_SUCCESS);
}

/**
 * CMD_OPCODE_V0_RDMA_* ops
 */

enum DevcmdStatus
Eth::_CmdCreateMR(void *req, void *req_data, void *resp, void *resp_data)
{
    struct create_mr_cmd  *cmd = (struct create_mr_cmd *) req;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_V0_CREATE_MR:"
            " pd_num {} start {:#x} len {} access_flags {:#x}"
            " lkey {} rkey {} "
            " page_size {} pt_size {}",
            hal_lif_info_.hw_lif_id + cmd->lif,
            cmd->pd_num, cmd->start, cmd->length, cmd->access_flags,
            cmd->lkey, cmd->rkey,
            cmd->page_size, cmd->nchunks);

    hal->CreateMR(hal_lif_info_.hw_lif_id + cmd->lif,
            cmd->pd_num, cmd->start, cmd->length, cmd->access_flags,
            cmd->lkey, cmd->rkey, cmd->page_size,
            (uint64_t *)devcmd->data, cmd->nchunks);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdCreateCQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct create_cq_cmd  *cmd = (struct create_cq_cmd  *) req;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_V0_CREATE_CQ "
                 "cq_num {} wqe_size {} num_wqes {} host_pg_size {}",
                 hal_lif_info_.hw_lif_id + cmd->lif_id, cmd->cq_num,
                 cmd->cq_wqe_size, cmd->num_cq_wqes,
                 cmd->host_pg_size);

    hal->CreateCQ(hal_lif_info_.hw_lif_id + cmd->lif_id,
            cmd->cq_num, cmd->cq_wqe_size, cmd->num_cq_wqes,
            cmd->host_pg_size,
            (uint64_t *)devcmd->data, cmd->pt_size);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdCreateQP(void *req, void *req_data, void *resp, void *resp_data)
{
    struct create_qp_cmd  *cmd = (struct create_qp_cmd  *) req;
    uint64_t *pt_table = (uint64_t *)&devcmd->data;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_V0_CREATE_QP:"
            " qp_num {} sq_wqe_size {}"
            " rq_wqe_size {} num_sq_wqes {}"
            " num_rq_wqes {} pd {}"
            " sq_cq_num {} rq_cq_num {} page_size {}"
            " pmtu {} service {} sq_pt_size {} pt_size {}",
            hal_lif_info_.hw_lif_id + cmd->lif_id,
            cmd->qp_num, cmd->sq_wqe_size,
            cmd->rq_wqe_size, cmd->num_sq_wqes,
            cmd->num_rq_wqes, cmd->pd,
            cmd->sq_cq_num, cmd->rq_cq_num, cmd->host_pg_size,
            cmd->pmtu, cmd->service, cmd->sq_pt_size, cmd->pt_size);

    hal->CreateQP(hal_lif_info_.hw_lif_id + cmd->lif_id,
                  cmd->qp_num, cmd->sq_wqe_size,
                  cmd->rq_wqe_size, cmd->num_sq_wqes,
                  cmd->num_rq_wqes, 256, 256, cmd->pd,
                  cmd->sq_cq_num, cmd->rq_cq_num, cmd->host_pg_size,
                  cmd->pmtu, cmd->service, cmd->flags,
                  cmd->sq_pt_size, cmd->pt_size, pt_table,
                  pci_resources.cmbpa, pci_resources.cmbsz);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdModifyQP(void *req, void *req_data, void *resp, void *resp_data)
{
    struct modify_qp_cmd  *cmd = (struct modify_qp_cmd  *) req;
    unsigned char *header = (unsigned char *)&devcmd->data;

    NIC_LOG_DEBUG("lif-{}: qp_num {} attr_mask {:#x}"
          " dest_qp_num {} q_key {}"
          " e_psn {} sq_psn {}"
          " header_template_ah_id {} header_template_size {}"
          " path_mtu {}. state: {}",
          hal_lif_info_.hw_lif_id,
          cmd->qp_num, cmd->attr_mask,
          cmd->dest_qp_num, cmd->q_key,
          cmd->e_psn, cmd->sq_psn,
          cmd->header_template_ah_id, cmd->header_template_size,
          cmd->path_mtu, cmd->state);

    hal->ModifyQP(hal_lif_info_.hw_lif_id + cmd->lif_id,
                  cmd->qp_num, cmd->attr_mask,
                  cmd->dest_qp_num, cmd->q_key,
                  cmd->e_psn, cmd->sq_psn,
                  cmd->header_template_ah_id, cmd->header_template_size,
                  header, cmd->path_mtu, cmd->state & 7);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdCreateAH(void *req, void *req_data, void *resp, void *resp_data)
{
    struct create_ah_cmd  *cmd = (struct create_ah_cmd  *) req;
    unsigned char *header = (unsigned char *)&devcmd->data;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_V0_CREATE_AH:"
            " ah_id {} pd_id {}"
            " header_template_size {}",
            hal_lif_info_.hw_lif_id,
            cmd->ah_id, cmd->pd_id,
            cmd->header_template_size);

    hal->CreateAh(hal_lif_info_.hw_lif_id,
                  cmd->ah_id, cmd->pd_id,
                  cmd->header_template_size, header);

    return (DEVCMD_SUCCESS);
}

/*
 * CMD_OPCODE_RDMA_* ops
 */
enum DevcmdStatus
Eth::_CmdRDMACreateEQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd  *cmd = (struct rdma_queue_cmd  *) req;
    eqcb_t       eqcb;
    uint64_t addr;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RDMA_CREATE_EQ "
                 "qid {} depth_log2 {} "
                 "stride_log2 {} dma_addr {} "
                 "cid {}", hal_lif_info_.hw_lif_id + cmd->lif_id, cmd->qid_ver,
                 1u << cmd->depth_log2, 1u << cmd->stride_log2,
                 cmd->dma_addr, cmd->cid);

    memset(&eqcb, 0, sizeof(eqcb_t));
    // EQ does not need scheduling, so set one less (meaning #rings as zero)
    eqcb.ring_header.total_rings = MAX_EQ_RINGS - 1;
    eqcb.eqe_base_addr = cmd->dma_addr | (1UL << 63) | ((uint64_t)(hal_lif_info_.hw_lif_id + cmd->lif_id) << 52);
    eqcb.log_wqe_size = cmd->stride_log2;
    eqcb.log_num_wqes = cmd->depth_log2;
    eqcb.int_enabled = 1;
    //eqcb.int_num = spec.int_num();
    eqcb.eq_id = cmd->cid;
    eqcb.color = 0;

    eqcb.int_assert_addr = intr_assert_addr(pci_resources.intrb + cmd->cid);

    memrev((uint8_t*)&eqcb, sizeof(eqcb_t));

    addr = GetQstateAddr(ETH_QTYPE_EQ, cmd->qid_ver);
    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RDMA_CREATE_EQ "
                 "QstateAddr = {:#x}",
                 hal_lif_info_.hw_lif_id + cmd->lif_id,
                 addr);

    if (addr == 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for EQ qid {}",
                    hal_lif_info_.hw_lif_id+ cmd->lif_id, cmd->qid_ver);
        return (DEVCMD_ERROR);
    }
    WRITE_MEM(addr, (uint8_t *)&eqcb, sizeof(eqcb), 0);
    invalidate_rxdma_cacheline(addr);
    invalidate_txdma_cacheline(addr);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRDMACreateCQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd *cmd = (struct rdma_queue_cmd *) req;
    uint32_t               lif = hal_lif_info_.hw_lif_id + cmd->lif_id;
    uint32_t               num_cq_wqes, cqwqe_size;
    cqcb_t                 cqcb;
    uint8_t                offset;
    int                    ret;

#if 0
    hal->RDMACreateCQ(hal_lif_info_.hw_lif_id + cmd->lif_id,
                      cmd->qid_ver, 1u << cmd->stride_log2, 1u << cmd->depth_log2,
                      1ull << (cmd->stride_log2 + cmd->depth_log2),
                      cmd->dma_addr, cmd->cid);
#endif

    NIC_LOG_DEBUG("--------------------- API Start ------------------------");
    NIC_LOG_DEBUG("PI-LIF:{}: RDMA CQ Create for lif {}", __FUNCTION__, lif);

    NIC_LOG_DEBUG("{}: Inputs: cq_num: {} cq_wqe_size: {} num_cq_wqes: {} "
                  "eq_id: {} hostmem_pg_size: {} ",
                  __FUNCTION__, cmd->qid_ver,
                  1u << cmd->stride_log2, 1u << cmd->depth_log2,
                  cmd->cid, 1ull << (cmd->stride_log2 + cmd->depth_log2));

    cqwqe_size = 1u << cmd->stride_log2;
    num_cq_wqes = 1u << cmd->depth_log2;

    NIC_LOG_DEBUG("cqwqe_size: {} num_cq_wqes: {}", cqwqe_size, num_cq_wqes);

    memset(&cqcb, 0, sizeof(cqcb_t));
    cqcb.ring_header.total_rings = MAX_CQ_RINGS;
    cqcb.ring_header.host_rings = MAX_CQ_HOST_RINGS;

    int32_t cq_pt_index = cmd->xxx_table_index;
    uint64_t  pt_table_base_addr = pd->rdma_get_pt_base_addr(lif);

    cqcb.pt_base_addr = pt_table_base_addr >> PT_BASE_ADDR_SHIFT;
    cqcb.log_cq_page_size = cmd->stride_log2 + cmd->depth_log2;
    cqcb.log_wqe_size = log2(cqwqe_size);
    cqcb.log_num_wqes = log2(num_cq_wqes);
    cqcb.cq_id = cmd->qid_ver;
    cqcb.eq_id = cmd->cid;
    cqcb.host_addr = 1;

    cqcb.pt_pa = cmd->dma_addr;
    if (cqcb.host_addr) {
        cqcb.pt_pa |= ((1UL << 63)  | (uint64_t)lif << 52);
    }

    cqcb.pt_pg_index = 0;
    cqcb.pt_next_pg_index = 0x1FF;

    int log_num_pages = cqcb.log_num_wqes + cqcb.log_wqe_size - cqcb.log_cq_page_size;
    NIC_LOG_DEBUG("{}: LIF: {}: pt_pa: {:#x}: pt_next_pa: {:#x}: pt_pa_index: {}: pt_next_pa_index: {}: log_num_pages: {}", __FUNCTION__, lif, cqcb.pt_pa, cqcb.pt_next_pa, cqcb.pt_pg_index, cqcb.pt_next_pg_index, log_num_pages);

    /* store  pt_pa & pt_next_pa in little endian. So need an extra memrev */
    memrev((uint8_t*)&cqcb.pt_pa, sizeof(uint64_t));

    ret = pd->lm_->GetPCOffset("p4plus", "rxdma_stage0.bin", "rdma_cq_rx_stage0", &offset);
    if (ret < 0) {
        NIC_LOG_ERR("Failed to get PC offset : {} for prog: {}, label: {}", ret, "rxdma_stage0.bin", "rdma_cq_rx_stage0");
        return DEVCMD_ERROR;
    }

    cqcb.ring_header.pc = offset;

    // write to hardware
    NIC_LOG_DEBUG("{}: LIF: {}: Writting initial CQCB State, "
                  "CQCB->PT: {:#x} cqcb_size: {}",
                  __FUNCTION__, lif, cqcb.pt_base_addr, sizeof(cqcb_t));
    // Convert data before writting to HBM
    memrev((uint8_t*)&cqcb, sizeof(cqcb_t));

    uint64_t addr = GetQstateAddr(ETH_QTYPE_CQ, cmd->qid_ver);;
    NIC_LOG_DEBUG("{}: QstateAddr = {:#x}", __FUNCTION__, addr);
    if (addr == 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for CQ qid {}",
                    lif, cmd->qid_ver);
        return DEVCMD_ERROR;
    }
    WRITE_MEM(addr, (uint8_t *)&cqcb, sizeof(cqcb), 0);

    uint64_t pt_table_addr = pt_table_base_addr+cq_pt_index*sizeof(uint64_t);

    // There is only one page table entry for adminq CQ
    WRITE_MEM(pt_table_addr, (uint8_t *)&cmd->dma_addr, sizeof(uint64_t), 0);
    NIC_LOG_DEBUG("PT Entry Write: Lif {}: CQ PT Idx: {} PhyAddr: {:#x}",
                  lif, cq_pt_index, cmd->dma_addr);
    invalidate_rxdma_cacheline(pt_table_addr);
    invalidate_txdma_cacheline(pt_table_addr);

    NIC_LOG_DEBUG("----------------------- API End ------------------------");

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRDMACreateAdminQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd  *cmd = (struct rdma_queue_cmd  *) req;
    int                    ret;
    uint32_t     lif = hal_lif_info_.hw_lif_id + cmd->lif_id;
    aqcb_t       aqcb;
    uint8_t     offset;

    NIC_LOG_DEBUG("--------------------- API Start ------------------------");
    NIC_LOG_DEBUG("PI-LIF:{}: RDMA AQ Create for lif {}", __FUNCTION__, lif);
    NIC_LOG_DEBUG("{}: Inputs: aq_num: {} aq_log_wqe_size: {} "
                    "aq_log_num_wqes: {} "
                    "cq_num: {} phy_base_addr: {}", __FUNCTION__, cmd->qid_ver,
                    cmd->stride_log2, cmd->depth_log2, cmd->cid,
                    cmd->dma_addr);

    memset(&aqcb, 0, sizeof(aqcb_t));
    aqcb.aqcb0.ring_header.total_rings = MAX_AQ_RINGS;
    aqcb.aqcb0.ring_header.host_rings = MAX_AQ_HOST_RINGS;

    aqcb.aqcb0.log_wqe_size = cmd->stride_log2;
    aqcb.aqcb0.log_num_wqes = cmd->depth_log2;
    aqcb.aqcb0.aq_id = cmd->qid_ver;
    aqcb.aqcb0.phy_base_addr = cmd->dma_addr | (1UL << 63) | ((uint64_t)lif << 52);
    aqcb.aqcb0.cq_id = cmd->cid;
    aqcb.aqcb0.cqcb_addr = GetQstateAddr(ETH_QTYPE_CQ, cmd->cid);

    aqcb.aqcb0.proxy_pindex = 0;

    ret = pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "rdma_aq_tx_stage0", &offset);
    if (ret < 0) {
        NIC_LOG_ERR("Failed to get PC offset : {} for prog: {}, label: {}", ret, "txdma_stage0.bin", "rdma_aq_tx_stage0");
        return DEVCMD_ERROR;
    }

    aqcb.aqcb0.ring_header.pc = offset;

    // write to hardware
    NIC_LOG_DEBUG("{}: LIF: {}: Writting initial AQCB State, "
                    "AQCB->phy_addr: {:#x} "
                    "aqcb_size: {}",
                    __FUNCTION__, lif, aqcb.aqcb0.phy_base_addr, sizeof(aqcb_t));

    // Convert data before writting to HBM
    memrev((uint8_t*)&aqcb.aqcb0, sizeof(aqcb0_t));
    memrev((uint8_t*)&aqcb.aqcb1, sizeof(aqcb1_t));

    uint64_t addr = GetQstateAddr(ETH_QTYPE_ADMIN, cmd->qid_ver);;
    NIC_LOG_DEBUG("{}: QstateAddr = {:#x}", __FUNCTION__, addr);
    if (addr == 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for CQ qid {}",
                    lif, cmd->qid_ver);
        return DEVCMD_ERROR;
    }
    WRITE_MEM(addr, (uint8_t *)&aqcb, sizeof(aqcb), 0);

    NIC_LOG_DEBUG("----------------------- API End ------------------------");
    return (DEVCMD_SUCCESS);
}

int
Eth::GenerateQstateInfoJson(pt::ptree &lifs)
{
    pt::ptree lif;
    pt::ptree qstates;

    NIC_LOG_DEBUG("lif-{}: Qstate Info to Json", hal_lif_info_.hw_lif_id);

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

types::LifType
Eth::ConvertDevTypeToLifType(EthDevType dev_type)
{
    switch(dev_type) {
        case ETH_HOST: return types::LIF_TYPE_HOST;
        case ETH_HOST_MGMT: return types::LIF_TYPE_HOST_MANAGEMENT;
        case ETH_MNIC_OOB_MGMT: return types::LIF_TYPE_MNIC_OOB_MANAGEMENT;
        case ETH_MNIC_INTERNAL_MGMT: return types::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT;
        case ETH_MNIC_INBAND_MGMT: return types::LIF_TYPE_MNIC_INBAND_MANAGEMENT;
        default: return types::LIF_TYPE_NONE;
    }
}

void
Eth::SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client)
{
    hal = hal_client;
    hal_common_client = hal_cmn_client;
}
