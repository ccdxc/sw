#ifndef __IPSEC_HPP__
#define __IPSEC_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/ipsec.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/utils/rule_match.hpp"
#include "nic/hal/src/utils/utils.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using ipsec::IpsecSAEncrypt;
using ipsec::IpsecSADecrypt;
using ipsec::IpsecRuleSpec;
using kh::IpsecSAEncryptKeyHandle;
using kh::IpsecSADecryptKeyHandle;
using kh::IpsecRuleKeyHandle;

using ipsec::IpsecCbSpec;
using ipsec::IpsecCbResponse;
using ipsec::IpsecCbDeleteRequest;
using ipsec::IpsecCbDeleteResponseMsg;
using ipsec::IpsecCbGetRequest;
using ipsec::IpsecCbGetResponseMsg;

using ipsec::IpsecRuleRequestMsg;
using ipsec::IpsecRuleStatus;
using ipsec::IpsecRuleResponse;
using ipsec::IpsecRuleResponseMsg;
using ipsec::IpsecRuleDeleteRequest;
using ipsec::IpsecRuleDeleteRequestMsg;
using ipsec::IpsecRuleDeleteResponse;
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
using ipsec::IpsecSAEncryptDeleteResponse;
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
using ipsec::IpsecSADecryptDeleteResponse;
using ipsec::IpsecSADecryptDeleteResponseMsg;
using ipsec::IpsecSADecryptGetRequest;
using ipsec::IpsecSADecryptGetResponse;
using ipsec::IpsecSADecryptGetRequestMsg;
using ipsec::IpsecSADecryptGetResponseMsg;

using ipsec::IpsecGlobalStatisticsGetResponse;
using ipsec::IpsecGlobalStatisticsGetRequest;
using ipsec::IpsecGlobalStatisticsGetResponseMsg;
using ipsec::IpsecGlobalStatisticsGetRequestMsg;

namespace hal {

#define MAX_IPSEC_KEY_SIZE  32
#define IPSEC_CB_RING_SIZE 256
#define IPSEC_BARCO_RING_SIZE 1024

#define IPSEC_DEF_IV_SIZE                8
#define IPSEC_AES_GCM_DEF_BLOCK_SIZE    16
#define IPSEC_AES_GCM_DEF_ICV_SIZE      16
#define IPSEC_AES_GCM_DEF_KEY_SIZE      32

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
    rule_key_t    rule_id;
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

    // operational
    sdk_spinlock_t       slock;
    hal_handle_t         hal_hdl;
} ipsec_cfg_pol_t;

typedef struct ipsec_sa_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
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
    uint64_t              total_rx_pkts;
    uint64_t              total_rx_bytes;
    uint64_t              total_rx_drops;

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ ipsec_sa_t;


typedef struct ipsec_rule_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
    ipsec_rule_id_t       rule_id;                   // CB id
    hal_handle_t          hal_handle;              // HAL allocated handle
    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ ipsec_rule_t;

typedef struct ipsec_global_stats_cb_s {
    uint64_t         encrypt_input_desc_errors;
    uint64_t         encrypt_output_desc_errors;
    uint64_t         encrypt_cb_ring_base_errors;
    uint64_t         encrypt_input_page_errors;
    uint64_t         encrypt_barco_req_addr_errors;
    uint64_t         encrypt_barco_cb_base_errors;
    uint64_t         encrypt_pad_addr_errors;
    uint64_t         encrypt_tail_bytes_errors;
    uint64_t         encrypt_output_page_errors;
    uint64_t         encrypt_stage4_inpage_errors;
    uint64_t         encrypt_table3_inpage_errors;
    uint64_t         encrypt_table2_inpage_errors;
    uint64_t         encrypt_table0_inpage_errors;
    uint64_t         encrypt_bad_barco_addr_errors;
    uint64_t         encrypt_barco_full_errors;
    uint64_t         encrypt_cb_ring_dma_errors;
    uint64_t         encrypt_desc_exhaust_errors;
    uint64_t         encrypt_txdma1_enter_counters;
    uint64_t         encrypt_txdma2_enter_counters;
    uint64_t         encrypt_txdma1_dummy_errors;
    uint64_t         encrypt_rxdma_dummy_desc_errors;
    uint64_t         encrypt_rxdma_enter_counters;
    uint64_t         encrypt_barco_bad_indesc_errors;
    uint64_t         encrypt_barco_bad_outdesc_errors;
    uint64_t         encrypt_txdma2_bad_indesc_free_errors;
    uint64_t         encrypt_txdma2_bad_outdesc_free_errors;
    uint64_t         encrypt_txdma1_bad_indesc_free_errors;
    uint64_t         encrypt_txdma1_bad_outdesc_free_errors;
    uint64_t         encrypt_txdma1_sem_free_errors;
    uint64_t         encrypt_txdma2_sem_free_errors;
    uint64_t         encrypt_txdma1_barco_ring_full_errors;
    uint64_t         encrypt_rxdma_cb_ring_full_errors;
    uint64_t         encrypt_txdma2_barco_req_errors;
    uint64_t         encrypt_pad[31];

    uint64_t         decrypt_input_desc_errors;
    uint64_t         decrypt_output_desc_errors;
    uint64_t         decrypt_cb_ring_base_errors;
    uint64_t         decrypt_input_page_errors;
    uint64_t         decrypt_barco_req_addr_errors;
    uint64_t         decrypt_barco_cb_addr_errors;
    uint64_t         decrypt_stage4_inpage_errors;
    uint64_t         decrypt_output_page_errors;
    uint64_t         decrypt_txdma1_enter_counters;
    uint64_t         decrypt_txdma2_enter_counters;
    uint64_t         decrypt_txdma1_drop_counters;
    uint64_t         decrypt_desc_exhaust_errors;
    uint64_t         decrypt_txdma1_dummy_errors;
    uint64_t         decrypt_load_ipsec_int_errors;
    uint64_t         decrypt_txdma2_dummy_free;
    uint64_t         decrypt_rxdma_dummy_desc_errors;
    uint64_t         decrypt_rxdma_enter_counters;
    uint64_t         decrypt_txdma2_barco_bad_indesc_errors;
    uint64_t         decrypt_txdma2_barco_bad_outdesc_errors;
    uint64_t         decrypt_txdma2_bad_indesc_free_errors;
    uint64_t         decrypt_txdma2_bad_outdesc_free_errors;
    uint64_t         decrypt_txdma1_sem_free_errors;
    uint64_t         decrypt_txdma2_sem_free_errors;
    uint64_t         decrypt_txdma1_bad_indesc_free_errors;
    uint64_t         decrypt_txdma1_bad_outdesc_free_errors;
    uint64_t         decrypt_rxdma_cb_ring_full_errors;
    uint64_t         decrypt_txdma1_barco_ring_full_errors;
    uint64_t         decrypt_txdma1_barco_full_errors;
    uint64_t         decrypt_txdma2_invalid_barco_req_errors;
    uint64_t         decrypt_pad[35];

    uint32_t         enc_rnmdpr_pi_counters;
    uint32_t         enc_rnmdpr_ci_counters;
    uint32_t         dec_rnmdpr_pi_counters;
    uint32_t         dec_rnmdpr_ci_counters;
    uint32_t         enc_global_barco_pi;
    uint32_t         enc_global_barco_ci;
    uint32_t         dec_global_barco_pi;
    uint32_t         dec_global_barco_ci;
    uint32_t         gcm0_full_counters;
    uint32_t         gcm1_full_counters;
} __PACK__ ipsec_global_stats_cb_t;

// max. number of CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_IPSEC_SA                          4096
#define HAL_MAX_IPSEC_SUPP_SA                     32
#define IPSEC_PER_CB_RING_SIZE                    256
#define IPSEC_PER_CB_BARCO_RING_SIZE              512
#define IPSEC_PER_CB_BARCO_SLOT_ELEM_SIZE         16
#define IPSEC_BARCO_ENCRYPT_AES_GCM_256           0x30000000
#define IPSEC_BARCO_DECRYPT_AES_GCM_256           0x30100000

extern acl_config_t ipsec_ip_acl_config_glbl;

//-----------------------------------------------------------------------------
// Inline functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Rule routines
//-----------------------------------------------------------------------------

// Slab delete must not be called directly. It will be called from the acl ref
// library when the ref_count drops to zero
static inline void
ipsec_cfg_rule_free (void *rule)
{
    hal::delay_delete_to_slab(HAL_SLAB_IPSEC_CFG_RULE, (ipsec_cfg_rule_t *)rule);
}

static inline void
ipsec_cfg_rule_init (ipsec_cfg_rule_t *rule)
{
    rule_match_init(&rule->match);
    dllist_reset(&rule->list_ctxt);
}

static inline void
ipsec_cfg_rule_uninit (ipsec_cfg_rule_t *rule)
{
    return;
}

static inline void
ipsec_cfg_rule_uninit_free (ipsec_cfg_rule_t *rule)
{
     ipsec_cfg_rule_uninit(rule);
     ipsec_cfg_rule_free(rule);
}

static inline ipsec_cfg_rule_t *
ipsec_cfg_rule_alloc (void)
{
    ipsec_cfg_rule_t *rule;
    rule = (ipsec_cfg_rule_t *)g_hal_state->ipsec_cfg_rule_slab()->alloc();
    // Slab free will be called when the ref count drops to zero
    ref_init(&rule->ref_count, [] (const acl::ref_t * ref_count) {
        ipsec_cfg_rule_uninit_free(RULE_MATCH_USER_DATA(ref_count, ipsec_cfg_rule_t, ref_count));
    });
    return rule;
}

static inline ipsec_cfg_rule_t *
ipsec_cfg_rule_alloc_init (void)
{
    ipsec_cfg_rule_t *rule;

    if ((rule = ipsec_cfg_rule_alloc()) ==  NULL)
        return NULL;

    ipsec_cfg_rule_init(rule);
    return rule;
}

static inline hal_ret_t
ipsec_cfg_rule_action_spec_extract (const ipsec::IpsecSAAction& spec,
                                    ipsec_cfg_rule_action_t *action)
{
    hal_ret_t ret = HAL_RET_OK;
    action->sa_action = spec.sa_action_type();
    if (action->sa_action == ipsec::IPSEC_SA_ACTION_TYPE_ENCRYPT) {
        action->sa_action_enc_handle = (hal_handle_t)(spec.enc_handle().cb_id());
    } else if (action->sa_action == ipsec::IPSEC_SA_ACTION_TYPE_DECRYPT) {
        action->sa_action_dec_handle = (hal_handle_t)(spec.dec_handle().cb_id());
    }

    HAL_TRACE_DEBUG("action type {} enc_handle {} dec_handle {}", action->sa_action, action->sa_action_enc_handle, action->sa_action_dec_handle);
    return ret;
}

static inline void
ipsec_cfg_rule_db_add (dllist_ctxt_t *head, ipsec_cfg_rule_t *rule)
{
    dllist_add_tail(head, &rule->list_ctxt);
}

static inline void
ipsec_cfg_rule_db_del (ipsec_cfg_rule_t *rule)
{
    dllist_del(&rule->list_ctxt);
}

static inline void
ipsec_cfg_rule_cleanup (ipsec_cfg_rule_t *rule)
{
    rule_match_cleanup(&rule->match);
    ipsec_cfg_rule_db_del(rule);
}

static inline void
ipsec_cfg_rule_list_cleanup (dllist_ctxt_t *head)
{
    dllist_ctxt_t *curr, *next;
    ipsec_cfg_rule_t *rule;

    dllist_for_each_safe(curr, next, head) {
        rule = dllist_entry(curr, ipsec_cfg_rule_t, list_ctxt);
        ipsec_cfg_rule_cleanup(rule);
        // Decrement ref count for the rule. When ref count goes to zero
        // the acl ref library will free up the entry from the slab
        ref_dec(&rule->ref_count);
    }
}

static inline hal_ret_t
ipsec_cfg_rule_data_spec_extract (const ipsec::IpsecRuleMatchSpec& spec,
                                  ipsec_cfg_rule_t *rule)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = rule_match_spec_extract(
           spec.match(), &rule->match)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    if ((ret = ipsec_cfg_rule_action_spec_extract(
           spec.sa_action(), &rule->action)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return ret;
}

static inline hal_ret_t
ipsec_cfg_rule_key_spec_extract (const ipsec::IpsecRuleMatchSpec& spec,
                                 ipsec_cfg_rule_key_t *key)
{
    key->rule_id = spec.rule_id();
    return HAL_RET_OK;
}

static inline hal_ret_t
ipsec_cfg_rule_spec_extract (const ipsec::IpsecRuleMatchSpec& spec, ipsec_cfg_rule_t *rule)
{
    hal_ret_t ret;

    if ((ret = ipsec_cfg_rule_key_spec_extract(
           spec, &rule->key)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    if ((ret = ipsec_cfg_rule_data_spec_extract(
           spec, rule)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

   return ret;
}

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

    SDK_SPINLOCK_INIT(&ipsec_sa->slock, PTHREAD_PROCESS_PRIVATE);

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
    SDK_SPINLOCK_DESTROY(&ipsec_sa->slock);
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

    SDK_ASSERT(entry != NULL);
    if ((ht_entry = (hal_handle_id_ht_entry_t *)entry) == NULL)
        return NULL;

    pol = (ipsec_cfg_pol_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(pol->key);
}

static inline uint32_t
ipsec_cfg_pol_key_size ()
{
    return sizeof(ipsec_cfg_pol_key_t);
}

//-----------------------------------------------------------------------------
// IPSec config alloc and init routines
//-----------------------------------------------------------------------------

static inline ipsec_cfg_pol_t *
ipsec_cfg_pol_alloc (void)
{
    return ((ipsec_cfg_pol_t *)g_hal_state->ipsec_cfg_pol_slab()->alloc());
}

static inline void
ipsec_cfg_pol_free (ipsec_cfg_pol_t *pol)
{
    hal::delay_delete_to_slab(HAL_SLAB_IPSEC_CFG_POL, pol);
}

static inline void
ipsec_cfg_pol_init (ipsec_cfg_pol_t *pol)
{
    SDK_SPINLOCK_INIT(&pol->slock, PTHREAD_PROCESS_SHARED);
    dllist_reset(&pol->rule_list);
    pol->hal_hdl = HAL_HANDLE_INVALID;
}

static inline void
ipsec_cfg_pol_uninit (ipsec_cfg_pol_t *pol)
{
    SDK_SPINLOCK_DESTROY(&pol->slock);
}

static inline ipsec_cfg_pol_t *
ipsec_cfg_pol_alloc_init (void)
{
    ipsec_cfg_pol_t *pol;

    if ((pol = ipsec_cfg_pol_alloc()) ==  NULL) {
        HAL_TRACE_DEBUG("Failed here");
        return NULL;
    }

    ipsec_cfg_pol_init(pol);
    return pol;
}

static inline void
ipsec_cfg_pol_uninit_free (ipsec_cfg_pol_t *pol)
{
     ipsec_cfg_pol_uninit(pol);
     ipsec_cfg_pol_free(pol);
}

static inline void
ipsec_cfg_pol_cleanup (ipsec_cfg_pol_t *pol)
{
    if (pol) {
        ipsec_cfg_rule_list_cleanup(&pol->rule_list);
        ipsec_cfg_pol_uninit_free(pol);
    }
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
ipsec_cfg_pol_create_db_handle (ipsec_cfg_pol_t *pol)
{
    hal_handle_id_ht_entry_t *entry;

    if ((entry = hal_handle_id_ht_entry_alloc_init(
            g_hal_state->hal_handle_id_ht_entry_slab(),
            pol->hal_hdl)) == NULL)
        return HAL_RET_OOM;

    return hal_handle_id_ht_entry_db_add(
        g_hal_state->ipsec_policy_ht(), &pol->key, entry);
}

//-----------------------------------------------------------------------------
// IPSec config object to its corresponding HAL handle management routines
//
// The IPSec config object can be obtained through key or hal handle. The node
// is inserted/deleted in two different hash tables.
//   a) key: hal_handle, data: config object
//   b) key: config object key, data: hal_handle
// In case of #b, a second lookup is done to get to object from hal_handle
//-----------------------------------------------------------------------------

static inline ipsec_cfg_pol_t *
ipsec_cfg_pol_hal_hdl_db_lookup (hal_handle_t hal_hdl)
{
    if (hal_hdl == HAL_HANDLE_INVALID)
        return NULL;

    auto hal_hdl_e = hal_handle_get_from_handle_id(hal_hdl);
    if (hal_hdl_e == NULL || hal_hdl_e->obj_id() != HAL_OBJ_ID_IPSEC_POLICY)
        return NULL;

    return (ipsec_cfg_pol_t *) hal_handle_get_obj(hal_hdl);
}

static inline ipsec_cfg_pol_t *
ipsec_cfg_pol_db_lookup (ipsec_cfg_pol_key_t *key)
{
    hal_handle_id_ht_entry_t *entry;

    if ((entry = hal_handle_id_ht_entry_db_lookup(
            g_hal_state->ipsec_policy_ht(), key)) == NULL)
        return NULL;

    return ipsec_cfg_pol_hal_hdl_db_lookup(entry->handle_id);
}

static inline hal_ret_t
ipsec_cfg_pol_delete_db_handle (ipsec_cfg_pol_key_t *key)
{
    hal_handle_id_ht_entry_t *entry;

    if ((entry = hal_handle_id_ht_entry_db_del(
            g_hal_state->ipsec_policy_ht(), key)) == NULL)
        return HAL_RET_IPSEC_RULE_NOT_FOUND;

    hal_handle_id_ht_entry_uninit_free(entry);
    return HAL_RET_OK;
}

static inline void ipsec_acl_ctx_name (char *name, vrf_id_t vrf_id)
{
    //thread_local static char name[ACL_NAMESIZE];
    std::snprintf(name, ACL_NAMESIZE, "ipsec-ipv4-rules:%lu", vrf_id);
    HAL_TRACE_VERBOSE("Returning {}", name);
    //return name;
}

static inline void
ipsec_cfg_pol_create_rsp_build (ipsec::IpsecRuleResponse *rsp, hal_ret_t ret,
                                hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK)
        rsp->mutable_status()->set_handle(hal_handle);
    rsp->set_api_status(hal_prepare_rsp(ret));
}

static inline void
ipsec_cfg_pol_delete_rsp_build (ipsec::IpsecRuleDeleteResponse *rsp, hal_ret_t ret)
{
    rsp->set_api_status(hal_prepare_rsp(ret));
}

//-----------------------------------------------------------------------------
// ACL LIB operational handling
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Operational handling (hal handles, acl libs, etc)
//-----------------------------------------------------------------------------

static inline hal_ret_t
ipsec_cfg_rule_acl_build (ipsec_cfg_rule_t *rule, const acl_ctx_t **acl_ctx)
{
    return rule_match_rule_add(acl_ctx, &rule->match, rule->key.rule_id, rule->prio, &rule->ref_count);
}

static inline void
ipsec_cfg_rule_acl_cleanup (ipsec_cfg_rule_t *rule)
{
    return;
}

static inline hal_ret_t
ipsec_cfg_pol_rule_acl_build (ipsec_cfg_pol_t *pol, const acl_ctx_t **acl_ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    ipsec_cfg_rule_t *rule;
    dllist_ctxt_t *entry;
    uint32_t prio = 0;

    dllist_for_each(entry, &pol->rule_list) {
        rule = dllist_entry(entry, ipsec_cfg_rule_t, list_ctxt);
        rule->prio = prio++;
        if ((ret = ipsec_cfg_rule_acl_build(rule, acl_ctx)) != HAL_RET_OK)
            return ret;
    }

    return ret;
}

static inline hal_ret_t
ipsec_cfg_pol_acl_build (ipsec_cfg_pol_t *pol, const acl_ctx_t **out_acl_ctx)
{
    hal_ret_t ret = HAL_RET_ERR;
    const acl_ctx_t *acl_ctx;
    char acl_name[ACL_NAMESIZE];

    ipsec_acl_ctx_name(acl_name, pol->key.vrf_id);
    if ((acl_ctx = rule_lib_init(acl_name, &ipsec_ip_acl_config_glbl)) == NULL)
        return ret;

    if ((ret = ipsec_cfg_pol_rule_acl_build(pol, &acl_ctx)) != HAL_RET_OK)
        return ret;

    *out_acl_ctx = acl_ctx;
    return ret;
}

static inline void
ipsec_cfg_pol_rule_acl_cleanup (ipsec_cfg_pol_t *pol)
{
    ipsec_cfg_rule_t *rule;
    dllist_ctxt_t *entry;

    dllist_for_each(entry, &pol->rule_list) {
        rule = dllist_entry(entry, ipsec_cfg_rule_t, list_ctxt);
        ipsec_cfg_rule_acl_cleanup(rule);
    }
}

static inline hal_ret_t
ipsec_cfg_pol_acl_cleanup (ipsec_cfg_pol_t *pol)
{
    char acl_name[ACL_NAMESIZE];

    ipsec_acl_ctx_name(acl_name, pol->key.vrf_id);
    ipsec_cfg_pol_rule_acl_cleanup(pol);
    rule_lib_delete(acl_name);
    return HAL_RET_OK;
}

extern void *ipsec_sa_get_key_func(void *entry);
extern uint32_t ipsec_sa_key_size(void);

extern void *ipsec_sa_get_handle_key_func(void *entry);
extern uint32_t ipsec_sa_handle_key_size(void);

hal_ret_t ipsec_saencrypt_create(ipsec::IpsecSAEncrypt& spec,
                       ipsec::IpsecSAEncryptResponse *rsp);

hal_ret_t ipsec_saencrypt_update(ipsec::IpsecSAEncrypt& spec,
                       ipsec::IpsecSAEncryptResponse *rsp);

hal_ret_t ipsec_saencrypt_delete(ipsec::IpsecSAEncryptDeleteRequest& req,
                       ipsec::IpsecSAEncryptDeleteResponse *rsp);

hal_ret_t ipsec_saencrypt_get(ipsec::IpsecSAEncryptGetRequest& req,
                    ipsec::IpsecSAEncryptGetResponseMsg *rsp);

hal_ret_t ipsec_sadecrypt_create(ipsec::IpsecSADecrypt& spec,
                       ipsec::IpsecSADecryptResponse *rsp);

hal_ret_t ipsec_sadecrypt_update(ipsec::IpsecSADecrypt& spec,
                       ipsec::IpsecSADecryptResponse *rsp);

hal_ret_t ipsec_sadecrypt_delete(ipsec::IpsecSADecryptDeleteRequest& req,
                       ipsec::IpsecSADecryptDeleteResponse *rsp);

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
ipsec_cfg_rule_spec_build(ipsec_cfg_rule_t *rule, ipsec::IpsecRuleMatchSpec *spec, ipsec::IpsecRuleSpec *rule_spec);

extern hal_ret_t
ipsec_cfg_rule_spec_handle(const ipsec::IpsecRuleMatchSpec& spec, dllist_ctxt_t *head);
extern hal_ret_t
ipsec_cfg_rule_create_oper_handle(ipsec_cfg_rule_t *rule, const acl_ctx_t *acl_ctx);

hal_ret_t ipsec_rule_create(ipsec::IpsecRuleSpec& spec,
                       ipsec::IpsecRuleResponse *rsp);

hal_ret_t ipsec_rule_update(ipsec::IpsecRuleSpec& spec,
                       ipsec::IpsecRuleResponse *rsp);

hal_ret_t ipsec_rule_delete(ipsec::IpsecRuleDeleteRequest& req,
                       ipsec::IpsecRuleDeleteResponse *rsp);

hal_ret_t ipsec_rule_get(ipsec::IpsecRuleGetRequest& req,
                    ipsec::IpsecRuleGetResponseMsg *rsp);

hal_ret_t ipsec_global_statistics_get(ipsec::IpsecGlobalStatisticsGetRequest& req,
                                      ipsec::IpsecGlobalStatisticsGetResponseMsg *rsp);

hal_ret_t ipseccb_create(ipsec::IpsecCbSpec& spec,
                         ipsec::IpsecCbResponse *rsp);

hal_ret_t ipseccb_update(ipsec::IpsecCbSpec& spec,
                         ipsec::IpsecCbResponse *rsp);

hal_ret_t ipseccb_delete(ipsec::IpsecCbDeleteRequest& req,
                         ipsec::IpsecCbDeleteResponseMsg *rsp);

hal_ret_t ipseccb_get(ipsec::IpsecCbGetRequest& req,
                      ipsec::IpsecCbGetResponseMsg *rsp);
}    // namespace hal

#endif    // __IPSEC_HPP__

