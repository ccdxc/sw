#include <base.h>
#include <hal.hpp>
#include <hal_state.hpp>
#include <session_svc.hpp>
#include <interface.hpp>
#include <endpoint.hpp>
#include <endpoint_api.hpp>
#include <session.hpp>
#include <utils.hpp>
#include <pd_api.hpp>
#include <interface_api.hpp>
#include <qos.hpp>

namespace hal {

void *
session_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->config.session_id);
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
    return (void *)&(((flow_t *)entry)->config.key);
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
        out.write("l2seg={}, smac={}, dmac={}", key.l2seg_id, key.smac, key.dmac);
        break;
    case FLOW_TYPE_V4:
    case FLOW_TYPE_V6:
        out.write("tid={}, ", key.tenant_id);
        if (key.flow_type == FLOW_TYPE_V4) {
            out.write("sip={}, dip={}, ", key.sip.v4_addr, key.dip.v4_addr);
        } else {
            out.write("sip={}, dip={}, ", key.sip.v6_addr, key.dip.v6_addr);
        }

        switch (key.proto) {
        case types::IP_PROTO_ICMP:
            out.write("proto=icmp, type={}, code={}, id={}",
                      key.icmp_type, key.icmp_code, key.icmp_id);
            break;
        case types::IP_PROTO_TCP:
            out.write("proto=tcp, sport={}, dport={}", key.sport, key.dport);
            break;
        case types::IP_PROTO_UDP:
            out.write("proto=udp, sport={}, dport={}", key.sport, key.dport);
            break;
        default:
            out.write("proto={}", key.proto);
        }
        break;
    default:
        out.write("flow-type=unknonw(%d)", key.flow_type);
    }

    out.write("}}");

    return buf;
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
        flow_key->ether_type = flow_spec_key.l2_key().ether_type();
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
            // Bharat: TODO: For now we are not handling ICMP flows which are 
            //               neither echo req or echo reply. Have to check 
            //               if we even need to install flows for other 
            //               types. If yes, we have to see how to form
            //               sport and dport. Revisit while full testing
            //               of icmp. icmp_id: 0(Req), 8 (Reply)
            HAL_ASSERT_RETURN(flow_key->icmp_type == 0 || 
                    flow_key->icmp_type == 8, HAL_RET_INVALID_ARG);
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
        } else if (flow_key->proto == types::IP_PROTO_ICMPV6) {
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
extract_flow_info_from_spec (flow_cfg_t *flow, bool is_initiator_flow,
                             const FlowSpec& flow_spec)
{
    hal_ret_t ret = HAL_RET_OK;

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

    ret = qos_extract_action_from_spec(&flow->in_qos_action, 
                                       flow_spec.flow_data().flow_info().in_qos_actions(),
                                       INGRESS_QOS);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = qos_extract_action_from_spec(&flow->eg_qos_action, 
                                       flow_spec.flow_data().flow_info().eg_qos_actions(),
                                       EGRESS_QOS);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// extract all the TCP related state from session spec
//------------------------------------------------------------------------------
hal_ret_t
extract_session_state_from_spec (session_state_t *session_state, bool is_initiator_flow,
                             const ConnTrackInfo& conn_track_info)
{
    flow_state_t    *flow_state;

    if (is_initiator_flow) {
        flow_state = &session_state->iflow_state;
    } else {
        flow_state = &session_state->rflow_state;
    }

    flow_state->tcp_seq_num = conn_track_info.tcp_seq_num();
    flow_state->tcp_ack_num = conn_track_info.tcp_ack_num();
    flow_state->tcp_win_sz = conn_track_info.tcp_win_sz();
    flow_state->tcp_win_scale = conn_track_info.tcp_win_scale();
    flow_state->tcp_mss = conn_track_info.tcp_mss();
    flow_state->create_ts = conn_track_info.flow_create_ts();
    flow_state->last_pkt_ts = flow_state->create_ts;
    flow_state->packets = conn_track_info.flow_packets();
    flow_state->bytes = conn_track_info.flow_bytes();
    flow_state->exception_bmap = conn_track_info.exception_bits();

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// given a flow key, get the source and destination endpoint records
//------------------------------------------------------------------------------
hal_ret_t
ep_get_from_flow_key(const flow_key_t* key, ep_t **sep, ep_t **dep)
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
        l3key.tenant_id = key->tenant_id;
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
// given a flow key spec, get the source and destination endpoint records
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
                          flow_key.l2_key().l2_segment_id(),
                          macaddr2str(mac_addr));
            return HAL_RET_EP_NOT_FOUND;
        }

        MAC_UINT64_TO_ADDR(mac_addr, flow_key.l2_key().dmac());
        *dep = find_ep_by_l2_key(flow_key.l2_key().l2_segment_id(),
                                 mac_addr);
        if (*dep == NULL) {
            HAL_TRACE_ERR("Dst EP with key ({}, {}) not found",
                          flow_key.l2_key().l2_segment_id(),
                          macaddr2str(mac_addr));
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
// Updates flow parameters for flow if the flow undergoes routing/bridging
//------------------------------------------------------------------------------
static inline hal_ret_t
update_flow_for_routing(session_args_t *args, SessionResponse *rsp, 
                           bool is_initiator_flow)
{
    hal_ret_t   ret = HAL_RET_OK;
    flow_cfg_t  *flow = NULL;
    ep_t        *sep = NULL, *dep = NULL;
    l2seg_t     *sl2seg = NULL, *dl2seg = NULL;
    if_t        *sif = NULL, *dif = NULL;
    bool        /*ing_tag = false, */egr_tag = false;
    uint8_t     ivlan_v = 0, evlan_v = 0;
    uint16_t    ivlan_id = 0, evlan_id = 0;

    if (is_initiator_flow) {
        flow = args->iflow;
        sep = args->sep;
        dep = args->dep;
        sl2seg = args->sl2seg;
        dl2seg = args->dl2seg;
    } else {
        flow = args->rflow;
        sep = args->dep;
        dep = args->sep;
        sl2seg = args->dl2seg;
        dl2seg = args->sl2seg;
    }

    sif = ep_find_if_by_handle(sep);
    HAL_ASSERT_RETURN(sif != NULL, HAL_RET_IF_NOT_FOUND);

    dif = ep_find_if_by_handle(dep);
    HAL_ASSERT_RETURN(dif != NULL, HAL_RET_IF_NOT_FOUND);

	// Derive lport
	// TODO: Handle service case
	flow->lport = hal::pd::if_get_lport_id(dif);

    if_l2seg_get_encap(sif, sl2seg, &ivlan_v, &ivlan_id);
    // ing_tag = (ivlan_v == 1) ? true : false;

    if_l2seg_get_encap(dif, dl2seg, &evlan_v, &evlan_id);
    egr_tag = (evlan_v == 1) ? true : false;

    HAL_TRACE_DEBUG("if_id:{}, l2seg_id:{}", dif->if_id, dl2seg->seg_id);

	// ---------------------     Encap Logic    ----------------------
	//  Bridging:
	//      Flow: mac_sa_rw:0, mac_da_rw:0, ttl_dec:0
	//      -> Untag:
	//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 0]
	//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, dscp rwr)
	//          -> tnnl_rwr_table[0] (nop)
	//      -> Tag:
	//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 1, 
    //				    tnnl_vnid = fab_encap if dif is uplink, 
    //                  if enic comes from output_mapping]
	//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, dscp rwr)
	//          -> tnnl_rwr_table[1] (encap with tnnl_vnid if eif is Uplink,
	//                                encap from outpu_mapping, cos rwr)
	//
	//  Routing:
	//     Flow: mac_sa_rw:1, mac_da_rw:1, ttl_dec:1
	//      -> Untag:
	//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 0]
	//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, 
    //                                           dscp rw, macs' rw, ttl_dec)
	//          -> tnnl_rwr_table[0] (nop)
	//      -> Tag:
	//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 1]
	//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, dscp rwr)
	//          -> tnnl_rwr_table[1] (encap with tnnl_vnid if eif is Uplink,
	//                                encap from outpu_mapping, cos rwr)
    // TODO: Have to still handle priority tag. When do we send a priority
	//       tagged pkt.


    if (sl2seg != dl2seg) {
        // Routing
        flow->ttl_dec = true;
        flow->mac_sa_rewrite = true;
        flow->mac_da_rewrite = true;
        flow->rw_act = REWRITE_REWRITE_ID;

    } else {
        // Bridging
        flow->ttl_dec = false;
        flow->mac_sa_rewrite = false;
        flow->mac_da_rewrite = false;
        flow->rw_act = REWRITE_REWRITE_ID;

    }

	if (dif->if_type != intf::IF_TYPE_TUNNEL) {
		if (!egr_tag) {
			flow->tnnl_rw_act = TUNNEL_REWRITE_NOP_ID;
		} else {
			flow->tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VLAN_ID;
			if (dif->if_type == intf::IF_TYPE_UPLINK ||
					dif->if_type == intf::IF_TYPE_UPLINK_PC) {
				flow->tnnl_vnid = dl2seg->fabric_encap.val;
			}
		} 
	} else {
		// TODO: Handle Tunnel case
		// Vnid: flow's tnnl_vnid
		// OVlan, Omacsa, Omacda, OSIP, ODIP: from tunnl_rewr table
		// flow->tnnl_vnid = dl2seg->fabric_encap.val;
		// flow->tnnl_rw_act = 
	}

	return ret;
}



//------------------------------------------------------------------------------
// Updates flow parameters for sessions if the flow undergoes routing
//------------------------------------------------------------------------------
static inline hal_ret_t
update_session_forwarding_info(session_args_t *args, SessionResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = update_flow_for_routing(args, rsp, TRUE);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = update_flow_for_routing(args, rsp, FALSE);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Updates the flow parameters for NAT
//------------------------------------------------------------------------------
static inline hal_ret_t
update_flow_for_dest_nat(tenant_t *tenant, flow_cfg_t *flow, l2seg_t **dl2seg, 
                         if_t **dif, ep_t **dep)
{
    l2seg_t *dl2seg_nat;
    if_t *dif_nat;
    ep_t *dep_nat;

    if (flow->nat_type != NAT_TYPE_DNAT && flow->nat_type != NAT_TYPE_TWICE_NAT) {
        return HAL_RET_OK;
    }

    if (flow->nat_dip.af == IP_AF_IPV4) {
        dep_nat = find_ep_by_v4_key(tenant->tenant_id,
                                    flow->nat_dip.addr.v4_addr);
    } else {
        dep_nat = find_ep_by_v6_key(tenant->tenant_id, &flow->nat_dip);
    }

    if (dep_nat == NULL) {
        return HAL_RET_EP_NOT_FOUND;
    }

    dl2seg_nat = find_l2seg_by_handle(dep_nat->l2seg_handle);
    HAL_ASSERT(dl2seg_nat != NULL);

    dif_nat = find_if_by_handle(dep_nat->if_handle);
    HAL_ASSERT(dif_nat != NULL);

#if 0
    if (dl2seg_nat != *dl2seg) {
        // routing
        flow->mac_sa_rewrite = true;
    }

    if (dep_nat != *dep) {
        flow->mac_da_rewrite = true;
    }
#endif

    *dl2seg = dl2seg_nat;
    *dif = dif_nat;
    *dep = dep_nat;

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

static flow_t *
flow_create(const flow_cfg_t *cfg, session_t *session)
{
    flow_t *flow = (flow_t *)g_hal_state->flow_slab()->alloc();
    if (!flow) {
        return NULL;
    }

    *flow = {};
    HAL_SPINLOCK_INIT(&flow->slock, PTHREAD_PROCESS_PRIVATE);
    flow->flow_key_ht_ctxt.reset();

    if (cfg) {
        flow->config = *cfg;
    }

    flow->session = session;
    return flow;
}

#if 0
hal_ret_t
flow_update_fwding_info(flow_cfg_t *flow, if_t *dif)
{
    flow->lif = hal::pd::if_get_hw_lif_id(dif);

    if (dif->if_type != intf::IF_TYPE_ENIC) {
        flow->qid_en = 0;
        return HAL_RET_OK;
    }

    // TODO Update flow_info

    return HAL_RET_OK;
}
#endif

hal_ret_t
session_create (const session_args_t *args, hal_handle_t *session_handle)
{
    hal_ret_t ret;
    nwsec_profile_t         *nwsec_prof;
    pd::pd_session_args_t    pd_session_args;
    session_t               *session;

    HAL_ASSERT(args->tenant && args->iflow && args->sep && args->dep &&
               args->sif && args->dif && args->sl2seg && args->dl2seg);

    // allocate a session
    session = (session_t *)g_hal_state->session_slab()->alloc();
    if (session == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    *session = {};
    session->config = *args->session;

    // fetch the security profile, if any
    if (args->tenant->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        nwsec_prof = find_nwsec_profile_by_handle(args->tenant->nwsec_profile_handle);
    } else {
        nwsec_prof = NULL;
    }

    //create flows
    session->iflow = flow_create(args->iflow, session);
    if (session->iflow == NULL) {
        ret = HAL_RET_OOM;
        goto end;
     }

    if (args->rflow) {
        session->rflow = flow_create(args->rflow, session);
        if (session->rflow == NULL) {
            ret = HAL_RET_OOM;
            goto end;
        }
        session->iflow->reverse_flow = session->rflow;
        session->rflow->reverse_flow = session->iflow;
    }

    // allocate all PD resources and finish programming, if any
    pd::pd_session_args_init(&pd_session_args);
    pd_session_args.tenant = args->tenant;
    pd_session_args.nwsec_prof = nwsec_prof;
    pd_session_args.l2seg_s = args->sl2seg;
    pd_session_args.l2seg_d = args->dl2seg;
    pd_session_args.sif = args->sif;
    pd_session_args.dif = args->dif;
    pd_session_args.sep = args->sep;
    pd_session_args.dep = args->dep;
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;

    ret = pd::pd_session_create(&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session create failure, err : {}", ret);
        goto end;
    }

    // add this session to our db
    add_session_to_db(args->tenant, args->sl2seg, args->dl2seg,
                      args->sep, args->dep, args->sif, args->dif, session);
    HAL_ASSERT(ret == HAL_RET_OK);

    if (session_handle) {
        *session_handle = session->hal_handle;
    }

 end:
    if (session && ret != HAL_RET_OK) {
        HAL_TRACE_ERR("session create failure, err={}", ret);
        session_cleanup(session);
    }

    return ret;
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
    hal_handle_t            session_handle;
    session_args_t          args = {};
    session_cfg_t           session = {};
    flow_cfg_t              iflow = {};
    flow_cfg_t              rflow = {};
    session_state_t      session_state = {};

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-Session:{}: Session id {} Create in Tenant id {}", __FUNCTION__, 
                    spec.session_id(), spec.meta().tenant_id());

    // do basic validation
    ret = validate_session_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    args.session = &session;
    session.session_id = spec.session_id();

    // fetch the tenant information
    tid = spec.meta().tenant_id();
    args.tenant = find_tenant_by_id(tid);
    if (args.tenant == NULL) {
        rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        HAL_TRACE_ERR("{}: tenant {} not found", __func__, tid);
        return HAL_RET_INVALID_ARG;
    }
    args.iflow = &iflow;
    if (spec.has_responder_flow()) {
        args.rflow = &rflow;
    }


    // get the src and dst EPs from the flow key
    if (ep_get_from_flow_key_spec(tid, spec.initiator_flow().flow_key(),
                                  &args.sep, &args.dep) != HAL_RET_OK) {
        if (args.sep == NULL) {
            HAL_TRACE_ERR("Source EP not found");
        }
        if (args.dep == NULL) {
            HAL_TRACE_ERR("Destination EP not found");
        }
        rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
        return HAL_RET_EP_NOT_FOUND;
    }

    // extract initiator's flow key and data from flow spec
    extract_flow_key_from_spec(tid, args.sep->ep_flags & EP_FLAGS_LOCAL,
                               &args.iflow->key, spec.initiator_flow());
    ret = extract_flow_info_from_spec(args.iflow, TRUE, spec.initiator_flow());
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return ret;
    }
    session.conn_track_en = spec.conn_track_en();
    if (session.conn_track_en) {
        args.session_state = &session_state;
        args.session_state->tcp_ts_option = spec.tcp_ts_option();
        extract_session_state_from_spec(args.session_state, TRUE,
                          spec.initiator_flow().flow_data().conn_track_info());
        session.syn_ack_delta = spec.iflow_syn_ack_delta();
    }

    // extract responder's flow key and data from flow spec
    if (args.rflow) {
        extract_flow_key_from_spec(tid, args.dep->ep_flags & EP_FLAGS_LOCAL,
                                   &args.rflow->key, spec.responder_flow());
        ret = extract_flow_info_from_spec(args.rflow, FALSE, spec.responder_flow());
        if (ret != HAL_RET_OK) {
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return ret;
        }
        if (session.conn_track_en) {
            extract_session_state_from_spec(args.session_state, FALSE,
                          spec.responder_flow().flow_data().conn_track_info());
        }
    }

    // lookup ingress & egress interfaces
    args.sif = (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&args.sep->if_handle);
    args.dif = (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&args.dep->if_handle);
    if ((args.sif == NULL) || (args.dif == NULL)) {
        HAL_TRACE_ERR("Src/Dst interface not found");
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return  HAL_RET_IF_NOT_FOUND;
    }

    // lookup ingress & egress L2 segments
    args.sl2seg =
        (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&args.sep->l2seg_handle);
    args.dl2seg =
        (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&args.dep->l2seg_handle);
    if ((args.sl2seg == NULL) || (args.dl2seg == NULL)) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }


    // update flows for dnat
    ret = update_flow_for_dest_nat(args.tenant, args.iflow, &args.dl2seg, &args.dif, &args.dep);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
        return ret;
    }
    ret = update_flow_for_dest_nat(args.tenant, args.rflow, &args.sl2seg, &args.sif, &args.sep);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
        return ret;
    }

    ret = update_session_forwarding_info(&args, rsp);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    
#if 0
    // Update fwding info
    ret = flow_update_fwding_info(args.iflow, args.dif);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return ret;
    }

    ret = flow_update_fwding_info(args.rflow, args.sif);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return ret;
    }
#endif

    ret = session_create(&args, &session_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("session create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        return ret;
    }

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_session_handle(session_handle);

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

hal_ret_t
session_get (SessionGetRequest& spec, SessionGetResponse *rsp)
{
    return HAL_RET_OK;
}
}    // namespace hal

