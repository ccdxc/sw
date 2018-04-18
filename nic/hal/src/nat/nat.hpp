//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NAT_HPP__
#define __NAT_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/gen/proto/hal/nat.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/include/ip.h"
#include "nic/fte/acl/acl.hpp"

using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;

using kh::NatPoolKeyHandle;
using nat::NatPoolSpec;
using nat::NatPoolResponse;
using nat::NatPoolDeleteRequest;
using nat::NatPoolDeleteResponse;
using nat::NatPoolGetRequest;
using nat::NatPoolGetResponseMsg;
using kh::NatPolicyKeyHandle;
using nat::NatPolicySpec;
using nat::NatPolicyStatus;
using nat::NatPolicyStats;
using nat::NatPolicyResponse;
using nat::NatPolicyRequestMsg;
using nat::NatPolicyResponseMsg;
using nat::NatPolicyDeleteRequest;
using nat::NatPolicyDeleteResponse;
using nat::NatPolicyGetRequest;
using nat::NatPolicyGetResponseMsg;
using nat::NatVpnMappingSpec;
using nat::NatVpnMappingResponse;
using nat::NatVpnMappingGetRequest;
using nat::NatVpnMappingGetResponseMsg;

namespace hal {

typedef unsigned int rule_id_t;

//------------------------------------------------------------------------------
// NAT Rule Key
// Key passed by user to identify the rule
//------------------------------------------------------------------------------
typedef struct nat_rule_key_s {
    rule_id_t   rule_id;
    vrf_id_t    vrf_id;
} __PACK__ nat_rule_key_t;

//------------------------------------------------------------------------------
// NAT Rule
//
// Each NAT rule config is saved in this structure.
//
// There are many ways of accessing this node for ease-of use.
//
//   @key/key_ctx - To lookup based on vrf-id/rule-id
//   @hal_hdl/hal_hdl_ctx - To lookup based on hal handle
//   @acl_ctx - To lookup based on the packet fields (ip-addr/port)
//------------------------------------------------------------------------------
typedef struct nat_rule_s {
    hal_spinlock_t   slock;
    nat_rule_key_t   key;
    ht_ctxt_t        key_ctx;

    const acl_ctx_t  *acl_ctx;
    hal_handle_t     hal_hdl;
    ht_ctxt_t        hal_hdl_ctx;
} __PACK__ nat_rule_t;

hal_ret_t nat_pool_create(NatPoolSpec& spec, NatPoolResponse *rsp);
hal_ret_t nat_pool_update(NatPoolSpec& spec, NatPoolResponse *rsp);
hal_ret_t nat_pool_delete(NatPoolDeleteRequest& req,
                          NatPoolDeleteResponse *rsp);
hal_ret_t nat_pool_get(NatPoolGetRequest& req,
                       NatPoolGetResponseMsg *rsp);

hal_ret_t nat_policy_create(NatPolicySpec& spec, NatPolicyResponse *rsp);
hal_ret_t nat_policy_update(NatPolicySpec& spec, NatPolicyResponse *rsp);
hal_ret_t nat_policy_delete(NatPolicyDeleteRequest& req,
                            NatPolicyDeleteResponse *rsp);
hal_ret_t nat_policy_get(NatPolicyGetRequest& req,
                         NatPolicyGetResponseMsg *res);

hal_ret_t nat_vpn_mapping_create(NatVpnMappingSpec& spec,
                                 NatVpnMappingResponse *rsp);
hal_ret_t nat_vpn_mapping_get(NatVpnMappingGetRequest& req,
                              NatVpnMappingGetResponseMsg *res);

}    // namespace hal

#endif    // __NAT_HPP__

