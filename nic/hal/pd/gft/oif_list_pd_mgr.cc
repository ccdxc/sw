// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.hpp"
#include "nic/hal/pd/pd_api.hpp"

using namespace hal;

namespace hal {
namespace pd {

// Creates a new oif_list and returns handle
hal_ret_t pd_oif_list_create(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Creates a contiguous block of oif_lists and returns handle to the first one
hal_ret_t pd_oif_list_create_block(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Takes an oiflis_handle and deletes it
hal_ret_t pd_oif_list_delete(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Takes an oiflis_handle and deletes a block starting from it
hal_ret_t pd_oif_list_delete_block(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Attach an existing Replication List to another existing Replication List
// This is useful for jumping to (*, G) entries at the end of (S, G) entries
// Also helpful in jumping to all-multicast list at the end of specific lists
// ----------------------------------------------------------------------------
hal_ret_t pd_oif_list_attach(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Detach an existing Replication List from another existing Replication List
// ----------------------------------------------------------------------------
hal_ret_t pd_oif_list_detach(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Adds an oif to list
hal_ret_t pd_oif_list_add_oif(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Adds an rdma qp based oif to list
hal_ret_t pd_oif_list_add_qp_oif(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Removes an oif from list
hal_ret_t pd_oif_list_remove_oif(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Check if an oif is present in the list
hal_ret_t pd_oif_list_is_member(pd_func_args_t *pd_func_args) {
    return HAL_RET_OK;
}

// Get an array of all oifs in the list
hal_ret_t pd_oif_list_get_num_oifs(pd_func_args_t *pd_func_args) {
    return HAL_RET_OK;
}

// Adds a special node for ingress driven copy
hal_ret_t pd_oif_list_set_honor_ingress(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Deletes the special node for ingress driven copy
hal_ret_t pd_oif_list_clr_honor_ingress(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

// Get Function for proto
hal_ret_t pd_oif_list_get(pd_func_args_t *pd_func_args)
{
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

