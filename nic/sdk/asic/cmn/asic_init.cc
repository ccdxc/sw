// {C} Copyright 2020 Pensando Systems Inc. All rights reserved


#include <boost/algorithm/string/predicate.hpp>
#include "asic/asic.hpp"
#include "asic/cmn/asic_cfg.hpp"
#include "asic/rw/asicrw.hpp"
#include "asic/cmn/asic_hbm.hpp"
#include "lib/p4/p4_api.hpp"
#include "include/sdk/mem.hpp"

namespace sdk {
namespace asic {

static asic_init_type_t asic_init_type = ASIC_INIT_TYPE_HARD;
static asic_state_t asic_state = ASIC_STATE_RUNNING;

__attribute__((constructor)) void asic_init_type_ (void)
{
    char *value;

    if ((value = getenv("ASIC_SOFT_INIT"))) {
        asic_init_type = ASIC_INIT_TYPE_SOFT;
    } else {
        asic_init_type = ASIC_INIT_TYPE_HARD;
    }
}

bool
asic_is_soft_init (void)
{
    return asic_init_type == ASIC_INIT_TYPE_SOFT ? true : false;
}

bool
asic_is_hard_init (void)
{
    return asic_init_type == ASIC_INIT_TYPE_HARD ? true : false;
}

void
asic_set_init_type (asic_init_type_t type)
{
    asic_init_type = type;
}

void
set_state (asic_state_t state)
{
    asic_state = state;
}

bool
asic_is_quiesced (void)
{
    return asic_state == ASIC_STATE_QUIESCED ? true : false;
}

sdk_ret_t
asic_pgm_init (asic_cfg_t *cfg)
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
        ret = asic_load_config((char *)full_path.c_str());
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Failed to load config %s", full_path);
            return ret;
        }
    }

    return SDK_RET_OK;
}

// if write_to_mem is false, the below functions prepares the asm data structures
// to retrieve pc offset and other infos. if it is true it would write the asm's
// to the memory specified by mem.json
sdk_ret_t
asic_asm_init (asic_cfg_t *cfg, bool write_to_mem)
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
        if(cfg->asm_cfg[i].symbols_func) {
            num_symbols = cfg->asm_cfg[i].symbols_func((void **)&symbols, cfg->platform);
        }

        base_addr = asic_get_mem_addr(cfg->asm_cfg[i].base_addr.c_str());
        SDK_TRACE_DEBUG("base addr 0x%llx", base_addr);
        iret = sdk::p4::p4_load_mpu_programs(cfg->asm_cfg[i].name.c_str(),
           (char *)full_path.c_str(),
           base_addr,
           symbols,
           num_symbols,
           cfg->asm_cfg[i].sort_func,
           write_to_mem);

       if(symbols)
           SDK_FREE(SDK_MEM_ALLOC_PD, symbols);

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

sdk_ret_t
asic_hbm_regions_init (asic_cfg_t *cfg)
{
    sdk_ret_t           ret = SDK_RET_OK;
    bool                asm_write_to_mem = true;

    // reset all the HBM regions that are marked for reset
    asic_reset_hbm_regions(cfg);

    ret = asic_asm_init(cfg, asm_write_to_mem);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = asic_pgm_init(cfg);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return ret;
}

}    // asic
}    // sdk 
