/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstring>

#include "base.hpp"
#include "eth_dev.hpp"
#include "p4pd_api.hpp"
#include "p4plus_pd_api.h"
#include "common_rxdma_actions_p4pd_table.h"
#include "logger.hpp"
#include "table_monitor.hpp"
#include "pd_client.hpp"
#include "hal_client.hpp"
#include "capri_tbl_rw.hpp"
#include "capri_hbm.hpp"
#include "logger.hpp"
#include "capri_common.h"
#include "hal_cfg.hpp"

#define ENTRY_TRACE_EN      true

static uint8_t *memrev(uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

void table_health_monitor(uint32_t table_id,
                          char *name,
                          table_health_state_t curr_state,
                          uint32_t capacity,
                          uint32_t usage,
                          table_health_state_t *new_state)
{
    // TODO: Update new_state depending on usage and
    //       raise an event on state change.
    NIC_LOG_DEBUG("table id: {}, name: {}, capacity: {}, "
                  "usage: {}, curr state: {}, new state: {}",
                  table_id, name, capacity, usage, curr_state, *new_state);
}

int
PdClient::p4plus_rxdma_init_tables()
{
    uint32_t                   tid;
    p4pd_table_properties_t    tinfo;
    p4pd_error_t               rc;
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "iris/capri_p4_rxdma_table_map.json",
            .p4pd_pgm_name       = "iris",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_,
    };

    memset(&tinfo, 0, sizeof(tinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_rxdma_init(&p4pd_cfg);
    assert(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_rxdma_dm_tables_ =
            (directmap **)calloc(sizeof(directmap *),
            (P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX -
             P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    assert(p4plus_rxdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_rxdma_table_properties_get(tid, &tinfo);
        assert(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
            case P4_TBL_TYPE_INDEX:
                p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                        directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                           false, ENTRY_TRACE_EN, table_health_monitor);
                assert(p4plus_rxdma_dm_tables_
                [tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
                break;

            case P4_TBL_TYPE_MPU:
            default:
                break;
        }
    }

    return 0;
}

PdClient* PdClient::factory(platform_mode_t platform)
{
    int ret;
    hal::hal_cfg_t      hal_cfg;
    PdClient            *pdc = new PdClient();

    NIC_LOG_DEBUG("{}: Entered\n", __FUNCTION__);
    assert(pdc);

    pdc->platform_ = platform;
    pdc->hal_cfg_path_ = std::getenv("HAL_CONFIG_PATH");

    if (!pdc->hal_cfg_path_) {
        pdc->hal_cfg_path_ = (char*)"./";
    }

    NIC_LOG_INFO("Loading p4plus RxDMA asic lib tables cfg_path: {}...", pdc->hal_cfg_path_);
    ret = pdc->p4plus_rxdma_init_tables();
    assert(ret == 0);

    NIC_LOG_INFO("Initializing HBM Memory Partitions ...");
    pdc->mp_ = sdk::platform::utils::mpartition::factory((string(pdc->hal_cfg_path_) +
                                                         "/iris/hbm_mem.json").c_str(),
                                                         CAPRI_HBM_BASE);
    assert(pdc->mp_);

    NIC_LOG_INFO("Initializing Program Info ...");
    pdc->pinfo_ = sdk::platform::program_info::factory((string(pdc->hal_cfg_path_) +
                                                       "/gen/mpu_prog_info.json").c_str());
    assert(pdc->pinfo_);

    switch (pdc->platform_){
        case PLATFORM_MODE_SIM:
            hal_cfg.platform_mode = hal::HAL_PLATFORM_MODE_SIM;
            break;
        case PLATFORM_MODE_HW:
            hal_cfg.platform_mode = hal::HAL_PLATFORM_MODE_HW;
            break;
        case PLATFORM_MODE_HAPS:
            hal_cfg.platform_mode = hal::HAL_PLATFORM_MODE_HAPS;
            break;
        case PLATFORM_MODE_RTL:
            hal_cfg.platform_mode = hal::HAL_PLATFORM_MODE_RTL;
            break;
        case PLATFORM_MODE_MOCK:
            hal_cfg.platform_mode = hal::HAL_PLATFORM_MODE_MOCK;
            break;
        default :
            hal_cfg.platform_mode = hal::HAL_PLATFORM_MODE_NONE;
            break;
    }

    hal_cfg.cfg_path = pdc->hal_cfg_path_;
    NIC_LOG_INFO("Initializing table rw ...");
    ret = capri_p4plus_table_rw_init(&hal_cfg);
    assert(ret == 0);

    NIC_LOG_INFO("Initializing NIC LIF Mgr ...");
    pdc->lm_ = NicLIFManager::factory(pdc->mp_, pdc->pinfo_);
    assert(pdc->lm_);

    NIC_LOG_DEBUG("{}: Exited\n", __FUNCTION__);

    return pdc;
}

void PdClient::destroy(PdClient *pdc)
{
    uint32_t    tid;
    if (pdc->p4plus_rxdma_dm_tables_) {
        for (tid = P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX; tid++) {
            if (pdc->p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN]) {
                directmap::destroy(pdc->p4plus_rxdma_dm_tables_[tid -
                                                           P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN]);
            }
        }

        free(pdc->p4plus_rxdma_dm_tables_);
    }

    lm_->destroy(lm_);
    mp_->destroy(mp_);
    delete pdc;
}

int
PdClient::lif_qstate_map_init(uint64_t hw_lif_id,
                              struct queue_info* queue_info,
                              uint8_t coses)
{
    LIFQStateParams qs_params = { 0 };
    int32_t         ret      = 0;
    uint8_t         hint_cos = 0;

    for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto & qinfo = queue_info[i];
        if (qinfo.size < 1) continue;

        if (qinfo.size > 7 || qinfo.entries > 24) {
            NIC_LOG_ERR("Invalid entry in LifSpec : size={} entries={}",
                          qinfo.size, qinfo.entries);
            return -1;
        }

        if (qinfo.purpose > intf::LifQPurpose_MAX) {
            NIC_LOG_ERR("Invalid entry in LifSpec : purpose={}", qinfo.purpose);
            return -1;
        }

        qs_params.type[qinfo.type_num].size    = qinfo.size;
        qs_params.type[qinfo.type_num].entries = qinfo.entries;

        // Set both cosA,cosB to admin_cos(cosA) value for admin-qtype.
        if (qinfo.purpose != LIF_QUEUE_PURPOSE_ADMIN) {
            qs_params.type[qinfo.type_num].cosA = (coses & 0x0f);
            qs_params.type[qinfo.type_num].cosB = (coses & 0xf0) >> 4;
        } else {
            qs_params.type[qinfo.type_num].cosA = qs_params.type[qinfo.type_num].cosB = (coses & 0x0f);
        }
    }

    // cosB (default cos) will be the hint_cos for the lif.
    hint_cos = (coses & 0xf0) >> 4;
    // make sure that when you are creating with hw_lif_id the lif is alloced
    // already, otherwise this call may return an error
    if ((ret = lm_->InitLIFQState(hw_lif_id, &qs_params, hint_cos)) < 0) {
        NIC_LOG_ERR("Failed to initialize LIFQState: err_code : {}", ret);
        return -1;
    }

    return 0;
}

int
PdClient::lif_qstate_init(uint64_t hw_lif_id, struct queue_info* queue_info)
{
    uint32_t total_queues = 0;
    std::unique_ptr<uint8_t[]> buf;

    for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto & qinfo = queue_info[i];
        if (qinfo.size < 1) continue;

        for (uint32_t qid = 0; qid < (uint32_t) pow(2, qinfo.entries); qid++) {
            uint8_t *state = (uint8_t *) qinfo.qstate;
            uint32_t state_sz = (uint32_t) pow(2, qinfo.size + 5);
            if (qinfo.label) {
                uint8_t off = 0;
                int ret = lm_->GetPCOffset("p4plus", qinfo.prog, qinfo.label, &off);
                if (ret < 0) {
                    NIC_LOG_ERR("Failed to get PC offset : {} for prog: {}, label: {}", ret, qinfo.prog, qinfo.label);
                    return -1;
                }
                if (state_sz != 0) {
                    buf.reset(new uint8_t[state_sz]);
                    bcopy(qinfo.qstate, buf.get(), state_sz);
                    buf.get()[0] = off;
                    state = buf.get();
                } else {
                    NIC_LOG_DEBUG("qstate size{}", state_sz);
                }
            }

            int ret = lm_->WriteQState(hw_lif_id, qinfo.type_num, qid, state, state_sz);
            if (ret < 0) {
                NIC_LOG_ERR("Failed to set LIFQState : {}", ret);
                return -1;
            }
        }

        total_queues++;
    }

    NIC_LOG_INFO("total number of queues: {}", total_queues);
    return 0;
}

int PdClient::program_qstate(struct queue_info* queue_info,
                             struct lif_info *lif_info,
                             uint8_t coses)
{
    int ret;
    NIC_LOG_INFO("{}: Entered for hw_lif_id: {}\n",
                 __FUNCTION__, lif_info->hw_lif_id);

    // init queue state map
    ret = lif_qstate_map_init(lif_info->hw_lif_id, queue_info, coses);

    if (ret != 0) {
        return -1;
    }
    // init queues
    ret = lif_qstate_init(lif_info->hw_lif_id, queue_info);
    if (ret != 0) {
        NIC_LOG_ERR("Failed to do lif qstate: ret: {}", ret);
        return -1;
    }

    for (uint32_t type = 0; type < NUM_QUEUE_TYPES; type++) {
        auto &qinfo = queue_info[type];
        if (qinfo.size < 1) continue;

        lif_info->qstate_addr[type] = lm_->GetLIFQStateAddr(lif_info->hw_lif_id, type, 0);
        NIC_LOG_INFO("hw_lif_id{}: lif_id:{}, qtype: {}, qstate_base: {:#x}",
                     lif_info->hw_lif_id, lif_info->lif_id,
                     type, lif_info->qstate_addr[type]);
    }

    NIC_LOG_DEBUG("{}: Leaving\n", __FUNCTION__);
    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_rss_params_table_entry_add(
        uint32_t hw_lif_id, uint8_t rss_type, uint8_t *rss_key)
{
    p4pd_error_t        pd_err;
    eth_rx_rss_params_actiondata data = { 0 };

    assert(hw_lif_id < MAX_LIFS);
    assert(rss_key != NULL);

    data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_type = rss_type;
    memcpy(&data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           rss_key,
           sizeof(data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));
    memrev((uint8_t *)&data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data.eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    pd_err = p4pd_global_entry_write(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                     hw_lif_id, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        assert(0);
    }

    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_rss_params_table_entry_get(
        uint32_t hw_lif_id, eth_rx_rss_params_actiondata *data)
{
    p4pd_error_t        pd_err;

    assert(hw_lif_id < MAX_LIFS);
    assert(data != NULL);

    pd_err = p4pd_global_entry_read(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                    hw_lif_id, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        assert(0);
    }

    memrev((uint8_t *)&data->eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data->eth_rx_rss_params_action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
        uint32_t hw_lif_id, uint8_t index, uint8_t enable, uint8_t qid)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;
    eth_rx_rss_indir_actiondata data = { 0 };

    if (hw_lif_id >= MAX_LIFS ||
        index >= ETH_RSS_LIF_INDIR_TBL_SZ ||
        qid >= ETH_RSS_MAX_QUEUES) {
        NIC_LOG_INFO("{}: {}, index : {}, qid : {}",
                     __FUNCTION__, hw_lif_id, index, qid);
        return -1;
    };

    data.eth_rx_rss_indir_action_u.eth_rx_rss_indir_eth_rx_rss_indir.enable = enable;
    data.eth_rx_rss_indir_action_u.eth_rx_rss_indir_eth_rx_rss_indir.qid = qid;

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = mp_->start_addr(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    NIC_LOG_DEBUG("{}: hw_lif_id : {}, index : {}, addr : {:x}, enable : {}, qid : {}",
                  __FUNCTION__, hw_lif_id, index, addr, enable, qid);
    capri_hbm_write_mem(addr,
                        (uint8_t *)&data.eth_rx_rss_indir_action_u,
                        sizeof(data.eth_rx_rss_indir_action_u));
    p4plus_invalidate_cache(addr, sizeof(data.eth_rx_rss_indir_action_u),
                            P4PLUS_CACHE_INVALIDATE_RXDMA);

    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_rss_indir_table_entry_get(
        uint32_t hw_lif_id, uint8_t index, eth_rx_rss_indir_actiondata *data)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;

    if (hw_lif_id >= MAX_LIFS ||
        index >= ETH_RSS_LIF_INDIR_TBL_SZ) {
        NIC_LOG_INFO("{}: hw_lif_id : {} index : {}",
                     __FUNCTION__, hw_lif_id, index);
        return -1;
    };

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = mp_->start_addr(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    capri_hbm_read_mem(addr,
                       (uint8_t *)&data->eth_rx_rss_indir_action_u,
                       sizeof(data->eth_rx_rss_indir_action_u));

    return 0;
}

int
PdClient::eth_program_rss(uint32_t hw_lif_id, uint16_t rss_type, uint8_t *rss_key, uint8_t *rss_indir,
                uint16_t num_queues)
{
    NIC_LOG_DEBUG("{}: Entered\n", __FUNCTION__);

    assert(hw_lif_id < MAX_LIFS);
    assert(num_queues < ETH_RSS_MAX_QUEUES);

    p4pd_common_p4plus_rxdma_rss_params_table_entry_add(hw_lif_id, rss_type, rss_key);

    if (num_queues > 0) {
        for (unsigned int index = 0; index < ETH_RSS_LIF_INDIR_TBL_LEN; index++) {
            NIC_LOG_DEBUG("{}: hw_lif_id {} index {} type {} qid {}\n",
                          __FUNCTION__, hw_lif_id, index, rss_type, rss_indir[index]);
            p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
                    hw_lif_id, index, rss_type, rss_indir[index]);
        }
    }

    NIC_LOG_DEBUG("{}: Leaving\n", __FUNCTION__);
    return 0;
}

sdk::platform::utils::mem_addr_t
PdClient::mem_start_addr (const char *region)
{
    return mp_->start_addr(region);
}
