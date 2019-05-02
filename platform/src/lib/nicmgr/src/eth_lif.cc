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
#include <sys/param.h>

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"

#include "nic/include/edmaq.h"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/p4/common/defines.h"

#ifndef APOLLO
#include "gen/proto/nicmgr/metrics.delphi.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"
#endif

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/utils/mpartition.hpp"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"

#include "logger.hpp"
#include "eth_if.h"
#include "eth_dev.hpp"
#include "rdma_dev.hpp"
#include "pd_client.hpp"
#include "eth_lif.hpp"
#include "adminq.hpp"
#include "edmaq.hpp"

// ----------------------------------------------------------------------------
// Mac address to string
// ----------------------------------------------------------------------------
char *
macaddr2str (mac_t mac_addr)
{
    static char       macaddr_str[4][20];
    static uint8_t    macaddr_str_next = 0;
    char              *buf;
    uint8_t           *mac_byte = (uint8_t *)&mac_addr;

    buf = macaddr_str[macaddr_str_next++ & 0x3];
    snprintf(buf, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_byte[5], mac_byte[4], mac_byte[3],
             mac_byte[2], mac_byte[1], mac_byte[0]);
    return buf;
}

sdk::lib::indexer *EthLif::fltr_allocator = sdk::lib::indexer::factory(4096);

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

#define CASE(opcode) case opcode: return #opcode

const char*
EthLif::lif_state_to_str(enum eth_lif_state state)
{
    switch(state) {
        CASE(LIF_STATE_RESETING);
        CASE(LIF_STATE_RESET);
        CASE(LIF_STATE_CREATING);
        CASE(LIF_STATE_CREATED);
        CASE(LIF_STATE_INITING);
        CASE(LIF_STATE_INIT);
        CASE(LIF_STATE_UP);
        CASE(LIF_STATE_DOWN);
        default: return "LIF_STATE_INVALID";
    }
}

const char*
EthLif::opcode_to_str(cmd_opcode_t opcode)
{
    switch(opcode) {
        CASE(CMD_OPCODE_NOP);
        CASE(CMD_OPCODE_LIF_IDENTIFY);
        CASE(CMD_OPCODE_LIF_GETATTR);
        CASE(CMD_OPCODE_LIF_SETATTR);
        CASE(CMD_OPCODE_RX_MODE_SET);
        CASE(CMD_OPCODE_RX_FILTER_ADD);
        CASE(CMD_OPCODE_RX_FILTER_DEL);
        CASE(CMD_OPCODE_Q_INIT);
        CASE(CMD_OPCODE_Q_CONTROL);
        CASE(CMD_OPCODE_RDMA_RESET_LIF);
        CASE(CMD_OPCODE_RDMA_CREATE_EQ);
        CASE(CMD_OPCODE_RDMA_CREATE_CQ);
        CASE(CMD_OPCODE_RDMA_CREATE_ADMINQ);
        default: return "ADMINCMD_UNKNOWN";
    }
}

EthLif::EthLif(devapi *dev_api,
                // HalCommonClient *hal_common_client,
                void *dev_spec,
                PdClient *pd_client,
                eth_lif_res_t *res)
{
    EthLif::dev_api = dev_api;
    EthLif::spec = (struct eth_devspec *)dev_spec;
    EthLif::res = res;
    EthLif::pd = pd_client;
    EthLif::adminq = NULL;
    strncpy0(EthLif::name, spec->name.c_str(), sizeof(EthLif::name));

    // Create LIF
    state = LIF_STATE_CREATING;

    memset(&hal_lif_info_, 0, sizeof(lif_info_t));
    strncpy0(hal_lif_info_.name, name, sizeof(hal_lif_info_.name));
    hal_lif_info_.lif_id = res->lif_id;
    hal_lif_info_.type = (sdk::platform::lif_type_t)Eth::ConvertDevTypeToLifType(spec->eth_type);
    hal_lif_info_.pinned_uplink_port_num = spec->uplink_port_num;
    hal_lif_info_.enable_rdma = spec->enable_rdma;

    memset(qinfo, 0, sizeof(qinfo));

    qinfo[ETH_HW_QTYPE_RX] = {
        .type_num = ETH_HW_QTYPE_RX,
        .size = 1,
        .entries = (uint32_t)log2(spec->rxq_count),
    };

    qinfo[ETH_HW_QTYPE_TX] = {
        .type_num = ETH_HW_QTYPE_TX,
        .size = 1,
        .entries = (uint32_t)log2(spec->txq_count),
    };

    qinfo[ETH_HW_QTYPE_ADMIN] = {
        .type_num = ETH_HW_QTYPE_ADMIN,
        .size = 2,
        .entries = (uint32_t)log2(spec->adminq_count + spec->rdma_aq_count),
    };

    qinfo[ETH_HW_QTYPE_SQ] = {
        .type_num = ETH_HW_QTYPE_SQ,
        .size = 4,
        .entries = (uint32_t)log2(spec->rdma_sq_count),
    };

    qinfo[ETH_HW_QTYPE_RQ] = {
        .type_num = ETH_HW_QTYPE_RQ,
        .size = 4,
        .entries = (uint32_t)log2(spec->rdma_rq_count),
    };

    qinfo[ETH_HW_QTYPE_CQ] = {
        .type_num = ETH_HW_QTYPE_CQ,
        .size = 1,
        .entries = (uint32_t)log2(spec->rdma_cq_count),
    };

    qinfo[ETH_HW_QTYPE_EQ] = {
        .type_num = ETH_HW_QTYPE_EQ,
        .size = 1,
        .entries = (uint32_t)log2(spec->eq_count + spec->rdma_eq_count),
    };

    qinfo[ETH_HW_QTYPE_SVC] = {
        .type_num = ETH_HW_QTYPE_SVC,
        .size = 1,
        .entries = 2,
    };

    memcpy(hal_lif_info_.queue_info, qinfo, sizeof(hal_lif_info_.queue_info));

    pd->program_qstate((struct queue_info*)hal_lif_info_.queue_info,
        &hal_lif_info_, 0x0);

    NIC_LOG_INFO("{}: created lif_id {} mac {} uplink {}",
                 spec->name,
                 hal_lif_info_.lif_id,
                 macaddr2str(spec->mac_addr),
                 spec->uplink_port_num);

    // Stats
    lif_stats_addr = pd->mem_start_addr(MEM_REGION_LIF_STATS_NAME);
    if (lif_stats_addr == INVALID_MEM_ADDRESS) {
        NIC_LOG_ERR("{}: Failed to allocate stats region",
            hal_lif_info_.name);
        throw;
    }
    lif_stats_addr += (hal_lif_info_.lif_id << LG2_LIF_STATS_SIZE);
    host_lif_stats_addr = 0;

    NIC_LOG_INFO("{}: lif_stats_addr: {:#x}",
        hal_lif_info_.name, lif_stats_addr);

#ifndef APOLLO
    auto lif_stats =
        delphi::objects::LifMetrics::NewLifMetrics(hal_lif_info_.lif_id, lif_stats_addr);
    if (lif_stats == NULL) {
        NIC_LOG_ERR("{}: Failed lif metrics registration with delphi",
            hal_lif_info_.name);
        throw;
    }
#endif

    // Lif Status
    lif_status_addr = pd->nicmgr_mem_alloc(sizeof(struct lif_status));
    host_lif_status_addr = 0;
    // TODO: mmap instead of calloc
    lif_status = (struct lif_status *)calloc(1, sizeof(struct lif_status));
    // lif_status = (struct lif_status *)pal_mem_map(lif_status_addr, sizeof(struct lif_status), 0);
    if (lif_status == NULL) {
        NIC_LOG_ERR("{}: Failed to map notify block!", hal_lif_info_.name);
        throw;
    }
    MEM_SET(lif_status_addr, 0, sizeof(struct lif_status), 0);
    // memset(lif_status, 0, sizeof(struct lif_status));

    NIC_LOG_INFO("{}: lif_status_addr {:#x}",
        hal_lif_info_.name, lif_status_addr);

    // NotifyQ
    notify_enabled = 0;
    notify_ring_head = 0;
    notify_ring_base = pd->nicmgr_mem_alloc(4096 + (sizeof(union notifyq_comp) * ETH_NOTIFYQ_RING_SIZE));
    if (notify_ring_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate notify ring!",
            hal_lif_info_.name);
        throw;
    }
    MEM_SET(notify_ring_base, 0, 4096 + (sizeof(union notifyq_comp) * ETH_NOTIFYQ_RING_SIZE), 0);

    NIC_LOG_INFO("{}: notify_ring_base {:#x}", hal_lif_info_.name, notify_ring_base);

    // EdmaQ
    edma_buf_base = pd->nicmgr_mem_alloc(4096);
    if (edma_buf_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma buffer!",
            hal_lif_info_.name);
        throw;
    }

    NIC_LOG_INFO("{}: edma_buf_base {:#x}", hal_lif_info_.name, edma_buf_base);

    edmaq = new EdmaQ(
        hal_lif_info_.name,
        pd,
        hal_lif_info_.lif_id,
        ETH_EDMAQ_QTYPE, ETH_EDMAQ_QID, ETH_EDMAQ_RING_SIZE
    );

    adminq = new AdminQ(hal_lif_info_.name,
        pd,
        hal_lif_info_.lif_id,
        ETH_ADMINQ_REQ_QTYPE, ETH_ADMINQ_REQ_QID, ETH_ADMINQ_REQ_RING_SIZE,
        ETH_ADMINQ_RESP_QTYPE, ETH_ADMINQ_RESP_QID, ETH_ADMINQ_RESP_RING_SIZE,
        AdminCmdHandler, this
    );

    state = LIF_STATE_CREATED;
    active_q_ref_cnt = 0;

}

status_code_t
EthLif::Init(void *req, void *req_data, void *resp, void *resp_data)
{
    sdk_ret_t rs = SDK_RET_OK;
    uint64_t addr;
    struct lif_init_cmd *cmd = (struct lif_init_cmd *)req;

    NIC_LOG_DEBUG("{}: LIF_INIT", hal_lif_info_.name);

    if (state == LIF_STATE_INIT) {
        NIC_LOG_WARN("{}: {} + INIT => {}", hal_lif_info_.name,
            lif_state_to_str(state),
            lif_state_to_str(state));
        return (IONIC_RC_SUCCESS);
    }

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    if (state == LIF_STATE_CREATED) {

        state = LIF_STATE_INITING;

        DEVAPI_CHECK
        rs = dev_api->lif_create(&hal_lif_info_);
        if (rs != SDK_RET_OK) {
            NIC_LOG_ERR("{}: Failed to create LIF", hal_lif_info_.name);
            return (IONIC_RC_ERROR);
        }

        NIC_LOG_INFO("{}: created", hal_lif_info_.name);

        cosA = 1;
        cosB = 0;
        DEVAPI_CHECK
        dev_api->qos_get_txtc_cos(spec->qos_group, spec->uplink_port_num, &cosB);
        if (cosB < 0) {
            NIC_LOG_ERR("{}: Failed to get cosB for group {}, uplink {}",
                        hal_lif_info_.name, spec->qos_group,
                        spec->uplink_port_num);
            return (IONIC_RC_ERROR);
        }

        ctl_cosA = 1;
        ctl_cosB = 0;
        DEVAPI_CHECK
        dev_api->qos_get_txtc_cos("CONTROL", spec->uplink_port_num, &ctl_cosB);
        if (ctl_cosB < 0) {
            NIC_LOG_ERR("{}: Failed to get cosB for group {}, uplink {}",
                        hal_lif_info_.name, "CONTROL",
                        spec->uplink_port_num);
            return (IONIC_RC_ERROR);
        }

        if (spec->enable_rdma) {
            pd->rdma_lif_init(hal_lif_info_.lif_id, spec->key_count,
                            spec->ah_count, spec->pte_count,
                            res->cmb_mem_addr, res->cmb_mem_size);
            // TODO: Handle error
        }
    }

#ifndef APOLLO
    int ret;
    // Reset RSS configuration
    rss_type = LIF_RSS_TYPE_NONE;
    memset(rss_key, 0x00, RSS_HASH_KEY_SIZE);
    memset(rss_indir, 0x00, RSS_IND_TBL_SIZE);
    ret = pd->eth_program_rss(hal_lif_info_.lif_id, rss_type, rss_key, rss_indir,
                              spec->rxq_count);
    if (ret != 0) {
        NIC_LOG_DEBUG("{}: Unable to program hw for RSS HASH", ret);
        return (IONIC_RC_ERROR);
    }
#endif

    // Clear PC to drop all traffic
    for (uint32_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_RX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, fldsiz(eth_rx_qstate_t, pc_offset), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(eth_rx_qstate_t), P4PLUS_CACHE_INVALIDATE_BOTH);
    }

    for (uint32_t qid = 0; qid < spec->txq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_TX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, fldsiz(eth_tx_qstate_t, pc_offset), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(eth_tx_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_ADMIN, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, fldsiz(admin_qstate_t, pc_offset), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(admin_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
    }

    // Initialize NotifyQ
    notify_enabled = 0;
    notify_ring_head = 0;
    MEM_SET(notify_ring_base, 0, 4096 + (sizeof(union notifyq_comp) * ETH_NOTIFYQ_RING_SIZE), 0);

    // Initialize EDMA service
    if (!edmaq->Init(0, ctl_cosA, ctl_cosB)) {
        NIC_LOG_ERR("{}: Failed to initialize EdmaQ service", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    // Initialize ADMINQ service
    if (!adminq->Init(0, ctl_cosA, ctl_cosB)) {
        NIC_LOG_ERR("{}: Failed to initialize AdminQ service", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (cmd->info_pa) {
        host_lif_status_addr = cmd->info_pa + offsetof(struct lif_info, status);
        NIC_LOG_INFO("{}: host_lif_status_addr {:#x}",
                    hal_lif_info_.name, host_lif_status_addr);

        host_lif_stats_addr = cmd->info_pa + offsetof(struct lif_info, stats);
        NIC_LOG_INFO("{}: host_lif_stats_addr {:#x}",
                    hal_lif_info_.name, host_lif_stats_addr);

        // starts a non-repeating timer to update stats. the timer is reset when
        // stats update is complete.
        evutil_timer_start(&stats_timer, &EthLif::StatsUpdate, this, 0.2, 0.0);
    }

    // Init the status block
    lif_status->eid = 0;
    lif_status->link_flap_count = 0;

    // Init the stats region
    MEM_SET(lif_stats_addr, 0, LIF_STATS_SIZE, 0);

    state = LIF_STATE_INIT;

    return (IONIC_RC_SUCCESS);
}

void
EthLif::FreeUpMacFilters()
{
    uint64_t filter_id;
    indexer::status rs;

    for (auto it = mac_addrs.cbegin(); it != mac_addrs.cend();) {
        filter_id = it->first;
        rs = fltr_allocator->free(filter_id);
        if (rs != indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to free filter_id: {}, err: {}",
                          filter_id, rs);
            // return (IONIC_RC_ERROR);
        }
        NIC_LOG_DEBUG("Freed filter_id: {}", filter_id);
        it = mac_addrs.erase(it);
    }
}

void
EthLif::FreeUpVlanFilters()
{
    uint64_t filter_id;
    indexer::status rs;

    for (auto it = vlans.cbegin(); it != vlans.cend();) {
        filter_id = it->first;
        rs = fltr_allocator->free(filter_id);
        if (rs != indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to free filter_id: {}, err: {}",
                          filter_id, rs);
            // return (IONIC_RC_ERROR);
        }
        NIC_LOG_DEBUG("Freed filter_id: {}", filter_id);
        it = vlans.erase(it);
    }
}

void
EthLif::FreeUpMacVlanFilters()
{
    uint64_t filter_id;
    indexer::status rs;

    for (auto it = mac_vlans.cbegin(); it != mac_vlans.cend();) {
        filter_id = it->first;
        rs = fltr_allocator->free(filter_id);
        if (rs != indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to free filter_id: {}, err: {}",
                          filter_id, rs);
            // return (IONIC_RC_ERROR);
        }
        NIC_LOG_DEBUG("Freed filter_id: {}", filter_id);
        it = mac_vlans.erase(it);
    }
}

status_code_t
EthLif::Reset(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;

    NIC_LOG_DEBUG("{}: LIF_RESET", hal_lif_info_.name);

    if (state == LIF_STATE_CREATED || state == LIF_STATE_RESET) {
        NIC_LOG_WARN("{}: {} + RESET => {}", hal_lif_info_.name,
            lif_state_to_str(state),
            lif_state_to_str(state));
        return (IONIC_RC_SUCCESS);
    }

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    state = LIF_STATE_RESETING;

    // Update name to the lif-id before doing a reset
    // to avoid name collisions during re-addition of the lifs
    // TODO: Lif delete has to be called here instead of just
    // doing an update
#if 0
    lif->UpdateName(std::to_string(lif->GetHwLifId()));
    lif->Reset();
#endif
    DEVAPI_CHECK
    dev_api->lif_upd_name(hal_lif_info_.lif_id,
                          std::to_string(hal_lif_info_.lif_id));
    dev_api->lif_reset(hal_lif_info_.lif_id);
    FreeUpMacFilters();
    FreeUpVlanFilters();
    FreeUpMacVlanFilters();

#ifndef APOLLO
    int ret;
    // Reset RSS configuration
    rss_type = LIF_RSS_TYPE_NONE;
    memset(rss_key, 0x00, RSS_HASH_KEY_SIZE);
    memset(rss_indir, 0x00, RSS_IND_TBL_SIZE);
    ret = pd->eth_program_rss(hal_lif_info_.lif_id, rss_type, rss_key, rss_indir,
                              spec->rxq_count);
    if (ret != 0) {
        NIC_LOG_DEBUG("{}: Unable to program hw for RSS HASH", ret);
        return (IONIC_RC_ERROR);
    }
#endif

    // Clear PC to drop all traffic
    for (uint32_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_RX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, fldsiz(eth_rx_qstate_t, pc_offset), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(eth_rx_qstate_t), P4PLUS_CACHE_INVALIDATE_BOTH);
    }

    for (uint32_t qid = 0; qid < spec->txq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_TX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, fldsiz(eth_tx_qstate_t, pc_offset), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(eth_tx_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_ADMIN, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, fldsiz(admin_qstate_t, pc_offset), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(admin_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
    }

    // Reset EDMA service
    edmaq->Reset();

    // Reset NotifyQ service
    notify_enabled = 0;

    // Reset ADMINQ service
    adminq->Reset();

    // Disable Stats
    if (host_lif_stats_addr != 0) {
        host_lif_stats_addr = 0;
        evutil_timer_stop(&stats_timer);
    }

    state = LIF_STATE_RESET;

    return (IONIC_RC_SUCCESS);
}

bool
EthLif::EdmaProxy(edma_opcode opcode, uint64_t from, uint64_t to, uint16_t size,
    struct edmaq_ctx *ctx)
{
    return edmaq->Post(opcode, from, to, size, ctx);
}

status_code_t
EthLif::AdminQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr, nicmgr_qstate_addr;
    struct q_init_cmd *cmd = (struct q_init_cmd *)req;
    struct q_init_comp *comp = (struct q_init_comp *)resp;

    NIC_LOG_DEBUG("{}: {}: "
        "type {} index {} ring_base {:#x} ring_size {} intr_index {} flags {}{}",
        hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->opcode),
        cmd->type,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        (cmd->flags & IONIC_QINIT_F_IRQ) ? 'I' : '-',
        (cmd->flags & IONIC_QINIT_F_ENA) ? 'E' : '-');

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (cmd->index >= spec->adminq_count) {
        NIC_LOG_ERR("{}: bad qid {}", hal_lif_info_.name, cmd->index);
        return (IONIC_RC_EQID);
    }

    if ((cmd->flags & IONIC_QINIT_F_IRQ) && cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("{}: bad intr {}", hal_lif_info_.name, cmd->intr_index);
        return (IONIC_RC_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("{}: bad ring size {}", hal_lif_info_.name, cmd->ring_size);
        return (IONIC_RC_ERROR);
    }

    if (cmd->ring_base & ~BIT_MASK(52)) {
        NIC_LOG_ERR("{}: bad ring base {:#x}", hal_lif_info_.name, cmd->ring_base);
        return (IONIC_RC_EINVAL);
    }

    addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_ADMIN,
                cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for ADMIN qid {}",
            hal_lif_info_.name, cmd->index);
        return (IONIC_RC_ERROR);
    }

    nicmgr_qstate_addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id,
        ETH_ADMINQ_REQ_QTYPE, ETH_ADMINQ_REQ_QID);
    if (nicmgr_qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get request qstate address for ADMIN qid {}",
            hal_lif_info_.name, cmd->index);
        return (IONIC_RC_ERROR);
    }

    uint8_t off;
    admin_qstate_t qstate = {0};
    if (pd->get_pc_offset("txdma_stage0.bin", "adminq_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: adminq_stage0");
        return (IONIC_RC_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = ctl_cosA;
    qstate.cosB = ctl_cosB;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.ci_fetch = 0;
    qstate.sta.color = 1;
    qstate.cfg.enable = (cmd->flags & IONIC_QINIT_F_ENA) ? 1 : 0;
    qstate.cfg.intr_enable = (cmd->flags & IONIC_QINIT_F_IRQ) ? 1 : 0;
    qstate.cfg.host_queue = spec->host_dev;
    if (spec->host_dev) {
        qstate.ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->ring_base);
        qstate.cq_ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->cq_ring_base);
    } else {
        qstate.ring_base = cmd->ring_base;
        qstate.cq_ring_base = cmd->cq_ring_base;
    }
    qstate.ring_size = cmd->ring_size;
    if (cmd->flags & IONIC_QINIT_F_IRQ)
        qstate.intr_assert_index = res->intr_base + cmd->intr_index;
    qstate.nicmgr_qstate_addr = nicmgr_qstate_addr;

    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(qstate), P4PLUS_CACHE_INVALIDATE_TXDMA);

    comp->hw_index = cmd->index;
    comp->hw_type = ETH_HW_QTYPE_ADMIN;

    NIC_LOG_DEBUG("{}: qid {} qtype {}", hal_lif_info_.name, comp->hw_index, comp->hw_type);

    return (IONIC_RC_SUCCESS);
}

void
EthLif::AdminCmdHandler(void *obj,
    void *req, void *req_data, void *resp, void *resp_data)
{
    EthLif *lif = (EthLif *)obj;
    lif->CmdHandler(req, req_data, resp, resp_data);
}

status_code_t
EthLif::CmdHandler(void *req, void *req_data,
    void *resp, void *resp_data)
{
    union dev_cmd *cmd = (union dev_cmd *)req;
    union dev_cmd_comp *comp = (union dev_cmd_comp *)resp;
    status_code_t status = IONIC_RC_SUCCESS;

    NIC_LOG_DEBUG("{}: Handling cmd: {}", hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->cmd.opcode));

    switch ((cmd_opcode_t)cmd->cmd.opcode) {

    case CMD_OPCODE_NOP:
        status = IONIC_RC_SUCCESS;
        break;

    case CMD_OPCODE_LIF_GETATTR:
        status = _CmdGetAttr(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_LIF_SETATTR:
        status = _CmdSetAttr(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RX_MODE_SET:
        status = _CmdRxSetMode(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RX_FILTER_ADD:
        status = _CmdRxFilterAdd(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RX_FILTER_DEL:
        status = _CmdRxFilterDel(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_Q_INIT:
        status = _CmdQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_Q_CONTROL:
        status = _CmdQControl(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RDMA_RESET_LIF:
        status = _CmdRDMAResetLIF(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RDMA_CREATE_EQ:
        status = _CmdRDMACreateEQ(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RDMA_CREATE_CQ:
        status = _CmdRDMACreateCQ(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RDMA_CREATE_ADMINQ:
        status = _CmdRDMACreateAdminQ(req, req_data, resp, resp_data);
        break;

    default:
        NIC_LOG_ERR("{}: Unknown Opcode {}", hal_lif_info_.name, cmd->cmd.opcode);
        status = IONIC_RC_EOPCODE;
        break;
    }

    comp->comp.status = status;
    comp->comp.rsvd = 0xff;
    NIC_LOG_DEBUG("{}: Done cmd: {}, status: {}", hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->cmd.opcode), status);

    return (status);
}

status_code_t
EthLif::CmdProxyHandler(void *req, void *req_data,
    void *resp, void *resp_data)
{
    // Allow all commands to be proxied for now
    return CmdHandler(req, req_data, resp, resp_data);
}

#if 0
status_code_t
EthLif::_CmdHangNotify(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    eth_rx_qstate_t rx_qstate = {0};
    eth_tx_qstate_t tx_qstate = {0};
    admin_qstate_t aq_state;
    intr_state_t intr_st;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_HANG_NOTIFY", hal_lif_info_.name);

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    for (uint32_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_RX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&rx_qstate), sizeof(rx_qstate), 0);
        NIC_LOG_DEBUG("{}: rxq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.name, qid,
            rx_qstate.p_index0, rx_qstate.c_index0, rx_qstate.comp_index,
            rx_qstate.intr_assert_index);
    }

    for (uint32_t qid = 0; qid < spec->txq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_TX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&tx_qstate), sizeof(tx_qstate), 0);
        NIC_LOG_DEBUG("{}: txq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.name, qid,
            tx_qstate.p_index0, tx_qstate.c_index0, tx_qstate.comp_index,
            tx_qstate.intr_assert_index);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_ADMIN, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&aq_state), sizeof(aq_state), 0);
        NIC_LOG_DEBUG("{}: adminq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.name, qid,
            aq_state.p_index0, aq_state.c_index0, aq_state.comp_index,
            aq_state.intr_assert_index);
    }

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_state_get(res->intr_base + intr, &intr_st);
        NIC_LOG_DEBUG("{}: intr{}: fwcfg_lif {} fwcfg_function_mask {}"
            " drvcfg_mask {} drvcfg_int_credits {} drvcfg_mask_on_assert {}",
            hal_lif_info_.name, res->intr_base + intr,
            intr_st.fwcfg_lif, intr_st.fwcfg_function_mask,
            intr_st.drvcfg_mask, intr_st.drvcfg_int_credits,
            intr_st.drvcfg_mask_on_assert);
    }

    return (IONIC_RC_SUCCESS);
}
#endif

status_code_t
EthLif::_CmdQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    status_code_t ret = IONIC_RC_ERROR;
    struct q_init_cmd *cmd = (struct q_init_cmd *)req;

    switch (cmd->type) {
        case IONIC_QTYPE_ADMINQ:
            ret = AdminQInit(req, req_data, resp, resp_data);
            break;
        case IONIC_QTYPE_NOTIFYQ:
            ret = NotifyQInit(req, req_data, resp, resp_data);
            break;
        case IONIC_QTYPE_RXQ:
            ret = RxQInit(req, req_data, resp, resp_data);
            break;
        case IONIC_QTYPE_TXQ:
            ret = TxQInit(req, req_data, resp, resp_data);
            break;
        default:
            ret = IONIC_RC_EINVAL;
            NIC_LOG_ERR("{}: Invalid qtype {} qid {}", hal_lif_info_.name,
                cmd->type, cmd->index);
            break;
    }

    return ret;
}

status_code_t
EthLif::TxQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct q_init_cmd *cmd = (struct q_init_cmd *)req;
    struct q_init_comp *comp = (struct q_init_comp *)resp;
    eth_tx_qstate_t qstate = {0};

    NIC_LOG_DEBUG("{}: {}: "
        "type {} index {} cos {} ring_base {:#x} ring_size {} intr_index {} flags {}{}{}",
        hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->opcode),
        cmd->type,
        cmd->index,
        cmd->cos,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        (cmd->flags & IONIC_QINIT_F_SG) ? 'S' : '-',
        (cmd->flags & IONIC_QINIT_F_IRQ) ? 'I' : '-',
        (cmd->flags & IONIC_QINIT_F_ENA) ? 'E' : '-');

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (cmd->index >= spec->txq_count) {
        NIC_LOG_ERR("{}: bad qid {}", hal_lif_info_.name, cmd->index);
        return (IONIC_RC_EQID);
    }

    if ((cmd->flags & IONIC_QINIT_F_IRQ) && cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("{}: bad intr {}", hal_lif_info_.name, cmd->intr_index);
        return (IONIC_RC_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("{}: bad ring_size {}", hal_lif_info_.name, cmd->ring_size);
        return (IONIC_RC_ERROR);
    }

    if (cmd->ring_base & ~BIT_MASK(52)) {
        NIC_LOG_ERR("{}: bad ring base {:#x}", hal_lif_info_.name, cmd->ring_base);
        return (IONIC_RC_EINVAL);
    }

    addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_TX,
                cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for TX qid {}",
            hal_lif_info_.name, cmd->index);
        return (IONIC_RC_ERROR);
    }

    uint8_t off;
    if (pd->get_pc_offset("txdma_stage0.bin", "eth_tx_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: eth_tx_stage0");
        return (IONIC_RC_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = cosA;
    qstate.cosB = cosB;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.ci_fetch = 0;
    qstate.ci_miss = 0;
    qstate.sta.color = 1;
    qstate.sta.spec_miss = 0;
    qstate.cfg.enable = (cmd->flags & IONIC_QINIT_F_ENA) ? 1 : 0;
    qstate.cfg.intr_enable = (cmd->flags & IONIC_QINIT_F_IRQ) ? 1 : 0;
    qstate.cfg.host_queue = spec->host_dev;
    if (spec->host_dev) {
        qstate.ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->ring_base);
        qstate.cq_ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->cq_ring_base);
        if (cmd->flags & IONIC_QINIT_F_SG)
            qstate.sg_ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->sg_ring_base);
    } else {
        qstate.ring_base = cmd->ring_base;
        qstate.cq_ring_base = cmd->cq_ring_base;
        if (cmd->flags & IONIC_QINIT_F_SG)
            qstate.sg_ring_base = cmd->sg_ring_base;
    }
    qstate.ring_size = cmd->ring_size;
    if (cmd->flags & IONIC_QINIT_F_IRQ)
        qstate.intr_assert_index = res->intr_base + cmd->intr_index;
    qstate.spurious_db_cnt = 0;

    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(qstate), P4PLUS_CACHE_INVALIDATE_TXDMA);

    comp->hw_index = cmd->index;
    comp->hw_type = ETH_HW_QTYPE_TX;

    NIC_LOG_DEBUG("{}: qid {} qtype {}",
                 hal_lif_info_.name, comp->hw_index, comp->hw_type);
    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::RxQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct q_init_cmd *cmd = (struct q_init_cmd *)req;
    struct q_init_comp *comp = (struct q_init_comp *)resp;
    eth_rx_qstate_t qstate = {0};

    NIC_LOG_DEBUG("{}: {}: "
        "type {} index {} cos {} ring_base {:#x} ring_size {} intr_index {} flags {}{}{}",
        hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->opcode),
        cmd->type,
        cmd->index,
        cmd->cos,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        (cmd->flags & IONIC_QINIT_F_SG) ? 'S' : '-',
        (cmd->flags & IONIC_QINIT_F_IRQ) ? 'I' : '-',
        (cmd->flags & IONIC_QINIT_F_ENA) ? 'E' : '-');

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (cmd->index >= spec->rxq_count) {
        NIC_LOG_ERR("{}: bad qid {}", hal_lif_info_.name, cmd->index);
        return (IONIC_RC_EQID);
    }

    if ((cmd->flags & IONIC_QINIT_F_IRQ) && cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("{}: bad intr {}", hal_lif_info_.name, cmd->intr_index);
        return (IONIC_RC_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("{}: bad ring_size {}", hal_lif_info_.name, cmd->ring_size);
        return (IONIC_RC_ERROR);
    }

    if (cmd->ring_base & ~BIT_MASK(52)) {
        NIC_LOG_ERR("{}: bad ring base {:#x}", hal_lif_info_.name, cmd->ring_base);
        return (IONIC_RC_EINVAL);
    }

    addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_RX,
        cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for RX qid {}",
            hal_lif_info_.name, cmd->index);
        return (IONIC_RC_ERROR);
    }

    uint8_t off;
    if (pd->get_pc_offset("rxdma_stage0.bin", "eth_rx_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: rxdma_stage0.bin label: eth_rx_stage0");
        return (IONIC_RC_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = cosA;
    qstate.cosB = cosB;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.sta.color = 1;
    qstate.cfg.enable = (cmd->flags & IONIC_QINIT_F_ENA) ? 1 : 0;
    qstate.cfg.intr_enable = (cmd->flags & IONIC_QINIT_F_IRQ) ? 1 : 0;
    qstate.cfg.host_queue = spec->host_dev;
    if (spec->host_dev) {
        qstate.ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->ring_base);
        qstate.cq_ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->cq_ring_base);
        if (cmd->flags & IONIC_QINIT_F_SG)
            qstate.sg_ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->sg_ring_base);
    } else {
        qstate.ring_base = cmd->ring_base;
        qstate.cq_ring_base = cmd->cq_ring_base;
        if (cmd->flags & IONIC_QINIT_F_SG)
            qstate.sg_ring_base = cmd->sg_ring_base;
    }
    qstate.ring_size = cmd->ring_size;
    if (cmd->flags & IONIC_QINIT_F_IRQ)
        qstate.intr_assert_index = res->intr_base + cmd->intr_index;

    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(qstate), P4PLUS_CACHE_INVALIDATE_BOTH);

    comp->hw_index = cmd->index;
    comp->hw_type = ETH_HW_QTYPE_RX;

    NIC_LOG_DEBUG("{}: qid {} qtype {}",
                 hal_lif_info_.name, comp->hw_index, comp->hw_type);
    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::NotifyQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct q_init_cmd *cmd = (struct q_init_cmd *)req;
    struct q_init_comp *comp = (struct q_init_comp *)resp;

    NIC_LOG_INFO("{}: {}: "
        "type {} index {} ring_base {:#x} ring_size {} intr_index {} flags {}{}",
        hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->opcode),
        cmd->type,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        (cmd->flags & IONIC_QINIT_F_IRQ) ? 'I' : '-',
        (cmd->flags & IONIC_QINIT_F_ENA) ? 'E' : '-');

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (cmd->index >= spec->eq_count) {
        NIC_LOG_ERR("{}: bad qid {}", hal_lif_info_.name, cmd->index);
        return (IONIC_RC_EQID);
    }

    if ((cmd->flags & IONIC_QINIT_F_IRQ) && cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("{}: bad intr {}", hal_lif_info_.name, cmd->intr_index);
        return (IONIC_RC_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("{}: bad ring_size {}", hal_lif_info_.name, cmd->ring_size);
        return (IONIC_RC_ERROR);
    }

    if (cmd->ring_base & ~BIT_MASK(52)) {
        NIC_LOG_ERR("{}: bad ring base {:#x}", hal_lif_info_.name, cmd->ring_base);
        return (IONIC_RC_EINVAL);
    }

    addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_NOTIFYQ_QTYPE,
            cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for NOTIFYQ qid {}",
            hal_lif_info_.name, cmd->index);
        return (IONIC_RC_ERROR);
    }

    uint8_t off;
    uint64_t host_ring_base;
    notify_qstate_t qstate = {0};
    if (pd->get_pc_offset("txdma_stage0.bin", "notify_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR("Failed to resolve program: txdma_stage0.bin label: notify_stage0");
        return (IONIC_RC_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = ctl_cosA;
    qstate.cosB = ctl_cosB;
    qstate.host = 0;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.host_pindex = 0;
    qstate.sta = {0};
    qstate.cfg.enable = (cmd->flags & IONIC_QINIT_F_ENA) ? 1 : 0;
    qstate.cfg.intr_enable = (cmd->flags & IONIC_QINIT_F_IRQ) ? 1 : 0;
    qstate.cfg.host_queue = spec->host_dev;
    qstate.ring_base = notify_ring_base;
    qstate.ring_size = LG2_ETH_NOTIFYQ_RING_SIZE;
    if (spec->host_dev)
        host_ring_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->ring_base);
    else
        host_ring_base = cmd->ring_base;
    qstate.host_ring_base = roundup(host_ring_base + (sizeof(notifyq_comp) << cmd->ring_size), 4096);
    qstate.host_ring_size = cmd->ring_size;
    if (cmd->flags & IONIC_QINIT_F_IRQ)
        qstate.host_intr_assert_index = res->intr_base + cmd->intr_index;

    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(qstate), P4PLUS_CACHE_INVALIDATE_TXDMA);

    comp->hw_index = cmd->index;
    comp->hw_type = ETH_HW_QTYPE_SVC;

    NIC_LOG_INFO("{}: qid {} qtype {}", hal_lif_info_.name,
        comp->hw_index, comp->hw_type);

    // Enable notifications
    notify_enabled = 1;

    /*
     * EDMA is not going through. We tried to write a well-known pattern during lif
     * initialization in driver and thats being not updated without this hack.
     * This hack tries to update the notify block multiple times and in the
     * worst case we see the third attempt going through. Along with the original
     * notify block update we retry 3 times.
     */
#ifdef APOLLO
    uint8_t count = 3;
    while(count--) {
        port_status_t retry_status = {0};
        retry_status.status = lif_status->link_status;
        retry_status.speed = lif_status->link_speed;
        retry_status.id = spec->uplink_port_num;
        NIC_LOG_INFO("{}: Retrying link event status: {}, speed: {}, port_num: {}",
                     hal_lif_info_.name, retry_status.status, retry_status.speed,
                     retry_status.id);
        LinkEventHandler(&retry_status);
        usleep(1000 * 1000);
    }
#endif

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::SetFeatures(void *req, void *req_data, void *resp, void *resp_data)
{
    struct lif_setattr_cmd *cmd = (struct lif_setattr_cmd *)req;
    struct lif_setattr_comp *comp = (struct lif_setattr_comp *)resp;
    sdk_ret_t ret = SDK_RET_OK;

    NIC_LOG_DEBUG("{}: wanted "
        "vlan_strip {} vlan_insert {} rx_csum {} tx_csum {} rx_hash {} tx_sg {} rx_sg {}",
        hal_lif_info_.name,
        (cmd->features & ETH_HW_VLAN_RX_STRIP) ? 1 : 0,
        (cmd->features & ETH_HW_VLAN_TX_TAG) ? 1 : 0,
        (cmd->features & ETH_HW_RX_CSUM) ? 1 : 0,
        (cmd->features & ETH_HW_TX_CSUM) ? 1 : 0,
        (cmd->features & ETH_HW_RX_HASH) ? 1 : 0,
        (cmd->features & ETH_HW_TX_SG)  ? 1 : 0,
        (cmd->features & ETH_HW_RX_SG)  ? 1 : 0
    );

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    comp->status = 0;
    comp->features = (
                       ETH_HW_VLAN_RX_STRIP |
                       ETH_HW_VLAN_TX_TAG |
                       ETH_HW_VLAN_RX_FILTER |
                       ETH_HW_RX_CSUM |
                       ETH_HW_TX_CSUM |
                       ETH_HW_RX_HASH |
                       ETH_HW_TX_SG |
                       ETH_HW_RX_SG |
                       ETH_HW_TSO |
                       ETH_HW_TSO_IPV6);

    bool vlan_strip = cmd->features & comp->features & ETH_HW_VLAN_RX_STRIP;
    bool vlan_insert = cmd->features & comp->features & ETH_HW_VLAN_TX_TAG;

    DEVAPI_CHECK
    ret = dev_api->lif_upd_vlan_offload(hal_lif_info_.lif_id,
                                        vlan_strip, vlan_insert);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to update Vlan offload",
            hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    NIC_LOG_INFO("{}: vlan_strip {} vlan_insert {}", hal_lif_info_.name,
        vlan_strip, vlan_insert);

    NIC_LOG_DEBUG("{}: supported {}", hal_lif_info_.name, comp->features);

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdGetAttr(void *req, void *req_data, void *resp, void *resp_data)
{
    struct lif_getattr_cmd *cmd = (struct lif_getattr_cmd *)req;
    struct lif_getattr_comp *comp = (struct lif_getattr_comp *)resp;
    uint64_t mac_addr;

    NIC_LOG_DEBUG("{}: {}: attr {}",
        hal_lif_info_.name, opcode_to_str((cmd_opcode_t)cmd->opcode),
        cmd->attr);

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    switch (cmd->attr) {
        case IONIC_LIF_ATTR_STATE:
            break;
        case IONIC_LIF_ATTR_NAME:
            break;
        case IONIC_LIF_ATTR_MTU:
            break;
        case IONIC_LIF_ATTR_MAC:
            mac_addr = be64toh(spec->mac_addr) >> (8 * sizeof(spec->mac_addr) - 8 * sizeof(uint8_t[6]));
            memcpy((uint8_t *)comp->mac, (uint8_t *)&mac_addr, sizeof(comp->mac));
            NIC_LOG_DEBUG("{}: station mac address {}", hal_lif_info_.name,
                macaddr2str(mac_addr));
            break;
        case IONIC_LIF_ATTR_FEATURES:
            break;
        default:
            NIC_LOG_ERR("{}: UNKNOWN ATTR {}", hal_lif_info_.name, cmd->attr);
            return (IONIC_RC_ERROR);
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdSetAttr(void *req, void *req_data, void *resp, void *resp_data)
{
    struct lif_setattr_cmd *cmd = (struct lif_setattr_cmd *)req;

    NIC_LOG_DEBUG("{}: {}: attr {}",
        hal_lif_info_.name, opcode_to_str(cmd->opcode), cmd->attr);

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    switch (cmd->attr) {
        case IONIC_LIF_ATTR_STATE:
            break;
        case IONIC_LIF_ATTR_NAME:
            strncpy0(name, cmd->name, sizeof(name));
            strncpy0(hal_lif_info_.name, cmd->name, sizeof(hal_lif_info_.name));
            DEVAPI_CHECK
            dev_api->lif_upd_name(hal_lif_info_.lif_id, name);
            break;
        case IONIC_LIF_ATTR_MTU:
            break;
        case IONIC_LIF_ATTR_MAC:
            break;
        case IONIC_LIF_ATTR_FEATURES:
            return SetFeatures(req, req_data, resp, resp_data);
        case IONIC_LIF_ATTR_RSS:
            return RssConfig(req, req_data, resp, resp_data);
        default:
            NIC_LOG_ERR("{}: UNKNOWN ATTR {}", hal_lif_info_.name, cmd->attr);
            return (IONIC_RC_ERROR);
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdQControl(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct q_control_cmd *cmd = (struct q_control_cmd *)req;
    // q_enable_comp *comp = (q_enable_comp *)resp;
    struct eth_rx_cfg_qstate rx_cfg = {0};
    struct eth_tx_cfg_qstate tx_cfg = {0};
    struct admin_cfg_qstate admin_cfg = {0};

    NIC_LOG_DEBUG("{}: {}: type {} index {} oper {}",
        hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->opcode),
        cmd->type, cmd->index, cmd->oper);

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (cmd->type >= 8) {
        NIC_LOG_ERR("{}: bad qtype {}", hal_lif_info_.name, cmd->type);
        return (IONIC_RC_EQTYPE);
    }

    switch (cmd->type) {
    case IONIC_QTYPE_RXQ:
        if (cmd->index >= spec->rxq_count) {
            NIC_LOG_ERR("{}: bad qid {}", hal_lif_info_.name, cmd->index);
            return (IONIC_RC_EQID);
        }
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_RX, cmd->index);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.name, cmd->index);
            return (IONIC_RC_ERROR);
        }
        READ_MEM(addr + offsetof(eth_rx_qstate_t, cfg), (uint8_t *)&rx_cfg, sizeof(rx_cfg), 0);
        if (cmd->oper == IONIC_Q_ENABLE)
            rx_cfg.enable = 0x1;
        else if (cmd->oper == IONIC_Q_DISABLE)
            rx_cfg.enable = 0x0;
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, cfg), (uint8_t *)&rx_cfg, sizeof(rx_cfg), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(eth_rx_qstate_t), P4PLUS_CACHE_INVALIDATE_BOTH);
        active_q_ref_cnt--;
        break;
    case IONIC_QTYPE_TXQ:
        if (cmd->index >= spec->txq_count) {
            NIC_LOG_ERR("{}: bad qid {}", hal_lif_info_.name, cmd->index);
            return (IONIC_RC_EQID);
        }
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_TX, cmd->index);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.name, cmd->index);
            return (IONIC_RC_ERROR);
        }
        READ_MEM(addr + offsetof(eth_tx_qstate_t, cfg), (uint8_t *)&tx_cfg, sizeof(tx_cfg), 0);
        if (cmd->oper == IONIC_Q_ENABLE)
            tx_cfg.enable = 0x1;
        else if (cmd->oper == IONIC_Q_DISABLE)
            tx_cfg.enable = 0x0;
        WRITE_MEM(addr + offsetof(eth_tx_qstate_t, cfg), (uint8_t *)&tx_cfg, sizeof(tx_cfg), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(eth_tx_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
        active_q_ref_cnt--;
        break;
    case IONIC_QTYPE_ADMINQ:
        if (cmd->index >= spec->adminq_count) {
            NIC_LOG_ERR("{}: bad qid {}", hal_lif_info_.name, cmd->index);
            return (IONIC_RC_EQID);
        }
        addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, ETH_HW_QTYPE_ADMIN, cmd->index);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.name, cmd->index);
            return (IONIC_RC_ERROR);
        }
        READ_MEM(addr + offsetof(admin_qstate_t, cfg), (uint8_t *)&admin_cfg, sizeof(admin_cfg), 0);
        if (cmd->oper == IONIC_Q_ENABLE)
            admin_cfg.enable = 0x1;
        else if (cmd->oper == IONIC_Q_DISABLE)
            admin_cfg.enable = 0x0;
        WRITE_MEM(addr + offsetof(admin_qstate_t, cfg), (uint8_t *)&admin_cfg, sizeof(admin_cfg), 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(admin_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);
        break;
    default:
        NIC_LOG_ERR("{}: invalid qtype {}", hal_lif_info_.name, cmd->type);
        return (IONIC_RC_ERROR);
        break;
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdRxSetMode(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rx_mode_set_cmd *cmd = (struct rx_mode_set_cmd *)req;
    // rx_mode_set_comp *comp = (rx_mode_set_comp *)resp;
    sdk_ret_t ret = SDK_RET_OK;

    NIC_LOG_DEBUG("{}: {}: rx_mode {} {}{}{}{}{}",
            hal_lif_info_.name,
            opcode_to_str((cmd_opcode_t)cmd->opcode),
            cmd->rx_mode,
            cmd->rx_mode & RX_MODE_F_UNICAST   ? 'u' : '-',
            cmd->rx_mode & RX_MODE_F_MULTICAST ? 'm' : '-',
            cmd->rx_mode & RX_MODE_F_BROADCAST ? 'b' : '-',
            cmd->rx_mode & RX_MODE_F_PROMISC   ? 'p' : '-',
            cmd->rx_mode & RX_MODE_F_ALLMULTI  ? 'a' : '-');

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    bool broadcast = cmd->rx_mode & RX_MODE_F_BROADCAST;
    bool all_multicast = cmd->rx_mode & RX_MODE_F_ALLMULTI;
    bool promiscuous = cmd->rx_mode & RX_MODE_F_PROMISC;
#if 0
    ret = lif->UpdateReceiveMode(broadcast, all_multicast, promiscuous);
    if (ret != HAL_IRISC_RET_SUCCESS) {
#endif
    DEVAPI_CHECK
    ret = dev_api->lif_upd_rx_mode(hal_lif_info_.lif_id,
                                   broadcast, all_multicast, promiscuous);
    if (ret != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to update rx mode",
            hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdRxFilterAdd(void *req, void *req_data, void *resp, void *resp_data)
{
    //int status;
    uint64_t mac_addr;
    uint16_t vlan;
    uint32_t filter_id = 0;
    struct rx_filter_add_cmd *cmd = (struct rx_filter_add_cmd *)req;
    struct rx_filter_add_comp *comp = (struct rx_filter_add_comp *)resp;
    sdk_ret_t ret = SDK_RET_OK;

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    if (cmd->match == RX_FILTER_MATCH_MAC) {

        memcpy((uint8_t *)&mac_addr, (uint8_t *)&cmd->mac.addr, sizeof(cmd->mac.addr));
        mac_addr = be64toh(mac_addr) >> (8 * sizeof(mac_addr) - 8 * sizeof(cmd->mac.addr));

        NIC_LOG_DEBUG("{}: Add RX_FILTER_MATCH_MAC mac {}",
            hal_lif_info_.name, macaddr2str(mac_addr));

        // ret = lif->AddMac(mac_addr);
        DEVAPI_CHECK
        ret = dev_api->lif_add_mac(hal_lif_info_.lif_id, mac_addr);

        if (ret != SDK_RET_OK) {
            NIC_LOG_WARN("{}: Failed Add Mac:{} ret: {}",
                         hal_lif_info_.name, macaddr2str(mac_addr), ret);
            if (ret == sdk::SDK_RET_ENTRY_EXISTS)
                return (IONIC_RC_EEXIST);
            else
                return (IONIC_RC_ERROR);
        }
 
        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate MAC address filter");
            return (IONIC_RC_ERROR);
        }
        mac_addrs[filter_id] = mac_addr;
    } else if (cmd->match == RX_FILTER_MATCH_VLAN) {
        vlan = cmd->vlan.vlan;

        NIC_LOG_DEBUG("{}: Add RX_FILTER_MATCH_VLAN vlan {}",
                      hal_lif_info_.name, vlan);
        // ret = lif->AddVlan(vlan);
        DEVAPI_CHECK
        ret = dev_api->lif_add_vlan(hal_lif_info_.lif_id, vlan);
        if (ret != SDK_RET_OK) {
            NIC_LOG_WARN("{}: Failed Add Vlan:{}. ret: {}", hal_lif_info_.name, vlan, ret);
            if (ret == sdk::SDK_RET_ENTRY_EXISTS)
                return (IONIC_RC_EEXIST);
            else
                return (IONIC_RC_ERROR);
        }

        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate VLAN filter");
            return (IONIC_RC_ERROR);
        }
        vlans[filter_id] = vlan;
    } else {
        memcpy((uint8_t *)&mac_addr, (uint8_t *)&cmd->mac_vlan.addr, sizeof(cmd->mac_vlan.addr));
        mac_addr = be64toh(mac_addr) >> (8 * sizeof(mac_addr) - 8 * sizeof(cmd->mac_vlan.addr));
        vlan = cmd->mac_vlan.vlan;

        NIC_LOG_DEBUG("{}: Add RX_FILTER_MATCH_MAC_VLAN mac {} vlan {}",
                      hal_lif_info_.name, macaddr2str(mac_addr), vlan);

        // ret = lif->AddMacVlan(mac_addr, vlan);
        DEVAPI_CHECK
        ret = dev_api->lif_add_macvlan(hal_lif_info_.lif_id,
                                       mac_addr, vlan);
        if (ret != SDK_RET_OK) {
            NIC_LOG_WARN("{}: Failed Add Mac-Vlan:{}-{}. ret: {}", hal_lif_info_.name,
                         macaddr2str(mac_addr), vlan, ret);
            if (ret == sdk::SDK_RET_ENTRY_EXISTS)
                return (IONIC_RC_EEXIST);
            else
                return (IONIC_RC_ERROR);
        }

        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate VLAN filter");
            return (IONIC_RC_ERROR);
        }
        mac_vlans[filter_id] = std::make_tuple(mac_addr, vlan);
    }

    comp->filter_id = filter_id;
    NIC_LOG_DEBUG("{}: filter_id {}",
                 hal_lif_info_.name, comp->filter_id);
    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdRxFilterDel(void *req, void *req_data, void *resp, void *resp_data)
{
    //int status;
    sdk_ret_t ret;
    uint64_t mac_addr;
    uint16_t vlan;
    struct rx_filter_del_cmd *cmd = (struct rx_filter_del_cmd *)req;
    //struct rx_filter_del_comp *comp = (struct rx_filter_del_comp *)resp;
    indexer::status rs;

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    if (!hal_status) {
        NIC_LOG_ERR("{}: HAL is not UP!", spec->name);
        return (IONIC_RC_EAGAIN);
    }

    if (mac_addrs.find(cmd->filter_id) != mac_addrs.end()) {
        mac_addr = mac_addrs[cmd->filter_id];
        NIC_LOG_DEBUG("{}: Del RX_FILTER_MATCH_MAC mac {}",
                      hal_lif_info_.name,
                      macaddr2str(mac_addr));
        DEVAPI_CHECK
        ret = dev_api->lif_del_mac(hal_lif_info_.lif_id, mac_addr);
        mac_addrs.erase(cmd->filter_id);
    } else if (vlans.find(cmd->filter_id) != vlans.end()) {
        vlan = vlans[cmd->filter_id];
        NIC_LOG_DEBUG("{}: Del RX_FILTER_MATCH_VLAN vlan {}",
                     hal_lif_info_.name, vlan);
        DEVAPI_CHECK
        ret = dev_api->lif_del_vlan(hal_lif_info_.lif_id, vlan);
        vlans.erase(cmd->filter_id);
    } else if (mac_vlans.find(cmd->filter_id) != mac_vlans.end()) {
        auto mac_vlan = mac_vlans[cmd->filter_id];
        mac_addr = std::get<0>(mac_vlan);
        vlan = std::get<1>(mac_vlan);
        NIC_LOG_DEBUG("{}: Del RX_FILTER_MATCH_MAC_VLAN mac {} vlan {}",
                     hal_lif_info_.name, macaddr2str(mac_addr), vlan);
        DEVAPI_CHECK
        ret = dev_api->lif_del_macvlan(hal_lif_info_.lif_id, mac_addr, vlan);
        mac_vlans.erase(cmd->filter_id);
    } else {
        NIC_LOG_ERR("Invalid filter(Non-exist) id {}", cmd->filter_id);
        return (IONIC_RC_ENOENT);
    }

    rs = fltr_allocator->free(cmd->filter_id);
    if (rs != indexer::SUCCESS) {
        NIC_LOG_ERR("Failed to free filter_id: {}, err: {}",
                      cmd->filter_id, rs);
        return (IONIC_RC_ERROR);
    }
    NIC_LOG_DEBUG("Freed filter_id: {}", cmd->filter_id);

    if (ret == sdk::SDK_RET_ENTRY_NOT_FOUND) {
        return (IONIC_RC_ENOENT);
    }
    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::RssConfig(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t rss_indir_base = 0;
    struct lif_setattr_cmd *cmd = (struct lif_setattr_cmd *)req;
    //struct lif_setattr_comp *comp = (struct lif_setattr_comp *)resp;

    if (state == LIF_STATE_CREATED || state == LIF_STATE_INITING) {
        NIC_LOG_ERR("{}: Lif is not initialized", hal_lif_info_.name);
        return (IONIC_RC_ERROR);
    }

    rss_type = cmd->rss.types;
    memcpy(rss_key, cmd->rss.key, IONIC_RSS_HASH_KEY_SIZE);

    if (spec->host_dev) {
        rss_indir_base = HOST_ADDR(hal_lif_info_.lif_id, cmd->rss.addr);
    } else {
        rss_indir_base = cmd->rss.addr;
    }

    // Get indirection table from host
    edmaq->Post(
        spec->host_dev ? EDMA_OPCODE_HOST_TO_LOCAL : EDMA_OPCODE_LOCAL_TO_LOCAL,
        rss_indir_base,
        edma_buf_base,
        RSS_IND_TBL_SIZE,
        NULL
    );

    READ_MEM(edma_buf_base, rss_indir, RSS_IND_TBL_SIZE, 0);

    NIC_LOG_DEBUG("{}: {}: type {:#x} key {} table {}",
        hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->opcode),
        rss_type, rss_key, rss_indir);

    // Validate indirection table entries
    for (int i = 0; i < RSS_IND_TBL_SIZE; i++) {
        if (rss_indir[i] > spec->rxq_count) {
            NIC_LOG_ERR("{}: Invalid indirection table entry index {} qid {}",
                hal_lif_info_.name, i, rss_indir[i]);
            return (IONIC_RC_ERROR);
        }
    }

#ifndef APOLLO
    int ret;
    ret = pd->eth_program_rss(hal_lif_info_.lif_id, rss_type, rss_key, rss_indir,
                              spec->rxq_count);
    if (ret != 0) {
        NIC_LOG_DEBUG("{}: Unable to program hw for RSS HASH", ret);
        return (IONIC_RC_ERROR);
    }
#endif

    return IONIC_RC_SUCCESS;
}

/*
 * RDMA Commands
 */
status_code_t
EthLif::_CmdRDMAResetLIF(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t addr;
    struct rdma_queue_cmd *cmd = (struct rdma_queue_cmd *) req;
    uint64_t               lif_id = hal_lif_info_.lif_id + cmd->lif_index;

    NIC_LOG_DEBUG("{}: {}: lif {} ", hal_lif_info_.name,
        opcode_to_str((cmd_opcode_t)cmd->opcode),
        lif_id);

    // Clear PC and state of all SQ
    for (uint64_t qid = 0; qid < spec->rdma_sq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(lif_id, ETH_HW_QTYPE_SQ, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for RDMA SQ qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, 1, 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(cqcb_t), P4PLUS_CACHE_INVALIDATE_BOTH);

        addr += sizeof(cqcb_t);
        MEM_SET(addr, 0, 1, 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(cqcb_t), P4PLUS_CACHE_INVALIDATE_BOTH);
    }

    for (uint64_t qid = 0; qid < spec->rdma_rq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(lif_id, ETH_HW_QTYPE_RQ, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for RDMA RQ qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, 1, 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(cqcb_t), P4PLUS_CACHE_INVALIDATE_BOTH);

        addr += sizeof(cqcb_t);
        MEM_SET(addr, 0, 1, 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(cqcb_t), P4PLUS_CACHE_INVALIDATE_BOTH);
    }

    for (uint64_t qid = 0; qid < spec->rdma_cq_count; qid++) {
        addr = pd->lm_->get_lif_qstate_addr(lif_id, ETH_HW_QTYPE_CQ, qid);
        if (addr < 0) {
            NIC_LOG_ERR("{}: Failed to get qstate address for RDMA CQ qid {}",
                hal_lif_info_.name, qid);
            return (IONIC_RC_ERROR);
        }
        MEM_SET(addr, 0, 1, 0);
        PAL_barrier();
        p4plus_invalidate_cache(addr, sizeof(cqcb_t), P4PLUS_CACHE_INVALIDATE_BOTH);
    }

    addr = pd->rdma_get_kt_base_addr(lif_id);
    MEM_SET(addr, 0, spec->key_count * sizeof(key_entry_t), 0);
    PAL_barrier();
    p4plus_invalidate_cache(addr, spec->key_count * sizeof(key_entry_t), P4PLUS_CACHE_INVALIDATE_BOTH);

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdRDMACreateEQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd  *cmd = (struct rdma_queue_cmd  *) req;
    eqcb_t      eqcb;
    int64_t     addr;
    uint64_t    lif_id = hal_lif_info_.lif_id + cmd->lif_index;

    NIC_LOG_DEBUG("{}: {}: lif {} "
                 "qid {} depth_log2 {} stride_log2 {} dma_addr {} "
                 "cid {}",
                  hal_lif_info_.name,
                  opcode_to_str((cmd_opcode_t)cmd->opcode),
                  lif_id, cmd->qid_ver,
                  1u << cmd->depth_log2, 1u << cmd->stride_log2,
                  cmd->dma_addr, cmd->cid);

    memset(&eqcb, 0, sizeof(eqcb_t));
    // EQ does not need scheduling, so set one less (meaning #rings as zero)
    eqcb.ring_header.total_rings = MAX_EQ_RINGS - 1;
    eqcb.eqe_base_addr = cmd->dma_addr | (1UL << 63) | (lif_id << 52);
    eqcb.log_wqe_size = cmd->stride_log2;
    eqcb.log_num_wqes = cmd->depth_log2;
    eqcb.int_enabled = 1;
    //eqcb.int_num = spec.int_num();
    eqcb.eq_id = cmd->cid;
    eqcb.color = 0;

    eqcb.int_assert_addr = intr_assert_addr(res->intr_base + cmd->cid);

    memrev((uint8_t*)&eqcb, sizeof(eqcb_t));

    addr = pd->lm_->get_lif_qstate_addr(lif_id, ETH_HW_QTYPE_EQ, cmd->qid_ver);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for EQ qid {}",
                    lif_id, cmd->qid_ver);
        return (IONIC_RC_ERROR);
    }
    WRITE_MEM(addr, (uint8_t *)&eqcb, sizeof(eqcb), 0);
    p4plus_invalidate_cache(addr, sizeof(eqcb_t), P4PLUS_CACHE_INVALIDATE_BOTH);

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdRDMACreateCQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd *cmd = (struct rdma_queue_cmd *) req;
    uint32_t               num_cq_wqes, cqwqe_size;
    cqcb_t                 cqcb;
    uint8_t                offset;
    int                    ret;
    int64_t                addr;
    uint64_t               lif_id = hal_lif_info_.lif_id + cmd->lif_index;

    NIC_LOG_DEBUG("{}: {} lif {} cq_num {} cq_wqe_size {} num_cq_wqes {} "
                  "eq_id {} hostmem_pg_size {} ",
                  hal_lif_info_.name,
                  opcode_to_str((cmd_opcode_t)cmd->opcode),
                  lif_id, cmd->qid_ver,
                  1u << cmd->stride_log2, 1u << cmd->depth_log2,
                  cmd->cid, 1ull << (cmd->stride_log2 + cmd->depth_log2));

    cqwqe_size = 1u << cmd->stride_log2;
    num_cq_wqes = 1u << cmd->depth_log2;

    NIC_LOG_DEBUG("cqwqe_size: {} num_cq_wqes: {}", cqwqe_size, num_cq_wqes);

    memset(&cqcb, 0, sizeof(cqcb_t));
    cqcb.ring_header.total_rings = MAX_CQ_RINGS;
    cqcb.ring_header.host_rings = MAX_CQ_HOST_RINGS;

    int32_t cq_pt_index = cmd->xxx_table_index;
    uint64_t  pt_table_base_addr = pd->rdma_get_pt_base_addr(lif_id);

    cqcb.pt_base_addr = pt_table_base_addr >> PT_BASE_ADDR_SHIFT;
    cqcb.log_cq_page_size = cmd->stride_log2 + cmd->depth_log2;
    cqcb.log_wqe_size = log2(cqwqe_size);
    cqcb.log_num_wqes = log2(num_cq_wqes);
    cqcb.cq_id = cmd->qid_ver;
    cqcb.eq_id = cmd->cid;
    cqcb.host_addr = 1;

    cqcb.pt_pa = cmd->dma_addr;
    if (cqcb.host_addr) {
        cqcb.pt_pa |= ((1UL << 63) | lif_id << 52);
    }

    cqcb.pt_pg_index = 0;
    cqcb.pt_next_pg_index = 0x1FF;

    int log_num_pages = cqcb.log_num_wqes + cqcb.log_wqe_size - cqcb.log_cq_page_size;
    NIC_LOG_DEBUG("{}: pt_pa: {:#x}: pt_next_pa: {:#x}: pt_pa_index: {}: pt_next_pa_index: {}: log_num_pages: {}",
        lif_id, cqcb.pt_pa, cqcb.pt_next_pa, cqcb.pt_pg_index, cqcb.pt_next_pg_index, log_num_pages);

    /* store  pt_pa & pt_next_pa in little endian. So need an extra memrev */
    memrev((uint8_t*)&cqcb.pt_pa, sizeof(uint64_t));

    ret = pd->get_pc_offset("rxdma_stage0.bin", "rdma_cq_rx_stage0", &offset, NULL);
    if (ret < 0) {
        NIC_LOG_ERR("Failed to get PC offset : {} for prog: {}, label: {}", ret, "rxdma_stage0.bin", "rdma_cq_rx_stage0");
        return (IONIC_RC_ERROR);
    }

    cqcb.ring_header.pc = offset;

    // write to hardware
    NIC_LOG_DEBUG("{}: Writting initial CQCB State, "
                  "CQCB->PT: {:#x} cqcb_size: {}",
                  lif_id, cqcb.pt_base_addr, sizeof(cqcb_t));
    // Convert data before writting to HBM
    memrev((uint8_t*)&cqcb, sizeof(cqcb_t));

    addr = pd->lm_->get_lif_qstate_addr(lif_id, ETH_HW_QTYPE_CQ, cmd->qid_ver);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for CQ qid {}",
                    lif_id, cmd->qid_ver);
        return IONIC_RC_ERROR;
    }
    WRITE_MEM(addr, (uint8_t *)&cqcb, sizeof(cqcb), 0);

    uint64_t pt_table_addr = pt_table_base_addr+cq_pt_index*sizeof(uint64_t);

    // There is only one page table entry for adminq CQ
    WRITE_MEM(pt_table_addr, (uint8_t *)&cmd->dma_addr, sizeof(uint64_t), 0);
    NIC_LOG_DEBUG("PT Entry Write: Lif {}: CQ PT Idx: {} PhyAddr: {:#x}",
                  lif_id, cq_pt_index, cmd->dma_addr);
    p4plus_invalidate_cache(pt_table_addr, sizeof(uint64_t), P4PLUS_CACHE_INVALIDATE_BOTH);

    return (IONIC_RC_SUCCESS);
}

status_code_t
EthLif::_CmdRDMACreateAdminQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd  *cmd = (struct rdma_queue_cmd  *) req;
    int                     ret;
    aqcb_t                  aqcb;
    uint8_t                 offset;
    int64_t                 addr;
    uint64_t                lif_id = hal_lif_info_.lif_id + cmd->lif_index;

    NIC_LOG_DEBUG("{}: {}: lif: {} aq_num: {} aq_log_wqe_size: {} "
                    "aq_log_num_wqes: {} "
                    "cq_num: {} phy_base_addr: {}",
                    hal_lif_info_.name,
                    opcode_to_str((cmd_opcode_t)cmd->opcode),
                    lif_id, cmd->qid_ver,
                    cmd->stride_log2, cmd->depth_log2, cmd->cid,
                    cmd->dma_addr);

    memset(&aqcb, 0, sizeof(aqcb_t));
    aqcb.aqcb0.ring_header.total_rings = MAX_AQ_RINGS;
    aqcb.aqcb0.ring_header.host_rings = MAX_AQ_HOST_RINGS;
    aqcb.aqcb0.ring_header.cosA = 1;
    aqcb.aqcb0.ring_header.cosB = 1;

    aqcb.aqcb0.log_wqe_size = cmd->stride_log2;
    aqcb.aqcb0.log_num_wqes = cmd->depth_log2;
    aqcb.aqcb0.aq_id = cmd->qid_ver;
    aqcb.aqcb0.phy_base_addr = cmd->dma_addr | (1UL << 63) | (lif_id << 52);
    aqcb.aqcb0.cq_id = cmd->cid;
    addr = pd->lm_->get_lif_qstate_addr(lif_id, ETH_HW_QTYPE_CQ, cmd->cid);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for CQ qid {}",
                    lif_id, cmd->cid);
        return IONIC_RC_ERROR;
    }
    aqcb.aqcb0.cqcb_addr = addr;

    aqcb.aqcb0.first_pass = 1;

    ret = pd->get_pc_offset("txdma_stage0.bin", "rdma_aq_tx_stage0", &offset, NULL);
    if (ret < 0) {
        NIC_LOG_ERR("Failed to get PC offset : {} for prog: {}, label: {}", ret, "txdma_stage0.bin", "rdma_aq_tx_stage0");
        return IONIC_RC_ERROR;
    }

    aqcb.aqcb0.ring_header.pc = offset;

    // write to hardware
    NIC_LOG_DEBUG("{}: Writting initial AQCB State, "
                    "AQCB->phy_addr: {:#x} "
                    "aqcb_size: {}",
                    lif_id, aqcb.aqcb0.phy_base_addr, sizeof(aqcb_t));

    // Convert data before writting to HBM
    memrev((uint8_t*)&aqcb.aqcb0, sizeof(aqcb0_t));
    memrev((uint8_t*)&aqcb.aqcb1, sizeof(aqcb1_t));

    addr = pd->lm_->get_lif_qstate_addr(lif_id, ETH_HW_QTYPE_ADMIN, cmd->qid_ver);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for AQ qid {}",
                    lif_id, cmd->qid_ver);
        return IONIC_RC_ERROR;
    }
    WRITE_MEM(addr, (uint8_t *)&aqcb, sizeof(aqcb), 0);

    return (IONIC_RC_SUCCESS);
}

/*
 * Event Handlers
 */

void
EthLif::HalEventHandler(bool status)
{
    hal_status = status;

    if (!status) {
        return;
    }
}

void
EthLif::LinkEventHandler(port_status_t *evd)
{
    if (spec->uplink_port_num != evd->id) {
        NIC_LOG_WARN("{}: Uplink port number not matching status_port: {}. up_port: {}",
                     hal_lif_info_.name, evd->id, spec->uplink_port_num);
        return;
    }

    if (!dev_api) {
        NIC_LOG_WARN("{}: devapi not initialized", hal_lif_info_.name);
        return;
    }

    // drop the event if the lif is not initialized
    if (state != LIF_STATE_INIT &&
        state != LIF_STATE_UP &&
        state != LIF_STATE_DOWN) {
        NIC_LOG_INFO("{}: {} + {} => {}",
            hal_lif_info_.name,
            lif_state_to_str(state),
            (evd->status == 1) ? "LINK_UP" : "LINK_DN",
            lif_state_to_str(state));
        return;
    }

    // Update local lif status
    lif_status->link_status = evd->status;
    lif_status->link_speed =  evd->speed;
    ++lif_status->eid;
    ++lif_status->link_flap_count;
    WRITE_MEM(lif_status_addr, (uint8_t *)lif_status, sizeof(struct lif_status), 0);

    // Update host lif status
    if (host_lif_status_addr != 0) {
        edmaq->Post(
            spec->host_dev ? EDMA_OPCODE_LOCAL_TO_HOST : EDMA_OPCODE_LOCAL_TO_LOCAL,
            lif_status_addr,
            host_lif_status_addr,
            sizeof(struct lif_status),
            NULL
        );
    }

    if (notify_enabled == 0) {
        return;
    }

    uint64_t addr, req_db_addr;

    // Send the link event notification
    struct link_change_event msg = {
        .eid = lif_status->eid,
        .ecode = EVENT_OPCODE_LINK_CHANGE,
        .link_status = evd->status,
        .link_speed = evd->speed,
    };

    addr = notify_ring_base + notify_ring_head * sizeof(union notifyq_comp);
    WRITE_MEM(addr, (uint8_t *)&msg, sizeof(union notifyq_comp), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                (hal_lif_info_.lif_id << 6) +
                (ETH_NOTIFYQ_QTYPE << 3);

    // NIC_LOG_DEBUG("{}: Sending notify event, eid {} notify_idx {} notify_desc_addr {:#x}",
    //     hal_lif_info_.lif_id, lif_status->eid, notify_ring_head, addr);
    notify_ring_head = (notify_ring_head + 1) % ETH_NOTIFYQ_RING_SIZE;
    PAL_barrier();
    WRITE_DB64(req_db_addr, (ETH_NOTIFYQ_QID << 24) | notify_ring_head);

    // FIXME: Wait for completion

    state = (evd->status == 1) ? LIF_STATE_UP : LIF_STATE_DOWN;
    NIC_LOG_INFO("{}: {} + {} => {}",
        hal_lif_info_.name,
        lif_state_to_str(state),
        (evd->status == 1) ? "LINK_UP" : "LINK_DN",
        lif_state_to_str(state));
}

void
EthLif::XcvrEventHandler(port_status_t *evd)
{
    if (spec->uplink_port_num != evd->id) {
        return;
    }

    if (state != LIF_STATE_INIT &&
        state != LIF_STATE_UP &&
        state != LIF_STATE_DOWN) {
        NIC_LOG_INFO("{}: {} => {}",
            hal_lif_info_.name,
            lif_state_to_str(state),
            lif_state_to_str(state));
        return;
    }

    lif_status->link_status = evd->status;
    lif_status->link_speed =  evd->speed;
    ++lif_status->eid;
    ++lif_status->link_flap_count;
    WRITE_MEM(lif_status_addr, (uint8_t *)lif_status, sizeof(struct lif_status), 0);

    // Update host lif status
    if (host_lif_status_addr != 0) {
        edmaq->Post(
            spec->host_dev ? EDMA_OPCODE_LOCAL_TO_HOST : EDMA_OPCODE_LOCAL_TO_LOCAL,
            lif_status_addr,
            host_lif_status_addr,
            sizeof(struct lif_status),
            NULL
        );
    }

    // Send the link event notification
    struct link_change_event msg = {
        .eid = lif_status->eid,
        .ecode = EVENT_OPCODE_LINK_CHANGE, //TODO: need to change event as XCVR_EVENT
        .link_status = evd->status,
        .link_speed = evd->speed,
    };

    uint64_t addr, req_db_addr;

    addr = notify_ring_base + notify_ring_head * sizeof(union notifyq_comp);
    WRITE_MEM(addr, (uint8_t *)&msg, sizeof(union notifyq_comp), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                (hal_lif_info_.lif_id << 6) +
                (ETH_NOTIFYQ_QTYPE << 3);

    // NIC_LOG_DEBUG("{}: Sending notify event, eid {} notify_idx {} notify_desc_addr {:#x}",
    //     hal_lif_info_.lif_id, lif_status->eid, notify_ring_head, addr);
    notify_ring_head = (notify_ring_head + 1) % ETH_NOTIFYQ_RING_SIZE;
    PAL_barrier();
    WRITE_DB64(req_db_addr, (ETH_NOTIFYQ_QID << 24) | notify_ring_head);

    // FIXME: Wait for completion

    state = (evd->status == 1) ? LIF_STATE_UP : LIF_STATE_DOWN;
    NIC_LOG_INFO("{}: {} + {} => {}",
        hal_lif_info_.name,
        lif_state_to_str(state),
        (evd->status == 1) ? "LINK_UP" : "LINK_DN",
        lif_state_to_str(state));
}

void
EthLif::SendFWDownEvent()
{
    if (state == LIF_STATE_RESET) {
        NIC_LOG_WARN("{}: state: {} Cannot send RESET event when lif is in RESET state!",
            hal_lif_info_.name, lif_state_to_str(state));
        return;
    }

    // Update local lif status
    lif_status->link_status = 0;
    lif_status->link_speed = 0;
    ++lif_status->eid;
    ++lif_status->link_flap_count;
    WRITE_MEM(lif_status_addr, (uint8_t *)lif_status, sizeof(struct lif_status), 0);

    // Update host lif status
    if (host_lif_status_addr != 0) {
        edmaq->Post(
            spec->host_dev ? EDMA_OPCODE_LOCAL_TO_HOST : EDMA_OPCODE_LOCAL_TO_LOCAL,
            lif_status_addr,
            host_lif_status_addr,
            sizeof(struct lif_status),
            NULL
        );
    }

    if (notify_enabled == 0) {
        return;
    }

    uint64_t addr, req_db_addr;

    // Send the link event notification
    struct reset_event msg = {
        .eid = lif_status->eid,
        .ecode = EVENT_OPCODE_RESET,
        .reset_code = 0, //FIXME: not sure what to program here
        .state = 1, //reset complete
    };

    addr = notify_ring_base + notify_ring_head * sizeof(union notifyq_comp);
    WRITE_MEM(addr, (uint8_t *)&msg, sizeof(union notifyq_comp), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                (hal_lif_info_.lif_id << 6) +
                (ETH_NOTIFYQ_QTYPE << 3);

    // NIC_LOG_DEBUG("{}: Sending notify event, eid {} notify_idx {} notify_desc_addr {:#x}",
    //     hal_lif_info_.lif_id, notify_block->eid, notify_ring_head, addr);
    notify_ring_head = (notify_ring_head + 1) % ETH_NOTIFYQ_RING_SIZE;
    PAL_barrier();
    WRITE_DB64(req_db_addr, (ETH_NOTIFYQ_QID << 24) | notify_ring_head);

    // FIXME: Wait for completion

    NIC_LOG_INFO("{}: {} + {} => {}",
        hal_lif_info_.name,
        lif_state_to_str(state),
        "RESET",
        lif_state_to_str(state));
}

void
EthLif::SetHalClient(devapi *dapi)
{
    dev_api = dapi;
}

/*
 * Callbacks
 */

void
EthLif::StatsUpdate(void *obj)
{
    EthLif *eth = (EthLif *)obj;

    struct edmaq_ctx ctx = {
        .cb = &EthLif::StatsUpdateComplete,
        .obj = obj
    };

    if (eth->lif_stats_addr != 0 && eth->host_lif_stats_addr != 0) {
        eth->edmaq->Post(
            eth->spec->host_dev ? EDMA_OPCODE_LOCAL_TO_HOST : EDMA_OPCODE_LOCAL_TO_LOCAL,
            eth->lif_stats_addr,
            eth->host_lif_stats_addr,
            sizeof(struct lif_stats),
            &ctx
        );
    }
}

void
EthLif::StatsUpdateComplete(void *obj)
{
    EthLif *eth = (EthLif *)obj;

    evutil_timer_again(&eth->stats_timer);
}

int
EthLif::GenerateQstateInfoJson(pt::ptree &lifs)
{
    pt::ptree lif;
    pt::ptree qstates;

    NIC_LOG_DEBUG("{}: Qstate Info to Json", hal_lif_info_.name);

    lif.put("lif_id", hal_lif_info_.lif_id);

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
