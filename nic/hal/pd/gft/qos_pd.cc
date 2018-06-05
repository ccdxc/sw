// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/hal/pd/gft/qos_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/qos_api.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_qos_class_get_admin_cos (pd_func_args_t *pd_func_args)
{
    pd_qos_class_get_admin_cos_args_t *args  = pd_func_args->pd_qos_class_get_admin_cos;
    args->cos = HAL_QOS_ADMIN_COS;
    return HAL_RET_OK;
}

hal_ret_t
pd_qos_class_get_qos_class_id (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

hal_ret_t
pd_qos_class_periodic_stats_update (pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
