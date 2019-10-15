//----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//----------------------------------------------------------------------------

#pragma once

hal_ret_t create_uplink(uint32_t if_id, uint32_t port, 
                        uint32_t native_l2seg = 0, bool is_oob = false);
hal_ret_t update_uplink(uint32_t if_id, uint32_t port, 
                        uint32_t native_l2seg = 0, bool is_oob = false);
hal_ret_t create_vrf(uint32_t vrf_id, types::VrfType type, uint32_t des_if_id);
hal_ret_t update_l2seg(uint32_t vrf_id, uint32_t l2seg_id, uint32_t encap,
                       uint32_t up_ifid[], uint32_t ifid_count, 
                       l2segment::MulticastFwdPolicy mcast_pol,
                       l2segment::BroadcastFwdPolicy bcast_pol, 
                       bool is_swm,
                       bool create);
hal_ret_t delete_l2seg (uint32_t vrf_id, uint32_t l2seg_id);
hal_ret_t create_lif(uint32_t lif_id, uint32_t if_id, types::LifType type, string name,
                     bool is_oob = false, bool is_int = false, bool recv_bc = false, 
                     bool recv_allmc = false, bool recv_prom = false, bool vlan_strip = false, 
                     bool vlan_ins = false);
hal_ret_t create_enic(uint32_t if_id, uint32_t lif_id, intf::IfEnicType type,
                      uint32_t l2seg_id, uint32_t encap, // smart enic fields
                      uint32_t native_l2seg_id); // classic fields
hal_ret_t delete_enic(uint32_t if_id);
hal_ret_t create_ep(uint32_t vrf_id, uint32_t l2seg_id, uint32_t if_id, uint64_t mac);
hal_ret_t delete_ep(uint32_t vrf_id, uint32_t l2seg_id, uint64_t mac);
hal_ret_t create_mcast(uint32_t l2seg_id, uint64_t mac, uint32_t ifid[], uint32_t ifid_count);
hal_ret_t delete_mcast (uint32_t l2seg_id, uint64_t mac);
