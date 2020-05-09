//----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//----------------------------------------------------------------------------

#pragma once

typedef enum gtest_oper_e {
    GTEST_CREATE,
    GTEST_UPDATE,
    GTEST_DELETE
} gtest_oper_t;

#define GTEST_EP_NUM_IPS 10
#define GTEST_EP_MIRR_SESSIONS 8

typedef struct gtest_ep_s {
    uint32_t vrf_id;
    uint32_t l2seg_id;
    uint32_t if_id;
    uint64_t mac;
    uint32_t ip[GTEST_EP_NUM_IPS];
    uint32_t ip_count;
} __PACK__ gtest_ep_t;

typedef struct gtest_enic_s {
    uint32_t if_id; 
    uint32_t lif_id; 
    intf::IfEnicType type;
    uint32_t l2seg_id; 
    uint32_t encap;
    uint32_t native_l2seg_id;
    uint32_t tx_mirr[GTEST_EP_MIRR_SESSIONS];
    uint32_t tx_mirr_count;
    uint32_t rx_mirr[GTEST_EP_MIRR_SESSIONS];
    uint32_t rx_mirr_count;
} __PACK__ gtest_enic_t;

hal_ret_t create_uplink(uint32_t if_id, uint32_t port, 
                        uint32_t native_l2seg = 0, bool is_oob = false);
hal_ret_t update_uplink(uint32_t if_id, uint32_t port, 
                        uint32_t native_l2seg = 0, bool is_oob = false);
hal_ret_t create_tunnel(uint32_t if_id, uint32_t vrf, uint32_t src_ip, uint32_t dst_ip);
hal_ret_t delete_interface(uint32_t if_id);
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
hal_ret_t create_enic(gtest_enic_t *enic);
hal_ret_t update_enic(gtest_enic_t *enic, gtest_oper_t oper);
hal_ret_t delete_enic(uint32_t if_id);
hal_ret_t create_ep(uint32_t vrf_id, uint32_t l2seg_id, uint32_t if_id, uint64_t mac);
hal_ret_t create_ep(uint32_t vrf_id, uint32_t l2seg_id, uint32_t if_id, uint64_t mac, uint32_t ip[], uint32_t ip_count);
hal_ret_t create_ep(gtest_ep_t *ep);
hal_ret_t update_ep(uint32_t vrf_id, uint32_t l2seg_id, uint32_t if_id, uint64_t mac, uint32_t ip[], uint32_t ip_count);
hal_ret_t update_ep(gtest_ep_t *ep);
hal_ret_t delete_ep(uint32_t vrf_id, uint32_t l2seg_id, uint64_t mac);
hal_ret_t create_mcast(uint32_t l2seg_id, uint64_t mac, uint32_t ifid[], uint32_t ifid_count);
hal_ret_t delete_mcast(uint32_t l2seg_id, uint64_t mac);
hal_ret_t create_mirror(uint32_t session_id, uint32_t vrf_id, uint32_t sip, uint32_t dip, bool vlan_strip_en = false, uint8_t erspan_type=3);
hal_ret_t create_collector(uint32_t cid, uint32_t vrf, uint32_t l2seg, 
                           uint32_t src_ip, uint32_t dst_ip);
