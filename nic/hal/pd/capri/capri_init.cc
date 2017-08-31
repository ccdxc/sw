#include <base.h>
#include "capri.hpp"
#include "capri_hbm.hpp"
#include "capri_config.hpp"
#include "capri_loader.h"
#include "capri_tbl_rw.hpp"

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
                        8 * sizeof(capri_prog_param_info_t));
    symbols[0].name = "tcp-read-rnmdr-alloc-idx.bin";
    symbols[0].num_params = 1;
    symbols[0].params[0].name = RNMDR_TABLE_BASE;
    symbols[0].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_RX);
    symbols[1].name = "tcp-read-rnmpr-alloc-idx.bin";
    symbols[1].num_params = 1;
    symbols[1].params[0].name = RNMPR_TABLE_BASE;
    symbols[1].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_RX);
    symbols[2].name = "tls-read-tnmdr-alloc-idx.bin";
    symbols[2].num_params = 1;
    symbols[2].params[0].name = TNMDR_TABLE_BASE;
    symbols[2].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    symbols[3].name = "tls-read-tnmpr-alloc-idx.bin";
    symbols[3].num_params = 1;
    symbols[3].params[0].name = TNMPR_TABLE_BASE;
    symbols[3].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);

    symbols[4].name = "tls-queue-brq-enc.bin";
    symbols[4].num_params = 1;
    symbols[4].params[0].name = BRQ_BASE;
    symbols[4].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);

    symbols[5].name = "tls-queue-brq-dec.bin";
    symbols[5].num_params = 1;
    symbols[5].params[0].name = BRQ_BASE;
    symbols[5].params[0].val = get_start_offset(CAPRI_HBM_REG_BRQ);

    symbols[6].name = "tls-read-tnmdr-free-idx.bin";
    symbols[6].num_params = 1;
    symbols[6].params[0].name = TNMDR_TABLE_BASE;
    symbols[6].params[0].val = get_start_offset(CAPRI_HBM_REG_NMDR_TX);
    symbols[7].name = "tls-read-tnmpr-free-idx.bin";
    symbols[7].num_params = 1;
    symbols[7].params[0].name = TNMPR_TABLE_BASE;
    symbols[7].params[0].val = get_start_offset(CAPRI_HBM_REG_NMPR_BIG_TX);


    p4plus_prm_base_addr = (uint64_t)get_start_offset((char *)JP4PLUS_PRGM);
    HAL_TRACE_DEBUG("base addr {:#x}", p4plus_prm_base_addr);
    capri_load_mpu_programs("p4plus", (char *)full_path.c_str(),
                            p4plus_prm_base_addr, symbols, 8);

    HAL_FREE(capri_prog_param_info_t, symbols);

    return ret;
}
