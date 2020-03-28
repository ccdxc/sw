// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// These functions act as a pass through for PD access by PI.

#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/accel/accel_rgroup.hpp"

namespace hal {
namespace pd {


hal_ret_t
pd_accel_rgroup_init (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_init_args_t *args = pd_func_args->pd_accel_rgroup_init;
    return accel_rgroup_init(args->tid);
}

hal_ret_t
pd_accel_rgroup_fini (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_fini_args_t *args = pd_func_args->pd_accel_rgroup_fini;
    return accel_rgroup_fini(args->tid);
}

hal_ret_t
pd_accel_rgroup_add (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_add_args_t *args = pd_func_args->pd_accel_rgroup_add;
    return accel_rgroup_add(args->rgroup_name, args->metrics_mem_addr,
                            args->metrics_mem_size);
}

hal_ret_t
pd_accel_rgroup_del (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_del_args_t *args = pd_func_args->pd_accel_rgroup_del;
    return accel_rgroup_del(args->rgroup_name);
}

hal_ret_t
pd_accel_rgroup_ring_add (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_ring_add_args_t *args = pd_func_args->pd_accel_rgroup_ring_add;
    return accel_rgroup_ring_add(args->rgroup_name, args->ring_name,
                                 args->ring_handle);
}

hal_ret_t
pd_accel_rgroup_ring_del (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_ring_del_args_t *args = pd_func_args->pd_accel_rgroup_ring_del;
    return accel_rgroup_ring_del(args->rgroup_name, args->ring_name);
}

hal_ret_t
pd_accel_rgroup_reset_set (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_reset_set_args_t *args = pd_func_args->pd_accel_rgroup_reset_set;
    return accel_rgroup_reset_set(args->rgroup_name, args->sub_ring, &args->last_ring_handle,
                                  &args->last_sub_ring, args->reset_sense);
}

hal_ret_t
pd_accel_rgroup_enable_set (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_enable_set_args_t *args = pd_func_args->pd_accel_rgroup_enable_set;
    return accel_rgroup_enable_set(args->rgroup_name, args->sub_ring, &args->last_ring_handle,
                                   &args->last_sub_ring, args->enable_sense);
}

hal_ret_t
pd_accel_rgroup_pndx_set (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_pndx_set_args_t *args = pd_func_args->pd_accel_rgroup_pndx_set;
    return accel_rgroup_pndx_set(args->rgroup_name, args->sub_ring, &args->last_ring_handle,
                                 &args->last_sub_ring, args->val, args->conditional);
}

hal_ret_t
pd_accel_rgroup_info_get (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_info_get_args_t *args = pd_func_args->pd_accel_rgroup_info_get;
    return accel_rgroup_info_get(args->rgroup_name, args->sub_ring,
                                 args->cb_func, args->usr_ctx);
}

hal_ret_t
pd_accel_rgroup_indices_get (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_indices_get_args_t *args = pd_func_args->pd_accel_rgroup_indices_get;
    return accel_rgroup_indices_get(args->rgroup_name, args->sub_ring,
                                    args->cb_func, args->usr_ctx);
}

hal_ret_t
pd_accel_rgroup_metrics_get (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_metrics_get_args_t *args = pd_func_args->pd_accel_rgroup_metrics_get;
    return accel_rgroup_metrics_get(args->rgroup_name, args->sub_ring,
                                    args->cb_func, args->usr_ctx);
}

hal_ret_t
pd_accel_rgroup_misc_get (pd_func_args_t *pd_func_args)
{
    pd_accel_rgroup_misc_get_args_t *args = pd_func_args->pd_accel_rgroup_misc_get;
    return accel_rgroup_misc_get(args->rgroup_name, args->sub_ring,
                                 args->cb_func, args->usr_ctx);
}

} // namespace pd
} // namespace hal
