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

#include "intrutils.h"
#include "eth_dev.hpp"
#include "hal_client.hpp"


static bool
mac_is_multicast(uint64_t mac) {
    return ((mac & 0x010000000000) == 0x010000000000);
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

const char rdma_qstate_1024[1024] = { 0 };
const char rdma_qstate_32[32]     = { 0 };
const char rdma_qstate_64[32]     = { 0 };

sdk::lib::indexer *Eth_PF::fltr_allocator = sdk::lib::indexer::factory(4096);

struct queue_info Eth_PF::qinfo [NUM_QUEUE_TYPES] = {
    [ETH_QTYPE_RX] = {
        .type_num = ETH_QTYPE_RX,
        .size = 1,
        .entries = 3,   // TODO: Set as log2(spec->rxq_count)
        .purpose = ::intf::LIF_QUEUE_PURPOSE_RX,
        .prog = "rxdma_stage0.bin",
        .label = "eth_rx_stage0",
        .qstate = qstate
    },
    [ETH_QTYPE_TX] = {
        .type_num = ETH_QTYPE_TX,
        .size = 1,
        .entries = 3,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_TX,
        .prog = "txdma_stage0.bin",
        .label = "eth_tx_stage0",
        .qstate = qstate
    },
    [ETH_QTYPE_ADMIN] = {
        .type_num = ETH_QTYPE_ADMIN,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
        .prog = "txdma_stage0.bin",
        .label = "adminq_stage0",
        .qstate = qstate
    },
    [ETH_QTYPE_SQ] = {
        .type_num = ETH_QTYPE_SQ,
        .size = 5,  //TODO: why not 10??
        .entries = 14,   
        .purpose = ::intf::LIF_QUEUE_PURPOSE_RDMA_SEND,
        .prog = "txdma_stage0.bin",
        .label = "rdma_req_tx_stage0",
        .qstate = rdma_qstate_1024
    },
    [ETH_QTYPE_RQ] = {
        .type_num = ETH_QTYPE_RQ,
        .size = 5,
        .entries = 14,   
        .purpose = ::intf::LIF_QUEUE_PURPOSE_RDMA_SEND,
        .prog = "rxdma_stage0.bin",
        .label = "rdma_resp_rx_stage0",
        .qstate = rdma_qstate_1024
    },
    [ETH_QTYPE_CQ] = {
        .type_num = ETH_QTYPE_CQ,
        .size = 1,
        .entries = 15,   
        .purpose = ::intf::LIF_QUEUE_PURPOSE_CQ,
        .prog = "txdma_stage0.bin", //hack
        .label = "rdma_req_tx_stage0", //hack
        .qstate = rdma_qstate_64
    },
    [ETH_QTYPE_EQ] = {
        .type_num = ETH_QTYPE_EQ,
        .size = 0,
        .entries = 10,   
        .purpose = ::intf::LIF_QUEUE_PURPOSE_EQ,
        .prog = "txdma_stage0.bin", //hack
        .label = "rdma_req_tx_stage0", //hack
        .qstate = rdma_qstate_32
    },
};

Eth_PF::Eth_PF(HalClient *hal_client, void *dev_spec)
{
    uint64_t lif_handle;

    hal = hal_client;
    spec = (struct eth_devspec *)dev_spec;

    // Create LIF
    qinfo[ETH_QTYPE_RX].entries = (uint32_t)log2(spec->rxq_count);
    qinfo[ETH_QTYPE_TX].entries = (uint32_t)log2(spec->txq_count);
    qinfo[ETH_QTYPE_SQ].entries = (uint32_t)log2(spec->rdma_sq_count);
    qinfo[ETH_QTYPE_RQ].entries = (uint32_t)log2(spec->rdma_rq_count);
    qinfo[ETH_QTYPE_CQ].entries = (uint32_t)log2(spec->rdma_cq_count);
    qinfo[ETH_QTYPE_EQ].entries = (uint32_t)log2(spec->rdma_eq_count);

    for (auto it = hal->enic2ep_map[spec->enic_id].cbegin();
            it != hal->enic2ep_map[spec->enic_id].cend();
            it++) {
        if (hal->EndpointDelete(spec->vrf_id, *it)) {
            printf("[ERROR] Failed to delete ENDPOINT, handle = %lu\n",
                   *it);
            return;
        }
    }

    for (auto it = hal->mcast_groups.cbegin(); it != hal->mcast_groups.cend(); it++) {
        auto it_enic = find(it->second.cbegin(), it->second.cend(), spec->enic_id);
        if (it_enic != it->second.cend()) {
            if (hal->MulticastGroupLeave(get<2>(it->first),
                                         spec->vrf_id,
                                         get<1>(it->first),
                                         spec->enic_id) != 0) {
                printf("[ERROR] lif%lu: Failed to leave group 0x%012lx from l2segment handle %lu\n",
                       info.hw_lif_id, get<2>(it->first), get<1>(it->first));
                return;
            }
        }
    }

    if (hal->enic_map.find(spec->enic_id) != hal->enic_map.end()) {
        if (hal->InterfaceDelete(spec->enic_id)) {
            printf("[ERROR] Failed to delete ENIC, id = %lu\n", spec->enic_id);
            return;
        }
    }

    if (hal->lif_map.find(spec->lif_id) != hal->lif_map.end()) {
        if (hal->LifDelete(spec->lif_id)) {
            printf("[ERROR] Failed to delete LIF, id = %lu\n", spec->lif_id);
            return;
        }
    }

    lif_handle = hal->LifCreate(spec->lif_id, qinfo, &info,
                                spec->enable_rdma, spec->max_pt_entries, spec->max_keys);
    if (lif_handle == 0) {
        printf("[ERROR] Failed to create LIF\n");
        return;
    }

    printf("[INFO] lif%lu: mac %0lx\n", info.hw_lif_id, spec->mac_addr);

    uint32_t filter_id;
    if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
        printf("[ERROR] lif%lu: Failed to allocate VLAN filter\n", info.hw_lif_id);
        return;
    }
    vlans[filter_id] = spec->native_vlan;

    name = string_format("eth%d", spec->lif_id);

    // Configure PCI resources
    memset(&pci_resources, 0, sizeof(pci_resources));
    pci_resources.lif_valid = 1;
    pci_resources.lif = info.hw_lif_id;
    pci_resources.intrb = spec->intr_base;
    pci_resources.intrc = spec->intr_count;
    pci_resources.port = spec->pcie_port;
    pci_resources.npids = spec->rdma_pid_count;
    // TODO: Need an allocator for this
    pci_resources.devcmdpa = DEVCMD_BASE + (info.hw_lif_id * 4096 * 2);
    pci_resources.devcmddbpa = pci_resources.devcmdpa + 4096;

    static_assert(sizeof(struct dev_cmd_regs) == 4096);
    static_assert((offsetof(struct dev_cmd_regs, cmd)  % 4) == 0);
    static_assert(sizeof(devcmd->cmd) == 64);
    static_assert((offsetof(struct dev_cmd_regs, comp) % 4) == 0);
    static_assert(sizeof(devcmd->comp) == 16);
    static_assert((offsetof(struct dev_cmd_regs, data) % 4) == 0);

    // Init Devcmd Region
    // TODO: mmap instead of calloc after porting to real pal
    devcmd = (struct dev_cmd_regs *)calloc(1, sizeof(struct dev_cmd_regs));
    devcmd->signature = DEV_CMD_SIGNATURE;
    WRITE_MEM(pci_resources.devcmdpa, (uint8_t *)devcmd, sizeof(*devcmd));
    MEM_SET(pci_resources.devcmddbpa, 0, 4096);

    printf("[INFO] lif%lu: Devcmd PA 0x%lx DevcmdDB PA 0x%lx\n", info.hw_lif_id,
           pci_resources.devcmdpa, pci_resources.devcmddbpa);

    if (spec->pcie_port == 0xff) {
        printf("[INFO] lif%lu: Skipped creating PCI device, pcie_port %d\n",
               info.hw_lif_id, spec->pcie_port);
        return;
    }

#ifdef __aarch64__
    // Create PCI device
    pdev = pciehdev_eth_new(name.c_str(), &pci_resources);
    if (pdev == NULL) {
        printf("[ERROR] lif%lu: Failed to create Eth_PF PCI device\n",
               info.hw_lif_id);
        return;
    }
    pciehdev_set_priv(pdev, (void *)this);

    // Add device to PCI topology
    int ret = pciehdev_add(pdev);
    if (ret != 0) {
        printf("[ERROR] lif%lu: Failed to add Eth_PF PCI device to topology\n",
               info.hw_lif_id);
        return;
    }
#endif

    // RSS configuration
    rss_type = LifRssType::RSS_TYPE_NONE;
    const char s[128] = {0};
    rss_key = string(s, 40);
    rss_indir = string(s, 128);
}

uint64_t
Eth_PF::GetQstateAddr(uint8_t qtype, uint32_t qid)
{
    uint32_t cnt, sz;

    assert(qtype < NUM_QUEUE_TYPES);

    cnt = 1 << this->qinfo[qtype].entries;
    sz = 1 << (5 + this->qinfo[qtype].size);

    assert(qid < cnt);

    return info.qstate_addr[qtype] + (qid * sz);
}

void
Eth_PF::DevcmdPoll()
{
    dev_cmd_db_t    db;
    dev_cmd_db_t    db_clear = {0};

    if (spec->host_dev) {
        return;
    }

    db.v = 0;
    READ_MEM(pci_resources.devcmddbpa, (uint8_t *)&db, sizeof(db));
    if (db.v) {
        printf("[INFO] %s lif%lu active\n", __FUNCTION__, info.hw_lif_id);
        DevcmdHandler();
        WRITE_MEM(pci_resources.devcmddbpa, (uint8_t *)&db_clear,
                    sizeof(db_clear));
    }
}

void
Eth_PF::DevcmdHandler()
{
    enum DevcmdStatus status;

    // read devcmd region
    READ_MEM(pci_resources.devcmdpa, (uint8_t *)devcmd,
             sizeof(struct dev_cmd_regs));

    if (devcmd->done != 0) {
        printf("[ERROR] lif%lu: Devcmd done is set before processing command, opcode = %d\n",
               info.hw_lif_id, devcmd->cmd.cmd.opcode);
        status = DEVCMD_ERROR;
        goto devcmd_done;
    }

    if (devcmd->signature != DEV_CMD_SIGNATURE) {
        printf("[ERROR] lif%lu: Devcmd signature mismatch, opcode = %d\n",
               info.hw_lif_id, devcmd->cmd.cmd.opcode);
        status = DEVCMD_ERROR;
        goto devcmd_done;
    }

    status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->comp, &devcmd->data);

    // write data
    if (status == DEVCMD_SUCCESS) {
        WRITE_MEM(pci_resources.devcmdpa + offsetof(struct dev_cmd_regs, data),
                  (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, data),
                  sizeof(devcmd->data));
    }

devcmd_done:
    devcmd->comp.comp.status = status;
    devcmd->done = 1;

    // write completion
    WRITE_MEM(pci_resources.devcmdpa + offsetof(struct dev_cmd_regs, comp),
              (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, comp),
              sizeof(devcmd->comp));

    // write done
    WRITE_MEM(pci_resources.devcmdpa + offsetof(struct dev_cmd_regs, done),
              (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, done),
              sizeof(devcmd->done));
}

enum DevcmdStatus
Eth_PF::CmdHandler(void *req, void *req_data,
    void *resp, void *resp_data)
{
    union dev_cmd *cmd = (union dev_cmd *)req;
    union dev_cmd_comp *comp = (union dev_cmd_comp *)resp;
    enum DevcmdStatus status;

    switch (cmd->cmd.opcode) {

    case CMD_OPCODE_NOP:
        printf("[INFO] lif%lu: CMD_OPCODE_NOP\n", info.hw_lif_id);
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
        printf("[INFO] lif%lu: CMD_OPCODE_HANG_NOTIFY\n", info.hw_lif_id);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_Q_ENABLE:
        status = this->_CmdQEnable(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_Q_DISABLE:
        status = this->_CmdQDisable(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_STATION_MAC_ADDR_GET:
        status = this->_CmdMacAddrGet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_MTU_SET:
        printf("[INFO] lif%lu: CMD_OPCODE_MTU_SET\n", info.hw_lif_id);
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
        printf("[INFO] lif%lu: CMD_OPCODE_STATS_DUMP_START\n", info.hw_lif_id);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_STATS_DUMP_STOP:
        printf("[INFO] lif%lu: CMD_OPCODE_STATS_DUMP_STOP\n", info.hw_lif_id);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_DEBUG_Q_DUMP:
        printf("[INFO] lif%lu: CMD_OPCODE_DEBUG_Q_DUMP\n", info.hw_lif_id);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RSS_HASH_SET:
        status = this->_CmdRssHashSet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RSS_INDIR_SET:
        status = this->_CmdRssIndirSet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_V0_RDMA_CREATE_MR:
        status = this->_CmdRDMACreateMR(req, req_data, resp, resp_data);
        break;
        
    case CMD_OPCODE_V0_RDMA_CREATE_CQ:
        status = this->_CmdRDMACreateCQ(req, req_data, resp, resp_data);
        break;
        
    case CMD_OPCODE_V0_RDMA_CREATE_QP:
        status = this->_CmdRDMACreateQP(req, req_data, resp, resp_data);
        break;
        
    case CMD_OPCODE_V0_RDMA_MODIFY_QP:
        status = this->_CmdRDMAModifyQP(req, req_data, resp, resp_data);
        break;

    default:
        printf("[ERROR] lif%lu: Unknown Opcode %d\n", info.hw_lif_id,
            cmd->cmd.opcode);
        status = DEVCMD_UNKNOWN;
        break;
    }

    comp->comp.status = status;
    comp->comp.rsvd = 0xff;

    return (status);
}

enum DevcmdStatus
Eth_PF::_CmdIdentify(void *req, void *req_data, void *resp, void *resp_data)
{
    union identity *rsp = (union identity *)resp_data;
    struct identify_comp *comp = (struct identify_comp *)resp;

    printf("[INFO] lif%lu: CMD_OPCODE_IDENTIFY\n", info.hw_lif_id);

    // TODO: Get these from hw
    rsp->dev.asic_type = 0x00;
    rsp->dev.asic_rev = 0xA0;
    sprintf((char *)&rsp->dev.serial_num, "haps");
    // TODO: Get this from sw
    sprintf((char *)&rsp->dev.fw_version, "v0.0.1");
    rsp->dev.nlifs = 1;
    rsp->dev.ndbpgs_per_lif = 1;
    rsp->dev.nadminqs_per_lif = spec->adminq_count;
    rsp->dev.ntxqs_per_lif = spec->txq_count;
    rsp->dev.nrxqs_per_lif = spec->rxq_count;
    rsp->dev.ncqs_per_lif = 0;
    rsp->dev.nintrs = spec->intr_count;
    rsp->dev.nucasts_per_lif = 0;
    rsp->dev.nmcasts_per_lif = 0;
    rsp->dev.nrdmasqs_per_lif = spec->rdma_sq_count;
    rsp->dev.nrdmarqs_per_lif = spec->rdma_rq_count;
    rsp->dev.neqs_per_lif = spec->eq_count;

    comp->ver = IDENTITY_VERSION_1;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdReset(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    printf("[INFO] lif%lu: CMD_OPCODE_RESET\n", info.hw_lif_id);

    for (auto it = endpoints.cbegin(); it != endpoints.cend(); it++) {
        if (hal->EndpointDelete(spec->vrf_id, it->second)) {
            printf("[ERROR] lif%lu: Failed to delete endpoint for mac 0x%012lx vlan %u, handle = %lu\n",
            info.hw_lif_id, get<0>(it->first), get<1>(it->first), it->second);
            return (DEVCMD_ERROR);
        }
        endpoints.erase(it->first);
    }

    for (auto it = hal->mcast_groups.cbegin(); it != hal->mcast_groups.cend(); it++) {
        auto it_enic = find(it->second.cbegin(), it->second.cend(), spec->enic_id);
        if (it_enic != it->second.cend()) {
            if (hal->MulticastGroupLeave(get<2>(it->first),
                                         spec->vrf_id,
                                         get<1>(it->first),
                                         spec->enic_id) != 0) {
                printf("[ERROR] lif%lu: Failed to leave group 0x%012lx from l2segment handle %lu\n",
                       info.hw_lif_id, get<2>(it->first), get<1>(it->first));
                return (DEVCMD_ERROR);
            }
        }
    }

    if (hal->enic_map.find(spec->enic_id) != hal->enic_map.end()) {
        if (hal->InterfaceDelete(spec->enic_id)) {
            printf("[ERROR] Failed to delete ENIC, id = %lu\n", spec->enic_id);
            return (DEVCMD_ERROR);
        }
    }

    // Clear all fields after p_index0
    for (uint32_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_RX, qid);
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(eth_rx_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(eth_rx_qstate_t, p_index0));
        invalidate_rxdma_cacheline(addr);
    }

    for (uint32_t qid = 0; qid < spec->txq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_TX, qid);
        WRITE_MEM(addr + offsetof(eth_tx_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(eth_tx_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(eth_tx_qstate_t, p_index0));
        invalidate_txdma_cacheline(addr);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_ADMIN, qid);
        WRITE_MEM(addr + offsetof(eth_admin_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(eth_admin_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(eth_admin_qstate_t, p_index0));
        invalidate_txdma_cacheline(addr);
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdLifInit(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t enic_handle;
    struct lif_init_cmd *cmd = (struct lif_init_cmd *)req;
    vector<uint64_t> l2seg_handles;

    printf("[INFO] lif%lu: CMD_OPCODE_LIF_INIT: lif_index %u\n", info.hw_lif_id,
    cmd->index);

    if (hal->uplink_map.find(spec->uplink) == hal->uplink_map.end()) {
        printf("[ERROR] lif%lu: bad uplink %u\n", info.hw_lif_id, spec->uplink);
        return (DEVCMD_ERROR);
    }

    if (hal->vlan2seg_map.find(spec->native_vlan) == hal->vlan2seg_map.end()) {
        printf("[ERROR] lif%lu: bad vlan %u\n", info.hw_lif_id, spec->native_vlan);
        return (DEVCMD_ERROR);
    }

    for (auto it = hal->vlan2seg_map.cbegin(); it != hal->vlan2seg_map.cend(); it++) {
        if (it->first == spec->native_vlan) {
            continue;
        }
        l2seg_handles.push_back(it->second);
    }

    // Create a classic ENIC
    // TODO: In smart nic mode we will have to create multiple enics
    enic_handle = hal->EnicCreate(spec->enic_id,
                                  spec->lif_id,
                                  hal->uplink_map[spec->uplink],
                                  hal->vlan2seg_map[spec->native_vlan],
                                  l2seg_handles,
                                  spec->mac_addr);
    if (enic_handle == 0) {
        printf("[ERROR] lif%lu: Failed to create ENIC, id = %lu\n",
               info.hw_lif_id, spec->enic_id);
        return (DEVCMD_ERROR);
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdAdminQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct adminq_init_cmd *cmd = (struct adminq_init_cmd *)req;
    struct adminq_init_comp *comp = (struct adminq_init_comp *)resp;
    eth_admin_qstate_t admin_qstate;

    printf("[INFO] lif%lu: CMD_OPCODE_ADMINQ_INIT: "
        "queue_index %u ring_base 0x%lx ring_size %u intr_index %u\n",
        info.hw_lif_id,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index);

    if (cmd->index >= spec->adminq_count) {
        printf("[ERROR] lif%lu: bad qid %d\n", info.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        printf("[ERROR] lif%lu: bad intr %d\n", info.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        printf("[ERROR] lif%lu: bad ring size %d\n", info.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = GetQstateAddr(ETH_QTYPE_ADMIN, cmd->index);

    READ_MEM(addr, (uint8_t *)&admin_qstate, sizeof(admin_qstate));
    //NOTE: admin_qstate.cosA is ignored for Admin Queues. Db should ring on cosB.
    admin_qstate.cosA = 0;
    //NOTE: admin_qstate.cosB is set by HAL LifCreate
    admin_qstate.host = 1;
    admin_qstate.total = 1;
    admin_qstate.pid = cmd->pid;
    admin_qstate.enable = 1;
    admin_qstate.color = 1;
    admin_qstate.host_queue = spec->host_dev;
    admin_qstate.rsvd1 = 0x1f;
    admin_qstate.p_index0 = 0;
    admin_qstate.c_index0 = 0;
    admin_qstate.comp_index = 0;
    admin_qstate.ci_fetch = 0;
    if (spec->host_dev)
        admin_qstate.ring_base = (1ULL << 63) | (info.hw_lif_id << 52) | cmd->ring_base;
    else
        admin_qstate.ring_base = cmd->ring_base;
    admin_qstate.ring_size = cmd->ring_size;
    admin_qstate.cq_ring_base = roundup(admin_qstate.ring_base + (64 << cmd->ring_size), 4096);
#ifdef __aarch64__
    admin_qstate.intr_assert_addr = intr_assert_addr(spec->intr_base + cmd->intr_index);
#endif
    admin_qstate.nicmgr_qstate_addr = 0xc0084000;
    WRITE_MEM(addr, (uint8_t *)&admin_qstate, sizeof(admin_qstate));

    invalidate_txdma_cacheline(addr);

    comp->qid = spec->adminq_base + cmd->index;
    comp->qtype = ETH_QTYPE_ADMIN;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdTxQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct txq_init_cmd *cmd = (struct txq_init_cmd *)req;
    struct txq_init_comp *comp = (struct txq_init_comp *)resp;
    eth_tx_qstate_t tx_qstate;

    printf("[INFO] lif%lu: CMD_OPCODE_TXQ_INIT: "
    "queue_index %u cos %u ring_base 0x%lx ring_size %u intr_index %u %c%c\n",
    info.hw_lif_id,
    cmd->index,
    cmd->cos,
    cmd->ring_base,
    cmd->ring_size,
    cmd->intr_index,
    cmd->I ? 'I' : '-',
    cmd->E ? 'E' : '-');

    if (cmd->index >= spec->txq_count) {
        printf("[ERROR] lif%lu: bad qid %d\n", info.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        printf("[ERROR] lif%lu: bad intr %d\n", info.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        printf("[ERROR] lif%lu: bad ring_size %d\n", info.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = GetQstateAddr(ETH_QTYPE_TX, cmd->index);

    READ_MEM(addr, (uint8_t *)&tx_qstate, sizeof(tx_qstate));
    tx_qstate.cosA = cmd->cos;
    //NOTE: tx_qstate.cosB is ignored for TX queues.
    tx_qstate.host = 1;
    tx_qstate.total = 1;
    tx_qstate.pid = cmd->pid;
    tx_qstate.enable = cmd->E;
    tx_qstate.color = 1;
    tx_qstate.host_queue = spec->host_dev;
    tx_qstate.p_index0 = 0;
    tx_qstate.c_index0 = 0;
    tx_qstate.comp_index = 0;
    tx_qstate.ci_fetch = 0;
    if (spec->host_dev)
        tx_qstate.ring_base = (1ULL << 63) | (info.hw_lif_id << 52) | cmd->ring_base;
    else
        tx_qstate.ring_base = cmd->ring_base;
    tx_qstate.ring_size = cmd->ring_size;
    tx_qstate.cq_ring_base = roundup(tx_qstate.ring_base + (16 << cmd->ring_size), 4096);
#ifdef __aarch64__
    tx_qstate.intr_assert_addr = intr_assert_addr(spec->intr_base + cmd->intr_index);
#endif
    tx_qstate.sg_ring_base = roundup(tx_qstate.cq_ring_base + (16 << cmd->ring_size), 4096);
    tx_qstate.spurious_db_cnt = 0;
    WRITE_MEM(addr, (uint8_t *)&tx_qstate, sizeof(tx_qstate));

    invalidate_txdma_cacheline(addr);

    comp->qid = spec->txq_base + cmd->index;
    comp->qtype = ETH_QTYPE_TX;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdRxQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct rxq_init_cmd *cmd = (struct rxq_init_cmd *)req;
    struct rxq_init_comp *comp = (struct rxq_init_comp *)resp;
    eth_rx_qstate_t rx_qstate;

    printf("[INFO] lif%lu: CMD_OPCODE_RXQ_INIT: "
    "queue_index %u ring_base 0x%lx ring_size %u intr_index %u %c%c\n",
    info.hw_lif_id,
    cmd->index,
    cmd->ring_base,
    cmd->ring_size,
    cmd->intr_index,
    cmd->I ? 'I' : '-',
    cmd->E ? 'E' : '-');

    if (cmd->index >= spec->rxq_count) {
        printf("[ERROR] lif%lu: bad qid %d\n", info.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        printf("[ERROR] lif%lu: bad intr %d\n", info.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        printf("[ERROR] lif%lu: bad ring_size %d\n", info.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = GetQstateAddr(ETH_QTYPE_RX, cmd->index);

    READ_MEM(addr, (uint8_t *)&rx_qstate, sizeof(rx_qstate));
    rx_qstate.cosA = 0;
    rx_qstate.cosB = 0;
    rx_qstate.host = 1;
    rx_qstate.total = 1;
    rx_qstate.pid = cmd->pid;
    rx_qstate.enable = cmd->E;
    rx_qstate.color = 1;
    rx_qstate.host_queue = spec->host_dev;
    rx_qstate.p_index0 = 0;
    rx_qstate.c_index0 = 0;
    rx_qstate.comp_index = 0;
    rx_qstate.c_index1 = 0;
    if (spec->host_dev)
        rx_qstate.ring_base = (1ULL << 63) | (info.hw_lif_id << 52) | cmd->ring_base;
    else
        rx_qstate.ring_base = cmd->ring_base;
    rx_qstate.ring_size = cmd->ring_size;
    rx_qstate.cq_ring_base = roundup(rx_qstate.ring_base + (16 << cmd->ring_size), 4096);
#ifdef __aarch64__
    rx_qstate.intr_assert_addr = intr_assert_addr(spec->intr_base + cmd->intr_index);
#endif
    rx_qstate.rss_type = 0;
    WRITE_MEM(addr, (uint8_t *)&rx_qstate, sizeof(rx_qstate));

    invalidate_rxdma_cacheline(addr);

    comp->qid = spec->rxq_base + cmd->index;
    comp->qtype = ETH_QTYPE_RX;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdFeatures(void *req, void *req_data, void *resp, void *resp_data)
{
    struct features_cmd *cmd = (struct features_cmd *)req;
    struct features_comp *comp = (struct features_comp *)resp;

    printf("[INFO] lif%lu: CMD_OPCODE_FEATURES: wanted "
        "vlan_strip %d vlan_insert %d rx_csum %d tx_csum %d rx_hash %d sg %d\n",
        info.hw_lif_id,
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
            ETH_HW_TX_SG
        );
    } else {
        comp->supported = (
            ETH_HW_VLAN_RX_STRIP |
            ETH_HW_VLAN_TX_TAG |
            ETH_HW_RX_CSUM |
            ETH_HW_TX_CSUM |
            ETH_HW_RX_HASH |
            ETH_HW_TX_SG |
            ETH_HW_TSO |
            ETH_HW_TSO_IPV6
        );
    }

    hal->LifSetVlanStrip(spec->lif_id, cmd->wanted & comp->supported & ETH_HW_VLAN_RX_STRIP);
    hal->LifSetVlanInsert(spec->lif_id, cmd->wanted & comp->supported & ETH_HW_VLAN_TX_TAG);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdQEnable(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct q_enable_cmd *cmd = (struct q_enable_cmd *)req;
    // q_enable_comp *comp = (q_enable_comp *)resp;
    uint8_t value;

    if (cmd->qtype >= 8) {
        printf("[ERROR] lif%lu: CMD_OPCODE_Q_ENABLE: bad qtype %d\n",
        info.hw_lif_id, cmd->qtype);
        return (DEVCMD_ERROR);
    }

    printf("[INFO] lif%lu: CMD_OPCODE_Q_ENABLE: type %d qid %d\n",
    info.hw_lif_id, cmd->qtype, cmd->qid);

    value = (spec->host_dev << 5) /* host_queue */
            | (1 << 6) /* color */
            | (1 << 7) /* enable */;

    switch (cmd->qtype) {
    case ETH_QTYPE_RX:
        if (cmd->qid >= spec->rxq_count) {
            printf("[ERROR] lif%lu: CMD_OPCODE_Q_ENABLE: bad qid %d\n",
            info.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_RX, cmd->qid);
        WRITE_MEM(addr + 16, (uint8_t *)&value, sizeof(value));
        invalidate_rxdma_cacheline(addr);
        break;
    case ETH_QTYPE_TX:
        if (cmd->qid >= spec->txq_count) {
            printf("[ERROR] lif%lu: CMD_OPCODE_Q_ENABLE: bad qid %d\n",
                   info.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_TX, cmd->qid);
        WRITE_MEM(addr + 16, (uint8_t *)&value, sizeof(value));
        invalidate_txdma_cacheline(addr);
        break;
    case ETH_QTYPE_ADMIN:
        if (cmd->qid >= spec->adminq_count) {
            printf("[ERROR] lif%lu: CMD_OPCODE_Q_ENABLE: bad qid %d\n",
                   info.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_ADMIN, cmd->qid);
        WRITE_MEM(addr + 16, (uint8_t *)&value, sizeof(value));
        invalidate_txdma_cacheline(addr);
        break;
    default:
        return (DEVCMD_ERROR);
        break;
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdQDisable(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct q_disable_cmd *cmd = (struct q_disable_cmd *)req;
    // q_disable_comp *comp = (q_disable_comp *)resp;
    uint8_t value;

    if (cmd->qtype >= 8) {
        printf("[ERROR] lif%lu: CMD_OPCODE_Q_DISABLE: bad qtype %d\n",
        info.hw_lif_id, cmd->qtype);
        return (DEVCMD_ERROR);
    }

    printf("[INFO] lif%lu: CMD_OPCODE_Q_DISABLE: type %d qid %d\n",
    info.hw_lif_id, cmd->qtype, cmd->qid);

    value = (spec->host_dev << 5) /* host_queue */
            | (1 << 6) /* color */
            | (0 << 7) /* enable */;

    switch (cmd->qtype) {
    case ETH_QTYPE_RX:
        if (cmd->qid >= spec->rxq_count) {
            printf("[ERROR] lif%lu: CMD_OPCODE_Q_ENABLE: bad qid %d\n",
            info.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_RX, cmd->qid);
        WRITE_MEM(addr + 16, (uint8_t *)&value, sizeof(value));
        invalidate_rxdma_cacheline(addr);
        break;
    case ETH_QTYPE_TX:
        if (cmd->qid >= spec->txq_count) {
            printf("[ERROR] lif%lu: CMD_OPCODE_Q_ENABLE: bad qid %d\n",
                   info.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_TX, cmd->qid);
        WRITE_MEM(addr + 16, (uint8_t *)&value, sizeof(value));
        invalidate_txdma_cacheline(addr);
        break;
    case ETH_QTYPE_ADMIN:
        if (cmd->qid >= spec->adminq_count) {
            printf("[ERROR] lif%lu: CMD_OPCODE_Q_ENABLE: bad qid %d\n",
                   info.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = GetQstateAddr(ETH_QTYPE_ADMIN, cmd->qid);
        WRITE_MEM(addr + 16, (uint8_t *)&value, sizeof(value));
        invalidate_txdma_cacheline(addr);
        break;
    default:
        return (DEVCMD_ERROR);
        break;
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdSetMode(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rx_mode_set_cmd *cmd = (struct rx_mode_set_cmd *)req;
    // rx_mode_set_comp *comp = (rx_mode_set_comp *)resp;

    printf("[INFO] lif%lu: CMD_OPCODE_RX_MODE_SET: rx_mode 0x%x %c%c%c%c%c\n",
            info.hw_lif_id,
            cmd->rx_mode,
            cmd->rx_mode & RX_MODE_F_UNICAST   ? 'u' : '-',
            cmd->rx_mode & RX_MODE_F_MULTICAST ? 'm' : '-',
            cmd->rx_mode & RX_MODE_F_BROADCAST ? 'b' : '-',
            cmd->rx_mode & RX_MODE_F_PROMISC   ? 'p' : '-',
            cmd->rx_mode & RX_MODE_F_ALLMULTI  ? 'a' : '-');

    hal->LifSetBroadcast(spec->lif_id, cmd->rx_mode & RX_MODE_F_BROADCAST);
    hal->LifSetAllMulticast(spec->lif_id, cmd->rx_mode & RX_MODE_F_ALLMULTI);
    // hal->LifSetPromiscuous(spec->lif_id, cmd->rx_mode & RX_MODE_F_PROMISC);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdRxFilterAdd(void *req, void *req_data, void *resp, void *resp_data)
{
    //int status;
    uint64_t endpoint_handle;
    uint64_t mac_addr;
    uint16_t vlan;
    uint32_t filter_id = 0;
    struct rx_filter_add_cmd *cmd = (struct rx_filter_add_cmd *)req;
    struct rx_filter_add_comp *comp = (struct rx_filter_add_comp *)resp;

    if (cmd->match == RX_FILTER_MATCH_MAC) {

        memcpy((uint8_t *)&mac_addr, (uint8_t *)&cmd->mac.addr, sizeof(cmd->mac.addr));
        mac_addr = be64toh(mac_addr) >> (8 * sizeof(mac_addr) - 8 * sizeof(cmd->mac.addr));

        printf("[INFO] lif%lu: CMD_OPCODE_RX_FILTER_ADD: type RX_FILTER_MATCH_MAC mac 0x%012lx\n",
                info.hw_lif_id, mac_addr);

        if (mac_is_multicast(mac_addr)) {

            // Join multicast group in all segments
            for (auto it = vlans.cbegin(); it != vlans.cend(); it++) {
                if (hal->MulticastGroupJoin(mac_addr,
                                            spec->vrf_id,
                                            hal->l2seg_handle2id[hal->vlan2seg_map[it->second]],
                                            spec->enic_id) != 0) {
                    printf("[ERROR] lif%lu: Failed to join group 0x%012lx in vlan %u\n",
                           info.hw_lif_id, mac_addr, it->second);
                    return (DEVCMD_ERROR);
                }
            }

        } else {

            // Create endpoints on all vlans
            for (auto it = vlans.cbegin(); it != vlans.cend(); it++) {

                tuple<uint64_t, uint16_t> key(mac_addr, it->second);

                // MAC address is already registered. nop!
                if (endpoints.find(key) != endpoints.end()) {
                    continue;
                }

                // Create endpoint
                endpoint_handle = hal->EndpointCreate(spec->vrf_id,
                                                      hal->vlan2seg_map[it->second],
                                                      spec->enic_id,
                                                      spec->sg_id,
                                                      mac_addr,
                                                      spec->ip_addr);
                if (endpoint_handle == 0) {
                    printf("[ERROR] lif%lu: Failed to create endpoint, mac %012lx vlan %u segment %lu\n",
                           info.hw_lif_id, mac_addr, it->second, hal->vlan2seg_map[it->second]);
                    return (DEVCMD_ERROR);
                }
                endpoints[key] = endpoint_handle;
            }

        }

        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            printf("[ERROR] Failed to allocate MAC address filter\n");
            return (DEVCMD_ERROR);
        }
        mac_addrs[filter_id] = mac_addr;

    } else {
        printf("[INFO] lif%lu: CMD_OPCODE_RX_FILTER_ADD\n", info.hw_lif_id);
    }

    if (cmd->match == RX_FILTER_MATCH_VLAN) {

        vlan = cmd->vlan.vlan;

        printf("[INFO] lif%lu: CMD_OPCODE_RX_FILTER_ADD: type RX_FILTER_MATCH_VLAN vlan %u\n",
                info.hw_lif_id, vlan);

        for (auto it = mac_addrs.cbegin(); it != mac_addrs.cend(); it++) {

            if (mac_is_multicast(it->second)) {

                // Join multicast groupfor all mac addresses
                if (hal->MulticastGroupJoin(it->second,
                                            spec->vrf_id,
                                            hal->l2seg_handle2id[hal->vlan2seg_map[vlan]],
                                            spec->enic_id) != 0) {
                    printf("[ERROR] lif%lu: Failed to join group 0x%012lx in vlan %u\n",
                           info.hw_lif_id, it->second, vlan);
                    return (DEVCMD_ERROR);
                }

            } else {

                // Create endpoints for all mac addresses
                tuple<uint64_t, uint16_t> key(it->second, vlan);

                // MAC address is already registered. nop!
                if (endpoints.find(key) != endpoints.end()) {
                    continue;
                }

                // Create endpoint
                endpoint_handle = hal->EndpointCreate(spec->vrf_id,
                                                      hal->vlan2seg_map[vlan],
                                                      spec->enic_id,
                                                      spec->sg_id,
                                                      it->second,
                                                      spec->ip_addr);
                if (endpoint_handle == 0) {
                    printf("[ERROR] lif%lu: Failed to create endpoint, mac %012lx vlan %u segment %lu\n",
                           info.hw_lif_id, it->second, vlan, hal->vlan2seg_map[vlan]);
                    return (DEVCMD_ERROR);
                }
                endpoints[key] = endpoint_handle;
            }
        }

        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            printf("[ERROR] Failed to allocate VLAN filter\n");
            return (DEVCMD_ERROR);
        }

        vlans[filter_id] = vlan;
    }

    comp->filter_id = filter_id;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdRxFilterDel(void *req, void *req_data, void *resp, void *resp_data)
{
    //int status;
    uint64_t mac_addr;
    uint16_t vlan;
    int match;
    struct rx_filter_del_cmd *cmd = (struct rx_filter_del_cmd *)req;
    //struct rx_filter_del_comp *comp = (struct rx_filter_del_comp *)resp;

    if (mac_addrs.find(cmd->filter_id) == mac_addrs.end()) {
        if (vlans.find(cmd->filter_id) == vlans.end()) {
            printf("[ERROR] Invalid filter id %u\n", cmd->filter_id);
            return (DEVCMD_ERROR);
        } else {
            match = RX_FILTER_MATCH_VLAN;
            vlan = vlans[cmd->filter_id];
        }
    } else {
        match = RX_FILTER_MATCH_MAC;
        mac_addr = mac_addrs[cmd->filter_id];
    }

    if (match == RX_FILTER_MATCH_MAC) {

        printf("[INFO] lif%lu: CMD_OPCODE_RX_FILTER_DEL: type RX_FILTER_MATCH_MAC filter_id %u\n",
            info.hw_lif_id, cmd->filter_id);

        if (mac_is_multicast(mac_addr)) {

            // Leave multicast group in all segments
            for (auto it = vlans.cbegin(); it != vlans.cend(); it++) {
                if (hal->MulticastGroupLeave(mac_addr,
                                             spec->vrf_id,                                             
                                             hal->l2seg_handle2id[hal->vlan2seg_map[it->second]],
                                             spec->enic_id) != 0) {
                    printf("[ERROR] lif%lu: Failed to leave group 0x%012lx in vlan %u\n",
                           info.hw_lif_id, mac_addr, it->second);
                    return (DEVCMD_ERROR);
                }
            }

        } else {

            // Delete endpoints from all vlans
            for (auto it = vlans.cbegin(); it != vlans.cend(); it++) {

                tuple<uint64_t, uint16_t> key(mac_addr, it->second);

                // MAC address is not registered. nop!
                if (endpoints.find(key) == endpoints.end()) {
                    continue;
                }

                if (hal->EndpointDelete(spec->vrf_id, endpoints[key])) {
                    printf("[ERROR] lif%lu: Failed to delete endpoint for mac 0x%012lx vlan %u segment %lu\n",
                           info.hw_lif_id, mac_addr, it->second, hal->vlan2seg_map[it->second]);
                    return (DEVCMD_ERROR);
                }
                endpoints.erase(key);
            }
        }

        mac_addrs.erase(cmd->filter_id);

    } else {
        printf("[INFO] lif%lu: CMD_OPCODE_RX_FILTER_DEL\n", info.hw_lif_id);
    }

    if (match == RX_FILTER_MATCH_VLAN) {

        printf("[INFO] lif%lu: CMD_OPCODE_RX_FILTER_DEL: type RX_FILTER_MATCH_VLAN filter %u\n",
                info.hw_lif_id, cmd->filter_id);

        for (auto it = mac_addrs.cbegin(); it != mac_addrs.cend(); it++) {

            if (mac_is_multicast(it->second)) {

                // Leave multicast group for all mac addresses
                if (hal->MulticastGroupLeave(it->second,
                                             spec->vrf_id,
                                             hal->l2seg_handle2id[hal->vlan2seg_map[vlan]],
                                             spec->enic_id) != 0) {
                    printf("[ERROR] lif%lu: Failed to leave group 0x%012lx in vlan %u\n",
                           info.hw_lif_id, it->second, vlan);
                    return (DEVCMD_ERROR);
                }

            } else {

                // Delete endpoints for all mac addresses
                tuple<uint64_t, uint16_t> key(it->second, vlan);

                // MAC address is not registered. nop!
                if (endpoints.find(key) == endpoints.end()) {
                    continue;
                }

                // Delete endpoint
                if (hal->EndpointDelete(spec->vrf_id, endpoints[key])) {
                    printf("[ERROR] lif%lu: Failed to delete endpoint, mac %012lx vlan %u segment %lu\n",
                           info.hw_lif_id, it->second, vlan, hal->vlan2seg_map[vlan]);
                    return (DEVCMD_ERROR);
                }
                endpoints.erase(key);
            }
        }

        vlans.erase(cmd->filter_id);
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdMacAddrGet(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t mac_addr;

    //struct station_mac_addr_get_cmd *cmd = (struct station_mac_addr_get_cmd *)req;
    struct station_mac_addr_get_comp *comp = (struct station_mac_addr_get_comp *)resp;

    printf("[INFO] lif%lu: CMD_OPCODE_STATION_MAC_ADDR_GET\n", info.hw_lif_id);

    mac_addr = be64toh(spec->mac_addr) >> (8 * sizeof(spec->mac_addr) - 8 * sizeof(uint8_t[6]));
    memcpy((uint8_t *)comp->addr, (uint8_t *)&mac_addr, sizeof(comp->addr));

    printf("[INFO] lif%lu: station mac address 0x%012lx\n", info.hw_lif_id, mac_addr);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdRssHashSet(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct rss_hash_set_cmd *cmd = (struct rss_hash_set_cmd *)req;
    //rss_hash_set_comp *comp = (struct rss_hash_set_comp *)resp;

    rss_type = cmd->types;
    rss_key = string((const char *)cmd->key, 40);

    printf("[INFO] lif%lu: CMD_OPCODE_RSS_HASH_SET: type %x key %s table %s\n",
        info.hw_lif_id, rss_type, rss_key.c_str(), rss_indir.c_str());

    hal->LifSetRssConfig(spec->lif_id, (LifRssType)rss_type, rss_key, rss_indir);

    for (uint16_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = GetQstateAddr(ETH_QTYPE_RX, qid);
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, rss_type), (uint8_t *)&rss_type, sizeof(rss_type));
        invalidate_rxdma_cacheline(addr);
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdRssIndirSet(void *req, void *req_data, void *resp, void *resp_data)
{
    //struct rss_indir_set_cmd *cmd = (struct rss_indir_set_cmd *)req;
    //rss_indir_set_comp *comp = (struct rss_indir_set_comp *)resp;

    rss_indir = string((const char *)devcmd->data, 128);

    printf("[INFO] lif%lu: CMD_OPCODE_RSS_INDIR_SET: type %x key %s table %s\n",
        info.hw_lif_id, rss_type, rss_key.c_str(), rss_indir.c_str());

    hal->LifSetRssConfig(spec->lif_id, (LifRssType)rss_type, rss_key, rss_indir);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdRDMACreateMR(void *req, void *req_data, void *resp, void *resp_data)
{
    struct create_mr_cmd  *cmd = (struct create_mr_cmd  *) req;

    printf("%s:%s", __FILE__, __FUNCTION__);

    hal->CreateMR(info.hw_lif_id, cmd->pd_num, cmd->start, cmd->length, cmd->access_flags, cmd->lkey, cmd->rkey, cmd->page_size, (uint64_t *)devcmd->data, cmd->nchunks);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdRDMACreateCQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct create_cq_cmd  *cmd = (struct create_cq_cmd  *) req;

    printf("%s:%s", __FILE__, __FUNCTION__);

    hal->CreateCQ(info.hw_lif_id, cmd->cq_num, cmd->cq_wqe_size, cmd->num_cq_wqes, cmd->cq_va, cmd->va_len, cmd->cq_lkey, cmd->host_pg_size, (uint64_t *)devcmd->data, cmd->pt_size, cmd->eq_id);

    return (DEVCMD_SUCCESS);
    
}
enum DevcmdStatus
Eth_PF::_CmdRDMACreateQP(void *req, void *req_data, void *resp, void *resp_data)
{
    struct create_qp_cmd  *cmd = (struct create_qp_cmd  *) req;
    uint64_t *pt_table = (uint64_t *)&devcmd->data;
    
    printf("%s:%s", __FILE__, __FUNCTION__);

    hal->CreateQP(info.hw_lif_id, cmd->qp_num, cmd->sq_wqe_size,
                  cmd->rq_wqe_size, cmd->num_sq_wqes,
                  cmd->num_rq_wqes, cmd->num_rsq_wqes,
                  cmd->num_rrq_wqes, cmd->pd,
                  cmd->sq_cq_num, cmd->rq_cq_num, cmd->host_pg_size,
                  cmd->pmtu,
                  cmd->service,
                  cmd->sq_pt_size,
                  cmd->pt_size, pt_table);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth_PF::_CmdRDMAModifyQP(void *req, void *req_data, void *resp, void *resp_data)
{
    struct modify_qp_cmd  *cmd = (struct modify_qp_cmd  *) req;

    printf("%s:%s", __FILE__, __FUNCTION__);

    hal->ModifyQP(info.hw_lif_id, cmd->qp_num, cmd->attr_mask,
                  cmd->dest_qp_num, cmd->q_key, cmd->e_psn,
                  cmd->sq_psn, cmd->header_template_size,
                  (unsigned char *)devcmd->data);

    return (DEVCMD_SUCCESS);
}

ostream &operator<<(ostream& os, const Eth_PF& obj) {

    os << "LIF INFO:" << endl;
    os << "\tlif_id = " << obj.spec->lif_id << endl;
    os << "\thw_lif_id = " << obj.info.hw_lif_id << endl;
    os << "\tqstate_addr: " << endl;
    for (int i = 0; i < NUM_QUEUE_TYPES; i++) {
        os << "\t\ttype = " << i
           << ", qstate = 0x" << hex << obj.info.qstate_addr[i] << resetiosflags(ios::hex)
           << endl;
    }

    os << "ENICS:" << endl;
    os << "\tenic_id = " << obj.spec->enic_id << endl;

    os << "ENDPOINTS:" << endl;
    for (auto it = obj.endpoints.cbegin(); it != obj.endpoints.cend(); it++) {
        os << "\tmac = 0x" << setfill('0') << setw(12) << hex << get<0>(it->first) << resetiosflags(ios::hex)
           << " vlan = " << get<1>(it->first)
           << ", endpoint_handle = " << it->second
           << endl;
    }

    return os;
}
