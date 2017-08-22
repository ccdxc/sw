#ifndef __PROXY_HPP__
#define __PROXY_HPP__

#include <indexer.hpp>
#include <base.h>
#include <ht.hpp>
#include <proxy.pb.h>
#include <pd.hpp>
#include <hal_state.hpp>

using hal::utils::indexer;
using hal::utils::ht_ctxt_t;

namespace hal {

#define HAL_MAX_PROXY                           5
#define HAL_MAX_QID                             16777215
#define SERVICE_LIF_START                       1001

#define PROXY_TCP_DEF_QTYPE                     0
#define PROXY_TCP_DEF_QSTATE_SZ                 4       // 512-bytes
#define PROXY_TCP_DEF_QSTATE_ENTRIES            10      // 1024: FIXME
#define PROXY_TLS_DEF_QTYPE                     0
#define PROXY_TLS_DEF_QSTATE_SZ                 1       // 64-bytes
#define PROXY_TLS_DEF_QSTATE_ENTRIES            10      // 1024: FIXME

enum {
    SERVICE_LIF_TCP_PROXY = SERVICE_LIF_START,
    SERVICE_LIF_TLS_PROXY,
    SERVICE_LIF_END
};

typedef uint32_t lif_id_t;
typedef uint8_t  qtype_t;
typedef uint32_t  qid_t;

typedef struct proxy_s {
    hal_spinlock_t        slock;                   // lock to protect this structure
    types::ProxyType      type;                    // Proxy Type
    lif_id_t              lif_id;                  // LIF for this service
    qtype_t               qtype;                   // Default QType
    uint8_t               qstate_size;             // Size of each qstate
    uint8_t               qstate_entries;          // # of entries in the qstate

    // operational state of Proxy
    hal_handle_t          hal_handle;              // HAL allocated handle
    uint64_t              base_addr;               // Base address of qstate
    // PD state
    void                  *pd;                     // all PD specific state
    
    indexer               *qid_idxr_;              // Indexer to allocate qid
    ht_ctxt_t             ht_ctxt;                 // id based hash table ctxt
    ht_ctxt_t             hal_handle_ht_ctxt;      // hal handle based hash able ctxt
} __PACK__ proxy_t;


// allocate a proxyment instance
static inline proxy_t *
proxy_alloc (void)
{
    proxy_t    *proxy;

    proxy = (proxy_t *)g_hal_state->proxy_slab()->alloc();
    if (proxy == NULL) {
        return NULL;
    }
    return proxy;
}

// initialize a proxyment instance
static inline proxy_t *
proxy_init (proxy_t *proxy)
{
    if (!proxy) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&proxy->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    proxy->pd = NULL;

    // initialize meta information
    proxy->ht_ctxt.reset();
    proxy->hal_handle_ht_ctxt.reset();

    return proxy;
}

// allocate and initialize a PROXY instance
static inline proxy_t *
proxy_alloc_init (void)
{
    return proxy_init(proxy_alloc());
}

static inline hal_ret_t
proxy_free (proxy_t *proxy)
{
    HAL_SPINLOCK_DESTROY(&proxy->slock);
    g_hal_state->proxy_slab()->free(proxy);
    return HAL_RET_OK;
}

static inline proxy_t *
find_proxy_by_type (types::ProxyType proxy_type)
{
    return (proxy_t *)g_hal_state->proxy_type_ht()->lookup(&proxy_type);
}

static inline proxy_t *
find_proxy_by_handle (hal_handle_t handle)
{
    return (proxy_t *)g_hal_state->proxy_hal_handle_ht()->lookup(&handle);
}

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

hal_ret_t proxy_allocate_qid(types::ProxyType type,
                             lif_id_t&   lif,
                             qtype_t& qtype,
                             qid_t&   qid);

hal_ret_t
proxy_program_qid(types::ProxyType type, lif_id_t lif, 
                  qtype_t qtype, qid_t qid);

hal_ret_t
proxy_allocate_program_qid(types::ProxyType type,
                           lif_id_t&        lif,
                           qtype_t&         qtype, 
                           qid_t&           qid); 

hal_ret_t proxy_enable(proxy::ProxySpec& spec,
                       proxy::ProxyResponse *rsp);

hal_ret_t proxy_update(proxy::ProxySpec& spec,
                       proxy::ProxyResponse *rsp);

hal_ret_t proxy_get(proxy::ProxyGetRequest& req,
                    proxy::ProxyGetResponse *rsp);
}    // namespace hal

#endif    // __PROXY_HPP__

