// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __DOS_HPP__
#define __DOS_HPP__

#include "nic/include/base.h"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/hal/lib/hal_handle.hpp"
#include "nic/gen/proto/hal/dos.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/include/pd.hpp"

using dos::DoSPolicySpec;
using dos::DoSPolicyStatus;
using dos::DoSPolicyRequestMsg;
using dos::DoSPolicyResponse;
using dos::DoSPolicyResponseMsg;
using dos::DoSPolicyDeleteRequest;
using dos::DoSPolicyDeleteRequestMsg;
using dos::DoSPolicyDeleteResponse;
using dos::DoSPolicyDeleteResponseMsg;
using dos::DoSPolicyGetRequest;
using dos::DoSPolicyStats;
using dos::DoSPolicyGetResponse;
using dos::DoSPolicyGetRequestMsg;
using dos::DoSPolicyGetResponseMsg;
using dos::IngressDoSPolicy;
using dos::EgressDoSPolicy;
using dos::DoSProtectionSpec;
using dos::DoSService;

namespace hal {

typedef struct dos_service_s {
    uint8_t         ip_proto;       // IP protocol
    bool            is_icmp;        // Is ICMP msg type ?
    union {
        uint16_t    dport;          // TCP or UDP port, 0 for other protos
        struct {
            uint8_t    icmp_msg_type;   // ICMP msg type
            uint8_t    icmp_msg_code;   // ICMP code
        } __PACK__;
    };
} __PACK__ dos_service_t;

typedef struct dos_session_limits_s {
    uint32_t    max_sessions;       // max. no of sessions
    uint32_t    blocking_timeout;   // cool off period once session count
                                    // comes below the above limit
} __PACK__ dos_session_limits_t;

// DoS aggregate policer
typedef struct dos_policer_s {
    uint32_t    bytes_per_sec;      // max. bytes per sec
    uint32_t    peak_rate;          // bytes-per-sec
    uint32_t    burst_size;         // bytes
} __PACK__ dos_policer_t;

typedef struct dos_flood_limits_s {
    uint32_t restrict_pps;          // restrict pps limit
    uint32_t restrict_burst_pps;    // restrict pps burst
    uint32_t restrict_duration;     // restrict action duration (secs)

    uint32_t protect_pps;           // protect pps limit
    uint32_t protect_burst_pps;     // protect pps burst
    uint32_t protect_duration;      // protect action duration (secs)
} __PACK__ dos_flood_limits_t;

typedef struct dos_policy_prop_s {
    dos_service_t         service;                // service attachment point
    dos_policer_t         policer;                // DoS agg. policer config
    dos_flood_limits_t    tcp_syn_flood_limits;   // TCP syn flood limits
    dos_flood_limits_t    udp_flood_limits;       // UDP flood limits
    dos_flood_limits_t    icmp_flood_limits;      // ICMP flood limits
    dos_flood_limits_t    other_flood_limits;     // Other flood limits
    dos_session_limits_t  session_limits;         // Session limits
    uint32_t              session_setup_rate;     // 0 means no limits
    uint32_t              peer_sg_id;             // Peer security group
} __PACK__ dos_policy_prop_t;

typedef struct dos_policy_s {
    hal_spinlock_t        slock;                  // lock to protect this structure
    hal_handle_t          hal_handle;             // HAL allocated handle
    hal_handle_t          vrf_handle;             // vrf handle

    bool                  ingr_pol_valid;
    bool                  egr_pol_valid;

    dos_policy_prop_t     ingress;
    dos_policy_prop_t     egress;

    // PD state
    void                  *pd;                    // all PD specific state
    // Security group list
    dllist_ctxt_t         sg_list_head;           // List of security groups
} __PACK__ dos_policy_t;

typedef struct dos_policy_sg_list_entry_s {
    int              sg_id;
    dllist_ctxt_t    dllist_ctxt;
} __PACK__ dos_policy_sg_list_entry_t;

hal_ret_t dospolicy_create(dos::DoSPolicySpec& spec,
                           dos::DoSPolicyResponse *rsp);

hal_ret_t dospolicy_update(dos::DoSPolicySpec& spec,
                           dos::DoSPolicyResponse *rsp);

hal_ret_t dospolicy_delete(dos::DoSPolicyDeleteRequest& req,
                           dos::DoSPolicyDeleteResponse *rsp);

hal_ret_t dospolicy_get(dos::DoSPolicyGetRequest& req,
                        dos::DoSPolicyGetResponseMsg *rsp);

typedef struct dos_policy_create_app_ctx_s {
} __PACK__ dos_policy_create_app_ctx_t;

typedef struct dos_policy_update_app_ctx_s {
} __PACK__ dos_policy_update_app_ctx_t;

// max. number of dos policies supported
#define HAL_MAX_DOS_POLICIES                       256

extern uint32_t dos_policy_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool dos_policy_id_compare_key_func(void *key1, void *key2);
dos_policy_t *dos_policy_lookup_handle(const dos::DoSPolicySpec& dp);
dos_policy_t *find_dos_policy_by_handle(hal_handle_t handle);
hal_ret_t hal_dos_init_cb(hal_cfg_t *hal_cfg);
hal_ret_t hal_dos_cleanup_cb(void);

}   // namespace hal

#endif // __DOS_HPP__
