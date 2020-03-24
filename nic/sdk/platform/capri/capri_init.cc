// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "platform/capri/capri_cfg.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "platform/capri/capri_txs_scheduler.hpp"
#include "platform/capri/capri_qstate.hpp"
#include "platform/capri/capri_quiesce.hpp"
#include "asic/rw/asicrw.hpp"
#include "lib/p4/p4_api.hpp"
#include "lib/pal/pal.hpp"
#include "include/sdk/mem.hpp"
#include "platform/capri/capri_pxb_pcie.hpp"
#include "platform/capri/capri_state.hpp"
#include "platform/capri/capri_common.hpp"
#include "platform/capri/capri_hbm_rw.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "platform/capri/capri_barco_crypto.hpp"
#include "third-party/asic/capri/verif/apis/cap_npv_api.h"
#include "third-party/asic/capri/verif/apis/cap_dpa_api.h"
#include "third-party/asic/capri/verif/apis/cap_pics_api.h"
#include "third-party/asic/capri/verif/apis/cap_pict_api.h"
#include "third-party/asic/capri/verif/apis/cap_ppa_api.h"
#include "third-party/asic/capri/verif/apis/cap_prd_api.h"
#include "third-party/asic/capri/verif/apis/cap_psp_api.h"
#include "third-party/asic/capri/verif/apis/cap_ptd_api.h"
#include "third-party/asic/capri/verif/apis/cap_stg_api.h"
#include "third-party/asic/capri/verif/apis/cap_wa_api.h"
#include "third-party/asic/capri/verif/apis/cap_elam_api.h"
//#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "third-party/asic/capri/model/cap_prd/cap_prd_csr.h"
#include "third-party/asic/capri/model/cap_ms/cap_ms_csr.h"
#include "third-party/asic/capri/model/utils/cap_csr_py_if.h"
#include "third-party/asic/capri/model/cap_top/cap_top_csr_defines.h"

namespace sdk {
namespace platform {
namespace capri {

class capri_state_pd *g_capri_state_pd;
/* capri_default_config_init
 * Load any bin files needed for initializing default configs
 */
sdk_ret_t
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
                                "Error loading init phase %d binaries ret %d",
                                i, ret);

        // Now do any polling for init complete for this phase
        capri_tm_hw_config_load_poll(i);
    }

    return ret;
}

sdk_ret_t
capri_pgm_init (void)
{
    sdk_ret_t      ret;
    std::string    full_path;
    capri_cfg_t *cfg = g_capri_state_pd->cfg();

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
    sdk::p4::p4_param_info_t *symbols = NULL;

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
        if (cfg->asm_cfg[i].symbols_func) {
            num_symbols = cfg->asm_cfg[i].symbols_func((void **)&symbols, cfg->platform);
        }

        base_addr = capri_get_mem_addr(cfg->asm_cfg[i].base_addr.c_str());
        SDK_TRACE_DEBUG("base addr 0x%llx", base_addr);
        iret = sdk::p4::p4_load_mpu_programs(cfg->asm_cfg[i].name.c_str(),
           (char *)full_path.c_str(),
           base_addr,
           symbols,
           num_symbols,
           cfg->asm_cfg[i].sort_func,
           sdk::asic::is_soft_init());

       if (symbols) {
           for (uint32_t j = 0; j < num_symbols; j++) {
               symbols[j].name = "";
           }
           ::free(symbols);
       }

       if (iret != 0) {
          SDK_TRACE_ERR("Failed to load program %s", full_path);
          return SDK_RET_ERR;
       }
   }

    // Taking too much time
    if (unlikely(cfg->platform == platform_type_t::PLATFORM_TYPE_SIM)) {
        sdk::p4::p4_dump_program_info(cfg->cfg_path.c_str());
    }

   return SDK_RET_OK;
}

static sdk_ret_t
capri_hbm_regions_init (capri_cfg_t *cfg)
{
    sdk_ret_t           ret = SDK_RET_OK;

    // reset all the HBM regions that are marked for reset
    reset_hbm_regions(cfg);

    ret = capri_asm_init(cfg);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = capri_pgm_init();
    if (ret != SDK_RET_OK) {
        return ret;
    }

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


static sdk_ret_t
capri_prd_init()
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_pr_csr_t &pr_csr = cap0.pr.pr;
    cap_pt_csr_t &pt_csr = cap0.pt.pt;
    cap_ms_csr_t &ms_csr = cap0.ms.ms;

/*
 * ASIC teams wants to disable Error recovery of Seq ID check pRDMA,
 * as this recovery path is not tested thoroughly, and we might be
 * runing into for an outstanding issue is suspicion.
 * Disabling from SDK for now, but Helen will commit this
 * into prd_asic_init api called from SDK, then this will be removed
 */
    pr_csr.prd.cfg_ctrl.read();
    pr_csr.prd.cfg_ctrl.pkt_phv_sync_err_recovery_en(0);
    pr_csr.prd.cfg_ctrl.write();
    SDK_TRACE_DEBUG("Disabled pkt_phv_sync_err_recovery_en in pr_prd_cfg_ctrl");


/*
 * Workaround for TxDMA stuck on recirc issue -
 * If a recirc packet hits PDMA when the pipeline is full, the pipeline gets stuck
 * since there is an XOFF from stage0 preventing the recirc packet from re-entering
 * the pipeline. This XOFF to recirc packet will in-turn back-pressure the pipeline
 * resulting in a deadlock. Workaround for this is to bring down the max-number-of-phv
 * setting in P4+ thereby creating more room in pipeline.
 * Programming it to 84 in both TxDMA/RxDMA for now which is holding up for RDMA.
 * ASIC team is working on the right number to tune this.
 */

    pr_csr.prd.cfg_xoff.read();
    pr_csr.prd.cfg_xoff.numphv_thresh(84);
    pr_csr.prd.cfg_xoff.write();

    pt_csr.ptd.cfg_xoff.read();
    pt_csr.ptd.cfg_xoff.numphv_thresh(84);
    pt_csr.ptd.cfg_xoff.write();
    SDK_TRACE_DEBUG("Programmed numphv_thresh to 84 in pr_prd_cfg_xoff/pt_ptd_cfg_xoff");

/*
 *  Enable elam by default to get a snapshot of PDMA after hitting an error.
 *  ASIC team confirmed that there should not be any impact on performance if we turn this
 *  on by default.
 */

    cap_ms_elam_write_m_fields(0, 0, 0, 0, 121, 1, 1, 123, 1, 1);
    cap_ms_elam_write_capture_en_fields(0, 0, 0, 0, 1);

    pr_csr.prd.cfg_debug_port.read();
    pr_csr.prd.cfg_debug_port.enable(1);
    pr_csr.prd.cfg_debug_port.select(1);
    pr_csr.prd.cfg_debug_port.write();

    pr_csr.prd.cfg_debug_ctrl.read();
    pr_csr.prd.cfg_debug_ctrl.dbg_bus_sel(2);
    pr_csr.prd.cfg_debug_ctrl.cmn_dbg_sel(1);
    pr_csr.prd.cfg_debug_ctrl.write();


    ms_csr.cfg_elam_general.read();
    ms_csr.cfg_elam_general.rst(1);
    ms_csr.cfg_elam_general.write();

    ms_csr.cfg_elam_general.read();
    ms_csr.cfg_elam_general.rst(0);
    ms_csr.cfg_elam_general.arm(1);
    ms_csr.cfg_elam_general.num_post_sample(1);
    ms_csr.cfg_elam_general.write();

    return SDK_RET_OK;
}


static sdk_ret_t
capri_repl_init (capri_cfg_t *cfg)
{
    uint64_t hbm_repl_table_offset = capri_get_mem_offset(MEM_REGION_MCAST_REPL_NAME);
    if (hbm_repl_table_offset != INVALID_MEM_ADDRESS) {
        capri_tm_repl_table_base_addr_set(hbm_repl_table_offset / CAPRI_REPL_ENTRY_WIDTH);
        capri_tm_repl_table_token_size_set(cfg->repl_entry_width * 8);
    }
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

    ret = sdk::platform::capri::capri_state_pd_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "capri_state_pd_init failure, err : %d", ret);

    ret = capri_table_rw_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "capri_tbl_rw_init failure, err : %d", ret);

    // soft/upgrade, need to initialize only asm and tm.
    if (!sdk::asic::is_hard_init()) {
        ret = capri_asm_init(cfg);
        SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                                "Capri ASM init failure, err : %d", ret);
        // initialize the profiles for capri register accesses by other modules (link manager).
        ret = capri_tm_init(cfg->catalog,
                        &cfg->device_profile->qos_profile);
        SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                                "Capri TM Slave init failure, err : %d", ret);
        goto end;
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

    // Call PXB/PCIE init only in MODEL and RTL simulation
    // This will be done by PCIe manager for the actual chip
    if (cfg->platform == platform_type_t::PLATFORM_TYPE_SIM ||
        cfg->platform == platform_type_t::PLATFORM_TYPE_RTL) {
        ret = capri_pxb_pcie_init();
        SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                                "PXB/PCIE init failure, err : %d", ret);
    }

    ret = capri_tm_init(cfg->catalog,
                        &cfg->device_profile->qos_profile);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri TM init failure, err : %d", ret);

    ret = capri_repl_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri replication init failure, err : %d", ret);

    ret = capri_barco_crypto_init(cfg->platform);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri barco crypto init failure, err : {}", ret);

    // TODO: It's not clear why this is needed here
    ret = capri_quiesce_init();
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri quiesce init failure, err : %d", ret);

    ret = capri_prd_init();
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Capri PRD init failure, err : %d", ret);

    ret = capri_txs_scheduler_init(cfg->admin_cos, cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                             "Capri scheduler init failure, err : %d", ret);

end:
    if (cfg->completion_func) {
        cfg->completion_func(sdk::SDK_STATUS_ASIC_INIT_DONE);
    }

    return ret;
}

inline uint64_t
capri_local_dbaddr (void)
{
    uint64_t db_addr =
#ifdef __aarch64__
       CAP_ADDR_BASE_DB_WA_OFFSET +
#endif // __aarch64__
       CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS;

    return db_addr;
}

inline uint64_t
capri_local_db32_addr (void)
{
    uint64_t db_addr =
#ifdef __aarch64__
       CAP_ADDR_BASE_DB_WA_OFFSET +
#endif // __aarch64__
       CAP_WA_CSR_DHS_32B_DOORBELL_BYTE_ADDRESS;

    return db_addr;
}

inline uint64_t
capri_host_dbaddr (void)
{
    return CAP_WA_CSR_DHS_HOST_DOORBELL_BYTE_ADDRESS;
}

}    // namespace capri
}    // namespace platform
}    // namespace sdk

namespace sdk {
namespace asic {

static asic_init_type_t asic_init_type = ASIC_INIT_TYPE_HARD;
static asic_state_t asic_state = ASIC_STATE_RUNNING;

__attribute__((constructor)) void asic_slave_init_(void) {
    char *value;

    if ((value = getenv("ASIC_SOFT_INIT"))) {
        asic_init_type = ASIC_INIT_TYPE_SOFT;
    } else {
        asic_init_type = ASIC_INIT_TYPE_HARD;
    }
}

bool
is_soft_init (void)
{
    return asic_init_type == ASIC_INIT_TYPE_SOFT ? true : false;
}

bool
is_hard_init (void)
{
    return asic_init_type == ASIC_INIT_TYPE_HARD ? true : false;
}

void
set_init_type (asic_init_type_t type)
{
    asic_init_type = type;
}

void
set_state (asic_state_t state)
{
    asic_state = state;
}

bool
is_quiesced (void)
{
    return asic_state == ASIC_STATE_QUIESCED ? true : false;
}

uint64_t
asic_local_dbaddr_get (void)
{
    return sdk::platform::capri::capri_local_dbaddr();
}

uint64_t
asic_local_db32_addr_get (void)
{
    return sdk::platform::capri::capri_local_db32_addr();
}

uint64_t
asic_host_dbaddr_get (void)
{
    return sdk::platform::capri::capri_host_dbaddr();
}

//------------------------------------------------------------------------------
// perform all the CAPRI specific initialization
//------------------------------------------------------------------------------
sdk_ret_t
asic_init (asic_cfg_t *cfg)
{
    capri_cfg_t    capri_cfg;

    SDK_ASSERT(cfg != NULL);
    capri_cfg.default_config_dir = cfg->default_config_dir;
    capri_cfg.cfg_path = cfg->cfg_path;
    capri_cfg.admin_cos = cfg->admin_cos;
    capri_cfg.repl_entry_width = cfg->repl_entry_width;
    capri_cfg.catalog = cfg->catalog;
    capri_cfg.mempartition = cfg->mempartition;
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
    for (int i = 0; i < cfg->num_rings; i++) {
        sdk::platform::ring ring;
        ring.init(&cfg->ring_meta[i], cfg->mempartition);
    }

    capri_cfg.completion_func = cfg->completion_func;
    capri_cfg.device_profile = cfg->device_profile;
    return capri_init(&capri_cfg);
}

void
asic_cleanup (void)
{
    sdk::p4::p4_cleanup();
}

}    // namespace asic
}    // namespace sdk
