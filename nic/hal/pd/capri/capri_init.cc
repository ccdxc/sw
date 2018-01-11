#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/pd/capri/capri.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_config.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/include/asic_pd.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/hal/pd/capri/capri_pxb_pcie.hpp"

#define CAPRI_P4PLUS_NUM_SYMBOLS 84

/* capri_default_config_init
 * Load any bin files needed for initializing default configs
 */
static hal_ret_t
capri_default_config_init (void)
{
    hal_ret_t   ret = HAL_RET_OK;
    char        *cfg_path;
    std::string full_path;
    int         num_phases = 2;
    int         i;

    cfg_path = getenv("HAL_CONFIG_PATH");
    if (!cfg_path) {
        HAL_TRACE_ERR("Please set HAL_CONFIG_PATH env. variable");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    for (i = 0; i < num_phases; i++) {
        full_path =  std::string(cfg_path) + "/" + "init_" + 
                                        std::to_string(i) + "_bin";

        // Check if directory is present
        if (access(full_path.c_str(), R_OK) < 0) {
            HAL_TRACE_DEBUG("Skipping init binaries");
            return HAL_RET_OK;
        }

        HAL_TRACE_DEBUG("Init phase {} Binaries dir: {}", i, full_path.c_str());

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
    hal_ret_t ret = HAL_RET_OK;
    
    HAL_TRACE_DEBUG("Capri Init ");

    ret = capri_hbm_parse();

    ret = capri_hbm_regions_init();
    
    if (capri_table_rw_init()) {
        return HAL_RET_ERR;
    }
 
    // Do asic init before overwriting with the default configs
    if (ret == HAL_RET_OK) {
        ret = capri_tm_asic_init();
    }

    if (ret == HAL_RET_OK) {
        ret = capri_default_config_init();
    }

    if (ret == HAL_RET_OK) {
        ret = capri_txs_scheduler_init();
    }
 
    if (ret == HAL_RET_OK) {
        ret = capri_pxb_pcie_init();
    }

    if (ret == HAL_RET_OK) {
        ret = capri_tm_init();
    }

    if (ret == HAL_RET_OK) {
        ret = capri_repl_init();
    }

   if (cfg && !cfg->loader_info_file.empty()) {
        capri_list_program_addr(cfg->loader_info_file.c_str());
    }


    return ret;
}

//------------------------------------------------------------------------------
// perform all the CAPRI specific initialization
//------------------------------------------------------------------------------
hal_ret_t
hal::pd::asic_init (hal::pd::asic_cfg_t *cfg = NULL)
{
    capri_cfg_t capri_cfg;
    capri_cfg.loader_info_file = cfg->loader_info_file;
    return capri_init(&capri_cfg);
}

static hal_ret_t
capri_timer_hbm_init(void)
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

hal_ret_t
capri_hbm_regions_init()
{
    hal_ret_t ret = HAL_RET_OK;

    ret = capri_p4_asm_init();
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = capri_p4p_asm_init();
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = capri_p4_pgm_init();
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = capri_timer_hbm_init();
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

hal_ret_t
capri_p4_asm_init()
{
    hal_ret_t               ret = HAL_RET_OK;
    uint64_t                p4_prm_base_addr;
    char                    *cfg_path;
    std::string             full_path;

    cfg_path = getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + "asm_bin";
    } else {
        HAL_TRACE_ERR("Please set HAL_CONFIG_PATH env. variable");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }
    HAL_TRACE_DEBUG("P4 ASM Binaries dir: {}", full_path.c_str());

    // Check if directory is present
    if (access(full_path.c_str(), R_OK) < 0) {
        HAL_TRACE_ERR("{} not_present/no_read_permissions", full_path.c_str());
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    p4_prm_base_addr = (uint64_t)get_start_offset((char *)JP4_PRGM);
    HAL_TRACE_DEBUG("base addr {:#x}", p4_prm_base_addr);
    capri_load_mpu_programs("iris", (char *)full_path.c_str(),
                            p4_prm_base_addr, NULL, 0);

    return ret;
}

hal_ret_t
capri_p4_pgm_init()
{
    hal_ret_t               ret = HAL_RET_OK;
 char               *cfg_path;
 std::string        full_path;

    cfg_path = getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + "pgm_bin";
    } else {
        HAL_TRACE_ERR("Please set HAL_CONFIG_PATH env. variable");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }
    HAL_TRACE_DEBUG("PGM Binaries dir: {}", full_path.c_str());

    // Check if directory is present
    if (access(full_path.c_str(), R_OK) < 0) {
        HAL_TRACE_ERR("{} not_present/no_read_permissions", full_path.c_str());
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    ret = capri_load_config((char *)full_path.c_str());

    return ret;
}

hal_ret_t
capri_p4p_asm_init()
{
    hal_ret_t                           ret = HAL_RET_OK;
    uint64_t                            p4plus_prm_base_addr;
    char                                *cfg_path;
    std::string                         full_path;
    capri_prog_param_info_t             *symbols;

    cfg_path = getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + "p4plus_bin";
        std::cerr << "full path " << full_path << std::endl;
    } else {
        full_path = std::string("p4plus_bin");
    }
    HAL_TRACE_DEBUG("P4+ ASM Binaries dir: {}", full_path.c_str());

    // Check if directory is present
    if (access(full_path.c_str(), R_OK) < 0) {
        HAL_TRACE_ERR("{} not_present/no_read_permissions", full_path.c_str());
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    symbols = (capri_prog_param_info_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_PD,
                        CAPRI_P4PLUS_NUM_SYMBOLS * sizeof(capri_prog_param_info_t));
    symbols[0].name = "tcp-read-rnmdr-alloc-idx.bin";
    symbols[0].num_params = 1;
    symbols[0].params[0].name = RNMDR_TABLE_BASE;
    symbols[0].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[1].name = "tcp-read-rnmpr-alloc-idx.bin";
    symbols[1].num_params = 1;
    symbols[1].params[0].name = RNMPR_TABLE_BASE;
    symbols[1].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[2].name = "tls-enc-read-tnmdr-alloc-idx.bin";
    symbols[2].num_params = 1;
    symbols[2].params[0].name = TNMDR_TABLE_BASE;
    symbols[2].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    symbols[3].name = "tls-enc-read-tnmpr-alloc-idx.bin";
    symbols[3].num_params = 1;
    symbols[3].params[0].name = TNMPR_TABLE_BASE;
    symbols[3].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[4].name = "tls-enc-queue-brq.bin";
    symbols[4].num_params = 1;
    symbols[4].params[0].name = BRQ_BASE;
    symbols[4].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);

    symbols[5].name = "tls-dec-queue-brq.bin";
    symbols[5].num_params = 1;
    symbols[5].params[0].name = BRQ_BASE;
    symbols[5].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);

    symbols[6].name = "tls-enc-read-rnmdr-free-idx.bin";
    symbols[6].num_params = 1;
    symbols[6].params[0].name = RNMDR_TABLE_BASE;
    symbols[6].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    
    symbols[7].name = "tls-enc-read-rnmpr-free-idx.bin";
    symbols[7].num_params = 1;
    symbols[7].params[0].name = RNMPR_TABLE_BASE;
    symbols[7].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[8].name = "esp_ipv4_tunnel_h2n_ipsec_encap_rxdma_initial_table.bin";
    symbols[8].num_params = 1;
    symbols[8].params[0].name = IPSEC_PAD_BYTES_HBM_TABLE_BASE;
    symbols[8].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSEC_PAD_TABLE);

    symbols[9].name = "esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet.bin";
    symbols[9].num_params = 2;
    symbols[9].params[0].name = IPSEC_CB_BASE;
    symbols[9].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);
    symbols[9].params[1].name = IPSEC_IP_HDR_BASE;
    symbols[9].params[1].val = get_start_offset(CAPRI_HBM_REG_IPSEC_IP_HDR);

    symbols[10].name = "esp_ipv4_tunnel_h2n_update_input_desc_aol.bin";
    symbols[10].num_params = 1;
    symbols[10].params[0].name = IPSEC_CB_BASE;
    symbols[10].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);

    symbols[11].name = "esp_ipv4_tunnel_n2h_txdma2_load_in_desc.bin";
    symbols[11].num_params = 1;
    symbols[11].params[0].name = IPSEC_CB_BASE;
    symbols[11].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);

    symbols[12].name = "esp_ipv4_tunnel_n2h_txdma_initial_table.bin";
    symbols[12].num_params = 1;
    symbols[12].params[0].name = IPSEC_CB_BASE;
    symbols[12].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);

    symbols[13].name = "esp_ipv4_tunnel_n2h_update_input_desc_aol.bin";
    symbols[13].num_params = 1;
    symbols[13].params[0].name = IPSEC_CB_BASE;
    symbols[13].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);

    symbols[14].name = "esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore.bin";
    symbols[14].num_params = 1;
    symbols[14].params[0].name = RNMDR_TABLE_BASE;
    symbols[14].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[15].name = "esp_ipv4_tunnel_h2n_allocate_input_page_semaphore.bin";
    symbols[15].num_params = 1;
    symbols[15].params[0].name = RNMPR_TABLE_BASE;
    symbols[15].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[16].name = "esp_ipv4_tunnel_h2n_allocate_output_desc_semaphore.bin";
    symbols[16].num_params = 1;
    symbols[16].params[0].name = TNMDR_TABLE_BASE;
    symbols[16].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    symbols[17].name = "esp_ipv4_tunnel_h2n_allocate_output_page_semaphore.bin";
    symbols[17].num_params = 1;
    symbols[17].params[0].name = TNMPR_TABLE_BASE;
    symbols[17].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[18].name = "esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore.bin";
    symbols[18].num_params = 1;
    symbols[18].params[0].name = RNMDR_TABLE_BASE;
    symbols[18].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[19].name = "esp_ipv4_tunnel_n2h_allocate_input_page_semaphore.bin";
    symbols[19].num_params = 1;
    symbols[19].params[0].name = RNMPR_TABLE_BASE;
    symbols[19].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[20].name = "esp_ipv4_tunnel_n2h_allocate_output_desc_semaphore.bin";
    symbols[20].num_params = 1;
    symbols[20].params[0].name = TNMDR_TABLE_BASE;
    symbols[20].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    symbols[21].name = "esp_ipv4_tunnel_n2h_allocate_output_page_semaphore.bin";
    symbols[21].num_params = 1;
    symbols[21].params[0].name = TNMPR_TABLE_BASE;
    symbols[21].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[22].name = "esp_ipv4_tunnel_n2h_txdma2_initial_table.bin";
    symbols[22].num_params = 1;
    symbols[22].params[0].name = BRQ_BASE;
    symbols[22].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[23].name = "esp_ipv4_tunnel_n2h_allocate_barco_req_pindex.bin";
    symbols[23].num_params = 1;
    symbols[23].params[0].name = BRQ_BASE;
    symbols[23].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[24].name = "esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table.bin";
    symbols[24].num_params = 1;
    symbols[24].params[0].name = BRQ_BASE;
    symbols[24].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    symbols[25].name = "esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex.bin";
    symbols[25].num_params = 1;
    symbols[25].params[0].name = BRQ_BASE;
    symbols[25].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);
    
    symbols[26].name = "cpu_read_desc_pindex.bin";
    symbols[26].num_params = 1;
    symbols[26].params[0].name = RNMDR_TABLE_BASE;
    symbols[26].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    
    symbols[27].name = "cpu_read_page_pindex.bin";
    symbols[27].num_params = 1;
    symbols[27].params[0].name = RNMPR_TABLE_BASE;
    symbols[27].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);

    symbols[28].name = "cpu_read_arqrx_pindex.bin";
    symbols[28].num_params = 1;
    symbols[28].params[0].name = ARQRX_QIDXR_BASE;
    symbols[28].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);

    symbols[29].name = "tls-dec-read-tnmdr-alloc-idx.bin";
    symbols[29].num_params = 1;
    symbols[29].params[0].name = TNMDR_TABLE_BASE;
    symbols[29].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);

    symbols[30].name = "tls-dec-read-tnmpr-alloc-idx.bin";
    symbols[30].num_params = 1;
    symbols[30].params[0].name = TNMPR_TABLE_BASE;
    symbols[30].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[31].name = "esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc.bin";
    symbols[31].num_params = 1;
    symbols[31].params[0].name = IPSEC_CB_BASE;
    symbols[31].params[0].val = get_start_offset(CAPRI_HBM_REG_IPSECCB);

    symbols[32].name = "tcp-fc.bin";
    symbols[32].num_params = 1;
    symbols[32].params[0].name = ARQRX_QIDXR_BASE;
    symbols[32].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);

    symbols[33].name = "tls-dec-read-rnmdr-free-idx.bin";
    symbols[33].num_params = 1;
    symbols[33].params[0].name = RNMDR_TABLE_BASE;
    symbols[33].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);

    symbols[34].name = "tls-dec-read-rnmpr-free-idx.bin";
    symbols[34].num_params = 1;
    symbols[34].params[0].name = RNMPR_TABLE_BASE;
    symbols[34].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[35].name = "tls-dec-bld-barco-req.bin";
    symbols[35].num_params = 2;
    symbols[35].params[0].name = ARQTX_BASE;
    symbols[35].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQTX);
    symbols[35].params[1].name = ARQRX_QIDXR_BASE;
    symbols[35].params[1].val = get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);

    symbols[36].name = "ipfix.bin";
    symbols[36].num_params = 1;
    symbols[36].params[0].name = P4_FLOW_HASH_BASE;
    symbols[36].params[0].val =
        get_start_offset(p4pd_tbl_names[P4TBL_ID_FLOW_HASH]);

    symbols[37].name = "ipfix_flow_hash.bin";
    symbols[37].num_params = 1;
    symbols[37].params[0].name = P4_FLOW_INFO_BASE;
    symbols[37].params[0].val =
        get_start_offset(p4pd_tbl_names[P4TBL_ID_FLOW_INFO]);

    symbols[38].name = "ipfix_flow_info.bin";
    symbols[38].num_params = 1;
    symbols[38].params[0].name = P4_SESSION_STATE_BASE;
    symbols[38].params[0].val =
        get_start_offset(p4pd_tbl_names[P4TBL_ID_SESSION_STATE]);

    symbols[39].name = "ipfix_session_state.bin";
    symbols[39].num_params = 2;
    symbols[39].params[0].name = P4_FLOW_STATS_BASE;
    symbols[39].params[0].val =
        get_start_offset(p4pd_tbl_names[P4TBL_ID_FLOW_STATS]);
    symbols[39].params[1].name = P4_FLOW_ATOMIC_STATS_BASE;
    symbols[39].params[1].val = get_start_offset(JP4_ATOMIC_STATS);

    symbols[40].name = "tcp-tx-read-gc-nmdr-idx.bin";
    symbols[40].num_params = 2;
    symbols[40].params[0].name = RNMDR_GC_TABLE_BASE;
    symbols[40].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX_GC);
    symbols[40].params[1].name = TNMDR_GC_TABLE_BASE;
    symbols[40].params[1].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX_GC);

    symbols[41].name = "gc_tx_inc_descr_free_pair_pi.bin";
    symbols[41].num_params = 2;
    symbols[41].params[0].name = RNMDR_TABLE_BASE;
    symbols[41].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[41].params[1].name = TNMDR_TABLE_BASE;
    symbols[41].params[1].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);

    symbols[42].name = "gc_tx_inc_page_free_pair_pi.bin";
    symbols[42].num_params = 2;
    symbols[42].params[0].name = RNMPR_TABLE_BASE;
    symbols[42].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[42].params[1].name = TNMPR_TABLE_BASE;
    symbols[42].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[43].name = "tcp-l7-read-rnmdr-alloc-idx.bin";
    symbols[43].num_params = 1;
    symbols[43].params[0].name = RNMDR_TABLE_BASE;
    symbols[43].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);

    symbols[44].name = "tls-dec-read-l7-rnmdr-pidx.bin";
    symbols[44].num_params = 1;
    symbols[44].params[0].name = RNMDR_TABLE_BASE;
    symbols[44].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);

    // TODO: This is a placeholder. Replace this with appropriate value based on 
    // clock frequency.
    symbols[45].name = "resp_rx_dcqcn_ecn_process.bin";
    symbols[45].num_params = 1;
    symbols[45].params[0].name = NUM_CLOCK_TICKS_PER_CNP;
    symbols[45].params[0].val = 50000; 

    // TODO: This is a placeholder. Replace this with appropriate value based on 
    // clock frequency.
    symbols[46].name = "req_rx_dcqcn_ecn_process.bin";
    symbols[46].num_params = 1;
    symbols[46].params[0].name = NUM_CLOCK_TICKS_PER_CNP;
    symbols[46].params[0].val = 50000;

    symbols[47].name = "tls-enc-queue-brq-mpp.bin";
    symbols[47].num_params = 1;
    symbols[47].params[0].name = BRQ_BASE;
    symbols[47].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP1);

    symbols[48].name = "tls-dec-queue-brq-mpp.bin";
    symbols[48].num_params = 1;
    symbols[48].params[0].name = BRQ_BASE;
    symbols[48].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP1);

    symbols[49].name = "resp_rx_eqcb_process.bin";
    symbols[49].num_params = 1;
    symbols[49].params[0].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[49].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);

    symbols[50].name = "req_rx_eqcb_process.bin";
    symbols[50].num_params = 1;
    symbols[50].params[0].name = RDMA_EQ_INTR_TABLE_BASE;
    symbols[50].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_EQ_INTR_TABLE);

    symbols[51].name = "resp_rx_rqcb_process.bin";
    symbols[51].num_params = 1;
    symbols[51].params[0].name = RDMA_ATOMIC_RESOURCE_ADDR;
    symbols[51].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR);

    symbols[52].name = "resp_rx_atomic_resource_process.bin";
    symbols[52].num_params = 2;
    symbols[52].params[0].name = RDMA_ATOMIC_RESOURCE_ADDR;
    symbols[52].params[0].val = get_start_offset(CAPRI_HBM_REG_RDMA_ATOMIC_RESOURCE_ADDR);
    symbols[52].params[1].name = RDMA_PCIE_ATOMIC_BASE_ADDR;
    symbols[52].params[1].val = CAPRI_PCIE_ATOMIC_BASE_ADDR;

    symbols[53].name = "p4pt.bin";
    symbols[53].num_params = 1;
    symbols[53].params[0].name = P4PT_TCB_BASE;
    symbols[53].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_MEM);

    symbols[54].name = "p4pt_update_tcb.bin";
    symbols[54].num_params = 1;
    symbols[54].params[0].name = P4PT_TCB_ISCSI_REC_BASE;
    symbols[54].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_REC_MEM);

    symbols[55].name = "p4pt_update_tcb_rec.bin";
    symbols[55].num_params = 1;
    symbols[55].params[0].name = P4PT_TCB_ISCSI_STATS_BASE;
    symbols[55].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_STATS_MEM);

    symbols[56].name = "p4pt_update_tcb_stats.bin";
    symbols[56].num_params = 1;
    symbols[56].params[0].name = P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_BASE;
    symbols[56].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_READ_LATENCY_DISTRIBUTION_MEM);

    symbols[57].name = "p4pt_update_read_latency_distribution.bin";
    symbols[57].num_params = 1;
    symbols[57].params[0].name = P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_BASE;
    symbols[57].params[0].val = get_start_offset(CAPRI_HBM_REG_P4PT_TCB_ISCSI_WRITE_LATENCY_DISTRIBUTION_MEM);

    symbols[58].name = "rawr_desc_sem_post_update.bin";
    symbols[58].num_params = 1;
    symbols[58].params[0].name = RNMDR_TABLE_BASE;
    symbols[58].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);

    symbols[59].name = "rawr_desc_free.bin";
    symbols[59].num_params = 1;
    symbols[59].params[0].name = RNMDR_TABLE_BASE;
    symbols[59].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);

    symbols[60].name = "rawr_ppage_sem_post_update.bin";
    symbols[60].num_params = 1;
    symbols[60].params[0].name = RNMPR_TABLE_BASE;
    symbols[60].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);

    symbols[61].name = "rawr_mpage_sem_post_update.bin";
    symbols[61].num_params = 1;
    symbols[61].params[0].name = RNMPR_SMALL_TABLE_BASE;
    symbols[61].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);

    symbols[62].name = "rawr_mpage_free.bin";
    symbols[62].num_params = 2;
    symbols[62].params[0].name = RNMPR_TABLE_BASE;
    symbols[62].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[62].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[62].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);

    symbols[63].name = "rawc_desc_free.bin";
    symbols[63].num_params = 1;
    symbols[63].params[0].name = RNMDR_TABLE_BASE;
    symbols[63].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);

    symbols[64].name = "rawc_page0_free.bin";
    symbols[64].num_params = 2;
    symbols[64].params[0].name = RNMPR_TABLE_BASE;
    symbols[64].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[64].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[64].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);

    symbols[65].name = "proxyr_mpage_sem_pindex_post_update.bin";
    symbols[65].num_params = 1;
    symbols[65].params[0].name = RNMPR_SMALL_TABLE_BASE;
    symbols[65].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);

    symbols[66].name = "proxyr_desc_free.bin";
    symbols[66].num_params = 1;
    symbols[66].params[0].name = RNMDR_TABLE_BASE;
    symbols[66].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);

    symbols[67].name = "proxyr_mpage_free.bin";
    symbols[67].num_params = 2;
    symbols[67].params[0].name = RNMPR_TABLE_BASE;
    symbols[67].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[67].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[67].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);

    symbols[68].name = "proxyc_desc_free.bin";
    symbols[68].num_params = 1;
    symbols[68].params[0].name = RNMDR_TABLE_BASE;
    symbols[68].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);

    symbols[69].name = "proxyc_page0_free.bin";
    symbols[69].num_params = 2;
    symbols[69].params[0].name = RNMPR_TABLE_BASE;
    symbols[69].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[69].params[1].name = RNMPR_SMALL_TABLE_BASE;
    symbols[69].params[1].val = get_start_offset(CAPRI_HBM_REG_NMPR_SMALL_RX);

    symbols[70].name = "tls-dec-aesgcm-newseg-queue-barco.bin";
    symbols[70].num_params = 1;
    symbols[70].params[0].name = BRQ_BASE;
    symbols[70].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);

    symbols[71].name = "tls-dec-aesgcm-newseg-read-tnmdr-odesc-alloc-idx.bin";
    symbols[71].num_params = 1;
    symbols[71].params[0].name = TNMDR_TABLE_BASE;
    symbols[71].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);

    symbols[72].name = "tls-dec-aesgcm-newseg-read-tnmpr-opage-alloc-idx.bin";
    symbols[72].num_params = 1;
    symbols[72].params[0].name = TNMPR_TABLE_BASE;
    symbols[72].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[73].name = "tls-dec-aesgcm-newseg-read-tnmdr-idesc-alloc-idx.bin";
    symbols[73].num_params = 1;
    symbols[73].params[0].name = TNMDR_TABLE_BASE;
    symbols[73].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);

    // Replace this with appropriate value based on 
    // clock frequency.
    symbols[74].name = "req_tx_dcqcn_enforce_process.bin";
    symbols[74].num_params = 1;
    symbols[74].params[0].name = NUM_CLOCK_TICKS_PER_US;
    symbols[74].params[0].val = 1000;

    // Replace this with appropriate value based on 
    // clock frequency.
    symbols[75].name = "resp_tx_dcqcn_enforce_process.bin";
    symbols[75].num_params = 1;
    symbols[75].params[0].name = NUM_CLOCK_TICKS_PER_US;
    symbols[75].params[0].val = 1000;
    
    symbols[76].name = "rx_table_cpu_hash.bin";
    symbols[76].num_params = 2;
    symbols[76].params[0].name = CAPRI_CPU_HASH_MASK;
    symbols[76].params[0].val = 0x1;
    symbols[76].params[1].name = CAPRI_CPU_MAX_ARQID;
    symbols[76].params[1].val = 0x0; 

    symbols[77].name = "tls-mac-queue-brq.bin";
    symbols[77].num_params = 1;
    symbols[77].params[0].name = BRQ_BASE;
    symbols[77].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP2);

    symbols[78].name = "tls-mac-queue-enc-brq.bin";
    symbols[78].num_params = 1;
    symbols[78].params[0].name = BRQ_BASE;
    symbols[78].params[0].val = get_start_offset(CAPRI_HBM_REG_BARCO_RING_MPP3);

    symbols[79].name = "tls-mac-read-idesc.bin";
    symbols[79].num_params = 1;
    symbols[79].params[0].name = TLS_PROXY_PAD_BYTES_HBM_TABLE_BASE;
    symbols[79].params[0].val = get_start_offset(CAPRI_HBM_REG_TLS_PROXY_PAD_TABLE);

    symbols[80].name = "tls-mac-read-tnmdr-alloc-idx.bin";
    symbols[80].num_params = 1;
    symbols[80].params[0].name = TNMDR_TABLE_BASE;
    symbols[80].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);

    symbols[81].name = "tls-mac-read-tnmpr-alloc-idx.bin";
    symbols[81].num_params = 1;
    symbols[81].params[0].name = TNMPR_TABLE_BASE;
    symbols[81].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[82].name = "cpu_write_arq.bin";
    symbols[82].num_params = 1;
    symbols[82].params[0].name = ARQRX_BASE;
    symbols[82].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);

    symbols[83].name = "tcp-write-arq.bin";
    symbols[83].num_params = 1;
    symbols[83].params[0].name = ARQRX_BASE;
    symbols[83].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);

    // Please increment CAPRI_P4PLUS_NUM_SYMBOLS when you want to add more below

    p4plus_prm_base_addr = (uint64_t)get_start_offset((char *)JP4PLUS_PRGM);
    HAL_TRACE_DEBUG("base addr {:#x}", p4plus_prm_base_addr);
    capri_load_mpu_programs("p4plus", (char *)full_path.c_str(),
                            p4plus_prm_base_addr, symbols, CAPRI_P4PLUS_NUM_SYMBOLS);

    HAL_FREE(hal::HAL_MEM_ALLOC_PD, symbols);

    return ret;
}
