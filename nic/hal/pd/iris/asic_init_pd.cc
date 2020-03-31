// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/platform/utils/lif_mgr/lif_mgr.hpp"
#include "nic/hal/pd/common_p4plus/common_p4plus.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"

using namespace sdk::asic::pd;

namespace hal {
namespace pd {

hal_ret_t
pd_asic_init (pd_func_args_t *pd_func_args)
{
    sdk_ret_t              sdk_ret;
    hal_ret_t              ret;
    pd_asic_init_args_t    *args = pd_func_args->pd_asic_init;

    args->cfg->repl_entry_width = P4_REPL_ENTRY_WIDTH;
    args->cfg->pgm_name = std::string("iris");

    args->cfg->num_pgm_cfgs = 1;
    args->cfg->pgm_cfg[0].path = std::string("pgm_bin");

    args->cfg->num_asm_cfgs = 2;
    args->cfg->asm_cfg[0].name = std::string("iris");
    args->cfg->asm_cfg[0].path = std::string("asm_bin");
    args->cfg->asm_cfg[0].symbols_func = NULL;
    args->cfg->asm_cfg[0].sort_func = NULL;
    args->cfg->asm_cfg[0].base_addr = std::string(JP4_PRGM);

    args->cfg->asm_cfg[1].name = std::string("p4plus");
    args->cfg->asm_cfg[1].path = std::string("p4plus_bin");
    args->cfg->asm_cfg[1].symbols_func = common_p4plus_symbols_init;
    args->cfg->asm_cfg[1].sort_func = NULL;
    args->cfg->asm_cfg[1].base_addr = std::string(JP4PLUS_PRGM);

    args->cfg->completion_func = asiccfg_init_completion_event;

    sdk_ret = sdk::asic::pd::asicpd_init(args->cfg);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "Asic init failure, err : {}", ret);

    sdk_ret = asicpd_sw_phv_init();
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "Capri s/w phv init failure, err : {}", ret);

    ret = (hal_ret_t)sdk::platform::utils::lif_mgr::lifs_reset(
                                                    HAL_LIF_ID_UPLINK_MIN - 1,
                                                    HAL_LIF_ID_SVC_LIF_MAX);
    SDK_ASSERT_TRACE_RETURN((ret == HAL_RET_OK), ret,
                            "LIF reset failure, err : {}", ret);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
