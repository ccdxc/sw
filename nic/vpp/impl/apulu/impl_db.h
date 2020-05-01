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
#include <nic/apollo/p4/include/apulu_table_sizes.h>
#include <api.h>
#include "pdsa_impl_db_hdlr.h"

#define PDS_VPP_MAX_SUBNET  BD_TABLE_SIZE
#define PDS_VPP_MAX_VNIC    VNIC_INFO_TABLE_SIZE
#define PDS_VPP_MAX_VPC     VPC_TABLE_SIZE

typedef enum {
    PDS_ETH_ENCAP_NO_VLAN,
    PDS_ETH_ENCAP_DOT1Q,    // single tag
    PDS_ETH_ENCAP_DOT1AD,   // double tag
} pds_eth_encap_type;

typedef struct {
    mac_addr_t mac;                 // vnic mac
    u32 max_sessions;               // max number of sessions from this vnic
    volatile u32 active_ses_count;  // currently active sessions on the vnic
    u16 subnet_hw_id;               // subnet hwid to index subnet store in infra
    u8 flow_log_en : 1;             // flag indicating flow logging enabled
    u8 reserve : 7;
    u8 encap_type;                  // pds_eth_encap_type
    u8 l2_encap_len;                // layer2 encapsulation length
    u16 vlan_id;                    // vlan id if encap type is != no vlan
    u16 nh_hw_id;                   // nexthop id
    u16 host_lif_hw_id;             // host lif id
    u8 *rewrite;
} pds_impl_db_vnic_entry_t;

typedef struct {
    mac_addr_t mac;                 // subnet mac
    u8 prefix_len;                  // subnet prefix len
    ip46_address_t vr_ip;           // subnet VR ip
    u32 vnid;
    u16 vpc_id;                     // VPC id to which the subnet belongs
} pds_impl_db_subnet_entry_t;

typedef struct {
    u16 hw_bd_id;                   // vpc's bd id
    u16 flags;                      // vpc flags
} pds_impl_db_vpc_entry_t;

typedef struct {
    mac_addr_t device_mac;          // device MAC address
    ip46_address_t device_ip;       // device IP address
    u8 overlay_routing_en;          // overlay Routing enabled or not
} pds_impl_db_device_entry_t;

#define foreach_impl_db_element                         \
 _(uint16_t, subnet)                                    \
 _(uint16_t, vnic)                                      \
 _(uint16_t, vpc)                                       \

typedef struct {
#define _(type, obj)                                    \
    u16 * obj##_pool_idx;                               \
    pds_impl_db_##obj##_entry_t * obj##_pool_base;

    foreach_impl_db_element
#undef _
    pds_impl_db_device_entry_t device;
} pds_impl_db_ctx_t;

extern pds_impl_db_ctx_t impl_db_ctx;

#define _(type, obj)                                                \
pds_impl_db_##obj##_entry_t * pds_impl_db_##obj##_get(type hw_id);
    foreach_impl_db_element
#undef _

pds_impl_db_device_entry_t * pds_impl_db_device_get(void);
int pds_impl_db_vr_ip_mac_get (uint16_t subnet, uint32_t *vr_ip, uint8_t **vr_mac);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_IMPL_APULU_DB_H__
