//-----------------------------------------------------------------------------
// L2 HAL internal data structures
//-----------------------------------------------------------------------------
#ifndef __L2_HAL_HPP__
#define __L2_HAL_HPP__

#include <l2.hpp>

typedef struct l2_entry_info_s {
    l2_entry_data_t                         data;
    LIST_ENTRY(l2_entry_data_s)             l2_entry_link;
    LIST_HEAD(session_list_s, session_s)    session_list;
    uint8_t                                 learn_src:4;
} __PACK__ l2_entry_info_t;

typedef struct vlan_info_s {
    vlan_data_t                   data;
    uint16_t                      num_eps;
    LIST_ENTRY(vlan_data_s)       vrf_vlan_list;
    LIST_HEAD(ep_list_s, ep_s)    ep_list;
} __PACK__ vlan_info_t;

#endif    // __L2_HAL_HPP__

