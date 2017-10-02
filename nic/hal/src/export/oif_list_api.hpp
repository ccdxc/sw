// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __OIF_LIST_API_HPP__
#define __OIF_LIST_API_HPP__

#include "nic/include/base.h"
#include "nic/include/interface_api.hpp"
#include "nic/include/l2segment_api.hpp"

namespace hal {

    // Place for all public definitions of the OIF List manager.

    typedef struct oif_s {
        // if_id_t  if_id;
        // l2seg_id_t  l2_seg_id;
        if_t     *intf;
        l2seg_t  *l2seg;
    } oif_t;

    hal_ret_t oif_list_create(oif_list_id_t *list); // Creates a new oif_list and returns handle
    hal_ret_t oif_list_delete(oif_list_id_t list);  // Takes an oiflis_handle and deletes it
    hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif); // Adds an oif to list
    hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif); // Removes an oif from list
    hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif); // Check if an oif is present in the list
    hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs); // Get the number of oifs in the list
    hal_ret_t oif_list_get_oif_array(oif_list_id_t list, uint32_t &num_oifs, oif_t *oifs); // Get an array of all oifs in the list

}    // namespace hal

#endif /* __OIF_LIST_API_HPP__ */
