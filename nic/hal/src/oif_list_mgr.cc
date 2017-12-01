// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/src/oif_list_mgr.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/base.h"

namespace hal {

// Creates a new oif_list and returns handle
hal_ret_t oif_list_create(oif_list_id_t *list)
{
    return pd::oif_list_create(list);
}

// Creates a contiguous block of oif_lists and returns handle to the first one
hal_ret_t oif_list_create_block(oif_list_id_t *list, uint32_t size)
{
    return pd::oif_list_create_block(list, size);
}

// Takes an oiflis_handle and deletes it
hal_ret_t oif_list_delete(oif_list_id_t list)
{
    return pd::oif_list_delete(list);
}

// Takes an oiflis_handle and deletes a block starting from it
hal_ret_t oif_list_delete_block(oif_list_id_t list, uint32_t size)
{
    return pd::oif_list_delete_block(list, size);
}

// Adds an oif to list
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif)
{
    return pd::oif_list_add_oif(list, oif);
}

// Adds an RDMA QP oif to list
hal_ret_t oif_list_add_qp_oif(oif_list_id_t list, oif_t *oif)
{
    return pd::oif_list_add_qp_oif(list, oif);
}

// Removes an oif from list
hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif)
{
    return pd::oif_list_remove_oif(list, oif);
}

// Check if an oif is present in the list
hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif)
{
    return pd::oif_list_is_member(list, oif);
}

// Get the number of oifs in the list
hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs)
{
    return pd::oif_list_get_num_oifs(list, num_oifs);
}

// Get an array of all oifs in the list
hal_ret_t oif_list_get_oif_array(oif_list_id_t list, uint32_t &num_oifs, oif_t *oifs)
{
    return pd::oif_list_get_oif_array(list, num_oifs, oifs);
}

hal_ret_t oif_list_set_honor_ingress(oif_list_id_t list)
{
    return pd::oif_list_set_honor_ingress(list);
}

}    // namespace hal
