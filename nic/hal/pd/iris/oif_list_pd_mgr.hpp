// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __OIF_LIST_PD_MGR_HPP__
#define __OIF_LIST_PD_MGR_HPP__

#include "nic/include/oif_list_api.hpp"

namespace hal {
namespace pd {

// Creates a new oif_list and returns handle
hal_ret_t oif_list_create(oif_list_id_t *list);
// Takes an oiflis_handle and deletes it
hal_ret_t oif_list_delete(oif_list_id_t list);
// Adds an oif to list
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif);
// Adds QP based oif to list
hal_ret_t oif_list_add_qp_oif(oif_list_id_t list, oif_t *oif);
// Removes an oif from list
hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif);
// Check if an oif is present in the list
hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif);
// Get the number of oifs in the list
hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs);
// Get an array of all oifs in the list
hal_ret_t oif_list_get_oif_array(oif_list_id_t list, uint32_t &num_oifs, oif_t *oifs);
// Adds a special node for ingress driven copy
hal_ret_t oif_list_set_honor_ingress(oif_list_id_t list);

}   // namespace pd
}   // namespace hal

#endif /* __OIF_LIST_PD_MGR_HPP__ */
