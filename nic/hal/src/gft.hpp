// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __GFT_HPP__
#define __GFT_HPP__

#include "nic/include/base.h"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/utils/block_list/block_list.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/gen/proto/hal/gft.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal.hpp"

using kh::GftExactMatchProfileKeyHandle;
using kh::GftHeaderTranspositionProfileKeyHandle;
using kh::GftExactMatchFlowEntryKeyHandle;

using gft::GftExactMatchProfileSpec;
using gft::GftExactMatchProfileRequestMsg;
using gft::GftExactMatchProfileStatus;
using gft::GftExactMatchProfileResponse;
using gft::GftExactMatchProfileResponseMsg;
using gft::GftHeaderGroupExactMatchProfile;
using gft::GftHeaders;
using gft::GftHeaderFields;

using gft::GftHeaderTranspositionProfileSpec;
using gft::GftHeaderTranspositionProfileRequestMsg;
using gft::GftHeaderTranspositionProfileStatus;
using gft::GftHeaderTranspositionProfileResponse;
using gft::GftHeaderTranspositionProfileResponseMsg;

using gft::GftHeaders;
using gft::GftHeaderFields;
using gft::GftEthFields;
using gft::EncapOrTransportMatch;
using gft::EncapOrTransportTransposition;
using gft::GftHeaderGroupTransposition;
using gft::GftHeaderGroupExactMatch;
using gft::GftTableType;

using gft::GftExactMatchFlowEntrySpec;
using gft::GftExactMatchFlowEntryRequestMsg;
using gft::GftExactMatchFlowEntryStatus;
using gft::GftExactMatchFlowEntryResponse;
using gft::GftExactMatchFlowEntryResponseMsg;

// bitmap of flags indicating headers of interest per profile
#define GFT_HEADER_ETHERNET                        0x00000001
#define GFT_HEADER_IPV4                            0x00000002
#define GFT_HEADER_IPV6                            0x00000004
#define GFT_HEADER_TCP                             0x00000008
#define GFT_HEADER_UDP                             0x00000010
#define GFT_HEADER_ICMP                            0x00000020
#define GFT_HEADER_NO_ENCAP                        0x00000040
#define GFT_HEADER_IP_IN_IP_ENCAP                  0x00000080
#define GFT_HEADER_IP_IN_GRE_ENCAP                 0x00000100
#define GFT_HEADER_NVGRE_ENCAP                     0x00000200
#define GFT_HEADER_VXLAN_ENCAP                     0x00000400

// bitmap of flags indicating header fields of interest per profile
#define GFT_HEADER_FIELD_DST_MAC_ADDR              0x00000001ULL
#define GFT_HEADER_FIELD_SRC_MAC_ADDR              0x00000002ULL
#define GFT_HEADER_FIELD_ETH_TYPE                  0x00000004ULL
#define GFT_HEADER_FIELD_CUSTOMER_VLAN_ID          0x00000008ULL
#define GFT_HEADER_FIELD_PROVIDER_VLAN_ID          0x00000010ULL
#define GFT_HEADER_FIELD_8021P_PRIORITY            0x00000020ULL
#define GFT_HEADER_FIELD_SRC_IP_ADDR               0x00000040ULL
#define GFT_HEADER_FIELD_DST_IP_ADDR               0x00000080ULL
#define GFT_HEADER_FIELD_TTL                       0x00000100ULL
#define GFT_HEADER_FIELD_IP_PROTOCOL               0x00000200ULL
#define GFT_HEADER_FIELD_IP_DSCP                   0x00000400ULL
#define GFT_HEADER_FIELD_TRANSPORT_SRC_PORT        0x00000800ULL
#define GFT_HEADER_FIELD_TRANSPORT_DST_PORT        0x00001000ULL
#define GFT_HEADER_FIELD_TCP_FLAGS                 0x00002000ULL
#define GFT_HEADER_FIELD_TENANT_ID                 0x00004000ULL
#define GFT_HEADER_FIELD_ENTROPY                   0x00008000ULL
#define GFT_HEADER_FIELD_ICMP_TYPE                 0x00010000ULL
#define GFT_HEADER_FIELD_ICMP_CODE                 0x00020000ULL
#define GFT_HEADER_FIELD_OOB_VLAN                  0x00040000ULL
#define GFT_HEADER_FIELD_OOB_TENANT_ID             0x00080000ULL
#define GFT_HEADER_FIELD_GRE_PROTOCOL              0x00100000ULL



namespace hal {

typedef uint32_t vport_id_t;
typedef uint32_t gft_profile_id_t;
typedef uint32_t gft_table_id_t;
typedef uint32_t gft_flow_entry_id_t;

typedef struct gft_eth_fields_s {
    mac_addr_t    dmac;
    mac_addr_t    smac;
    uint16_t      eth_type;
    uint16_t      customer_vlan_id;
    uint16_t      provider_vlan_id;
    uint8_t       priority;
} __PACK__ gft_eth_fields_t;

typedef union encap_or_transport_match_u {
    struct {
        uint16_t    sport;
        uint16_t    dport;
    } __PACK__ udp;
    struct {
        uint16_t    sport;
        uint16_t    dport;
        uint8_t     tcp_flags;
    } __PACK__ tcp;
    struct {
        uint8_t     type;
        uint8_t     code;
    } __PACK__ icmp;
    struct {
        uint32_t    tenant_id;
        uint16_t    gre_protocol;
    } __PACK__ encap;
} __PACK__ encap_or_transport_match_t;

typedef union encap_or_transport_xposition_u {
    struct {
        uint16_t    sport;
        uint16_t    dport;
    } __PACK__ udp;
    struct {
        uint16_t    sport;
        uint16_t    dport;
        uint8_t     tcp_flags;  // Do we need this?
    } __PACK__ tcp;
    struct {
        uint8_t     type;
        uint8_t     code;
    } __PACK__ icmp;
    struct {
        uint32_t    tenant_id;
        uint16_t    gre_protocol;
        uint16_t    entropy;
    } __PACK__ encap;
} __PACK__ encap_or_transport_xposition_t;

typedef enum gft_table_type_e {
    GFT_TABLE_TYPE_NONE,
    GFT_TABLE_TYPE_WILDCARD_INGRESS,
    GFT_TABLE_TYPE_WILDCARD_EGRESS,
    GFT_TABLE_TYPE_EXACT_MATCH_INGRESS,
    GFT_TABLE_TYPE_EXACT_MATCH_EGRESS,
    GFT_TABLE_TYPE_PACKET_DIRECT,
    GFT_TABLE_TYPE_MAX,
} gft_table_type_t;

static inline const char *
gft_table_type_to_str(gft_table_type_t type) {
    switch (type) {
        case GFT_TABLE_TYPE_NONE: return "NONE";
        case GFT_TABLE_TYPE_WILDCARD_INGRESS: return "WILDCARD_INGRESS";
        case GFT_TABLE_TYPE_WILDCARD_EGRESS: return "WILDCARD_EGRESS";
        case GFT_TABLE_TYPE_EXACT_MATCH_INGRESS: return "EXACT_INGRESS";
        case GFT_TABLE_TYPE_EXACT_MATCH_EGRESS: return "EXACT_EGRESS";
        case GFT_TABLE_TYPE_PACKET_DIRECT:
        default: return "ERROR";
    }
}

// flags for GFT header group exact match profile
#define GFT_HDR_GROUP_EXACT_MATCH_PROFILE_IS_TTL_ONE    0x00000001
typedef struct gft_hdr_group_exact_match_profile_s {
    uint32_t    flags;           // GFT_HDR_GROUP_EXACT_MATCH_PROFILE_XXX flags, if any
    uint32_t    headers;         // bitmap iof GFT_HEADER_XXX
    uint64_t    match_fields;    // bitmap of GFT_HEADER_FIELD_XXX
} __PACK__ gft_hdr_group_exact_match_profile_t;

// flags for GFT exact match profile
#define GFT_EXACT_MATCH_PROFILE_RDMA_FLOW      0x00000001
typedef struct gft_exact_match_profile_s {
    hal_spinlock_t                         slock;      // lock to protect this structure
    gft_profile_id_t                       profile_id; // profile id
    uint32_t                               flags;      // GFT_EXACT_MATCH_PROFILE_XXX flags, if any
    gft_table_type_t                       table_type;
    uint32_t                               num_hdr_group_exact_match_profiles;
    gft_hdr_group_exact_match_profile_t    *hgem_profiles;

    // operational state
    hal_handle_t                           hal_handle;              // HAL allocated handle

    void                                   *pd;    // PD state, if any
} __PACK__ gft_exact_match_profile_t;

typedef struct gft_emp_create_app_ctxt_s {
} __PACK__ gft_emp_create_app_ctxt_t;

#define HAL_MAX_GFT_EXACT_MATCH_PROFILES    512

// allocate a GFT exact match profile instance
static inline gft_exact_match_profile_t *
gft_exact_match_profile_alloc (void)
{
    gft_exact_match_profile_t    *profile;

    profile = (gft_exact_match_profile_t *)
                  g_hal_state->gft_exact_match_profile_slab()->alloc();
    if (profile == NULL) {
        return NULL;
    }
    return profile;
}

// initialize a GFT exact match profile instance
static inline gft_exact_match_profile_t *
gft_exact_match_profile_init (gft_exact_match_profile_t *profile)
{
    if (!profile) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&profile->slock, PTHREAD_PROCESS_PRIVATE);
    profile->profile_id = 0;
    profile->flags = 0;
    profile->table_type = GFT_TABLE_TYPE_NONE;
    profile->num_hdr_group_exact_match_profiles = 0;
    profile->hgem_profiles = NULL;

    profile->hal_handle = HAL_HANDLE_INVALID;
    profile->pd = NULL;

    return profile;
}

// allocate and initialize a GFT exact match profile instance
static inline gft_exact_match_profile_t *
gft_exact_match_profile_alloc_init (void)
{
    return gft_exact_match_profile_init(gft_exact_match_profile_alloc());
}

// free a GFT exact match profile instance
static inline hal_ret_t
gft_exact_match_profile_free (gft_exact_match_profile_t *profile)
{
    HAL_SPINLOCK_DESTROY(&profile->slock);
    hal::delay_delete_to_slab(HAL_SLAB_GFT_EXACT_MATCH_PROFILE, profile);
    return HAL_RET_OK;
}

// cleanup and free a GFT exact match profile
static inline hal_ret_t
gft_exact_match_profile_cleanup (gft_exact_match_profile_t *profile)
{
    gft_exact_match_profile_free(profile);
    return HAL_RET_OK;
}

// find a GFT exact match profile instance by its id
static inline gft_exact_match_profile_t *
find_gft_exact_match_profile_by_id (gft_profile_id_t profile_id)
{
    hal_handle_id_ht_entry_t     *entry;
    gft_exact_match_profile_t    *profile;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
                gft_exact_match_profile_id_ht()->lookup(&profile_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_GFT_EXACT_MATCH_PROFILE);

        profile =
            (gft_exact_match_profile_t *)hal_handle_get_obj(entry->handle_id);
        return profile;
    }
    return NULL;
}

// find a GFT exact match profile instance by its handle
static inline gft_exact_match_profile_t *
find_gft_exact_match_profile_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_GFT_EXACT_MATCH_PROFILE) {
        HAL_TRACE_DEBUG("Failed to find GFT exact match profile with handle {}",
                        handle);
        return NULL;
    }
    return (gft_exact_match_profile_t *)hal_handle_get_obj(handle);
}

// flags for GFT header group exact match
#define GFT_HDR_GROUP_EXACT_MATCH_IS_TTL_ONE        0x00000001

typedef struct gft_hdr_group_exact_match_s {
    uint32_t                      flags;            // GFT_HDR_GROUP_EXACT_MATCH_XXX flags, if any
    uint32_t                      headers;
    uint64_t                      fields;
    gft_eth_fields_t              eth_fields;
    ip_addr_t                     src_ip_addr;
    ip_addr_t                     dst_ip_addr;
    uint8_t                       ttl;
    uint8_t                       dscp;
    uint8_t                       ip_proto;
    encap_or_transport_match_t    encap_or_transport;
} __PACK__ gft_hdr_group_exact_match_t;

// header transposition actions
typedef enum gft_hdr_group_xposition_action_e {
    GFT_HDR_GROUP_XPOSITION_ACTION_NONE,
    GFT_HDR_GROUP_XPOSITION_ACTION_MODIFY,
    GFT_HDR_GROUP_XPOSITION_ACTION_IGNORE,
    GFT_HDR_GROUP_XPOSITION_ACTION_PUSH,
    GFT_HDR_GROUP_XPOSITION_ACTION_POP,
    GFT_HDR_GROUP_XPOSITION_ACTION_MAX,
} gft_hdr_group_xposition_action_t;

static inline const char *
gft_xpos_action_to_str(gft_hdr_group_xposition_action_t act) {
    switch (act) {
        case GFT_HDR_GROUP_XPOSITION_ACTION_NONE: return "NONE";
        case GFT_HDR_GROUP_XPOSITION_ACTION_MODIFY: return "MODIFY";
        case GFT_HDR_GROUP_XPOSITION_ACTION_IGNORE: return "IGNORE";
        case GFT_HDR_GROUP_XPOSITION_ACTION_PUSH: return "PUSH";
        case GFT_HDR_GROUP_XPOSITION_ACTION_POP: return "POP";
        default: return "ERROR";
    }
}

// flags for header group transposition profile
#define GFT_HDR_GROUP_XPOSITION_PROFILE_DECREMENT_TTL_IF_NOT_ONE    0x00000001

typedef struct gft_hdr_group_xposition_profile_s {
    uint32_t                            flags;
    gft_hdr_group_xposition_action_t    action;
    uint32_t                            headers;         // bitmap iof GFT_HEADER_XXX
    uint64_t                            match_fields;    // bitmap of GFT_HEADER_FIELD_XXX
} __PACK__ gft_hdr_group_xposition_profile_t;

// flags in GFT header transposition profile
#define GFT_HXP_REDIRECT_TO_INGRESS_QUEUE_OF_VPORT                 0x00000001
#define GFT_HXP_REDIRECT_TO_EGRESS_QUEUE_OF_VPORT                  0x00000002
#define GFT_HXP_REDIRECT_TO_INGRESS_QUEUE_OF_VPORT_IF_TTL_IS_ONE   0x00000004
#define GFT_HXP_REDIRECT_TO_EGRESS_QUEUE_OF_VPORT_IF_TTL_IS_ONE    0x00000008
#define GFT_HXP_COPY_ALL_PACKETS                                   0x00000010
#define GFT_HXP_COPY_FIRST_PACKET                                  0x00000020
#define GFT_HXP_COPY_WHEN_TCP_FLAG_SET                             0x00000040
#define GFT_HXP_CUSTOM_ACTION_PRESENT                              0x00000080
#define GFT_HXP_META_ACTION_BEFORE_HEADER_TRANSPOSITION            0x00000100
typedef struct gft_hdr_xposition_profile_s {
    hal_spinlock_t                       slock;         // lock to protect this structure
    gft_profile_id_t                     profile_id;    // profile id
    uint32_t                             flags;         // GFT_HXP_XXX flags, if any
    gft_table_type_t                     table_type;    // table type
    uint32_t                             num_htp;       // number of header transposition profiles
    gft_hdr_group_xposition_profile_t    *hdr_xposition_profiles;
    
    // operational state
    hal_handle_t          hal_handle;              // HAL allocated handle

    void                                 *pd;    // PD state, if any
} __PACK__ gft_hdr_xposition_profile_t;

typedef struct gft_htp_create_app_ctxt_s {
} __PACK__ gft_htp_create_app_ctxt_t;

#define HAL_MAX_GFT_HDR_TRANSPOSITION_PROFILES    65536

// allocate a GFT header transposition profile instance
static inline gft_hdr_xposition_profile_t *
gft_hdr_transposition_profile_alloc (void)
{
    gft_hdr_xposition_profile_t    *profile;

    profile = (gft_hdr_xposition_profile_t *)
                  g_hal_state->gft_hdr_transposition_profile_slab()->alloc();
    if (profile == NULL) {
        return NULL;
    }
    return profile;
}

// initialize a GFT header transposition profile instance
static inline gft_hdr_xposition_profile_t *
gft_hdr_transposition_profile_init (gft_hdr_xposition_profile_t *profile)
{
    if (!profile) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&profile->slock, PTHREAD_PROCESS_PRIVATE);
    profile->profile_id = 0;
    profile->flags = 0;
    profile->table_type = GFT_TABLE_TYPE_NONE;
    profile->num_htp = 0;
    profile->hdr_xposition_profiles = NULL;

    profile->hal_handle = HAL_HANDLE_INVALID;
    profile->pd = NULL;

    return profile;
}

// allocate and initialize a GFT header transposition profile instance
static inline gft_hdr_xposition_profile_t *
gft_hdr_transposition_profile_alloc_init (void)
{
    return gft_hdr_transposition_profile_init(gft_hdr_transposition_profile_alloc());
}

// free a GFT header transposition profile instance
static inline hal_ret_t
gft_hdr_transposition_profile_free (gft_hdr_xposition_profile_t *profile)
{
    HAL_SPINLOCK_DESTROY(&profile->slock);
    hal::delay_delete_to_slab(HAL_SLAB_GFT_HDR_TRANSPOSITION_PROFILE, profile);
    return HAL_RET_OK;
}

// cleanup and free a GFT header transposition profile
static inline hal_ret_t
gft_hdr_transposition_profile_cleanup (gft_hdr_xposition_profile_t *profile)
{
    gft_hdr_transposition_profile_free(profile);
    return HAL_RET_OK;
}

// find a GFT header transposition profile instance by its id
static inline gft_hdr_xposition_profile_t *
find_gft_hdr_xposition_profile_by_id (gft_profile_id_t profile_id)
{
    hal_handle_id_ht_entry_t       *entry;
    gft_hdr_xposition_profile_t    *profile;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
                gft_hdr_transposition_profile_id_ht()->lookup(&profile_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_GFT_HDR_TRANSPOSITION_PROFILE);

        profile =
            (gft_hdr_xposition_profile_t *)hal_handle_get_obj(entry->handle_id);
        return profile;
    }
    return NULL;
}

// find a GFT header transposition profile instance by its handle
static inline gft_hdr_xposition_profile_t *
find_gft_hdr_xposition_profile_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_GFT_HDR_TRANSPOSITION_PROFILE) {
        HAL_TRACE_DEBUG("Failed to find GFT header transposition profile with handle {}",
                        handle);
        return NULL;
    }
    return (gft_hdr_xposition_profile_t *)hal_handle_get_obj(handle);
}

// flags for GFT header group transposition
#define GFT_HDR_GROUP_XPOSITION_DECREMENT_TTL_IF_NOT_ONE    0x00000001
typedef struct gft_hdr_group_xposition_s {
    uint32_t                            flags;    // GFT_HDR_GROUP_XPOSITION_XXX flags, if any
    gft_hdr_group_xposition_action_t    action;
    uint32_t                            headers;  // GFT_HEADER_XX
    uint64_t                            fields;
    gft_eth_fields_t                    eth_fields;
    ip_addr_t                           src_ip_addr;
    ip_addr_t                           dst_ip_addr;
    uint8_t                             ttl;
    uint8_t                             dscp;
    uint8_t                             ip_proto;
    encap_or_transport_xposition_t      encap_or_transport;
} __PACK__ gft_hdr_group_xposition_t;

// exact match flow entry flags
// NOTE: not all flags are supported
#define GFT_EMFE_ADD_IN_ACTIVATED_STATE                            0x00000001
#define GFT_EMFE_MATCH_AND_ACTION_MUST_BE_SUPPORTED                0x00000002
#define GFT_EMFE_RDMA_FLOW                                         0x00000004
#define GFT_EMFE_REDIRECT_TO_INGRESS_QUEUE_OF_VPORT                0x00001000
#define GFT_EMFE_REDIRECT_TO_EGRESS_QUEUE_OF_VPORT                 0x00002000
#define GFT_EMFE_REDIRECT_TO_INGRESS_QUEUE_OF_VPORT_IF_TTL_IS_ONE  0x00004000
#define GFT_EMFE_REDIRECT_TO_EGRESS_QUEUE_OF_VPORT_IF_TTL_IS_ONE   0x00008000
#define GFT_EMFE_COPY_ALL_PACKETS                                  0x00010000
#define GFT_EMFE_COPY_FIRST_PACKET                                 0x00020000
#define GFT_EMFE_COPY_WHEN_TCP_FLAG_SET                            0x00040000
#define GFT_EMFE_CUSTOM_ACTION_PRESENT                             0x00080000
#define GFT_EMFE_META_ACTION_BEFORE_HEADER_TRANSPOSITION           0x00100000
#define GFT_EMFE_COPY_AFTER_TCP_FIN_FLAG_SET                       0x00200000
#define GFT_EMFE_COPY_AFTER_TCP_RST_FLAG_SET                       0x00400000
#define GFT_EMFE_COPY_CONDITION_CHANGED                            0x01000000
#define GFT_EMFE_ALL_VPORT_FLOW_ENTRIES                            0x02000000
#define GFT_EMFE_COUNTER_ALLOCATE                                  0x00000001
#define GFT_EMFE_COUNTER_MEMORY_MAPPED                             0x00000002
#define GFT_EMFE_COUNTER_CLIENT_SPECIFIED_ADDRESS                  0x00000004

typedef enum gft_flow_entry_cache_hint_e {
    GFT_FLOW_ENTRY_CACHE_HINT_NONE,
    GFT_FLOW_ENTRY_CACHE_HINT_LOW_FREQUENCY,
    GFT_FLOW_ENTRY_CACHE_HINT_MEDIUM_FREQUENCY,
    GFT_FLOW_ENTRY_CACHE_HINT_HIGH_FREQUENCY,
    GFT_FLOW_ENTRY_CACHE_HINT_MAX,
} gft_flow_entry_cache_hint_t;

typedef struct gft_exact_match_flow_entry_s {
    hal_spinlock_t                 slock;                              // lock to protect this structure
    gft_flow_entry_id_t            flow_entry_id;                      // flow entry id allocated by the app

    uint32_t                       flags;                              // GFT_EMFE_XXX flags
    hal_handle_t                   gft_emp_hal_handle;                 // exact match profile id
    hal_handle_t                   gft_htp_hal_handle;                 // header xposition profile id
    gft_table_type_t               table_type;
    vport_id_t                     vport_id;                           // vport to apply this flow entry to
    vport_id_t                     redirect_vport_id;                  // redirect vport id, if any
    vport_id_t                     ttl_one_redirect_vport_id;          // vport id to redirect to if TTL is one
    gft_flow_entry_cache_hint_t    cache_hint;                         // cache hint, if any
    uint32_t                       num_exact_matches;                  // # of header group exact matches
    uint32_t                       num_transpositions;                 // # of header group transpositions
    gft_hdr_group_exact_match_t    *exact_matches;                     // exact match list
    gft_hdr_group_xposition_t      *transpositions;                    // header transposition list

    // operational state
    hal_handle_t                   hal_handle;                         // HAL allocated handle

    void                           *pd;                                // PD state, if any
} __PACK__ gft_exact_match_flow_entry_t;

typedef struct gft_emfe_create_app_ctxt_s {
    GftExactMatchFlowEntryResponse *rsp;
} __PACK__ gft_emfe_create_app_ctxt_t;

#define HAL_MAX_GFT_EXACT_MATCH_FLOW_ENTRIES        1048576

// allocate a GFT exact match flow entry instance
static inline gft_exact_match_flow_entry_t *
gft_exact_match_flow_entry_alloc (void)
{
    gft_exact_match_flow_entry_t    *flow_entry;

    flow_entry = (gft_exact_match_flow_entry_t *)
                      g_hal_state->gft_exact_match_flow_entry_slab()->alloc();
    if (flow_entry == NULL) {
        return NULL;
    }
    return flow_entry;
}

// initialize a GFT exact match flow entry instance
static inline gft_exact_match_flow_entry_t *
gft_exact_match_flow_entry_init (gft_exact_match_flow_entry_t *flow_entry)
{
    if (!flow_entry) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&flow_entry->slock, PTHREAD_PROCESS_PRIVATE);
    flow_entry->flow_entry_id = 0;
    flow_entry->flags = 0;
    flow_entry->table_type = GFT_TABLE_TYPE_NONE;
    flow_entry->vport_id = 0;
    flow_entry->gft_emp_hal_handle = HAL_HANDLE_INVALID;
    flow_entry->gft_htp_hal_handle = HAL_HANDLE_INVALID;
    flow_entry->redirect_vport_id = 0;
    flow_entry->ttl_one_redirect_vport_id = 0;
    flow_entry->cache_hint = GFT_FLOW_ENTRY_CACHE_HINT_NONE;
    flow_entry->num_exact_matches = 0;
    flow_entry->num_transpositions = 0;
    flow_entry->exact_matches = NULL;
    flow_entry->transpositions = NULL;

    flow_entry->pd = NULL;
    flow_entry->hal_handle = HAL_HANDLE_INVALID;

    return flow_entry;
}

// allocate and initialize a GFT exact match flow entry instance
static inline gft_exact_match_flow_entry_t *
gft_exact_match_flow_entry_alloc_init (void)
{
    return gft_exact_match_flow_entry_init(gft_exact_match_flow_entry_alloc());
}

// free a GFT exact match flow entry instance
static inline hal_ret_t
gft_exact_match_flow_entry_free (gft_exact_match_flow_entry_t *flow_entry)
{
    HAL_SPINLOCK_DESTROY(&flow_entry->slock);
    hal::delay_delete_to_slab(HAL_SLAB_GFT_EXACT_MATCH_FLOW_ENTRY, flow_entry);
    return HAL_RET_OK;
}

// cleanup and free a GFT exact match flow entry instance
static inline hal_ret_t
gft_exact_match_flow_entry_cleanup (gft_exact_match_flow_entry_t *flow_entry)
{
    gft_exact_match_flow_entry_free(flow_entry);
    return HAL_RET_OK;
}

// find a GFT exact match flow entry instance by its id
static inline gft_exact_match_flow_entry_t *
find_gft_exact_match_flow_entry_by_id (gft_flow_entry_id_t flow_entry_id)
{
    hal_handle_id_ht_entry_t        *entry;
    gft_exact_match_flow_entry_t    *flow_entry;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
                gft_exact_match_flow_entry_id_ht()->lookup(&flow_entry_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_GFT_EXACT_MATCH_FLOW_ENTRY);
        flow_entry =
            (gft_exact_match_flow_entry_t *)hal_handle_get_obj(entry->handle_id);
        return flow_entry;
    }
    return NULL;
}

// find a GFT exact match flow entry instance by its handle
static inline gft_exact_match_flow_entry_t *
find_gft_exact_match_flow_entry_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_GFT_EXACT_MATCH_FLOW_ENTRY) {
        HAL_TRACE_DEBUG("Failed to find GFT exact match flow entry with handle {}",
                        handle);
        return NULL;
    }
    return (gft_exact_match_flow_entry_t *)hal_handle_get_obj(handle);
}

void *gft_exact_match_profile_id_get_key_func(void *entry);
uint32_t gft_exact_match_profile_id_compute_hash_func(void *key,
                                                      uint32_t ht_size);
bool gft_exact_match_profile_id_compare_key_func(void *key1, void *key2);

void *gft_hdr_transposition_profile_id_get_key_func(void *entry);
uint32_t gft_hdr_transposition_profile_id_compute_hash_func(void *key,
                                                            uint32_t ht_size);
bool gft_hdr_transposition_profile_id_compare_key_func(void *key1, void *key2);

void *gft_exact_match_flow_entry_id_get_key_func(void *entry);
uint32_t gft_exact_match_flow_entry_id_compute_hash_func(void *key,
                                                         uint32_t ht_size);
bool gft_exact_match_flow_entry_id_compare_key_func(void *key1, void *key2);

// SVC CRUD APIs
hal_ret_t gft_exact_match_profile_create(GftExactMatchProfileSpec& spec,
                                         GftExactMatchProfileResponse *rsp);
hal_ret_t gft_header_transposition_profile_create(GftHeaderTranspositionProfileSpec &spec,
                                                  GftHeaderTranspositionProfileResponse *rsp);
hal_ret_t gft_exact_match_flow_entry_create(GftExactMatchFlowEntrySpec &spec,
                                            GftExactMatchFlowEntryResponse *rsp);

void gft_exact_match_flow_entry_print (gft_exact_match_flow_entry_t *fe);
void gft_exact_match_profile_print (gft_exact_match_profile_t *mp);
}    // namespace hal

#endif    // __GFT_HPP__

