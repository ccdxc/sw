/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstring>
#include "nic/sdk/asic/pd/pd.hpp"
#include "gen/platform/mem_regions.hpp"

#include "nic/p4/common/defines.h"

#include "logger.hpp"
#include "pd_client.hpp"
#include "eth_dev.hpp"
#include "rdma_dev.hpp"
#include "nicmgr_init.hpp"

using namespace sdk::asic::pd;
using namespace sdk::platform::utils;

#define ENTRY_TRACE_EN          true

#if defined(APOLLO) || defined(ARTEMIS) || defined(APULU) || defined(ATHENA)
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN            P4_P4PLUS_RXDMA_TBL_ID_INDEX_MIN
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX            P4_P4PLUS_RXDMA_TBL_ID_INDEX_MAX
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN            P4_P4PLUS_TXDMA_TBL_ID_INDEX_MIN
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX            P4_P4PLUS_TXDMA_TBL_ID_INDEX_MAX
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMIN               P4_P4PLUS_RXDMA_TBL_ID_TBLMIN
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_TBLMAX               P4_P4PLUS_RXDMA_TBL_ID_TBLMAX
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN               P4_P4PLUS_TXDMA_TBL_ID_TBLMIN
#define P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX               P4_P4PLUS_TXDMA_TBL_ID_TBLMAX
#define P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_PARAMS    P4_P4PLUS_RXDMA_TBL_ID_ETH_RX_RSS_PARAMS
#endif

const static char *kLif2QstateHBMLabel = MEM_REGION_NICMGRQSTATE_MAP_NAME;

const static char *kNicmgrHBMLabel = MEM_REGION_NICMGR_NAME;
const static uint32_t kNicmgrAllocUnit = 64;

const static char *kDevcmdHBMLabel = MEM_REGION_DEVCMD_NAME;
const static uint32_t kDevcmdAllocUnit = 4096;

const static char *kCmbHBMLabel = MEM_REGION_ETH_CMB_NAME;
const static uint32_t kCmbHBMAllocUnit = 8 * 1024 * 1024;


uint8_t *memrev(uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

static void table_health_monitor(uint32_t table_id,
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
#ifdef IRIS
    p4pd_cfg_t                 p4pd_cfg = {
            .table_map_cfg_file  = "iris/capri_p4_rxdma_table_map.json",
            .p4pd_pgm_name       = "iris",
            .p4pd_rxdma_pgm_name = "p4plus",
            .p4pd_txdma_pgm_name = "p4plus",
            .cfg_path            = hal_cfg_path_.c_str(),
    };

    NIC_LOG_DEBUG("Initializing p4plus RXDMA table");
    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_rxdma_init(&p4pd_cfg);
    assert(rc == P4PD_SUCCESS);
#endif

    memset(&tinfo, 0, sizeof(tinfo));
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
#ifdef IRIS
    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "iris/capri_p4_txdma_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = hal_cfg_path_.c_str(),
    };

    NIC_LOG_DEBUG("Initializing p4plus TXDMA table");
    // parse the NCC generated table info file for p4+ tables
    rc = p4pluspd_txdma_init(&p4pd_cfg);
    assert(rc == P4PD_SUCCESS);
#endif

    memset(&tinfo, 0, sizeof(tinfo));
    // start instantiating tables based on the parsed information
    p4plus_txdma_dm_tables_ =
        (directmap **)calloc(sizeof(directmap *),
                             (P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX -
                              P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN + 1));
    SDK_ASSERT(p4plus_txdma_dm_tables_ != NULL);

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMIN;
         tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_TBLMAX; tid++) {
        rc = p4pluspd_txdma_table_properties_get(tid, &tinfo);
        SDK_ASSERT(rc == P4PD_SUCCESS);

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

void
PdClient::cmb_mem_init()
{
    uint64_t hbm_addr = mp_->start_addr(kCmbHBMLabel);
    assert(hbm_addr > 0);

    uint64_t size = mp_->size(kCmbHBMLabel);
    assert(size != 0);

    uint64_t num_units = size / kCmbHBMAllocUnit;
    uint64_t align = kCmbHBMAllocUnit - 1;
    if (hbm_addr & align) {
        // Not naturally aligned.
        hbm_addr = (hbm_addr + align) & ~align;
        num_units--;
    }

    cmb_hbm_base_ = hbm_addr;
    cmb_hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));

    NIC_FUNC_DEBUG("cmb_hbm_base : {:#x}", cmb_hbm_base_);
}

uint64_t
PdClient::cmb_mem_alloc(uint64_t size)
{
    uint64_t addr = 0;
    uint32_t alloc_units;

    alloc_units = (size + kCmbHBMAllocUnit - 1) & ~(kCmbHBMAllocUnit - 1);
    alloc_units /= kCmbHBMAllocUnit;
    int alloc_offset = cmb_hbm_allocator_->Alloc(alloc_units);

    if (alloc_offset < 0) {
        NIC_FUNC_ERR("Invalid alloc_offset {}", alloc_offset);
        return 0;
    }

    cmb_allocation_sizes_[alloc_offset] = alloc_units;
    alloc_offset *= kCmbHBMAllocUnit;
    addr = cmb_hbm_base_ + alloc_offset;
    NIC_FUNC_DEBUG("size: {} alloc_offset: {} addr: {:#x}",
                    size, alloc_offset, addr);

    // Allocations must be naturally aligned because this memory is used
    // for cmb bar.
    assert((addr & (kCmbHBMAllocUnit - 1)) == 0);

    return addr;
}

int
PdClient::cmb_mem_reserve(uint64_t addr, uint64_t size)
{
    uint32_t alloc_units;
    int alloc_offset, reserved_offset = 0;

    // Allocations must be naturally aligned because this memory is used
    // for cmb bar.
    assert((addr & (kCmbHBMAllocUnit - 1)) == 0);

    alloc_offset = ((addr - cmb_hbm_base_)/kCmbHBMAllocUnit);

    alloc_units = (size + kCmbHBMAllocUnit - 1) & ~(kCmbHBMAllocUnit - 1);
    alloc_units /= kCmbHBMAllocUnit;
    reserved_offset = cmb_hbm_allocator_->CheckAndReserve(alloc_offset, alloc_units);

    if ((reserved_offset < 0) || (reserved_offset != alloc_offset)) {
        NIC_FUNC_ERR("Failed to reserve cmb mem at addr {}", addr);
        return -1;
    }

    cmb_allocation_sizes_[alloc_offset] = alloc_units;
    NIC_FUNC_DEBUG("size: {} reserved_offset: {} addr: {:#x}",
                    size, reserved_offset, addr);

    return 0;
}

void
PdClient::nicmgr_mem_init(void)
{
    uint64_t hbm_addr = mp_->start_addr(kNicmgrHBMLabel);
    assert(hbm_addr > 0);

    uint64_t size = mp_->size(kNicmgrHBMLabel);
    assert(size != 0);

    uint64_t num_units = size / kNicmgrAllocUnit;
    if (hbm_addr & 0xFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
        num_units--;
    }

    nicmgr_hbm_base_ = hbm_addr;
    nicmgr_hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));

    NIC_FUNC_DEBUG("nicmgr_hbm_base : {:#x}", nicmgr_hbm_base_);
}

uint64_t
PdClient::nicmgr_mem_alloc(uint64_t size)
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

int
PdClient::nicmgr_mem_reserve(uint64_t addr, uint64_t size)
{
    uint32_t alloc_units;
    int alloc_offset, reserved_offset = 0;

    // Allocations must be cache-line aligned because this memory is used
    // for rings.
    assert((addr & 0x3F) == 0);

    alloc_offset = ((addr - nicmgr_hbm_base_)/kNicmgrAllocUnit);

    alloc_units = (size + kNicmgrAllocUnit - 1) & ~(kNicmgrAllocUnit - 1);
    alloc_units /= kNicmgrAllocUnit;
    reserved_offset = nicmgr_hbm_allocator_->CheckAndReserve(alloc_offset, alloc_units);

    if ((reserved_offset < 0) || (reserved_offset != alloc_offset)) {
        NIC_FUNC_ERR("Failed to reserve nicmgr mem at addr {}", addr);
        return -1;
    }

    nicmgr_allocation_sizes_[alloc_offset] = alloc_units;
    NIC_FUNC_DEBUG("size: {} reserved_offset: {} addr: {:#x}",
                    size, reserved_offset, addr);

    return 0;
}

void
PdClient::devcmd_mem_init(void)
{
    uint64_t hbm_addr = mp_->start_addr(kDevcmdHBMLabel);
    assert(hbm_addr > 0);

    uint64_t size = mp_->size(kDevcmdHBMLabel);
    assert(size != 0);

    uint64_t num_units = size / kDevcmdAllocUnit;
    if (hbm_addr & 0xFFF) {
        // Not 4K aligned.
        hbm_addr = (hbm_addr + 0xFFF) & ~0xFFFULL;
        num_units--;
    }

    devcmd_hbm_base_ = hbm_addr;
    devcmd_hbm_allocator_.reset(new sdk::lib::BMAllocator(num_units));

    NIC_FUNC_DEBUG("devcmd_hbm_base : {:#x}", devcmd_hbm_base_);
}

uint64_t
PdClient::devcmd_mem_alloc(uint64_t size)
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

int
PdClient::devcmd_mem_reserve(uint64_t addr, uint64_t size)
{
    uint32_t alloc_units;
    int alloc_offset, reserved_offset = 0;

    // devcmd allocations should be page aligned
    assert((addr & 0xFFF) == 0);

    alloc_offset = ((addr - devcmd_hbm_base_)/kDevcmdAllocUnit);

    alloc_units = (size + kDevcmdAllocUnit - 1) & ~(kDevcmdAllocUnit - 1);
    alloc_units /= kDevcmdAllocUnit;
    reserved_offset = devcmd_hbm_allocator_->CheckAndReserve(alloc_offset, alloc_units);

    if ((reserved_offset < 0) || (reserved_offset != alloc_offset)) {
        NIC_FUNC_ERR("Failed to reserve devcmd mem at addr {}", addr);
        return -1;
    }

    devcmd_allocation_sizes_[alloc_offset] = alloc_units;
    NIC_FUNC_DEBUG("size: {} reserved_offset: {} addr: {:#x}",
                    size, reserved_offset, addr);

    return 0;
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
PdClient::intr_reserve(uint32_t intr_base, uint32_t count)
{
    auto ret = intr_allocator->alloc_withid(intr_base, count);
    if (ret != sdk::lib::indexer::SUCCESS) {
        NIC_FUNC_ERR("Failed to reserve interrupts");
        return -1;
    }

    NIC_FUNC_DEBUG("reserved: base {} count {}", intr_base, count);

    return 0;
}

int
PdClient::create_dirs() {
    struct stat  st = { 0 };

    if (hal_cfg_path_ == "") {
        // use the current dir
        hal_cfg_path_ = "./";
        gen_dir_path_ = "./";
    } else {
        gen_dir_path_ = string(hal_cfg_path_ + "/gen");
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

void
PdClient::init()
{
    int ret;

#ifdef IRIS
    // initialize asicpd_state_pd
    sdk::asic::pd::asicpd_state_pd_init(NULL);
#endif

    NIC_LOG_DEBUG("Loading p4plus RxDMA asic lib tables cfg_path: {}...", hal_cfg_path_);
    ret = p4plus_rxdma_init_tables();
    assert(ret == 0);

    NIC_LOG_DEBUG("Loading p4plus TxDMA asic lib tables cfg_path: {}...", hal_cfg_path_);
    ret = p4plus_txdma_init_tables();
    assert(ret == 0);

    NIC_LOG_DEBUG("Initializing HBM Memory Partitions from: {}...", hal_cfg_path_);
    mp_ = mpartition::factory(mpart_cfg_path_.c_str());
    assert(mp_);

    NIC_LOG_DEBUG("Initializing LIF Manager ...");
    lm_ = lif_mgr::factory(kNumMaxLIFs, mp_, kLif2QstateHBMLabel);
    assert(lm_);

#ifdef IRIS
    NIC_LOG_DEBUG("Initializing table rw ...");
    ret = asicpd_p4plus_table_rw_init();
    assert(ret == 0);
#endif

    rdma_mgr_ = rdma_manager_init(mp_, lm_);

    nicmgr_mem_init();
    devcmd_mem_init();
    cmb_mem_init();

    intr_allocator = sdk::lib::indexer::factory(4096);
    assert(intr_allocator != NULL);
}

// called after HAL is UP and running
void PdClient::update(void)
{
    set_program_info();
}

PdClient* PdClient::factory(sdk::platform::platform_type_t platform,
                            std::string mpart_file,
                            std::string cfg_path)
{
    int ret;
    PdClient *pdc = new PdClient();

    assert(pdc);
    assert(!cfg_path.empty());

    pdc->platform_ = platform;

    pdc->hal_cfg_path_ = cfg_path;
    NIC_LOG_INFO("HAL config path {}", pdc->hal_cfg_path_);

    pdc->gen_dir_path_ = pdc->hal_cfg_path_ + "/gen";
    NIC_LOG_INFO("GEN directory path {}", pdc->gen_dir_path_);

    pdc->mpart_cfg_path_ = mpart_file;
    NIC_LOG_INFO("MPART config path {}", pdc->mpart_cfg_path_);

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
    lif_qstate_t qstate;

    memset(&qstate, 0, sizeof(lif_qstate_t));

    qstate.lif_id = hw_lif_id;

    for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto & qinfo = queue_info[i];
        if (qinfo.size < 1) continue;

        if (qinfo.size > 7 || qinfo.entries > 24) {
            NIC_LOG_ERR("Invalid entry in LifSpec : size={} entries={}",
                          qinfo.size, qinfo.entries);
            return -1;
        }

        if (qinfo.purpose > sdk::platform::LIF_QUEUE_PURPOSE_MAX) {
            NIC_LOG_ERR("Invalid entry in LifSpec : purpose={}", qinfo.purpose);
            return -1;
        }

        qstate.type[qinfo.type_num].qtype_info.size = qinfo.size;
        qstate.type[qinfo.type_num].qtype_info.entries = qinfo.entries;

        // Set both cosA,cosB to admin_cos(cosA) value for admin-qtype.
        if (qinfo.purpose != sdk::platform::LIF_QUEUE_PURPOSE_ADMIN) {
            qstate.type[qinfo.type_num].qtype_info.cosA = (coses & 0x0f);
            qstate.type[qinfo.type_num].qtype_info.cosB = (coses & 0xf0) >> 4;
        } else {
            qstate.type[qinfo.type_num].qtype_info.cosA =
                qstate.type[qinfo.type_num].qtype_info.cosB = (coses & 0x0f);
        }
    }

    qstate.hint_cos = (coses & 0xf0) >> 4;

    // Reserve lif id
    lm_->reserve_id(qstate.lif_id, 1);

    // Init the lif
    lm_->init(&qstate);

    // Zero out qstate
    lm_->clear_qstate(qstate.lif_id);

    // Program qstate map
    lm_->enable(qstate.lif_id);

    return 0;
}

int
PdClient::lif_qstate_init(uint64_t hw_lif_id, struct queue_info* queue_info)
{
    sdk_ret_t ret = SDK_RET_OK;
    uint8_t bzero64[64] = {0};

    NIC_FUNC_INFO("lif{}: Started initializing Qstate", hw_lif_id);

    for (uint32_t qtype = 0; qtype < NUM_QUEUE_TYPES; qtype++) {
        auto & qinfo = queue_info[qtype];
        if (qinfo.size < 1) continue;

        for (uint32_t qid = 0; qid < (uint32_t) pow(2, qinfo.entries); qid++) {
            ret = lm_->write_qstate(hw_lif_id, qtype, qid, bzero64, sizeof(bzero64));
            if (ret != SDK_RET_OK) {
                NIC_LOG_ERR("Failed to set LIFQState : {}", ret);
                return -1;
            }
        }
    }

    NIC_FUNC_INFO("lif{}: Finished initializing Qstate", hw_lif_id);

    return 0;
}

int
PdClient::program_qstate(struct queue_info* queue_info,
                             lif_info_t *lif_info,
                             uint8_t coses)
{
    int ret;

    NIC_LOG_DEBUG("lif-{}: Programming qstate", lif_info->lif_id);

    // init queue state map
    ret = lif_qstate_map_init(lif_info->lif_id, queue_info, coses);
    if (ret != 0) {
        return -1;
    }

    // init queues
    ret = lif_qstate_init(lif_info->lif_id, queue_info);
    if (ret != 0) {
        NIC_LOG_ERR("Failed to do lif qstate: ret: {}", ret);
        return -1;
    }

    for (uint32_t type = 0; type < NUM_QUEUE_TYPES; type++) {
        auto &qinfo = queue_info[type];
        if (qinfo.size < 1) continue;

        // lif_info->qstate_addr[type] = lm_->GetLIFQStateAddr(lif_info->lif_id, type, 0);
        lif_info->qstate_addr[type] = lm_->get_lif_qstate_addr(lif_info->lif_id, type, 0);
        NIC_LOG_DEBUG("lif-{}: qtype: {}, qstate_base: {:#x}",
                     lif_info->lif_id,
                     type, lif_info->qstate_addr[type]);
    }

    return 0;
}

uint8_t
PdClient::get_iq(uint8_t pcp_or_dscp, uint8_t pinned_uplink_port_num)
{
#ifdef IRIS
    typedef struct pd_qos_dscp_cos_map_s {
        uint8_t     is_dscp : 1;
        uint8_t     rsvd1: 7;
        uint8_t     no_drop[16]; // 128-bits. 2 bits per DSCP/PCP.
        uint8_t     txdma_iq[32];
        uint8_t     no_drop1_txdma_iq : 4;
        uint8_t     no_drop2_txdma_iq : 4;
        uint8_t     no_drop3_txdma_iq : 4;
        uint8_t     rsvd2: 4;
        uint8_t     rsvd3[13];
    } __PACK__ pd_qos_dscp_cos_map_t;

    pd_qos_dscp_cos_map_t qos_map;
    uint8_t     no_drop_index = 0, tx_iq = 0;

    if (pcp_or_dscp > 64) {
        NIC_LOG_ERR("Invalid pcp_or_dscp value {}", pcp_or_dscp);
        return 0;
    }

    uint64_t addr = mp_->start_addr(MEM_REGION_QOS_DSCP_COS_MAP);
    sdk::asic::asic_mem_read(addr, (uint8_t *)&qos_map, sizeof(qos_map));

    if ((pcp_or_dscp % 4) == 0) {
        no_drop_index = qos_map.no_drop[pcp_or_dscp / 4] & 0xc0;
        no_drop_index >>= 6;
    } else if ((pcp_or_dscp % 4) == 1) {
        no_drop_index = qos_map.no_drop[pcp_or_dscp / 4] & 0x30;
        no_drop_index >>= 4;
    } else if ((pcp_or_dscp % 4) == 2) {
        no_drop_index = qos_map.no_drop[pcp_or_dscp / 4] & 0xc;
        no_drop_index >>= 2;
    } else {
        no_drop_index = qos_map.no_drop[pcp_or_dscp / 4] & 0x3;
    }

    // No-Drop IQ. Also, IQ for uplink port 1 for Drop-TC
    if ((pcp_or_dscp % 2) == 0) {
        tx_iq = qos_map.txdma_iq[pcp_or_dscp / 2] & 0xf0;
        tx_iq >>= 4;
    } else {
        tx_iq = qos_map.txdma_iq[pcp_or_dscp / 2] & 0x0f;
    }

   switch (no_drop_index) {
       case 0:
           // Drop TC. Return tx_iq
           break;

       case 1:
           // No-drop TC
           //  Is this a good check for second MAC?
           if (pinned_uplink_port_num >= 5)
               tx_iq = qos_map.no_drop1_txdma_iq;
           break;

       case 2:
           // No-drop TC
           if (pinned_uplink_port_num >= 5)
               tx_iq = qos_map.no_drop2_txdma_iq;
           break;

       case 3:
           // No-drop TC
           if (pinned_uplink_port_num >= 5)
               tx_iq = qos_map.no_drop3_txdma_iq;
           break;

       default:
           break;
   }

    NIC_LOG_DEBUG("Programming tx-iq {} for DCSP/PCP {} and uplink_port_num {}",
                   tx_iq, pcp_or_dscp, pinned_uplink_port_num);
    return tx_iq;
#else
    return 0;
#endif
}

int
PdClient::eth_program_rss(uint32_t hw_lif_id, uint16_t rss_type,
                          uint8_t *rss_key, uint8_t *rss_indir,
                          uint16_t num_queues)
{
    sdk_ret_t ret = SDK_RET_OK;
    assert(hw_lif_id < MAX_LIFS);
    assert(num_queues < ETH_RSS_MAX_QUEUES);

    // program the indirection table before the params table as it is downstream
    // from the params table. rss params like should be set after
    // the indirection table is completely programmed.
    NIC_LOG_DEBUG("RSS Programming indirection table hw-lif-id {} start",
                  hw_lif_id);
    if (num_queues > 0) {
        for (unsigned int index = 0; index < ETH_RSS_LIF_INDIR_TBL_LEN; index++) {
            ret = p4plus_rxdma_rss_indir_update(
                    hw_lif_id, index, 0xff, rss_indir[index]);
            if(ret != SDK_RET_OK) {
                NIC_LOG_ERR("RSS Programming indir failed error : {}"
                              "hw-lif-id : {}, index : {}, rss_indir : {}",
                              ret, hw_lif_id, index, rss_indir[index]);
                return 1;
            }
        }
    }

    ret = p4plus_rxdma_rss_params_update(hw_lif_id, rss_type, rss_key, false);
    if(ret != SDK_RET_OK) {
        NIC_LOG_ERR("RSS Programming failed error : {} "
                      "hw-lif-id : {}, rss_type : {}",
                      ret, hw_lif_id, rss_type);
        return 1;
    }

    NIC_LOG_DEBUG("RSS Programming indirection table hw-lif-id {} end",
                  hw_lif_id);

    return 0;
}

uint64_t
PdClient::rdma_get_pt_base_addr (uint32_t lif)
{
    return rdma_mgr_->rdma_get_pt_base_addr(lif);
}

uint64_t
PdClient::rdma_get_kt_base_addr (uint32_t lif)
{
    return rdma_mgr_->rdma_get_kt_base_addr(lif);
}

#if 0
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
#endif

sdk_ret_t
PdClient::rdma_lif_init (uint32_t lif, uint32_t max_keys,
                         uint32_t max_ahs, uint32_t max_ptes,
                         uint64_t mem_bar_addr, uint32_t mem_bar_size,
                         uint32_t max_prefetch_wqes)
{
    if (rdma_mgr_) {
        return rdma_mgr_->lif_init(lif, max_keys, max_ahs, max_ptes,
                                   mem_bar_addr, mem_bar_size, max_prefetch_wqes);
    }
    return SDK_RET_ERR;
}

void
PdClient::set_program_info()
{
    NIC_LOG_DEBUG("Initializing Program Info ...");
    pinfo_ = program_info::factory((gen_dir_path_ +
                                    "/mpu_prog_info.json").c_str());
    assert(pinfo_);
    // lm_->set_program_info(pinfo_);
}

// TODO: Eventually may have to be moved to SDK
// label NULL => return base address
int32_t
PdClient::get_pc_offset(const char *prog_name, const char *label,
                        uint8_t *offset, uint64_t *base)
{
     mem_addr_t off;

     if (label == NULL) {
         off = pinfo_->program_base_address((char *)prog_name);
         if (off == SDK_INVALID_HBM_ADDRESS)
             return -ENOENT;
         // 64 byte alignment check
         if ((off & 0x3F) != 0) {
             return -EIO;
         }
         *base = off;
     } else {
         off = pinfo_->symbol_address((char *)prog_name, (char *)label);
         if (off == SDK_INVALID_HBM_ADDRESS)
             return -ENOENT;
         // 64 byte alignment check
         if ((off & 0x3F) != 0) {
             return -EIO;
         }
         // offset can be max 14 bits
         if (off > 0x3FC0) {
             return -EIO;
         }
         *offset = (uint8_t) (off >> 6);
     }
     return 0;
}

bool
PdClient::is_dev_hwinit_done (const char *dev_name)
{
    // TODO, check for Dev Status in HW
    if (!sdk::asic::asic_is_hard_init()) {
        if (!dev_name) {
            return true;
        } // else TODO
        return true;
    } else {
        return false;
    }
}
