// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
// #include "nic/hal/svc/session_svc.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/include/endpoint_api.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/src/qos.hpp"

using telemetry::MirrorSessionId;
using session::FlowInfo;

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
    HAL_TRACE_DEBUG("Entry HAL Handle:{}", ((session_t*)entry)->hal_handle);

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
session_get_iflow_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->iflow->config.key);
}

uint32_t
session_compute_iflow_hash_func (void *key, uint32_t ht_size)
{
    return (utils::hash_algo::fnv_hash(key, sizeof(flow_key_t)) % ht_size);
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
    return (utils::hash_algo::fnv_hash(key, sizeof(flow_key_t)) % ht_size);
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
        out.write("l2seg={}, smac={}, dmac={} etype={}", key.l2seg_id, macaddr2str(key.smac),
                  macaddr2str(key.dmac), key.ether_type);
        break;
    case FLOW_TYPE_V4:
    case FLOW_TYPE_V6:
        out.write("tid={}, ", key.tenant_id);
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
extract_flow_key_from_spec (tenant_id_t tid,
                            flow_key_t *flow_key, 
                            const FlowSpec& flow_spec)
{
    const FlowKey&    flow_spec_key = flow_spec.flow_key();
    return extract_flow_key_from_spec(tid, flow_key, flow_spec_key);
}

hal_ret_t
extract_flow_key_from_spec(tenant_id_t tid,
                           flow_key_t *flow_key,
                           const FlowKey& flow_spec_key)
{
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
        if ((flow_key->proto == types::IPPROTO_TCP) ||
            (flow_key->proto == types::IPPROTO_UDP)) {
            flow_key->sport = flow_spec_key.v4_key().tcp_udp().sport();
            flow_key->dport = flow_spec_key.v4_key().tcp_udp().dport();;
        } else if (flow_key->proto == types::IPPROTO_ICMP) {
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
        if ((flow_key->proto == types::IPPROTO_TCP) ||
            (flow_key->proto == types::IPPROTO_UDP)) {
            flow_key->sport = flow_spec_key.v6_key().tcp_udp().sport();
            flow_key->dport = flow_spec_key.v6_key().tcp_udp().dport();;
        } else if (flow_key->proto == types::IPPROTO_ICMPV6) {
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
extract_session_state_from_spec (session_state_t *session_state,
                                 bool is_initiator_flow,
                                 const FlowData& flow_data)
{
    flow_state_t    *flow_state;
    const ConnTrackInfo& conn_track_info = flow_data.conn_track_info();

    if (is_initiator_flow) {
        flow_state = &session_state->iflow_state;
    } else {
        flow_state = &session_state->rflow_state;
    }

    flow_state->state = flow_data.flow_info().tcp_state();
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

    HAL_TRACE_DEBUG("has_l2_key: {}; has_v4_key: {}; has_v6_key: {}",
                    flow_key.has_l2_key(), flow_key.has_v4_key(),
                    flow_key.has_v6_key());

    *sep = *dep = NULL;
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

// ---------------------     Encap Logic    ----------------------
//  Bridging:
//      Flow: mac_sa_rw:0, mac_da_rw:0, ttl_dec:0
//      -> Untag:
//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 0]
//          -> rewrite_table[EP's rewr_act] (decap if ing. tag, dscp rwr)
//          -> tnnl_rwr_table[0] (nop)
//      -> Tag:
//          -> Flow[rewr_idx: EP's rewr_act, tnnl_rewr: 1, 
//                    tnnl_vnid = fab_encap if dif is uplink, 
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
//       tagged pkt?
static inline hal_ret_t
update_encap_rw_info(if_t *dif, l2seg_t *sl2seg, l2seg_t *dl2seg,
                     flow_pgm_attrs_t *flow_attrs)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint8_t             evlan_v = 0;
    uint16_t            evlan_id = 0;
    bool                egr_tag = false;

    flow_attrs->lport = hal::pd::if_get_lport_id(dif);

    if (dif->if_type != intf::IF_TYPE_TUNNEL) {
        if_l2seg_get_encap(dif, dl2seg, &evlan_v, &evlan_id);
    }
    egr_tag = (evlan_v == 1) ? true : false;

    HAL_TRACE_DEBUG("if_id:{}, l2seg_id:{}", dif->if_id, dl2seg->seg_id);

    if (sl2seg != dl2seg) {
        // Routing
        flow_attrs->ttl_dec = true;
        flow_attrs->mac_sa_rewrite = true;
        flow_attrs->mac_da_rewrite = true;
        // flow_attrs->rw_act = REWRITE_REWRITE_ID;

    } else {
        // Bridging
        flow_attrs->ttl_dec = false;
        flow_attrs->mac_sa_rewrite = false;
        flow_attrs->mac_da_rewrite = false;
        // flow_attrs->rw_act = REWRITE_REWRITE_ID;

    }

    if (dif->if_type != intf::IF_TYPE_TUNNEL) {
        if (!egr_tag) {
            flow_attrs->tnnl_rw_act = TUNNEL_REWRITE_NOP_ID;
        } else {
            flow_attrs->tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VLAN_ID;
            if (dif->if_type == intf::IF_TYPE_UPLINK ||
                    dif->if_type == intf::IF_TYPE_UPLINK_PC) {
                flow_attrs->tnnl_vnid = dl2seg->fabric_encap.val;
            }
        } 
        flow_attrs->tunnel_orig = FALSE;
    } else if (dif->if_type == intf::IF_TYPE_TUNNEL) {
        // Vnid: flow's tnnl_vnid
        // OVlan, Omacsa, Omacda, OSIP, ODIP: from tunnl_rewr table
        HAL_ASSERT(dl2seg->fabric_encap.type == types::encapType::ENCAP_TYPE_VXLAN);
        flow_attrs->tnnl_vnid = dl2seg->fabric_encap.val;
        flow_attrs->tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VXLAN_ID;
        flow_attrs->tunnel_orig = TRUE;
    }
    return ret;
}


//------------------------------------------------------------------------------
// Get rw idx from l4lb 
//------------------------------------------------------------------------------
static inline uint32_t
flow_get_l4lb_rw_idx(flow_t *flow, rewrite_actions_en rw_act)
{
    l4lb_key_t              l4lb_key = {0};
    l4lb_service_entry_t    *l4lb = NULL;

    l4lb_key.tenant_id = flow->config.key.tenant_id;
    l4lb_key.proto = flow->config.key.proto;
    // Get the l4lb from vip and then (vip, port)
    memcpy(&l4lb_key.service_ip, &flow->pgm_attrs.nat_sip, sizeof(ip_addr_t));
    l4lb = find_l4lb_by_key(&l4lb_key);
    if (l4lb != NULL) {
        return pd::l4lb_pd_get_rw_tbl_idx_from_pi_l4lb(l4lb, rw_act);
    }
    l4lb_key.service_port = flow->pgm_attrs.nat_sport;
    l4lb = find_l4lb_by_key(&l4lb_key);
    if (l4lb != NULL) {
        return pd::l4lb_pd_get_rw_tbl_idx_from_pi_l4lb(l4lb, rw_act);
    } else {
        HAL_ASSERT_RETURN(0, 0);
    }
    return 0;
}


//------------------------------------------------------------------------------
// Get rewrite action given a flow - assumption is flow will have a rewrite
//------------------------------------------------------------------------------
static inline hal_ret_t
flow_update_rewrite_info(flow_t *flow)
{
    hal_ret_t           ret = HAL_RET_OK;
    flow_t              *res_flow = NULL;
    rewrite_actions_en  *rw_act = &flow->pgm_attrs.rw_act;
    flow_pgm_attrs_t    *flow_attrs = &flow->pgm_attrs;

    // get resultant flow
    res_flow = flow;
    if (flow->is_aug_flow) {
        res_flow = flow->assoc_flow;
    }
    
    // if there is no NAT or flow is L2, then set the default rw act.
    if (res_flow->config.nat_type == NAT_TYPE_NONE ||
            res_flow->config.key.flow_type == FLOW_TYPE_L2) {
        *rw_act = REWRITE_REWRITE_ID;
        goto end;
    }
    if (res_flow->config.key.flow_type == FLOW_TYPE_V4) {
        if (res_flow->config.nat_type == NAT_TYPE_SNAT &&
                res_flow->config.nat_type != NAT_TYPE_TWICE_NAT) {
            if (res_flow->config.key.proto == types::IPPROTO_ICMP) {
                *rw_act = REWRITE_IPV4_NAT_SRC_REWRITE_ID;
            } else {
                if (res_flow->config.nat_sport == 0) {
                    *rw_act = REWRITE_IPV4_NAT_SRC_REWRITE_ID;
                } else {
                    flow_attrs->nat_l4_port = flow->config.nat_sport;
                    *rw_act = (res_flow->config.key.proto == types::IPPROTO_TCP) ?
                        REWRITE_IPV4_NAT_SRC_TCP_REWRITE_ID :
                        REWRITE_IPV4_NAT_SRC_UDP_REWRITE_ID;
                }
            }
        }
        if (res_flow->config.nat_type == NAT_TYPE_DNAT &&
                res_flow->config.nat_type != NAT_TYPE_TWICE_NAT) {
            if (res_flow->config.key.proto == types::IPPROTO_ICMP) {
                *rw_act = REWRITE_IPV4_NAT_DST_REWRITE_ID;
            } else {
                if (res_flow->config.nat_dport == 0) {
                    *rw_act = REWRITE_IPV4_NAT_DST_REWRITE_ID;
                } else {
                    flow_attrs->nat_l4_port = flow->config.nat_dport;
                    *rw_act = (res_flow->config.key.proto == types::IPPROTO_TCP) ?
                        REWRITE_IPV4_NAT_DST_TCP_REWRITE_ID :
                        REWRITE_IPV4_NAT_DST_UDP_REWRITE_ID;
                }
            }
        }
        if (res_flow->config.nat_type == NAT_TYPE_TWICE_NAT) {
            if (res_flow->config.key.proto == types::IPPROTO_ICMP) {
                *rw_act = REWRITE_IPV4_TWICE_NAT_REWRITE_ID;
            } else {
                // only if both ports have to be rewritten
                // there is no option to write only one
                if (res_flow->config.nat_sport != 0 && res_flow->config.nat_dport != 0) {
                    *rw_act = (res_flow->config.key.proto == types::IPPROTO_TCP) ?
                        REWRITE_IPV4_TWICE_NAT_TCP_REWRITE_ID:
                        REWRITE_IPV4_TWICE_NAT_UDP_REWRITE_ID;
                    if (res_flow->role == FLOW_ROLE_INITIATOR) {
                        flow_attrs->nat_l4_port = flow->config.nat_sport;
                    } else {
                        flow_attrs->nat_l4_port = flow->config.nat_dport;
                    }
                } else {
                    *rw_act = REWRITE_IPV4_TWICE_NAT_REWRITE_ID;
                }
            }
        }
    }

    // v6 cases
    if (res_flow->config.key.flow_type == FLOW_TYPE_V6) {
        if (res_flow->config.nat_type == NAT_TYPE_SNAT &&
                res_flow->config.nat_type != NAT_TYPE_TWICE_NAT) {
            if (res_flow->config.key.proto == types::IPPROTO_ICMPV6) {
                *rw_act = REWRITE_IPV6_NAT_SRC_REWRITE_ID;
            } else {
                if (res_flow->config.nat_sport == 0) {
                    *rw_act = REWRITE_IPV6_NAT_SRC_REWRITE_ID;
                } else {
                    flow_attrs->nat_l4_port = flow->config.nat_sport;
                    *rw_act = (res_flow->config.key.proto == types::IPPROTO_TCP) ?
                        REWRITE_IPV6_NAT_SRC_TCP_REWRITE_ID :
                        REWRITE_IPV6_NAT_SRC_UDP_REWRITE_ID;
                }
            }
        }
        if (res_flow->config.nat_type == NAT_TYPE_DNAT &&
                res_flow->config.nat_type != NAT_TYPE_TWICE_NAT) {
            if (res_flow->config.key.proto == types::IPPROTO_ICMPV6) {
                *rw_act = REWRITE_IPV6_NAT_DST_REWRITE_ID;
            } else {
                if (res_flow->config.nat_dport == 0) {
                    *rw_act = REWRITE_IPV6_NAT_DST_REWRITE_ID;
                } else {
                    flow_attrs->nat_l4_port = flow->config.nat_dport;
                    *rw_act = (res_flow->config.key.proto == types::IPPROTO_TCP) ?
                        REWRITE_IPV6_NAT_DST_TCP_REWRITE_ID :
                        REWRITE_IPV6_NAT_DST_UDP_REWRITE_ID;
                }
            }
        }
        if (res_flow->config.nat_type == NAT_TYPE_TWICE_NAT) {
            if (res_flow->config.key.proto == types::IPPROTO_ICMPV6) {
                *rw_act = REWRITE_IPV6_TWICE_NAT_REWRITE_ID;
            } else {
                // only if both ports have to be rewritten
                // there is no option to write only one
                if (res_flow->config.nat_sport != 0 && res_flow->config.nat_dport != 0) {
                    *rw_act = (res_flow->config.key.proto == types::IPPROTO_TCP) ?
                        REWRITE_IPV6_TWICE_NAT_TCP_REWRITE_ID:
                        REWRITE_IPV6_TWICE_NAT_UDP_REWRITE_ID;
                    if (res_flow->role == FLOW_ROLE_INITIATOR) {
                        flow_attrs->nat_l4_port = flow->config.nat_sport;
                    } else {
                        flow_attrs->nat_l4_port = flow->config.nat_dport;
                    }
                } else {
                    *rw_act = REWRITE_IPV6_TWICE_NAT_REWRITE_ID;
                }
            }
        }
    }

    flow->pgm_attrs.nat_type = res_flow->config.nat_type;
    if (res_flow->config.nat_type == NAT_TYPE_SNAT) {
        memcpy(&flow->pgm_attrs.nat_sip, 
                &flow->config.nat_sip, sizeof(ip_addr_t));
        flow->pgm_attrs.nat_sport = flow->config.nat_sport;
    } else if (res_flow->config.nat_type == NAT_TYPE_DNAT) {
        memcpy(&flow->pgm_attrs.nat_dip, 
                &flow->config.nat_dip, sizeof(ip_addr_t));
        flow->pgm_attrs.nat_dport = flow->config.nat_dport;
        flow->pgm_attrs.mac_da_rewrite = true;
    } else {
        // Twice NAT 
    }

end:
    // TODO: populate rw_idx instead of dep.
    // flow->pgm_attrs.dep = res_flow->dep;
    if (res_flow->config.nat_type == NAT_TYPE_SNAT && 
            (res_flow->sl2seg == res_flow->dl2seg)) { 
        // if SNAT and bridging, we have to get rw_idx from 
        //   l4lb. TODO: Cleanup to not get from l4lb but
        //   from mac_sa, mac_da: 0, rw_act
            flow->pgm_attrs.rw_idx = flow_get_l4lb_rw_idx(flow, *rw_act);
            flow->pgm_attrs.mac_sa_rewrite = true;
    } else {
        flow->pgm_attrs.rw_idx = hal::pd::ep_pd_get_rw_tbl_idx_from_pi_ep(res_flow->dep, 
                flow->pgm_attrs.rw_act);
    }
    flow->pgm_attrs.tnnl_rw_idx = hal::pd::ep_pd_get_tnnl_rw_tbl_idx_from_pi_ep(res_flow->dep,
            flow->pgm_attrs.tnnl_rw_act);

    return ret;
}

//------------------------------------------------------------------------------
// Updates flow parameters for ifflow 
//------------------------------------------------------------------------------
static inline hal_ret_t
update_iflow_forwarding_info(const session_args_t *args, session_t *session)
{
    hal_ret_t   ret = HAL_RET_OK;
    flow_t      *flow = NULL, *assoc_flow = NULL;
    l2seg_t     *sl2seg = NULL, *dl2seg = NULL;
    if_t        *dif = NULL;

    flow = session->iflow;
    assoc_flow = flow->assoc_flow;
    dif = flow->dif;
    sl2seg = flow->sl2seg;
    dl2seg = flow->dl2seg;

    flow->pgm_attrs.role = flow->role;

    if (flow->config.action == session::FLOW_ACTION_DROP) {
        // TODO: Please visit if assoc_flow will even be valid in this case
        flow->pgm_attrs.drop = 1;
        goto end;
    }

    if (assoc_flow) {
        assoc_flow->pgm_attrs.role = flow->role;
    }

    // check which of iflows will have rewrite info
    if (!flow->assoc_flow) {
        // No associated flow - no service
        update_encap_rw_info(dif, sl2seg, dl2seg, &flow->pgm_attrs); 
        flow_update_rewrite_info(flow);
    } else {
        if (flow->role == FLOW_ROLE_INITIATOR && 
                flow->src_type == FLOW_END_TYPE_P4PLUS &&
                (flow->dst_type == FLOW_END_TYPE_HOST ||
                 flow->dst_type == FLOW_END_TYPE_NETWORK)) {
            update_encap_rw_info(dif, sl2seg, dl2seg, &flow->pgm_attrs);
            flow_update_rewrite_info(flow);
        } else {
            // No rw for flow
            flow->pgm_attrs.rw_act = REWRITE_NOP_ID;
            flow->pgm_attrs.tnnl_rw_act = TUNNEL_REWRITE_NOP_ID;
        }
        if (assoc_flow->role == FLOW_ROLE_INITIATOR && 
                assoc_flow->src_type == FLOW_END_TYPE_P4PLUS &&
                (assoc_flow->dst_type == FLOW_END_TYPE_HOST ||
                 assoc_flow->dst_type == FLOW_END_TYPE_NETWORK)) {
            update_encap_rw_info(dif, sl2seg, dl2seg, &assoc_flow->pgm_attrs);
            flow_update_rewrite_info(assoc_flow);
        } else {
            // No rw for flow
            assoc_flow->pgm_attrs.rw_act = REWRITE_NOP_ID;
            assoc_flow->pgm_attrs.tnnl_rw_act = TUNNEL_REWRITE_NOP_ID;
        }
    }

end:
    HAL_TRACE_DEBUG("PI-Session:{} iflow: role:{}, drop:{}, mac_sa_rw:{}, mac_da_rw:{},"
            "rw_act:{}, rw_idx:{} tnnl_rw_act:{}, tnnl_vnid:{}, ttl_dec:{}, lport:{}, "
            "qid_en:{}, qtype:{}, qid:{}, nat_sip:{}, nat_dip:{}, nat_sport:{},"
            "nat_dport:{}, nat_l4_port:{}, mcast_en:{}",
            __FUNCTION__, flow->pgm_attrs.role, flow->pgm_attrs.drop,
            flow->pgm_attrs.mac_sa_rewrite, flow->pgm_attrs.mac_da_rewrite,
            flow->pgm_attrs.rw_act, flow->pgm_attrs.rw_idx, flow->pgm_attrs.tnnl_rw_act, 
            flow->pgm_attrs.tnnl_vnid, flow->pgm_attrs.ttl_dec, 
            flow->pgm_attrs.lport, flow->pgm_attrs.qid_en,
            flow->pgm_attrs.qtype, flow->pgm_attrs.qid, 
            ipaddr2str(&flow->pgm_attrs.nat_sip), 
            ipaddr2str(&flow->pgm_attrs.nat_dip), 
            flow->pgm_attrs.nat_sport, flow->pgm_attrs.nat_dport,
            flow->pgm_attrs.nat_l4_port, flow->pgm_attrs.mcast_en);

    if (flow->assoc_flow) {
        HAL_TRACE_DEBUG("PI-Session:{} iflow_assoc: role:{}, drop:{}, mac_sa_rw:{}, mac_da_rw:{},"
                "rw_act:{}, rw_idx:{}, tnnl_rw_act:{}, tnnl_vnid:{}, ttl_dec:{}, lport:{}, "
                "qid_en:{}, qtype:{}, qid:{}, nat_sip:{}, nat_dip:{}, nat_sport:{},"
                "nat_dport:{}, nat_l4_port:{}, mcast_en:{}",
                __FUNCTION__, assoc_flow->pgm_attrs.role, assoc_flow->pgm_attrs.drop,
                assoc_flow->pgm_attrs.mac_sa_rewrite, assoc_flow->pgm_attrs.mac_da_rewrite,
                assoc_flow->pgm_attrs.rw_act, assoc_flow->pgm_attrs.rw_idx, 
                assoc_flow->pgm_attrs.tnnl_rw_act, 
                assoc_flow->pgm_attrs.tnnl_vnid, assoc_flow->pgm_attrs.ttl_dec, 
                assoc_flow->pgm_attrs.lport, assoc_flow->pgm_attrs.qid_en,
                assoc_flow->pgm_attrs.qtype, assoc_flow->pgm_attrs.qid, 
                ipaddr2str(&assoc_flow->pgm_attrs.nat_sip), 
                ipaddr2str(&assoc_flow->pgm_attrs.nat_dip), 
                assoc_flow->pgm_attrs.nat_sport, assoc_flow->pgm_attrs.nat_dport,
                assoc_flow->pgm_attrs.nat_l4_port, assoc_flow->pgm_attrs.mcast_en);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Updates flow parameters for rflow 
//------------------------------------------------------------------------------
static inline hal_ret_t
update_rflow_forwarding_info(const session_args_t *args, session_t *session)
{
    hal_ret_t   ret = HAL_RET_OK;
    flow_t      *flow = NULL, *assoc_flow = NULL;
    l2seg_t     *sl2seg = NULL, *dl2seg = NULL;
    if_t        *dif = NULL;

    flow = session->rflow;
    assoc_flow = flow->assoc_flow;
    dif = flow->dif;
    sl2seg = flow->sl2seg;
    dl2seg = flow->dl2seg;

    flow->pgm_attrs.role = flow->role;
    if (flow->config.action == session::FLOW_ACTION_DROP) {
        // TODO: Please visit if assoc_flow will even be valid in this case
        flow->pgm_attrs.drop = 1;
        goto end;
    }

    if (assoc_flow) {
        assoc_flow->pgm_attrs.role = flow->role;
    }

    // check which of iflows will have rewrite info
    if (!flow->assoc_flow) {
        // No associated flow - no service
        update_encap_rw_info(dif, sl2seg, dl2seg, &flow->pgm_attrs);
        flow_update_rewrite_info(flow);
    } else {
        if (flow->role == FLOW_ROLE_RESPONDER && 
                flow->dst_type == FLOW_END_TYPE_P4PLUS &&
                (flow->src_type == FLOW_END_TYPE_HOST ||
                 flow->src_type == FLOW_END_TYPE_NETWORK)) {
            update_encap_rw_info(dif, sl2seg, dl2seg, &flow->pgm_attrs);
            flow_update_rewrite_info(flow);
        } else {
            // No rw for flow
            flow->pgm_attrs.rw_act = REWRITE_NOP_ID;
            flow->pgm_attrs.tnnl_rw_act = TUNNEL_REWRITE_NOP_ID;
        }
        if (assoc_flow->role == FLOW_ROLE_RESPONDER && 
                assoc_flow->dst_type == FLOW_END_TYPE_P4PLUS &&
                (assoc_flow->src_type == FLOW_END_TYPE_HOST ||
                 assoc_flow->src_type == FLOW_END_TYPE_NETWORK)) {
            update_encap_rw_info(dif, sl2seg, dl2seg, &assoc_flow->pgm_attrs);
            flow_update_rewrite_info(assoc_flow);
        } else {
            // No rw for flow
            assoc_flow->pgm_attrs.rw_act = REWRITE_NOP_ID;
            assoc_flow->pgm_attrs.tnnl_rw_act = TUNNEL_REWRITE_NOP_ID;
        }
    }

end:
    HAL_TRACE_DEBUG("PI-Session:{} rflow: role:{}, drop:{}, mac_sa_rw:{}, mac_da_rw:{},"
            "rw_act:{}, rw_idx:{}, tnnl_rw_act:{}, tnnl_vnid:{}, ttl_dec:{}, lport:{}, "
            "qid_en:{}, qtype:{}, qid:{}, nat_sip:{}, nat_dip:{}, nat_sport:{},"
            "nat_dport:{}, nat_l4_port:{}, mcast_en:{}",
            __FUNCTION__, flow->pgm_attrs.role, flow->pgm_attrs.drop,
            flow->pgm_attrs.mac_sa_rewrite, flow->pgm_attrs.mac_da_rewrite,
            flow->pgm_attrs.rw_act, flow->pgm_attrs.rw_idx, flow->pgm_attrs.tnnl_rw_act, 
            flow->pgm_attrs.tnnl_vnid, flow->pgm_attrs.ttl_dec, 
            flow->pgm_attrs.lport, flow->pgm_attrs.qid_en,
            flow->pgm_attrs.qtype, flow->pgm_attrs.qid, 
            ipaddr2str(&flow->pgm_attrs.nat_sip), 
            ipaddr2str(&flow->pgm_attrs.nat_dip), 
            flow->pgm_attrs.nat_sport, flow->pgm_attrs.nat_dport,
            flow->pgm_attrs.nat_l4_port, flow->pgm_attrs.mcast_en);

    if (flow->assoc_flow) {
        HAL_TRACE_DEBUG("PI-Session:{} rflow_assoc: role:{}, drop:{}, mac_sa_rw:{}, mac_da_rw:{},"
                "rw_act:{}, rw_idx:{}, tnnl_rw_act:{}, tnnl_vnid:{}, ttl_dec:{}, lport:{}, "
                "qid_en:{}, qtype:{}, qid:{}, nat_sip:{}, nat_dip:{}, nat_sport:{},"
                "nat_dport:{}, nat_l4_port:{}, mcast_en:{}",
                __FUNCTION__, assoc_flow->pgm_attrs.role, assoc_flow->pgm_attrs.drop,
                assoc_flow->pgm_attrs.mac_sa_rewrite, assoc_flow->pgm_attrs.mac_da_rewrite,
                assoc_flow->pgm_attrs.rw_act, assoc_flow->pgm_attrs.rw_idx, assoc_flow->pgm_attrs.tnnl_rw_act, 
                assoc_flow->pgm_attrs.tnnl_vnid, assoc_flow->pgm_attrs.ttl_dec, 
                assoc_flow->pgm_attrs.lport, assoc_flow->pgm_attrs.qid_en,
                assoc_flow->pgm_attrs.qtype, assoc_flow->pgm_attrs.qid, 
                ipaddr2str(&assoc_flow->pgm_attrs.nat_sip), 
                ipaddr2str(&assoc_flow->pgm_attrs.nat_dip), 
                assoc_flow->pgm_attrs.nat_sport, assoc_flow->pgm_attrs.nat_dport,
                assoc_flow->pgm_attrs.nat_l4_port, assoc_flow->pgm_attrs.mcast_en);
    }
    return ret;
}


//------------------------------------------------------------------------------
// Updates flow forwarding parameters for sessions
//------------------------------------------------------------------------------
static inline hal_ret_t
update_session_forwarding_info(const session_args_t *args, session_t *session)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = update_iflow_forwarding_info(args, session);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = update_rflow_forwarding_info(args, session);
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
    HAL_TRACE_DEBUG("Entering ADD session to DB:{}", session->hal_handle);


    session->session_id_ht_ctxt.reset();
    g_hal_state->session_id_ht()->insert(session,
                                         &session->session_id_ht_ctxt);

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
    // utils::dllist_add(&sif->session_list_head, &session->sif_session_lentry);
    HAL_SPINLOCK_UNLOCK(&sif->slock);

    if (sif != dif) {
        utils::dllist_reset(&session->dif_session_lentry);
        HAL_SPINLOCK_LOCK(&dif->slock);
        // utils::dllist_add(&dif->session_list_head,
        //                  &session->dif_session_lentry);
        HAL_SPINLOCK_UNLOCK(&dif->slock);
    }

    utils::dllist_reset(&session->tenant_session_lentry);
    HAL_SPINLOCK_LOCK(&tenant->slock);
    utils::dllist_add(&tenant->session_list_head,
                      &session->tenant_session_lentry);
    HAL_SPINLOCK_UNLOCK(&tenant->slock);

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Returns true if the flow needs an associate flow
//-----------------------------------------------------------------------------
bool
flow_needs_associate_flow (const flow_key_t *flow_key)
{
    // TODO: Query to check if the flow needs an associate flow.
    return false;
}

//-----------------------------------------------------------------------------
// Handle NAT config and Update flow structure for iflow 
//-----------------------------------------------------------------------------
hal_ret_t
process_iflow_for_lb(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;
    flow_t                  *iflow = NULL;
    flow_cfg_t              *flow_cfg = NULL;
    l2seg_t                 *dl2seg_nat = NULL;
    if_t                    *dif_nat = NULL;
    ep_t                    *dep_nat = NULL;

    iflow = session->iflow;
    flow_cfg = &iflow->config;
    
    HAL_TRACE_DEBUG("PI-Session:{} iflow: nat_type:{}",
            __FUNCTION__, flow_cfg->nat_type);

    // No-op if there is no DNAT or Twice NAT
    if (flow_cfg->nat_type != NAT_TYPE_DNAT && 
            flow_cfg->nat_type != NAT_TYPE_TWICE_NAT) {
        return HAL_RET_OK;
    }

    // change the dep, dl2seg and dif
    if (flow_cfg->nat_dip.af == IP_AF_IPV4) {
        dep_nat = find_ep_by_v4_key(session->tenant->tenant_id,
                                    flow_cfg->nat_dip.addr.v4_addr);
    } else {
        dep_nat = find_ep_by_v6_key(session->tenant->tenant_id, 
                                    &flow_cfg->nat_dip);
    }

    if (dep_nat == NULL) {
        HAL_TRACE_ERR("PI-Session:{} NAT EP not found", __FUNCTION__);
        args->rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
        return HAL_RET_EP_NOT_FOUND;
    }

    dl2seg_nat = find_l2seg_by_handle(dep_nat->l2seg_handle);
    HAL_ASSERT(dl2seg_nat != NULL);

    dif_nat = find_if_by_handle(dep_nat->if_handle);
    HAL_ASSERT(dif_nat != NULL);

    HAL_TRACE_DEBUG("PI-Session:{} iflow: DNAT/TNAT dep: {} -> {}, "
            "dif: {} -> {}, dl2seg: {} -> {}", __FUNCTION__,
            ep_l2_key_to_str(iflow->dep), ep_l2_key_to_str(dep_nat),
            iflow->dif ? iflow->dif->if_id : (uint32_t)-1, dif_nat->if_id,
            iflow->dl2seg ? iflow->dl2seg->seg_id : (uint32_t)-1, 
            dl2seg_nat->seg_id);

    iflow->dep = dep_nat;
    iflow->dif = dif_nat;
    iflow->dl2seg = dl2seg_nat;

    return ret;
}

//-----------------------------------------------------------------------------
// Handle NAT config and Update flow structure for rflow 
//-----------------------------------------------------------------------------
hal_ret_t
process_rflow_for_lb(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;
    flow_t                  *rflow = NULL;
    flow_cfg_t              *flow_cfg = NULL;
    l2seg_t                 *dl2seg_nat = NULL;
    if_t                    *dif_nat = NULL;
    ep_t                    *dep_nat = NULL;

    rflow = session->rflow;
    flow_cfg = &rflow->config;

    if (!rflow) {
        // No rflow, nothing to process
        return ret;
    }

    HAL_TRACE_DEBUG("PI-Session:{} rflow: nat_type:{}",
            __FUNCTION__, flow_cfg->nat_type);

    // No-op if there is no DNAT or Twice NAT
    if (flow_cfg->nat_type != NAT_TYPE_DNAT && 
            flow_cfg->nat_type != NAT_TYPE_TWICE_NAT) {
        return HAL_RET_OK;
    }

    // change the dep, dl2seg and dif
    if (flow_cfg->nat_dip.af == IP_AF_IPV4) {
        dep_nat = find_ep_by_v4_key(session->tenant->tenant_id,
                                    flow_cfg->nat_dip.addr.v4_addr);
    } else {
        dep_nat = find_ep_by_v6_key(session->tenant->tenant_id, 
                                    &flow_cfg->nat_dip);
    }

    if (dep_nat == NULL) {
        HAL_TRACE_ERR("PI-Session:{} NAT EP not found", __FUNCTION__);
        args->rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
        return HAL_RET_EP_NOT_FOUND;
    }

    dl2seg_nat = find_l2seg_by_handle(dep_nat->l2seg_handle);
    HAL_ASSERT(dl2seg_nat != NULL);

    dif_nat = find_if_by_handle(dep_nat->if_handle);
    HAL_ASSERT(dif_nat != NULL);

    HAL_TRACE_DEBUG("PI-Session:{} rflow: DNAT/TNAT dep: {} -> {}, "
            "dif: {} -> {}, dl2seg: {} -> {}",
            ep_l2_key_to_str(rflow->dep), ep_l2_key_to_str(dep_nat),
            rflow->dif ? rflow->dif->if_id : (uint32_t)-1, dif_nat->if_id,
            rflow->dl2seg ? rflow->dl2seg->seg_id : (uint32_t)-1, 
            dl2seg_nat->seg_id);

    rflow->dep = dep_nat;
    rflow->dif = dif_nat;
    rflow->dl2seg = dl2seg_nat;

    return ret;
}

//-----------------------------------------------------------------------------
// Process session for LB (NAT)
//-----------------------------------------------------------------------------
hal_ret_t 
process_session_for_lb(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;

    ret = process_iflow_for_lb(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Processing iflow for LB(NAT) failed.",
                __FUNCTION__);
        goto end;
    }


    ret = process_rflow_for_lb(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Processing rflow for LB(NAT) failed.",
                __FUNCTION__);
        goto end;
    }

end:
    return ret;
}


//-----------------------------------------------------------------------------
// Build flow structure for iflow
//-----------------------------------------------------------------------------
hal_ret_t
process_iflow_base_spec(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;
    flow_t                  *iflow = NULL;
    ep_t                    *sep = NULL, *dep = NULL;
    bool                    dnat_v = false;

    iflow = session->iflow;

    dnat_v = (iflow->config.nat_type == NAT_TYPE_DNAT ||
                iflow->config.nat_type == NAT_TYPE_TWICE_NAT);

    // get the src and dst EPs from the flow key
    if (ep_get_from_flow_key_spec(args->tenant->tenant_id, 
                                  args->spec->initiator_flow().flow_key(),
                                  &sep, &dep) != HAL_RET_OK) {
        if (sep == NULL) {
            HAL_TRACE_ERR("PI-Session:{} Source EP not found", __FUNCTION__);
            args->rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
            return HAL_RET_EP_NOT_FOUND;
        }
        if (dep == NULL && !dnat_v) {
            HAL_TRACE_ERR("PI-Session:{} Destination EP not found", __FUNCTION__);
            args->rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
            return HAL_RET_EP_NOT_FOUND;
        }
    }

    iflow->sep = sep;
    iflow->dep = dep;

    iflow->config.key.dir = 
        (sep->ep_flags & EP_FLAGS_LOCAL) ? FLOW_DIR_FROM_ENIC : 
        FLOW_DIR_FROM_UPLINK; 
    HAL_TRACE_DEBUG("iflow direction: {} sep_ep_flags: {}", iflow->config.key.dir, sep->ep_flags);

    // lookup ingress & egress interfaces
    // iflow->sif = (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&sep->if_handle);
    iflow->sif = find_if_by_handle(sep->if_handle);
    if (dep) {
        // iflow->dif = (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&dep->if_handle);
        iflow->dif = find_if_by_handle(dep->if_handle);
    }
    if ((iflow->sif == NULL) || (dep && iflow->dif == NULL)) {
        HAL_TRACE_ERR("Src/Dst interface not found");
        args->rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return  HAL_RET_IF_NOT_FOUND;
    }

    // lookup ingress & egress L2 segments
    iflow->sl2seg =
        find_l2seg_by_handle(sep->l2seg_handle);
        // (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&sep->l2seg_handle);
    if (dep) {
        iflow->dl2seg =
            find_l2seg_by_handle(dep->l2seg_handle);
            // (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&dep->l2seg_handle);
    }
    if ((iflow->sl2seg == NULL) || (dep && iflow->dl2seg == NULL)) {
        args->rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("PI-Session:{} iflow: sep:{}, dep:{}, sif_id:{}, "
            "dif_id:{}, sl2seg_id:{}, dl2seg_id:{}", __FUNCTION__,
            ep_l2_key_to_str(iflow->sep), ep_l2_key_to_str(iflow->dep),
            iflow->sif->if_id, iflow->dif ? iflow->dif->if_id : -1, 
            iflow->sl2seg->seg_id, iflow->dl2seg ? iflow->dl2seg->seg_id : -1);
    return ret;
}

//-----------------------------------------------------------------------------
// Build flow structure for rflow
//-----------------------------------------------------------------------------
hal_ret_t
process_rflow_base_spec(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;
    flow_t                  *rflow = NULL;
    ep_t                    *sep = NULL, *dep = NULL;
    bool                    dnat_v = false;

    rflow = session->rflow;
    if (!rflow) {
        // No rflow, nothing to process
        return ret;
    }

    dnat_v = (rflow->config.nat_type == NAT_TYPE_DNAT ||
                rflow->config.nat_type == NAT_TYPE_TWICE_NAT);

    // get the src and dst EPs from the flow key
    if (ep_get_from_flow_key_spec(args->tenant->tenant_id, 
                                  args->spec->responder_flow().flow_key(),
                                  &sep, &dep) != HAL_RET_OK) {
        if (sep == NULL) {
            HAL_TRACE_ERR("PI-Session:{} Source EP not found", __FUNCTION__);
            args->rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
            return HAL_RET_EP_NOT_FOUND;
        }
        if (dep == NULL && !dnat_v) {
            HAL_TRACE_ERR("PI-Session:{} Destination EP not found", __FUNCTION__);
            args->rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
            return HAL_RET_EP_NOT_FOUND;
        }
    }

    rflow->sep = sep;
    rflow->dep = dep;

    rflow->config.key.dir = 
        (sep->ep_flags & EP_FLAGS_LOCAL) ? FLOW_DIR_FROM_ENIC : 
        FLOW_DIR_FROM_UPLINK; 

    // lookup ingress & egress interfaces
    // rflow->sif = (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&sep->if_handle);
    rflow->sif = find_if_by_handle(sep->if_handle);
    if (dep) { 
        // rflow->dif = (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&dep->if_handle);
        rflow->dif = find_if_by_handle(dep->if_handle);
    }
    if ((rflow->sif == NULL) || (dep && rflow->dif == NULL)) {
        HAL_TRACE_ERR("Src/Dst interface not found");
        args->rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return  HAL_RET_IF_NOT_FOUND;
    }

    // lookup ingress & egress L2 segments
    rflow->sl2seg =
        find_l2seg_by_handle(sep->l2seg_handle);
        // (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&sep->l2seg_handle);
    if (dep) {
        rflow->dl2seg =
            find_l2seg_by_handle(dep->l2seg_handle);
            // (l2seg_t *)g_hal_state->l2seg_hal_handle_ht()->lookup(&dep->l2seg_handle);
    }
    if ((rflow->sl2seg == NULL) || (dep && rflow->dl2seg == NULL)) {
        args->rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("PI-Session:{} rflow: sep:{}, dep:{}, sif_id:{}, "
            "dif_id:{}, sl2seg_id:{}, dl2seg_id:{}", __FUNCTION__,
            ep_l2_key_to_str(rflow->sep), ep_l2_key_to_str(rflow->dep),
            rflow->sif->if_id, rflow->dif ? rflow->dif->if_id : -1, 
            rflow->sl2seg->seg_id, rflow->dl2seg ? rflow->dl2seg->seg_id : -1);

    return ret;
}

//-----------------------------------------------------------------------------
// Process base spec and populate flow
//-----------------------------------------------------------------------------
hal_ret_t 
process_session_base_spec(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;

    ret = process_iflow_base_spec(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Processing of base spec of iflow failed.",
                __FUNCTION__);
        goto end;
    }

    ret = process_rflow_base_spec(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Processing of base spec of rflow failed.",
                __FUNCTION__);
        goto end;
    }

end:
    return ret;
}



//-----------------------------------------------------------------------------
// Pre-Process IFlow
//-----------------------------------------------------------------------------
hal_ret_t 
pre_process_iflow(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;
    flow_t                  *iflow = NULL, *assoc_flow = NULL;
    if_t                    *sif = NULL, *dif = NULL;

    iflow = session->iflow;
    assoc_flow = iflow->assoc_flow;

    sif = iflow->sif;
    dif = iflow->dif;

    // set role
    iflow->role = FLOW_ROLE_INITIATOR;
    // set src_type
    if (sif->if_type == intf::IF_TYPE_ENIC) {
        iflow->src_type = FLOW_END_TYPE_HOST;
    } else {
        iflow->src_type = FLOW_END_TYPE_NETWORK;
    }

    // set dst_type
    if (assoc_flow) {
        // service is enabled
        iflow->dst_type = FLOW_END_TYPE_P4PLUS;
    } else {
        if (dif->if_type == intf::IF_TYPE_ENIC) {
            iflow->dst_type = FLOW_END_TYPE_HOST;
        } else {
            iflow->dst_type = FLOW_END_TYPE_NETWORK;
        }
    }

    // set role, src_type, dst_type for assoc flow
    if (assoc_flow) {
        assoc_flow->role = FLOW_ROLE_INITIATOR;
        assoc_flow->src_type = FLOW_END_TYPE_P4PLUS;
        if (dif->if_type == intf::IF_TYPE_ENIC) {
            assoc_flow->dst_type = FLOW_END_TYPE_HOST;
        } else {
            assoc_flow->dst_type = FLOW_END_TYPE_NETWORK;
        }
    }

    HAL_TRACE_DEBUG("PI-Session:{} iflow: role:{}, src_type:{}, dst_type:{}",
            __FUNCTION__, iflow->role, iflow->src_type, iflow->dst_type);
    if (assoc_flow) {
        HAL_TRACE_DEBUG("PI-Session:{} iflow_assoc: role:{}, src_type:{}, "
                "dst_type:{}",
                __FUNCTION__, assoc_flow->role, assoc_flow->src_type, 
                assoc_flow->dst_type);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Pre-Process RFlow
//-----------------------------------------------------------------------------
hal_ret_t 
pre_process_rflow(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;
    flow_t                  *rflow = NULL, *assoc_flow = NULL;
    if_t                    *sif = NULL, *dif = NULL;

    rflow = session->rflow;
    assoc_flow = rflow->assoc_flow;

    if (!rflow) {
        // No rflow, nothing to process
        return ret;
    }

    sif = rflow->sif;
    dif = rflow->dif;

    // set role
    rflow->role = FLOW_ROLE_RESPONDER;
    // set dst_type
    if (dif->if_type == intf::IF_TYPE_ENIC) {
        rflow->dst_type = FLOW_END_TYPE_HOST;
    } else {
        rflow->dst_type = FLOW_END_TYPE_NETWORK;;
    }

    // set src_type
    if (assoc_flow) {
        // service is enabled
        rflow->src_type = FLOW_END_TYPE_P4PLUS;
    } else {
        if (sif->if_type == intf::IF_TYPE_ENIC) {
            rflow->src_type = FLOW_END_TYPE_HOST;
        } else {
            rflow->src_type = FLOW_END_TYPE_NETWORK;
        }
    }

    // set role, src_type, dst_type for assoc flow
    if (assoc_flow) {
        assoc_flow->role = FLOW_ROLE_RESPONDER;
        assoc_flow->dst_type = FLOW_END_TYPE_P4PLUS;
        if (sif->if_type == intf::IF_TYPE_ENIC) {
            assoc_flow->src_type = FLOW_END_TYPE_HOST;
        } else {
            assoc_flow->src_type = FLOW_END_TYPE_NETWORK;
        }
    }
    
    HAL_TRACE_DEBUG("PI-Session:{} rflow: role:{}, src_type:{}, dst_type:{}",
            __FUNCTION__, rflow->role, rflow->src_type, rflow->dst_type);
    if (assoc_flow) {
    HAL_TRACE_DEBUG("PI-Session:{} rflow_assoc: role:{}, src_type:{}, "
            "dst_type:{}",
            __FUNCTION__, assoc_flow->role, assoc_flow->src_type, 
            assoc_flow->dst_type);
    }
    return ret;
}

//-----------------------------------------------------------------------------
// Pre Process Session
//-----------------------------------------------------------------------------
hal_ret_t 
pre_process_session(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;
    if_t                    *sif = NULL, *dif = NULL;

    sif = session->iflow->sif;
    dif = session->iflow->dif;

    if (sif->if_type == intf::IF_TYPE_ENIC) {
        session->src_dir = SESSION_DIR_H;
    } else {
        session->src_dir = SESSION_DIR_N;
    }

    if (dif->if_type == intf::IF_TYPE_ENIC) {
        session->dst_dir = SESSION_DIR_H;
    } else {
        session->dst_dir = SESSION_DIR_N;
    }

    HAL_TRACE_DEBUG("PI-Session:{} src_dir:{}, dst_dir:{}", __FUNCTION__,
            session->src_dir, session->dst_dir);

    return ret;
}

//-----------------------------------------------------------------------------
// Process Flows
//-----------------------------------------------------------------------------
hal_ret_t 
pre_process_flows(const session_args_t *args, session_t *session)
{
    hal_ret_t               ret = HAL_RET_OK;

    ret = pre_process_iflow(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Pre-processing of iflow failed.",
                __FUNCTION__);
        goto end;
    }

    ret = pre_process_rflow(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Pre-processing of rflow failed.",
                __FUNCTION__);
        goto end;
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Flow Create
//-----------------------------------------------------------------------------
static flow_t *
flow_create(const flow_cfg_t *cfg, session_t *session)
{
    flow_t      *assoc_flow = NULL;

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

    flow->assoc_flow = NULL;
    // Check if we have to create associated flow
    if (flow_needs_associate_flow(&cfg->key)) {
        assoc_flow = (flow_t *)g_hal_state->flow_slab()->alloc();
        if (!assoc_flow) {
            return NULL;
        }
        *assoc_flow = {};
        HAL_SPINLOCK_INIT(&assoc_flow->slock, PTHREAD_PROCESS_PRIVATE);
        assoc_flow->flow_key_ht_ctxt.reset();
        if (cfg) {
            assoc_flow->config = *cfg;
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
session_create (const session_args_t *args, hal_handle_t *session_handle)
{
    hal_ret_t ret;
    nwsec_profile_t         *nwsec_prof;
    pd::pd_session_args_t    pd_session_args;
    session_t               *session;

    // allocate a session
    session = (session_t *)g_hal_state->session_slab()->alloc();
    if (session == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    *session = {};
    session->config = *args->session;
    session->tenant = args->tenant;

    // fetch the security profile, if any
    if (args->tenant->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        nwsec_prof =
            find_nwsec_profile_by_handle(args->tenant->nwsec_profile_handle);
    } else {
        nwsec_prof = NULL;
    }

    // create flows
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
    session->hal_handle = hal_alloc_handle();

    // Process base specs to populate flows
    ret = process_session_base_spec(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Processing of base session spec failed.",
                __FUNCTION__);
        goto end;
    }

    // LB(NAT) processing
    ret = process_session_for_lb(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Processing session for LB (NAT)  failed.",
                __FUNCTION__);
        goto end;
    }

    // pre process flows
    ret = pre_process_flows(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Pre-processing of flows failed.",
                __FUNCTION__);
        goto end;
    }

    // pre process session
    ret = pre_process_session(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Pre-processing of session failed.",
                __FUNCTION__);
        goto end;
    }

    // populate forwarding info for the 
    ret = update_session_forwarding_info(args, session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Session:{}: Pre-processing of session failed.",
                __FUNCTION__);
        goto end;
    }

    // allocate all PD resources and finish programming, if any
    pd::pd_session_args_init(&pd_session_args);
    pd_session_args.tenant = args->tenant;
    pd_session_args.nwsec_prof = nwsec_prof;
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;
    pd_session_args.rsp = args->rsp;

    ret = pd::pd_session_create(&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session create failure, err : {}", ret);
        goto end;
    }

    // add this session to our db
    add_session_to_db(session->tenant, session->iflow->sl2seg, session->iflow->dl2seg,
                      session->iflow->sep, session->iflow->dep, session->iflow->sif, 
                      session->iflow->dif, session);
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
    session_state_t         session_state = {};
    uint8_t                 ingress, egress;

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
    args.tenant = tenant_lookup_by_id(tid);
    if (args.tenant == NULL) {
        rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        HAL_TRACE_ERR("{}: tenant {} not found", __func__, tid);
        return HAL_RET_INVALID_ARG;
    }
    args.iflow = &iflow;
    if (spec.has_responder_flow()) {
        args.rflow = &rflow;
    }

    // extract initiator's flow key and data from flow spec
    extract_flow_key_from_spec(tid, &args.iflow->key, spec.initiator_flow());
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
                          spec.initiator_flow().flow_data());
        session_state.iflow_state.syn_ack_delta = spec.iflow_syn_ack_delta();
    }

    // extract responder's flow key and data from flow spec
    if (args.rflow) {
        extract_flow_key_from_spec(tid, &args.rflow->key, spec.responder_flow());
        ret = extract_flow_info_from_spec(args.rflow, FALSE, spec.responder_flow());
        if (ret != HAL_RET_OK) {
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return ret;
        }
        if (session.conn_track_en) {
            extract_session_state_from_spec(args.session_state, FALSE,
                          spec.responder_flow().flow_data());
            session_state.rflow_state.syn_ack_delta = spec.iflow_syn_ack_delta();
        }
    }

    ret = extract_mirror_sessions(spec.initiator_flow(), &ingress, &egress);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("session create failure extracting mirror sessions: {}", __FUNCTION__);
        return ret;
    }
    args.iflow->ing_mirror_session = ingress;
    args.iflow->eg_mirror_session = egress;

    if (args.rflow) {
        ret = extract_mirror_sessions(spec.responder_flow(), &ingress, &egress);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("session create failure extracting mirror sessions: {}", __FUNCTION__);
            return ret;
         }
         args.iflow->ing_mirror_session = ingress;
         args.iflow->eg_mirror_session = egress;
    }

    args.spec = &spec;
    args.rsp = rsp;
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


//-----------------------------------------------------------------------------
// Flow Create FTE
//-----------------------------------------------------------------------------
static flow_t *
flow_create_fte(const flow_cfg_t *cfg,
                const flow_cfg_t *cfg_assoc,
                const flow_pgm_attrs_t *attrs,
                const flow_pgm_attrs_t *attrs_assoc,
                session_t *session, bool bridged)
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

//-----------------------------------------------------------------------------
// Flow Update FTE
//-----------------------------------------------------------------------------
static hal_ret_t
flow_update_fte(flow_t *flow, const flow_cfg_t *cfg,
                const flow_pgm_attrs_t *attrs)
{
    hal_ret_t      ret = HAL_RET_OK;

    if (cfg) {
        flow->config = *cfg;
    }

    if (attrs) {
        flow->pgm_attrs = *attrs;
    }

    return ret;
}

hal_ret_t
session_create_fte(const session_args_fte_t *args, hal_handle_t *session_handle)
{
    hal_ret_t ret;
    nwsec_profile_t         *nwsec_prof;
    pd::pd_session_args_t    pd_session_args;
    session_t               *session;
    bool bridged = (args->sl2seg == args->dl2seg);

    HAL_ASSERT(args->tenant && args->iflow && args->iflow_attrs &&args->sep && args->dep &&
               args->sif && args->dif && args->sl2seg && args->dl2seg);

    // allocate a session
    session = (session_t *)g_hal_state->session_slab()->alloc();
    if (session == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    *session = {};
    session->config = *args->session;
    session->tenant = args->tenant;

    // fetch the security profile, if any
    if (args->tenant->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        nwsec_prof =
            find_nwsec_profile_by_handle(args->tenant->nwsec_profile_handle);
    } else {
        nwsec_prof = NULL;
    }

    // Handle the spec info not handled in the FTE
    // TODO(goli) all these should go to appropriate fte features
    if (args->spec) {
        uint8_t  ingress, egress;
        qos_extract_action_from_spec(&args->iflow[0]->in_qos_action, 
                                     args->spec->initiator_flow().flow_data().flow_info().in_qos_actions(),
                                     hal::INGRESS_QOS);
        qos_extract_action_from_spec(&args->iflow[0]->eg_qos_action, 
                                     args->spec->initiator_flow().flow_data().flow_info().eg_qos_actions(),
                                     hal::EGRESS_QOS);
        ret = extract_mirror_sessions(args->spec->initiator_flow(), &ingress, &egress);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("session create failure extracting mirror sessions: {}", __FUNCTION__);
            return ret;
        }
        args->iflow[0]->ing_mirror_session = ingress;
        args->iflow[0]->eg_mirror_session = egress;

        if(args->rflow[0] && args->spec->has_responder_flow()) {
            qos_extract_action_from_spec(&args->rflow[0]->in_qos_action, 
                                         args->spec->responder_flow().flow_data().flow_info().in_qos_actions(),
                                         hal::INGRESS_QOS);
            qos_extract_action_from_spec(&args->rflow[0]->eg_qos_action, 
                                         args->spec->responder_flow().flow_data().flow_info().eg_qos_actions(),
                                         hal::EGRESS_QOS);
            ret = extract_mirror_sessions(args->spec->responder_flow(), &ingress, &egress);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("session create failure extracting mirror sessions: {}", __FUNCTION__);
                return ret;
            }
            args->rflow[0]->ing_mirror_session = ingress;
            args->rflow[0]->eg_mirror_session = egress;
        }
    }


    // create flows
    session->iflow = flow_create_fte(args->iflow[0], args->iflow[1],
                                     args->iflow_attrs[0], args->iflow_attrs[1],
                                     session, bridged);
    if (session->iflow == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    session->iflow->sl2seg = args->sl2seg;
    session->iflow->dl2seg = args->dl2seg;
    session->iflow->sep = args->sep;
    session->iflow->dep = args->dep;
    session->iflow->sif = args->sif;
    session->iflow->dif = args->dif;

    if (args->rflow) {
        session->rflow = flow_create_fte(args->rflow[0], args->rflow[1],
                                         args->rflow_attrs[0], args->rflow_attrs[1],
                                         session, bridged);
        if (session->rflow == NULL) {
            ret = HAL_RET_OOM;
            goto end;
        }

        session->rflow->sl2seg = args->dl2seg;
        session->rflow->dl2seg = args->sl2seg;
        session->rflow->sep = args->dep;
        session->rflow->dep = args->sep;
        session->rflow->sif = args->dif;
        session->rflow->dif = args->sif;

        session->iflow->reverse_flow = session->rflow;
        session->rflow->reverse_flow = session->iflow;
    }
    session->hal_handle = hal_alloc_handle();
    session->alg_proto_state = args->alg_proto_state;

    // allocate all PD resources and finish programming, if any
    pd::pd_session_args_init(&pd_session_args);
    pd_session_args.tenant = args->tenant;
    pd_session_args.nwsec_prof = nwsec_prof;
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;
    pd_session_args.rsp = args->rsp;
    pd_session_args.pgm_rflow = args->pgm_rflow;

    ret = pd::pd_session_create(&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session create failure, err : {}", ret);
        goto end;
    }

    // add this session to our db
    add_session_to_db(session->tenant, args->sl2seg, args->dl2seg,
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

session_t *
session_lookup_fte(flow_key_t key, flow_role_t *role)
{
    session_t *session = NULL;

    // Should we look at iflow first ?
    session = (session_t *)g_hal_state->session_hal_rflow_ht()->lookup(std::addressof(key));   
    *role = FLOW_ROLE_RESPONDER; 
    if (session == NULL) { 
        session = (session_t *)g_hal_state->session_hal_iflow_ht()->lookup(std::addressof(key));
        *role = FLOW_ROLE_INITIATOR;
    }

    return session;        
}

hal_ret_t
session_update_fte(const session_args_fte_t *args, session_t *session) 
{
    hal_ret_t                ret;
    pd::pd_session_args_t    pd_session_args;

    // Update PI Flows
    if (args->pgm_rflow) {
        ret = flow_update_fte(session->rflow, args->rflow[0], args->rflow_attrs[0]);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Flow update failure, err : {}", ret);
            return ret;
        }
    }
    
    // allocate all PD resources and finish programming, if any
    pd::pd_session_args_init(&pd_session_args);
    pd_session_args.tenant = args->tenant;
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;
    pd_session_args.rsp = args->rsp;
    pd_session_args.pgm_rflow = args->pgm_rflow;

    ret = pd::pd_session_update(&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session update failure, err : {}", ret);
    }
    
    return ret;
}

hal_ret_t
session_delete_fte(const session_args_fte_t *args, session_t *session)
{
    hal_ret_t                ret;
    pd::pd_session_args_t    pd_session_args;

    // allocate all PD resources and finish programming, if any
    pd::pd_session_args_init(&pd_session_args);
    pd_session_args.tenant = args->tenant;
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;
    pd_session_args.rsp = args->rsp;
    pd_session_args.pgm_rflow = args->pgm_rflow;

    ret = pd::pd_session_delete(&pd_session_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session delete failure, err : {}", ret);
        session_cleanup(session);
    }
    session_cleanup(session);

    return ret;
}

}    // namespace hal

