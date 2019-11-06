//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __PROXY_HPP__
#define __PROXY_HPP__

#include "lib/indexer/indexer.hpp"
#include "nic/include/base.hpp"
#include "lib/ht/ht.hpp"
#include "gen/proto/proxy.pb.h"
#include "nic/include/globals.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "gen/proto/tcp_proxy.pb.h"

using sdk::lib::indexer;
using sdk::lib::ht_ctxt_t;

using proxy::ProxySpec;
using proxy::ProxyStatus;
using proxy::ProxyResponse;
using proxy::ProxyKeyHandle;
using proxy::ProxyRequestMsg;
using proxy::ProxyResponseMsg;
using proxy::ProxyDisableRequestMsg;
using proxy::ProxyDisableResponseMsg;
using proxy::ProxyGetRequest;
using proxy::ProxyGetRequestMsg;
using proxy::ProxyGetResponse;
using proxy::ProxyGetResponseMsg;

namespace hal {

#define HAL_MAX_PROXY                           7
#define HAL_MAX_PROXY_FLOWS                     16000
#define HAL_MAX_QID                             16777215

typedef struct proxy_meta_qtype_s {
    qtype_t     qtype_val;
    uint8_t     qstate_size;
    uint8_t     qstate_entries;
} proxy_meta_qtype_t;

typedef struct proxy_meta_lif_s {
    lif_id_t    lif_id;
    uint32_t    num_qtype;
    proxy_meta_qtype_t  qtype_info[HAL_PROXY_MAX_QTYPE_PER_LIF];
    uint32_t    lport_id;                // lport-id for the proxy lif
} proxy_meta_lif_t;

typedef struct proxy_meta_s {
    bool                is_system_svc;
    uint32_t            num_lif;
    proxy_meta_lif_t    lif_info[HAL_PROXY_MAX_ST_LIF_PER_SVC];
    types::ProxyType    parent_proxy;
    bool                indexer_skip_zero;
} proxy_meta_t;

typedef struct proxy_s {
    sdk_spinlock_t        slock;                   // lock to protect this structure
    types::ProxyType      type;                    // Proxy Type

    // meta
    proxy_meta_t          *meta;                    // meta information
    // operational state of Proxy
    hal_handle_t          hal_handle;              // HAL allocated handle

    // PD state
    void                  *pd;                     // all PD specific state

    indexer               *qid_idxr_;              // Indexer to allocate qid
    ht                    *flow_ht_;               // hash table to store per flow info

    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash able ctxt
} __PACK__ proxy_t;


typedef struct ipsec_esp_flow_info_s {

} ipsec_esp_flow_info_t;

typedef struct ipsec_host_flow_info_s {
    flow_key_t      esp_flow_key;
} ipsec_host_flow_info_t;

typedef struct ipsec_flow_info_s {

    union {
        ipsec_host_flow_info_t  host_flow;
        ipsec_esp_flow_info_t   esp_flow;
    } u;
} ipsec_flow_info_t;

typedef struct tls_proxy_flow_info_ecdsa_keys_s {
    uint32_t                 sign_key_id;
} tls_proxy_flow_info_ecdsa_keys_t;

typedef struct tls_proxy_flow_info_rsa_keys_s {
    uint32_t                 sign_key_id;
    uint32_t                 decrypt_key_id;
} tls_proxy_flow_info_rsa_keys_t;

typedef struct tls_proxy_flow_info_s {
    bool                     is_valid;
    uint32_t                 cert_id;
    std::string              ciphers;
    types::CryptoAsymKeyType key_type;
    tcp_proxy::TlsProxySide  tls_proxy_side;
    union {
        tls_proxy_flow_info_ecdsa_keys_t ecdsa_keys;
        tls_proxy_flow_info_rsa_keys_t   rsa_keys;
    } u;
} tls_proxy_flow_info_t;

typedef struct proxy_flow_info_s {
    sdk_spinlock_t      slock;                   // lock to protect this structure
    flow_key_t          flow_key;                // Flow
    qid_t               qid1;                    // qid instance 1 (e.g. hflow)
    qid_t               qid2;                    // qid instance 2 (e.g. nflow)
    
    types::ProxyType    proxy_type;
    union {
        ipsec_flow_info_t           ipsec;
        tls_proxy_flow_info_t       tlsproxy;
    } u;

    proxy_t*            proxy;                   // proxy service
    ht_ctxt_t           flow_ht_ctxt;            // Hash table for flow info
} proxy_flow_info_t;


static inline proxy_flow_info_t*
find_proxy_flow_info(proxy_t* proxy, const flow_key_t* flow_key)
{
    if(!proxy) {
        HAL_TRACE_ERR("Proxy is NULL");
        return NULL;
    }
    return (proxy_flow_info_t *) proxy->flow_ht_->lookup((void *)flow_key);
}

extern void *proxy_flow_ht_get_key_func(void *entry);
extern uint32_t proxy_flow_ht_key_size(void);

extern void *proxy_get_key_func(void *entry);
extern uint32_t proxy_key_size(void);

extern void *proxy_get_handle_key_func(void *entry);
extern uint32_t proxy_handle_key_size(void);

/****************************
 * INIT APIs
 ***************************/

hal_ret_t hal_proxy_svc_init(void);

hal_ret_t proxy_enable(proxy::ProxySpec& spec,
                       proxy::ProxyResponse *rsp);

hal_ret_t proxy_update(proxy::ProxySpec& spec,
                       proxy::ProxyResponse *rsp);

hal_ret_t proxy_get(proxy::ProxyGetRequest& req,
                    proxy::ProxyGetResponse *rsp);

hal_ret_t proxy_flow_config(proxy::ProxyFlowConfigRequest& req,
                            proxy::ProxyResponse *rsp);

hal_ret_t proxy_flow_enable(types::ProxyType proxy_type,
                            const flow_key_t &flow_key,
                            bool alloc_qid,
                            proxy::ProxyResponse *rsp,
                            const proxy::IpsecFlowConfig *req);
hal_ret_t
proxy_flow_handle_tls_config(types::ProxyType proxy_type,
                             const flow_key_t &flow_key,
                             const proxy::TlsProxyFlowConfig &tls_flow_config,
                             proxy::ProxyResponse *rsp);

proxy_flow_info_t* proxy_get_flow_info(types::ProxyType proxy_type,
                                       flow_key_t flow_key);

hal_ret_t proxy_get_flow_info(proxy::ProxyGetFlowInfoRequest& req,
                              proxy::ProxyGetFlowInfoResponse* rsp);

bool is_proxy_enabled_for_flow(types::ProxyType proxy_type,
                               flow_key_t flow_key);

hal_ret_t proxy_globalcfg_set(proxy::ProxyGlobalCfgRequest& req,
			      proxy::ProxyGlobalCfgResponseMsg *rsp);

}    // namespace hal

#endif    // __PROXY_HPP__

