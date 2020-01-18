//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __ACL_HPP__
#define __ACL_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "lib/ht/ht.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "gen/proto/acl.pb.h"
#include "gen/proto/kh.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"

// Include key of internal fields for use only with DOL/testing infra
// For production builds this needs to be removed
// TODO: REMOVE
#define ACL_DOL_TEST_ONLY

using sdk::lib::ht_ctxt_t;
using kh::AclKeyHandle;

using acl::AclSpec;
using acl::AclSelector;
using acl::AclActionInfo;
using acl::AclStatus;
using acl::AclResponse;
using acl::AclRequestMsg;
using acl::AclResponseMsg;
using acl::AclDeleteRequest;
using acl::AclDeleteResponse;
using acl::AclDeleteRequestMsg;
using acl::AclDeleteResponseMsg;
using acl::AclGetRequest;
using acl::AclGetRequestMsg;
using acl::AclGetResponse;
using acl::AclGetResponseMsg;
using acl::AclStats;
using acl::IPSelector;

namespace hal {

/*
 * General Rule:
 * - All drop ACLs have to be above FTE span. Otherwise, packets will not be dropped
 *   and they will have unintended behavior.
 *
 * ACL_HOSTPIN_HOST_MGMT_DROP has to be above EPLearn:
 * - Packets coming from host mgmt IFs with hostpin mode should not be accepted. 
 *   ARPs should hit host-mgmt-drop acl before EP Learn.
 *
 * ACL_IP_FRAGMENT_DROP_ENTRY_PRIORITY has to be above ACL_FTE_SPAN_PRIORITY  
 * - Fragmented packets should be dropped with acl. If span is above frag drop,
 *   frag packets will have undefined behavior as flow key will not be 
 *   formed properly.
 */

#define HAL_MAX_ACLS 512

#define ACL_IP_FRAGMENT_DROP_ENTRY_ID    0
#define ACL_FTE_SPAN_ENTRY_ID            1
#define ACL_QUIESCE_ENTRY_ID             2
#define ACL_EPLEARN_ENTRY_ID_BEGIN       3
#define ACL_EPLEARN_ENTRY_ID_END         7
#define ACL_NCSI_BCAST_ARP               8
#define ACL_NCSI_BCAST_DHCP_CLIENT       ACL_NCSI_BCAST_ARP + 2
#define ACL_NCSI_BCAST_DHCP_SERVER       ACL_NCSI_BCAST_DHCP_CLIENT + 2
#define ACL_NCSI_BCAST_NETBIOS_NS        ACL_NCSI_BCAST_DHCP_SERVER + 2
#define ACL_NCSI_BCAST_NETBIOS_DS        ACL_NCSI_BCAST_NETBIOS_NS + 2
#define ACL_NCSI_BCAST_ALL               ACL_NCSI_BCAST_NETBIOS_DS + 2
#define ACL_NCSI_MCAST_IPV6_NADV         ACL_NCSI_BCAST_ALL + 2
#define ACL_NCSI_MCAST_IPV6_RADV         ACL_NCSI_MCAST_IPV6_NADV + 2
#define ACL_NCSI_MCAST_DHCPV6_RELAY      ACL_NCSI_MCAST_IPV6_RADV + 2
#define ACL_NCSI_MCAST_DHCPV6_MCAST      ACL_NCSI_MCAST_DHCPV6_RELAY + 2
#define ACL_NCSI_MCAST_IPV6_MLD_LQ       ACL_NCSI_MCAST_DHCPV6_MCAST + 2
#define ACL_NCSI_MCAST_IPV6_MLD_LR       ACL_NCSI_MCAST_IPV6_MLD_LQ + 2
#define ACL_NCSI_MCAST_IPV6_MLD_LD       ACL_NCSI_MCAST_IPV6_MLD_LR + 2
#define ACL_NCSI_MCAST_IPV6_NSOL         ACL_NCSI_MCAST_IPV6_MLD_LD + 2
#define ACL_NCSI_MCAST_ALL               ACL_NCSI_MCAST_IPV6_NSOL + 2           // Last entry: 37


#define ACL_IP_FRAGMENT_DROP_ENTRY_PRIORITY    0
#define ACL_HOSTPIN_HOST_MGMT_DROP             1
#define ACL_FTE_SPAN_PRIORITY                  2
#define ACL_QUIESCE_ENTRY_PRIORITY             3
#define ACL_EPLEARN_ENTRY_PRIORITY_BEGIN       4
#define ACL_EPLEARN_ENTRY_PRIORITY_END         8
#define ACL_SNAKE_TEST_PRIORITY_BEGIN          9
#define ACL_SNAKE_TEST_PRIORITY_END            24
#define ACL_RDMA_SNIFFER_PRIORITY              25
#define ACL_NCSI_BCAST_ARP_PRIORITY            26
#define ACL_NCSI_BCAST_DHCP_CLIENT_PRIORITY    ACL_NCSI_BCAST_ARP_PRIORITY + 2
#define ACL_NCSI_BCAST_DHCP_SERVER_PRIORITY    ACL_NCSI_BCAST_DHCP_CLIENT_PRIORITY + 2
#define ACL_NCSI_BCAST_NETBIOS_NS_PRIORITY     ACL_NCSI_BCAST_DHCP_SERVER_PRIORITY + 2
#define ACL_NCSI_BCAST_NETBIOS_DS_PRIORITY     ACL_NCSI_BCAST_NETBIOS_NS_PRIORITY + 2
#define ACL_NCSI_BCAST_ALL_PRIORITY            ACL_NCSI_BCAST_NETBIOS_DS_PRIORITY + 2
#define ACL_NCSI_MCAST_IPV6_NADV_PRIORITY      ACL_NCSI_BCAST_ALL_PRIORITY + 2
#define ACL_NCSI_MCAST_IPV6_RADV_PRIORITY      ACL_NCSI_MCAST_IPV6_NADV_PRIORITY + 2
#define ACL_NCSI_MCAST_DHCPV6_RELAY_PRIORITY   ACL_NCSI_MCAST_IPV6_RADV_PRIORITY + 2
#define ACL_NCSI_MCAST_DHCPV6_MCAST_PRIORITY   ACL_NCSI_MCAST_DHCPV6_RELAY_PRIORITY + 2
#define ACL_NCSI_MCAST_IPV6_MLD_LQ_PRIORITY    ACL_NCSI_MCAST_DHCPV6_MCAST_PRIORITY + 2
#define ACL_NCSI_MCAST_IPV6_MLD_LR_PRIORITY    ACL_NCSI_MCAST_IPV6_MLD_LQ_PRIORITY + 2
#define ACL_NCSI_MCAST_IPV6_MLD_LD_PRIORITY    ACL_NCSI_MCAST_IPV6_MLD_LR_PRIORITY + 2
#define ACL_NCSI_MCAST_IPV6_NSOL_PRIORITY      ACL_NCSI_MCAST_IPV6_MLD_LD_PRIORITY + 2
#define ACL_NCSI_MCAST_ALL_PRIORITY            ACL_NCSI_MCAST_IPV6_NSOL_PRIORITY + 2        // Last entry: 55


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
    bool            direction;
    bool            flow_miss;
    bool            ip_options;
    bool            ip_frag;
    bool            tunnel_terminate;
    bool            from_cpu;
    drop_reason_codes_t drop_reasons;
    bool            no_drop;
    mac_addr_t      outer_mac_da;
    bool            ep_learn_en;
} __PACK__ acl_internal_match_spec_t;
#endif

// Specifications for the Acl
typedef struct acl_match_spec_s {
    bool            vrf_match;
    hal_handle_t    vrf_handle;     // vrf handle
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
    bool           mac_sa_rewrite;
    bool           mac_da_rewrite;
    bool           ttl_dec;
    types::CpucbId qid;
    bool           qid_en;
    uint32_t       rw_idx;              // rewrite index
    uint32_t       tnnl_vnid;           // tunnel vnid / encap vlan
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
    hal_handle_t   copp_handle;
    hal_handle_t   redirect_if_handle;
    bool           egress_drop;
#ifdef ACL_DOL_TEST_ONLY
    // Internal fields for use only with DOL/testing infra
    // For production builds this needs to be removed
    // TODO: REMOVE
    acl_internal_action_spec_t int_as;
#endif

} __PACK__ acl_action_spec_t;

typedef struct acl_key_s {
    acl_id_t acl_id;    // Acl id assigned
} __PACK__ acl_key_t;

inline std::ostream& operator<<(std::ostream& os, const acl_key_t& s)
{
   return os << fmt::format("{{acl_id={}}}", s.acl_id);
}

// Acl structure
typedef struct acl_s {
    sdk_spinlock_t    slock;         // lock to protect this structure
    acl_key_t         key;           // acl key information

    uint32_t          priority;
    acl_match_spec_t  match_spec;
    acl_action_spec_t action_spec;

    hal_handle_t      hal_handle;    // HAL allocated handle

    pd::pd_acl_t      *pd;
} __PACK__ acl_t;

// cb data structures
typedef struct acl_create_app_ctxt_s {
} __PACK__ acl_create_app_ctxt_t;

typedef struct acl_update_app_ctxt_s {
} __PACK__ acl_update_app_ctxt_t;

// allocate a Acl instance
static inline acl_t *
acl_alloc (void)
{
    acl_t    *acl;

    acl = (acl_t *)g_hal_state->acl_slab()->alloc();
    if (acl == NULL) {
        return NULL;
    }
    return acl;
}

// initialize a Acl instance
static inline acl_t *
acl_init (acl_t *acl)
{
    if (!acl) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&acl->slock, PTHREAD_PROCESS_PRIVATE);

    return acl;
}

// allocate and initialize a acl instance
static inline acl_t *
acl_alloc_init (void)
{
    return acl_init(acl_alloc());
}

static inline acl_t *
find_acl_by_id (acl_id_t acl_id)
{
    hal_handle_id_ht_entry_t *entry;
    acl_key_t                acl_key;
    acl_t                    *acl;

    acl_key.acl_id= acl_id;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->acl_ht()->lookup(&acl_key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        SDK_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_ACL);
        acl = (acl_t *)hal_handle_get_obj(entry->handle_id);
        return acl;
    }
    return NULL;
}

static inline acl_t *
find_acl_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_ERR("{}:failed to find object with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_ACL) {
        HAL_TRACE_ERR("{}:failed to find acl with handle:{}",
                        __FUNCTION__, handle);
        return NULL;
    }
    return (acl_t *)hal_handle_get_obj(handle);
}

static inline acl_t *
acl_lookup_by_key_or_handle (const AclKeyHandle& kh)
{
    if (kh.key_or_handle_case() == AclKeyHandle::kAclId) {
        return find_acl_by_id(kh.acl_id());
    } else if (kh.key_or_handle_case() == AclKeyHandle::kAclHandle) {
        return find_acl_by_handle(kh.acl_handle());
    }
    return NULL;
}

void *acl_get_key_func(void *entry);
uint32_t acl_key_size(void);

hal_ret_t acl_create(acl::AclSpec& spec,
                     acl::AclResponse *rsp);
hal_ret_t acl_update(acl::AclSpec& spec,
                     acl::AclResponse *rsp);
hal_ret_t acl_delete(acl::AclDeleteRequest& req,
                     acl::AclDeleteResponse *rsp);
hal_ret_t acl_get(acl::AclGetRequest& req,
                  acl::AclGetResponseMsg *rsp);
hal_ret_t acl_store_cb(void *obj, uint8_t *mem,
                       uint32_t len, uint32_t *mlen);
uint32_t acl_restore_cb(void *obj, uint32_t len);
}    // namespace hal

#endif    // __ACL_HPP__
