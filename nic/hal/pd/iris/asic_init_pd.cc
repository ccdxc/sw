// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_asic_init (pd_asic_init_args_t *args) 
{
    args->cfg->pgm_name = std::string("iris");
    asic_init(args->cfg);
    return HAL_RET_OK;
}
}    // namespace pd
}    // namespace hal
