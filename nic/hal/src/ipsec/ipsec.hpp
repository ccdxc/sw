#ifndef __IPSEC_HPP__
#define __IPSEC_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/ipsec.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/utils/rule_match.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using ipsec::IpsecSAEncrypt;
using ipsec::IpsecSADecrypt;
using ipsec::IpsecRuleSpec;
using kh::IpsecSAEncryptKeyHandle;
using kh::IpsecSADecryptKeyHandle;
using kh::IpsecRuleKeyHandle;

using ipsec::IpsecRuleRequestMsg;
using ipsec::IpsecRuleStatus;
using ipsec::IpsecRuleResponse;
using ipsec::IpsecRuleResponseMsg;
using ipsec::IpsecRuleDeleteRequest;
using ipsec::IpsecRuleDeleteRequestMsg;
using ipsec::IpsecRuleDeleteResponseMsg;
using ipsec::IpsecRuleGetRequest;
using ipsec::IpsecRuleGetRequestMsg;
using ipsec::IpsecRuleGetResponseMsg;

using ipsec::IpsecSAEncryptRequestMsg;
using ipsec::IpsecSAEncryptStatus;
using ipsec::IpsecSAEncryptResponse;
using ipsec::IpsecSAEncryptResponseMsg;
using ipsec::IpsecSAEncryptDeleteRequest;
using ipsec::IpsecSAEncryptDeleteRequestMsg;
using ipsec::IpsecSAEncryptDeleteResponseMsg;
using ipsec::IpsecSAEncryptGetRequest;
using ipsec::IpsecSAEncryptGetResponse;
using ipsec::IpsecSAEncryptGetRequestMsg;
using ipsec::IpsecSAEncryptGetResponseMsg;

using ipsec::IpsecSADecryptRequestMsg;
using ipsec::IpsecSADecryptStatus;
using ipsec::IpsecSADecryptResponse;
using ipsec::IpsecSADecryptResponseMsg;
using ipsec::IpsecSADecryptDeleteRequest;
using ipsec::IpsecSADecryptDeleteRequestMsg;
using ipsec::IpsecSADecryptDeleteResponseMsg;
using ipsec::IpsecSADecryptGetRequest;
using ipsec::IpsecSADecryptGetResponse;
using ipsec::IpsecSADecryptGetRequestMsg;
using ipsec::IpsecSADecryptGetResponseMsg;

namespace hal {

#define MAX_IPSEC_KEY_SIZE  32
#define IPSEC_CB_RING_SIZE 256
#define IPSEC_BARCO_RING_SIZE 1024 

typedef uint64_t rule_id_t;
typedef uint64_t pol_id_t;

//------------------------------------------------------------------------------
// IPSec Policy (cfg) data structure layout
//
//    ipsec_policy <policy_key> {  policy_key = pol_id + vrf_id
//        ipsec_rule <rule_key> {  rule_key = rule_id
//
//            // match criteria for rule match
//            ipsec_rule_match {
//                list of src_addr range;
//                list of dst_addr range;
//
//                list of src-ports range;
//                list of dst-ports range;
//
//                list of src-security-group range;
//                list of dst-security-group range;
//
//                ESPInfo 
//            }
//
//            // action for rule match
//            ipsec_rule_action {
//                sa_action;
//                sa_action_enc_handle;
//                sa_action_dec_handle;
//            }
//        }
//    }
//
// IPSec Policy (oper) data structure layout
//
//
//------------------------------------------------------------------------------

typedef struct ipsec_cfg_rule_action_s {
    ipsec::IpsecSAActionType   sa_action;
    hal_handle_t      sa_action_enc_handle;
    hal_handle_t      sa_action_dec_handle;
    vrf_id_t    vrf;
} ipsec_cfg_rule_action_t;

typedef struct ipsec_cfg_rule_key_s {
    rule_id_t    rule_id;
} __PACK__ ipsec_cfg_rule_key_t;

typedef struct ipsec_cfg_rule_s {
    ipsec_cfg_rule_key_t     key;
    rule_match_t             match;
    ipsec_cfg_rule_action_t  action;

    // operational
    uint32_t                 prio;
    dllist_ctxt_t            list_ctxt;
    acl::ref_t               ref_count;
} ipsec_cfg_rule_t;

typedef struct ipsec_cfg_pol_key_s {
    pol_id_t    pol_id;
    vrf_id_t    vrf_id;
} __PACK__ ipsec_cfg_pol_key_t;

typedef struct ipsec_cfg_pol_create_app_ctxt_s {
    const acl::acl_ctx_t    *acl_ctx;
} __PACK__ ipsec_cfg_pol_create_app_ctxt_t;

typedef struct ipsec_cfg_pol_s {
    ipsec_cfg_pol_key_t    key;
    dllist_ctxt_t        rule_list;
//    dllist_ctxt_t        list_ctxt;
    ht_ctxt_t            ht_ctxt;

    // operational
    hal_spinlock_t       slock;
    hal_handle_t         hal_hdl;
} ipsec_cfg_pol_t;

typedef struct ipsec_sa_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    ipsec_sa_id_t         sa_id;                   // CB id
    ip_addr_t             tunnel_sip4;
    ip_addr_t             tunnel_dip4;
    uint8_t               iv_size;
    uint8_t               icv_size;
    uint8_t               block_size;
    int32_t               key_index;
    //types::CryptoKeyType  key_type;
    uint32_t              key_type;
    uint32_t              key_size;
    uint8_t               key[MAX_IPSEC_KEY_SIZE];
    int32_t               new_key_index;
    //types::CryptoKeyType  new_key_type;
    uint32_t              new_key_type;
    uint32_t              new_key_size;
    uint8_t               new_key[MAX_IPSEC_KEY_SIZE];
    uint32_t              barco_enc_cmd;
    uint64_t              iv;
    uint32_t              iv_salt;
    uint32_t              esn_hi;
    uint32_t              esn_lo;
    uint32_t              spi;
    uint32_t              new_spi;
    uint32_t              expected_seq_no;
    uint64_t              seq_no_bmp;
    hal_handle_t          hal_handle;              // HAL allocated handle
    mac_addr_t            smac;
    mac_addr_t            dmac; 
    uint8_t               is_v6; 
    uint8_t               is_nat_t;
    uint8_t               is_random;
    uint8_t               extra_pad;
    uint8_t               flags;
    uint64_t              vrf;
    uint64_t              vrf_handle;
    uint16_t              vrf_vlan;
    uint32_t              last_replay_seq_no;
    // PD state
    void                  *pd;                     // all PD specific state
    void                  *pd_decrypt;                     // all PD specific state
    uint16_t              cb_pindex;
    uint16_t              cb_cindex;
    uint16_t              barco_pindex;
    uint16_t              barco_cindex;
    uint64_t              total_pkts;
    uint64_t              total_bytes;
    uint64_t              total_drops;

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ ipsec_sa_t;


typedef struct ipsec_rule_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    ipsec_rule_id_t       rule_id;                   // CB id
    hal_handle_t          hal_handle;              // HAL allocated handle
    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ ipsec_rule_t;

// max. number of CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_IPSEC_SA                          2048 
#define IPSEC_BARCO_ENCRYPT_AES_GCM_256           0x30000000
#define IPSEC_BARCO_DECRYPT_AES_GCM_256           0x30100000

//-----------------------------------------------------------------------------
// Inline functions
//-----------------------------------------------------------------------------

// allocate a ipsec_sament instance
static inline ipsec_sa_t *
ipsec_sa_alloc (void)
{
    ipsec_sa_t    *ipsec_sa;

    ipsec_sa = (ipsec_sa_t *)g_hal_state->ipsec_sa_slab()->alloc();
    if (ipsec_sa == NULL) {
        return NULL;
    }
    return ipsec_sa;
}

// initialize a ipsec_sament instance
static inline ipsec_sa_t *
ipsec_sa_init (ipsec_sa_t *ipsec_sa)
{
    if (!ipsec_sa) {
        return NULL;
    }
    memset(ipsec_sa, 0, sizeof(ipsec_sa_t));
 
    HAL_SPINLOCK_INIT(&ipsec_sa->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    ipsec_sa->pd = NULL;

    // initialize meta information
    ipsec_sa->ht_ctxt.reset();
    ipsec_sa->hal_handle_ht_ctxt.reset();

    return ipsec_sa;
}

// allocate and initialize a IPSECCB instance
static inline ipsec_sa_t *
ipsec_sa_alloc_init (void)
{
    return ipsec_sa_init(ipsec_sa_alloc());
}

static inline hal_ret_t
ipsec_sa_free (ipsec_sa_t *ipsec_sa)
{
    HAL_SPINLOCK_DESTROY(&ipsec_sa->slock);
    hal::delay_delete_to_slab(HAL_SLAB_IPSEC_SA, ipsec_sa);
    return HAL_RET_OK;
}

static inline ipsec_sa_t*
find_ipsec_sa_by_id (ipsec_sa_id_t ipsec_sa_id)
{
    return (ipsec_sa_t *)g_hal_state->ipsec_sa_id_ht()->lookup(&ipsec_sa_id);
}

static inline void *
ipsec_cfg_pol_key_func_get (void *entry)
{
    ipsec_cfg_pol_t *pol = NULL;
    hal_handle_id_ht_entry_t *ht_entry;

    HAL_ASSERT(entry != NULL);
    if ((ht_entry = (hal_handle_id_ht_entry_t *)entry) == NULL)
        return NULL;

    pol = (ipsec_cfg_pol_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(pol->key);
}

static inline uint32_t
ipsec_cfg_pol_hash_func_compute (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(ipsec_cfg_pol_key_t)) % ht_size;
}

static inline bool
ipsec_cfg_pol_key_func_compare (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(ipsec_cfg_pol_key_t)))
        return true;

    return false;
}

//-----------------------------------------------------------------------------
// IPSec config object to its corresponding HAL handle management routines
//-----------------------------------------------------------------------------

static inline hal_handle_id_ht_entry_t *
ipsec_hal_handle_id_ht_entry_alloc (void)
{
    return ((hal_handle_id_ht_entry_t *)g_hal_state->
                hal_handle_id_ht_entry_slab()->alloc());
}

static inline void
ipsec_hal_handle_id_ht_entry_free (hal_handle_id_ht_entry_t *entry)
{
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
}

static inline void
ipsec_hal_handle_id_ht_entry_init (hal_handle_id_ht_entry_t *entry,
                             hal_handle_t hal_hdl)
{
    entry->handle_id = hal_hdl;
}

static inline void
ipsec_hal_handle_id_ht_entry_uninit (hal_handle_id_ht_entry_t *entry)
{
}

static inline hal_handle_id_ht_entry_t *
ipsec_hal_handle_id_ht_entry_alloc_init (hal_handle_t hal_hdl)
{
    hal_handle_id_ht_entry_t *entry;

    if ((entry = ipsec_hal_handle_id_ht_entry_alloc()) == NULL)
        return NULL;

    ipsec_hal_handle_id_ht_entry_init(entry, hal_hdl);
    return entry;
}

static inline void
ipsec_hal_handle_id_ht_entry_uninit_free (hal_handle_id_ht_entry_t *entry)
{
    if (entry) {
        ipsec_hal_handle_id_ht_entry_uninit(entry);
        ipsec_hal_handle_id_ht_entry_free(entry);
    }
}

static inline hal_ret_t
ipsec_hal_handle_id_ht_entry_db_add (ht *root, ht_ctxt_t *ht_ctxt, void *key,
                               hal_handle_id_ht_entry_t *entry)
{
    sdk_ret_t sdk_ret;

    sdk_ret = g_hal_state->ipsec_policy_ht()->insert_with_key(
        key, entry, ht_ctxt);

    hal_ret_t ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        ipsec_hal_handle_id_ht_entry_uninit_free(entry);
        return ret;
    }
    return ret;
}

static inline hal_ret_t
ipsec_cfg_pol_db_add (hal_handle_id_ht_entry_t *entry, ipsec_cfg_pol_t *pol)
{
    return ipsec_hal_handle_id_ht_entry_db_add(
        g_hal_state->ipsec_policy_ht(), &pol->ht_ctxt, &pol->key, entry);
}

static inline hal_ret_t
ipsec_cfg_pol_create_db_handle (ipsec_cfg_pol_t *pol)
{
    hal_ret_t ret;
    hal_handle_id_ht_entry_t *entry;

    if ((entry = ipsec_hal_handle_id_ht_entry_alloc_init(pol->hal_hdl)) == NULL)
        return HAL_RET_OOM;

    if ((ret = ipsec_cfg_pol_db_add(entry, pol)) != HAL_RET_OK)
        return ret;

    return ret;
}


static inline void ipsec_acl_ctx_name (char *name, vrf_id_t vrf_id)
{
    //thread_local static char name[ACL_NAMESIZE];
    std::snprintf(name, ACL_NAMESIZE, "ipsec-ipv4-rules:%lu", vrf_id);
    HAL_TRACE_DEBUG("Returning {}", name);
    //return name;
}

extern void *ipsec_sa_get_key_func(void *entry);
extern uint32_t ipsec_sa_compute_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_sa_compare_key_func(void *key1, void *key2);

extern void *ipsec_sa_get_handle_key_func(void *entry);
extern uint32_t ipsec_sa_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_sa_compare_handle_key_func(void *key1, void *key2);

hal_ret_t ipsec_saencrypt_create(ipsec::IpsecSAEncrypt& spec,
                       ipsec::IpsecSAEncryptResponse *rsp);

hal_ret_t ipsec_saencrypt_update(ipsec::IpsecSAEncrypt& spec,
                       ipsec::IpsecSAEncryptResponse *rsp);

hal_ret_t ipsec_saencrypt_delete(ipsec::IpsecSAEncryptDeleteRequest& req,
                       ipsec::IpsecSAEncryptDeleteResponseMsg *rsp);

hal_ret_t ipsec_saencrypt_get(ipsec::IpsecSAEncryptGetRequest& req,
                    ipsec::IpsecSAEncryptGetResponseMsg *rsp);

#if 0
extern void *ipsec_sa_decrypt_get_key_func(void *entry);
extern uint32_t ipsec_sa_decrypt_compute_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_sa_decrypt_compare_key_func(void *key1, void *key2);

extern void *ipsec_sa_decrypt_get_handle_key_func(void *entry);
extern uint32_t ipsec_sa_decrypt_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_sa_decryptcompare_handle_key_func(void *key1, void *key2);
#endif


hal_ret_t ipsec_sadecrypt_create(ipsec::IpsecSADecrypt& spec,
                       ipsec::IpsecSADecryptResponse *rsp);

hal_ret_t ipsec_sadecrypt_update(ipsec::IpsecSADecrypt& spec,
                       ipsec::IpsecSADecryptResponse *rsp);

hal_ret_t ipsec_sadecrypt_delete(ipsec::IpsecSADecryptDeleteRequest& req,
                       ipsec::IpsecSADecryptDeleteResponseMsg *rsp);

hal_ret_t ipsec_sadecrypt_get(ipsec::IpsecSADecryptGetRequest& req,
                    ipsec::IpsecSADecryptGetResponseMsg *rsp);

extern void *ipsec_rule_get_key_func(void *entry);
extern uint32_t ipsec_rule_compute_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_rule_compare_key_func(void *key1, void *key2);

extern void *ipsec_rule_get_handle_key_func(void *entry);
extern uint32_t ipsec_rule_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_rule_compare_handle_key_func(void *key1, void *key2);
extern const acl::acl_ctx_t *
ipsec_cfg_pol_create_app_ctxt_init(ipsec_cfg_pol_t *pol);
extern hal_ret_t
ipsec_cfg_pol_create_oper_handle(ipsec_cfg_pol_t *pol);
extern void
ipsec_cfg_pol_rsp_build(ipsec::IpsecRuleResponse *rsp, hal_ret_t ret,
                        hal_handle_t hal_handle);
extern hal_ret_t
ipsec_cfg_pol_rule_spec_build(ipsec_cfg_pol_t *pol,
                               ipsec::IpsecRuleSpec *spec);
extern hal_ret_t
ipsec_cfg_rule_spec_build(ipsec_cfg_rule_t *rule, ipsec::IpsecRuleMatchSpec *spec);

extern hal_ret_t
ipsec_cfg_rule_spec_handle(const ipsec::IpsecRuleMatchSpec& spec, dllist_ctxt_t *head);
extern hal_ret_t
ipsec_cfg_rule_create_oper_handle(ipsec_cfg_rule_t *rule, const acl_ctx_t *acl_ctx);

hal_ret_t ipsec_rule_create(ipsec::IpsecRuleSpec& spec,
                       ipsec::IpsecRuleResponse *rsp);

hal_ret_t ipsec_rule_update(ipsec::IpsecRuleSpec& spec,
                       ipsec::IpsecRuleResponse *rsp);

hal_ret_t ipsec_rule_delete(ipsec::IpsecRuleDeleteRequest& req,
                       ipsec::IpsecRuleDeleteResponseMsg *rsp);

hal_ret_t ipsec_rule_get(ipsec::IpsecRuleGetRequest& req,
                    ipsec::IpsecRuleGetResponseMsg *rsp);

}    // namespace hal

#endif    // __IPSECCB_HPP__

