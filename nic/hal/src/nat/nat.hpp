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
#include "nic/include/l4.h"
#include "nic/fte/acl/acl.hpp"
#include "nic/hal/src/utils/addr_list.hpp"
#include "pol.hpp"
#include "pool.hpp"

using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;

using kh::NatPoolKey;
using kh::NatPoolKeyHandle;
using nat::NatPoolSpec;
using nat::NatPoolResponse;
using nat::NatPoolDeleteRequest;
using nat::NatPoolDeleteResponse;
using nat::NatPoolGetRequest;
using nat::NatPoolGetResponse;
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

using kh::NatMappingKeyHandle;
using nat::NatMappingSpec;
using nat::NatMappingResponse;
using nat::NatMappingDeleteRequest;
using nat::NatMappingDeleteResponse;
using nat::NatMappingGetRequest;
using nat::NatMappingGetResponse;
using nat::NatMappingGetResponseMsg;

namespace hal {

#define HAL_MAX_NAT_ADDR_MAP        8192

void *nat_mapping_get_key_func(void *entry);
uint32_t nat_mapping_compute_hash_func(void *key, uint32_t ht_size);
bool nat_mapping_compare_key_func(void *key1, void *key2);

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

hal_ret_t nat_mapping_create(NatMappingSpec& spec, NatMappingResponse *rsp);
hal_ret_t nat_mapping_delete(NatMappingDeleteRequest& req,
                             NatMappingDeleteResponse *rsp);
hal_ret_t nat_mapping_get(NatMappingGetRequest& req,
                          NatMappingGetResponseMsg *res);

// module level callbacks
hal_ret_t hal_nat_init_cb(hal_cfg_t *hal_cfg);
hal_ret_t hal_nat_cleanup_cb(void);

}    // namespace hal

#endif    // __NAT_HPP__

