#ifndef __ACL_HPP__
#define __ACL_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/ip.h"
#include "nic/gen/proto/hal/acl.pb.h"

// Include key of internal fields for use only with DOL/testing infra
// For production builds this needs to be removed
// TODO: REMOVE
#define ACL_DOL_TEST_ONLY

using hal::utils::ht_ctxt_t;
using acl::AclSpec;
using acl::AclResponse;

namespace hal {

typedef uint32_t acl_id_t;

#define HAL_MAX_ACLS 512 

typedef struct acl_eth_match_spec_s {
    uint16_t    ether_type;
    mac_addr_t  mac_sa;
    mac_addr_t  mac_da;
} __PACK__ acl_eth_match_spec_t;

typedef struct acl_icmp_match_spec_s {
    uint8_t        icmp_type;    // ICMP type
    uint8_t        icmp_code;    // ICMP code
} __PACK__ acl_icmp_match_spec_t;

typedef struct acl_udp_match_spec_s {
    uint16_t sport; // Src port
    uint16_t dport; // Dest port
} __PACK__ acl_udp_match_spec_t;

typedef struct acl_tcp_match_spec_s {
    uint16_t sport;        // Src port
    uint16_t dport;        // Dest port
    uint8_t  tcp_flags;    // TCP flags
} __PACK__ acl_tcp_match_spec_t;

typedef struct acl_ip_match_spec_s {
    ip_addr_t sip;         // Source IP address
    ip_addr_t dip;         // Dest IP address
    uint8_t   ip_proto;    // IP Protocol
    union {
        acl_icmp_match_spec_t   icmp;
        acl_udp_match_spec_t    udp;
        acl_tcp_match_spec_t    tcp;
    } __PACK__ u;
} __PACK__ acl_ip_match_spec_t;

typedef enum {
    ACL_TYPE_NONE = 0,
    ACL_TYPE_ETH,       // Eth type
    ACL_TYPE_IP,        // IP type - match on both v4/v6
    ACL_TYPE_IPv4,      // IPv4 type - match on v4 only
    ACL_TYPE_IPv6,      // IPv6 type - match on v6 only
    ACL_TYPE_INVALID
} acl_type_e;

#ifdef ACL_DOL_TEST_ONLY
// Key of internal fields for use only with DOL/testing infra
// For production builds this needs to be removed
// TODO: REMOVE
typedef struct acl_internal_match_spec_s {
    bool       direction;
    bool       flow_miss;
    bool       ip_options;
    bool       ip_frag;
    bool       tunnel_terminate;
    uint32_t   drop_reason;
    mac_addr_t outer_mac_da;
} __PACK__ acl_internal_match_spec_t;
#endif

// Specifications for the Acl
typedef struct acl_match_spec_s {
    bool            tenant_match;
    hal_handle_t    tenant_handle;     // tenant handle
    bool            src_if_match;
    hal_handle_t    src_if_handle;     // source if handle
    bool            dest_if_match;
    hal_handle_t    dest_if_handle;    // dest if handle
    bool            l2seg_match;
    hal_handle_t    l2seg_handle;      // l2 segment handle
    acl_type_e      acl_type;
    union {
        acl_eth_match_spec_t eth;
        acl_ip_match_spec_t  ip;
    } __PACK__ key;
    union {
        acl_eth_match_spec_t eth;
        acl_ip_match_spec_t  ip;
    } __PACK__ mask;

#ifdef ACL_DOL_TEST_ONLY
    // Key of internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    acl_internal_match_spec_t int_key;
    acl_internal_match_spec_t int_mask;
#endif

} __PACK__ acl_match_spec_t;

#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
typedef struct acl_internal_action_spec_s {
    bool     mac_sa_rewrite;
    bool     mac_da_rewrite;
    bool     ttl_dec;
    uint32_t rw_idx;              // rewrite index
    uint32_t tnnl_vnid;           // tunnel vnid / encap vlan
} __PACK__ acl_internal_action_spec_t;
#endif

// Action specifications for the Acl
typedef struct acl_action_spec_s {
    acl::AclAction action;
    bool           ing_mirror_en;
    hal_handle_t   ing_mirror_session_handle;
    bool           egr_mirror_en;
    hal_handle_t   egr_mirror_session_handle;
    uint8_t        egr_mirror_session;   // Mirror sessions in egress direction
    uint8_t        ing_mirror_session;   // Mirror sessions in ingress direction
    hal_handle_t   copp_policer_handle;
    hal_handle_t   redirect_if_handle;
#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    acl_internal_action_spec_t int_as;
#endif

} __PACK__ acl_action_spec_t;

// Acl structure
typedef struct acl_s {
    hal_spinlock_t    slock;                 // lock to protect this structure
    acl_id_t          acl_id;                // Acl id assigned

    uint32_t          priority;
    acl_match_spec_t  match_spec;
    acl_action_spec_t action_spec;

    hal_handle_t      hal_handle;            // HAL allocated handle

    ht_ctxt_t         ht_ctxt;               // acl_id based hash table ctxt
    ht_ctxt_t         hal_handle_ht_ctxt;    // hal handle based hash table ctxt

    void              *pd_acl;
} __PACK__ acl_t;

static inline acl_t *
find_acl_by_id (acl_id_t acl_id)
{
    return (acl_t *)g_hal_state->acl_id_ht()->lookup(&acl_id);
}

static inline acl_t *
find_acl_by_handle (hal_handle_t handle)
{
    return (acl_t *)g_hal_state->acl_hal_handle_ht()->lookup(&handle);
}

extern void *acl_get_key_func(void *entry);
extern uint32_t acl_compute_hash_func(void *key, uint32_t ht_size);
extern bool acl_compare_key_func(void *key1, void *key2);

extern void *acl_get_handle_key_func(void *entry);
extern uint32_t acl_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool acl_compare_handle_key_func(void *key1, void *key2);

hal_ret_t acl_create(acl::AclSpec& spec,
                     acl::AclResponse *rsp);
hal_ret_t acl_update(acl::AclSpec& spec,
                     acl::AclResponse *rsp);
hal_ret_t acl_delete(acl::AclDeleteRequest& req,
                     acl::AclDeleteResponse *rsp);


}    // namespace hal

#endif    // __ACL_HPP__
