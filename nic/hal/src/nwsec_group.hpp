// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __NWSEC_GROUP_HPP__
#define __NWSEC_GROUP_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/include/pd.hpp"
//#include "nic/include/fte_lib/acl.hpp"
#include "nic/fte/acl/acl.hpp"


using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;
using acl::acl_config_t;
using acl::ref_t;
using namespace acl;

#if 0
using nwsec::SecurityGroupSpec;
using nwsec::SecurityGroupRequestMsg;
using nwsec::SecurityGroupStatus;
using nwsec::SecurityGroupResponse;
using nwsec::SecurityGroupGetResponse;
using kh::SecurityGroupKeyHandle;
using nwsec::SecurityGroupPolicySpec;
using nwsec::SecurityGroupPolicyRequestMsg;
using nwsec::SecurityGroupPolicyStatus;
using nwsec::SecurityGroupPolicyResponse;
using nwsec::SecurityGroupPolicyGetResponse;
using kh::SecurityGroupPolicyKeyHandle;
#endif
using nwsec::Service;
using types::IPProtocol;
using types::ICMPMsgType;
using nwsec::FirewallAction;
using nwsec::ALGName;

namespace hal {

// extern void *nwsec_profile_get_handle_key_func(void *entry);
// extern uint32_t nwsec_profile_compute_handle_hash_func(void *key, uint32_t ht_size);
// extern bool nwsec_profile_compare_handle_key_func(void *key1, void *key2);


// Data structure to maintain for security group:
// CFG_DB:
//    nwsec_policy_cfg_t - sg_id (key and stored into nwsec_policy_cfg_ht in cfg_db)
//            maintains linked list of ingres and egress rules.
//            rules are of data type - nwsec_policy_rules_t
//            each rule has list of services defined by - nwsec_policy_svc_t;
//
//
// OPER_DB: 
//     nwsec_policy_entry_t
//        We need to key in (src sg_id, dst sg_id)
//        Define a structure nwsec_policy_entry_t and this will point to the rules list
// To DO:
//      Introduce a indirection and make
//      nwsec_policy_rules_t sharable between cfg and oper db
//
//

typedef struct nwsec_policy_key_s {
    uint32_t            sg_id;
    uint32_t            peer_sg_id;
} __PACK__ nwsec_policy_key_t;

typedef struct nwsec_policy_svc_s {
    hal_spinlock_t      slock;              // Lock to protect this structure
    IPProtocol          ipproto;
    union {
        uint32_t        dst_port;
        ICMPMsgType     icmp_msg_type;
    }__PACK__;
    ALGName             alg;
    dllist_ctxt_t       lentry;
} __PACK__ nwsec_policy_svc_t;

typedef struct nwsec_policy_appid_s {
    hal_spinlock_t      slock;              // Lock to protect this structure
    uint32_t            appid;
    dllist_ctxt_t       lentry;
} __PACK__ nwsec_policy_appid_t;

typedef  struct nwsec_policy_rules_s {
    hal_spinlock_t         slock;
    dllist_ctxt_t          fw_svc_list_head;
    dllist_ctxt_t          appid_list_head;
    bool                   log;
    FirewallAction         action;
    dllist_ctxt_t          lentry;
} __PACK__ nwsec_policy_rules_t;


typedef struct nwsec_policy_cfg_s {
    hal_spinlock_t               slock;      // lock to protect this strucuture
    nwsec_policy_key_t           plcy_key;
    dllist_ctxt_t                rules_head; // List of rules - nwsec_policy_rules_cfg_t

    // operational state of security group policy
    hal_handle_t                 hal_handle;         // HAL allocated handle
    ht_ctxt_t                    ht_ctxt;
} __PACK__ nwsec_policy_cfg_t;

typedef struct nwsec_group_s {
    hal_spinlock_t     slock;
    uint32_t           sg_id;
    dllist_ctxt_t      ep_list_head;
    dllist_ctxt_t      nw_list_head;
    // Operational state of Security Group
    hal_handle_t       hal_handle;
    ht_ctxt_t          ht_ctxt;
}__PACK__ nwsec_group_t;


// Empty context for now
typedef struct nwsec_policy_cfg_create_app_ctxt_s {
} __PACK__ nwsec_policy_cfg_create_app_ctxt_t;

typedef struct nwsec_group_create_app_ctxt_s {
} __PACK__ nwsec_group_create_app_ctxt_t;

//-----------------------------------------------------------------------------
//  APIs related to nwsec_policy_svc_t
//_____________________________________________________________________________
static inline nwsec_policy_svc_t *
nwsec_policy_svc_alloc(void)
{
    nwsec_policy_svc_t *nwsec_plcy_svc;
    nwsec_plcy_svc = (nwsec_policy_svc_t *)
                        g_hal_state->nwsec_policy_svc_slab()->alloc();
    if (nwsec_plcy_svc == NULL) {
        return NULL;
    }
    return nwsec_plcy_svc;
}

static inline nwsec_policy_svc_t *
nwsec_policy_svc_init(nwsec_policy_svc_t *nwsec_plcy_svcs)
{
    if (!nwsec_plcy_svcs) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&nwsec_plcy_svcs->slock, PTHREAD_PROCESS_PRIVATE);
    dllist_reset(&nwsec_plcy_svcs->lentry);
    return nwsec_plcy_svcs;
}

static inline nwsec_policy_svc_t *
nwsec_policy_svc_alloc_and_init(void)
{
    return nwsec_policy_svc_init(nwsec_policy_svc_alloc());
}

static inline hal_ret_t
nwsec_policy_svc_free(nwsec_policy_svc_t *nwsec_plcy_rules)
{
    HAL_SPINLOCK_DESTROY(&nwsec_plcy_rules->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NWSEC_POLICY_SVC, nwsec_plcy_rules);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
//  APIs related to nwsec_policy_appid_t
//_____________________________________________________________________________
static inline nwsec_policy_appid_t *
nwsec_policy_appid_alloc(void)
{
    nwsec_policy_appid_t *nwsec_plcy_appid;
    nwsec_plcy_appid = (nwsec_policy_appid_t *)
                        g_hal_state->nwsec_policy_appid_slab()->alloc();
    return nwsec_plcy_appid;
}

static inline nwsec_policy_appid_t *
nwsec_policy_appid_init(nwsec_policy_appid_t *nwsec_plcy_appids)
{
    if (!nwsec_plcy_appids) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&nwsec_plcy_appids->slock, PTHREAD_PROCESS_PRIVATE);
    dllist_reset(&nwsec_plcy_appids->lentry);
    return nwsec_plcy_appids;
}

static inline nwsec_policy_appid_t *
nwsec_policy_appid_alloc_and_init(void)
{
    return nwsec_policy_appid_init(nwsec_policy_appid_alloc());
}

static inline hal_ret_t
nwsec_policy_appid_free(nwsec_policy_appid_t *nwsec_plcy_rules)
{
    HAL_SPINLOCK_DESTROY(&nwsec_plcy_rules->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NWSEC_POLICY_APPID, nwsec_plcy_rules);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
//  APIs related to nwsec_policy_rules_t
//_____________________________________________________________________________
static inline nwsec_policy_rules_t *
nwsec_policy_rules_alloc(void)
{
    nwsec_policy_rules_t *nwsec_plcy_rules;
    nwsec_plcy_rules = (nwsec_policy_rules_t *)
                        g_hal_state->nwsec_policy_rules_slab()->alloc();
    if (nwsec_plcy_rules == NULL) {
        return NULL;
    }
    return nwsec_plcy_rules;
}

static inline nwsec_policy_rules_t *
nwsec_policy_rules_init(nwsec_policy_rules_t *nwsec_plcy_rules)
{
    if (!nwsec_plcy_rules) {
        return NULL;
    }
    dllist_reset(&nwsec_plcy_rules->fw_svc_list_head);
    dllist_reset(&nwsec_plcy_rules->appid_list_head);
    dllist_reset(&nwsec_plcy_rules->lentry);
    HAL_SPINLOCK_INIT(&nwsec_plcy_rules->slock, PTHREAD_PROCESS_PRIVATE);

    return nwsec_plcy_rules;
}

static inline nwsec_policy_rules_t *
nwsec_policy_rules_alloc_and_init(void)
{
    return nwsec_policy_rules_init(nwsec_policy_rules_alloc());
}

static inline hal_ret_t
nwsec_policy_rules_free(nwsec_policy_rules_t *nwsec_plcy_rules)
{
    HAL_SPINLOCK_DESTROY(&nwsec_plcy_rules->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NWSEC_POLICY_RULES, nwsec_plcy_rules);
    return HAL_RET_OK;
}
//-----------------------------------------------------------------------------
//  APIs related to nwsec_policy_cfg_t
//_____________________________________________________________________________

extern void *
nwsec_policy_cfg_get_key_func (void *entry);

extern uint32_t
nwsec_policy_cfg_compute_hash_func (void *key, uint32_t ht_size);

extern bool
nwsec_policy_cfg_compare_key_func (void *key1, void *key2);

// max. number of SGs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NW_SEC_POLICY_CFG  16536
#define HAL_MAX_NW_SEC_POLICY      256

// nwsec_policy_cfg related functions

static inline nwsec_policy_cfg_t *
nwsec_policy_cfg_alloc(void)
{
    nwsec_policy_cfg_t     *nwsec_plcy_cfg;

    nwsec_plcy_cfg = (nwsec_policy_cfg_t *)
                              g_hal_state->nwsec_policy_cfg_slab()->alloc();
    if (nwsec_plcy_cfg == NULL) {
        return NULL;
    }
    return nwsec_plcy_cfg;
}

// initialize a security policy cfg instance
static inline nwsec_policy_cfg_t *
nwsec_policy_cfg_init (nwsec_policy_cfg_t *nwsec_plcy_cfg)
{
    if (!nwsec_plcy_cfg) {
        return NULL;
    }
    // initialize the operational state

    // initialize the meta information
    nwsec_plcy_cfg->ht_ctxt.reset();
    nwsec_plcy_cfg->hal_handle = HAL_HANDLE_INVALID;
    dllist_reset(&nwsec_plcy_cfg->rules_head);
    return nwsec_plcy_cfg;
}

// allocate and initialize a security policy cfg instance
static inline nwsec_policy_cfg_t *
nwsec_policy_cfg_alloc_init (void)
{
    return nwsec_policy_cfg_init(nwsec_policy_cfg_alloc());
}

// free security policy cfg instance
static inline hal_ret_t
nwsec_policy_cfg_free(nwsec_policy_cfg_t *nwsec_plcy_cfg)
{
    hal::delay_delete_to_slab(HAL_SLAB_NWSEC_POLICY_CFG, nwsec_plcy_cfg);
    return HAL_RET_OK;
}

// insert a security policy to meta data strucutre
static inline hal_ret_t
add_nwsec_policy_cfg_to_db (nwsec_policy_cfg_t *nwsec_plcy_cfg)
{
    g_hal_state->nwsec_policy_cfg_ht()->insert(nwsec_plcy_cfg,
                                               &nwsec_plcy_cfg->ht_ctxt);
    return HAL_RET_OK;
}


// find a security policy by sg
static inline nwsec_policy_cfg_t *
nwsec_policy_cfg_lookup_by_key(nwsec_policy_key_t plcy_key)
{
    return (nwsec_policy_cfg_t *)
         g_hal_state->nwsec_policy_cfg_ht()->lookup(&plcy_key);
}

/** 
  * Security Group related config
  *
**/
extern void *
nwsec_group_get_key_func (void *entry);

extern uint32_t
nwsec_group_compute_hash_func (void *key, uint32_t ht_size);

extern bool
nwsec_group_compare_key_func (void *key1, void *key2);

// max. number of SGs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_NW_SEC_GROUP_CFG  8192

// nwsec_policy_cfg related functions

static inline nwsec_group_t *
nwsec_group_alloc(void)
{
    nwsec_group_t     *nwsec_grp;

    nwsec_grp = (nwsec_group_t *)
                              g_hal_state->nwsec_group_slab()->alloc();
    if (nwsec_grp == NULL) {
        return NULL;
    }
    return nwsec_grp;
}

// initialize a security policy cfg instance
static inline nwsec_group_t *
nwsec_group_init (nwsec_group_t *nwsec_grp)
{
    if (!nwsec_grp) {
        return NULL;
    }
    // initialize the operational state

    // initialize the meta information
    nwsec_grp->ht_ctxt.reset();
    nwsec_grp->hal_handle = HAL_HANDLE_INVALID;
    dllist_reset(&nwsec_grp->nw_list_head);
    dllist_reset(&nwsec_grp->ep_list_head);
    HAL_SPINLOCK_INIT(&nwsec_grp->slock, PTHREAD_PROCESS_PRIVATE);
 
    return nwsec_grp;
}

// allocate and initialize a security policy cfg instance
static inline nwsec_group_t *
nwsec_group_alloc_init (void)
{
    return nwsec_group_init(nwsec_group_alloc());
}

// free security policy cfg instance
static inline hal_ret_t
nwsec_group_free(nwsec_group_t *nwsec_grp)
{
    HAL_SPINLOCK_DESTROY(&nwsec_grp->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NWSEC_GROUP, nwsec_grp);
    return HAL_RET_OK;
}

// insert a security policy to meta data strucutre
static inline hal_ret_t
add_nwsec_group_to_db (nwsec_group_t *nwsec_grp)
{
    hal_ret_t                       ret;
    sdk_ret_t                       sdk_ret;
    hal_handle_id_ht_entry_t        *entry;

    HAL_TRACE_DEBUG("Adding to security group hash table sg_id {}", nwsec_grp->sg_id);
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
            hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }
    // add mapping from security group id to its handle
    entry->handle_id   = nwsec_grp->hal_handle;

    sdk_ret = g_hal_state->nwsec_group_ht()->insert_with_key(&nwsec_grp->sg_id, entry,
                                                             &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add security group {} to handle mapping, "
                      "err : {}", nwsec_grp->sg_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);;
    //nwsec_grp->hal_handle = handle_id;
    return ret;
}

// find a security group by key
static inline nwsec_group_t *
nwsec_group_lookup_by_key(uint32_t sg_id)
{
    hal_handle_id_ht_entry_t     *entry;
    nwsec_group_t                *nwsec_group;

    HAL_TRACE_DEBUG("Lookup sg_id {}", sg_id);
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->nwsec_group_ht()->lookup(&sg_id);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_SECURITY_GROUP);
        nwsec_group = (nwsec_group_t *) hal_handle_get_obj(entry->handle_id);
        return nwsec_group;
    } else {
        HAL_TRACE_DEBUG("security group id:{} entry not found", sg_id);
    }

    return NULL;
}

// find a security group by handle
static inline nwsec_group_t *
nwsec_group_lookup_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("failed to find hal object with handle:{}",
                        handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_SECURITY_GROUP) {
        HAL_TRACE_DEBUG("failed to find security_group  with handle:{}",
                        handle);
        return NULL;
    }
    return (nwsec_group_t *)hal_handle->get_obj();
}

//APIs to be consumed by other features
nwsec_group_t *
nwsec_group_lookup_key_or_handle(const kh::SecurityGroupKeyHandle& key_or_handle);
dllist_ctxt_t *
get_ep_list_for_security_group(uint32_t sg_id);
hal_ret_t
add_ep_to_security_group(uint32_t sg_id, hal_handle_t ep_handle);
hal_ret_t
del_ep_from_security_group(uint32_t sg_id, hal_handle_t ep_handle);

hal_ret_t
add_nw_to_security_group(uint32_t sg_id, hal_handle_t nw_handle);
hal_ret_t
del_nw_from_security_group(uint32_t sg_id, hal_handle_t nw_handle);
dllist_ctxt_t *
get_nw_list_for_security_group(uint32_t sg_id);


//------------------------------------------------------------------
//
//
// Security policy rules - V3
//
//
//

#define MAX_VERSION       1

#define MAX_TENANTS     1
extern acl::acl_config_t             ip_acl_config;


void *
nwsec_policy_get_key_func (void *entry);

uint32_t
nwsec_policy_compute_hash_func (void *key, uint32_t ht_size);

bool
nwsec_policy_compare_key_func (void *key1, void *key2);


void *
nwsec_rule_get_key_func (void *entry);

uint32_t
nwsec_rule_compute_hash_func (void *key, uint32_t ht_size);

bool
nwsec_rule_compare_key_func (void *key1, void *key2);
typedef struct app_list {
    dllist_ctxt_t   dlentry;
} app_list_t;

typedef struct app_group_list_s {
    dllist_ctxt_t   dlentry;
} app_group_list_t;

typedef struct port_range {
    int port_lo;
    int port_hi;
} port_range_t;

typedef struct sg_range {
    int sg_lo;
    int sg_hi;
} sg_range_t;

#define FIELD_DEF(name, type, fieldname, fld_sz) typedef struct name { \
    type            fieldname[fld_sz];  \
    dllist_ctxt_t   dlentry; \
} __PACK__ name;

#define FIELD_ALLOC_MEM(type, fieldname)  (fieldname *) HAL_TRACE_MALLOC(hal::HAL_MEM_ALLOC_## type, 1);

#define FIELD_FREE(type, ptr)       HAL_FREE(hal::HAL_MEM_ALLOC_ ## type, ptr);

FIELD_DEF(address_field_t, ipvx_range_t, addr, 10);
FIELD_DEF(sg_field_t, sg_range_t, sg, 10);
FIELD_DEF(port_range_field_t, port_range_t, port, 10);

typedef struct nwsec_rule_s {
    uint64_t              rule_id;
    uint32_t              rule_no;
    bool                  enable;
    //nwsec_rule_action     action;
    bool                  count_en;
    dllist_ctxt_t         app_list_head;
    dllist_ctxt_t         app_group_list_head;
    address_field_t       src_address;
    int                   src_addr_len;
    sg_field_t            src_sg;
    int                   src_sg_len;
    address_field_t       dst_address;
    int                   dst_addr_len;
    sg_field_t            dst_sg;
    int                   dst_sg_len;
    port_range_field_t    dst_port_range;
    int                   dst_port_len;
    port_range_field_t    src_port_range;
    int                   src_port_len;
    uint32_t              hash_value;
    dllist_ctxt_t         dlentry;
    ht_ctxt_t             ht_ctxt;
    uint32_t              priority;
    hal_handle_t          hal_handle;
    acl::ref_t            ref_count;
} nwsec_rule_t;

typedef struct policy_key_s {
    uint32_t        policy_id;
    uint32_t        vrf_id_or_handle;
}policy_key_t;
#define         MAX_RULES       1024
typedef struct nwsec_policy_s {
    policy_key_t    key;
    int             version;
    uint32_t        rule_len;
    nwsec_rule_t   *dense_rules[MAX_RULES]; // Dense rules will be linearly arranged hashes with pointer to the rules.
    ht             *rules_ht[MAX_VERSION];
    hal_handle_t    hal_handle;
    acl_ctx_t      *acl_ctx;
    ht_ctxt_t       ht_ctxt;
} nwsec_policy_t;


static inline nwsec_policy_t *
nwsec_policy_alloc(void)
{
    nwsec_policy_t     *policy = NULL;

    policy = (nwsec_policy_t *)
                        g_hal_state->nwsec_policy_cfg_slab()->alloc();
    if (policy == NULL) {
        return NULL;
    }
    return policy;
}

// Initialize a nwsec_policy instance
static inline nwsec_policy_t *
nwsec_policy_init (nwsec_policy_t *policy)
{
    if (!policy) {
        return NULL;
    }
    memset(&policy->key, 0, sizeof(policy_key_t));

    for (int i = 0; i < MAX_VERSION; i++) {
        policy->rules_ht[policy->version] = ht::factory(HAL_MAX_NW_SEC_GROUP_CFG,
                                                        hal::nwsec_rule_get_key_func,
                                                        hal::nwsec_rule_compute_hash_func,
                                                        hal::nwsec_rule_compare_key_func);
        HAL_ASSERT_RETURN((policy->rules_ht != NULL), NULL);
    }
    policy->ht_ctxt.reset();
    policy->acl_ctx = (acl::acl_ctx_t *)lib_acl_create(&ip_acl_config);

    return policy;
}

// allocate and initialize a match_template
static inline nwsec_policy_t *
nwsec_policy_alloc_init()
{
    return nwsec_policy_init(nwsec_policy_alloc());
}

// free
//
static inline hal_ret_t
nwsec_policy_free(nwsec_policy_t *policy)
{
    for (uint32_t rule_index = 0; rule_index < policy->rule_len; rule_index++) {
        ref_dec(&policy->dense_rules[rule_index]->ref_count);
    }

    // Free dense rules one we make it dynamic array
    //HAL_MEM_FREE(policy->dense_rules);
    g_hal_state->nwsec_policy_cfg_slab()->free(policy);
    return HAL_RET_OK;


}

static inline hal_ret_t
add_nwsec_policy_to_db (nwsec_policy_t *policy)
{
    sdk_ret_t                  sdk_ret;
    hal_handle_id_ht_entry_t  *entry;

    HAL_TRACE_DEBUG("adding policy to hash table");

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
            hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    entry->handle_id = policy->hal_handle;

    sdk_ret = g_hal_state->nwsec_policy_ht()->insert_with_key(&policy->key,
                                                              entry,
                                                              &policy->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to security policy  {} to policy db,"
                      "err : {}", policy->key.policy_id);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    return hal_sdk_ret_to_hal_ret(sdk_ret);

}

static inline hal_ret_t
del_nwsec_policy_from_db (nwsec_policy_t *policy)
{
    hal_handle_id_ht_entry_t  *entry;

    HAL_TRACE_DEBUG("removing policy from hash table");

    entry = (hal_handle_id_ht_entry_t *) 
             g_hal_state->nwsec_policy_ht()->remove(&policy->key);
    if (entry) {
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    return HAL_RET_OK;

}

// find a security policy by sg
static inline nwsec_policy_t *
find_nwsec_policy_by_key(uint32_t policy_id, uint32_t vrf_id_or_handle)
{
    hal_handle_id_ht_entry_t    *entry;
    policy_key_t                 policy_key = { 0 };
    nwsec_policy_t              *policy;

    policy_key.policy_id = policy_id;
    policy_key.vrf_id_or_handle = vrf_id_or_handle;
    entry = (hal_handle_id_ht_entry_t *) g_hal_state->
            nwsec_policy_ht()->lookup(&policy_key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == HAL_OBJ_ID_SECURITY_POLICY);
        policy = (nwsec_policy_t *)hal_handle_get_obj(entry->handle_id);
        return policy;
    }
    return NULL;

}

static inline nwsec_policy_t *
find_nwsec_policy_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("failed to find hal object with handle:{}",
                        handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_SECURITY_POLICY) {
        HAL_TRACE_DEBUG("failed to find security_group  with handle:{}",
                        handle);
        return NULL;
    }
    return (nwsec_policy_t *)hal_handle->get_obj();
}

/** Rules within a security policy **/
static inline nwsec_rule_t *
nwsec_rule_alloc(void)
{
    nwsec_rule_t     *rule = NULL;

    rule = (nwsec_rule_t *)
                        g_hal_state->nwsec_rule_slab()->alloc();
    if (rule == NULL) {
        return NULL;
    }
    return rule;
}

// Initialize a nwsec_rule instance
static inline nwsec_rule_t *
nwsec_rule_init (nwsec_rule_t *rule)
{
    if (!rule) {
        return NULL;
    }

    rule->ht_ctxt.reset();
    ref_init(&rule->ref_count, [] (const ref_t * ref) {
        nwsec_rule_t * rule = container_of(ref, nwsec_rule_t, ref_count);
        g_hal_state->nwsec_rule_slab()->free(rule);
    });
    ref_inc(&rule->ref_count);

    dllist_reset(&rule->app_list_head);
    dllist_reset(&rule->app_group_list_head);
    dllist_reset(&rule->dlentry);
    return rule;
}

// allocate and initialize a match_template
static inline nwsec_rule_t *
nwsec_rule_alloc_init()
{
    return nwsec_rule_init(nwsec_rule_alloc());
}


static inline hal_ret_t
add_nwsec_rule_to_db (nwsec_policy_t *policy, nwsec_rule_t *rule, int rule_index)
{
    policy->rules_ht[policy->version]->insert(rule,
                                     &rule->ht_ctxt);
    policy->dense_rules[rule_index] = rule;

    return HAL_RET_OK;
}

// find a network rule by id??
static inline nwsec_rule_t *
nwsec_rule_lookup_by_key(nwsec_policy_t *policy, nwsec_rule_t *rule)
{
    int version = policy->version;
    return ((nwsec_rule_t *)policy->rules_ht[version]->lookup(&rule->hash_value));

}

static inline hal_ret_t
nwsec_rule_free(nwsec_rule_t *rule)
{
    return  HAL_RET_OK;
}


// Rule DD related
// nwsec_rules_t rules are expanded to rules of type ipv4_rules and are inserted
// into the library using acl_* calls

struct ipv4_tuple {
    uint8_t   proto;
    uint32_t  ip_src;
    uint32_t  ip_dst;
    uint32_t  port_src;
    uint32_t  port_dst;
    uint32_t  src_sg;
    uint32_t  dst_sg;
};

enum {
    PROTO = 0, IP_SRC, IP_DST, PORT_SRC, PORT_DST, SRC_SG, DST_SG,
    NUM_FIELDS
};



#define ACL_FLD_DEF(typ, struct_name, fld_name)      \
    {typ, sizeof(((struct_name*)0)->fld_name),   \
            offsetof(struct_name, fld_name) }

/* Expanded field defs
struct nwsec_flow_field_def flow_defs[5] = {
    {
        .type = ACL_FIELD_TYPE_EXACT
        .size = sizeof(uint8_t)
        .offset = offsetof (struct ipv4_tuple, proto);
    },
    {
        .type = ACL_FIELD_TYPE_RANGE,
        .size = sizeof(uint32_t),
        .offset = offsetof (struct ipv4_tuple, ip_src);
    },

    {
        .type = ACL_FIELD_TYPE_RANGE,
        .size = sizeof(uint32_t),
        .offset = offsetof (struct ipv4_tuple, ip_dst);
    },
    {
        .type = ACL_FIELD_TYPE_RANGE,
        .size = sizeof(uint32_t),
        .offset = offsetof (struct ipv4_tuple, port_src);
    },

    {
        .type = ACL_FIELD_TYPE_RANGE,
        .size = sizeof(uint32_t),
        .offset = offsetof (struct ipv4_tuple, port_dst);
    },

}*/

ACL_RULE_DEF(ipv4_rule_t, NUM_FIELDS);

hal_ret_t nwsec_policy_init();

typedef struct nwsec_rule_create_app_ctxt_s {
} __PACK__ nwsec_rule_create_app_ctxt_t;



} // namespace hal

#endif    // __NWSEC_GROUP_HPP__

