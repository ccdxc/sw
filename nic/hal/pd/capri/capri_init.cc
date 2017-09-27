#include "nic/include/base.h"
#include "nic/hal/pd/capri/capri.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_config.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"

#define CAPRI_P4PLUS_NUM_SYMBOLS 36

//------------------------------------------------------------------------------
// perform all the CAPRI specific initialization
// - link all the P4 programs, by resolving symbols, labels etc.
// - load the P4/P4+ programs in HBM
// - do all the parser/deparser related register programming
// - do all the table configuration related register programming
//------------------------------------------------------------------------------
hal_ret_t
capri_init (void)
{
    hal_ret_t ret = HAL_RET_OK;
    
    HAL_TRACE_DEBUG("Capri Init ");

    ret = capri_hbm_parse();

    ret = capri_hbm_regions_init();

    if (capri_table_rw_init()) {
        return HAL_RET_ERR;
    }

    if (ret == HAL_RET_OK) {
        ret = capri_repl_init();
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

    return ret;
}

hal_ret_t
capri_p4_asm_init()
{
    hal_ret_t               ret = HAL_RET_OK;
    uint64_t                p4_prm_base_addr;
	char             		*cfg_path;
	std::string      		full_path;

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

    symbols = (capri_prog_param_info_t *)HAL_CALLOC(capri_prog_param_info_t,
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

    symbols[28].name = "cpu_initial_action.bin";
    symbols[28].num_params = 2;
    symbols[28].params[0].name = ARQRX_BASE;
    symbols[28].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    symbols[28].params[1].name = ARQRX_QIDXR_BASE;
    symbols[28].params[1].val = get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);

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
    symbols[32].num_params = 2;
    symbols[32].params[0].name = ARQRX_BASE;
    symbols[32].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    symbols[32].params[1].name = ARQRX_QIDXR_BASE;
    symbols[32].params[1].val = get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);

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
    symbols[35].params[0].name = ARQRX_BASE;
    symbols[35].params[0].val = get_start_offset(CAPRI_HBM_REG_ARQRX);
    symbols[35].params[1].name = ARQRX_QIDXR_BASE;
    symbols[35].params[1].val = get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);

    // Please increment CAPRI_P4PLUS_NUM_SYMBOLS when you want to add more below
    
    p4plus_prm_base_addr = (uint64_t)get_start_offset((char *)JP4PLUS_PRGM);
    HAL_TRACE_DEBUG("base addr {:#x}", p4plus_prm_base_addr);
    capri_load_mpu_programs("p4plus", (char *)full_path.c_str(),
                            p4plus_prm_base_addr, symbols, CAPRI_P4PLUS_NUM_SYMBOLS);

    HAL_FREE(capri_prog_param_info_t, symbols);

    return ret;
}
