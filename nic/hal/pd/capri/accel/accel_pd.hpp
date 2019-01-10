
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __HAL_PD_ACCEL_HPP__
#define __HAL_PD_ACCEL_HPP__

#include "nic/include/base.hpp"

namespace hal {
namespace pd {

hal_ret_t pd_accel_rgroup_add (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_del (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_ring_add (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_ring_del (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_reset_set (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_enable_set (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_pndx_set (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_info_get (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_indices_get (pd_func_args_t *pd_func_args);
hal_ret_t pd_accel_rgroup_metrics_get (pd_func_args_t *pd_func_args);

} // namespace pd
} // namespace hal

#endif    //__HAL_PD_ACCEL_HPP__

