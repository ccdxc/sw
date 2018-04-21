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
#include "pol.hpp"

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
using nat::NatVpnMappingSpec;
using nat::NatVpnMappingResponse;
using nat::NatVpnMappingGetRequest;
using nat::NatVpnMappingGetResponseMsg;

namespace hal {

// NAT pool key
typedef struct nat_pool_key_s {
    vrf_id_t         vrf_id;
    nat_pool_id_t    pool_id;
} __PACK__ nat_pool_key_t;
#define HAL_MAX_NAT_POOLS        1024

// address range list element used to store
// NAT address ranges
typedef struct addr_range_list_elem_s {
    dllist_ctxt_t     list_ctxt;
    ip_range_t        ip_range;
} __PACK__ addr_range_list_elem_t;

// NAT pool
typedef struct nat_pool_s {
    hal_spinlock_t    slock;          // lock to protect this structure
    nat_pool_key_t    key;            // key for the nat pool
    dllist_ctxt_t     addr_ranges;    // NAT address ranges

    // operational state of nat pool
    hal_handle_t       hal_handle;    // HAL allocated handle

    // stats
    uint32_t           num_in_use;
} __PACK__ nat_pool_t;

void *nat_pool_id_get_key_func(void *entry);
uint32_t nat_pool_id_compute_hash_func(void *key, uint32_t ht_size);
bool nat_pool_id_compare_key_func(void *key1, void *key2);

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

// module level callbacks
hal_ret_t hal_nat_init_cb(hal_cfg_t *hal_cfg);
hal_ret_t hal_nat_cleanup_cb(void);

}    // namespace hal

#endif    // __NAT_HPP__

