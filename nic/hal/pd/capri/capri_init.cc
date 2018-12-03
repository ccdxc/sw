// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/include/base.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/pd/capri/capri.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_config.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/include/hal.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/hal/pd/p4pd/p4pd_api.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/hal/pd/capri/capri_pxb_pcie.hpp"
#include "nic/hal/pd/capri/capri_state.hpp"
#include "nic/hal/pd/capri/capri_sw_phv.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/include/capri_common.h"
#include "nic/include/cpupkt_headers.hpp"
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
#include "nic/hal/pd/capri/capri_quiesce.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_prd/cap_prd_csr.h"
#include "nic/asic/capri/model/utils/cap_csr_py_if.h"

#define P4PLUS_SYMBOLS_MAX  135

class capri_state_pd *g_capri_state_pd;
uint64_t capri_hbm_base;
uint64_t hbm_repl_table_offset;
uint32_t capri_coreclk_freq; //Mhz

/* capri_default_config_init
 * Load any bin files needed for initializing default configs
 */
hal_ret_t
capri_default_config_init (capri_cfg_t *cfg)
{
    hal_ret_t   ret = HAL_RET_OK;
    std::string hbm_full_path;
    std::string full_path;
    int         num_phases = 2;
    int         i;

    for (i = 0; i < num_phases; i++) {
        full_path =  std::string(cfg->cfg_path) + "/init_bins/" + cfg->default_config_dir + "/init_" +
                                            std::to_string(i) + "_bin";

        HAL_TRACE_DEBUG("Init phase {} Binaries dir: {}", i, full_path.c_str());

        // Check if directory is present
        if (access(full_path.c_str(), R_OK) < 0) {
            HAL_TRACE_DEBUG("Skipping init binaries");
            return HAL_RET_OK;
        }

        ret = capri_load_config((char *)full_path.c_str());
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error loading init phase {} binaries ret {}", i, ret);
            HAL_ASSERT_RETURN(0, HAL_RET_ERR);
        }

        // Now do any polling for init complete for this phase
        capri_tm_hw_config_load_poll(i);
    }

    return ret;
}

hal_ret_t
capri_default_config_verify (void)
{
    hal_ret_t   ret = HAL_RET_OK;
    std::string full_path;
    int         num_phases = 2;
    int         i;

    g_capri_state_pd->cfg_path();
    for (i = 0; i < num_phases; i++) {
        full_path =  g_capri_state_pd->cfg_path()  + "/init_bins/" + "init_" +
                         std::to_string(i) + "_bin";

        // check if directory is present
        if (access(full_path.c_str(), R_OK) < 0) {
            HAL_TRACE_DEBUG("Skipping init binaries");
            return HAL_RET_OK;
        }

        HAL_TRACE_DEBUG("Init phase {} Binaries dir: {}", i, full_path.c_str());

        ret = capri_verify_config((char *)full_path.c_str());
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error verifying init phase {} binaries ret {}", i, ret);
            HAL_ASSERT_RETURN(0, HAL_RET_ERR);
        }

    }

    return ret;
}

//------------------------------------------------------------------------------
// perform all the CAPRI specific initialization
//------------------------------------------------------------------------------
hal_ret_t
hal::pd::asic_init (asic_cfg_t *cfg)
{
    capri_cfg_t    capri_cfg;

    HAL_ASSERT(cfg != NULL);
    capri_cfg.loader_info_file = cfg->loader_info_file;
    capri_cfg.default_config_dir = cfg->default_config_dir;
    capri_cfg.cfg_path = cfg->cfg_path;
    capri_cfg.admin_cos = cfg->admin_cos;
    capri_cfg.repl_entry_width = cfg->repl_entry_width;
    capri_cfg.pgm_name = cfg->pgm_name;
    capri_cfg.catalog = cfg->catalog;
    capri_cfg.p4_cache = true;
    capri_cfg.p4plus_cache = true;
    capri_cfg.llc_cache = true;
    return capri_init(&capri_cfg);
}

static hal_ret_t
capri_timer_hbm_init (void)
{
    hal_ret_t ret = HAL_RET_OK;
    uint64_t timer_key_hbm_base_addr;
    uint64_t timer_key_hbm_addr;
    uint64_t zero_data[8] = { 0 };

    timer_key_hbm_base_addr = (uint64_t)get_start_offset((char *)JTIMERS);
    HAL_TRACE_DEBUG("HBM timer key base addr {:#x}", timer_key_hbm_base_addr);
    timer_key_hbm_addr = timer_key_hbm_base_addr;
    while (timer_key_hbm_addr < timer_key_hbm_base_addr +
                                CAPRI_TIMER_HBM_KEY_SPACE) {
        capri_hbm_write_mem(timer_key_hbm_addr, (uint8_t *)zero_data, sizeof(zero_data));
        timer_key_hbm_addr += sizeof(zero_data);
    }

    return ret;
}

static hal_ret_t
capri_p4_asm_init (capri_cfg_t *cfg)
{
    hal_ret_t      ret = HAL_RET_OK;
    uint64_t       p4_prm_base_addr;
    std::string    full_path;

    full_path =  std::string(cfg->cfg_path) + "/" + cfg->pgm_name + "/" + "asm_bin";
    HAL_TRACE_DEBUG("P4 ASM Binaries dir: {}", full_path.c_str());

    // Check if directory is present
    if (access(full_path.c_str(), R_OK) < 0) {
        HAL_TRACE_ERR("{} not_present/no_read_permissions", full_path.c_str());
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    p4_prm_base_addr = (uint64_t)get_start_offset((char *)JP4_PRGM);
    HAL_TRACE_DEBUG("base addr {:#x}", p4_prm_base_addr);
    capri_load_mpu_programs(cfg->pgm_name.c_str(), (char *)full_path.c_str(),
                            p4_prm_base_addr, NULL, 0, NULL);

    return ret;
}

static hal_ret_t
capri_p4_pgm_init (capri_cfg_t *cfg)
{
    hal_ret_t      ret;
    std::string    full_path;

    full_path =  std::string(cfg->cfg_path) + "/" + cfg->pgm_name + "/" + "pgm_bin";
    HAL_TRACE_DEBUG("PGM Binaries dir: {}", full_path.c_str());

    // check if directory is present
    if (access(full_path.c_str(), R_OK) < 0) {
        HAL_TRACE_ERR("{} not present/no read permissions", full_path.c_str());
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }
    ret = capri_load_config((char *)full_path.c_str());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to load config for {}", full_path);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
capri_p4p_asm_init (capri_cfg_t *cfg, hal::hal_cfg_t *hal_cfg)
{
    hal_ret_t                  ret = HAL_RET_OK;
    uint64_t                   p4plus_prm_base_addr;
    uint64_t                   offset;
    std::string                full_path;
    capri_prog_param_info_t    *symbols;
    int                        i = 0;

    full_path =  std::string(cfg->cfg_path) + "/" + cfg->pgm_name + "/p4plus_bin";
    std::cerr << "full path " << full_path << std::endl;
    HAL_TRACE_DEBUG("P4+ ASM Binaries dir: {}", full_path.c_str());

    // Check if directory is present
    if (access(full_path.c_str(), R_OK) < 0) {
        HAL_TRACE_ERR("{} not_present/no_read_permissions", full_path.c_str());
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    capri_coreclk_freq = (uint32_t)(capri_get_coreclk_freq(hal_cfg) / 1000000);

    HAL_TRACE_DEBUG("Capri core clock freq is {} Mhz", capri_coreclk_freq);

    symbols = (capri_prog_param_info_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_PD,
                        P4PLUS_SYMBOLS_MAX * sizeof(capri_prog_param_info_t));
    symbols[i].name = "tcp-read-rnmdr-alloc-idx.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_RX);
    symbols[i].params[1].name = TCP_PROXY_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tcp-read-rnmpr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    i++;

    symbols[i].name = "tcp-read-sesq-ci.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TCP_PROXY_STATS;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tcp-read-sesq-retx-ci.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TCP_PROXY_STATS;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tcp-clean-retx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TCP_PROXY_STATS;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tls-enc-read-tnmdr-alloc-idx.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_TX);
    symbols[i].params[1].name = TLS_PROXY_GLOBAL_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_GLOBAL_STATS);
    i++;

    symbols[i].name = "tls-enc-read-tnmpr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "tls-enc-queue-brq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    i++;

    symbols[i].name = "tls-dec-queue-brq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_GCM1);
    i++;

    symbols[i].name = "tls-enc-read-rnmdr-free-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "tls-enc-read-rnmpr-free-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_PAD_BYTES_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_PAD_TABLE);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_rxmda_ring_full_error.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table2.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_ring_full_error.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_update_input_desc_aol2.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_update_input_desc_aol.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_txdma2_load_in_desc.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_rxmda_ring_full_error.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_write_barco_req.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_build_decap_packet.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_txdma2_load_ipsec_int.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_txdma_initial_table.bin";
    symbols[i].num_params = 3;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                BARCO_GCM1_PI_HBM_TABLE_OFFSET;
    symbols[i].params[2].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_update_input_desc_aol.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore.bin";
    symbols[i].num_params = 5;
    symbols[i].params[0].name = IPSEC_RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_NMDR_RX);
    symbols[i].params[1].name = IPSEC_RNMPR_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_NMPR_RX);
    symbols[i].params[2].name = IPSEC_TNMDR_TABLE_BASE;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_IPSEC_NMDR_TX);
    symbols[i].params[3].name = IPSEC_TNMPR_TABLE_BASE;
    symbols[i].params[3].val = get_start_offset(CAPRI_HBM_REG_IPSEC_NMPR_TX);
    symbols[i].params[4].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[4].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore.bin";
    symbols[i].num_params = 5;
    symbols[i].params[0].name = IPSEC_RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[i].params[1].name = IPSEC_TNMDR_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    symbols[i].params[2].name = IPSEC_RNMPR_TABLE_BASE;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    symbols[i].params[3].name = IPSEC_TNMPR_TABLE_BASE;
    symbols[i].params[3].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_TX);
    symbols[i].params[4].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[4].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_txdma2_initial_table.bin";
    symbols[i].num_params = 3;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_GCM1);
    symbols[i].params[1].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM1_PI_HBM_TABLE_OFFSET;
    symbols[i].params[2].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_n2h_allocate_barco_req_pindex.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_GCM1);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS) + 512;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table.bin";
    symbols[i].num_params = 3;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[i].params[1].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    symbols[i].params[2].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);

    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;


    symbols[i].name = "cpu_read_desc_pindex.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_RX);
    i++;

    symbols[i].name = "cpu_read_page_pindex.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    i++;

    symbols[i].name = "cpu_tx_read_asq_descr.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = CPU_TX_DOT1Q_HDR_OFFSET;
    symbols[i].params[0].val = sizeof(hal::pd::cpu_to_p4plus_header_t) + \
        sizeof(hal::pd::p4plus_to_p4_header_t) + L2HDR_DOT1Q_OFFSET;
    i++;

    symbols[i].name = "tls-dec-read-tnmdr-alloc-idx.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_TX);
    symbols[i].params[1].name = TLS_PROXY_GLOBAL_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_GLOBAL_STATS);
    i++;

    symbols[i].name = "tls-dec-read-tnmpr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = IPSEC_CB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "tls-dec-read-rnmdr-free-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "tls-dec-read-rnmpr-free-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "tls-dec-bld-barco-req.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = ARQRX_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    i++;

    if ((offset = get_start_offset("flow_hash")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = "ipfix.bin";
        symbols[i].params[0].name = P4_FLOW_HASH_BASE;
        symbols[i].params[0].val = offset;
        if ((offset = get_start_offset("flow_hash_overflow")) != INVALID_MEM_ADDRESS) {
            symbols[i].num_params = 2;
            symbols[i].params[1].name = P4_FLOW_HASH_OVERFLOW_BASE;
            symbols[i].params[1].val = offset;
        } else {
            symbols[i].num_params = 1;
        }
        i++;
    }

    if ((offset = get_start_offset("flow_info")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = "ipfix_flow_hash.bin";
        symbols[i].num_params = 1;
        symbols[i].params[0].name = P4_FLOW_INFO_BASE;
        symbols[i].params[0].val = offset;
        i++;
    }

    if ((offset = get_start_offset("session_state")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = "ipfix_flow_info.bin";
        symbols[i].num_params = 1;
        symbols[i].params[0].name = P4_SESSION_STATE_BASE;
        symbols[i].params[0].val = offset;
        i++;
    }

    if ((offset = get_start_offset("flow_stats")) != INVALID_MEM_ADDRESS) {
        symbols[i].name = "ipfix_session_state.bin";
        symbols[i].num_params = 2;
        symbols[i].params[0].name = P4_FLOW_STATS_BASE;
        symbols[i].params[0].val = offset;
        symbols[i].params[1].name = P4_FLOW_ATOMIC_STATS_BASE;
        symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_P4_ATOMIC_STATS);
        i++;
    }

    symbols[i].name = "tcp-tx-read-gc-nmdr-idx.bin";
    symbols[i].num_params = 3;
    symbols[i].params[0].name = RNMDR_GC_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX_GC);
    symbols[i].params[1].name = TNMDR_GC_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX_GC);
    symbols[i].params[2].name = TCP_PROXY_STATS;
    symbols[i].params[2].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    i++;

    symbols[i].name = "gc_tx_inc_descr_free_pair_pi.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[i].params[1].name = TNMDPR_BIG_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TCP/TLS global resources.
     */
    symbols[i].name = "gc_tx_dummy.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = GC_GLOBAL_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                    CAPRI_GC_GLOBAL_TABLE;
    i++;

    symbols[i].name = "tcp-l7-read-rnmdr-alloc-idx.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_RX);
    i++;

    symbols[i].name = "tls-dec-read-l7-rnmdr-pidx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDPR_BIG_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDPR_BIG_RX);
    i++;

    symbols[i].name = "tls-enc-queue-brq-mpp.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP1);
    i++;

    symbols[i].name = "tls-dec-queue-brq-mpp.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP1);
    i++;

    symbols[i].name = "resp_rx_eqcb_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);
    i++;

    symbols[i].name = "req_rx_eqcb_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);
    i++;

    symbols[i].name = "rdma_cq_rx_eqcb_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);
    i++;

    symbols[i].name = "resp_rx_rqcb_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RDMA_ATOMIC_RESOURCE_ADDR;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR);
    i++;

    symbols[i].name = "resp_rx_atomic_resource_process.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RDMA_ATOMIC_RESOURCE_ADDR;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR);
    symbols[i].params[1].name = RDMA_PCIE_ATOMIC_BASE_ADDR;
    symbols[i].params[1].val = CAPRI_PCIE_ATOMIC_BASE_ADDR;
    i++;

    symbols[i].name = "p4pt.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_MEM);
    i++;

    symbols[i].name = "p4pt_update_tcb.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_ISCSI_REC_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_REC_MEM);
    i++;

    symbols[i].name = "p4pt_update_tcb_rec.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_ISCSI_STATS_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_STATS_MEM);
    i++;

    symbols[i].name = "p4pt_update_tcb_stats.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_MEM);
    i++;

    symbols[i].name = "p4pt_update_read_latency_distribution.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_MEM);
    i++;

    symbols[i].name = "rawr_desc_sem_post_update.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "rawr_desc_free.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "rawr_ppage_sem_post_update.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    i++;

    symbols[i].name = "rawr_mpage_sem_post_update.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "rawr_mpage_free.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[i].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "rawc_desc_free.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "rawc_page0_free.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[i].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "proxyr_mpage_sem_pindex_post_update.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "proxyr_desc_free.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "proxyr_mpage_free.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[i].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "proxyc_desc_free.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = RNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    i++;

    symbols[i].name = "proxyc_page0_free.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[i].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);
    i++;

    symbols[i].name = "tls-dec-aesgcm-newseg-queue-barco.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_GCM1);
    i++;

    symbols[i].name = "tls-dec-aesgcm-newseg-read-tnmdr-odesc-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "tls-dec-aesgcm-newseg-read-tnmpr-opage-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "tls-dec-aesgcm-newseg-read-tnmdr-idesc-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "req_tx_dcqcn_enforce_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = NUM_CLOCK_TICKS_PER_US;
    symbols[i].params[0].val = capri_coreclk_freq;
    i++;

    symbols[i].name = "resp_tx_dcqcn_enforce_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = NUM_CLOCK_TICKS_PER_US;
    symbols[i].params[0].val = capri_coreclk_freq;
    i++;


    symbols[i].name = "req_tx_timer_expiry_process.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = NUM_CLOCK_TICKS_PER_US;
    symbols[i].params[0].val = capri_coreclk_freq;
    i++;

    symbols[i].name = "cpu_hash_calc_id.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = CAPRI_CPU_HASH_MASK;
    symbols[i].params[0].val = 0x1;
    symbols[i].params[1].name = CAPRI_CPU_MAX_ARQID;
    symbols[i].params[1].val = 0x0;
    i++;

    symbols[i].name = "tls-mac-queue-brq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP2);
    i++;

    symbols[i].name = "tls-mac-queue-enc-brq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP3);
    i++;

    symbols[i].name = "tls-mac-read-idesc.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE);
    i++;

    symbols[i].name = "tls-mac-read-tnmdr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMDR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    i++;

    symbols[i].name = "tls-mac-read-tnmpr-alloc-idx.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = TNMPR_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);
    i++;

    symbols[i].name = "cpu_write_arq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = ARQRX_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    i++;

    symbols[i].name = "tcp-write-arq.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = ARQRX_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    i++;

    symbols[i].name = "rawr_hash_calc_id.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = CAPRI_CPU_HASH_MASK;
    symbols[i].params[0].val = 0x1;
    symbols[i].params[1].name = CAPRI_CPU_MAX_ARQID;
    symbols[i].params[1].val = 0x0;
    i++;

    symbols[i].name = "tls-enc-serq-consume.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;

    symbols[i].params[1].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = "tls-dec-read-header.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM1_PI_HBM_TABLE_OFFSET;

    symbols[i].params[1].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_s1_dummy.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = BRQ_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[i].params[1].name = IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_GLOBAL_DROP_STATS);
    i++;

    symbols[i].name = "tls-enc-gc-setup.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDR_TLS_GC_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX_GC) +
        CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE * CAPRI_HBM_RNMDR_ENTRY_SIZE * CAPRI_RNMDR_GC_TLS_RING_PRODUCER;
    symbols[i].params[1].name = TCP_PROXY_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tls-dec-gc-setup.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = RNMDR_TLS_GC_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX_GC) +
        CAPRI_HBM_GC_PER_PRODUCER_RING_SIZE * CAPRI_HBM_RNMDR_ENTRY_SIZE * CAPRI_RNMDR_GC_TLS_RING_PRODUCER;
    symbols[i].params[1].name = TCP_PROXY_STATS;
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TCP_PROXY_STATS);
    i++;

    symbols[i].name = "tls-enc-bsq.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM0_PI_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    symbols[i].params[1].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = "tls-dec-bsq.bin";
    symbols[i].num_params = 2;
    symbols[i].params[0].name = TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                BARCO_GCM1_PI_HBM_TABLE_OFFSET;
    symbols[i].params[1].name = TLS_PROXY_BARCO_MPP1_PI_HBM_TABLE_BASE;

    /*
     * The 'CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE' region is provisioned for 1KB out of
     * which CAPRI_MAX_TLS_PAD_SIZE is used for Pad bytes. We'll use the remaining
     * HBM memory from this region to store other TLS global resources.
     */
    symbols[i].params[1].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE) +
                                 BARCO_MPP1_PI_HBM_TABLE_OFFSET;
    i++;

    symbols[i].name = "eth_rx_stats.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = LIF_STATS_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_LIF_STATS);
    i++;

    symbols[i].name = "eth_tx_stats.bin";
    symbols[i].num_params = 1;
    symbols[i].params[0].name = LIF_STATS_BASE;
    symbols[i].params[0].val = get_start_offset(CAPRI_HBM_REG_LIF_STATS);
    i++;

    HAL_ASSERT(i <= P4PLUS_SYMBOLS_MAX);
    // Please increment CAPRI_P4PLUS_NUM_SYMBOLS when you want to add more below

    p4plus_prm_base_addr = (uint64_t)get_start_offset((char *)JP4PLUS_PRGM);
    HAL_TRACE_DEBUG("base addr {:#x}", p4plus_prm_base_addr);
    capri_load_mpu_programs("p4plus", (char *)full_path.c_str(),
                            p4plus_prm_base_addr, symbols, i, NULL);

    HAL_FREE(hal::HAL_MEM_ALLOC_PD, symbols);

    return ret;
}

static hal_ret_t
capri_hbm_regions_init (capri_cfg_t *cfg, hal::hal_cfg_t *hal_cfg)
{
    hal_ret_t           ret = HAL_RET_OK;

    ret = capri_p4_asm_init(cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = capri_p4p_asm_init(cfg, hal_cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = capri_p4_pgm_init(cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = capri_timer_hbm_init();
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // reset all the HBM regions that are marked for reset
    reset_hbm_regions();

    return ret;
}

static hal_ret_t
capri_cache_init (capri_cfg_t *cfg)
{
    hal_ret_t   ret = HAL_RET_OK;

    // Program Global parameters of the cache.
    ret = capri_hbm_cache_init(cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // Process all the regions.
    ret = capri_hbm_cache_regions_init();
    if (ret != HAL_RET_OK) {
        return ret;
    }
    return ret;
}


/*
 * ASIC teams wants to disable Error recovery of Seq ID check pRDMA, 
 * as this recovery path is not tested thoroughly, and we might be
 * runing into for an outstanding issue is suspicion.
 * Disabling from HAL for now, but Helen will commit this
 * into prd_asic_init api called from HAL, then this will be removed
 */ 
static hal_ret_t
capri_prd_init()
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pr_csr_t &pr_csr = cap0.pr.pr;

    pr_csr.prd.cfg_ctrl.read();
    pr_csr.prd.cfg_ctrl.pkt_phv_sync_err_recovery_en(0);
    pr_csr.prd.cfg_ctrl.write();
    HAL_TRACE_DEBUG("Disabled pkt_phv_sync_err_recovery_en in pr_prd_cfg_ctrl");
    return HAL_RET_OK;
}


hal_ret_t
capri_repl_init (capri_cfg_t *cfg)
{
    capri_hbm_base = get_hbm_base();
    hbm_repl_table_offset = get_hbm_offset(JP4_REPL);
    if (hbm_repl_table_offset != INVALID_MEM_ADDRESS) {
        capri_tm_repl_table_base_addr_set(hbm_repl_table_offset / CAPRI_REPL_ENTRY_WIDTH);
        capri_tm_repl_table_token_size_set(cfg->repl_entry_width * 8);
    }

    return HAL_RET_OK;
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
static hal_ret_t
capri_block_reset(capri_cfg_t *cfg)
{
    hal_ret_t    ret         = HAL_RET_OK;
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
static hal_ret_t
capri_block_init_start(capri_cfg_t *cfg)
{
    hal_ret_t    ret         = HAL_RET_OK;
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
static hal_ret_t
capri_block_init_done(capri_cfg_t *cfg)
{
    hal_ret_t    ret         = HAL_RET_OK;
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
// Init all the capri blocks owned by HAL
//------------------------------------------------------------------------------
hal_ret_t
capri_block_init(capri_cfg_t *cfg)
{
    hal_ret_t           ret = HAL_RET_OK;

    // initialize block info
    capri_block_info_init();

    // soft reset
    ret = capri_block_reset(cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // init blocks
    ret = capri_block_init_start(cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // wait for blocks to be inited
    ret = capri_block_init_done(cfg);
    if (ret != HAL_RET_OK) {
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
hal_ret_t
capri_init (capri_cfg_t *cfg = NULL)
{
    hal_ret_t    ret;

    HAL_TRACE_DEBUG("Initializing Capri");

    hal::hal_cfg_t *hal_cfg =
        (hal::hal_cfg_t *)hal::hal_get_current_thread()->data();
    HAL_ASSERT(hal_cfg);

    g_capri_state_pd = capri_state_pd::factory();
    HAL_ASSERT_RETURN((g_capri_state_pd != NULL), HAL_RET_ERR);

    g_capri_state_pd->set_cfg_path(cfg->cfg_path);
    ret = capri_hbm_parse(cfg);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    if (capri_table_rw_init(hal_cfg) != CAPRI_OK) {
        return HAL_RET_ERR;
    }

    ret = capri_hbm_regions_init(cfg, hal_cfg);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    ret = capri_block_init(cfg);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    ret = capri_cache_init(cfg);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    // do asic init before overwriting with the default configs
    ret = capri_tm_asic_init();
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    if (!cfg->catalog->qos_sw_init_enabled()) {
        ret = capri_default_config_init(cfg);
        HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    }

    ret = capri_txs_scheduler_init(cfg->admin_cos, hal_cfg);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    // Call PXB/PCIE init only in MODEL and RTL simulation
    // This will be done by PCIe manager for the actual chip
    if (hal_cfg->platform == hal::HAL_PLATFORM_SIM ||
        hal_cfg->platform == hal::HAL_PLATFORM_RTL) {
        ret = capri_pxb_pcie_init();
        HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    }

    ret = capri_tm_init(cfg->catalog);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    ret = capri_repl_init(cfg);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    ret = hal::pd::capri_sw_phv_init();
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    if (cfg && !cfg->loader_info_file.empty()) {
        capri_list_program_addr(cfg->loader_info_file.c_str());
    }

    ret = hal::pd::capri_barco_crypto_init();
    if (ret != HAL_RET_OK) {
        // GFT: Always fails here.
        HAL_TRACE_ERR("Failed to inic barco_crypto. err: {}", ret);
        return ret;
    }

    ret = hal::pd::capri_quiesce_init();
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    ret = capri_prd_init();
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    hal::svc::set_hal_status(hal::HAL_STATUS_ASIC_INIT_DONE);

    return ret;
}
