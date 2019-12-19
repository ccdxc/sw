//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __L2SEGMENT_API_HPP__
#define __L2SEGMENT_API_HPP__

#include "nic/hal/plugins/cfg/nw/interface.hpp"

namespace hal {

void *l2seg_get_pd(l2seg_t *seg);
l2seg_id_t l2seg_get_l2seg_id(l2seg_t *seg);
types::encapType l2seg_get_tunnel_encap_type(l2seg_t *seg);
uint32_t l2seg_get_tunnel_encap_val(l2seg_t *seg);
types::encapType l2seg_get_wire_encap_type(l2seg_t *seg);
encap_t l2seg_get_wire_encap(l2seg_t *seg);
uint32_t l2seg_get_wire_encap_val(l2seg_t *seg);
void *l2seg_get_pi_nwsec(l2seg_t *l2seg);
l2seg_t *l2seg_get_infra_l2seg();
uint32_t l2seg_get_ipsg_en(l2seg_t *pi_l2seg);
uint32_t l2seg_get_bcast_fwd_policy(l2seg_t *pi_l2seg);
#if 0
oif_list_id_t l2seg_get_bcast_oif_list(l2seg_t *pi_l2seg);
oif_list_id_t l2seg_get_mcast_oif_list(l2seg_t *pi_l2seg);
oif_list_id_t l2seg_get_prmsc_oif_list(l2seg_t *pi_l2seg);
oif_list_id_t l2seg_get_shared_bcast_oif_list(l2seg_t *pi_l2seg);
oif_list_id_t l2seg_get_shared_mcast_oif_list(l2seg_t *pi_l2seg);
#endif
ip_addr_t *l2seg_get_gipo(l2seg_t *pi_l2seg);
mac_addr_t *l2seg_get_rtr_mac(l2seg_t *pi_l2seg);
hal_handle_t l2seg_get_pinned_uplink(l2seg_t *seg);
// bool l2seg_is_shared_mgmt (l2seg_t *l2seg);
bool l2seg_is_shared_mgmt_attached(l2seg_t *l2seg);
uint32_t l2seg_num_attached_l2segs(l2seg_t *l2seg);

hal_ret_t l2seg_update_oiflist_oif(l2seg_t *l2seg, if_t *hal_if, bool add,
                                   bool only_non_designated, bool update_bcast, 
                                   bool update_mcast, bool update_prmsc);

hal_ret_t l2seg_attach_oifls(l2seg_t *cl_l2seg, l2seg_t *hp_l2seg, 
                             if_t *uplink_if);
hal_ret_t l2seg_detach_oifls (l2seg_t *cl_l2seg, l2seg_t *hp_l2seg, 
                              if_t *uplink_if);
oif_list_id_t l2seg_base_oifl_id(l2seg_t *l2seg, if_t *uplink_if);

} // namespace hal


#endif // __L2SEGMENT_API_HPP__
