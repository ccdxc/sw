//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_DB_H__
#define __VPP_IMPL_APULU_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <vnet/ip/ip.h>
#include <nic/vpp/infra/utils.h>
#include <api.h>

#define PDS_VPP_MAX_SUBNET  64
#define PDS_VPP_MAX_VNIC    128

typedef enum {
    PDS_ETH_ENCAP_NO_VLAN,
    PDS_ETH_ENCAP_DOT1Q,    // single tag
    PDS_ETH_ENCAP_DOT1AD,   // double tag
} pds_eth_encap_type;

typedef struct {
    mac_addr_t mac;                 // vnic mac
    u32 max_sessions;               // max number of sessions from this vnic
    u32 active_session_count;       // currently active sessions on the vnic
    u16 subnet_hw_id;               // subnet hwid to index subnet store in infra
    u8 flow_log_en : 1;             // flag indicating flow logging enabled
    u8 reserve : 7;
    u8 enacp_type;                  // pds_eth_encap_type
    u8 l2_encap_len;                // layer2 encapsulation length
    u16 vlan_id;                    // vlan id if encap type is != no vlan
    u16 nh_hw_id;                   // nexthop id
} pds_impl_db_vnic_entry_t;

typedef struct {
    mac_addr_t mac;                 // subnet mac
    u8 prefix_len;                  // subnet prefix len
    ip46_address_t vr_ip;           // subnet VR ip
} pds_impl_db_subnet_entry_t;

#define foreach_impl_db_element                         \
 _(uint16_t, subnet)                                    \
 _(uint16_t, vnic)                                      \

typedef struct {
#define _(type, obj)                                    \
    u16 * obj##_pool_idx;                               \
    pds_impl_db_##obj##_entry_t * obj##_pool_base;

    foreach_impl_db_element
#undef _
} pds_impl_db_ctx_t;

extern pds_impl_db_ctx_t impl_db_ctx;

#define _(type, obj)                                                \
pds_impl_db_##obj##_entry_t * pds_impl_db_##obj##_get(type hw_id);
    foreach_impl_db_element
#undef _

#ifdef __cplusplus
}
#endif

#endif    // __VPP_IMPL_APULU_DB_H__
