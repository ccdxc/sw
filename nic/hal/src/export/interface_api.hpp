/* 
 * ----------------------------------------------------------------------------
 *
 * interface_api.hpp
 *
 * Interface APIs exported by PI to PD.
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __INTERFACE_API_HPP__
#define __INTERFACE_API_HPP__


namespace hal {

using hal::lif_t;
using hal::if_t;
using hal::l2seg_t;

// LIF APIs
uint32_t lif_get_port_num(lif_t *pi_lif);
uint32_t lif_get_lif_id(lif_t *pi_lif);
void lif_set_pd_lif(lif_t *pi_lif, void *pd_lif);
void *lif_get_pd_lif(lif_t *pi_lif);

// Interface APIs
intf::IfType intf_get_if_type(if_t *pi_if);
uint32_t if_get_if_id(if_t *pi_if);
uint32_t uplinkif_get_port_num(if_t *pi_if);
void if_set_pd_if(if_t *pi_if, void *pd_upif);
void *if_get_pd_if(if_t *pi_if);
bool is_l2seg_native(l2seg_t *l2seg, if_t *pi_if);
lif_t *if_get_lif(if_t *pi_if);

// Interface APIs for EnicIf
intf::IfEnicType if_get_enicif_type(if_t *pi_if);
vlan_id_t if_get_encap_vlan(if_t *pi_if);
mac_addr_t *if_get_mac_addr(if_t *pi_if);
void *if_enicif_get_pd_l2seg(if_t *pi_if);
void *if_enicif_get_pi_l2seg(if_t *pi_if);
void *if_enicif_get_pd_nwsec(if_t *pi_if);
void *if_enicif_get_pi_nwsec(if_t *pi_if);
uint32_t if_enicif_get_ipsg_en(if_t *pi_if);



} // namespace hal
#endif // __INTERFACE_API_HPP__
