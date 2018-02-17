// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/include/endpoint_api.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/src/qos.hpp"
#include "sdk/timestamp.hpp"
#include "nic/include/fte.hpp"

using telemetry::MirrorSessionId;
using session::FlowInfo;
using session::FlowKeyTcpUdpInfo;
using session::FlowKeyICMPInfo;
using session::FlowData;
using session::ConnTrackInfo;

namespace hal {

thread_local void *g_session_timer;

#define SESSION_SW_DEFAULT_TIMEOUT (3600 * TIME_NSECS_PER_SEC) 

void *
session_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->config.session_id);
}

uint32_t
session_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(session_id_t)) % ht_size;
}

bool
session_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(session_id_t *)key1 == *(session_id_t *)key2) {
        return true;
    }
    return false;
}

void *
session_get_handle_key_func(void *entry)
{
    HAL_TRACE_DEBUG("Entry HAL Handle:{}", ((session_t*)entry)->hal_handle);

    HAL_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->hal_handle);
}

uint32_t
session_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
session_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

void *
session_get_iflow_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->iflow->config.key);
}

uint32_t
session_compute_iflow_hash_func (void *key, uint32_t ht_size)
{
    return (sdk::lib::hash_algo::fnv_hash(key, sizeof(flow_key_t)) % ht_size);
}

bool
session_compare_iflow_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(flow_key_t))) {
        return true;
    }
    return false;
}

void *
session_get_rflow_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->rflow->config.key);
}

uint32_t
session_compute_rflow_hash_func (void *key, uint32_t ht_size)
{
    return (sdk::lib::hash_algo::fnv_hash(key, sizeof(flow_key_t)) % ht_size);
}

bool
session_compare_rflow_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(flow_key_t))) {
        return true;
    }
    return false;
}

void *
flow_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((flow_t *)entry)->config.key);
}

uint32_t
flow_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(flow_key_t)) % ht_size;
}

bool
flow_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(flow_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// thread safe helper to stringify flow_key_t
//------------------------------------------------------------------------------
const char *
flowkey2str (const flow_key_t& key)
{
    static thread_local char       key_str[4][400];
    static thread_local uint8_t    key_str_next = 0;
    char                           *buf;

    buf = key_str[key_str_next++ & 0x3];
    fmt::ArrayWriter out(buf, 400);

    out.write("{{dir={}, ", key.dir);

    switch (key.flow_type) {
    case FLOW_TYPE_L2:
        out.write("vrf={}, l2seg={}, smac={}, dmac={} etype={}", key.vrf_id, key.l2seg_id, macaddr2str(key.smac),
                  macaddr2str(key.dmac), key.ether_type);
        break;
    case FLOW_TYPE_V4:
    case FLOW_TYPE_V6:
        out.write("vrf={}, ", key.vrf_id);
        if (key.flow_type == FLOW_TYPE_V4) {
            out.write("sip={}, dip={}, ", ipv4addr2str(key.sip.v4_addr), ipv4addr2str(key.dip.v4_addr));
        } else {
            out.write("sip={}, dip={}, ", key.sip.v6_addr, key.dip.v6_addr);
        }

        switch (key.proto) {
        case types::IPPROTO_ICMP:
        case types::IPPROTO_ICMPV6:
            out.write("proto=icmp, type={}, code={}, id={}",
                      key.icmp_type, key.icmp_code, key.icmp_id);
            break;
        case types::IPPROTO_ESP:
            out.write("spi={}", key.spi);
            break;
        case types::IPPROTO_TCP:
            out.write("proto=tcp, sport={}, dport={}", key.sport, key.dport);
            break;
        case types::IPPROTO_UDP:
            out.write("proto=udp, sport={}, dport={}", key.sport, key.dport);
            break;
        default:
            out.write("proto={}", key.proto);
        }
        break;
    default:
        out.write("flow-type=unknown(%d)", key.flow_type);
    }

    out.write("}}");
    
    buf[out.size()] = '\0';

    return buf;
}

// extract mirror sessions specified in spec into ingress and egress bitmaps
//------------------------------------------------------------------------------
static hal_ret_t
extract_mirror_sessions (const FlowSpec& spec, uint8_t *ingress, uint8_t *egress)
{
    int i;
    *ingress = 0;
    *egress = 0;
    FlowInfo flinfo = spec.flow_data().flow_info();
    for (i = 0; i < flinfo.ing_mirror_sessions_size(); ++i) {
        uint32_t id = flinfo.ing_mirror_sessions(i).session_id();
        if (id > 7) {
            return HAL_RET_INVALID_ARG;
        }
        *ingress = *ingress | (1 << id);
        HAL_TRACE_DEBUG("  Adding ingress session {}", id);
    }
    for (i = 0; i < flinfo.egr_mirror_sessions_size(); ++i) {
        uint32_t id = flinfo.egr_mirror_sessions(i).session_id();
        if (id > 7) {
            return HAL_RET_INVALID_ARG;
        }
        *egress = *egress | (1 << id);
        HAL_TRACE_DEBUG("  Adding egress session {}", id);
    }
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// release all the resources and allocated ids etc. associated with a session
// TODO: if session is added to DBs, remove from them as well !!
//------------------------------------------------------------------------------
hal_ret_t
session_cleanup (session_t *session)
{
    if (session->iflow) {
        hal::delay_delete_to_slab(HAL_SLAB_FLOW, session->iflow);
    }
    if (session->rflow) {
        hal::delay_delete_to_slab(HAL_SLAB_FLOW, session->rflow);
    }
    hal::delay_delete_to_slab(HAL_SLAB_SESSION, session);

    return HAL_RET_OK;
}

hal_ret_t
extract_flow_key_from_spec(vrf_id_t tid,
                           flow_key_t *key,
                           const FlowKey& flow_spec_key)
{
    key->vrf_id = tid;

    if (flow_spec_key.has_l2_key()) {
        key->flow_type = hal::FLOW_TYPE_L2;
        key->l2seg_id = flow_spec_key.l2_key().l2_segment_id();
        key->ether_type = flow_spec_key.l2_key().ether_type();
        MAC_UINT64_TO_ADDR(key->smac, flow_spec_key.l2_key().smac());
        MAC_UINT64_TO_ADDR(key->dmac, flow_spec_key.l2_key().dmac());
    } else if (flow_spec_key.has_v4_key()) {
        key->flow_type = hal::FLOW_TYPE_V4;
            key->sip.v4_addr = flow_spec_key.v4_key().sip();
        key->dip.v4_addr = flow_spec_key.v4_key().dip();
        key->proto = flow_spec_key.v4_key().ip_proto();
        if ((key->proto == IP_PROTO_TCP) ||
            (key->proto == IP_PROTO_UDP)) {
            key->sport = flow_spec_key.v4_key().tcp_udp().sport();
            key->dport = flow_spec_key.v4_key().tcp_udp().dport();
        } else if (key->proto == IP_PROTO_ICMP) {
            key->icmp_type = flow_spec_key.v4_key().icmp().type();
            key->icmp_code = flow_spec_key.v4_key().icmp().code();
            if ((key->icmp_type == 0) || (key->icmp_type == 8)) {
                /* ICMP id is valid only for echo req & rep */
                key->icmp_id = flow_spec_key.v4_key().icmp().id();
            } else {
                key->icmp_id = 0;
            }
        } else if (key->proto == IPPROTO_ESP) {
            key->spi = flow_spec_key.v4_key().esp().spi();
        } else {
            key->sport = key->dport = 0;
        }
    } else if (flow_spec_key.has_v6_key()) {
        key->flow_type = hal::FLOW_TYPE_V6;
        memcpy(key->sip.v6_addr.addr8,
               flow_spec_key.v6_key().sip().v6_addr().c_str(),
               IP6_ADDR8_LEN);
        memcpy(key->dip.v6_addr.addr8,
               flow_spec_key.v6_key().dip().v6_addr().c_str(),
               IP6_ADDR8_LEN);
        key->proto = flow_spec_key.v6_key().ip_proto();
        if ((key->proto == IP_PROTO_TCP) ||
            (key->proto == IP_PROTO_UDP)) {
            key->sport = flow_spec_key.v6_key().tcp_udp().sport();
            key->dport = flow_spec_key.v6_key().tcp_udp().dport();;
        } else if (key->proto == IP_PROTO_ICMPV6) {
            key->icmp_type = flow_spec_key.v6_key().icmp().type();
            key->icmp_code = flow_spec_key.v6_key().icmp().code();
            key->icmp_id = flow_spec_key.v6_key().icmp().id();
            // only echo request and reply
            if(key->icmp_type < 128 || key->icmp_type > 136) {
                HAL_TRACE_DEBUG("fte: invalid icmp type {}", key->icmp_type);
                return HAL_RET_INVALID_ARG;
            }
        } else if (key->proto == IPPROTO_ESP) {
            key->spi = flow_spec_key.v6_key().esp().spi();
        } else {
            key->sport = key->dport = 0;
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// given a flow key, get the source and destination endpoint records
//------------------------------------------------------------------------------
hal_ret_t
ep_get_from_flow_key (const flow_key_t* key, ep_t **sep, ep_t **dep)
{
    *sep = *dep = NULL;

    switch  (key->flow_type) {
    case FLOW_TYPE_L2:
        *sep = find_ep_by_l2_key(key->l2seg_id, key->smac);
        *dep = find_ep_by_l2_key(key->l2seg_id, key->dmac);
        break;

    case FLOW_TYPE_V4:
    case FLOW_TYPE_V6:
        ep_l3_key_t l3key;
        l3key.vrf_id = key->vrf_id;
        l3key.ip_addr.af = key->flow_type == FLOW_TYPE_V4 ? IP_AF_IPV4 : IP_AF_IPV6;

        l3key.ip_addr.addr = key->sip;
        *sep = find_ep_by_l3_key(&l3key);

        l3key.ip_addr.addr = key->dip;
        *dep = find_ep_by_l3_key(&l3key);
        break;
    }

    if (*sep == NULL || *dep == NULL) {
        return HAL_RET_EP_NOT_FOUND;
    }

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// insert this session in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_session_to_db (vrf_t *vrf, l2seg_t *l2seg_s, l2seg_t *l2seg_d,
                   ep_t *sep, ep_t *dep, if_t *sif, if_t *dif,
                   session_t *session)
{
    HAL_TRACE_DEBUG("Entering ADD session to DB:{}", session->hal_handle);


    session->session_id_ht_ctxt.reset();
    g_hal_state->session_id_ht()->insert(session,
                                         &session->session_id_ht_ctxt);
    HAL_TRACE_DEBUG("Session: {:p} session id ht ctxt: {:p}", (void *)session, 
                    (void *)session->session_id_ht_ctxt.entry);

    session->hal_handle_ht_ctxt.reset();
    g_hal_state->session_hal_handle_ht()->insert(session,
                                                 &session->hal_handle_ht_ctxt);

    session->hal_iflow_ht_ctxt.reset();
    g_hal_state->session_hal_iflow_ht()->insert(session,
                                                &session->hal_iflow_ht_ctxt);
   
    if (session->rflow) {
        session->hal_rflow_ht_ctxt.reset();
        g_hal_state->session_hal_rflow_ht()->insert_with_key(
                                     (void *)std::addressof(session->rflow->config.key),
                                     session, &session->hal_rflow_ht_ctxt);
    }

    if (sep) {
        ep_add_session(sep, session);
    }
    if (dep) {
        ep_add_session(dep, session);
    }

    sdk::lib::dllist_reset(&session->sif_session_lentry);
    if (sif) {
        HAL_SPINLOCK_LOCK(&sif->slock);
        // sdk::lib::dllist_add(&sif->session_list_head, &session->sif_session_lentry);
        HAL_SPINLOCK_UNLOCK(&sif->slock);
    }

    sdk::lib::dllist_reset(&session->dif_session_lentry);
    if (dif && sif != dif) {
        HAL_SPINLOCK_LOCK(&dif->slock);
        // sdk::lib::dllist_add(&dif->session_list_head,
        //                  &session->dif_session_lentry);
        HAL_SPINLOCK_UNLOCK(&dif->slock);
    }

#if 0
    // session list is changed to block list
    sdk::lib::dllist_reset(&session->vrf_session_lentry);
    if (vrf) {
        HAL_SPINLOCK_LOCK(&vrf->slock);
        sdk::lib::dllist_add(&vrf->session_list_head,
                          &session->vrf_session_lentry);
        HAL_SPINLOCK_UNLOCK(&vrf->slock);
    }
#endif

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// remove this session from all meta data structures
//------------------------------------------------------------------------------
static inline void
del_session_from_db (ep_t *sep, ep_t *dep, session_t *session)
{
    HAL_TRACE_DEBUG("Entering DEL session from DB:{}", session->hal_handle);

    // All the sessions are supposed to have session id
    // Need to remove this check once the session id allocation
    // happens for flow-miss
    if (session->config.session_id)
        g_hal_state->session_id_ht()->remove_entry(session,
                                         &session->session_id_ht_ctxt);

    g_hal_state->session_hal_handle_ht()->remove_entry(session,
                                                 &session->hal_handle_ht_ctxt);

    g_hal_state->session_hal_iflow_ht()->remove_entry(session,
                                                &session->hal_iflow_ht_ctxt);

    if (session->rflow) {
        g_hal_state->session_hal_rflow_ht()->remove_entry(session, 
                                                    &session->hal_rflow_ht_ctxt);
    }

    if (sep) {
        ep_del_session(sep, session);
    }
    if (dep) {
        ep_del_session(dep, session);
    }
}


static void
flow_tcp_to_flow_tcp_spec(flow_t *flow, FlowKeyTcpUdpInfo *tcp_udp)
{
    tcp_udp->set_sport(flow->config.key.sport);
    tcp_udp->set_dport(flow->config.key.dport);
}

static void
flow_icmp_to_flow_icmp_spec(flow_t *flow, FlowKeyICMPInfo *icmp)
{
    icmp->set_type(flow->config.key.icmp_type);
    icmp->set_code(flow->config.key.icmp_code);
    icmp->set_id(flow->config.key.icmp_id);
}

static void
flow_data_to_flow_data_spec(flow_t *flow, FlowData *flow_data)
{
    FlowInfo *flow_info = flow_data->mutable_flow_info();
    flow_info->set_flow_action((session::FlowAction)(flow->config.action));
    flow_info->set_nat_type((session::NatType)(flow->config.nat_type));
    ip_addr_to_spec(flow_info->mutable_nat_sip(), &flow->config.nat_sip);
    ip_addr_to_spec(flow_info->mutable_nat_dip(), &flow->config.nat_dip);
    flow_info->set_nat_sport(flow->config.nat_sport);
    flow_info->set_nat_dport(flow->config.nat_dport);

    /* TODO: Connection Tracking Info framework has to be done still.
    conn_track_to_conn_track_spec(flow,
            flow_data->mutable_conn_track_info());
            */
}

static void
flow_to_flow_spec(flow_t *flow, FlowSpec *spec)
{
    if (flow->config.key.flow_type == FLOW_TYPE_L2) {
        FlowKeyL2 *l2_key = spec->mutable_flow_key()->mutable_l2_key();
        l2_key->set_smac(MAC_TO_UINT64(flow->config.key.smac));
        l2_key->set_dmac(MAC_TO_UINT64(flow->config.key.dmac));
        l2_key->set_ether_type(flow->config.key.ether_type);
        l2_key->set_l2_segment_id(flow->config.key.l2seg_id);
    } else if (flow->config.key.flow_type == FLOW_TYPE_V4) {
        FlowKeyV4 *v4_key = spec->mutable_flow_key()->mutable_v4_key();
        v4_key->set_sip(flow->config.key.sip.v4_addr);
        v4_key->set_dip(flow->config.key.dip.v4_addr);
        v4_key->set_ip_proto(types::IPProtocol(flow->config.key.proto));
        if ((flow->config.key.proto == types::IPPROTO_TCP) ||
            (flow->config.key.proto == types::IPPROTO_UDP)) {
            flow_tcp_to_flow_tcp_spec(flow, v4_key->mutable_tcp_udp());
        } else if (flow->config.key.proto == types::IPPROTO_ICMP) {
            flow_icmp_to_flow_icmp_spec(flow, v4_key->mutable_icmp());
        }

    } else if (flow->config.key.flow_type == FLOW_TYPE_V6) {
        FlowKeyV6 *v6_key = spec->mutable_flow_key()->mutable_v6_key();
        v6_key->mutable_sip()->set_v6_addr(&flow->config.key.sip.v6_addr, IP6_ADDR8_LEN);
        v6_key->mutable_dip()->set_v6_addr(&flow->config.key.dip.v6_addr, IP6_ADDR8_LEN);
        v6_key->set_ip_proto(types::IPProtocol(flow->config.key.proto));
        if ((flow->config.key.proto == types::IPPROTO_TCP) ||
            (flow->config.key.proto == types::IPPROTO_UDP)) {
            flow_tcp_to_flow_tcp_spec(flow, v6_key->mutable_tcp_udp());
        } else if (flow->config.key.proto == types::IPPROTO_ICMPV6) {
            flow_icmp_to_flow_icmp_spec(flow, v6_key->mutable_icmp());
        }
    }

    flow_data_to_flow_data_spec(flow, spec->mutable_flow_data());

}

static void
session_to_session_get_response (session_t *session, SessionGetResponse *response)
{
    vrf_t   *vrf = vrf_lookup_by_handle(session->vrf_handle);

    response->mutable_spec()->mutable_meta()->set_vrf_id(vrf->vrf_id);
    response->mutable_spec()->set_session_id(session->config.session_id);
    response->mutable_spec()->set_conn_track_en(session->config.conn_track_en);
    response->mutable_spec()->set_tcp_ts_option(session->config.tcp_ts_option);

    flow_to_flow_spec(session->iflow, response->mutable_spec()->mutable_initiator_flow());
    flow_to_flow_spec(session->rflow, response->mutable_spec()->mutable_responder_flow());
}


hal_ret_t
session_get (SessionGetRequest& req, SessionGetResponse *response)
{
    session_t              *session;

    if (!req.has_meta() ||
        req.meta().vrf_id() == HAL_VRF_ID_INVALID) {
        response->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    session = find_session_by_handle(req.session_handle());

    if (session == NULL) {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_SESSION_NOT_FOUND;
    }

    session_to_session_get_response(session, response);
    response->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// Flow Create FTE
//-----------------------------------------------------------------------------
static flow_t *
flow_create_fte(const flow_cfg_t *cfg,
                const flow_cfg_t *cfg_assoc,
                const flow_pgm_attrs_t *attrs,
                const flow_pgm_attrs_t *attrs_assoc,
                session_t *session)
{
    flow_t      *assoc_flow = NULL;

    flow_t *flow = (flow_t *)g_hal_state->flow_slab()->alloc();
    if (!flow) {
        return NULL;
    }

    *flow = {};
    HAL_SPINLOCK_INIT(&flow->slock, PTHREAD_PROCESS_PRIVATE);
    flow->flow_key_ht_ctxt.reset();

    if (cfg){
        flow->config = *cfg;
    }

    if (attrs) {
        flow->pgm_attrs = *attrs;
    }

    flow->session = session;

    // Check if we have to create associated flow
    if (cfg_assoc) {
        assoc_flow = (flow_t *)g_hal_state->flow_slab()->alloc();
        if (!assoc_flow) {
            return NULL;
        }
        *assoc_flow = {};
        HAL_SPINLOCK_INIT(&assoc_flow->slock, PTHREAD_PROCESS_PRIVATE);
        assoc_flow->flow_key_ht_ctxt.reset();
        assoc_flow->config = *cfg_assoc;
        if (attrs_assoc) {
            assoc_flow->pgm_attrs = *attrs_assoc;            
        }
        assoc_flow->session = session;

        // If its an aug flow, goto assoc flow to get all params
        assoc_flow->is_aug_flow = true;

        // Link 
        flow->assoc_flow = assoc_flow;
        assoc_flow->assoc_flow = flow;
    }

    return flow;
}

hal_ret_t
session_create(const session_args_t *args, hal_handle_t *session_handle, 
               session_t **session_p)
{
    hal_ret_t ret;
    nwsec_profile_t         *nwsec_prof;
    pd::pd_session_create_args_t    pd_session_args;
    session_t               *session;

    HAL_ASSERT(args->vrf && args->iflow && args->iflow_attrs);

    // allocate a session
    session = (session_t *)g_hal_state->session_slab()->alloc();
    if (session == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    *session = {};
    dllist_reset(&session->feature_list_head);
    session->config = *args->session;
    session->vrf_handle = args->vrf->hal_handle;
    session->tcp_close_timer = NULL;

    // fetch the security profile, if any
    if (args->vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        nwsec_prof =
            find_nwsec_profile_by_handle(args->vrf->nwsec_profile_handle);
    } else {
        nwsec_prof = NULL;
    }

    // Handle the spec info not handled in the FTE
    // TODO(goli) all these should go to appropriate fte features
    if (args->spec) {
        uint8_t  ingress, egress;
        ret = extract_mirror_sessions(args->spec->initiator_flow(), &ingress, &egress);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("session create failure extracting mirror sessions: {}", __FUNCTION__);
            return ret;
        }
        args->iflow[0]->ing_mirror_session |= ingress;
        args->iflow[0]->eg_mirror_session |= egress;

        if(args->rflow[0] && args->spec->has_responder_flow()) {
            ret = extract_mirror_sessions(args->spec->responder_flow(), &ingress, &egress);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("session create failure extracting mirror sessions: {}", __FUNCTION__);
                return ret;
            }
            args->rflow[0]->ing_mirror_session |= ingress;
            args->rflow[0]->eg_mirror_session |= egress;
        }
    }


    // create flows
    session->iflow = flow_create_fte(args->iflow[0], args->iflow[1],
                                     args->iflow_attrs[0], args->iflow_attrs[1],
                                     session);
    if (session->iflow == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }


    if (args->valid_rflow) {
        session->rflow = flow_create_fte(args->rflow[0], args->rflow[1],
                                         args->rflow_attrs[0], args->rflow_attrs[1],
                                         session);
        if (session->rflow == NULL) {
            ret = HAL_RET_OOM;
            goto end;
        }

        session->iflow->reverse_flow = session->rflow;
        session->rflow->reverse_flow = session->iflow;
    }
    session->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming, if any
    pd::pd_session_create_args_init(&pd_session_args);
    pd_session_args.vrf = args->vrf;
    pd_session_args.nwsec_prof = nwsec_prof;
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;
    pd_session_args.rsp = args->rsp;
    pd_session_args.update_iflow = true;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_CREATE, (void *)&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session create failure, err : {}", ret);
        goto end;
    }

    // add this session to our db
    add_session_to_db(args->vrf, args->sl2seg, args->dl2seg,
                      args->sep, args->dep, args->sif, args->dif, session);
    HAL_ASSERT(ret == HAL_RET_OK);

    if (session_handle) {
        *session_handle = session->hal_handle;
    }

    if (session_p) {
        *session_p = session;
    }

 end:
    if (session && ret != HAL_RET_OK) {
        HAL_TRACE_ERR("session create failure, err={}", ret);
        session_cleanup(session);
    }

    return ret;
}

session_t *
session_lookup(flow_key_t key, flow_role_t *role)
{
    session_t *session = NULL;

    // Should we look at iflow first ?
    session = (session_t *)g_hal_state->session_hal_rflow_ht()->lookup(std::addressof(key));
    if (role)
        *role = FLOW_ROLE_RESPONDER;
    if (session == NULL) {
        session = (session_t *)g_hal_state->session_hal_iflow_ht()->lookup(std::addressof(key));
        if (role)
            *role = FLOW_ROLE_INITIATOR;
    }

    return session;
}

hal_ret_t
session_update(const session_args_t *args, session_t *session) 
{
    hal_ret_t                ret;
    pd::pd_session_update_args_t    pd_session_args;

    if(args->iflow[0]) {
        session->iflow->config = *args->iflow[0];
        session->iflow->pgm_attrs = *args->iflow_attrs[0];
        if(session->iflow->assoc_flow && args->iflow[1]) {
            session->iflow->assoc_flow->config = *args->iflow[1];
            session->iflow->assoc_flow->pgm_attrs = *args->iflow_attrs[1];
        }
    }

    if(session->rflow && args->rflow[0]) {
        session->rflow->config = *args->rflow[0];
        session->rflow->pgm_attrs = *args->rflow_attrs[0];
        if(session->rflow->assoc_flow && args->rflow[1]) {
            session->rflow->assoc_flow->config = *args->rflow[1];
            session->rflow->assoc_flow->pgm_attrs = *args->rflow_attrs[1];
        }
    } else if (args->valid_rflow) {
        session->rflow = flow_create_fte(args->rflow[0], args->rflow[1],
                                         args->rflow_attrs[0], args->rflow_attrs[1],
                                         session);
        if (session->rflow == NULL) {
            return HAL_RET_OOM;
        }

        session->iflow->reverse_flow = session->rflow;
        session->rflow->reverse_flow = session->iflow;
    }
 
    // allocate all PD resources and finish programming, if any
    pd::pd_session_update_args_init(&pd_session_args);
    pd_session_args.vrf = args->vrf;
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;
    pd_session_args.rsp = args->rsp;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_UPDATE, (void *)&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session update failure, err : {}", ret);
    }
    
    return ret;
}

hal_ret_t
session_delete(const session_args_t *args, session_t *session)
{
    hal_ret_t                ret;
    pd::pd_session_delete_args_t    pd_session_args;

    // allocate all PD resources and finish programming, if any
    pd::pd_session_delete_args_init(&pd_session_args);
    pd_session_args.vrf =
        args ? args->vrf : vrf_lookup_by_handle(session->vrf_handle);
    pd_session_args.session = session;
    pd_session_args.session_state = args ? args->session_state : NULL;
    pd_session_args.rsp = args ? args->rsp : NULL;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_DELETE, (void *)&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session delete failure, err : {}", ret);
    }

    del_session_from_db(args->sep, args->dep, session);

    session_cleanup(session);

    return ret;
}

#define HAL_SESSION_AGE_SCAN_INTVL                   (5 * TIME_MSECS_PER_SEC)
#define HAL_SESSION_BUCKETS_TO_SCAN_PER_INTVL        4
#define HAL_TCP_CLOSE_WAIT_INTVL                     (10 * TIME_MSECS_PER_SEC)

//------------------------------------------------------------------------------
// determine aging timeout of a session based on its properties
// TODO: look at the nwsec profile
//------------------------------------------------------------------------------
static uint64_t
session_aging_timeout (session_t *session,
                       flow_t *iflow, flow_state_t *iflow_state,
                       flow_t *rflow, flow_state_t *rflow_state)
{
    uint64_t            timeout = SESSION_SW_DEFAULT_TIMEOUT;
    vrf_t              *vrf = NULL;
    nwsec_profile_t    *nwsec_prof = NULL;

    vrf = vrf_lookup_by_handle(session->vrf_handle);
    if (vrf != NULL) {
        nwsec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
        if (nwsec_prof != NULL) {
            timeout = (uint64_t)(nwsec_prof->session_idle_timeout * TIME_NSECS_PER_SEC);
        }
    }

    return timeout;
}

//------------------------------------------------------------------------------
// determine whether a given session should be aged or not
//------------------------------------------------------------------------------
bool
session_age_cb (void *entry, void *ctxt)
{
    pd::pd_conv_hw_clock_to_sw_clock_args_t    clock_args;
    hal_ret_t                                  ret;
    session_t                                 *session = (session_t *)entry;
    flow_t                                    *iflow, *rflow;
    flow_state_t                               iflow_state, rflow_state;
    uint64_t                                   ctime_ns = *(uint64_t *)ctxt;
    uint64_t                                   last_pkt_ts;
    uint64_t                                   session_timeout;
    pd::pd_flow_get_args_t                     args;
    SessionSpec                                spec;
    SessionResponse                            rsp; 

    if (session->tcp_close_timer != NULL) {
        HAL_TRACE_DEBUG("FIN/RST is being processed for session {} -- bailing aging",
                        session->config.session_id);
        return false;
    }

    // read the initiator flow record
    iflow = session->iflow;
    if (!iflow) {
        HAL_TRACE_ERR("session {} has no iflow, ignoring ...",
                      session->config.session_id);
        return false;
    }
    args.pd_session = session->pd;
    args.role = FLOW_ROLE_INITIATOR;
    args.flow_state = &iflow_state;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_FLOW_GET, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to fetch iflow record of session {}",
                       session->config.session_id);
        return false;
    }

    // read the responder flow record
    rflow = session->rflow;
    if (!rflow) {
        HAL_TRACE_ERR("session {} has no rflow, ignoring ...",
                      session->config.session_id);
    } else {
        args.pd_session = session->pd;
        args.role = FLOW_ROLE_RESPONDER;
        args.flow_state = &rflow_state;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_FLOW_GET, (void *)&args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to fetch rflow record of session {}",
                          session->config.session_id);
            return ret;
        }
    }

    // check if iflow has expired now
    session_timeout = session_aging_timeout(session, iflow, &iflow_state,
                                            rflow, rflow ? &rflow_state : NULL);

    // Convert hw clock to sw clock resolving any deltas
    clock_args.hw_tick = iflow_state.last_pkt_ts;
    clock_args.sw_ns = &last_pkt_ts;
    pd::hal_pd_call(pd::PD_FUNC_ID_CONV_HW_CLOCK_TO_SW_CLOCK, (void *)&clock_args);
    HAL_TRACE_DEBUG("Hw tick: {}", iflow_state.last_pkt_ts);
    HAL_TRACE_DEBUG("session_age_cb: last pkt ts: {} ctime_ns: {} session_timeout: {}", 
                    last_pkt_ts, ctime_ns, session_timeout);
    if ((ctime_ns - last_pkt_ts) < session_timeout) {
        // session hasn't aged yet, move on
        return false;
    } else  {
        // no activity detected on initiator flow for a while, check responder
        clock_args.hw_tick = rflow_state.last_pkt_ts;
        clock_args.sw_ns = &last_pkt_ts;
        pd::hal_pd_call(pd::PD_FUNC_ID_CONV_HW_CLOCK_TO_SW_CLOCK, (void *)&clock_args);
        if ((ctime_ns - last_pkt_ts) < session_timeout) {
            // responder flow seems to be active still
            return false;
        }

        // time to clean up the session
        ret = fte::session_delete(session);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delte aged session {}",
                          session->config.session_id);
            return false;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
// callback invoked by the HAL periodic thread for session aging
//------------------------------------------------------------------------------
void
session_age_walk_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    uint32_t      i, bucket = *((uint32_t *)(&ctxt));
    timespec_t    ctime;
    uint64_t      ctime_ns;

    // get current time
    clock_gettime(CLOCK_MONOTONIC, &ctime);
    sdk::timestamp_to_nsecs(&ctime, &ctime_ns);

    //HAL_TRACE_DEBUG("[{}:{}] timer id {}, bucket: {}",
    //                __FUNCTION__, __LINE__, timer_id,  bucket);
    for (i = 0; i < HAL_SESSION_BUCKETS_TO_SCAN_PER_INTVL; i++) {
        g_hal_state->session_hal_handle_ht()->walk_bucket_safe(bucket,
                                                     session_age_cb, &ctime_ns);
        bucket = (bucket + 1)%g_hal_state->session_hal_handle_ht()->num_buckets();
    }

    // store the bucket id to resume on next invocation
    hal::periodic::timer_update(timer, HAL_SESSION_AGE_SCAN_INTVL,
                                true, reinterpret_cast<void *>(bucket));
}

//------------------------------------------------------------------------------
// initialize the session management module
//------------------------------------------------------------------------------
hal_ret_t
session_init (void)
{
    // Disable aging when FTE is disabled
    if (getenv("DISABLE_AGING")) {
        return HAL_RET_OK;
    }

    // wait until the periodic thread is ready
    while (!hal::periodic::periodic_thread_is_running()) {
        pthread_yield();
    }
    g_session_timer =
        hal::periodic::timer_schedule(HAL_TIMER_ID_SESSION_AGEOUT,            // timer_id
                                      HAL_SESSION_AGE_SCAN_INTVL,
                                      (void *)0,    // ctxt
                                      session_age_walk_cb, true);
    if (!g_session_timer) {
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("session timer: {:p}", g_session_timer);
    return HAL_RET_OK;
}

void
tcp_close_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    hal_ret_t  ret; 
    session_t *session = (session_t *)ctxt;

    // time to clean up the session
    ret = fte::session_delete(session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delte aged session {}",
                      session->config.session_id);
    }    
}

hal_ret_t
tcp_close_timer_schedule (session_t *session)
{
    if (getenv("DISABLE_AGING")) {
        return HAL_RET_OK;
    }

    session->tcp_close_timer = hal::periodic::timer_schedule(HAL_TIMER_ID_TCP_CLOSE_WAIT,
                                                       HAL_TCP_CLOSE_WAIT_INTVL,
                                                       (void *)session,
                                                       tcp_close_cb, false);

    if (!session->tcp_close_timer) {
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("TCP Close timer started for session {}",
                     session->config.session_id);

    return HAL_RET_OK;
}

}    // namespace hal
