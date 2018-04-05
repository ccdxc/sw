// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"
#include "nic/gen/iris/include/p4pd.h"

namespace hal {
namespace pd {

hal_ret_t
pd_asic_init (pd_asic_init_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;

    args->cfg->pgm_name = std::string("iris");
    args->cfg->repl_entry_width = P4_REPL_ENTRY_WIDTH;
    ret = asic_init(args->cfg);
    return ret;
}

}    // namespace pd
}    // namespace hal
