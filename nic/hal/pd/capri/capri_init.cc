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

	if (ret == HAL_RET_OK) {
			ret = capri_hbm_mem_init();
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
    capri_p4_prgm_init();
    capri_p4p_prgm_init();

    ret = capri_p4_pgm_init();
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
    HAL_TRACE_DEBUG("base addr {#x}", p4_prm_base_addr);
    capri_load_mpu_programs("iris", (char *)full_path.c_str(), p4_prm_base_addr, NULL, 0);

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
capri_p4p_prgm_init()
{
    hal_ret_t                           ret = HAL_RET_OK;
    uint64_t                            p4plus_prm_base_addr;
    char                                        *cfg_path;
    std::string                         full_path;

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

    p4plus_prm_base_addr = (uint64_t)get_start_offset((char *)JP4PLUS_PRGM);
    HAL_TRACE_DEBUG("base addr {#x}", p4plus_prm_base_addr);
    capri_load_mpu_programs("p4plus", (char *)full_path.c_str(), p4plus_prm_base_addr, NULL, 0);

    return ret;
}
