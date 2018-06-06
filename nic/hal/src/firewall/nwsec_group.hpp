//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NWSEC_GROUP_HPP__
#define __NWSEC_GROUP_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/kh.pb.h"
#include "nic/include/pd.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nic/hal/src/utils/rule_match.hpp"


using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;
using acl::acl_config_t;
using acl::ref_t;
using namespace acl;

using nwsec::Service;
using types::IPProtocol;
using types::ICMPMsgType;
using nwsec::FirewallAction;
using nwsec::ALGName;

namespace hal {

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

typedef struct nwsec_policy_appid_s {
    hal_spinlock_t      slock;              // Lock to protect this structure
    uint32_t            appid;
    dllist_ctxt_t       lentry;
} __PACK__ nwsec_policy_appid_t;

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
    HAL_SPINLOCK_INIT(&nwsec_plcy_appids->slock, PTHREAD_PROCESS_SHARED);
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
    HAL_SPINLOCK_INIT(&nwsec_grp->slock, PTHREAD_PROCESS_SHARED);

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
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);;
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add security group {} to handle mapping, "
                      "err : {}", nwsec_grp->sg_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
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
    return (nwsec_group_t *)hal_handle->obj();
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


#define MAX_VERSION       1

#define MAX_TENANTS     1
extern acl::acl_config_t             ip_acl_config;


void *nwsec_policy_get_key_func(void *entry);
uint32_t nwsec_policy_compute_hash_func(void *key, uint32_t ht_size);
bool nwsec_policy_compare_key_func(void *key1, void *key2);

void *
nwsec_rule_get_key_func (void *entry);

uint32_t
nwsec_rule_compute_hash_func (void *key, uint32_t ht_size);

bool
nwsec_rule_compare_key_func (void *key1, void *key2);
typedef struct app_list {
    dllist_ctxt_t   dlentry;
} app_list_t;

#define FIELD_DEF(name, type, fieldname, fld_sz) typedef struct name { \
    type            fieldname[fld_sz];  \
    dllist_ctxt_t   dlentry; \
} __PACK__ name;

#define FIELD_ALLOC_MEM(type, fieldname)  (fieldname *) HAL_TRACE_MALLOC(hal::HAL_MEM_ALLOC_## type, 1);

#define FIELD_FREE(type, ptr)       HAL_FREE(hal::HAL_MEM_ALLOC_ ## type, ptr);

typedef struct fw_action_s {
    nwsec::SecurityAction    sec_action;
    nwsec::LogAction         log_action;
    nwsec::ALGName           alg;
    //TBD:lseshan 
    //app_data_t               app_data;  Multiple app data - This should be
    //replaced by dllist_ctxt_t app_data
} fw_action_t;

typedef uint64_t rule_key_t;
typedef struct nwsec_rule_s {
    rule_key_t            rule_id;
    rule_match_t          fw_rule_match;        // Rule match conditions
    fw_action_t           fw_rule_action;       // Rule actionsd
    dllist_ctxt_t         appid_list_head;

    //Operationl state of rules
    uint32_t              hash_value;
    dllist_ctxt_t         dlentry;
    ht_ctxt_t             ht_ctxt;      // Hash context when this data strucutre is stored in nwsec_policy_t->hash_tree
    uint32_t              priority;
    hal_handle_t          hal_handle;
    acl::ref_t            ref_count;
} nwsec_rule_t;

#define APP_MATCH(match)  match.app

typedef struct policy_key_s {
    uint32_t         policy_id;
    vrf_id_t         vrf_id;
} policy_key_t;

#define         MAX_RULES       1024
typedef struct nwsec_policy_s {
    policy_key_t    key;
    int             version;
    uint32_t        rule_len;
    nwsec_rule_t   *dense_rules[MAX_RULES]; // Dense rules will be linearly arranged hashes with pointer to the rules.
    ht             *rules_ht[MAX_VERSION];  // Hash table of rules
    hal_handle_t    hal_handle;
    const acl_ctx_t *acl_ctx;  // lib acl context needed by acl lib
    ht_ctxt_t       ht_ctxt;  // Hash context for storing it in the config db
    acl::ref_t      ref_count;
} nwsec_policy_t;

static inline nwsec_policy_t *
nwsec_policy_alloc(void)
{
    nwsec_policy_t     *policy = NULL;

    policy = (nwsec_policy_t *)
                        g_hal_state->nwsec_policy_slab()->alloc();
    if (policy == NULL) {
        return NULL;
    }
    return policy;
}

static inline const char *nwsec_acl_ctx_name(vrf_id_t vrf_id)
{
    thread_local static char name[ACL_NAMESIZE];

    std::snprintf(name, sizeof(name), "nwsec-ipv4-rules:%lu", vrf_id);

    return name;
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
    policy->acl_ctx = NULL;

    return policy;
}

// allocate and initialize a match_template
static inline nwsec_policy_t *
nwsec_policy_alloc_init()
{
    return nwsec_policy_init(nwsec_policy_alloc());
}

static inline hal_ret_t
nwsec_policy_rules_free(nwsec_policy_t *policy)
{
    for (uint32_t rule_index = 0; rule_index < policy->rule_len; rule_index++) {
        ref_dec(&policy->dense_rules[rule_index]->ref_count);
    }
    return HAL_RET_OK;
}

// free
//
static inline hal_ret_t
nwsec_policy_free(nwsec_policy_t *policy)
{
    nwsec_policy_rules_free(policy);
    // Free dense rules one we make it dynamic array
    //HAL_MEM_FREE(policy->dense_rules);
    g_hal_state->nwsec_policy_slab()->free(policy);
    return HAL_RET_OK;
}

static inline hal_ret_t
add_nwsec_policy_to_db (nwsec_policy_t *policy)
{
    hal_ret_t                 ret;
    sdk_ret_t                 sdk_ret;
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
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add security policy  {} to policy db,"
                      "err : {}", policy->key.policy_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    return ret;

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
find_nwsec_policy_by_key(uint32_t policy_id, uint32_t vrf_id)
{
    hal_handle_id_ht_entry_t    *entry;
    policy_key_t                 policy_key = { 0 };
    nwsec_policy_t              *policy;

    policy_key.policy_id = policy_id;
    policy_key.vrf_id = vrf_id;
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
    return (nwsec_policy_t *)hal_handle->obj();
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
        HAL_TRACE_DEBUG("Calling rule free");
        g_hal_state->nwsec_rule_slab()->free(rule);
    });
    rule_match_init(&rule->fw_rule_match);
    dllist_reset(&rule->dlentry);
    dllist_reset(&rule->appid_list_head);
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

hal_ret_t nwsec_policy_init();

typedef struct nwsec_policy_create_app_ctxt_s {
    const acl_ctx_t    *acl_ctx;
} __PACK__ nwsec_policy_create_app_ctxt_t;

typedef struct nwsec_policy_upd_app_ctxt_s {
    const acl_ctx_t    *acl_ctx;
    const acl_ctx_t    *acl_ctx_clone;
} __PACK__ nwsec_policy_upd_app_ctxt_t;

} // namespace hal

#endif    // __NWSEC_GROUP_HPP__

