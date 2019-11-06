//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/src/internal/cpucb.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/app_redir_shared.h"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/src/internal/cpucb.hpp"
#include "nic/hal/src/internal/rawrcb.hpp"
#include "nic/hal/src/internal/rawccb.hpp"
#include "nic/hal/src/internal/proxyrcb.hpp"
#include "nic/hal/src/internal/proxyccb.hpp"
#include "nic/hal/src/internal/p4pt.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/tls/tls_api.hpp"

namespace hal {

static proxy_meta_t g_meta[types::ProxyType_ARRAYSIZE];

hal_ret_t
proxy_meta_init() {
    /*
     * Add meta info for each service
     * Fomat: is system, lif, qtype, qstate size, qstate entries
     */

    // CB size 1024, num_entries 32K
    g_meta[types::PROXY_TYPE_TCP] =
        (proxy_meta_t) {false, 1, {SERVICE_LIF_TCP_PROXY, 2, {{0, 5, 15}, {1, 1, 15}}},types::PROXY_TYPE_NONE, false};

    // CB size 1024, num_entries 32K
    g_meta[types::PROXY_TYPE_TLS] =
        (proxy_meta_t) {false, 1, {SERVICE_LIF_TLS_PROXY, 1, {0, 5, 15}}, types::PROXY_TYPE_NONE, false};

    g_meta[types::PROXY_TYPE_IPSEC] =
        (proxy_meta_t) {true, 1, {SERVICE_LIF_IPSEC_ESP, 2, {{0, 3, 10}, {1, 3, 10}}},
                        types::PROXY_TYPE_NONE, false};

    // num qtype = 2 (RNMDR, TNMDR)
    // CB size 64 bytes
    // num_entries = 8 (number of producers - tcp, tls etc.)
    //  QID 0 : TCP Producer
    //  QID 1 : TLS Producer
    //  QID 2 : IPSEC Producer
    //  QID 3 : CPU Producer
    //  QID 4 : ...
    g_meta[types::PROXY_TYPE_GC] =
        (proxy_meta_t) {true, 1, {SERVICE_LIF_GC, 2, {{0, 1, 3}, {1, 1, 3}}}, types::PROXY_TYPE_NONE, false};

    g_meta[types::PROXY_TYPE_IPFIX] =
        (proxy_meta_t) {true, 1, {SERVICE_LIF_IPFIX, 1, {0, 1, 5}}, types::PROXY_TYPE_NONE, false};

    g_meta[types::PROXY_TYPE_APP_REDIR] =
        (proxy_meta_t) {false, 1, {SERVICE_LIF_APP_REDIR, APP_REDIR_NUM_QTYPES_MAX,
            {{APP_REDIR_RAWR_QTYPE, RAWRCB_TABLE_ENTRY_MULTIPLE,
                        (uint8_t)log2(RAWRCB_NUM_ENTRIES_MAX)},
             {APP_REDIR_RAWC_QTYPE, RAWCCB_TABLE_ENTRY_MULTIPLE,
                        (uint8_t)log2(RAWCCB_NUM_ENTRIES_MAX)},
             {APP_REDIR_PROXYR_QTYPE, PROXYRCB_TABLE_ENTRY_MULTIPLE,
                        (uint8_t)log2(PROXYRCB_NUM_ENTRIES_MAX)},
             {APP_REDIR_PROXYC_QTYPE, PROXYCCB_TABLE_ENTRY_MULTIPLE,
                        (uint8_t)log2(PROXYCCB_NUM_ENTRIES_MAX)}}},
             types::PROXY_TYPE_NONE, true};
    g_meta[types::PROXY_TYPE_APP_REDIR_PROXY_TCP] = {false, 0, {}, types::PROXY_TYPE_TCP, false},
    g_meta[types::PROXY_TYPE_APP_REDIR_SPAN] = {false, 0, {}, types::PROXY_TYPE_APP_REDIR, true},
    g_meta[types::PROXY_TYPE_APP_REDIR_PROXY_TCP_SPAN] = {false, 0, {}, types::PROXY_TYPE_TCP, false},
    g_meta[types::PROXY_TYPE_NVME] = {false, 0, {}, types::PROXY_TYPE_TCP, false},

    // 128 bytes of P4PT state per connection (e.g. dir) and a total of 2^12 connections
    g_meta[types::PROXY_TYPE_P4PT] =
        (proxy_meta_t) {false, 1, {SERVICE_LIF_P4PT, 1, {0, 1, 12}}, types::PROXY_TYPE_NONE, false};

    return HAL_RET_OK;
}

void *
proxy_flow_ht_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((proxy_flow_info_t *)entry)->flow_key);
}

uint32_t proxy_flow_key_size(void) {
    return sizeof(flow_key_t);
}

void *
proxy_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((proxy_t *)entry)->type);
}

uint32_t proxy_key_size(void) {
    return sizeof(types::ProxyType);
}

void *
proxy_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((proxy_t *)entry)->hal_handle);
}

uint32_t proxy_handle_key_size(void) {
    return sizeof(hal_handle_t);
}

// allocate a proxy instance
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
    SDK_SPINLOCK_INIT(&proxy->slock, PTHREAD_PROCESS_PRIVATE);

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
    SDK_SPINLOCK_DESTROY(&proxy->slock);
    hal::delay_delete_to_slab(HAL_SLAB_PROXY, proxy);
    return HAL_RET_OK;
}

// allocate a proxy flow info instance
static inline proxy_flow_info_t *
proxy_flow_info_alloc (void)
{
    proxy_flow_info_t       *pfi;
    pfi = (proxy_flow_info_t *)g_hal_state->proxy_flow_info_slab()->alloc();
    if(pfi == NULL) {
        return NULL;
    }
    return pfi;
}

// initialize a proxyment instance
static inline proxy_flow_info_t *
proxy_flow_info_init (proxy_flow_info_t *pfi)
{
    if(!pfi) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&pfi->slock, PTHREAD_PROCESS_PRIVATE);
    pfi->flow_ht_ctxt.reset();
    return pfi;
}

// allocate and initialize a PROXY instance
static inline proxy_flow_info_t *
proxy_flow_info_alloc_init (void)
{
    return proxy_flow_info_init(proxy_flow_info_alloc());
}

static inline hal_ret_t
proxy_flow_info_free (proxy_flow_info_t *proxy_flow_info)
{
    SDK_SPINLOCK_DESTROY(&proxy_flow_info->slock);
    hal::delay_delete_to_slab(HAL_SLAB_PROXY_FLOW_INFO,
                                        proxy_flow_info);
    return HAL_RET_OK;
}

static inline proxy_t *
find_proxy_by_type (types::ProxyType proxy_type)
{
    return (proxy_t *)g_hal_state->proxy_type_ht()->lookup(&proxy_type);
}

//------------------------------------------------------------------------------
// validate an incoming PROXY enable request
// TODO:
// 1. check if PROXY exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_proxy_enable (ProxySpec& spec, ProxyResponse *rsp)
{
    // must have key-handle set
    if (spec.proxy_type() == types::PROXY_TYPE_NONE) {
        rsp->set_api_status(types::API_STATUS_PROXY_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this Proxy in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_proxy_to_db (proxy_t *proxy)
{
    g_hal_state->proxy_type_ht()->insert(proxy, &proxy->ht_ctxt);
    return HAL_RET_OK;
}

static inline hal_ret_t
add_proxy_flow_info_to_db(proxy_flow_info_t* pfi)
{
    pfi->proxy->flow_ht_->insert(pfi, &pfi->flow_ht_ctxt);
    return HAL_RET_OK;
}

static const char* proxy_type_to_str(types::ProxyType type)
{
    switch(type) {
        case types::PROXY_TYPE_NONE: return "none";
        case types::PROXY_TYPE_TCP:  return "tcp_proxy";
        case types::PROXY_TYPE_TLS:  return "tls_proxy";
        case types::PROXY_TYPE_IPSEC: return "ipsec_proxy";
        case types::PROXY_TYPE_GC: return "gc_proxy";
        case types::PROXY_TYPE_IPFIX: return "ipf_proxy";
        case types::PROXY_TYPE_APP_REDIR: return "AR_proxy";
        case types::PROXY_TYPE_P4PT: return "p4pt_proxy";
        case types::PROXY_TYPE_APP_REDIR_PROXY_TCP: return "AR_tcp_proxy";
        case types::PROXY_TYPE_APP_REDIR_SPAN: return "AR_span_proxy";
        case types::PROXY_TYPE_APP_REDIR_PROXY_TCP_SPAN: return "AR_tcp_span_proxy";
        default: return "error";
    }
}


//-----------------------------------------------------------------------------
// API to program LIF
//-----------------------------------------------------------------------------
hal_ret_t
proxy_program_lif(proxy_t* proxy)
{
    hal_ret_t                         ret = HAL_RET_OK;
    intf::LifSpec                     lif_spec;
    intf::LifResponse                 rsp;
    lif_hal_info_t                    lif_hal_info = {0};
    proxy_meta_t                      *meta = &g_meta[proxy->type];
    proxy_meta_lif_t                  *meta_lif_info = NULL;
    proxy_meta_qtype_t                *meta_qtype_info = NULL;
    pd::pd_lif_get_lport_id_args_t    args = { 0 };
    pd::pd_func_args_t                pd_func_args = {0};

    // program LIF(s)
    for(uint i = 0; i < meta->num_lif; i++) {
        meta_lif_info = &(meta->lif_info[i]);
        lif_spec.Clear();
        // Create LIF
        lif_spec.mutable_key_or_handle()->set_lif_id(meta_lif_info->lif_id);
        lif_spec.set_admin_status(intf::IF_STATUS_UP);
        lif_spec.set_name(proxy_type_to_str(proxy->type));
        lif_hal_info.with_hw_lif_id = true;
        lif_hal_info.hw_lif_id = meta_lif_info->lif_id;
        lif_hal_info.dont_zero_qstate_mem = true;

        for(uint j = 0; j < meta_lif_info->num_qtype; j++) {
            meta_qtype_info = &(meta_lif_info->qtype_info[j]);
            lif_spec.add_lif_qstate_map();
            lif_spec.mutable_lif_qstate_map(j)->set_type_num(meta_qtype_info->qtype_val);
            lif_spec.mutable_lif_qstate_map(j)->set_size(meta_qtype_info->qstate_size);
            lif_spec.mutable_lif_qstate_map(j)->set_entries(meta_qtype_info->qstate_entries);

            //qstate_params.dont_zero_memory = true;
            HAL_TRACE_DEBUG("Added LIF: {}, entries: {}, size: {}",
                    meta_lif_info->lif_id,
                    meta_qtype_info->qstate_entries,
                    meta_qtype_info->qstate_size);
        }

        HAL_TRACE_DEBUG("Calling lif create with id: {}",
                    lif_hal_info.hw_lif_id);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = lif_create(lif_spec, &rsp, &lif_hal_info);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("lif creation failed for proxy service {} with id: {}",
                                    proxy->type, meta_lif_info->lif_id);
            hal::hal_cfg_db_close();
            return ret;
        } else {
            hal::hal_cfg_db_close();
        }

        // get lport-id for this lif
        lif_t* lif = find_lif_by_id(meta_lif_info->lif_id);
        SDK_ASSERT_RETURN((NULL != lif), HAL_RET_LIF_NOT_FOUND);

#if 0
        lif_args.pi_lif = lif;
        pd::lif_get_lport_id(&lif_args);
        meta_lif_info->lport_id = lif_args.lport_id;
#endif
        args.pi_lif = lif;
        pd_func_args.pd_lif_get_lport_id = &args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_GET_LPORTID, &pd_func_args);
        meta_lif_info->lport_id = args.lport_id;
        HAL_TRACE_DEBUG("Received lport-id: {} for lif: {}",
                        meta_lif_info->lport_id, meta_lif_info->lif_id);
    } // end lif loop
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a Proxy enable request
// TODO: if Proxy exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
proxy_create_cpucb(uint8_t cpucb_id)
{
    hal_ret_t               ret = HAL_RET_OK;
    cpucb::CpuCbSpec        spec;
    cpucb::CpuCbResponse    rsp;

    spec.mutable_key_or_handle()->set_cpucb_id(cpucb_id);
    ret = cpucb_create(spec, &rsp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create cpucb: {}", ret);
    }

    return ret;
}

hal_ret_t
proxy_create_cpucb(void)
{
    hal_ret_t   ret = HAL_RET_OK;
    for(int i = 0; i < types::CpucbId_ARRAYSIZE; i++) {
        ret = proxy_create_cpucb(i);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to create cpucb for id: {}, ret: {}", i, ret);
            return ret;
        }
    }

    return ret;
}


hal_ret_t
proxy_init_default_params(proxy_t* proxy)
{
    if(NULL == proxy)
    {
        return HAL_RET_INVALID_ARG;
    }

    proxy->meta = &g_meta[proxy->type];

    return HAL_RET_OK;
}

hal_ret_t
proxy_init_inherit_parent_meta(proxy_t* proxy)
{
    proxy_t         *parent_proxy;
    proxy_meta_t    *meta;

    if((NULL == proxy) || (NULL == proxy->meta))
    {
        return HAL_RET_INVALID_ARG;
    }

    meta = proxy->meta;
    if (!meta->num_lif && (meta->parent_proxy != types::PROXY_TYPE_NONE)) {
        parent_proxy = find_proxy_by_type(meta->parent_proxy);
        assert(parent_proxy);
        proxy->meta = parent_proxy->meta;
        proxy->qid_idxr_ = parent_proxy->qid_idxr_;
    }

    return HAL_RET_OK;
}

hal_ret_t
proxy_post_lif_program_init(proxy_t* proxy)
{
    hal_ret_t   ret = HAL_RET_OK;
    if(NULL == proxy)
    {
        return HAL_RET_INVALID_ARG;
    }

    // type specific proxy initialization
    switch(proxy->type) {
    case types::PROXY_TYPE_TLS:
        ret = hal::tls::tls_api_init();
        break;
    case types::PROXY_TYPE_P4PT:
        // TODO: how is this code supposed to run with another P4 program?
        //       the only interface between PI and PD is via p4pd_api.hpp
        //       can't directly go to iris !!!
        pd::p4pt_pd_init_args_t args;
        hal::pd::pd_func_args_t pd_func_args;
        pd_func_args.p4pt_pd_init = &args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_P4PT_INIT, &pd_func_args);
        break;
    default:
        break;

    }
    return ret;
}

proxy_t*
proxy_factory(types::ProxyType type)
{
    hal_ret_t       ret = HAL_RET_OK;
    proxy_t         * proxy = NULL;
    proxy_meta_t    *meta;

    assert(type < types::ProxyType_ARRAYSIZE);
    meta = &g_meta[type];

    // instantiate Proxy
    proxy = proxy_alloc_init();
    if (proxy == NULL) {
        HAL_TRACE_ERR("Failed to allocate proxy service");
        return NULL;
    }

    proxy->type = type;
    proxy->hal_handle = hal_alloc_handle();

    // Initialize flow info structures
    proxy->flow_ht_ = ht::factory(HAL_MAX_PROXY_FLOWS,
                                  hal::proxy_flow_ht_get_key_func,
                                  hal::proxy_flow_key_size());
    SDK_ASSERT(proxy->flow_ht_ != NULL);

    // Instantiate QID indexer
    if (meta->num_lif) {
        proxy->qid_idxr_ = sdk::lib::indexer::factory(HAL_MAX_QID, true,
                                              meta->indexer_skip_zero);
        SDK_ASSERT(NULL != proxy->qid_idxr_);
    }

   // initialize default params
    proxy_init_default_params(proxy);

    // program LIF for this proxy
    proxy_program_lif(proxy);

    // inherit parent's meta if applicable
    proxy_init_inherit_parent_meta(proxy);

    // post lif programming initialization
    proxy_post_lif_program_init(proxy);

    // add this Proxy to our db
    ret = add_proxy_to_db(proxy);
    SDK_ASSERT(ret == HAL_RET_OK);

    return proxy;
}

hal_ret_t
proxy_enable(ProxySpec& spec, ProxyResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    proxy_t                *proxy;

    // validate the request message
    ret = validate_proxy_enable(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("Proxy validate failure, err : {}", ret);
        return ret;
    }
    /* Validate if proxy type is already enabled */
    proxy = find_proxy_by_type(spec.proxy_type());
    if (proxy != NULL) {
        rsp->set_api_status(types::API_STATUS_OK);
        rsp->mutable_proxy_status()->set_proxy_handle(proxy->hal_handle);
        return HAL_RET_OK;
    }

    // instantiate Proxy
    proxy = proxy_factory(spec.proxy_type());
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_proxy_status()->set_proxy_handle(proxy->hal_handle);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Initialize default proxy services
//------------------------------------------------------------------------------
hal_ret_t
hal_proxy_system_svc_init(void)
{
    for(int i = 1; i < types::ProxyType_ARRAYSIZE; i++) {
        if(!g_meta[i].is_system_svc) {
            continue;
        }
        if(NULL ==  proxy_factory(types::ProxyType(i))) {
            HAL_TRACE_ERR("Failed to initialize service of type: {}",
                            types::ProxyType(i));
            return HAL_RET_NO_RESOURCE;
        }
        HAL_TRACE_DEBUG("Initialized service of type: {}",
                          types::ProxyType_Name(types::ProxyType(i)));
    }

    return HAL_RET_OK;
}

hal_ret_t
hal_proxy_svc_init(void)
{
    hal_ret_t       ret = HAL_RET_OK;
    sdk_ret_t       sret = SDK_RET_OK;

    sret = lif_manager()->reserve_id(SERVICE_LIF_START, (SERVICE_LIF_END - SERVICE_LIF_START));
    if (sret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to reserve service LIF");
        return HAL_RET_NO_RESOURCE;
    }

#if 0
    // Reserve Service LIFs
    if(lif_manager()->LIFRangeAlloc(SERVICE_LIF_START, (SERVICE_LIF_END - SERVICE_LIF_START))
            <= 0)
    {
        HAL_TRACE_ERR("Failed to reserve service LIF");
        return HAL_RET_NO_RESOURCE;
    }
#endif

    // Initialize meta
    ret = proxy_meta_init();
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize meta for proxy services");
        return ret;
    }

    // Enable system services
    ret = hal_proxy_system_svc_init();
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize proxy system services");
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a Proxy update request
//------------------------------------------------------------------------------
hal_ret_t
proxy_update (ProxySpec& spec, ProxyResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a Proxy get request
//------------------------------------------------------------------------------
hal_ret_t
proxy_get (ProxyGetRequest& req, ProxyGetResponse *rsp)
{
#if 0
    hal_ret_t              ret = HAL_RET_OK;
    proxy_t                rproxy;
    proxy_t*               proxy;
    pd::pd_proxy_args_t    pd_proxy_args;

    if (!req.has_meta()) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    auto kh = req.key_or_handle();

    if (kh.key_or_handle_case() == proxy::ProxyKeyHandle::kProxybId) {
        proxy = find_proxy_by_id(kh.proxy_id());
    } else if (kh.key_or_handle_case() == proxy::ProxyKeyHandle::kProxyHandle) {
        proxy = find_proxy_by_handle(kh.proxy_handle());
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (proxy == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_TCP_CB_NOT_FOUND;
    }

    proxy_init(&rproxy);
    rproxy.cb_id = proxy->cb_id;
    pd::pd_proxy_args_init(&pd_proxy_args);
    pd_proxy_args.proxy = &rproxy;

    ret = pd::pd_proxy_get(&pd_proxy_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXY: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this TCP CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_proxy_id(rproxy.cb_id);
    rsp->mutable_spec()->set_rcv_nxt(rproxy.rcv_nxt);

    // fill operational state of this TCP CB
    rsp->mutable_status()->set_proxy_handle(proxy->hal_handle);
#endif

    // fill stats of this TCP CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

hal_ret_t
validate_proxy_flow_config_request(proxy::ProxyFlowConfigRequest& req,
                                   proxy::ProxyResponse *rsp)
{
    vrf_t*       vrf = NULL;
    vrf_id_t     tid = 0;

    if(!req.has_spec() ||
       req.spec().proxy_type() == types::PROXY_TYPE_NONE) {
       rsp->set_api_status(types::API_STATUS_PROXY_TYPE_INVALID);
        HAL_TRACE_ERR("no proxy_type found");
       return HAL_RET_INVALID_ARG;
    }

    if(!req.spec().has_vrf_key_handle()) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        HAL_TRACE_ERR("no vrf id found");
        return HAL_RET_INVALID_ARG;
    }

    if (req.spec().vrf_key_handle().vrf_id() == HAL_VRF_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        HAL_TRACE_ERR("vrf {}", req.spec().vrf_key_handle().vrf_id());
        return HAL_RET_INVALID_ARG;
    }

    tid = req.spec().vrf_key_handle().vrf_id();
    vrf = vrf_lookup_by_id(tid);
    if(vrf == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        HAL_TRACE_ERR("vrf {} not found", tid);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

hal_ret_t
proxy_flow_handle_tls_config(types::ProxyType proxy_type,
                             const flow_key_t &flow_key,
                             const proxy::TlsProxyFlowConfig &tls_flow_config,
                             proxy::ProxyResponse *rsp)
{
    proxy_flow_info_t   *pfi = NULL;
    proxy_t*            proxy = NULL;
    flow_key_t          key = flow_key;

    proxy = find_proxy_by_type(proxy_type);
    if(proxy == NULL) {
        if(rsp)
            rsp->set_api_status(types::API_STATUS_PROXY_NOT_ENABLED);
        HAL_TRACE_ERR("proxy {} not found", proxy_type);
        return HAL_RET_PROXY_NOT_FOUND;
    }

    key.lkpvrf=0;
    pfi = find_proxy_flow_info(proxy, &key);
    if(!pfi) {
        HAL_TRACE_DEBUG("Failed to find the proxy flow info for the  flow");
        if(rsp)
            rsp->set_api_status(types::API_STATUS_FLOW_INFO_INVALID);
        return HAL_RET_FLOW_NOT_FOUND;
    }

    pfi->u.tlsproxy.cert_id = tls_flow_config.cert_id();
    HAL_TRACE_DEBUG("Received ciphers: {}", tls_flow_config.ciphers());
    if(tls_flow_config.ciphers().length() > 0) {
        pfi->u.tlsproxy.ciphers = tls_flow_config.ciphers();
    }
    pfi->u.tlsproxy.key_type = tls_flow_config.key_type();
    switch(tls_flow_config.key_type()) {
    case types::CRYPTO_ASYM_KEY_TYPE_ECDSA:
        pfi->u.tlsproxy.u.ecdsa_keys.sign_key_id =
            tls_flow_config.ecdsa_keys().sign_key_idx();
        break;
    case types::CRYPTO_ASYM_KEY_TYPE_RSA:
        pfi->u.tlsproxy.u.rsa_keys.sign_key_id =
            tls_flow_config.rsa_keys().sign_key_idx();
        pfi->u.tlsproxy.u.rsa_keys.decrypt_key_id =
            tls_flow_config.rsa_keys().decrypt_key_idx();
        break;
    default:
        HAL_TRACE_ERR("Unknown key type: {}", tls_flow_config.key_type());
    }
    pfi->u.tlsproxy.is_valid = true;

    HAL_TRACE_DEBUG("TLS proxy config for qid: {}, cert: {}",
                    pfi->qid1,
                    pfi->u.tlsproxy.cert_id);

    return HAL_RET_OK;
}

hal_ret_t
proxy_flow_enable(types::ProxyType proxy_type,
                  const flow_key_t &flow_key,
                  bool alloc_qid,
                  proxy::ProxyResponse *rsp,
                  const proxy::IpsecFlowConfig *ipsec_flow_config)
{
    proxy_flow_info_t   *pfi = NULL;
    proxy_t*            proxy = NULL;
    indexer::status     rs;
    flow_key_t          key = flow_key;

    HAL_TRACE_DEBUG("proxy: enable proxy for the flow: {}", flow_key);

    proxy = find_proxy_by_type(proxy_type);
    if(proxy == NULL) {
        if(rsp)
            rsp->set_api_status(types::API_STATUS_PROXY_NOT_ENABLED);
        HAL_TRACE_ERR("proxy {} not found", proxy_type);
        return HAL_RET_PROXY_NOT_FOUND;
    }

    key.lkpvrf = 0;
    pfi = find_proxy_flow_info(proxy, &key);
    if(pfi) {
        HAL_TRACE_DEBUG("Proxy already enabled for the flow");
        if(rsp)
            rsp->set_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pfi = proxy_flow_info_alloc_init();
    if(!pfi) {
        if(rsp)
            rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    pfi->flow_key = flow_key;
    pfi->proxy = proxy;

    // Allocate QID for this flow
    if(alloc_qid) {
        rs = proxy->qid_idxr_->alloc((uint32_t *)&(pfi->qid1));
        if(rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Error in qid1 allocation, err: {}", rs);
            if(rsp)
                rsp->set_api_status(types::API_STATUS_OUT_OF_RESOURCE);
            return HAL_RET_NO_RESOURCE;
        }

        rs = proxy->qid_idxr_->alloc((uint32_t *)&(pfi->qid2));
        if(rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Error in qid2 allocation, err: {}", rs);
            if(rsp)
                rsp->set_api_status(types::API_STATUS_OUT_OF_RESOURCE);
            return HAL_RET_NO_RESOURCE;
        }

        HAL_TRACE_DEBUG("Received qid1: {}, qid2: {}", pfi->qid1, pfi->qid2);
    }

    if(proxy->type == types::PROXY_TYPE_IPSEC) {
        HAL_TRACE_DEBUG("ipsec proxy flow configured");
        if(ipsec_flow_config->encrypt()) {
            HAL_TRACE_DEBUG("ipsec proxy host flow configured");
            extract_flow_key_from_spec(flow_key.svrf_id,
                                       &pfi->u.ipsec.u.host_flow.esp_flow_key,
                                       ipsec_flow_config->esp_flow_key());
        }
    }
    add_proxy_flow_info_to_db(pfi);
    return HAL_RET_OK;
}

hal_ret_t
proxy_flow_config(proxy::ProxyFlowConfigRequest& req,
                  proxy::ProxyResponse *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    vrf_id_t            tid = 0;
    flow_key_t          flow_key = {0};
    proxy_flow_info_t*  pfi = NULL;
    proxy_t*            proxy = NULL;

    ret = validate_proxy_flow_config_request(req, rsp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("proxy: flow config request validation failed: {}", ret);
        return ret;
    }

    tid = req.spec().vrf_key_handle().vrf_id();
    extract_flow_key_from_spec(tid, &flow_key, req.flow_key());

    // ignore direction for the flow.
    flow_key.dir = 0;

    if(req.proxy_en()) {
        ret = proxy_flow_enable(req.spec().proxy_type(),
                                flow_key,
                                req.alloc_qid(),
                                rsp,
                                &(req.ipsec_config()));
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to enable proxy service for the flow");
            return ret;
        }

        if(req.has_tls_proxy_config()) {
            HAL_TRACE_DEBUG("Found TLS proxy config");
            ret = proxy_flow_handle_tls_config(req.spec().proxy_type(),
                                               flow_key,
                                               req.tls_proxy_config(),
                                               rsp);
            if(ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to update tls config for the flow");
                return ret;
            }
        }
    } else {
        HAL_TRACE_DEBUG("proxy: disable proxy for the flow");
        proxy = find_proxy_by_type(req.spec().proxy_type());
        if(proxy == NULL) {
            if(rsp)
                rsp->set_api_status(types::API_STATUS_PROXY_NOT_ENABLED);
            HAL_TRACE_ERR("proxy {} not found", req.spec().proxy_type());
            return HAL_RET_PROXY_NOT_FOUND;
        }
        pfi = (proxy_flow_info_t *)proxy->flow_ht_->remove(&flow_key);
        proxy_flow_info_free(pfi);
    }
    rsp->set_api_status(types::API_STATUS_OK);
    return ret;
}

hal_ret_t
validate_proxy_get_flow_info_request(proxy::ProxyGetFlowInfoRequest& req,
                                     proxy::ProxyGetFlowInfoResponse *rsp)
{
    vrf_t*       vrf = NULL;
    vrf_id_t     tid = 0;

    if (!req.has_spec() ||
       req.spec().proxy_type() == types::PROXY_TYPE_NONE) {
       rsp->set_api_status(types::API_STATUS_PROXY_TYPE_INVALID);
       return HAL_RET_INVALID_ARG;
    }

    if (req.spec().vrf_key_handle().vrf_id() == HAL_VRF_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    tid = req.spec().vrf_key_handle().vrf_id();
    vrf = vrf_lookup_by_id(tid);
    if (vrf == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        HAL_TRACE_ERR("vrf {} not found", tid);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

proxy_flow_info_t*
proxy_get_flow_info(types::ProxyType proxy_type,
                    flow_key_t flow_key)
{
    proxy_t*            proxy = NULL;

    if(proxy_type == types::PROXY_TYPE_NONE) {
        return NULL;
    }

    proxy = find_proxy_by_type(proxy_type);
    if (proxy == NULL) {
        //HAL_TRACE_DEBUG("proxy {} not found", proxy_type);
        return NULL;
    }

    flow_key.lkpvrf = 0;
    return find_proxy_flow_info(proxy, &flow_key);
}

hal_ret_t
proxy_get_flow_info(proxy::ProxyGetFlowInfoRequest& req,
                    proxy::ProxyGetFlowInfoResponse* rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    vrf_id_t         tid = 0;
    flow_key_t          flow_key = {0};
    proxy_t*            proxy = NULL;
    proxy_flow_info_t*  pfi = NULL;

    ret = validate_proxy_get_flow_info_request(req, rsp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("get flow info requestvalidation failed: {}", ret);
        return ret;
    }

    tid = req.spec().vrf_key_handle().vrf_id();
    extract_flow_key_from_spec(tid, &flow_key, req.flow_key());

    pfi = proxy_get_flow_info(req.spec().proxy_type(), flow_key);
    if(!pfi) {
        HAL_TRACE_ERR("flow info not found for the flow {}", flow_key);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_PROXY_NOT_FOUND;
    }

    proxy = pfi->proxy;

    rsp->set_proxy_type(proxy->type);
    *(rsp->mutable_flow_key()) = req.flow_key();;
    rsp->set_lif_id(proxy->meta->lif_info[0].lif_id);
    rsp->set_qtype(proxy->meta->lif_info[0].qtype_info[0].qtype_val);
    rsp->set_qid1(pfi->qid1);
    rsp->set_qid2(pfi->qid2);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;;
}

bool
is_proxy_enabled_for_flow(types::ProxyType proxy_type,
                          flow_key_t flow_key)
{
    return (NULL != proxy_get_flow_info(proxy_type, flow_key));
}

/*
 * Process request to set global config for proxy services.
 */
hal_ret_t
proxy_globalcfg_set(proxy::ProxyGlobalCfgRequest& req,
		    proxy::ProxyGlobalCfgResponseMsg *rsp)
{

    /*
     * Validate the request message. Currently only TLS proxy
     * feature supported.
     */
    if (req.proxy_type() != types::PROXY_TYPE_TLS) {
        rsp->add_api_status(types::API_STATUS_PROXY_TYPE_INVALID);
        HAL_TRACE_ERR("Proxy global config, invalid proxy type: {}",
		      req.proxy_type());
        return HAL_RET_INVALID_ARG;
    }

    hal::tls::proxy_tls_bypass_mode = req.bypass_mode();
    HAL_TRACE_DEBUG("Proxy TLS: setting bypass mode {:d}",
		    hal::tls::proxy_tls_bypass_mode);

    // prepare the response
    rsp->add_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}    // namespace hal
