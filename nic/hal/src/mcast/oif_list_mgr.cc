// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/src/mcast/oif_list_mgr.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/base.h"

namespace hal {

// Creates a new oif_list and returns handle
hal_ret_t oif_list_create(oif_list_id_t *list)
{
    pd::pd_oif_list_create_args_t args;
    args.list = list;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_CREATE, (void *)&args);
}

// Creates a contiguous block of oif_lists and returns handle to the first one
hal_ret_t oif_list_create_block(oif_list_id_t *list, uint32_t size)
{
    pd::pd_oif_list_create_block_args_t args;
    args.list = list;
    args.size = size;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_CREATE_BLOCK, (void *)&args);
}

// Takes an oiflis_handle and deletes it
hal_ret_t oif_list_delete(oif_list_id_t list)
{
    pd::pd_oif_list_delete_args_t args;
    args.list = list;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_DELETE, (void *)&args);
}

// Takes an oiflis_handle and deletes a block starting from it
hal_ret_t oif_list_delete_block(oif_list_id_t list, uint32_t size)
{
    pd::pd_oif_list_delete_block_args_t args;
    args.list = list;
    args.size = size;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_DELETE_BLOCK, (void *)&args);
}

// ----------------------------------------------------------------------------
// Attach an existing Replication List to another existing Replication List
// This is useful for jumping to (*, G) entries at the end of (S, G) entries
// Also helpful in jumping to all-multicast list at the end of specific lists
// ----------------------------------------------------------------------------
hal_ret_t oif_list_attach(oif_list_id_t frm, oif_list_id_t to)
{
    pd::pd_oif_list_attach_args_t args;
    args.frm = frm;
    args.to = to;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_ATTACH, (void *)&args);
}

// ----------------------------------------------------------------------------
// Detach an existing Replication List from another existing Replication List
// ----------------------------------------------------------------------------
hal_ret_t oif_list_detach(oif_list_id_t frm)
{
    pd::pd_oif_list_detach_args_t args;
    args.frm = frm;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_DETACH, (void *)&args);
}

// Adds an oif to list
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif)
{
    pd::pd_oif_list_add_oif_args_t args;
    args.list = list;
    args.oif = oif;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_ADD_OIF, (void *)&args);
}

// Adds an RDMA QP oif to list
hal_ret_t oif_list_add_qp_oif(oif_list_id_t list, oif_t *oif)
{
    pd::pd_oif_list_add_qp_oif_args_t args;
    args.list = list;
    args.oif = oif;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_ADD_QP_OIF, (void *)&args);
}

// Removes an oif from list
hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif)
{
    pd::pd_oif_list_remove_oif_args_t args;
    args.list = list;
    args.oif = oif;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_REM_OIF, (void *)&args);
}

// Check if an oif is present in the list
hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif)
{
    pd::pd_oif_list_is_member_args_t args;
    args.list = list;
    args.oif = oif;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_IS_MEMBER, (void *)&args);
}

// Get the number of oifs in the list
hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs)
{
    pd::pd_oif_list_get_num_oifs_args_t args;
    args.list = list;
    args.num_oifs = &num_oifs;
    return pd::hal_pd_call(pd::PD_FUNC_ID_GET_NUM_OIFS, (void *)&args);
}

// Get an array of all oifs in the list
hal_ret_t oif_list_get_oif_array(oif_list_id_t list, uint32_t &num_oifs, oif_t *oifs)
{
    pd::pd_oif_list_get_oif_array_args_t args;
    args.list = list;
    args.num_oifs = &num_oifs;
    args.oifs = oifs;
    return pd::hal_pd_call(pd::PD_FUNC_ID_GET_OIF_ARRAY, (void *)&args);
}

hal_ret_t oif_list_set_honor_ingress(oif_list_id_t list)
{
    pd::pd_oif_list_set_honor_ingress_args_t args;
    args.list = list;
    return pd::hal_pd_call(pd::PD_FUNC_ID_SET_HONOR_ING, (void *)&args);
}

hal_ret_t oif_list_clr_honor_ingress(oif_list_id_t list)
{
    pd::pd_oif_list_clr_honor_ingress_args_t args;
    args.list = list;
    return pd::hal_pd_call(pd::PD_FUNC_ID_CLR_HONOR_ING, (void *)&args);
}

}    // namespace hal
