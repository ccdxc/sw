//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __L2SEGMENT_API_HPP__
#define __L2SEGMENT_API_HPP__

namespace hal {

void *l2seg_get_pd(l2seg_t *seg);
l2seg_id_t l2seg_get_l2seg_id(l2seg_t *seg);
types::encapType l2seg_get_tunnel_encap_type(l2seg_t *seg);
uint32_t l2seg_get_tunnel_encap_val(l2seg_t *seg);
types::encapType l2seg_get_wire_encap_type(l2seg_t *seg);
uint32_t l2seg_get_wire_encap_val(l2seg_t *seg);
void *l2seg_get_pi_nwsec(l2seg_t *l2seg);
l2seg_t *l2seg_get_infra_l2seg();
uint32_t l2seg_get_ipsg_en(l2seg_t *pi_l2seg);
uint32_t l2seg_get_bcast_fwd_policy(l2seg_t *pi_l2seg);
oif_list_id_t l2seg_get_bcast_oif_list(l2seg_t *pi_l2seg);
oif_list_id_t l2seg_get_mcast_oif_list(l2seg_t *pi_l2seg);
oif_list_id_t l2seg_get_prmsc_oif_list(l2seg_t *pi_l2seg);
ip_addr_t *l2seg_get_gipo(l2seg_t *pi_l2seg);
mac_addr_t *l2seg_get_rtr_mac(l2seg_t *pi_l2seg);
hal_handle_t l2seg_get_pinned_uplink(l2seg_t *seg);

} // namespace hal


#endif // __L2SEGMENT_API_HPP__
