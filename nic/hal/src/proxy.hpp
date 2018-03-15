#ifndef __PROXY_HPP__
#define __PROXY_HPP__

#include "sdk/indexer.hpp"
#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/proxy.pb.h"
#include "nic/include/pd.hpp"
#include "nic/hal/src/session.hpp"

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
#define SERVICE_LIF_START                       1001
#define HAL_PROXY_MAX_ST_LIF_PER_SVC            5
#define HAL_PROXY_MAX_QTYPE_PER_LIF             8

enum {
    SERVICE_LIF_TCP_PROXY = SERVICE_LIF_START,
    SERVICE_LIF_TLS_PROXY,
    SERVICE_LIF_CPU,
    SERVICE_LIF_IPSEC_ESP,
    SERVICE_LIF_IPFIX,
    SERVICE_LIF_APP_REDIR,
    SERVICE_LIF_GC,
    SERVICE_LIF_P4PT,
    SERVICE_LIF_END
};

inline uint16_t parse_service_lif(const char *s)
{
    static std::map<std::string, uint16_t> _map = {
        { "SERVICE_LIF_TCP_PROXY", SERVICE_LIF_TCP_PROXY},
        { "SERVICE_LIF_TLS_PROXY", SERVICE_LIF_TLS_PROXY},
        { "SERVICE_LIF_CPU"      , SERVICE_LIF_CPU},
        { "SERVICE_LIF_IPSEC_ESP", SERVICE_LIF_IPSEC_ESP},
        { "SERVICE_LIF_IPFIX"    , SERVICE_LIF_IPFIX},
        { "SERVICE_LIF_APP_REDIR", SERVICE_LIF_APP_REDIR},
        { "SERVICE_LIF_GC"       , SERVICE_LIF_GC},
    };

    return _map[s];
}

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
    hal_spinlock_t        slock;                   // lock to protect this structure
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

typedef struct proxy_flow_info_s {
    hal_spinlock_t      slock;                   // lock to protect this structure
    flow_key_t          flow_key;                // Flow 
    qid_t               qid1;                    // qid instance 1 (e.g. hflow)
    qid_t               qid2;                    // qid instance 2 (e.g. nflow)
  
    union {
        ipsec_flow_info_t    ipsec;
    } u;

    proxy_t*            proxy;                   // proxy service
    ht_ctxt_t           flow_ht_ctxt;            // Hash table for flow info
} __PACK__ proxy_flow_info_t;


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
extern uint32_t proxy_flow_ht_compute_hash_func(void *key, uint32_t ht_size);
extern bool proxy_flow_ht_compare_key_func(void *key1, void *key2);

extern void *proxy_get_key_func(void *entry);
extern uint32_t proxy_compute_hash_func(void *key, uint32_t ht_size);
extern bool proxy_compare_key_func(void *key1, void *key2);

extern void *proxy_get_handle_key_func(void *entry);
extern uint32_t proxy_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool proxy_compare_handle_key_func(void *key1, void *key2);

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
                            proxy::ProxyResponse *rsp = NULL,
                            const proxy::IpsecFlowConfig *ipsec_flow_config = NULL);

proxy_flow_info_t* proxy_get_flow_info(types::ProxyType proxy_type,
                                       const flow_key_t* flow_key);

hal_ret_t proxy_get_flow_info(proxy::ProxyGetFlowInfoRequest& req,
                              proxy::ProxyGetFlowInfoResponse* rsp);

bool is_proxy_enabled_for_flow(types::ProxyType proxy_type,
                               const flow_key_t &flow_key);

hal_ret_t proxy_globalcfg_set(proxy::ProxyGlobalCfgRequest& req,
			      proxy::ProxyGlobalCfgResponseMsg *rsp);

}    // namespace hal

#endif    // __PROXY_HPP__

