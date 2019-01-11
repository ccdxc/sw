#ifndef __TCP_PROXY_HPP__
#define __TCP_PROXY_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/encap.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/tcp_proxy.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/utils/rule_match.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "gen/proto/proxy.grpc.pb.h"
#include "nic/hal/src/internal/proxy.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using tcp_proxy::TcpProxyRuleSpec;
using kh::TcpProxyRuleKeyHandle;

using tcp_proxy::TcpProxyRuleRequestMsg;
using tcp_proxy::TcpProxyRuleStatus;
using tcp_proxy::TcpProxyRuleResponse;
using tcp_proxy::TcpProxyRuleResponseMsg;
using tcp_proxy::TcpProxyRuleDeleteRequest;
using tcp_proxy::TcpProxyRuleDeleteRequestMsg;
using tcp_proxy::TcpProxyRuleDeleteResponse;
using tcp_proxy::TcpProxyRuleDeleteResponseMsg;
using tcp_proxy::TcpProxyRuleGetRequest;
using tcp_proxy::TcpProxyRuleGetRequestMsg;
using tcp_proxy::TcpProxyRuleGetResponseMsg;

using tcp_proxy::TcpProxyCbSpec;
using tcp_proxy::TcpProxyCbStatus;
using tcp_proxy::TcpProxyCbResponse;
using tcp_proxy::TcpProxyCbKeyHandle;
using tcp_proxy::TcpProxyCbRequestMsg;
using tcp_proxy::TcpProxyCbResponseMsg;
using tcp_proxy::TcpProxyCbDeleteRequestMsg;
using tcp_proxy::TcpProxyCbDeleteResponseMsg;
using tcp_proxy::TcpProxyCbGetRequest;
using tcp_proxy::TcpProxyCbGetRequestMsg;
using tcp_proxy::TcpProxyCbGetResponse;
using tcp_proxy::TcpProxyCbGetResponseMsg;

using grpc::Status;
using proxy::ProxySpec;
using proxy::ProxyResponse;



#define INVALID_HEADER_TEMPLATE_LEN ((uint32_t)-1)

namespace hal {

#define MAX_TCP_PROXY_KEY_SIZE  32
#define TCP_PROXY_CB_RING_SIZE 256
#define TCP_PROXY_BARCO_RING_SIZE 1024 

typedef uint64_t rule_id_t;
typedef uint64_t pol_id_t;

//------------------------------------------------------------------------------
// TcpProxy Policy (cfg) data structure layout
//
//    tcp_proxy_policy <policy_key> {  policy_key = pol_id + vrf_id
//        tcp_proxy_rule <rule_key> {  rule_key = rule_id
//
//            // match criteria for rule match
//            tcp_proxy_rule_match {
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
//            tcp_proxy_rule_action {
//                tcp_proxy_action;
//                tcp_proxy_action_enc_handle;
//                tcp_proxy_action_dec_handle;
//            }
//        }
//    }
//
// TcpProxy Policy (oper) data structure layout
//
//
//------------------------------------------------------------------------------


typedef struct tls_proxy_tls_cfg_ecdsa_key_s {
    uint32_t                        sign_key_idx;
} tls_proxy_tls_cfg_ecdsa_key_t;

typedef struct tls_proxy_tls_cfg_rsa_key_s {
    uint32_t                        sign_key_idx;
    uint32_t                        decrypt_key_idx;
} tls_proxy_tls_cfg_rsa_key_t;

typedef struct tcp_proxy_tls_cfg_s {
    types::CryptoAsymKeyType        asym_key_type;
    uint32_t                        cert_id;
    uint32_t                        trust_root_id;
    std::string                     ciphers;
    tcp_proxy::TlsProxySide         tls_proxy_side;
    union {
        tls_proxy_tls_cfg_ecdsa_key_t   ecdsa_key;
        tls_proxy_tls_cfg_rsa_key_t     rsa_key;
    } u;
} tcp_proxy_tls_cfg_t;

typedef struct tcp_proxy_cfg_rule_action_s {
    tcp_proxy::TcpProxyActionType   tcp_proxy_action;
    types::ProxyType                proxy_type;
    union {
        tcp_proxy_tls_cfg_t         tls_cfg;
    } u;
    hal_handle_t      tcp_proxy_action_handle;
    vrf_id_t    vrf;
} tcp_proxy_cfg_rule_action_t;

typedef struct tcp_proxy_cfg_rule_key_s {
    rule_id_t    rule_id;
} __PACK__ tcp_proxy_cfg_rule_key_t;

typedef struct tcp_proxy_cfg_rule_s {
    tcp_proxy_cfg_rule_key_t     key;
    rule_match_t             match;
    tcp_proxy_cfg_rule_action_t  action;

    // operational
    uint32_t                 prio;
    dllist_ctxt_t            list_ctxt;
    acl::ref_t               ref_count;
} tcp_proxy_cfg_rule_t;

typedef struct tcp_proxy_cfg_pol_key_s {
    pol_id_t    pol_id;
    vrf_id_t    vrf_id;
} __PACK__ tcp_proxy_cfg_pol_key_t;

typedef struct tcp_proxy_cfg_pol_create_app_ctxt_s {
    const acl::acl_ctx_t    *acl_ctx;
} __PACK__ tcp_proxy_cfg_pol_create_app_ctxt_t;

typedef struct tcp_proxy_cfg_pol_s {
    tcp_proxy_cfg_pol_key_t    key;
    dllist_ctxt_t        rule_list;

    // operational
    hal_spinlock_t       slock;
    hal_handle_t         hal_hdl;
} tcp_proxy_cfg_pol_t;


typedef struct tcp_proxy_rule_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    tcp_proxy_rule_id_t       rule_id;                   // CB id
    hal_handle_t          hal_handle;              // HAL allocated handle
    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
} __PACK__ tcp_proxy_rule_t;

typedef struct tcp_proxy_cb_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    tcp_proxy_cb_id_t            cb_id;                   // TCP CB id
    uint32_t              rcv_nxt;
    uint32_t              snd_nxt;
    uint32_t              snd_una;
    uint32_t              rcv_tsval;
    uint32_t              ts_recent;
    uint64_t              rx_ts;
    uint64_t              serq_base;
    uint32_t              debug_dol;
    uint32_t              sesq_pi;
    uint32_t              sesq_ci;
    uint64_t              sesq_base;
    uint32_t              asesq_pi;
    uint32_t              asesq_ci;
    uint64_t              asesq_base;
    uint32_t              snd_wnd;
    uint32_t              snd_cwnd;
    uint32_t              rcv_mss;
    uint16_t              source_port;
    uint16_t              dest_port;
    uint8_t               header_template[64];
    uint32_t              state;
    uint16_t              source_lif;
    uint32_t              debug_dol_tx;
    uint32_t              header_len;
    uint32_t              pending_ack_send;
    types::AppRedirType   l7_proxy_type;
    uint32_t              sesq_retx_ci;            // for testing, check in DOL
    uint32_t              retx_snd_una;            // for testing, check in DOL
    uint32_t              rto;
    uint32_t              snd_cwnd_cnt;
    uint32_t              serq_pi;
    uint32_t              serq_ci;
    uint32_t              pred_flags;
    uint32_t              packets_out;
    uint32_t              rto_pi;
    uint32_t              retx_timer_ci;
    uint32_t              rto_backoff;
    uint8_t               cpu_id;

    // operational state of TCP Proxy CB
    hal_handle_t          hal_handle;              // HAL allocated handle

    // rx stats
    uint32_t              debug_stage0_7_thread;
    uint64_t              bytes_rcvd;
    uint64_t              pkts_rcvd;
    uint64_t              pages_alloced;
    uint64_t              desc_alloced;
    uint64_t              debug_num_phv_to_mem;
    uint64_t              debug_num_pkt_to_mem;

    uint64_t              debug_atomic_delta;
    uint64_t              debug_atomic0_incr1247;
    uint64_t              debug_atomic1_incr247;
    uint64_t              debug_atomic2_incr47;
    uint64_t              debug_atomic3_incr47;
    uint64_t              debug_atomic4_incr7;
    uint64_t              debug_atomic5_incr7;
    uint64_t              debug_atomic6_incr7;

    uint64_t              bytes_acked;
    uint64_t              slow_path_cnt;
    uint64_t              serq_full_cnt;
    uint64_t              ooo_cnt;

    uint8_t               debug_dol_tblsetaddr;

    // tx stats
    uint64_t              bytes_sent;
    uint64_t              pkts_sent;
    uint64_t              debug_num_phv_to_pkt;
    uint64_t              debug_num_mem_to_pkt;

    // PD state
    void                  *pd;                     // all PD specific state

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash table ctxt
    uint16_t              other_qid;
} __PACK__ tcp_proxy_cb_t;

// max. number of TCP CBs supported  (TODO: we can take this from cfg file)
// max. number of CBs supported  (TODO: we can take this from cfg file)
#define HAL_MAX_TCP_PROXY_SA                          2048 

#define HAL_MAX_TCPCB                           2048

extern acl_config_t tcp_proxy_ip_acl_config_glbl;

//-----------------------------------------------------------------------------
// Inline functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Rule routines
//-----------------------------------------------------------------------------

// Slab delete must not be called directly. It will be called from the acl ref
// library when the ref_count drops to zero
static inline void
tcp_proxy_cfg_rule_free (void *rule)
{
    hal::delay_delete_to_slab(HAL_SLAB_TCP_PROXY_CFG_RULE, (tcp_proxy_cfg_rule_t *)rule);
}

static inline void
tcp_proxy_cfg_rule_init (tcp_proxy_cfg_rule_t *rule)
{
    rule_match_init(&rule->match);
    dllist_reset(&rule->list_ctxt);
}

static inline void
tcp_proxy_cfg_rule_uninit (tcp_proxy_cfg_rule_t *rule)
{
    return;
}

static inline void
tcp_proxy_cfg_rule_uninit_free (tcp_proxy_cfg_rule_t *rule)
{
     tcp_proxy_cfg_rule_uninit(rule);
     tcp_proxy_cfg_rule_free(rule);
}

static inline tcp_proxy_cfg_rule_t *
tcp_proxy_cfg_rule_alloc (void)
{
    tcp_proxy_cfg_rule_t *rule;
    rule = (tcp_proxy_cfg_rule_t *)g_hal_state->tcp_proxy_cfg_rule_slab()->alloc();
    // Slab free will be called when the ref count drops to zero
    ref_init(&rule->ref_count, [] (const acl::ref_t * ref_count) {
        tcp_proxy_cfg_rule_uninit_free(RULE_MATCH_USER_DATA(ref_count, tcp_proxy_cfg_rule_t, ref_count));
    });
    return rule;
}

static inline tcp_proxy_cfg_rule_t *
tcp_proxy_cfg_rule_alloc_init (void)
{
    tcp_proxy_cfg_rule_t *rule;

    if ((rule = tcp_proxy_cfg_rule_alloc()) ==  NULL)
        return NULL;

    tcp_proxy_cfg_rule_init(rule);
    return rule;
}

static inline hal_ret_t tcp_proxy_enable(types::ProxyType proxy_type)
{
    ProxySpec           spec;
    ProxyResponse       rsp;
    Status              status;
    hal_ret_t           ret = HAL_RET_OK;

    spec.set_proxy_type(proxy_type);
    ret = proxy_enable(spec, &rsp);

    if (rsp.api_status() != types::API_STATUS_OK) {
        ret = HAL_RET_ERR;
    }
    return ret;
}

static inline hal_ret_t
tcp_proxy_cfg_rule_action_spec_extract (const tcp_proxy::TcpProxyAction& spec,
                                    tcp_proxy_cfg_rule_action_t *action)
{
    hal_ret_t ret = HAL_RET_OK;
    action->tcp_proxy_action = spec.tcp_proxy_action_type();
    action->proxy_type = spec.proxy_type();
    if (spec.proxy_type() == types::PROXY_TYPE_NONE) {
	action->proxy_type = types::PROXY_TYPE_TCP;
        HAL_TRACE_DEBUG("PROXY_TYPE_TCP: Policy Defaulting to TCP ")
    }
    else if (spec.proxy_type() == types::PROXY_TYPE_TCP) {
        HAL_TRACE_DEBUG("PROXY_TYPE_TCP: Policy")
    } else if (spec.proxy_type() == types::PROXY_TYPE_TLS) {
        HAL_TRACE_DEBUG("PROXY_TYPE_TLS: Policy")
        action->u.tls_cfg.asym_key_type = spec.tls().asym_key_type();
        action->u.tls_cfg.cert_id = spec.tls().cert_id();
        action->u.tls_cfg.trust_root_id = spec.tls().trust_root_id();
        if (spec.tls().tls_proxy_side() == tcp_proxy::TLS_PROXY_SIDE_NONE) {
            action->u.tls_cfg.tls_proxy_side = tcp_proxy::TLS_PROXY_SIDE_CLIENT;
        }
        else {
            action->u.tls_cfg.tls_proxy_side = spec.tls().tls_proxy_side();
        }

        if(spec.tls().ciphers().length() > 0) {
            action->u.tls_cfg.ciphers = spec.tls().ciphers();
        }
        switch (spec.tls().asym_key_type()) {
            case types::CRYPTO_ASYM_KEY_TYPE_ECDSA:
                action->u.tls_cfg.u.ecdsa_key.sign_key_idx =
                    spec.tls().ecdsa_key().sign_key_idx();
                HAL_TRACE_DEBUG("PROXY_TYPE_TLS: Setup: Key Type ECDSA: Sign Key IDX: {}", action->u.tls_cfg.u.ecdsa_key.sign_key_idx); 
                break;
            case types::CRYPTO_ASYM_KEY_TYPE_RSA:
                action->u.tls_cfg.u.rsa_key.sign_key_idx = 
                    spec.tls().rsa_key().sign_key_idx();
                action->u.tls_cfg.u.rsa_key.decrypt_key_idx= 
                    spec.tls().rsa_key().decrypt_key_idx();
                HAL_TRACE_DEBUG("PROXY_TYPE_TLS: Setup: Key Type RSA: Sign Key IDX: {}, Decrypt Key IDX: {}", action->u.tls_cfg.u.ecdsa_key.sign_key_idx, action->u.tls_cfg.u.rsa_key.decrypt_key_idx); 
                break;
            default:
                HAL_TRACE_ERR("Unknown key type: {}", spec.tls().asym_key_type());
        }
    }
    return ret;
}

static inline void
tcp_proxy_cfg_rule_db_add (dllist_ctxt_t *head, tcp_proxy_cfg_rule_t *rule)
{
    dllist_add_tail(head, &rule->list_ctxt);
}

static inline void
tcp_proxy_cfg_rule_db_del (tcp_proxy_cfg_rule_t *rule)
{
    dllist_del(&rule->list_ctxt);
}

static inline void
tcp_proxy_cfg_rule_cleanup (tcp_proxy_cfg_rule_t *rule)
{
    rule_match_cleanup(&rule->match);
    tcp_proxy_cfg_rule_db_del(rule);
}

static inline void
tcp_proxy_cfg_rule_list_cleanup (dllist_ctxt_t *head)
{
    dllist_ctxt_t *curr, *next;
    tcp_proxy_cfg_rule_t *rule;

    dllist_for_each_safe(curr, next, head) {
        rule = dllist_entry(curr, tcp_proxy_cfg_rule_t, list_ctxt);
        tcp_proxy_cfg_rule_cleanup(rule);
        // Decrement ref count for the rule. When ref count goes to zero
        // the acl ref library will free up the entry from the slab
        ref_dec(&rule->ref_count);
    }
}

static inline hal_ret_t
tcp_proxy_cfg_rule_data_spec_extract (const tcp_proxy::TcpProxyRuleMatchSpec& spec,
                                  tcp_proxy_cfg_rule_t *rule)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = rule_match_spec_extract(
           spec.match(), &rule->match)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    if ((ret = tcp_proxy_cfg_rule_action_spec_extract(
           spec.tcp_proxy_action(), &rule->action)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return ret;
}

static inline hal_ret_t
tcp_proxy_cfg_rule_key_spec_extract (const tcp_proxy::TcpProxyRuleMatchSpec& spec,
                                 tcp_proxy_cfg_rule_key_t *key)
{
    key->rule_id = spec.rule_id();
    return HAL_RET_OK;
}

static inline hal_ret_t
tcp_proxy_cfg_rule_spec_extract (const tcp_proxy::TcpProxyRuleMatchSpec& spec, tcp_proxy_cfg_rule_t *rule)
{
    hal_ret_t ret;

    if ((ret = tcp_proxy_cfg_rule_key_spec_extract(
           spec, &rule->key)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    if ((ret = tcp_proxy_cfg_rule_data_spec_extract(
           spec, rule)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

   return ret;
}

static inline void *
tcp_proxy_cfg_pol_key_func_get (void *entry)
{
    tcp_proxy_cfg_pol_t *pol = NULL;
    hal_handle_id_ht_entry_t *ht_entry;

    HAL_ASSERT(entry != NULL);
    if ((ht_entry = (hal_handle_id_ht_entry_t *)entry) == NULL)
        return NULL;

    pol = (tcp_proxy_cfg_pol_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(pol->key);
}

static inline uint32_t
tcp_proxy_cfg_pol_hash_func_compute (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(tcp_proxy_cfg_pol_key_t)) % ht_size;
}

static inline bool
tcp_proxy_cfg_pol_key_func_compare (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(tcp_proxy_cfg_pol_key_t)))
        return true;

    return false;
}

//-----------------------------------------------------------------------------
// TcpProxy config alloc and init routines
//-----------------------------------------------------------------------------

static inline tcp_proxy_cfg_pol_t *
tcp_proxy_cfg_pol_alloc (void)
{
    return ((tcp_proxy_cfg_pol_t *)g_hal_state->tcp_proxy_cfg_pol_slab()->alloc());
}

static inline void
tcp_proxy_cfg_pol_free (tcp_proxy_cfg_pol_t *pol)
{
    hal::delay_delete_to_slab(HAL_SLAB_TCP_PROXY_CFG_POL, pol);
}

static inline void
tcp_proxy_cfg_pol_init (tcp_proxy_cfg_pol_t *pol)
{
    HAL_SPINLOCK_INIT(&pol->slock, PTHREAD_PROCESS_SHARED);
    dllist_reset(&pol->rule_list);
    pol->hal_hdl = HAL_HANDLE_INVALID;
}

static inline void
tcp_proxy_cfg_pol_uninit (tcp_proxy_cfg_pol_t *pol)
{
    HAL_SPINLOCK_DESTROY(&pol->slock);
}

static inline tcp_proxy_cfg_pol_t *
tcp_proxy_cfg_pol_alloc_init (void)
{
    tcp_proxy_cfg_pol_t *pol;

    if ((pol = tcp_proxy_cfg_pol_alloc()) ==  NULL) {
        HAL_TRACE_DEBUG("Failed here");
        return NULL;
    }

    tcp_proxy_cfg_pol_init(pol);
    return pol;
}

static inline void
tcp_proxy_cfg_pol_uninit_free (tcp_proxy_cfg_pol_t *pol)
{
     tcp_proxy_cfg_pol_uninit(pol);
     tcp_proxy_cfg_pol_free(pol);
}

static inline void
tcp_proxy_cfg_pol_cleanup (tcp_proxy_cfg_pol_t *pol)
{
    if (pol) {
        tcp_proxy_cfg_rule_list_cleanup(&pol->rule_list);
        tcp_proxy_cfg_pol_uninit_free(pol);
    }
}

//-----------------------------------------------------------------------------
// TcpProxy config object to its corresponding HAL handle management routines
//-----------------------------------------------------------------------------

static inline hal_handle_id_ht_entry_t *
tcp_proxy_hal_handle_id_ht_entry_alloc (void)
{
    return ((hal_handle_id_ht_entry_t *)g_hal_state->
                hal_handle_id_ht_entry_slab()->alloc());
}

static inline void
tcp_proxy_hal_handle_id_ht_entry_free (hal_handle_id_ht_entry_t *entry)
{
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
}

static inline void
tcp_proxy_hal_handle_id_ht_entry_init (hal_handle_id_ht_entry_t *entry,
                             hal_handle_t hal_hdl)
{
    entry->handle_id = hal_hdl;
}

static inline void
tcp_proxy_hal_handle_id_ht_entry_uninit (hal_handle_id_ht_entry_t *entry)
{
}

static inline hal_handle_id_ht_entry_t *
tcp_proxy_hal_handle_id_ht_entry_alloc_init (hal_handle_t hal_hdl)
{
    hal_handle_id_ht_entry_t *entry;

    if ((entry = tcp_proxy_hal_handle_id_ht_entry_alloc()) == NULL)
        return NULL;

    tcp_proxy_hal_handle_id_ht_entry_init(entry, hal_hdl);
    return entry;
}

static inline void
tcp_proxy_hal_handle_id_ht_entry_uninit_free (hal_handle_id_ht_entry_t *entry)
{
    if (entry) {
        tcp_proxy_hal_handle_id_ht_entry_uninit(entry);
        tcp_proxy_hal_handle_id_ht_entry_free(entry);
    }
}

static inline hal_ret_t
tcp_proxy_hal_handle_id_ht_entry_db_add (ht *root, ht_ctxt_t *ht_ctxt, void *key,
                               hal_handle_id_ht_entry_t *entry)
{
    sdk_ret_t sdk_ret;

    sdk_ret = g_hal_state->tcp_proxy_policy_ht()->insert_with_key(
        key, entry, ht_ctxt);

    hal_ret_t ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        tcp_proxy_hal_handle_id_ht_entry_uninit_free(entry);
        return ret;
    }
    return ret;
}

static inline hal_ret_t
tcp_proxy_cfg_pol_create_db_handle (tcp_proxy_cfg_pol_t *pol)
{
    hal_handle_id_ht_entry_t *entry;
    hal_ret_t ret;
    if ((entry = hal_handle_id_ht_entry_alloc_init(
            g_hal_state->hal_handle_id_ht_entry_slab(),
            pol->hal_hdl)) == NULL)
        return HAL_RET_OOM;
    HAL_TRACE_DEBUG("Came here");
 
    ret =  hal_handle_id_ht_entry_db_add(
        g_hal_state->tcp_proxy_policy_ht(), &pol->key, entry);
    
    HAL_TRACE_DEBUG("Came here - ret {}", ret);
    return ret;
}

//-----------------------------------------------------------------------------
// TcpProxy config object to its corresponding HAL handle management routines
//
// The TcpProxy config object can be obtained through key or hal handle. The node
// is inserted/deleted in two different hash tables.
//   a) key: hal_handle, data: config object
//   b) key: config object key, data: hal_handle
// In case of #b, a second lookup is done to get to object from hal_handle
//-----------------------------------------------------------------------------

static inline tcp_proxy_cfg_pol_t *
tcp_proxy_cfg_pol_hal_hdl_db_lookup (hal_handle_t hal_hdl)
{
    HAL_TRACE_DEBUG("hal_hdl {}", hal_hdl);
    if (hal_hdl == HAL_HANDLE_INVALID) {
        HAL_TRACE_DEBUG("Entered here");
        return NULL;
     }

    auto hal_hdl_e = hal_handle_get_from_handle_id(hal_hdl);
    if (hal_hdl_e == NULL || hal_hdl_e->obj_id() != HAL_OBJ_ID_TCP_PROXY_POLICY) {
        HAL_TRACE_DEBUG("Entered here");
        return NULL;
    }

    return (tcp_proxy_cfg_pol_t *) hal_handle_get_obj(hal_hdl);
}

static inline tcp_proxy_cfg_pol_t *
tcp_proxy_cfg_pol_db_lookup (tcp_proxy_cfg_pol_key_t *key)
{
    hal_handle_id_ht_entry_t *entry;

    if ((entry = hal_handle_id_ht_entry_db_lookup(
            g_hal_state->tcp_proxy_policy_ht(), key)) == NULL) {
        HAL_TRACE_DEBUG("Entered here");
        return NULL;
    }

    return tcp_proxy_cfg_pol_hal_hdl_db_lookup(entry->handle_id);
}

static inline hal_ret_t
tcp_proxy_cfg_pol_delete_db_handle (tcp_proxy_cfg_pol_key_t *key)
{
    hal_handle_id_ht_entry_t *entry;

    if ((entry = hal_handle_id_ht_entry_db_del(
            g_hal_state->tcp_proxy_policy_ht(), key)) == NULL)
        return HAL_RET_TCP_PROXY_RULE_NOT_FOUND;

    hal_handle_id_ht_entry_uninit_free(entry);
    return HAL_RET_OK;
}

static inline void tcp_proxy_acl_ctx_name (char *name, vrf_id_t vrf_id)
{
    //thread_local static char name[ACL_NAMESIZE];
    std::snprintf(name, ACL_NAMESIZE, "tcp_proxy-ipv4-rules:%lu", vrf_id);
    HAL_TRACE_DEBUG("Returning {}", name);
    //return name;
}

static inline void
tcp_proxy_cfg_pol_create_rsp_build (tcp_proxy::TcpProxyRuleResponse *rsp, hal_ret_t ret,
                                hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK)
        rsp->mutable_status()->set_handle(hal_handle);
    rsp->set_api_status(hal_prepare_rsp(ret));
}

static inline void
tcp_proxy_cfg_pol_delete_rsp_build (tcp_proxy::TcpProxyRuleDeleteResponse *rsp, hal_ret_t ret)
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
tcp_proxy_cfg_rule_acl_build (tcp_proxy_cfg_rule_t *rule, const acl_ctx_t **acl_ctx)
{
    return rule_match_rule_add(acl_ctx, &rule->match, rule->key.rule_id, rule->prio, &rule->ref_count);
}

static inline void
tcp_proxy_cfg_rule_acl_cleanup (tcp_proxy_cfg_rule_t *rule)
{
    return;
}

static inline hal_ret_t
tcp_proxy_cfg_pol_rule_acl_build (tcp_proxy_cfg_pol_t *pol, const acl_ctx_t **acl_ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    tcp_proxy_cfg_rule_t *rule;
    dllist_ctxt_t *entry;
    uint32_t prio = 0;

    dllist_for_each(entry, &pol->rule_list) {
        rule = dllist_entry(entry, tcp_proxy_cfg_rule_t, list_ctxt);
        rule->prio = prio++;
        if ((ret = tcp_proxy_cfg_rule_acl_build(rule, acl_ctx)) != HAL_RET_OK)
            return ret;
    }

    return ret;
}

static inline hal_ret_t
tcp_proxy_cfg_pol_acl_build (tcp_proxy_cfg_pol_t *pol, const acl_ctx_t **out_acl_ctx)
{
    hal_ret_t ret = HAL_RET_ERR;
    const acl_ctx_t *acl_ctx;
    char acl_name[ACL_NAMESIZE];

    tcp_proxy_acl_ctx_name(acl_name, pol->key.vrf_id);
    if ((acl_ctx = rule_lib_init(acl_name, &tcp_proxy_ip_acl_config_glbl)) == NULL)
        return ret;

    if ((ret = tcp_proxy_cfg_pol_rule_acl_build(pol, &acl_ctx)) != HAL_RET_OK) 
        return ret;

    *out_acl_ctx = acl_ctx;
    return ret;
}

static inline void
tcp_proxy_cfg_pol_rule_acl_cleanup (tcp_proxy_cfg_pol_t *pol)
{
    tcp_proxy_cfg_rule_t *rule;
    dllist_ctxt_t *entry;

    dllist_for_each(entry, &pol->rule_list) {
        rule = dllist_entry(entry, tcp_proxy_cfg_rule_t, list_ctxt);
        tcp_proxy_cfg_rule_acl_cleanup(rule);
    }
}

static inline hal_ret_t
tcp_proxy_cfg_pol_acl_cleanup (tcp_proxy_cfg_pol_t *pol)
{
    char acl_name[ACL_NAMESIZE];

    tcp_proxy_acl_ctx_name(acl_name, pol->key.vrf_id);
    tcp_proxy_cfg_pol_rule_acl_cleanup(pol);
    rule_lib_delete(acl_name);
    return HAL_RET_OK;
}

// allocate a tcp_proxy_cbment instance
static inline tcp_proxy_cb_t *
tcp_proxy_cb_alloc (void)
{
    tcp_proxy_cb_t    *tcp_proxy_cb;

    tcp_proxy_cb = (tcp_proxy_cb_t *)g_hal_state->tcpcb_slab()->alloc();
    if (tcp_proxy_cb == NULL) {
        return NULL;
    }
    return tcp_proxy_cb;
}

// initialize a tcp_proxy_cbment instance
static inline tcp_proxy_cb_t *
tcp_proxy_cb_init (tcp_proxy_cb_t *tcp_proxy_cb)
{
    if (!tcp_proxy_cb) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&tcp_proxy_cb->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    tcp_proxy_cb->pd = NULL;

    // initialize meta information
    tcp_proxy_cb->ht_ctxt.reset();
    tcp_proxy_cb->hal_handle_ht_ctxt.reset();

    return tcp_proxy_cb;
}

// allocate and initialize a TCPCB instance
static inline tcp_proxy_cb_t *
tcp_proxy_cb_alloc_init (void)
{
    return tcp_proxy_cb_init(tcp_proxy_cb_alloc());
}

static inline hal_ret_t
tcp_proxy_cb_free (tcp_proxy_cb_t *tcp_proxy_cb)
{
    HAL_SPINLOCK_DESTROY(&tcp_proxy_cb->slock);
    hal::delay_delete_to_slab(HAL_SLAB_TCPCB, tcp_proxy_cb);
    return HAL_RET_OK;
}

static inline tcp_proxy_cb_t *
find_tcp_proxy_cb_by_id (tcp_proxy_cb_id_t tcp_proxy_cb_id)
{
    return (tcp_proxy_cb_t *)g_hal_state->tcpcb_id_ht()->lookup(&tcp_proxy_cb_id);
}

extern void *tcp_proxy_cb_get_key_func(void *entry);
extern uint32_t tcp_proxy_cb_compute_hash_func(void *key, uint32_t ht_size);
extern bool tcp_proxy_cb_compare_key_func(void *key1, void *key2);

extern void *tcp_proxy_cb_get_handle_key_func(void *entry);
extern uint32_t tcp_proxy_cb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool tcp_proxy_cb_compare_handle_key_func(void *key1, void *key2);


extern void *tcp_proxy_rule_get_key_func(void *entry);
extern uint32_t tcp_proxy_rule_compute_hash_func(void *key, uint32_t ht_size);
extern bool tcp_proxy_rule_compare_key_func(void *key1, void *key2);

extern void *tcp_proxy_rule_get_handle_key_func(void *entry);
extern uint32_t tcp_proxy_rule_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool tcp_proxy_rule_compare_handle_key_func(void *key1, void *key2);
extern const acl::acl_ctx_t *
tcp_proxy_cfg_pol_create_app_ctxt_init(tcp_proxy_cfg_pol_t *pol);
extern hal_ret_t
tcp_proxy_cfg_pol_create_oper_handle(tcp_proxy_cfg_pol_t *pol);
extern void
tcp_proxy_cfg_pol_rsp_build(tcp_proxy::TcpProxyRuleResponse *rsp, hal_ret_t ret,
                        hal_handle_t hal_handle);
extern hal_ret_t
tcp_proxy_cfg_pol_rule_spec_build(tcp_proxy_cfg_pol_t *pol,
                               tcp_proxy::TcpProxyRuleSpec *spec);
extern hal_ret_t
tcp_proxy_cfg_rule_spec_build(tcp_proxy_cfg_rule_t *rule, tcp_proxy::TcpProxyRuleMatchSpec *spec, tcp_proxy::TcpProxyRuleSpec *rule_spec);

extern hal_ret_t
tcp_proxy_cfg_rule_spec_handle(const tcp_proxy::TcpProxyRuleMatchSpec& spec, dllist_ctxt_t *head);
extern hal_ret_t
tcp_proxy_cfg_rule_create_oper_handle(tcp_proxy_cfg_rule_t *rule, const acl_ctx_t *acl_ctx);

hal_ret_t tcp_proxy_rule_create(tcp_proxy::TcpProxyRuleSpec& spec,
                       tcp_proxy::TcpProxyRuleResponse *rsp);

hal_ret_t tcp_proxy_rule_update(tcp_proxy::TcpProxyRuleSpec& spec,
                       tcp_proxy::TcpProxyRuleResponse *rsp);

hal_ret_t tcp_proxy_rule_delete(tcp_proxy::TcpProxyRuleDeleteRequest& req,
                       tcp_proxy::TcpProxyRuleDeleteResponse *rsp);

hal_ret_t tcp_proxy_rule_get(tcp_proxy::TcpProxyRuleGetRequest& req,
                    tcp_proxy::TcpProxyRuleGetResponseMsg *rsp);


extern void *tcp_proxy_cb_get_key_func(void *entry);
extern uint32_t tcp_proxy_cb_compute_hash_func(void *key, uint32_t ht_size);
extern bool tcp_proxy_cb_compare_key_func(void *key1, void *key2);

extern void *tcp_proxy_cb_get_handle_key_func(void *entry);
extern uint32_t tcp_proxy_cb_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool tcp_proxy_cb_compare_handle_key_func(void *key1, void *key2);

hal_ret_t tcp_proxy_cb_create(tcp_proxy::TcpProxyCbSpec& spec,
                       tcp_proxy::TcpProxyCbResponse *rsp);

hal_ret_t tcp_proxy_cb_update(tcp_proxy::TcpProxyCbSpec& spec,
                       tcp_proxy::TcpProxyCbResponse *rsp);

hal_ret_t tcp_proxy_cb_delete(tcp_proxy::TcpProxyCbDeleteRequest& req,
                       tcp_proxy::TcpProxyCbDeleteResponseMsg *rsp);

hal_ret_t tcp_proxy_cb_get(tcp_proxy::TcpProxyCbGetRequest& req,
                    tcp_proxy::TcpProxyCbGetResponseMsg *rsp);

hal_ret_t tcp_proxy_session_get(tcp_proxy::TcpProxySessionGetRequest& req,
                    tcp_proxy::TcpProxySessionGetResponseMsg *rsp);

hal_ret_t tcp_proxy_global_stats_get(tcp_proxy::TcpProxyGlobalStatsGetRequest& req,
                    tcp_proxy::TcpProxyGlobalStatsGetResponseMsg *rsp);

}    // namespace hal

#endif    // __TCP_PROXY_HPP__

