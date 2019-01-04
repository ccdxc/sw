// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/platform/capri/capri_cfg.hpp"
#include "include/sdk/platform/capri/capri_tm_rw.hpp"
#include "include/sdk/platform/capri/capri_txs_scheduler.hpp"
#include "include/sdk/platform/capri/capri_quiesce.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/platform/capri/capri_pxb_pcie.hpp"
#include "include/sdk/platform/capri/capri_state.hpp"
#include "include/sdk/platform/capri/capri_common.hpp"
#include "include/sdk/platform/capri/capri_hbm_rw.hpp"
#include "include/sdk/platform/capri/capri_tbl_rw.hpp"
#include "gen/platform/mem_regions.hpp"
#include "nic/asic/capri/verif/apis/cap_npv_api.h"
#include "nic/asic/capri/verif/apis/cap_dpa_api.h"
#include "nic/asic/capri/verif/apis/cap_pics_api.h"
#include "nic/asic/capri/verif/apis/cap_pict_api.h"
#include "nic/asic/capri/verif/apis/cap_ppa_api.h"
#include "nic/asic/capri/verif/apis/cap_prd_api.h"
#include "nic/asic/capri/verif/apis/cap_psp_api.h"
#include "nic/asic/capri/verif/apis/cap_ptd_api.h"
#include "nic/asic/capri/verif/apis/cap_stg_api.h"
#include "nic/asic/capri/verif/apis/cap_wa_api.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_prd/cap_prd_csr.h"
#include "nic/asic/capri/model/utils/cap_csr_py_if.h"

namespace sdk {
namespace platform {
namespace capri {

class capri_state_pd *g_capri_state_pd;
/* capri_default_config_init
 * Load any bin files needed for initializing default configs
 */
static sdk_ret_t
capri_default_config_init (capri_cfg_t *cfg)
{
    sdk_ret_t   ret = SDK_RET_OK;
    std::string hbm_full_path;
    std::string full_path;
    int         num_phases = 2;
    int         i;

    for (i = 0; i < num_phases; i++) {
        full_path =  std::string(cfg->cfg_path) + "/init_bins/" + cfg->default_config_dir + "/init_" +
                                            std::to_string(i) + "_bin";

        SDK_TRACE_DEBUG("Init phase %d Binaries dir: %s", i, full_path.c_str());

        // Check if directory is present
        if (access(full_path.c_str(), R_OK) < 0) {
            SDK_TRACE_DEBUG("Skipping init binaries");
            return SDK_RET_OK;
        }

        ret = capri_load_config((char *)full_path.c_str());
        SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                                "Error loading init phase {} binaries ret {}",
                                i, ret);

        // Now do any polling for init complete for this phase
        capri_tm_hw_config_load_poll(i);
    }

    return ret;
}

static sdk_ret_t
capri_timer_hbm_init (void)
{
    sdk_ret_t ret = SDK_RET_OK;
    uint64_t timer_key_hbm_base_addr;
    uint64_t timer_key_hbm_addr;
    uint64_t zero_data[8] = { 0 };

    timer_key_hbm_base_addr = get_start_offset(MEM_REGION_TIMERS_NAME);
    SDK_TRACE_DEBUG("HBM timer key base addr %lx", timer_key_hbm_base_addr);
    timer_key_hbm_addr = timer_key_hbm_base_addr;
    while (timer_key_hbm_addr < timer_key_hbm_base_addr +
                                CAPRI_TIMER_HBM_KEY_SPACE) {
        sdk::asic::asic_mem_write(timer_key_hbm_addr, (uint8_t *)zero_data,
                                  sizeof(zero_data),
                                  ASIC_WRITE_MODE_WRITE_THRU);
        timer_key_hbm_addr += sizeof(zero_data);
    }

    return ret;
}

static sdk_ret_t
capri_pgm_init (capri_cfg_t *cfg)
{
    sdk_ret_t      ret;
    std::string    full_path;

    for (uint8_t i = 0; i < cfg->num_pgm_cfgs; i++) {
        full_path =  std::string(cfg->cfg_path) + "/" + cfg->pgm_name +
            "/" + cfg->pgm_cfg[i].path;
        SDK_TRACE_DEBUG("Loading programs from dir %s", full_path.c_str());

        // check if directory is present
        if (access(full_path.c_str(), R_OK) < 0) {
            SDK_TRACE_ERR("%s not present/no read permissions",
                          full_path.c_str());
            return SDK_RET_ERR;
        }
        ret = capri_load_config((char *)full_path.c_str());
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Failed to load config %s", full_path);
            return ret;
        }
    }

    return SDK_RET_OK;
}

static sdk_ret_t
capri_asm_init (capri_cfg_t *cfg)
{
    int             iret = 0;
    uint64_t        base_addr;
    std::string     full_path;
    uint32_t num_symbols = 0;
    sdk::platform::p4_prog_param_info_t *symbols = NULL;

    for (uint8_t i = 0; i < cfg->num_asm_cfgs; i++) {
        full_path =  std::string(cfg->cfg_path) + "/" + cfg->pgm_name +
            "/" + cfg->asm_cfg[i].path;
        SDK_TRACE_DEBUG("Loading ASM binaries from dir %s", full_path.c_str());

	// Check if directory is present
	if (access(full_path.c_str(), R_OK) < 0) {
            SDK_TRACE_ERR("%s not_present/no_read_permissions",
                full_path.c_str());
            return SDK_RET_ERR;
        }

        symbols = NULL;
        if(cfg->asm_cfg[i].symbols_func) {
            num_symbols = cfg->asm_cfg[i].symbols_func((void **)&symbols, cfg->platform);
        }

        base_addr = get_start_offset(cfg->asm_cfg[i].base_addr.c_str());
        SDK_TRACE_DEBUG("base addr 0x%llx", base_addr);
        iret = sdk::platform::p4_load_mpu_programs(cfg->asm_cfg[i].name.c_str(),
           (char *)full_path.c_str(),
           base_addr,
           symbols,
           num_symbols,
           cfg->asm_cfg[i].sort_func);

       if(symbols)
           SDK_FREE(SDK_MEM_ALLOC_PD, symbols);

       if (iret != 0) {
          SDK_TRACE_ERR("Failed to load program %s", full_path);
          return SDK_RET_ERR;
       }
   }
   return SDK_RET_OK;
}

static sdk_ret_t
capri_hbm_regions_init (capri_cfg_t *cfg)
{
    sdk_ret_t           ret = SDK_RET_OK;

    ret = capri_asm_init(cfg);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = capri_pgm_init(cfg);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = capri_timer_hbm_init();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // reset all the HBM regions that are marked for reset
    reset_hbm_regions(cfg);

    return ret;
}

static sdk_ret_t
capri_cache_init (capri_cfg_t *cfg)
{
    sdk_ret_t   ret = SDK_RET_OK;

    // Program Global parameters of the cache.
    ret = capri_hbm_cache_init(cfg);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // Process all the regions.
    ret = capri_hbm_cache_regions_init();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    return ret;
}


/*
 * ASIC teams wants to disable Error recovery of Seq ID check pRDMA,
 * as this recovery path is not tested thoroughly, and we might be
 * runing into for an outstanding issue is suspicion.
 * Disabling from SDK for now, but Helen will commit this
 * into prd_asic_init api called from SDK, then this will be removed
 */
static sdk_ret_t
capri_prd_init()
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pr_csr_t &pr_csr = cap0.pr.pr;

    pr_csr.prd.cfg_ctrl.read();
    pr_csr.prd.cfg_ctrl.pkt_phv_sync_err_recovery_en(0);
    pr_csr.prd.cfg_ctrl.write();
    SDK_TRACE_DEBUG("Disabled pkt_phv_sync_err_recovery_en in pr_prd_cfg_ctrl");
    return SDK_RET_OK;
}


static sdk_ret_t
capri_repl_init (capri_cfg_t *cfg)
{
#ifdef MEM_REGION_MCAST_REPL_NAME
    uint64_t hbm_repl_table_offset = get_hbm_offset(MEM_REGION_MCAST_REPL_NAME);
    if (hbm_repl_table_offset != INVALID_MEM_ADDRESS) {
        capri_tm_repl_table_base_addr_set(hbm_repl_table_offset / CAPRI_REPL_ENTRY_WIDTH);
        capri_tm_repl_table_token_size_set(cfg->repl_entry_width * 8);
    }
#endif
    return SDK_RET_OK;
}

typedef struct block_info_s {
    int  inst_count;
    void (*soft_reset)(int chip_id, int inst);
    void (*init_start)(int chip_id, int inst);
    void (*init_done) (int chip_id, int inst);
} block_info_t;

#define MAX_INIT_BLOCKS 10

block_info_t blocks_info[MAX_INIT_BLOCKS];

static void
capri_block_info_init(void)
{
    blocks_info[0].inst_count = 1;
    blocks_info[0].soft_reset = cap_npv_soft_reset;
    blocks_info[0].init_start = cap_npv_init_start;
    blocks_info[0].init_done  = cap_npv_init_done;

    blocks_info[1].inst_count = 2;
    blocks_info[1].soft_reset = cap_dpa_soft_reset;
    blocks_info[1].init_start = cap_dpa_init_start;
    blocks_info[1].init_done  = cap_dpa_init_done;

    blocks_info[2].inst_count = 4;
    blocks_info[2].soft_reset = cap_pics_soft_reset;
    blocks_info[2].init_start = cap_pics_init_start;
    blocks_info[2].init_done  = cap_pics_init_done;

    blocks_info[3].inst_count = 2;
    blocks_info[3].soft_reset = cap_pict_soft_reset;
    blocks_info[3].init_start = cap_pict_init_start;
    blocks_info[3].init_done  = cap_pict_init_done;

    blocks_info[4].inst_count = 2;
    blocks_info[4].soft_reset = cap_ppa_soft_reset;
    blocks_info[4].init_start = cap_ppa_init_start;
    blocks_info[4].init_done  = cap_ppa_init_done;

    blocks_info[5].inst_count = 1;
    blocks_info[5].soft_reset = cap_prd_soft_reset;
    blocks_info[5].init_start = cap_prd_init_start;
    blocks_info[5].init_done  = cap_prd_init_done;

    blocks_info[6].inst_count = 1;
    blocks_info[6].soft_reset = cap_psp_soft_reset;
    blocks_info[6].init_start = cap_psp_init_start;
    blocks_info[6].init_done  = cap_psp_init_done;

    blocks_info[7].inst_count = 1;
    blocks_info[7].soft_reset = cap_ptd_soft_reset;
    blocks_info[7].init_start = cap_ptd_init_start;
    blocks_info[7].init_done  = cap_ptd_init_done;

    blocks_info[8].inst_count = 28;
    blocks_info[8].soft_reset = cap_stg_soft_reset;
    blocks_info[8].init_start = cap_stg_init_start;
    blocks_info[8].init_done  = cap_stg_init_done;

    blocks_info[9].inst_count = 1;
    blocks_info[9].soft_reset = cap_wa_soft_reset;
    blocks_info[9].init_start = cap_wa_init_start;
    blocks_info[9].init_done  = cap_wa_init_done;

    return;
}

//------------------------------------------------------------------------------
// Reset all the capri blocks
//------------------------------------------------------------------------------
static sdk_ret_t
capri_block_reset(capri_cfg_t *cfg)
{
    sdk_ret_t    ret         = SDK_RET_OK;
    int          chip_id     = 0;
    block_info_t *block_info = NULL;

    for (int block = 0; block < MAX_INIT_BLOCKS; ++block) {
        block_info = &blocks_info[block];

        for(int inst = 0; inst < block_info->inst_count; ++inst) {
            block_info->soft_reset(chip_id, inst);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// Start the init of blocks
//------------------------------------------------------------------------------
static sdk_ret_t
capri_block_init_start(capri_cfg_t *cfg)
{
    sdk_ret_t    ret         = SDK_RET_OK;
    int          chip_id     = 0;
    block_info_t *block_info = NULL;

    for (int block = 0; block < MAX_INIT_BLOCKS; ++block) {
        block_info = &blocks_info[block];

        for(int inst = 0; inst < block_info->inst_count; ++inst) {
            block_info->init_start(chip_id, inst);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// Wait for the blocks to initialize
//------------------------------------------------------------------------------
static sdk_ret_t
capri_block_init_done(capri_cfg_t *cfg)
{
    sdk_ret_t    ret         = SDK_RET_OK;
    int          chip_id     = 0;
    block_info_t *block_info = NULL;

    for (int block = 0; block < MAX_INIT_BLOCKS; ++block) {
        block_info = &blocks_info[block];

        for(int inst = 0; inst < block_info->inst_count; ++inst) {
            block_info->init_done(chip_id, inst);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// Init all the capri blocks owned by SDK
//------------------------------------------------------------------------------
sdk_ret_t
capri_block_init(capri_cfg_t *cfg)
{
    sdk_ret_t           ret = SDK_RET_OK;

    // initialize block info
    capri_block_info_init();

    // soft reset
    ret = capri_block_reset(cfg);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // init blocks
    ret = capri_block_init_start(cfg);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // wait for blocks to be inited
    ret = capri_block_init_done(cfg);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return ret;
}

//------------------------------------------------------------------------------
// perform all the CAPRI specific initialization
// - link all the P4 programs, by resolving symbols, labels etc.
// - load the P4/P4+ programs in HBM
// - do all the parser/deparser related register programming
// - do all the table configuration related register programming
//------------------------------------------------------------------------------
static sdk_ret_t
capri_init (capri_cfg_t *cfg)
{
    sdk_ret_t    ret;

    SDK_ASSERT_TRACE_RETURN((cfg != NULL), SDK_RET_INVALID_ARG, "Invalid cfg");
    SDK_TRACE_DEBUG("Initializing Capri");

    ret = capri_hbm_parse(cfg->cfg_path, cfg->pgm_name);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri HBM parse init failure, err : %d", ret);

    g_capri_state_pd = sdk::platform::capri::capri_state_pd::factory();
    SDK_ASSERT_TRACE_RETURN((g_capri_state_pd != NULL), SDK_RET_INVALID_ARG,
                            "Failed to instantiate Capri PD");

    g_capri_state_pd->set_cfg_path(cfg->cfg_path);
    if (capri_table_rw_init(cfg) != CAPRI_OK) {
        return SDK_RET_ERR;
    }

    ret = capri_hbm_regions_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri HBM region init failure, err : %d", ret);

    ret = capri_block_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri block region init failure, err : %d", ret);

    ret = capri_cache_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri cache init failure, err : %d", ret);

    // do asic init before overwriting with the default configs
    ret = capri_tm_asic_init();
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri TM ASIC init failure, err : %d", ret);

    if (!cfg->catalog->qos_sw_init_enabled()) {
        ret = capri_default_config_init(cfg);
        SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                        "Capri default config init failure, err : %d", ret);
    }

    ret = capri_txs_scheduler_init(cfg->admin_cos, cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri scheduler init failure, err : %d", ret);

    // Call PXB/PCIE init only in MODEL and RTL simulation
    // This will be done by PCIe manager for the actual chip
    if (cfg->platform == platform_type_t::PLATFORM_TYPE_SIM ||
        cfg->platform == platform_type_t::PLATFORM_TYPE_RTL) {
        ret = capri_pxb_pcie_init();
        SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                                "PXB/PCIE init failure, err : %d", ret);
    }

    ret = capri_tm_init(cfg->catalog);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri TM init failure, err : %d", ret);

    ret = capri_repl_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri replication init failure, err : %d", ret);

    if (!cfg->loader_info_file.empty()) {
        sdk::platform::p4_list_program_addr(
            cfg->cfg_path.c_str(), cfg->loader_info_file.c_str());
    }

    ret = capri_quiesce_init();
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri quiesce init failure, err : %d", ret);

    ret = capri_prd_init();
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri PRD init failure, err : %d", ret);
    if (cfg->completion_func) {
        cfg->completion_func(sdk::SDK_STATUS_ASIC_INIT_DONE);
    }

    return SDK_RET_OK;
}

} // namespace capri
} // namespace platform
} // namespace sdk

namespace sdk {
namespace asic {

//------------------------------------------------------------------------------
// perform all the CAPRI specific initialization
//------------------------------------------------------------------------------
sdk_ret_t
asic_init (asic_cfg_t *cfg)
{
    capri_cfg_t    capri_cfg;

    SDK_ASSERT(cfg != NULL);
    capri_cfg.loader_info_file = cfg->loader_info_file;
    capri_cfg.default_config_dir = cfg->default_config_dir;
    capri_cfg.cfg_path = cfg->cfg_path;
    capri_cfg.admin_cos = cfg->admin_cos;
    capri_cfg.repl_entry_width = cfg->repl_entry_width;
    capri_cfg.catalog = cfg->catalog;
    capri_cfg.p4_cache = true;
    capri_cfg.p4plus_cache = true;
    capri_cfg.llc_cache = true;
    capri_cfg.platform = cfg->platform;
    capri_cfg.num_pgm_cfgs = cfg->num_pgm_cfgs;
    capri_cfg.pgm_name = cfg->pgm_name;
    for (int i = 0; i < cfg->num_pgm_cfgs; i++) {
        capri_cfg.pgm_cfg[i].path = cfg->pgm_cfg[i].path;
    }
    capri_cfg.num_asm_cfgs = cfg->num_asm_cfgs;
    for (int i = 0; i < cfg->num_asm_cfgs; i++) {
        capri_cfg.asm_cfg[i].name = cfg->asm_cfg[i].name;
        capri_cfg.asm_cfg[i].path = cfg->asm_cfg[i].path;
        capri_cfg.asm_cfg[i].symbols_func = cfg->asm_cfg[i].symbols_func;
        capri_cfg.asm_cfg[i].base_addr = cfg->asm_cfg[i].base_addr;
        capri_cfg.asm_cfg[i].sort_func =
            cfg->asm_cfg[i].sort_func;
    }
    capri_cfg.completion_func = cfg->completion_func;
    return capri_init(&capri_cfg);
}

} // namespace asic
} // namespace sdk
