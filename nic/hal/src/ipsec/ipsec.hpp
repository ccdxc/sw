#ifndef __IPSEC_HPP__
#define __IPSEC_HPP__

#include "nic/include/base.h"
#include "nic/include/encap.hpp"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/ipsec.pb.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"

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

typedef struct ipsec_sa_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    ipsec_sa_id_t         sa_id;                   // CB id
    ip_addr_t             tunnel_sip4;
    ip_addr_t             tunnel_dip4;
    uint8_t               iv_size;
    uint8_t               icv_size;
    uint8_t               block_size;
    uint32_t              key_index;
    uint8_t               key[MAX_IPSEC_KEY_SIZE];
    uint32_t              new_key_index;
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
    uint16_t              vrf_vlan;
    uint32_t              last_replay_seq_no;
    // PD state
    void                  *pd;                     // all PD specific state
    void                  *pd_decrypt;                     // all PD specific state

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
#define HAL_MAX_IPSECCB                           4
#define IPSEC_BARCO_ENCRYPT_AES_GCM_256           0x30000000
#define IPSEC_BARCO_DECRYPT_AES_GCM_256           0x30100000

// allocate a ipsec_sament instance
static inline ipsec_sa_t *
ipsec_sa_alloc (void)
{
    ipsec_sa_t    *ipsec_sa;

    ipsec_sa = (ipsec_sa_t *)g_hal_state->ipseccb_slab()->alloc();
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
    hal::delay_delete_to_slab(HAL_SLAB_IPSECCB, ipsec_sa);
    return HAL_RET_OK;
}

static inline ipsec_sa_t *
find_ipsec_sa_by_id (ipsec_sa_id_t ipsec_sa_id)
{
    return (ipsec_sa_t *)g_hal_state->ipseccb_id_ht()->lookup(&ipsec_sa_id);
}

extern void *ipsec_sa_encrypt_get_key_func(void *entry);
extern uint32_t ipsec_sa_encrypt_compute_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_sa_encrypt_compare_key_func(void *key1, void *key2);

extern void *ipsec_sa_encrypt_get_handle_key_func(void *entry);
extern uint32_t ipsec_sa_encrypt_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_sa_encryptcompare_handle_key_func(void *key1, void *key2);

hal_ret_t ipsec_saencrypt_create(ipsec::IpsecSAEncrypt& spec,
                       ipsec::IpsecSAEncryptResponse *rsp);

hal_ret_t ipsec_saencrypt_update(ipsec::IpsecSAEncrypt& spec,
                       ipsec::IpsecSAEncryptResponse *rsp);

hal_ret_t ipsec_saencrypt_delete(ipsec::IpsecSAEncryptDeleteRequest& req,
                       ipsec::IpsecSAEncryptDeleteResponseMsg *rsp);

hal_ret_t ipsec_saencrypt_get(ipsec::IpsecSAEncryptGetRequest& req,
                    ipsec::IpsecSAEncryptGetResponseMsg *rsp);

extern void *ipsec_sa_decrypt_get_key_func(void *entry);
extern uint32_t ipsec_sa_decrypt_compute_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_sa_decrypt_compare_key_func(void *key1, void *key2);

extern void *ipsec_sa_decrypt_get_handle_key_func(void *entry);
extern uint32_t ipsec_sa_decrypt_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool ipsec_sa_decryptcompare_handle_key_func(void *key1, void *key2);

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

hal_ret_t ipsec_rule_create(ipsec::IpsecRuleSpec& spec,
                       ipsec::IpsecRuleResponse *rsp);

hal_ret_t ipsec_rule_update(ipsec::IpsecRuleSpec& spec,
                       ipsec::IpsecRuleResponse *rsp);

hal_ret_t ipsec_rule_delete(ipsec::IpsecRuleDeleteRequest& req,
                       ipsec::IpsecRuleDeleteResponseMsg *rsp);

hal_ret_t ipsec_rule_get(ipsec::IpsecRuleGetRequest& req,
                    ipsec::IpsecRuleGetRequestMsg *rsp);

}    // namespace hal

#endif    // __IPSECCB_HPP__

