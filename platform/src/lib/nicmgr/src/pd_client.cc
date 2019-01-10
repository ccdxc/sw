/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstring>

#include "nic/include/base.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/p4/common/defines.h"

#include "logger.hpp"
#include "pd_client.hpp"
#include "hal_client.hpp"
#include "eth_dev.hpp"
#include "rdma_dev.hpp"

using namespace sdk::platform::capri;
using namespace sdk::platform::utils;

#define ENTRY_TRACE_EN      true
#define QSTATE_INFO_FILE_NAME  "lif_qstate_info.json"

const static char *kLif2QstateHBMLabel = "nicmgrqstate_map";

const static char *kRdmaHBMLabel = "rdma";
const static uint32_t kRdmaAllocUnit = 4096;

const static char *kRdmaHBMBarLabel = "rdma-hbm-bar";
const static uint32_t kRdmaBarAllocUnit = 8 * 1024 * 1024;

const static char *kNicmgrHBMLabel = "nicmgr";
const static uint32_t kNicmgrAllocUnit = 64;

const static char *kDevcmdHBMLabel = "devcmd";
const static uint32_t kDevcmdAllocUnit = 4096;

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

static uint32_t
roundup_to_pow_2(uint32_t x)
{
    uint32_t power = 1;

    if (x == 1)
        return (power << 1);

    while(power < x)
        power*=2;
    return power;
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
            .cfg_path            = hal_cfg_path_.c_str(),
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
                if (tinfo.tabledepth) {
                    p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                        directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                           false, ENTRY_TRACE_EN, table_health_monitor);
                    assert(p4plus_rxdma_dm_tables_
                           [tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
                }
                break;

            case P4_TBL_TYPE_MPU:
            default:
                break;
        }
    }

    return 0;
}

int
PdClient::p4plus_txdma_init_tables()
{
    uint32_t                   tid;
    p4pd_table_properties_t    tinfo;
    p4pd_error_t               rc;
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "iris/capri_p4_txdma_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };

    memset(&tinfo, 0, sizeof(tinfo));

    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_txdma_init(&p4pd_cfg);
    assert(rc == P4PD_SUCCESS);

    // start instantiating tables based on the parsed information
    p4plus_txdma_dm_tables_ =
        (directmap **)calloc(sizeof(directmap *),
                             (P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                              P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    HAL_ASSERT(p4plus_txdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_txdma_table_properties_get(tid, &tinfo);
        HAL_ASSERT(rc == P4PD_SUCCESS);

        switch (tinfo.table_type) {
        case P4_TBL_TYPE_INDEX:
            if (tinfo.tabledepth) {
                p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] =
                    directmap::factory(tinfo.tablename, tid, tinfo.tabledepth, tinfo.actiondata_struct_size,
                                       false, ENTRY_TRACE_EN, table_health_monitor);
                assert(p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN] != NULL);
            }
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return 0;
}

void PdClient::rdma_manager_init (void)
{
    uint64_t hbm_addr = mp_->start_addr(kRdmaHBMLabel);
    assert(hbm_addr > 0);

    uint32_t size = mp_->size(kRdmaHBMLabel);
    assert(size != 0);

    uint32_t num_units = size / kRdmaAllocUnit;
    if (hbm_addr & 0xFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
        num_units--;
    }

    rdma_hbm_base_ = hbm_addr;
    rdma_hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));

    NIC_FUNC_DEBUG("rdma_hbm_base : {:#x}", rdma_hbm_base_);

    hbm_addr = mp_->start_addr(kRdmaHBMBarLabel);
    assert(hbm_addr > 0);

    size = mp_->size(kRdmaHBMBarLabel);
    assert(size != 0);

    num_units = size / kRdmaBarAllocUnit;
    if (hbm_addr & 0x7FFFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0x7FFFFF) & ~0x7FFFFFULL;
        num_units--;
    }

    rdma_hbm_bar_base_ = hbm_addr;
    rdma_hbm_bar_allocator_.reset(new sdk::lib::BMAllocator(num_units));

    NIC_FUNC_DEBUG("rdma_hbm_bar_base : {:#x}", rdma_hbm_bar_base_);

}

uint64_t PdClient::rdma_mem_alloc (uint32_t size)
{
    uint32_t alloc_units;

    alloc_units = (size + kRdmaAllocUnit - 1) & ~(kRdmaAllocUnit-1);
    alloc_units /= kRdmaAllocUnit;
    int alloc_offset = rdma_hbm_allocator_->Alloc(alloc_units);

    if (alloc_offset < 0) {
        NIC_FUNC_ERR("Invalid alloc_offset {}", alloc_offset);
        return 0;
    }

    rdma_allocation_sizes_[alloc_offset] = alloc_units;
    alloc_offset *= kRdmaAllocUnit;
    NIC_FUNC_DEBUG("size: {} alloc_offset: {} hbm_addr: {}",
                    size, alloc_offset, rdma_hbm_base_ + alloc_offset);
    return rdma_hbm_base_ + alloc_offset;
}

uint64_t PdClient::rdma_mem_bar_alloc (uint32_t size)
{
    uint32_t alloc_units;

    alloc_units = (size + kRdmaBarAllocUnit - 1) & ~(kRdmaBarAllocUnit-1);
    alloc_units /= kRdmaBarAllocUnit;
    int alloc_offset = rdma_hbm_bar_allocator_->Alloc(alloc_units);

    if (alloc_offset < 0) {
        NIC_FUNC_ERR("Invalid alloc_offset {}", alloc_offset);
        return 0;
    }

    rdma_bar_allocation_sizes_[alloc_offset] = alloc_units;
    alloc_offset *= kRdmaBarAllocUnit;
    NIC_FUNC_DEBUG("size: {} alloc_offset: {} hbm_addr: {:#x}",
                    size, alloc_offset, rdma_hbm_bar_base_ + alloc_offset);
    return rdma_hbm_bar_base_ + alloc_offset;
}

void PdClient::nicmgr_mem_init (void)
{
    uint64_t hbm_addr = mp_->start_addr(kNicmgrHBMLabel);
    assert(hbm_addr > 0);

    uint32_t size = mp_->size(kNicmgrHBMLabel);
    assert(size != 0);

    uint32_t num_units = size / kNicmgrAllocUnit;
    if (hbm_addr & 0xFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
        num_units--;
    }

    nicmgr_hbm_base_ = hbm_addr;
    nicmgr_hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));

    NIC_FUNC_DEBUG("nicmgr_hbm_base : {:#x}", nicmgr_hbm_base_);
}

uint64_t PdClient::nicmgr_mem_alloc(uint32_t size)
{
    uint64_t addr = 0;
    uint32_t alloc_units;

    alloc_units = (size + kNicmgrAllocUnit - 1) & ~(kNicmgrAllocUnit-1);
    alloc_units /= kNicmgrAllocUnit;
    int alloc_offset = nicmgr_hbm_allocator_->Alloc(alloc_units);

    if (alloc_offset < 0) {
        NIC_FUNC_ERR("Invalid alloc_offset {}", alloc_offset);
        return 0;
    }

    nicmgr_allocation_sizes_[alloc_offset] = alloc_units;
    alloc_offset *= kNicmgrAllocUnit;
    addr = nicmgr_hbm_base_ + alloc_offset;
    NIC_FUNC_DEBUG("size: {} alloc_offset: {} addr: {:#x}",
                    size, alloc_offset, addr);

    // Allocations must be cache-line aligned because this memory is used
    // for rings.
    assert((addr & 0x3F) == 0);

    return addr;
}

void PdClient::devcmd_mem_init (void)
{
    uint64_t hbm_addr = mp_->start_addr(kDevcmdHBMLabel);
    assert(hbm_addr > 0);

    uint32_t size = mp_->size(kDevcmdHBMLabel);
    assert(size != 0);

    uint32_t num_units = size / kDevcmdAllocUnit;
    if (hbm_addr & 0xFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
        num_units--;
    }

    devcmd_hbm_base_ = hbm_addr;
    devcmd_hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));

    NIC_FUNC_DEBUG("devcmd_hbm_base : {:#x}", devcmd_hbm_base_);
}

uint64_t PdClient::devcmd_mem_alloc(uint32_t size)
{
    uint64_t addr = 0;
    uint32_t alloc_units;

    alloc_units = (size + kDevcmdAllocUnit - 1) & ~(kDevcmdAllocUnit - 1);
    alloc_units /= kDevcmdAllocUnit;
    int alloc_offset = devcmd_hbm_allocator_->Alloc(alloc_units);

    if (alloc_offset < 0) {
        NIC_FUNC_ERR("Invalid alloc_offset {}", alloc_offset);
        return 0;
    }

    devcmd_allocation_sizes_[alloc_offset] = alloc_units;
    alloc_offset *= kDevcmdAllocUnit;
    addr = devcmd_hbm_base_ + alloc_offset;
    NIC_FUNC_DEBUG("size: {} alloc_offset: {} addr: {:#x}",
                    size, alloc_offset, addr);

    // devcmd allocations should be page aligned
    assert((addr & 0xFFF) == 0);

    return addr;
}

int32_t PdClient::intr_alloc(uint32_t count)
{
    uint32_t intr_base;

    auto ret = intr_allocator->alloc_block(&intr_base, count);
    if (ret != sdk::lib::indexer::SUCCESS) {
        NIC_FUNC_ERR("Failed to allocate interrupts");
        return -ENOMEM;
    }

    NIC_FUNC_DEBUG("base {} count {}", intr_base, count);

    return intr_base;
}

int
PdClient::create_dirs() {
    struct stat  st = { 0 };

    if (hal_cfg_path_ == "") {
        // use the current dir
        hal_cfg_path_ = "./";
        gen_dir_path_ = "./";
    } else {
        gen_dir_path_ = string(hal_cfg_path_ + "gen/");
        // check if the gen dir exists
        if (stat(gen_dir_path_.c_str(), &st) == -1) {
            // doesn't exist, try to create
            if (mkdir(gen_dir_path_.c_str(), 0755) < 0) {
                NIC_LOG_ERR("Gen directory {}/ doesn't exist, failed to create one",
                            gen_dir_path_.c_str());
                return -1;
            }
        } else {
            // gen dir exists, check if we have write permissions
            if (access(gen_dir_path_.c_str(), W_OK) < 0) {
                // don't have permissions to create this directory
                NIC_LOG_ERR("No permissions to create files in {}", gen_dir_path_.c_str());
                return -1;
            }
        }
    }

    return 0;
}

void PdClient::init(void)
{
    int ret;
    hal::hal_cfg_t hal_cfg;

    NIC_LOG_DEBUG("Loading p4plus RxDMA asic lib tables cfg_path: {}...", hal_cfg_path_);
    ret = p4plus_rxdma_init_tables();
    assert(ret == 0);
    NIC_LOG_DEBUG("Loading p4plus TxDMA asic lib tables cfg_path: {}...", hal_cfg_path_);
    ret = p4plus_txdma_init_tables();
    assert(ret == 0);
    NIC_LOG_DEBUG("Initializing HBM Memory Partitions from: {}...", hal_cfg_path_);
    mp_ = mpartition::factory();
    assert(mp_);
    NIC_LOG_DEBUG("Initializing LIF Manager ...");
    lm_ = LIFManager::factory(mp_, NULL, kLif2QstateHBMLabel);
    assert(lm_);

    switch (platform_){
    case PLATFORM_SIM:
        hal_cfg.platform = platform_type_t::PLATFORM_TYPE_SIM;
        break;
    case PLATFORM_HW:
        hal_cfg.platform = platform_type_t::PLATFORM_TYPE_HW;
        break;
    case PLATFORM_HAPS:
        hal_cfg.platform = platform_type_t::PLATFORM_TYPE_HAPS;
        break;
    case PLATFORM_RTL:
        hal_cfg.platform = platform_type_t::PLATFORM_TYPE_RTL;
        break;
    case PLATFORM_MOCK:
        hal_cfg.platform = platform_type_t::PLATFORM_TYPE_MOCK;
        break;
    default :
        hal_cfg.platform = platform_type_t::PLATFORM_TYPE_NONE;
        break;
    }

    hal_cfg.cfg_path = hal_cfg_path_;
    NIC_LOG_DEBUG("Initializing table rw ...");
    ret = capri_p4plus_table_rw_init();
    assert(ret == 0);

    rdma_manager_init();
    nicmgr_mem_init();
    devcmd_mem_init();
    intr_allocator = sdk::lib::indexer::factory(4096);
    assert(intr_allocator != NULL);
}

// called after HAL is UP and running
void PdClient::update(void)
{
    set_program_info();
}

PdClient* PdClient::factory(platform_t platform)
{
    int ret;
    PdClient *pdc = new PdClient();

    assert(pdc);
    pdc->platform_ = platform;
    pdc->hal_cfg_path_ = string(std::getenv("HAL_CONFIG_PATH")) + "/";
    ret = pdc->create_dirs();
    assert(ret == 0);
    pdc->init();

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

    if (pdc->p4plus_txdma_dm_tables_) {
        for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN;
             tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX; tid++) {
            if (pdc->p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN]) {
                directmap::destroy(pdc->p4plus_txdma_dm_tables_[tid -
                                                           P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN]);
            }
        }
        free(pdc->p4plus_txdma_dm_tables_);
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
    uint8_t bzero64[64] = {0};

    NIC_FUNC_INFO("lif{}: Started initializing Qstate", hw_lif_id);

    for (uint32_t qtype = 0; qtype < NUM_QUEUE_TYPES; qtype++) {
        auto & qinfo = queue_info[qtype];
        if (qinfo.size < 1) continue;

        for (uint32_t qid = 0; qid < (uint32_t) pow(2, qinfo.entries); qid++) {
            int ret = lm_->WriteQState(hw_lif_id, qtype, qid, bzero64,
                sizeof(bzero64));
            if (ret < 0) {
                NIC_LOG_ERR("Failed to set LIFQState : {}", ret);
                return -1;
            }
        }
    }

    NIC_FUNC_INFO("lif{}: Finished initializing Qstate", hw_lif_id);

    return 0;
}

int PdClient::program_qstate(struct queue_info* queue_info,
                             hal_lif_info_t *lif_info,
                             uint8_t coses)
{
    int ret;

    NIC_LOG_DEBUG("lif-{}: Programming qstate", lif_info->hw_lif_id);

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
        NIC_LOG_DEBUG("lif-{}: qtype: {}, qstate_base: {:#x}",
                     lif_info->hw_lif_id,
                     type, lif_info->qstate_addr[type]);
    }

    DeviceManager::GetInstance()->GenerateQstateInfoJson(gen_dir_path_ + QSTATE_INFO_FILE_NAME);
    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_rss_params_table_entry_add(
        uint32_t hw_lif_id, uint8_t rss_type, uint8_t *rss_key)
{
    p4pd_error_t        pd_err;
    eth_rx_rss_params_actiondata_t data = { 0 };

    assert(hw_lif_id < MAX_LIFS);
    assert(rss_key != NULL);

    data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_type = rss_type;
    memcpy(&data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           rss_key,
           sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));
    memrev((uint8_t *)&data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data.action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    pd_err = p4pd_global_entry_write(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                     hw_lif_id, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        assert(0);
    }

    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_rss_params_table_entry_get(
        uint32_t hw_lif_id, eth_rx_rss_params_actiondata_t *data)
{
    p4pd_error_t        pd_err;

    assert(hw_lif_id < MAX_LIFS);
    assert(data != NULL);

    pd_err = p4pd_global_entry_read(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS,
                                    hw_lif_id, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        assert(0);
    }

    memrev((uint8_t *)&data->action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key,
           sizeof(data->action_u.eth_rx_rss_params_eth_rx_rss_params.rss_key));

    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
        uint32_t hw_lif_id, uint8_t index, uint8_t enable, uint8_t qid)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;
    eth_rx_rss_indir_actiondata_t data = { 0 };

    if (hw_lif_id >= MAX_LIFS ||
        index >= ETH_RSS_LIF_INDIR_TBL_SZ ||
        qid >= ETH_RSS_MAX_QUEUES) {
        NIC_FUNC_ERR("{}, index : {}, qid : {}", hw_lif_id, index, qid);
        return -1;
    };

    data.action_u.eth_rx_rss_indir_eth_rx_rss_indir.enable = enable;
    data.action_u.eth_rx_rss_indir_eth_rx_rss_indir.qid = qid;

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = mp_->start_addr(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    NIC_FUNC_DEBUG("hw_lif_id : {}, index : {}, addr : {:x}, enable : {}, qid : {}",
                    hw_lif_id, index, addr, enable, qid);
    sdk::asic::asic_mem_write(addr,
                        (uint8_t *)&data.action_u,
                        sizeof(data.action_u));
    p4plus_invalidate_cache(addr, sizeof(data.action_u),
                            P4PLUS_CACHE_INVALIDATE_RXDMA);

    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_rss_indir_table_entry_get(
        uint32_t hw_lif_id, uint8_t index, eth_rx_rss_indir_actiondata_t *data)
{
    uint64_t tbl_base;
    uint64_t tbl_index;
    uint64_t addr;

    if (hw_lif_id >= MAX_LIFS ||
        index >= ETH_RSS_LIF_INDIR_TBL_SZ) {
        NIC_FUNC_ERR("hw_lif_id : {} index : {}", hw_lif_id, index);
        return -1;
    };

    tbl_index = (hw_lif_id * ETH_RSS_LIF_INDIR_TBL_SZ) +
                (index * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ);
    tbl_base = mp_->start_addr(CAPRI_HBM_REG_RSS_INDIR_TABLE);
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);
    addr = tbl_base + tbl_index;

    sdk::asic::asic_mem_read(addr,
                       (uint8_t *)&data->action_u,
                       sizeof(data->action_u));

    return 0;
}

int
PdClient::eth_program_rss(uint32_t hw_lif_id, uint16_t rss_type, uint8_t *rss_key, uint8_t *rss_indir,
                uint16_t num_queues)
{
    assert(hw_lif_id < MAX_LIFS);
    assert(num_queues < ETH_RSS_MAX_QUEUES);

    p4pd_common_p4plus_rxdma_rss_params_table_entry_add(hw_lif_id, rss_type, rss_key);

    if (num_queues > 0) {
        for (unsigned int index = 0; index < ETH_RSS_LIF_INDIR_TBL_LEN; index++) {
            NIC_FUNC_DEBUG("hw_lif_id {} index {} type {} qid {}",
                            hw_lif_id, index, rss_type, rss_indir[index]);
            p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
                    hw_lif_id, index, rss_type, rss_indir[index]);
        }
    }

    return 0;
}

int
PdClient::p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add (
    uint32_t idx,
    uint8_t rdma_en_qtype_mask,
    uint32_t pt_base_addr_page_id,
    uint8_t log_num_pt_entries,
    uint32_t cqcb_base_addr_hi,
    uint32_t sqcb_base_addr_hi,
    uint32_t rqcb_base_addr_hi,
    uint8_t log_num_cq_entries,
    uint32_t prefetch_pool_base_addr_page_id,
    uint8_t log_num_prefetch_pool_entries,
    uint8_t sq_qtype,
    uint8_t rq_qtype,
    uint8_t aq_qtype)
{
    p4pd_error_t        pd_err;
    //directmap                    *dm;
    rx_stage0_load_rdma_params_actiondata_t data = { 0 };

    assert(idx < MAX_LIFS);

    data.action_id = RX_STAGE0_LOAD_RDMA_PARAMS_RX_STAGE0_LOAD_RDMA_PARAMS_ID;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.cqcb_base_addr_hi = cqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.sqcb_base_addr_hi = sqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rqcb_base_addr_hi = rqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.prefetch_pool_base_addr_page_id = prefetch_pool_base_addr_page_id;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_prefetch_pool_entries = log_num_prefetch_pool_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.sq_qtype = sq_qtype;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rq_qtype = rq_qtype;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.aq_qtype = aq_qtype;

    /* TODO: Do we need memrev */
    pd_err = p4pd_global_entry_write(P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_LOAD_RDMA_PARAMS,
                                     idx, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        NIC_LOG_ERR("stage0 rdma LIF table write failure for rxdma, "
                    "idx : {}, err : {}",
                    idx, pd_err);
        assert(0);
    }
    NIC_LOG_DEBUG("stage0 rdma LIF table entry add successful for rxdma, "
                 "idx : {}, err : {}",
                 idx, pd_err);
    return 0;
}


int
PdClient::p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get (uint32_t idx, rx_stage0_load_rdma_params_actiondata_t *data)
{
    p4pd_error_t        pd_err;

    assert(idx < MAX_LIFS);
    assert(data != NULL);

    pd_err = p4pd_global_entry_read(
        P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_LOAD_RDMA_PARAMS,
        idx, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        NIC_LOG_ERR("stage0 rdma LIF table entry get failure for "
                    "rxdma, idx : {}, err : {}",
                    idx, pd_err);
        assert(0);
    }

    NIC_LOG_DEBUG("stage0 rdma LIF table entry get successful for "
                 "rxdma, idx : {}, err : {}",
                 idx, pd_err);
    return 0;
}

int
PdClient::p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add (
    uint32_t idx,
    uint8_t rdma_en_qtype_mask,
    uint32_t pt_base_addr_page_id,
    uint32_t ah_base_addr_page_id,
    uint8_t log_num_pt_entries,
    uint32_t cqcb_base_addr_hi,
    uint32_t sqcb_base_addr_hi,
    uint32_t rqcb_base_addr_hi,
    uint8_t log_num_cq_entries,
    uint32_t prefetch_pool_base_addr_page_id,
    uint8_t log_num_prefetch_pool_entries,
    uint8_t sq_qtype,
    uint8_t rq_qtype,
    uint8_t aq_qtype,
    uint64_t barmap_base_addr,
    uint32_t barmap_size)
{
    p4pd_error_t                  pd_err;
    tx_stage0_lif_params_table_actiondata_t data = { 0 };

    assert(idx < MAX_LIFS);

    data.action_id = TX_STAGE0_LIF_PARAMS_TABLE_TX_STAGE0_LIF_RDMA_PARAMS_ID;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.ah_base_addr_page_id = ah_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.cqcb_base_addr_hi = cqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.sqcb_base_addr_hi = sqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rqcb_base_addr_hi = rqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.prefetch_pool_base_addr_page_id = prefetch_pool_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_prefetch_pool_entries = log_num_prefetch_pool_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.sq_qtype = sq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rq_qtype = rq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.aq_qtype = aq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.barmap_base_addr = barmap_base_addr;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.barmap_size = barmap_size;

    /* TODO: Do we need memrev */
    pd_err = p4pd_global_entry_write(
        P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_LIF_PARAMS_TABLE,
        idx, NULL, NULL, &data);
    if (pd_err != P4PD_SUCCESS) {
        NIC_LOG_ERR("stage0 rdma LIF table write failure for txdma, "
                    "idx : {}, err : {}", idx, pd_err);
        assert(0);
    }
    NIC_LOG_DEBUG("stage0 rdma LIF table entry add successful for "
                 "txdma, idx : {}, err : {}", idx, pd_err);
    return HAL_RET_OK;
}


int
PdClient::p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_get (
    uint32_t idx,
    tx_stage0_lif_params_table_actiondata_t *data)
{
    p4pd_error_t                  pd_err;

    assert(idx < MAX_LIFS);
    assert(data != NULL);

    pd_err = p4pd_global_entry_read(
        P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_LIF_PARAMS_TABLE,
        idx, NULL, NULL, data);
    if (pd_err != P4PD_SUCCESS) {
        NIC_LOG_ERR("stage0 rdma LIF table entry get failure for "
                    "txdma, idx : {}, err : {}", idx, pd_err);
        assert(0);
    }
    NIC_LOG_DEBUG("stage0 rdma LIF table entry get successful for "
                 "txdma, idx : {}, err : {}", idx, pd_err);
    return 0;
}

uint64_t
PdClient::rdma_get_pt_base_addr (uint32_t lif)
{
    uint64_t            pt_table_base_addr;
    int                 rc;
    rx_stage0_load_rdma_params_actiondata_t data = {0};

    rc = p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get(lif, &data);
    if (rc) {
        NIC_LOG_ERR("stage0 rdma LIF table entry get failure for "
                    "rxdma, idx : {}, err : {}",
                    lif, rc);
        return rc;
    }

    pt_table_base_addr = data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.pt_base_addr_page_id;

    NIC_FUNC_DEBUG("lif-{}: Rx LIF params - pt_base_addr_page_id {}",
                    lif, pt_table_base_addr);

    pt_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;
    return(pt_table_base_addr);
}

uint64_t
PdClient::rdma_get_kt_base_addr (uint32_t lif)
{
    uint64_t            pt_table_base_addr;
    uint64_t            key_table_base_addr;
    uint32_t            log_num_pt_entries;
    int                 rc;
    rx_stage0_load_rdma_params_actiondata_t data = {0};

    rc = p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get(lif, &data);
    if (rc) {
        NIC_LOG_ERR("stage0 rdma LIF table entry get failure for "
                    "rxdma, idx : {}, err : {}",
                    lif, rc);
        return rc;
    }

    pt_table_base_addr = data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.pt_base_addr_page_id;
    log_num_pt_entries = data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_pt_entries;

    key_table_base_addr = (pt_table_base_addr << HBM_PAGE_SIZE_SHIFT) +
        (sizeof(uint64_t) << log_num_pt_entries);

    NIC_FUNC_DEBUG("lif-{}: Rx LIF params - pt_base_addr_page_id {} "
                    "log_num_pt_entries {} key_table_base_addr {}",
                    lif,
                    pt_table_base_addr, log_num_pt_entries,
                    key_table_base_addr);

    return key_table_base_addr;
}

uint64_t
PdClient::rdma_get_ah_base_addr (uint32_t lif)
{
    uint64_t            ah_table_base_addr;
    int                 rc;
    tx_stage0_lif_params_table_actiondata_t data = { 0 };

    rc = p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_get(lif, &data);
    if (rc) {
        NIC_LOG_ERR("stage0 rdma LIF table entry get failure for "
                    "txdma, idx : {}, err : {}",
                    lif, rc);
        return rc;
    }

    ah_table_base_addr = data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.ah_base_addr_page_id;

    NIC_FUNC_DEBUG("lif-{}: Rx LIF params - ah_base_addr_page_id {}",
                    lif,
                    ah_table_base_addr);

    ah_table_base_addr <<= HBM_PAGE_SIZE_SHIFT;
    return(ah_table_base_addr);
}

int
PdClient::rdma_lif_init (uint32_t lif, uint32_t max_keys,
                         uint32_t max_ahs, uint32_t max_ptes,
                         uint64_t mem_bar_addr, uint32_t mem_bar_size)
{
    sram_lif_entry_t    sram_lif_entry;
    uint32_t            pt_size, key_table_size, ah_table_size;
    uint32_t            total_size;
    uint64_t            base_addr;
    uint64_t            size;
    uint32_t            max_cqs;
    uint64_t            cq_base_addr; //address in HBM memory
    uint64_t            sq_base_addr; //address in HBM memory
    uint64_t            rq_base_addr; //address in HBM memory
    uint64_t            pad_size;
    int                 rc;

    NIC_FUNC_DEBUG("lif-{}: RDMA lif init", lif);

    LIFQState *qstate = lm_->GetLIFQState(lif);
    if (qstate == nullptr) {
        NIC_FUNC_ERR("lif-{}: GetLIFQState failed", lif);
        return HAL_RET_ERR;
    }

    max_cqs  = qstate->type[Q_TYPE_RDMA_CQ].num_queues;

    memset(&sram_lif_entry, 0, sizeof(sram_lif_entry_t));

    // Fill the CQ info in sram_lif_entry
    cq_base_addr = lm_->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_CQ);
    NIC_FUNC_DEBUG("lif-{}: cq_base_addr: {:#x}, max_cqs: {} ",
                  lif, cq_base_addr, roundup_to_pow_2(max_cqs));
    HAL_ASSERT((cq_base_addr & ((1 << SQCB_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.cqcb_base_addr_hi = cq_base_addr >> CQCB_ADDR_HI_SHIFT;
    sram_lif_entry.log_num_cq_entries = log2(roundup_to_pow_2(max_cqs));

    // Fill the SQ info in sram_lif_entry
    sq_base_addr = lm_->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_SQ);
    NIC_FUNC_DEBUG("lif-{}: sq_base_addr: {:#x}",
                    lif, sq_base_addr);
    HAL_ASSERT((sq_base_addr & ((1 << SQCB_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.sqcb_base_addr_hi = sq_base_addr >> SQCB_ADDR_HI_SHIFT;

    // Fill the RQ info in sram_lif_entry
    rq_base_addr = lm_->GetLIFQStateBaseAddr(lif, Q_TYPE_RDMA_RQ);
    NIC_FUNC_DEBUG("lif-{}: rq_base_addr: {:#x}",
                    lif, rq_base_addr);
    HAL_ASSERT((rq_base_addr & ((1 << RQCB_SIZE_SHIFT) - 1)) == 0);
    sram_lif_entry.rqcb_base_addr_hi = rq_base_addr >> RQCB_ADDR_HI_SHIFT;

    // Setup page table and key table entries
    max_ptes = roundup_to_pow_2(max_ptes);

    pt_size = sizeof(uint64_t) * max_ptes;
    //adjust to page boundary
    if (pt_size & (HBM_PAGE_SIZE - 1)) {
        pt_size = ((pt_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
    }

    max_keys = roundup_to_pow_2(max_keys);

    key_table_size = sizeof(key_entry_t) * max_keys;
    //adjust to page boundary
    if (key_table_size & (HBM_PAGE_SIZE - 1)) {
        key_table_size = ((key_table_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
    }

    max_ahs = roundup_to_pow_2(max_ahs);

    // TODO: Resize ah table after dcqcn related structures are moved to separate table
    pad_size = sizeof(ah_entry_t) + sizeof(dcqcn_cb_t);
    if (pad_size & ((1 << HDR_TEMP_ADDR_SHIFT) - 1)) {
        pad_size = ((pad_size >> HDR_TEMP_ADDR_SHIFT) + 1) << HDR_TEMP_ADDR_SHIFT;
    }

    ah_table_size = pad_size * max_ahs;
    //adjust to page boundary
    if (ah_table_size & (HBM_PAGE_SIZE - 1)) {
        ah_table_size = ((ah_table_size >> HBM_PAGE_SIZE_SHIFT) + 1) << HBM_PAGE_SIZE_SHIFT;
    }

    total_size = pt_size + key_table_size + ah_table_size + HBM_PAGE_SIZE;

    base_addr = rdma_mem_alloc(total_size);

    NIC_FUNC_DEBUG("lif-{}: pt_size: {}, key_table_size: {}, "
                  "ah_table_size: {}, base_addr: {:#x}",
                  lif,
                  pt_size, key_table_size,
                  ah_table_size, base_addr);

    size = base_addr;
    sram_lif_entry.pt_base_addr_page_id = size >> HBM_PAGE_SIZE_SHIFT;
    size += pt_size + key_table_size;
    sram_lif_entry.ah_base_addr_page_id = size >> HBM_PAGE_SIZE_SHIFT;
    sram_lif_entry.log_num_pt_entries = log2(max_ptes);
    size += ah_table_size;

    // TODO: Fill prefetch data and add corresponding code

    sram_lif_entry.rdma_en_qtype_mask =
        ((1 << Q_TYPE_RDMA_SQ) | (1 << Q_TYPE_RDMA_RQ) | (1 << Q_TYPE_RDMA_CQ) | (1 << Q_TYPE_RDMA_EQ) | (1 << Q_TYPE_ADMINQ));
    sram_lif_entry.sq_qtype = Q_TYPE_RDMA_SQ;
    sram_lif_entry.rq_qtype = Q_TYPE_RDMA_RQ;
    sram_lif_entry.aq_qtype = Q_TYPE_ADMINQ;

    NIC_FUNC_DEBUG("lif-{}: pt_base_addr_page_id: {}, log_num_pt: {}, "
                  "ah_base_addr_page_id: {}, rdma_en_qtype_mask: {} "
                  "sq_qtype: {} rq_qtype: {} aq_qtype: {}",
                    lif,
                    sram_lif_entry.pt_base_addr_page_id,
                    sram_lif_entry.log_num_pt_entries,
                    sram_lif_entry.ah_base_addr_page_id,
                    sram_lif_entry.rdma_en_qtype_mask,
                    sram_lif_entry.sq_qtype,
                    sram_lif_entry.rq_qtype,
                    sram_lif_entry.aq_qtype);

    //Controller Memory Buffer
    //meant for SQ/RQ in HBM for good performance
    //Allocated in units of 8MB
    if (mem_bar_size != 0) {
        sram_lif_entry.barmap_base_addr = mem_bar_addr >> MEM_BARMAP_SIZE_SHIFT;
        sram_lif_entry.barmap_size = mem_bar_size >> MEM_BARMAP_SIZE_SHIFT;
    } else {
        sram_lif_entry.barmap_base_addr = 0;
        sram_lif_entry.barmap_size = 0;
    }

    rc = p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add(
                            lif,
                            sram_lif_entry.rdma_en_qtype_mask,
                            sram_lif_entry.pt_base_addr_page_id,
                            sram_lif_entry.log_num_pt_entries,
                            sram_lif_entry.cqcb_base_addr_hi,
                            sram_lif_entry.sqcb_base_addr_hi,
                            sram_lif_entry.rqcb_base_addr_hi,
                            sram_lif_entry.log_num_cq_entries,
                            sram_lif_entry.prefetch_pool_base_addr_page_id,
                            sram_lif_entry.log_num_prefetch_pool_entries,
                            sram_lif_entry.sq_qtype,
                            sram_lif_entry.rq_qtype,
                            sram_lif_entry.aq_qtype);
    assert(rc == 0);

    rc = p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add(
                            lif,
                            sram_lif_entry.rdma_en_qtype_mask,
                            sram_lif_entry.pt_base_addr_page_id,
                            sram_lif_entry.ah_base_addr_page_id,
                            sram_lif_entry.log_num_pt_entries,
                            sram_lif_entry.cqcb_base_addr_hi,
                            sram_lif_entry.sqcb_base_addr_hi,
                            sram_lif_entry.rqcb_base_addr_hi,
                            sram_lif_entry.log_num_cq_entries,
                            sram_lif_entry.prefetch_pool_base_addr_page_id,
                            sram_lif_entry.log_num_prefetch_pool_entries,
                            sram_lif_entry.sq_qtype,
                            sram_lif_entry.rq_qtype,
                            sram_lif_entry.aq_qtype,
                            sram_lif_entry.barmap_base_addr,
                            sram_lif_entry.barmap_size);
    assert(rc == 0);

    NIC_FUNC_DEBUG("lif-{}: rdma_params_table init successful", lif);

    return HAL_RET_OK;
}

mem_addr_t
PdClient::mem_start_addr (const char *region)
{
    return mp_->start_addr(region);
}

void
PdClient::set_program_info()
{
    NIC_LOG_DEBUG("Initializing Program Info ...");
    pinfo_ = program_info::factory((gen_dir_path_ +
                                    "mpu_prog_info.json").c_str());
    assert(pinfo_);
    lm_->set_program_info(pinfo_);
}
