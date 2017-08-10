#include <base.h>
#include <hal.hpp>
#include <hal_state.hpp>
#include <session_svc.hpp>
#include <interface.hpp>
#include <endpoint.hpp>
#include <session.hpp>
#include <utils.hpp>
#include <pd_api.hpp>

namespace hal {

void *
session_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->session_id);
}

uint32_t
session_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(session_id_t)) % ht_size;
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
    HAL_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->hal_handle);
}

uint32_t
session_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
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
flow_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((flow_t *)entry)->key);
}

uint32_t
flow_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(flow_key_t)) % ht_size;
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
// validate an incoming session create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_session_create (SessionSpec& spec, SessionResponse *rsp)
{
    if (!spec.has_meta() ||
        spec.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!spec.has_initiator_flow()) {
        rsp->set_api_status(types::API_STATUS_FLOW_INFO_INVALID);
        return HAL_RET_INVALID_ARG;
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
        g_hal_state->flow_slab()->free(session->iflow);
    }
    if (session->rflow) {
        g_hal_state->flow_slab()->free(session->rflow);
    }
    if (session->tcp_state) {
        g_hal_state->tcp_state_slab()->free(session->tcp_state);
    }
    g_hal_state->session_slab()->free(session);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// extract flow key from the flow spec
//------------------------------------------------------------------------------
static hal_ret_t
extract_flow_key_from_spec (tenant_id_t tid, bool is_src_ep_local,
                            flow_key_t *flow_key, const FlowSpec& flow_spec)
{
    const FlowKey&    flow_spec_key = flow_spec.flow_key();

    flow_key->dir = is_src_ep_local ? FLOW_DIR_FROM_ENIC : FLOW_DIR_FROM_UPLINK;
    if (flow_spec_key.has_l2_key()) {
        flow_key->flow_type = FLOW_TYPE_L2;
        flow_key->l2seg_id = flow_spec_key.l2_key().l2_segment_id();
        MAC_UINT64_TO_ADDR(flow_key->smac, flow_spec_key.l2_key().smac());
        MAC_UINT64_TO_ADDR(flow_key->dmac, flow_spec_key.l2_key().dmac());

        HAL_TRACE_DEBUG("Flow Info: tid:{}; smac: {}; dmac: {})",
                tid, macaddr2str(flow_key->smac),
                macaddr2str(flow_key->dmac));
    } else if (flow_spec_key.has_v4_key()) {
        flow_key->flow_type = FLOW_TYPE_V4;
        flow_key->tenant_id = tid;
        flow_key->sip.v4_addr = flow_spec_key.v4_key().sip();
        flow_key->dip.v4_addr = flow_spec_key.v4_key().dip();
        flow_key->proto = flow_spec_key.v4_key().ip_proto();
        if ((flow_key->proto == types::IP_PROTO_TCP) ||
            (flow_key->proto == types::IP_PROTO_UDP)) {
            flow_key->sport = flow_spec_key.v4_key().tcp_udp().sport();
            flow_key->dport = flow_spec_key.v4_key().tcp_udp().dport();;
        } else if (flow_key->proto == types::IP_PROTO_ICMP) {
            flow_key->icmp_type = flow_spec_key.v4_key().icmp().type();
            flow_key->icmp_code = flow_spec_key.v4_key().icmp().code();
            flow_key->icmp_id = flow_spec_key.v4_key().icmp().id();
        } else {
            flow_key->sport = flow_key->dport = 0;
        }
        HAL_TRACE_DEBUG("Flow Info: tid:{}; sip: {}; dip: {}; "
                "proto: {}; sport: {}; dport: {}; "
                "icmp_type: {}; icmp_code: {}; icmp_id: {})",
                tid, ipv4addr2str(flow_key->sip.v4_addr),
                ipv4addr2str(flow_key->dip.v4_addr),
                flow_key->proto, flow_key->sport, flow_key->dport,
                flow_key->icmp_type, flow_key->icmp_code, flow_key->icmp_id);
    } else if (flow_spec_key.has_v6_key()) {
        flow_key->flow_type = FLOW_TYPE_V6;
        flow_key->tenant_id = tid;
        memcpy(flow_key->sip.v6_addr.addr8,
               flow_spec_key.v6_key().sip().v6_addr().c_str(),
               IP6_ADDR8_LEN);
        memcpy(flow_key->dip.v6_addr.addr8,
               flow_spec_key.v6_key().dip().v6_addr().c_str(),
               IP6_ADDR8_LEN);
        flow_key->proto = flow_spec_key.v6_key().ip_proto();
        if ((flow_key->proto == types::IP_PROTO_TCP) ||
            (flow_key->proto == types::IP_PROTO_UDP)) {
            flow_key->sport = flow_spec_key.v6_key().tcp_udp().sport();
            flow_key->dport = flow_spec_key.v6_key().tcp_udp().dport();;
        } else if (flow_key->proto == types::IP_PROTO_ICMP) {
            flow_key->icmp_type = flow_spec_key.v6_key().icmp().type();
            flow_key->icmp_code = flow_spec_key.v6_key().icmp().code();
            flow_key->icmp_id = flow_spec_key.v6_key().icmp().id();
        } else {
            flow_key->sport = flow_key->dport = 0;
        }
        HAL_TRACE_DEBUG("Flow Info: tid:{}; sip: {}; dip: {}; "
                "proto: {}; sport: {}; dport: {}; "
                "icmp_type: {}; icmp_code: {}; icmp_id: {})",
                tid, ipv6addr2str(flow_key->sip.v6_addr),
                ipv6addr2str(flow_key->dip.v6_addr),
                flow_key->proto, flow_key->sport, flow_key->dport,
                flow_key->icmp_type, flow_key->icmp_code, flow_key->icmp_id);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// extract all the flow data from flow spec
//------------------------------------------------------------------------------
static hal_ret_t
extract_flow_info_from_spec (flow_t *flow, bool is_initiator_flow,
                             const FlowSpec& flow_spec)
{
    HAL_SPINLOCK_INIT(&flow->slock, PTHREAD_PROCESS_PRIVATE);
    flow->state = flow_spec.flow_data().flow_info().tcp_state();
    flow->action = flow_spec.flow_data().flow_info().flow_action();
    flow->role = is_initiator_flow ? FLOW_ROLE_INITIATOR : FLOW_ROLE_RESPONDER;
    flow->nat_type = flow_spec.flow_data().flow_info().nat_type();
    ip_addr_spec_to_ip_addr(&flow->nat_sip,
                            flow_spec.flow_data().flow_info().nat_sip());
    ip_addr_spec_to_ip_addr(&flow->nat_dip,
                            flow_spec.flow_data().flow_info().nat_dip());
    flow->nat_sport = flow_spec.flow_data().flow_info().nat_sport();
    flow->nat_dport = flow_spec.flow_data().flow_info().nat_dport();
    flow->lif_qtype = flow_spec.flow_data().flow_info().queue_type();

    if (flow_spec.flow_data().has_conn_track_info()) {
        flow->create_ts =
            flow_spec.flow_data().conn_track_info().flow_create_ts();
        flow->last_pkt_ts = flow->create_ts;
        flow->packets = flow_spec.flow_data().conn_track_info().flow_packets();
        flow->bytes = flow_spec.flow_data().conn_track_info().flow_bytes();
        flow->exception_bmap =
            flow_spec.flow_data().conn_track_info().exception_bits();
    }
    flow->flow_key_ht_ctxt.reset();

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// extract all the TCP related state from session spec
//------------------------------------------------------------------------------
hal_ret_t
extract_tcp_state_from_spec (session_t *session, bool is_initiator_flow,
                             const ConnTrackInfo& conn_track_info)
{
    flow_tcp_state_t    *tcp_state;

    if (is_initiator_flow) {
        tcp_state = &session->tcp_state->iflow_state;
    } else {
        tcp_state = &session->tcp_state->rflow_state;
    }

    tcp_state->tcp_seq_num = conn_track_info.tcp_seq_num();
    tcp_state->tcp_ack_num = conn_track_info.tcp_ack_num();
    tcp_state->tcp_win_sz = conn_track_info.tcp_win_sz();
    tcp_state->tcp_win_scale = conn_track_info.tcp_win_scale();
    tcp_state->tcp_mss = conn_track_info.tcp_mss();

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// given a flow key, get the source and destination endpoint records
//------------------------------------------------------------------------------
static hal_ret_t
ep_get_from_flow_key_spec (tenant_id_t tid, const FlowKey& flow_key,
                           ep_t **sep, ep_t **dep)
{
    mac_addr_t    mac_addr;
    ip_addr_t     ip_addr;

    *sep = *dep = NULL;
    HAL_TRACE_DEBUG("has_l2_key: {}; has_v4_key: {}; has_v6_key: {}",
            flow_key.has_l2_key(), flow_key.has_v4_key(), flow_key.has_v6_key());
    if (flow_key.has_l2_key()) {
        MAC_UINT64_TO_ADDR(mac_addr, flow_key.l2_key().smac());
        *sep = find_ep_by_l2_key(flow_key.l2_key().l2_segment_id(),
                                 mac_addr);
        if (*sep == NULL) {
            HAL_TRACE_ERR("Src EP with key ({}, {}) not found",
                          tid, macaddr2str(mac_addr));
            return HAL_RET_EP_NOT_FOUND;
        }

        MAC_UINT64_TO_ADDR(mac_addr, flow_key.l2_key().dmac());
        *dep = find_ep_by_l2_key(flow_key.l2_key().l2_segment_id(),
                                 mac_addr);
        if (*dep == NULL) {
            HAL_TRACE_ERR("Dst EP with key ({}, {}) not found",
                          tid, macaddr2str(mac_addr));
            return HAL_RET_EP_NOT_FOUND;
        }
        return HAL_RET_OK;
    } else if (flow_key.has_v4_key()) {

        *sep = find_ep_by_v4_key(tid, flow_key.v4_key().sip());
        if (*sep == NULL) {
            HAL_TRACE_ERR("Src EP with key ({}, {}) not found",
                          tid, ipv4addr2str(flow_key.v4_key().sip()));
            return HAL_RET_EP_NOT_FOUND;
        }

        *dep = find_ep_by_v4_key(tid, flow_key.v4_key().dip());
        if (*dep == NULL) {
            HAL_TRACE_ERR("Dst EP with key ({}, {}) not found",
                          tid, ipv4addr2str(flow_key.v4_key().dip()));
            return HAL_RET_EP_NOT_FOUND;
        }
    } else if (flow_key.has_v6_key()) {
        ip_addr_spec_to_ip_addr(&ip_addr, flow_key.v6_key().sip());
        *sep = find_ep_by_v6_key(tid, &ip_addr);
        if (*sep == NULL) {
            HAL_TRACE_ERR("Src EP with key ({}, {}) not found",
                          tid, ipaddr2str(&ip_addr));
            return HAL_RET_EP_NOT_FOUND;
        }

        ip_addr_spec_to_ip_addr(&ip_addr, flow_key.v6_key().dip());
        *dep = find_ep_by_v6_key(tid, &ip_addr);
        if (*dep == NULL) {
            HAL_TRACE_ERR("Dst EP with key ({}, {}) not found",
                          tid, ipaddr2str(&ip_addr));
            return HAL_RET_EP_NOT_FOUND;
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this session in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_session_to_db (tenant_t *tenant, l2seg_t *l2seg_s, l2seg_t *l2seg_d,
                   ep_t *sep, ep_t *dep, if_t *sif, if_t *dif,
                   session_t *session)
{
    session->session_id_ht_ctxt.reset();
    g_hal_state->session_id_ht()->insert(session,
                                         &session->session_id_ht_ctxt);

    session->hal_handle_ht_ctxt.reset();
    g_hal_state->session_hal_handle_ht()->insert(session,
                                                 &session->hal_handle_ht_ctxt);

    if (sep) {
        utils::dllist_reset(&session->sep_session_lentry);
        HAL_SPINLOCK_LOCK(&sep->slock);
        utils::dllist_add(&sep->session_list_head,
                          &session->sep_session_lentry);
        HAL_SPINLOCK_UNLOCK(&sep->slock);
    }

    if (dep) {
        utils::dllist_reset(&session->dep_session_lentry);
        HAL_SPINLOCK_LOCK(&dep->slock);
        utils::dllist_add(&dep->session_list_head,
                          &session->dep_session_lentry);
        HAL_SPINLOCK_UNLOCK(&dep->slock);
    }

    utils::dllist_reset(&session->sif_session_lentry);
    HAL_SPINLOCK_LOCK(&sif->slock);
    utils::dllist_add(&sif->session_list_head, &session->sif_session_lentry);
    HAL_SPINLOCK_UNLOCK(&sif->slock);

    if (sif != dif) {
        utils::dllist_reset(&session->dif_session_lentry);
        HAL_SPINLOCK_LOCK(&dif->slock);
        utils::dllist_add(&dif->session_list_head,
                          &session->dif_session_lentry);
        HAL_SPINLOCK_UNLOCK(&dif->slock);
    }

    utils::dllist_reset(&session->tenant_session_lentry);
    HAL_SPINLOCK_LOCK(&tenant->slock);
    utils::dllist_add(&tenant->session_list_head,
                      &session->tenant_session_lentry);
    HAL_SPINLOCK_UNLOCK(&tenant->slock);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a session create request
// TODO:
// 1. dif will be different in case there is NAT enabled
// 2. in case EPs are not found, do prefix lookups (for IPv4/IPv6 key)
// 3. not inserting EPs in L2 segment session lists yet
//------------------------------------------------------------------------------
hal_ret_t
session_create (SessionSpec& spec, SessionResponse *rsp)
{
    hal_ret_t               ret;
    tenant_id_t             tid;
    tenant_t                *tenant;
    FlowSpec                flow_spec;
    FlowKey                 flow_key;
    if_t                    *sif, *dif;
    l2seg_t                 *l2seg_s, *l2seg_d;
    ep_t                    *sep, *dep;
    session_t               *session = NULL;
    bool                    tcp_session = false;
    nwsec_profile_t         *nwsec_prof;
    pd::pd_session_args_t    pd_session_args;


    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-Session:{}: Session id {} Create in Tenant id {}", __FUNCTION__, 
                    spec.session_id(), spec.meta().tenant_id());

    // do basic validation
    ret = validate_session_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the tenant information
    tid = spec.meta().tenant_id();
    tenant = find_tenant_by_id(tid);
    if (tenant == NULL) {
        rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        HAL_TRACE_ERR("{}: tenant {} not found", __func__, tid);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // fetch the security profile, if any
    if (tenant->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        nwsec_prof = find_nwsec_profile_by_handle(tenant->nwsec_profile_handle);
    } else {
        nwsec_prof = NULL;
    }

    // allocate a session
    session = (session_t *)g_hal_state->session_slab()->alloc();
    if (session == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    session->session_id = spec.session_id();
    session->app_session = NULL;
    session->iflow = session->rflow = NULL;
    session->tcp_state = NULL;

    // allocate memory for flows
    session->iflow = (flow_t *)g_hal_state->flow_slab()->alloc();
    if (session->iflow == NULL) {
        ret = HAL_RET_OOM;
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        goto end;
    }
    session->iflow->session = session;

    if (spec.has_responder_flow()) {
        session->rflow = (flow_t *)g_hal_state->flow_slab()->alloc();
        if (session->rflow == NULL) {
            ret = HAL_RET_OOM;
            rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
            goto end;
        }
        session->rflow->session = session;
        session->iflow->reverse_flow = session->rflow;
        session->rflow->reverse_flow = session->iflow;
    }

    // get the src and dst EPs from the flow key
    if (ep_get_from_flow_key_spec(tid, spec.initiator_flow().flow_key(),
                                  &sep, &dep) != HAL_RET_OK) {
        if (sep == NULL) {
            HAL_TRACE_ERR("Source EP not found");
        }
        if (dep == NULL) {
            HAL_TRACE_ERR("Destination EP not found");
        }
        rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
        ret = HAL_RET_EP_NOT_FOUND;
        goto end;
    }

    // extract initiator's flow key and data from flow spec
    extract_flow_key_from_spec(tid, sep->ep_flags & EP_FLAGS_LOCAL,
                               &session->iflow->key, spec.initiator_flow());
    extract_flow_info_from_spec(session->iflow, TRUE, spec.initiator_flow());
    if (spec.conn_track_en() &&
        ((session->iflow->key.flow_type == FLOW_TYPE_V4) ||
         (session->iflow->key.flow_type == FLOW_TYPE_V6)) &&
        (session->iflow->key.proto == types::IP_PROTO_TCP)) {
        session->tcp_state =
            (tcp_state_t *)g_hal_state->tcp_state_slab()->alloc();
        if (session->tcp_state == NULL) {
            ret = HAL_RET_OOM;
            rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
            goto end;
        }
        session->tcp_state->tcp_ts_option = spec.tcp_ts_option();
        tcp_session = true;
        extract_tcp_state_from_spec(session, TRUE,
                                    spec.initiator_flow().flow_data().conn_track_info());
        session->syn_ack_delta = spec.iflow_syn_ack_delta();
    }

    // extract responder's flow key and data from flow spec
    if (session->rflow) {
        extract_flow_key_from_spec(tid, dep->ep_flags & EP_FLAGS_LOCAL,
                                   &session->rflow->key,
                                   spec.responder_flow());
        extract_flow_info_from_spec(session->rflow, FALSE,
                                    spec.responder_flow());
        if (tcp_session) {
            extract_tcp_state_from_spec(session, FALSE,
                                        spec.responder_flow().flow_data().conn_track_info());
        }
    }

    // lookup ingress & egress interfaces
    sif = (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&sep->if_handle);
    dif = (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&dep->if_handle);
    if ((sif == NULL) || (dif == NULL)) {
        HAL_TRACE_ERR("Src/Dst interface not found");
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        ret = HAL_RET_IF_NOT_FOUND;
        goto end;
    }

    // lookup ingress & egress L2 segments
    l2seg_s =
        (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&sep->l2seg_handle);
    l2seg_d =
        (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&dep->l2seg_handle);
    if ((l2seg_s == NULL) || (l2seg_d == NULL)) {
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        goto end;
    }

    // allocate all PD resources and finish programming, if any
    pd::pd_session_args_init(&pd_session_args);
    pd_session_args.tenant = tenant;
    pd_session_args.nwsec_prof = nwsec_prof;
    pd_session_args.l2seg_s = l2seg_s;
    pd_session_args.l2seg_d = l2seg_d;
    pd_session_args.sif = sif;
    pd_session_args.dif = dif;
    pd_session_args.sep = sep;
    pd_session_args.dep = dep;
    pd_session_args.session = session;
    ret = pd::pd_session_create(&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // add this session to our db
    add_session_to_db(tenant, l2seg_s, l2seg_d, sep, dep, sif, dif, session);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_session_handle(session->hal_handle);

end:

    if (ret != HAL_RET_OK && session) {
        session_cleanup(session);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
session_get (SessionGetRequest& spec, SessionGetResponse *rsp)
{
    return HAL_RET_OK;
}
}    // namespace hal

