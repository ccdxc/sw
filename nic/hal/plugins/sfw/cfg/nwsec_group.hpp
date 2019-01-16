//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NWSEC_GROUP_HPP__
#define __NWSEC_GROUP_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/kh.pb.h"
#include "nic/include/pd.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nic/hal/src/utils/rule_match.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"

#define MAX_UUID_SZ 128

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

typedef struct rpc_programid_ {
    char     program_id[MAX_UUID_SZ];
    uint32_t timeout;
} rpc_programid_t;

typedef struct nwsec_policy_appid_s {
    sdk_spinlock_t      slock;              // Lock to protect this structure
    uint32_t            appid;
    dllist_ctxt_t       lentry;
} __PACK__ nwsec_policy_appid_t;

typedef struct nwsec_group_s {
    sdk_spinlock_t     slock;
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
    SDK_SPINLOCK_INIT(&nwsec_plcy_appids->slock, PTHREAD_PROCESS_SHARED);
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
    SDK_SPINLOCK_DESTROY(&nwsec_plcy_rules->slock);
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
    SDK_SPINLOCK_INIT(&nwsec_grp->slock, PTHREAD_PROCESS_SHARED);

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
    SDK_SPINLOCK_DESTROY(&nwsec_grp->slock);
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


static inline hal_ret_t
del_nwsec_group_from_db (nwsec_group_t *nwsec_group)
{
    hal_handle_id_ht_entry_t *entry;
    HAL_TRACE_DEBUG("removing security group from the hash table");

    entry = (hal_handle_id_ht_entry_t *)
            g_hal_state->nwsec_group_ht()->remove(&nwsec_group->sg_id);
    if (entry) {
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    } else {
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
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

bool
securitypolicy_is_allow(vrf_id_t svrf_id, hal::ipv4_tuple *acl_key);

#define MAX_VERSION       1

#define MAX_TENANTS     1
#define NWSEC_POLICY_DEFAULT false
    
extern acl::acl_config_t             ip_acl_config;

typedef struct app_list {
    dllist_ctxt_t   dlentry;
} app_list_t;

#define FIELD_DEF(name, type, fieldname, fld_sz) typedef struct name { \
    type            fieldname[fld_sz];  \
    dllist_ctxt_t   dlentry; \
} __PACK__ name;

#define FIELD_ALLOC_MEM(type, fieldname)  (fieldname *) HAL_TRACE_MALLOC(hal::HAL_MEM_ALLOC_## type, 1);

#define FIELD_FREE(type, ptr)       HAL_FREE(hal::HAL_MEM_ALLOC_ ## type, ptr);

typedef struct alg_options {
    union opt_ {
        struct ftp_options {
            bool allow_mismatch_ip_address;
        } ftp_opts;
        struct dns_options {
            bool      drop_multi_question_packets;
            bool      drop_large_domain_name_packets;
            bool      drop_long_label_packets;
            bool      drop_multizone_packets;
            uint32_t  max_msg_length;
            uint32_t  query_response_timeout;
        } dns_opts;

        struct msrpc_options {
            uint8_t           uuid_sz;
            rpc_programid_t  *uuids;
        } msrpc_opts;

        struct sunrpc_options {
            uint8_t           programid_sz;
            rpc_programid_t  *program_ids;
        } sunrpc_opts;
        
        struct sip_options {
            uint32_t ctimeout;
            uint32_t dscp_code_point;
            uint32_t media_inactivity_timeout;
            uint32_t max_call_duration;
            uint32_t t1_timer_value;
            uint32_t t4_timer_value;
        } sip_opts;
    } opt;
} alg_opts;
    

typedef struct fw_action_s {
    nwsec::SecurityAction    sec_action;
    nwsec::LogAction         log_action;
    nwsec::ALGName           alg;
    alg_opts                 app_options;
    uint32_t                 idle_timeout;
    //TBD:lseshan 
    //app_data_t               app_data;  Multiple app data - This should be
    //replaced by dllist_ctxt_t app_data
} fw_action_t;

typedef struct nwsec_rule_s {
    rule_key_t            rule_id;
    rule_match_t          fw_rule_match;        // Rule match conditions
    fw_action_t           fw_rule_action;       // Rule actionsd
    dllist_ctxt_t         appid_list_head;

    //Operationl state of rules
    dllist_ctxt_t         dlentry;
    uint32_t              priority;
    acl::ref_t            ref_count;
} nwsec_rule_t;


typedef struct nwsec_rulelist_s {
    rule_key_t          rule_id;
    dllist_ctxt_t       head;
    ht_ctxt_t           ht_ctxt;
    hal_handle_t        hal_handle;
    acl::ref_t          ref_count;
} nwsec_rulelist_t;

#define APP_MATCH(match)  match.app

typedef struct policy_key_s {
    uint32_t         policy_id;
    vrf_id_t         vrf_id;
} policy_key_t;

typedef struct nwsec_policy_s {
    policy_key_t    key;
    int             version;
    uint32_t        rule_len;
    ht              *rules_ht[MAX_VERSION];  // Hash table of rules
    hal_handle_t    hal_handle;
    const acl_ctx_t *acl_ctx;  // lib acl context needed by acl lib
    ht_ctxt_t       ht_ctxt;  // Hash context for storing it in the config db
    acl::ref_t      ref_count;
} nwsec_policy_t;


hal_ret_t nwsec_policy_init();

void *nwsec_policy_get_key_func(void *entry);
uint32_t nwsec_policy_compute_hash_func(void *key, uint32_t ht_size);
bool nwsec_policy_compare_key_func(void *key1, void *key2);
const char *nwsec_acl_ctx_name(vrf_id_t vrf_id);
nwsec_policy_t *
find_nwsec_policy_by_key(uint32_t policy_id, uint32_t vrf_id);

void *
nwsec_rule_get_key_func (void *entry);

uint32_t
nwsec_rule_compute_hash_func (void *key, uint32_t ht_size);

bool
nwsec_rule_compare_key_func (void *key1, void *key2);

typedef struct nwsec_policy_create_app_ctxt_s {
    const acl_ctx_t    *acl_ctx;
} __PACK__ nwsec_policy_create_app_ctxt_t;

typedef struct nwsec_policy_upd_app_ctxt_s {
    const acl_ctx_t    *acl_ctx;
    const acl_ctx_t    *acl_ctx_clone;
} __PACK__ nwsec_policy_upd_app_ctxt_t;

} // namespace hal

#endif    // __NWSEC_GROUP_HPP__

