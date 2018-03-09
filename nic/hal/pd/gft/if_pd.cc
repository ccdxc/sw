// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/include/app_redir_shared.h"
#include "nic/hal/pd/gft/uplinkif_pd.hpp"
#include "nic/hal/pd/gft/enicif_pd.hpp"
#include "nic/hal/pd/gft/if_pd.hpp"
// #include "nic/hal/pd/iris/cpuif_pd.hpp"
#include "nic/p4/gft/include/defines.h"

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// PD If Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_if_create (pd_if_create_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("{}: if create ", __FUNCTION__);


    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_create(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_create(args);
            break;
        default:
            HAL_ASSERT(0);
    }
    // Branch out for different interface types
    return ret;
}

// ----------------------------------------------------------------------------
// PD If Update
// ----------------------------------------------------------------------------
hal_ret_t
pd_if_update (pd_if_update_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("{}: if update", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_update(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_update(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// PD If Delete
// ----------------------------------------------------------------------------
hal_ret_t
pd_if_delete (pd_if_delete_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("{}: if delete", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_delete(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_delete(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// PD If mem free
// ----------------------------------------------------------------------------
hal_ret_t
pd_if_mem_free (pd_if_mem_free_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;

    HAL_TRACE_DEBUG("{}: if mem_free", __FUNCTION__);

    if_type = hal::intf_get_if_type(args->intf);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_mem_free(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_mem_free(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

hal_ret_t
pd_if_make_clone (pd_if_make_clone_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type;
    if_t *hal_if = args->hal_if;
    // if_t *clone = args->clone;

    HAL_TRACE_DEBUG("{}: if clone", __FUNCTION__);

    if_type = hal::intf_get_if_type(hal_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            ret = pd_enicif_make_clone(args);
            break;
        case intf::IF_TYPE_UPLINK:
            ret = pd_uplinkif_make_clone(args);
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}

}    // namespace pd
}    // namespace hal
