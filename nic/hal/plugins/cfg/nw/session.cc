//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <string>
#include <iostream>
#include <sstream>
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint_api.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/include/fte.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec_group.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/sdk/platform/capri/capri_hbm_rw.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "gen/proto/ftestats/ftestats.delphi.hpp"
#include "gen/proto/flowstats/flowstats.delphi.hpp"

using telemetry::MirrorSessionSpec;
using session::FlowInfo;
using session::FlowKeyTcpUdpInfo;
using session::FlowKeyICMPInfo;
using session::FlowKeyESPInfo;
using session::FlowData;
using session::ConnTrackInfo;
using session::FlowStats;
using session::ConnTrackExceptions;
using session::FlowInstance;
using types::FlowDirection;
using sys::FTEStats;
using sys::FTEFeatureStats;
using sys::FTEError;
using sys::FTEStatsInfo;
using sys::SystemResponse;
using sys::SessionSummaryStats;
using sys::PMDStats;
using sys::FTEInfo;
using sys::QCtr;
using sys::QGlobalInfo;
using sys::QInfo;

using namespace sdk::lib;

#define	HAL_FLOW_TELEMETRY_MAX_STATS_STATE	65536
#define	FLOW_TELEMETRY_STATS_SHIFT		7

#define TCP_IPV4_DOT1Q_PKT_SZ (sizeof(ether_header_t)+sizeof(vlan_header_t)+\
                              sizeof(ipv4_header_t)+sizeof(tcp_header_t)+TCP_TS_OPTION_LEN)
#define TCP_IPV4_PKT_SZ (sizeof(ether_header_t)+\
                         sizeof(ipv4_header_t)+sizeof(tcp_header_t))
#define TIME_DIFF(val1, val2) ((val1 <= val2)?(val2-val1):(val1-val2))

sdk::lib::indexer      *g_flow_proto_state_indexer;
sdk::types::mem_addr_t  g_flow_telemetry_hbm_start;

namespace hal {

std::ostream& operator<<(std::ostream& os, const hal::flow_state_t& val) {
    os << "{state=" << val.state;
    if (val.syn_ack_delta) {
        os << ", syn_ack_delta=" << val.syn_ack_delta;
    }
    if (val.tcp_seq_num) {
        os << ", tcp_seq_num=" << val.tcp_seq_num;
    }
    if (val.tcp_ack_num) {
        os << ", tcp_ack_num=" << val.tcp_ack_num;
    }
    return os << "}";
}

thread_local void *t_session_timer;
session_stats_t  *g_session_stats;

flow_telemetry_state_t *g_flow_telemetry_state_age_head_p; 
flow_telemetry_state_t *g_flow_telemetry_state_age_tail_p;
uint16_t                g_age_timer_ticks;

#define SESSION_SW_DEFAULT_TIMEOUT                 (3600)
#define SESSION_SW_DEFAULT_TCP_HALF_CLOSED_TIMEOUT (120 * TIME_MSECS_PER_SEC)
#define SESSION_SW_DEFAULT_TCP_CLOSE_TIMEOUT       (15 * TIME_MSECS_PER_SEC)
#define SESSION_SW_DEFAULT_TCP_CXNSETUP_TIMEOUT    (15 * TIME_MSECS_PER_SEC)
#define SESSION_DEFAULT_TCP_TICKLE_TIMEOUT         (3 * TIME_MSECS_PER_SEC)
#define HAL_SESSION_AGE_SCAN_INTVL                 (1000)
#define HAL_FTE_STATS_TIMER_INTVL                  (10 * TIME_MSECS_PER_SEC)
#define HAL_FTE_STATS_TIMER_INTVL_SECS             (10)
#define HAL_SESSIONS_TO_SCAN_PER_INTVL             (2000)
#define HAL_TCP_CLOSE_WAIT_INTVL                   (10 * TIME_MSECS_PER_SEC)
#define MAX_TCP_TICKLES                             3
#define HAL_MAX_SESSION_PER_ENQ                     128 
#define HAL_MAX_DATA_THREAD                        (g_hal_state->oper_db()->max_data_threads())
#define HAL_MAX_ERRORS                              255
#define HAL_SESSION_STATS_SHIFT                     7
#define HAL_SESSION_STATS_PTR(fte)                 (g_session_stats + (fte << HAL_SESSION_STATS_SHIFT))
#define SESSION_GET_STREAM_COUNT                   (250)

void *
session_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->hal_handle);
}

uint32_t
session_handle_key_size ()
{
    return sizeof(hal_handle_t);
}

void *
session_get_iflow_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->iflow->config.key);
}

uint32_t
session_iflow_key_size ()
{
    return sizeof(flow_key_t);
}

void *
session_get_rflow_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((session_t *)entry)->rflow->config.key);
}

uint32_t
session_rflow_key_size ()
{
    return sizeof(flow_key_t);
}

void *
flow_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((flow_t *)entry)->config.key);
}

uint32_t
flow_key_size ()
{
    return sizeof(flow_key_t);
}

session_t *
find_session_by_handle (hal_handle_t handle)
{
    return (session_t *)g_hal_state->session_hal_handle_ht()->lookup(&handle);
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

    if ( key.svrf_id == key.dvrf_id) {
        out.write("svrf={}, ", key.svrf_id);
    } else {
        out.write("svrf={}, dvrf={} ", key.svrf_id, key.dvrf_id);
    }

    switch (key.flow_type) {
    case FLOW_TYPE_L2:
        out.write("l2seg={}, smac={}, dmac={} etype={}",
                  key.l2seg_id, macaddr2str(key.smac),
                  macaddr2str(key.dmac), key.ether_type);
        break;

    case FLOW_TYPE_V4:
    case FLOW_TYPE_V6:
        if (key.flow_type == FLOW_TYPE_V4) {
            out.write("sip={}, dip={}, ", ipv4addr2str(key.sip.v4_addr),
                      ipv4addr2str(key.dip.v4_addr));
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
            out.write("proto=esp, spi={}", key.spi);
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

//------------------------------------------------------------------------------
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
        uint32_t id = flinfo.ing_mirror_sessions(i).mirrorsession_id();
        if (id > 7) {
            return HAL_RET_INVALID_ARG;
        }
        *ingress = *ingress | (1 << id);
        HAL_TRACE_DEBUG("  Adding ingress session {}", id);
    }
    for (i = 0; i < flinfo.egr_mirror_sessions_size(); ++i) {
        uint32_t id = flinfo.egr_mirror_sessions(i).mirrorsession_id();
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
extract_flow_key_from_spec (vrf_id_t tid, flow_key_t *key,
                            const FlowKey& flow_spec_key)
{
    key->svrf_id = key->dvrf_id = tid;

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
        l3key.ip_addr.af = key->flow_type == FLOW_TYPE_V4 ? IP_AF_IPV4 : IP_AF_IPV6;

        l3key.vrf_id = key->svrf_id;
        l3key.ip_addr.addr = key->sip;
        *sep = find_ep_by_l3_key(&l3key);

        l3key.vrf_id = key->dvrf_id;
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
add_session_to_db (hal_handle_t sep_handle, hal_handle_t dep_handle,
                   session_t *session)
{
    ep_t *sep = NULL, *dep = NULL;

    //session->session_id_ht_ctxt.reset();
    //g_hal_state->session_id_ht()->insert(session,
                                         //&session->session_id_ht_ctxt);
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

    sep = find_ep_by_handle(sep_handle);
    dep = find_ep_by_handle(dep_handle);

    if (sep) {
        ep_add_session(sep, session);
        session->sep_handle = sep->hal_handle;
    }
    if (dep) {
        ep_add_session(dep, session);
        session->dep_handle = dep->hal_handle;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// remove this session from all meta data structures
//------------------------------------------------------------------------------
static inline void
del_session_from_db (hal_handle_t sep_handle, hal_handle_t dep_handle, session_t *session)
{
    ep_t *sep = NULL, *dep = NULL;

    HAL_TRACE_DEBUG("Entering DEL session from DB:{}", session->hal_handle);

    sep = find_ep_by_handle(session->sep_handle);
    dep = find_ep_by_handle(session->dep_handle);

    if (sep)
        ep_del_session(sep, session);

    if (dep)
        ep_del_session(dep, session);

    g_hal_state->session_hal_handle_ht()->remove_entry(session,
                                                 &session->hal_handle_ht_ctxt);

    g_hal_state->session_hal_iflow_ht()->remove_entry(session,
                                                &session->hal_iflow_ht_ctxt);

    if (session->rflow) {
        g_hal_state->session_hal_rflow_ht()->remove_entry(session,
                                                    &session->hal_rflow_ht_ctxt);
    }
}

//------------------------------------------------------------------------------
// Get session idle timeout based on the flow key
// Use a default timeout in case security profile is not found
//------------------------------------------------------------------------------
static uint64_t
session_aging_timeout (session_t *session,
                       flow_t *iflow, flow_t *rflow)
{
    uint64_t            timeout = SESSION_SW_DEFAULT_TIMEOUT;
    vrf_t              *vrf = NULL;
    nwsec_profile_t    *nwsec_prof = NULL;

    if (session->idle_timeout != HAL_MAX_INACTIVTY_TIMEOUT) {
        return ((uint64_t)(session->idle_timeout * TIME_NSECS_PER_SEC));
    }

    vrf = vrf_lookup_by_handle(session->vrf_handle);
    if (vrf != NULL && vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        nwsec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
    } else {
        // Get the default security profile
        nwsec_prof = find_nwsec_profile_by_handle(
                        g_hal_state->oper_db()->default_security_profile_hdl());
    }

    if (nwsec_prof != NULL) {
        switch (iflow->config.key.proto) {
            case IPPROTO_TCP:
                if (iflow->pgm_attrs.drop) {
                    timeout = nwsec_prof->tcp_drop_timeout;
                } else {
                    timeout = nwsec_prof->tcp_timeout;
                }
                break;

            case IPPROTO_UDP:
                if (iflow->pgm_attrs.drop) {
                    timeout = nwsec_prof->udp_drop_timeout;
                } else {
                    timeout = nwsec_prof->udp_timeout;
                }
                break;

            case IPPROTO_ICMP:
                if (iflow->pgm_attrs.drop) {
                    timeout = nwsec_prof->icmp_drop_timeout;
                } else {
                    timeout = nwsec_prof->icmp_timeout;
                }
                break;

            default:
                if (iflow->pgm_attrs.drop) {
                    timeout = nwsec_prof->drop_timeout;
                 } else {
                    timeout = nwsec_prof->session_idle_timeout;
                 }
        }
    }

    return ((uint64_t)(timeout * TIME_NSECS_PER_SEC));
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
flow_esp_to_flow_esp_spec(flow_t *flow, FlowKeyESPInfo *esp)
{
    esp->set_spi(flow->config.key.spi);
}

static void
flow_data_to_flow_data_spec(flow_t *flow, FlowData *flow_data)
{
    FlowInfo             *flow_info = flow_data->mutable_flow_info();

    flow_info->set_flow_action((session::FlowAction)(flow->config.action));
    flow_info->set_nat_type((session::NatType)(flow->config.nat_type));
    ip_addr_to_spec(flow_info->mutable_nat_sip(), &flow->config.nat_sip);
    ip_addr_to_spec(flow_info->mutable_nat_dip(), &flow->config.nat_dip);
    flow_info->set_nat_sport(flow->config.nat_sport);
    flow_info->set_nat_dport(flow->config.nat_dport);
}

static void
flow_state_to_connection_track_response(flow_state_t *flow, ConnTrackInfo *conn_info)
{
    ConnTrackExceptions  *exceptions = conn_info->mutable_exceptions();

    // Set Connection Tracking info
    conn_info->set_flow_create_ts(flow->create_ts);
    conn_info->set_flow_packets(flow->packets);
    conn_info->set_flow_bytes(flow->bytes);
    conn_info->set_tcp_seq_num(flow->tcp_seq_num);
    conn_info->set_tcp_ack_num(flow->tcp_ack_num);
    conn_info->set_tcp_win_sz(flow->tcp_win_sz);
    conn_info->set_tcp_win_scale(flow->tcp_win_scale);
    conn_info->set_tcp_mss(flow->tcp_mss);
    conn_info->set_tcp_sack_perm_option_sent(flow->tcp_sack_perm_option_sent);
    conn_info->set_iflow_syn_ack_delta(flow->syn_ack_delta);
    conn_info->set_tcp_ws_option_sent(flow->tcp_ws_option_sent);
    conn_info->set_tcp_ts_option_sent(flow->tcp_ts_option_sent);

    // Set Connection Tracking Exceptions
    exceptions->set_tcp_syn_retransmit(flow->exception_bmap&TCP_SYN_REXMIT);
    exceptions->set_tcp_win_zero_drop(flow->exception_bmap&TCP_WIN_ZERO_DROP);
    exceptions->set_tcp_full_retransmit(flow->exception_bmap&TCP_FULL_REXMIT);
    exceptions->set_tcp_partial_overlap(flow->exception_bmap&TCP_PARTIAL_OVERLAP);
    exceptions->set_tcp_packet_reorder(flow->exception_bmap&TCP_PACKET_REORDER);
    exceptions->set_tcp_out_of_window(flow->exception_bmap&TCP_OUT_OF_WINDOW);
    exceptions->set_tcp_invalid_ack_num(flow->exception_bmap&TCP_ACK_ERR);
    exceptions->set_tcp_normalization_drop(flow->exception_bmap&TCP_NORMALIZATION_DROP);
    exceptions->set_tcp_split_handshake_detected(flow->exception_bmap&TCP_SPLIT_HANDSHAKE_DETECTED);
    exceptions->set_tcp_data_after_fin(flow->exception_bmap&TCP_DATA_AFTER_FIN);
    exceptions->set_tcp_non_rst_pkt_after_rst(flow->exception_bmap&TCP_NON_RST_PKT_AFTER_RST);
    exceptions->set_tcp_invalid_first_pkt_from_responder(flow->exception_bmap&TCP_INVALID_RESPONDER_FIRST_PKT);
    exceptions->set_tcp_unexpected_pkt(flow->exception_bmap&TCP_UNEXPECTED_PKT);
    exceptions->set_tcp_rst_with_invalid_ack_num(flow->exception_bmap&TCP_RST_WITH_INVALID_ACK_NUM);
}

static void
flow_to_flow_resp(flow_t *flow, FlowSpec *spec, FlowStatus *status)
{
    status->set_flow_direction((flow->config.key.dir == FLOW_DIR_FROM_UPLINK) ?
                               types::FLOW_DIRECTION_FROM_UPLINK : types::FLOW_DIRECTION_FROM_HOST);
    status->set_flow_instance((flow->pgm_attrs.lkp_inst == 0) ?
                               session::FLOW_INSTANCE_PRIMARY : session::FLOW_INSTANCE_SECONDARY);

    spec->mutable_flow_key()->set_src_vrf_id(flow->config.key.svrf_id);
    spec->mutable_flow_key()->set_dst_vrf_id(flow->config.key.dvrf_id);

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
        } else if (flow->config.key.proto == types::IPPROTO_ESP) {
            flow_esp_to_flow_esp_spec(flow, v4_key->mutable_esp());
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

    response->mutable_status()->set_session_handle(session->hal_handle);
    response->mutable_spec()->mutable_vrf_key_handle()->set_vrf_id(vrf->vrf_id);
    response->mutable_spec()->set_conn_track_en(session->conn_track_en);

    flow_to_flow_resp(session->iflow,
                      response->mutable_spec()->mutable_initiator_flow(),
                      response->mutable_status()->mutable_iflow_status());
    if (session->iflow->assoc_flow) {
        flow_to_flow_resp(session->iflow->assoc_flow,
                          response->mutable_spec()->mutable_peer_initiator_flow(),
                          response->mutable_status()->mutable_peer_iflow_status());
    }

    if (session->rflow) {
        HAL_TRACE_VERBOSE("valid rflow session");
        flow_to_flow_resp(session->rflow,
                          response->mutable_spec()->mutable_responder_flow(),
                          response->mutable_status()->mutable_rflow_status());
        if (session->rflow->assoc_flow) {
            flow_to_flow_resp(session->rflow->assoc_flow,
                              response->mutable_spec()->mutable_peer_responder_flow(),
                              response->mutable_status()->mutable_peer_rflow_status());
        }
    }
}

static void
flow_state_to_flow_stats_response (flow_stats_t *flow_stats,
                                   flow_state_t *flow_state,
                                   FlowStats *stats)
{
    stats->set_flow_permitted_packets(flow_state->packets);
    stats->set_flow_permitted_bytes(flow_state->bytes);
    stats->set_flow_dropped_packets(flow_state->drop_packets);
    stats->set_flow_dropped_bytes(flow_state->drop_bytes);
    stats->set_num_tcp_tickles_sent(flow_stats->num_tcp_tickles_sent);
    stats->set_num_tcp_rst_sent(flow_stats->num_tcp_rst_sent);
    return;
}

static void
session_state_to_session_get_response (session_t *session,
                                       session_state_t *session_state,
                                       SessionGetResponse *response)
{
    uint32_t        age;
    timespec_t      ctime;
    uint64_t        ctime_ns, create_ns;
    uint64_t        session_timeout=0, time_elapsed=0, time_remaining=0;

    // get current time
    clock_gettime(CLOCK_MONOTONIC, &ctime);
    sdk::timestamp_to_nsecs(&ctime, &ctime_ns);

    response->mutable_spec()->set_tcp_ts_option(session_state->tcp_ts_option);
    response->mutable_spec()->set_tcp_sack_perm_option(session_state->tcp_sack_perm_option);

    // iflow_age
    create_ns = session_state->iflow_state.create_ts;
    age = TIME_DIFF(ctime_ns, create_ns) / TIME_NSECS_PER_SEC;
    response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_flow_age(age);

    response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_last_packet_seen_time(session_state->iflow_state.last_pkt_ts);

    // Flow remaining inactivity timeout
    session_timeout = session_aging_timeout(session, session->iflow, session->rflow);
    if (!session_timeout) {
        response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_time_to_age(HAL_MAX_INACTIVTY_TIMEOUT);
    } else {
        time_elapsed = TIME_DIFF(ctime_ns, session_state->iflow_state.last_pkt_ts);
        time_remaining = (session_timeout > time_elapsed)?\
                         ((session_timeout-time_elapsed)/TIME_NSECS_PER_SEC):0;
        response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
            mutable_flow_info()->set_time_to_age(time_remaining);
    }

    // TCP state
    response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_tcp_state(session_state->iflow_state.state);
    response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_hal_tcp_state(session->iflow->state);

    response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_source_lif_check_enable(
                (session->iflow->pgm_attrs.expected_src_lif_en)?true:false);

    if (session->iflow->pgm_attrs.expected_src_lif_en) {
        response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_expected_source_lif(
                   session->iflow->pgm_attrs.expected_src_lif);
    }

    flow_state_to_flow_stats_response(&session->iflow->stats, &session_state->iflow_state,
         response->mutable_stats()->mutable_initiator_flow_stats());

    flow_state_to_connection_track_response(&session_state->iflow_state,
         response->mutable_spec()->mutable_initiator_flow()->mutable_flow_data()->\
         mutable_conn_track_info());

    if (session->rflow) {
        HAL_TRACE_VERBOSE("valid rflow session");
        // rflow age
        create_ns = session_state->rflow_state.create_ts;
        age = TIME_DIFF(ctime_ns, create_ns) / TIME_NSECS_PER_SEC;
        response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_flow_age(age);
        response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
           mutable_flow_info()->set_last_packet_seen_time(session_state->rflow_state.last_pkt_ts);
        // Flow remaining inactivity timeout
        if (!session_timeout) {
            response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_time_to_age(HAL_MAX_INACTIVTY_TIMEOUT);
        } else {
            time_elapsed = TIME_DIFF(ctime_ns, session_state->rflow_state.last_pkt_ts);
            time_remaining = (session_timeout > time_elapsed)?
                              ((session_timeout-time_elapsed)/TIME_NSECS_PER_SEC):0;
            response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
                 mutable_flow_info()->set_time_to_age(time_remaining);
        }

        // TCP state
        response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_tcp_state(session_state->rflow_state.state);
        response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_hal_tcp_state(session->rflow->state);

        response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_source_lif_check_enable(
                (session->rflow->pgm_attrs.expected_src_lif_en)?true:false);

        if (session->rflow->pgm_attrs.expected_src_lif_en) {
            response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
                 mutable_flow_info()->set_expected_source_lif(
                           session->rflow->pgm_attrs.expected_src_lif);
        }

        flow_state_to_flow_stats_response(&session->rflow->stats, &session_state->rflow_state,
                response->mutable_stats()->mutable_responder_flow_stats());

        flow_state_to_connection_track_response(&session_state->rflow_state,
           response->mutable_spec()->mutable_responder_flow()->mutable_flow_data()->\
           mutable_conn_track_info());
    }

    if (session->iflow->assoc_flow) {
        HAL_TRACE_VERBOSE("valid iflow aug session");
        // aug iflow age
        create_ns = session_state->iflow_aug_state.create_ts;
        age = TIME_DIFF(ctime_ns, create_ns) / TIME_NSECS_PER_SEC;
        response->mutable_spec()->mutable_peer_initiator_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_flow_age(age);

        flow_state_to_flow_stats_response(&session->iflow->assoc_flow->stats,
                &session_state->iflow_aug_state,
                response->mutable_stats()->mutable_peer_initiator_flow_stats());

        flow_state_to_connection_track_response(&session_state->iflow_aug_state,
           response->mutable_spec()->mutable_peer_initiator_flow()->mutable_flow_data()->\
           mutable_conn_track_info());
    }

    if (session->rflow && session->rflow->assoc_flow) {
        HAL_TRACE_VERBOSE("valid rflow aug session");
        // aug iflow age
        create_ns = session_state->rflow_aug_state.create_ts;
        age = TIME_DIFF(ctime_ns, create_ns) / TIME_NSECS_PER_SEC;
        response->mutable_spec()->mutable_peer_responder_flow()->mutable_flow_data()->\
              mutable_flow_info()->set_flow_age(age);

        flow_state_to_flow_stats_response(&session->rflow->assoc_flow->stats,
                &session_state->rflow_aug_state,
                response->mutable_stats()->mutable_peer_responder_flow_stats());

        flow_state_to_connection_track_response(&session_state->rflow_aug_state,
           response->mutable_spec()->mutable_peer_responder_flow()->mutable_flow_data()->\
           mutable_conn_track_info());
    }

    return;
}

hal_ret_t
system_get_fill_rsp (session_t *session, SessionGetResponse *response)
{
    pd::pd_session_get_args_t   args;
    session_state_t             session_state;
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_func_args_t          pd_func_args = {0};

    args.session = session;
    args.session_state = &session_state;
    pd_func_args.pd_session_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to fetch session state for session {}",
                      session->hal_handle);
        return HAL_RET_ERR;
    }

    session_to_session_get_response(session, response);
    session_state_to_session_get_response(session, &session_state, response);
    response->set_api_status(types::API_STATUS_OK);
    ret = fte::session_get(session, response);

    return ret;
}

static inline bool
session_is_alg_enabled(hal::session_t *session, nwsec::ALGName alg)
{
    std::string feature;
    switch (alg) {
        case nwsec::APP_SVC_TFTP:
             feature = "alg_tftp"; break;
        case nwsec::APP_SVC_FTP:
             feature = "alg_ftp"; break;
        case nwsec::APP_SVC_DNS:
             feature = "alg_dns"; break;
        case nwsec::APP_SVC_SUN_RPC:
        case nwsec::APP_SVC_MSFT_RPC:
             feature = "alg_rpc"; break;
        case nwsec::APP_SVC_RTSP:
             feature = "alg_rtsp"; break;
        default:
             return false;
    }
    return fte::session_is_feature_enabled(session, feature.c_str());
}

static inline bool
flow_matches_filter (hal::flow_t *flow, SessionFilter *filter)
{
    ip_addr_t ip_addr, check_addr;
    hal_ret_t ret;

    check_addr.af = IP_AF_IPV4;

    if (filter->vrf_id()) {
        if ((flow->config.key.svrf_id != filter->vrf_id()) &&
            (flow->config.key.dvrf_id != filter->vrf_id())) {
            return false;
        }
    }

    if (filter->l2_segment_id()) {
        if (flow->config.key.flow_type != FLOW_TYPE_L2) {
            return false;
        }
        if (flow->config.key.l2seg_id != filter->l2_segment_id()) {
            return false;
        }
    }

    if (flow->config.key.flow_type != FLOW_TYPE_V4) {
        return false;
    }

    if (filter->has_src_ip()) {
        ret = ip_addr_spec_to_ip_addr(&ip_addr, filter->src_ip());
        if (ret != HAL_RET_OK) {
            return false;
        }

        memcpy(&check_addr.addr, &flow->config.key.sip, sizeof(check_addr.addr));
        if (!ip_addr_check_equal(&check_addr, &ip_addr)) {
            return false;
        }
    }

    if (filter->has_dst_ip()) {
	    HAL_TRACE_DEBUG("Checking for dest ip");
        ret = ip_addr_spec_to_ip_addr(&ip_addr, filter->dst_ip());
        if (ret != HAL_RET_OK) {
            return false;
        }

        memcpy(&check_addr.addr, &flow->config.key.dip, sizeof(check_addr.addr));
        if (!ip_addr_check_equal(&check_addr, &ip_addr)) {
            return false;
        }
    }

    if (filter->src_port()) {
        if (flow->config.key.sport != filter->src_port()) {
            return false;
        }
    }

    if (filter->dst_port()) {
        if (flow->config.key.dport != filter->dst_port()) {
            return false;
        }
    }

    if (filter->ip_proto()) {
        if (flow->config.key.proto != filter->ip_proto()) {
            return false;
        }
    }

    return true;
}

static inline bool
session_matches_filter (hal::session_t *session, SessionFilter *filter)
{
    if (filter->alg()) {
        if (!session_is_alg_enabled(session, filter->alg())) {
            return false;
        }
    }

    if (flow_matches_filter(session->iflow, filter)) {
        return true;
    }

    if (session->rflow && flow_matches_filter(session->rflow, filter)) {
        return true;
    }

    return false;
}

hal_ret_t
session_get (SessionGetRequest& req, SessionGetResponseMsg *response)
{
    session_t                   *session;
    SessionGetResponse          *rsp;

    struct session_get_filter_t {
        SessionFilter           *filter;
        SessionGetResponseMsg   *response;
    } ctxt = {0};

    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_CLASSIC) {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_SESSION_NOT_FOUND;
    }

    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t          *session = (session_t *)entry;
        SessionGetResponseMsg   *rsp = (SessionGetResponseMsg *)(\
                           ((session_get_filter_t *)ctxt)->response);
        SessionFilter           *filter = (SessionFilter *)(\
                           ((session_get_filter_t *)ctxt)->filter);

        if (session_matches_filter(session, filter)) {
            system_get_fill_rsp(session, rsp->add_response());
        }
        return false;
    };

    if (req.has_session_filter()) {
        ctxt.filter = req.mutable_session_filter();
        ctxt.response = response;
        g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, &ctxt);
        return HAL_RET_OK;
    } else if (req.session_handle()) {
        rsp = response->add_response();
        session = find_session_by_handle(req.session_handle());
        if (session == NULL) {
            rsp->set_api_status(types::API_STATUS_NOT_FOUND);
            return HAL_RET_SESSION_NOT_FOUND;
        }
        return system_get_fill_rsp(session, rsp);
    } else {
        return session_get_all(response);
    }
}

hal_ret_t
session_delete (SessionDeleteRequest& req, SessionDeleteResponseMsg *response)
{
    session_t                   *session;
    SessionDeleteResponse          *rsp;
    hal_ret_t                       ret;

    struct session_delete_filter_t {
        SessionDeleteRequest       req;
        dllist_ctxt_t             *session_list;
    } ctxt = {};

    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t             *session = (session_t *)entry;
        SessionDeleteRequest        req = ((session_delete_filter_t *)ctxt)->req;
        SessionFilter              *filter = req.mutable_session_filter();
        dllist_ctxt_t              *list_head = ((session_delete_filter_t *)ctxt)->session_list;

        if (session_matches_filter(session, filter)) {
            hal_handle_id_list_entry_t *list_entry = (hal_handle_id_list_entry_t *)g_hal_state->\
                                   hal_handle_id_list_entry_slab()->alloc();
            if (list_entry == NULL) {
                HAL_TRACE_ERR("Out of memory - skipping delete session {}", session->hal_handle);
                return false;
            }
            list_entry->handle_id = session->hal_handle;
            dllist_add(list_head, &list_entry->dllist_ctxt);
        }

        return false;
    };

    if (req.has_session_filter()) {
        dllist_ctxt_t session_list;
        dllist_reset(&session_list);
        ctxt.req = req;
        ctxt.session_list = &session_list;

        g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, &ctxt);
        ret = session_delete_list(&session_list, false);
        rsp = response->add_response();
        rsp->set_api_status(hal::hal_prepare_rsp(ret));
        return HAL_RET_OK;
    } else if (req.session_handle()) {
        rsp = response->add_response();
        session = find_session_by_handle(req.session_handle());
        if (session == NULL) {
            rsp->set_api_status(types::API_STATUS_NOT_FOUND);
            return HAL_RET_SESSION_NOT_FOUND;
        }
        ret = fte::session_delete(session, true);
        rsp->set_api_status(hal::hal_prepare_rsp(ret));
        return HAL_RET_OK;
    } else {
        return session_delete_all(response);
    }
}

hal_ret_t
session_get_stream (SessionGetRequest& req, grpc::ServerWriter<session::SessionGetResponseMsg> *writer)
{
    session_t *session;

    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_CLASSIC) {
        SessionGetResponseMsg msg;
        msg.Clear();
        SessionGetResponse *rsp = msg.add_response();
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        writer->Write(msg);
        msg.Clear();
        return HAL_RET_SESSION_NOT_FOUND;
    }

    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t *session = (session_t *)entry;
        session_get_stream_filter_t *sget = (session_get_stream_filter_t *)ctxt;
        SessionFilter *filter = (SessionFilter *)(sget->filter);

        if (session_matches_filter(session, filter)) {
            system_get_fill_rsp(session, sget->msg.add_response());
            sget->count++;
            if (sget->count == SESSION_GET_STREAM_COUNT) {
                sget->writer->Write(sget->msg);
                sget->msg.Clear();
                sget->count = 0;
            }
        }
        return false;
    };

    if (req.has_session_filter()) {
        session_get_stream_filter_t ctxt = {0};
        ctxt.writer = writer;
        ctxt.msg.Clear();
        ctxt.filter = req.mutable_session_filter();

        g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, &ctxt);
        if (ctxt.count) {
            ctxt.writer->Write(ctxt.msg);
            ctxt.msg.Clear();
            ctxt.count = 0;
        }

        return HAL_RET_OK;
    } else if (req.session_handle()) {
        SessionGetResponseMsg msg;
        msg.Clear();
        SessionGetResponse *rsp = msg.add_response();
        session = find_session_by_handle(req.session_handle());
        if (session == NULL) {
            rsp->set_api_status(types::API_STATUS_NOT_FOUND);
            writer->Write(msg);
            msg.Clear();
            return HAL_RET_SESSION_NOT_FOUND;
        }
        system_get_fill_rsp(session, rsp);
        writer->Write(msg);
        msg.Clear();

        return HAL_RET_OK;
    } else {
        return session_get_all_stream(writer);
    }
}

hal_ret_t
session_get_all_stream(grpc::ServerWriter<session::SessionGetResponseMsg> *writer)
{
    session_get_t sget;

    sget.writer = writer;
    sget.msg.Clear();
    sget.count = 0;

    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t  *session = (session_t *)entry;
        session_get_t *sget = (session_get_t *)ctxt;
        system_get_fill_rsp(session, sget->msg.add_response());
        sget->count++;
        if (sget->count == SESSION_GET_STREAM_COUNT) {
            sget->writer->Write(sget->msg);
            sget->msg.Clear();
            sget->count = 0;
        }
        return false;
    };

    sdk_ret_t ret = g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, &sget);
    if (sget.count) {
        sget.writer->Write(sget.msg);
        sget.msg.Clear();
        sget.count = 0;
    }

    return hal_sdk_ret_to_hal_ret(ret);
}

hal_ret_t
session_get_all(SessionGetResponseMsg *rsp)
{
    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t  *session = (session_t *)entry;
        SessionGetResponseMsg *rsp = (SessionGetResponseMsg *)ctxt;
        system_get_fill_rsp(session, rsp->add_response());
        return false;
    };

    sdk_ret_t ret = g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, rsp);

    return hal_sdk_ret_to_hal_ret(ret);
}

hal_ret_t
session_update_list (dllist_ctxt_t *session_list, bool async, uint64_t featureid_bitmap)
{
    // update all sessions
    hal_ret_t ret = HAL_RET_OK;
    dllist_ctxt_t  *curr = NULL, *next = NULL;
    
    dllist_for_each_safe(curr, next, session_list) {
        hal_handle_id_list_entry_t  *entry =
            dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        hal::session_t *session = hal::find_session_by_handle(entry->handle_id);
        if (session) {
            if (async) {
                ret = fte::session_update_async(session, featureid_bitmap);
            } else {
                // TODO: No sync call in fte ?
            }
        }
        // Remove from list
        dllist_del(&entry->dllist_ctxt);
        g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
    }
    
    return ret;
}

hal_ret_t
session_delete_list (dllist_ctxt_t *session_list, bool async)
{
    // delete all sessions
    hal_ret_t ret = HAL_RET_OK;
    dllist_ctxt_t  *curr = NULL, *next = NULL;
    dllist_for_each_safe(curr, next, session_list) {
        hal_handle_id_list_entry_t  *entry =
            dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        hal::session_t *session = hal::find_session_by_handle(entry->handle_id);
        if (session) {
            if (async) {
                ret = fte::session_delete_async(session, true);
            } else {
                ret = fte::session_delete(session, true);
                if (ret != HAL_RET_OK) {
                    goto cleanup;
                }
            }
        }
        // Remove from list
        dllist_del(&entry->dllist_ctxt);
        g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
    }
cleanup:
    dllist_for_each_safe(curr, next, session_list) {
        hal_handle_id_list_entry_t  *entry =
            dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);

        // Remove from list
        dllist_del(&entry->dllist_ctxt);
        g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
    }

    return ret;
}

hal_ret_t
session_delete_all (SessionDeleteResponseMsg *rsp)
{

    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t  *session = (session_t *)entry;
        dllist_ctxt_t   *list_head = (dllist_ctxt_t  *)ctxt;

        hal_handle_id_list_entry_t *list_entry = (hal_handle_id_list_entry_t *)g_hal_state->
                hal_handle_id_list_entry_slab()->alloc();

        if (list_entry == NULL) {
            HAL_TRACE_ERR("Out of memory - skipping delete session {}", session->hal_handle);
            return false;
        }

        list_entry->handle_id = session->hal_handle;
        dllist_add(list_head, &list_entry->dllist_ctxt);
        return false;
    };

    // build list of session_ids
    dllist_ctxt_t session_list;
    dllist_reset(&session_list);
    g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, &session_list);

    hal_ret_t ret;
    ret = session_delete_list(&session_list);
    if (ret != HAL_RET_OK) ret = HAL_RET_OK;
    rsp->add_response()->set_api_status(hal::hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// flow create FTE
//-----------------------------------------------------------------------------
static flow_t *
flow_create_fte (const flow_cfg_t *cfg,
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
    SDK_SPINLOCK_INIT(&flow->slock, PTHREAD_PROCESS_SHARED);
    flow->flow_key_ht_ctxt.reset();

    // Enable FlowTelemetryDrop by default
    flow->flow_telemetry_enable_flags |= (1 << FLOW_TELEMETRY_DROP);

    if (cfg){
        flow->config = *cfg;
    }

    if (attrs) {
        flow->pgm_attrs = *attrs;
    }

    flow->session = session;

    // check if we have to create associated flow
    if (cfg_assoc) {
        assoc_flow = (flow_t *)g_hal_state->flow_slab()->alloc();
        if (!assoc_flow) {
            return NULL;
        }
        *assoc_flow = {};
        SDK_SPINLOCK_INIT(&assoc_flow->slock, PTHREAD_PROCESS_SHARED);
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

void incr_global_session_tcp_rst_stats (uint8_t fte_id) {
    HAL_SESSION_STATS_PTR(fte_id)->num_tcp_rst_sent += 1;
}

void incr_global_session_icmp_error_stats (uint8_t fte_id) {
    HAL_SESSION_STATS_PTR(fte_id)->num_icmp_error_sent += 1;
}

void
get_l2_flow_telemetry_key (flow_telemetry_state_t *flow_telemetry_state_p, 
                           flowstats::L2FlowKey *key_p)
{
    key_p->set_svrf(flow_telemetry_state_p->key.svrf);
    key_p->set_dvrf(flow_telemetry_state_p->key.dvrf);
    key_p->set_l2seg_id(flow_telemetry_state_p->key.u.l2.l2seg_id);
    key_p->set_smac(flow_telemetry_state_p->key.u.l2.smac);
    key_p->set_dmac(flow_telemetry_state_p->key.u.l2.dmac);
    key_p->set_ether_type(flow_telemetry_state_p->key.u.l2.ether_type);
}

void
get_ipv4_flow_telemetry_key (flow_telemetry_state_t *flow_telemetry_state_p, 
                             flowstats::IPv4FlowKey *key_p)
{
    key_p->set_svrf(flow_telemetry_state_p->key.svrf);
    key_p->set_dvrf(flow_telemetry_state_p->key.dvrf);
    key_p->set_sip(flow_telemetry_state_p->key.u.v4.sip);
    key_p->set_dip(flow_telemetry_state_p->key.u.v4.dip);
    key_p->set_sport(flow_telemetry_state_p->key.u.v4.sport);
    key_p->set_dport(flow_telemetry_state_p->key.u.v4.dport);
    key_p->set_ip_proto(flow_telemetry_state_p->key.u.v4.proto);
}

void
get_ipv6_flow_telemetry_key (flow_telemetry_state_t *flow_telemetry_state_p, 
                             flowstats::IPv6FlowKey *key_p)
{
    key_p->set_svrf(flow_telemetry_state_p->key.svrf);
    key_p->set_dvrf(flow_telemetry_state_p->key.dvrf);
    key_p->set_sip_hi(flow_telemetry_state_p->key.u.v6.sip_hi);
    key_p->set_sip_lo(flow_telemetry_state_p->key.u.v6.sip_lo);
    key_p->set_dip_hi(flow_telemetry_state_p->key.u.v6.dip_hi);
    key_p->set_dip_lo(flow_telemetry_state_p->key.u.v6.dip_lo);
    key_p->set_sport(flow_telemetry_state_p->key.u.v6.sport);
    key_p->set_dport(flow_telemetry_state_p->key.u.v6.dport);
    key_p->set_ip_proto(flow_telemetry_state_p->key.u.v6.proto);
}

inline void
get_l2_flow_proto_key (flow_key_t *flow_key_p, flowstats::L2FlowKey *key_p)
{
    key_p->set_svrf(flow_key_p->svrf_id);
    key_p->set_dvrf(flow_key_p->dvrf_id);
    key_p->set_l2seg_id(flow_key_p->l2seg_id);
    key_p->set_smac(MAC_TO_UINT64(flow_key_p->smac));
    key_p->set_dmac(MAC_TO_UINT64(flow_key_p->dmac));
    key_p->set_ether_type(flow_key_p->ether_type);
}

inline void
get_ipv4_flow_proto_key (flow_key_t *flow_key_p, flowstats::IPv4FlowKey *key_p)
{
    key_p->set_svrf(flow_key_p->svrf_id);
    key_p->set_dvrf(flow_key_p->dvrf_id);
    key_p->set_sip(flow_key_p->sip.v4_addr);
    key_p->set_dip(flow_key_p->dip.v4_addr);
    key_p->set_sport(flow_key_p->sport);
    key_p->set_dport(flow_key_p->dport);
    key_p->set_ip_proto(flow_key_p->proto);
}

inline void
get_ipv6_flow_proto_key (flow_key_t *flow_key_p, flowstats::IPv6FlowKey *key_p)
{
    key_p->set_svrf(flow_key_p->svrf_id);
    key_p->set_dvrf(flow_key_p->dvrf_id);
    key_p->set_sip_hi(flow_key_p->sip.v6_addr.addr64[0]);
    key_p->set_sip_lo(flow_key_p->sip.v6_addr.addr64[1]);
    key_p->set_dip_hi(flow_key_p->dip.v6_addr.addr64[0]);
    key_p->set_dip_lo(flow_key_p->dip.v6_addr.addr64[1]);
    key_p->set_sport(flow_key_p->sport);
    key_p->set_dport(flow_key_p->dport);
    key_p->set_ip_proto(flow_key_p->proto);
}

void
initialize_flow_telemetry_hbm_stats_state (flow_t *flow_p, 
                                           flow_state_t *flow_state_p)
{
    flow_telemetry_state_t *flow_telemetry_state_p;
    uint32_t                delta_packets, delta_bytes;

    flow_telemetry_state_p = flow_p->flow_telemetry_state_p;
    if (flow_p->flow_telemetry_enable_flags & (1 << FLOW_TELEMETRY_RAW)) {
        // Compute Delta-Packets/Bytes since last capture
        delta_packets = (uint32_t) flow_state_p->packets - 
        flow_telemetry_state_p->u1.raw_metrics.last_flow_table_packets;
        delta_bytes = (uint32_t) flow_state_p->bytes - flow_telemetry_state_p->
                                 u1.raw_metrics.last_flow_table_bytes;
        flow_telemetry_state_p->u1.raw_metrics.last_flow_table_packets =
                                (uint32_t) flow_state_p->packets;
        flow_telemetry_state_p->u1.raw_metrics.last_flow_table_bytes =
                                (uint32_t) flow_state_p->bytes;

        // Packets / Bytes are accumulated to handle Telemetry-HBM-resource
        // Re-use case
        flow_telemetry_state_p->u1.raw_metrics.packets += delta_packets;
        flow_telemetry_state_p->u1.raw_metrics.bytes += delta_bytes;
    }

    if (flow_p->flow_telemetry_enable_flags & (1 << FLOW_TELEMETRY_DROP)) {
        timespec_t ctime;
        uint64_t   ctime_ns;

        clock_gettime(CLOCK_MONOTONIC, &ctime);
        sdk::timestamp_to_nsecs(&ctime, &ctime_ns);

        // Sense for first-time occurring drop-events
        if (flow_telemetry_state_p->u1.drop_metrics.packets == 0)
            flow_telemetry_state_p->u1.drop_metrics.first_timestamp = ctime_ns;
        flow_telemetry_state_p->u1.drop_metrics.last_timestamp = ctime_ns;

        // Compute Delta-Packets/Bytes since last capture
        delta_packets = (uint32_t) flow_state_p->drop_packets -
        flow_telemetry_state_p->u1.drop_metrics.last_flow_table_packets;
        delta_bytes = (uint32_t) flow_state_p->drop_bytes - 
        flow_telemetry_state_p->u1.drop_metrics.last_flow_table_bytes;
        flow_telemetry_state_p->u1.drop_metrics.last_flow_table_packets =
                                (uint32_t) flow_state_p->drop_packets;
        flow_telemetry_state_p->u1.drop_metrics.last_flow_table_bytes =
                                (uint32_t) flow_state_p->drop_bytes;

        // Packets / Bytes are accumulated to handle Telemetry-HBM-resource
        // Re-use case
        flow_telemetry_state_p->u1.drop_metrics.packets += delta_packets;
        flow_telemetry_state_p->u1.drop_metrics.bytes += delta_bytes;
        flow_telemetry_state_p->u1.drop_metrics.reason |= flow_state_p->
                                                          exception_bmap;
    }
}

flow_telemetry_state_t *
check_for_flow_telemetry_hbm_stats_state_reuse (flow_t *flow_p,
                                                flow_state_t *flow_state_p)
{
    flow_telemetry_state_t *flow_telemetry_state_p = NULL;
    sdk::types::mem_addr_t  vaddr, pal_addr = INVALID_MEM_ADDRESS;
    uint8_t flow_type,      flow_telemetry_create_flags;

    flow_telemetry_create_flags = flow_p->flow_telemetry_create_flags;
    flow_type = flow_p->config.key.flow_type;
    switch (flow_type) {
        case FLOW_TYPE_L2:
            {
                flowstats::L2FlowKey key;

                get_l2_flow_proto_key(&flow_p->config.key, &key);
                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::L2FlowRawMetrics::Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::L2FlowDropMetrics::Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::L2FlowPerformanceMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::L2FlowBehavioralMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }
            }
            break;
        case FLOW_TYPE_V4:
            {
                flowstats::IPv4FlowKey key;

                get_ipv4_flow_proto_key(&flow_p->config.key, &key);
                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::IPv4FlowRawMetrics::Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::IPv4FlowDropMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::IPv4FlowPerformanceMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_LATENCY)) {
                    auto m_ptr = delphi::objects::IPv4FlowLatencyMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::IPv4FlowBehavioralMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }
            }
            break;
        case FLOW_TYPE_V6:
            {
                flowstats::IPv6FlowKey key;

                get_ipv6_flow_proto_key(&flow_p->config.key, &key);
                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::IPv6FlowRawMetrics::Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::IPv6FlowDropMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::IPv6FlowPerformanceMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_LATENCY)) {
                    auto m_ptr = delphi::objects::IPv6FlowLatencyMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::IPv6FlowBehavioralMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        pal_addr = m_ptr->GetPalAddr();
                        break;
                    }
                }
            }
            break;
        default:
            break;
    }

    // Re-use HBM-resource, if applicable
    if (pal_addr != INVALID_MEM_ADDRESS) {
        sdk::lib::pal_ret_t ret = sdk::lib::pal_physical_addr_to_virtual_addr(
                                            pal_addr, &vaddr);
        SDK_ASSERT(ret == sdk::lib::PAL_RET_OK);
        flow_telemetry_state_p = (flow_telemetry_state_t *) vaddr;

        // De-link Re-use HBM-resource from the Age-list
        if (flow_telemetry_state_p->prev_p == NULL) {
            g_flow_telemetry_state_age_head_p = flow_telemetry_state_p->next_p;
            if (g_flow_telemetry_state_age_head_p == NULL)
                g_flow_telemetry_state_age_tail_p = NULL;
            else
                g_flow_telemetry_state_age_head_p->prev_p = NULL;
        }
        else if (flow_telemetry_state_p->next_p == NULL) {
            g_flow_telemetry_state_age_tail_p = flow_telemetry_state_p->prev_p;
            g_flow_telemetry_state_age_tail_p->next_p = NULL;
        }
        else {
            flow_telemetry_state_t *next_p = flow_telemetry_state_p->next_p;
            flow_telemetry_state_t *prev_p = flow_telemetry_state_p->prev_p;

            prev_p->next_p = next_p;
            next_p->prev_p = prev_p;
        }
        flow_telemetry_state_p->prev_p = NULL;
        flow_telemetry_state_p->next_p = NULL;
        if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_RAW)) {
            flow_telemetry_state_p->u1.raw_metrics.last_flow_table_packets = 0;
            flow_telemetry_state_p->u1.raw_metrics.last_flow_table_bytes = 0;
        }
        if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_DROP)) {
            flow_telemetry_state_p->u1.drop_metrics.last_flow_table_packets = 0;
            flow_telemetry_state_p->u1.drop_metrics.last_flow_table_bytes = 0;
        }

        flow_telemetry_state_p->u1.drop_metrics.instances++;
        flow_p->flow_telemetry_state_p = flow_telemetry_state_p;

        initialize_flow_telemetry_hbm_stats_state(flow_p, flow_state_p);
    }

    return(flow_telemetry_state_p);
}

void
allocate_flow_telemetry_hbm_stats_state (flow_t *flow_p, 
                                         flow_state_t *flow_state_p)
{
    sdk::lib::indexer::status rs;
    sdk::types::mem_addr_t vaddr, pal_addr;
    flow_telemetry_state_t *flow_telemetry_state_p;
    uint32_t idx;

    // No need to allocate HBM-resource if already allocated
    if (flow_p->flow_telemetry_state_p != NULL)
        return;

    // Check and re-use if concerned Delphi-object is not deleted yet
    flow_telemetry_state_p = check_for_flow_telemetry_hbm_stats_state_reuse(
                             flow_p, flow_state_p);
    if (flow_telemetry_state_p != NULL)
        return;

    //
    // Allocate Flow-Proto-State in HBM, if we run out of HBM resources
    // exit gracefully
    //
    pal_addr = get_mem_addr(CAPRI_HBM_REG_FLOW_TELEMETRY_STATS);
    SDK_ASSERT(pal_addr != INVALID_MEM_ADDRESS);

    rs = g_flow_proto_state_indexer->alloc(&idx);
    if (rs != sdk::lib::indexer::SUCCESS)
        return;

    //
    // Convert Physical-Flow-Proto-State-ptr to Virtual-address
    //
    pal_addr += idx * (1 << FLOW_TELEMETRY_STATS_SHIFT);
    sdk::lib::pal_ret_t ret = sdk::lib::pal_physical_addr_to_virtual_addr(
                                        pal_addr, &vaddr);
    SDK_ASSERT(ret == sdk::lib::PAL_RET_OK);
    flow_telemetry_state_p = (flow_telemetry_state_t *) vaddr;
    flow_p->flow_telemetry_state_p = flow_telemetry_state_p;

    //
    // Initialize Flow-Proto-State with defaults
    //
    *flow_telemetry_state_p = {0};
    flow_telemetry_state_p->stats_idx = idx;
    flow_telemetry_state_p->u1.drop_metrics.instances++;

    // Initialize Key fields
    flow_telemetry_state_p->key.flow_type = flow_p->config.key.flow_type;
    flow_telemetry_state_p->key.svrf = (uint32_t) flow_p->config.key.svrf_id;
    flow_telemetry_state_p->key.dvrf = (uint32_t) flow_p->config.key.dvrf_id;
    switch (flow_p->config.key.flow_type) {
        case FLOW_TYPE_L2:
            flow_telemetry_state_p->key.u.l2.l2seg_id = (uint32_t) flow_p->
                                                         config.key.l2seg_id;
            flow_telemetry_state_p->key.u.l2.ether_type = flow_p->config.key.
                                                          ether_type;
            flow_telemetry_state_p->key.u.l2.smac = MAC_TO_UINT64(flow_p->
                                                    config.key.smac);
            flow_telemetry_state_p->key.u.l2.dmac = MAC_TO_UINT64(flow_p->
                                                    config.key.dmac);
            break;
        case FLOW_TYPE_V4:
            flow_telemetry_state_p->key.u.v4.sip = flow_p->config.key.sip.
                                                           v4_addr;
            flow_telemetry_state_p->key.u.v4.dip = flow_p->config.key.dip.
                                                           v4_addr;
            flow_telemetry_state_p->key.u.v4.sport = flow_p->config.key.sport;
            flow_telemetry_state_p->key.u.v4.dport = flow_p->config.key.dport;
            flow_telemetry_state_p->key.u.v4.proto = flow_p->config.key.proto;
            break;
        case FLOW_TYPE_V6:
            flow_telemetry_state_p->key.u.v6.sip_hi = flow_p->config.key.sip.
                                                              v6_addr.addr64[0];
            flow_telemetry_state_p->key.u.v6.sip_lo = flow_p->config.key.sip.
                                                              v6_addr.addr64[1];
            flow_telemetry_state_p->key.u.v6.dip_hi = flow_p->config.key.dip.
                                                              v6_addr.addr64[0];
            flow_telemetry_state_p->key.u.v6.dip_lo = flow_p->config.key.dip.
                                                              v6_addr.addr64[1];
            flow_telemetry_state_p->key.u.v6.sport = flow_p->config.key.sport;
            flow_telemetry_state_p->key.u.v6.dport = flow_p->config.key.dport;
            flow_telemetry_state_p->key.u.v6.proto = flow_p->config.key.proto;
            break;
        default:
            break;
    }
    initialize_flow_telemetry_hbm_stats_state(flow_p, flow_state_p);
}

void
free_flow_telemetry_hbm_stats_state (flow_telemetry_state_t 
                                     *flow_telemetry_state_p)
{
    g_flow_proto_state_indexer->free(flow_telemetry_state_p->stats_idx);
}

void
delete_flow_proto_state (flow_telemetry_state_t *flow_telemetry_state_p,
                         uint8_t flow_telemetry_delete_flags)
{
    uint8_t flow_type;

    flow_type = flow_telemetry_state_p->key.flow_type;
    switch (flow_type) {
        case FLOW_TYPE_L2:
            {
                flowstats::L2FlowKey key;

                get_l2_flow_telemetry_key(flow_telemetry_state_p, &key);
                if (flow_telemetry_delete_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::L2FlowRawMetrics::Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::L2FlowRawMetrics::Release(m_ptr);
                        delphi::objects::L2FlowRawMetrics::DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::L2FlowDropMetrics::Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::L2FlowDropMetrics::Release(m_ptr);
                        delphi::objects::L2FlowDropMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::L2FlowPerformanceMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::L2FlowPerformanceMetrics::
                                         Release(m_ptr);
                        delphi::objects::L2FlowPerformanceMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::L2FlowBehavioralMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::L2FlowBehavioralMetrics::
                                         Release(m_ptr);
                        delphi::objects::L2FlowBehavioralMetrics::
                                         DeleteComposite(key);
                    }
                }
            }
            break;
        case FLOW_TYPE_V4:
            {
                flowstats::IPv4FlowKey key;

                get_ipv4_flow_telemetry_key(flow_telemetry_state_p, &key);
                if (flow_telemetry_delete_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::IPv4FlowRawMetrics::Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv4FlowRawMetrics::Release(m_ptr);
                        delphi::objects::IPv4FlowRawMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::IPv4FlowDropMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv4FlowDropMetrics::Release(m_ptr);
                        delphi::objects::IPv4FlowDropMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::IPv4FlowPerformanceMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv4FlowPerformanceMetrics::
                                         Release(m_ptr);
                        delphi::objects::IPv4FlowPerformanceMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & 
                   (1 << FLOW_TELEMETRY_LATENCY)) {
                    auto m_ptr = delphi::objects::IPv4FlowLatencyMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv4FlowLatencyMetrics::Release(m_ptr);
                        delphi::objects::IPv4FlowLatencyMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::IPv4FlowBehavioralMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv4FlowBehavioralMetrics::
                                         Release(m_ptr);
                        delphi::objects::IPv4FlowBehavioralMetrics::
                                         DeleteComposite(key);
                    }
                }
            }
            break;
        case FLOW_TYPE_V6:
            {
                flowstats::IPv6FlowKey key;

                get_ipv6_flow_telemetry_key(flow_telemetry_state_p, &key);
                if (flow_telemetry_delete_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::IPv6FlowRawMetrics::Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv6FlowRawMetrics::Release(m_ptr);
                        delphi::objects::IPv6FlowRawMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::IPv6FlowDropMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv6FlowDropMetrics::Release(m_ptr);
                        delphi::objects::IPv6FlowDropMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::IPv6FlowPerformanceMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv6FlowPerformanceMetrics::
                                         Release(m_ptr);
                        delphi::objects::IPv6FlowPerformanceMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & 
                   (1 << FLOW_TELEMETRY_LATENCY)) {
                    auto m_ptr = delphi::objects::IPv6FlowLatencyMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv6FlowLatencyMetrics::Release(m_ptr);
                        delphi::objects::IPv6FlowLatencyMetrics::
                                         DeleteComposite(key);
                    }
                }

                if (flow_telemetry_delete_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::IPv6FlowBehavioralMetrics::
                                 Find(key);
                    if (m_ptr != NULL) {
                        delphi::objects::IPv6FlowBehavioralMetrics::
                                         Release(m_ptr);
                        delphi::objects::IPv6FlowBehavioralMetrics::
                                         DeleteComposite(key);
                    }
                }
            }
            break;
        default:
            return;
    }

    // De-Remember applicable FlowProto creation flags
    flow_telemetry_state_p->present_flags &= ~flow_telemetry_delete_flags;
}

void
create_flow_proto_state (flow_t *flow_p)
{
    sdk::types::mem_addr_t  stats_addr[FLOW_TELEMETRY_MAX], pal_addr;
    flow_telemetry_state_t *flow_telemetry_state_p;
    flow_telemetry_state_t *flow_telemetry_hbm_addr;
    uint8_t                 flow_type;
    uint8_t                 flow_telemetry_create_flags;
    uint8_t                 flow_telemetry_delete_flags;

    // No need to create if flow is not created (could happen for rflow)
    if (flow_p == NULL || flow_p->flow_telemetry_state_p == NULL)
        return;

    // Handle "on-the-fly" FlowProto disablement case
    flow_telemetry_state_p = flow_p->flow_telemetry_state_p;
    flow_telemetry_delete_flags = (~flow_p->flow_telemetry_enable_flags &
                                   flow_telemetry_state_p->present_flags);
    if (flow_telemetry_delete_flags)
        delete_flow_proto_state(flow_telemetry_state_p, 
                                flow_telemetry_delete_flags);

    // No need to create if FlowProto creation is not enabled or
    // FlowProto is already created
    flow_telemetry_create_flags = (flow_p->flow_telemetry_enable_flags &
                                   flow_p->flow_telemetry_create_flags &
                                  ~flow_telemetry_state_p->present_flags);
    if (flow_telemetry_create_flags == 0)
        return;

    // Fetch ptrs for various mertics blocks
    pal_addr = g_flow_telemetry_hbm_start + (flow_telemetry_state_p->stats_idx *                                            (1 << FLOW_TELEMETRY_STATS_SHIFT));
    flow_telemetry_hbm_addr = (flow_telemetry_state_t *) pal_addr;
    stats_addr[FLOW_TELEMETRY_RAW] = (mem_addr_t) &flow_telemetry_hbm_addr->
                                      u1.raw_metrics.instances;
    stats_addr[FLOW_TELEMETRY_DROP] = (mem_addr_t) &flow_telemetry_hbm_addr->
                                       u1.drop_metrics.instances;
    stats_addr[FLOW_TELEMETRY_PERFORMANCE] = (mem_addr_t) 
                     &flow_telemetry_hbm_addr->u1.performance_metrics.instances;
    stats_addr[FLOW_TELEMETRY_LATENCY] = (mem_addr_t) &flow_telemetry_hbm_addr->
                                          u1.latency_metrics.instances;
    stats_addr[FLOW_TELEMETRY_BEHAVIORAL] = (mem_addr_t) 
                      &flow_telemetry_hbm_addr->u1.behavioral_metrics.instances;

    flow_type = flow_p->config.key.flow_type;
    switch (flow_type) {
        case FLOW_TYPE_L2:
            {
                flowstats::L2FlowKey key;

                get_l2_flow_telemetry_key(flow_telemetry_state_p, &key);
                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::L2FlowRawMetrics::
                                 NewL2FlowRawMetrics(key, 
                                 stats_addr[FLOW_TELEMETRY_RAW]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::L2FlowDropMetrics::
                                 NewL2FlowDropMetrics(key, 
                                 stats_addr[FLOW_TELEMETRY_DROP]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::L2FlowPerformanceMetrics::
                                 NewL2FlowPerformanceMetrics(key, 
                                 stats_addr[FLOW_TELEMETRY_PERFORMANCE]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::L2FlowBehavioralMetrics::
                                 NewL2FlowBehavioralMetrics(key, 
                                 stats_addr[FLOW_TELEMETRY_BEHAVIORAL]);
                    SDK_ASSERT(m_ptr != NULL);
                }
            }
            break;
        case FLOW_TYPE_V4:
            {
                flowstats::IPv4FlowKey key;

                get_ipv4_flow_telemetry_key(flow_telemetry_state_p, &key);
                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::IPv4FlowRawMetrics::
                                 NewIPv4FlowRawMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_RAW]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::IPv4FlowDropMetrics::
                                 NewIPv4FlowDropMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_DROP]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::IPv4FlowPerformanceMetrics::
                                 NewIPv4FlowPerformanceMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_PERFORMANCE]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_LATENCY)) {
                    auto m_ptr = delphi::objects::IPv4FlowLatencyMetrics::
                                 NewIPv4FlowLatencyMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_LATENCY]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::IPv4FlowBehavioralMetrics::
                                 NewIPv4FlowBehavioralMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_BEHAVIORAL]);
                    SDK_ASSERT(m_ptr != NULL);
                }
            }
            break;
        case FLOW_TYPE_V6:
            {
                flowstats::IPv6FlowKey key;

                get_ipv6_flow_telemetry_key(flow_telemetry_state_p, &key);
                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_RAW)) {
                    auto m_ptr = delphi::objects::IPv6FlowRawMetrics::
                                 NewIPv6FlowRawMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_RAW]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & (1 << FLOW_TELEMETRY_DROP)) {
                    auto m_ptr = delphi::objects::IPv6FlowDropMetrics::
                                 NewIPv6FlowDropMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_DROP]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_PERFORMANCE)) {
                    auto m_ptr = delphi::objects::IPv6FlowPerformanceMetrics::
                                 NewIPv6FlowPerformanceMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_PERFORMANCE]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_LATENCY)) {
                    auto m_ptr = delphi::objects::IPv6FlowLatencyMetrics::
                                 NewIPv6FlowLatencyMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_LATENCY]);
                    SDK_ASSERT(m_ptr != NULL);
                }

                if (flow_telemetry_create_flags & 
                   (1 << FLOW_TELEMETRY_BEHAVIORAL)) {
                    auto m_ptr = delphi::objects::IPv6FlowBehavioralMetrics::
                                 NewIPv6FlowBehavioralMetrics(key,
                                 stats_addr[FLOW_TELEMETRY_BEHAVIORAL]);
                    SDK_ASSERT(m_ptr != NULL);
                }
            }
            break;
        default:
            return;
    }

    // Remember FlowProto creation
    flow_telemetry_state_p->present_flags |= flow_telemetry_create_flags;
    flow_p->flow_telemetry_create_flags = 0;
}

void
enqueue_flow_telemetry_state_to_age_list (flow_t *flow_p)
{
    flow_telemetry_state_t *flow_telemetry_state_p;

    // No need to enqueue to Age-list if no Proto-states are created
    if (flow_p == NULL || flow_p->flow_telemetry_state_p == NULL)
        return;

    // If Flow-Proto Delphi-objects are already purged, free up HBM memory
    flow_telemetry_state_p = flow_p->flow_telemetry_state_p;
    if (flow_telemetry_state_p->present_flags == 0) {
        free_flow_telemetry_hbm_stats_state(flow_telemetry_state_p);
    }
    else {
        // Enqueue Flow-Proto-State to Age-list queue
        if (g_flow_telemetry_state_age_tail_p == NULL) {
            g_flow_telemetry_state_age_head_p = flow_telemetry_state_p;
            g_flow_telemetry_state_age_tail_p = flow_telemetry_state_p;
        }
        else {
            flow_telemetry_state_p->prev_p = g_flow_telemetry_state_age_tail_p;
            g_flow_telemetry_state_age_tail_p->next_p = flow_telemetry_state_p;
            g_flow_telemetry_state_age_tail_p = flow_telemetry_state_p;
        }
        flow_telemetry_state_p->u2.delayed_age_ticks = 
                                        FLOW_TELEMETRY_DELAYED_AGE_TICKS_40SECS;
    }

    flow_p->flow_telemetry_state_p = NULL;
}

void
dequeue_flow_telemetry_state_from_age_list (
                                 flow_telemetry_state_t *flow_telemetry_state_p)
{
    // Dequeue Flow-Proto-State to Age-list queue
    //
    // The assumption here is that head-of-age-list is being dequeued since
    // that would be the first one to age out
    g_flow_telemetry_state_age_head_p = g_flow_telemetry_state_age_head_p->
                                        next_p;
    if (g_flow_telemetry_state_age_head_p == NULL)
        g_flow_telemetry_state_age_tail_p = NULL;;

    // Free Flow-Proto-Stats-State after deleting Delphi object
    delete_flow_proto_state(flow_telemetry_state_p, 
                            flow_telemetry_state_p->present_flags);
    free_flow_telemetry_hbm_stats_state(flow_telemetry_state_p);
}

inline void
update_global_session_stats (session_t *session, bool decr=false)
{
    flow_key_t key = session->iflow->config.key;

    if (session->iflow->pgm_attrs.drop) {
        HAL_SESSION_STATS_PTR(session->fte_id)->drop_sessions += 1;
    }

    if (key.flow_type == FLOW_TYPE_L2) {
        HAL_SESSION_STATS_PTR(session->fte_id)->l2_sessions += (decr)?(-1):1;
    } else if (key.flow_type == FLOW_TYPE_V4 ||
               key.flow_type == FLOW_TYPE_V6) {
        if (key.proto == types::IPPROTO_TCP) {
            HAL_SESSION_STATS_PTR(session->fte_id)->tcp_sessions += (decr)?(-1):1;
        } else if (key.proto == types::IPPROTO_UDP) {
            HAL_SESSION_STATS_PTR(session->fte_id)->udp_sessions += (decr)?(-1):1;
        } else if (key.proto == types::IPPROTO_ICMP) {
            HAL_SESSION_STATS_PTR(session->fte_id)->icmp_sessions += (decr)?(-1):1;
        }
    }

    HAL_SESSION_STATS_PTR(session->fte_id)->total_active_sessions += (decr)?(-1):1;
}

hal_ret_t
session_create (const session_args_t *args, hal_handle_t *session_handle,
                session_t **session_p)
{
    hal_ret_t ret;
    nwsec_profile_t              *nwsec_prof;
    pd::pd_session_create_args_t  pd_session_args;
    session_t                    *session;
    pd::pd_func_args_t          pd_func_args = {0};
    vrf_t                       *vrf = NULL;

    SDK_ASSERT(args->iflow && args->iflow_attrs);

    // allocate a session
    session = (session_t *)g_hal_state->session_slab()->alloc();
    if (session == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    *session = {};

    session->fte_id = fte::fte_id();
    HAL_TRACE_VERBOSE("Creating session {:p} with a rflow :{}", (void *)session, (args->valid_rflow) ? "valid" : "not valid");

    dllist_reset(&session->feature_list_head);
    session->vrf_handle = args->vrf_handle;
    session->tcp_cxntrack_timer = NULL;

    vrf = vrf_lookup_by_handle(args->vrf_handle);
    SDK_ASSERT_RETURN((vrf != NULL), HAL_RET_INVALID_ARG);

    // fetch the security profile, if any
    if (vrf->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        nwsec_prof =
            find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
    } else {
        nwsec_prof = NULL;
    }

    // Handle the spec info not handled in the FTE
    // TODO(goli) all these should go to appropriate fte features
    if (args->spec) {
        uint8_t  ingress, egress;
        ret = extract_mirror_sessions(args->spec->initiator_flow(), &ingress, &egress);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("session create failure extracting mirror sessions, "
                          "err : {}", ret);
            return ret;
        }
        args->iflow[0]->ing_mirror_session |= ingress;
        args->iflow[0]->eg_mirror_session |= egress;

        if(args->rflow[0] && args->spec->has_responder_flow()) {
            ret = extract_mirror_sessions(args->spec->responder_flow(),
                                          &ingress, &egress);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("session create failure extracting mirror sessions, "
                              "err : {}", ret);
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
    session->conn_track_en = args->session->conn_track_en;
    session->idle_timeout = args->session->idle_timeout;
    session->skip_sfw_reval = args->session->skip_sfw_reval;
    session->sfw_rule_id = args->session->sfw_rule_id;
    session->sfw_action = args->session->sfw_action;

    // allocate all PD resources and finish programming, if any
    pd::pd_session_create_args_init(&pd_session_args);
    pd_session_args.iflow_hash = args->flow_hash;
    pd_session_args.nwsec_prof = nwsec_prof;
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;
    pd_session_args.rsp = args->rsp;

    pd_func_args.pd_session_create = &pd_session_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK && ret != HAL_RET_COLLISION) {
        HAL_TRACE_ERR("PD session create failure, err : {}", ret);
        goto end;
    }

    // add this session to our db
    ret = add_session_to_db(args->sep_handle, args->dep_handle, session);
    SDK_ASSERT(ret == HAL_RET_OK);

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
        HAL_SESSION_STATS_PTR(session->fte_id)->num_session_create_err += 1;
    } else {
        update_global_session_stats(session);
    }

    return ret;
}

session_t *
session_lookup(flow_key_t key, flow_role_t *role)
{
    session_t *session = NULL;

    session = (session_t *)g_hal_state->session_hal_iflow_ht()->lookup(std::addressof(key));
    if (role)
        *role = FLOW_ROLE_INITIATOR;
    if (session == NULL) {
        session = (session_t *)g_hal_state->session_hal_rflow_ht()->lookup(std::addressof(key));
        if (role)
            *role = FLOW_ROLE_RESPONDER;
    }

    return session;
}

hal_ret_t
session_update(const session_args_t *args, session_t *session)
{
    hal_ret_t                       ret;
    pd::pd_session_update_args_t    pd_session_args;
    pd::pd_func_args_t              pd_func_args = {0};

    //SDK_ASSERT_RETURN(session->fte_id == fte::fte_id(), HAL_RET_INVALID_ARG);

    if (session->fte_id != fte::fte_id()) {
        HAL_TRACE_ERR("session fte_id {} current fte_id {}", session->fte_id, fte::fte_id());
        return HAL_RET_OK;
    }

    session->idle_timeout = args->session->idle_timeout;

    if (args->iflow[0]) {
        session->iflow->config = *args->iflow[0];
        session->iflow->pgm_attrs = *args->iflow_attrs[0];
        if (session->iflow->assoc_flow && args->iflow[1]) {
            session->iflow->assoc_flow->config = *args->iflow[1];
            session->iflow->assoc_flow->pgm_attrs = *args->iflow_attrs[1];
        }
    }

    if (session->rflow && args->rflow[0]) {
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
    pd_session_args.session = session;
    pd_session_args.session_state = args->session_state;
    pd_session_args.rsp = args->rsp;
    pd_session_args.update_iflow = args->update_iflow;
    pd_session_args.update_rflow = args->update_rflow;

    pd_func_args.pd_session_update = &pd_session_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_UPDATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session update failure, err : {}", ret);
    }

    return ret;
}

hal_ret_t
session_delete(const session_args_t *args, session_t *session)
{
    hal_ret_t                       ret;
    pd::pd_session_delete_args_t    pd_session_args;
    pd::pd_func_args_t              pd_func_args = {0};


    SDK_ASSERT_RETURN(session->fte_id == fte::fte_id(), HAL_RET_INVALID_ARG);

    // Handle Delphi-object referenced states in HBM in a delayed fashion
    // to make sure that last-updated data has been streamed out
    enqueue_flow_telemetry_state_to_age_list(session->iflow);
    enqueue_flow_telemetry_state_to_age_list(session->rflow);

    del_session_from_db(args->sep_handle, args->dep_handle, session);

    // allocate all PD resources and finish programming, if any
    pd::pd_session_delete_args_init(&pd_session_args);
    pd_session_args.vrf = vrf_lookup_by_handle(args->vrf_handle);
    pd_session_args.session = session;
    pd_session_args.session_state = args ? args->session_state : NULL;
    pd_session_args.rsp = args ? args->rsp : NULL;

    pd_func_args.pd_session_delete = &pd_session_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD session delete failure, err : {}", ret);
    }

    update_global_session_stats(session, true);

    session_cleanup(session);

    return ret;
}

#define SESSION_AGED_RET(ENTRY)                                     \
    ENTRY(SESSION_AGED_NONE,   0,  "SESSION_AGED_NONE")             \
    ENTRY(SESSION_AGED_IFLOW,  1,  "SESSION_AGED_IFLOW")            \
    ENTRY(SESSION_AGED_RFLOW,  2,  "SESSION_AGED_RFLOW")            \
    ENTRY(SESSION_AGED_BOTH,   3,  "SESSION_AGED_BOTH")             \

DEFINE_ENUM(session_aged_ret_t, SESSION_AGED_RET)
#undef SESSION_AGED_RET
/*
 * TCP tickle timer context used to
 * send out TCP keepalives & RST in case
 * of session aging
 */
typedef struct tcptkle_timer_ctx_ {
    hal_handle_t        session_handle;
    session_state_t     session_state;
    session_aged_ret_t  aged_flow;
    uint8_t             num_tickles;
    dllist_ctxt_t       dllist_ctxt;
} __PACK__ tcptkle_timer_ctx_t;

#define BUILD_TCP_SEND_TIMESTAMP 0x01
#define BUILD_TCP_SEND_RST       0x02
#define BUILD_TCP_SEND_FIN       0x04

static uint32_t
build_tcp_packet (hal::flow_t *flow, session_t *session,
                  flow_state_t state,
                  pd::cpu_to_p4plus_header_t *cpu_header,
                  pd::p4plus_to_p4_header_t *p4plus_header,
                  uint8_t *pkt, uint8_t flags)
{
    vlan_header_t       *vlan_hdr = NULL;
    ether_header_t      *eth_hdr = NULL;
    ipv4_header_t       *ip_hdr = NULL;
    tcp_header_t        *tcp_hdr = NULL;
    hal_ret_t            ret = HAL_RET_OK;
    ep_t                *sep = NULL, *dep = NULL;
    l2seg_t             *l2seg = NULL;
    pd::pd_func_args_t   pd_func_args = {0};
    hal::flow_key_t      key = flow->config.key;
    uint32_t             offset = 0;
    tcp_ts_option_t     *tcp_ts = NULL;
    bool                 send_tcp_ts = (flags & BUILD_TCP_SEND_TIMESTAMP);
    bool                 setrst = (flags & BUILD_TCP_SEND_RST);
    bool                 setfin = (flags & BUILD_TCP_SEND_FIN);

    if (!pkt) {
        return HAL_RET_INVALID_ARG;
    }

    bzero(cpu_header, sizeof(pd::cpu_to_p4plus_header_t));
    bzero(pkt, TCP_IPV4_DOT1Q_PKT_SZ);
    bzero(p4plus_header, sizeof(pd::p4plus_to_p4_header_t));

    //Fill the P4 plus header
    p4plus_header->p4plus_app_id = P4PLUS_APPTYPE_CPU;
    p4plus_header->compute_l4_csum = 1;
    p4plus_header->compute_ip_csum = 1;

    // Fill in P4Plus and CPU header info
    cpu_header->src_lif = HAL_LIF_CPU;
    cpu_header->l2_offset = 0;

    ret = ep_get_from_flow_key(&key, &sep, &dep);
    if (ret != HAL_RET_OK) {
        sep = find_ep_by_l2_key(flow->config.l2_info.l2seg_id, flow->config.l2_info.smac);
        dep = find_ep_by_l2_key(flow->config.l2_info.l2seg_id, flow->config.l2_info.dmac);
        if (sep == NULL) {
            if (key.dir == FLOW_DIR_FROM_DMA || dep == NULL) {
                HAL_TRACE_ERR("Couldnt get SEP/DEP from session :{}", key);
                return 0;
            }
        }
    }

    if (sep) {
        l2seg = hal::l2seg_lookup_by_handle(sep->l2seg_handle);
        if (l2seg == NULL) {
            HAL_TRACE_ERR("Couldnt get source l2seg for session :{}", key);
            return 0;
        }
    } else if (dep) {
        l2seg = hal::l2seg_lookup_by_handle(dep->l2seg_handle);
        if (l2seg == NULL) {
            HAL_TRACE_ERR("Couldnt get dest l2seg for session :{}", key);
            return 0;
        }
    }

    if (key.dir == FLOW_DIR_FROM_UPLINK) {
        cpu_header->src_lif = HAL_LIF_CPU;
        if (flow->pgm_attrs.use_vrf) {
            pd::pd_vrf_get_fromcpu_vlanid_args_t args;
            args.vrf = hal::vrf_lookup_by_handle(session->vrf_handle);
            args.vid = &cpu_header->hw_vlan_id;

            pd_func_args.pd_vrf_get_fromcpu_vlanid = &args;
            if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FRCPU_VLANID,
                                         &pd_func_args) == HAL_RET_OK) {
                cpu_header->flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
            }
        } else {
            pd::pd_l2seg_get_fromcpu_vlanid_args_t   args;
            args.l2seg = l2seg;
            args.vid = &cpu_header->hw_vlan_id;

            pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
            if (pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                                         &pd_func_args) == HAL_RET_OK) {
                cpu_header->flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
            }
        }
        eth_hdr = (ether_header_t *)(pkt);
        eth_hdr->etype = htons((key.flow_type == FLOW_TYPE_V4)?ETH_TYPE_IPV4:ETH_TYPE_IPV6);
        offset = sizeof(ether_header_t);
    } else { // FROM_DMA
        if_t   *sif = NULL;

        sif = hal::find_if_by_handle(sep->if_handle);
        if (sif == NULL) {
            HAL_TRACE_ERR("Couldnt get source if for session :{}", key);
            return HAL_RET_ERR;
        }

        pd::pd_if_get_hw_lif_id_args_t args;
        args.pi_if = sif;
        pd_func_args.pd_if_get_hw_lif_id = &args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_HW_LIF_ID, &pd_func_args);
        cpu_header->src_lif = args.hw_lif_id;
        vlan_hdr = (vlan_header_t *)(pkt);
        vlan_hdr->tpid = htons(ETH_TYPE_DOT1Q);
        vlan_hdr->vlan_tag = htons(sep->useg_vlan);
        vlan_hdr->etype = htons((key.flow_type == FLOW_TYPE_V4)?ETH_TYPE_IPV4:ETH_TYPE_IPV6);
        offset = sizeof(vlan_header_t);
        eth_hdr = (ether_header_t *)vlan_hdr;
    }

    memcpy(eth_hdr->smac, flow->config.l2_info.smac, ETH_ADDR_LEN);
    memcpy(eth_hdr->dmac, flow->config.l2_info.dmac, ETH_ADDR_LEN);

    // fix the IP header
    if (key.flow_type == FLOW_TYPE_V4) {
        ip_hdr = (ipv4_header_t *)(pkt + offset);
        ip_hdr->version = 4;
        ip_hdr->ihl = sizeof(ipv4_header_t)/4;
        uint16_t len = (ip_hdr->ihl << 2) + TCP_HDR_MIN_LEN + \
               ((send_tcp_ts)?TCP_TS_OPTION_LEN:0);
        ip_hdr->tos = 0;
        ip_hdr->tot_len = htons(len);
        ip_hdr->saddr = htonl(key.sip.v4_addr);
        ip_hdr->daddr = htonl(key.dip.v4_addr);
        ip_hdr->id = htons((uint16_t)0xBEAD);
        ip_hdr->protocol = IPPROTO_TCP;
        ip_hdr->frag_off = 0;
        ip_hdr->ttl = IP_DEFAULT_TTL;
        ip_hdr->check = 0;    // let P4 datapath compute checksum
    } else {
        // no IPv6 support
        return 0;
    }
    offset += sizeof(ipv4_header_t);

    // fix the TCP header
    uint8_t tcp_option_len = 0;

    tcp_hdr = (tcp_header_t *)(pkt + offset);
    tcp_hdr->sport = htons(key.sport);
    tcp_hdr->dport = htons(key.dport);
    tcp_hdr->seq = htonl((setrst)?state.tcp_seq_num:(state.tcp_seq_num-1));
    tcp_hdr->ack_seq = htonl(state.tcp_ack_num);
    tcp_option_len = (send_tcp_ts)?(TCP_TS_OPTION_LEN/4):0;
    tcp_hdr->doff = (5 + tcp_option_len);    // TCP header size in words
    tcp_hdr->res1 = 0;
    tcp_hdr->cwr = 0;
    tcp_hdr->ece = 0;
    tcp_hdr->urg = 0;
    tcp_hdr->ack = 1;
    tcp_hdr->psh = 0;
    tcp_hdr->rst = (setrst)?1:0;
    tcp_hdr->syn = 0;
    tcp_hdr->fin = (setfin)?1:0;
    tcp_hdr->window = htons(state.tcp_win_sz);
    tcp_hdr->check = 0;    // let P4 datapath compute checksum
    tcp_hdr->urg_ptr = 0;

    offset += sizeof(tcp_header_t);

    // Set TCP TS option if it was seen
    if (send_tcp_ts) {
        tcp_ts = (tcp_ts_option_t *)(pkt + offset);
        tcp_ts->pad1 = 1;
        tcp_ts->pad2 = 1;
        tcp_ts->kind = 8;
        tcp_ts->length = 10;
        tcp_ts->ts_value = htonl(state.last_pkt_ts);
        tcp_ts->echo_ts  = htonl(state.last_pkt_ts);
        offset += TCP_TS_OPTION_LEN;
    }

    return offset;
}

static session_aged_ret_t
hal_has_session_aged (session_t *session, uint64_t ctime_ns,
                      session_state_t *session_state_p, bool age_thread)
{
    flow_t                                    *iflow, *rflow;
    uint64_t                                   session_timeout;
    pd::pd_session_get_args_t                  args;
    hal_ret_t                                  ret;
    session_aged_ret_t                         retval = SESSION_AGED_NONE;
    pd::pd_func_args_t                         pd_func_args = {0};
    bool                                       tcp_session = false;

    // read the initiator flow record
    iflow = session->iflow;
    rflow = session->rflow;
    if (!iflow) {
        HAL_TRACE_ERR("session {} has no iflow, ignoring ...",
                      session->hal_handle);
        return retval;
    }

    tcp_session = (session->iflow->config.key.proto == IPPROTO_TCP);

    // Call applicable pd-session-get depending upon age_thread context or
    // otherwise 
    args.session = session;
    args.session_state = session_state_p;
    pd_func_args.pd_session_get = &args;
    if (age_thread) {
        // Pass Age-timer-ticks via session-state for pps / bw metrics 
        // calculations
        session_state_p->current_age_timer_ticks = g_age_timer_ticks;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_GET_FOR_AGE_THREAD,
                              &pd_func_args);
    }
    else {
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_GET, &pd_func_args);
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to fetch session state for session {}",
                      session->hal_handle);
        return retval;
    }

    // Create Flow-Proto-Stats-State in HBM, as hinted by create_flags
    if (age_thread) {
        if (iflow->flow_telemetry_create_flags) {
            allocate_flow_telemetry_hbm_stats_state(iflow, &session_state_p->
                                                            iflow_state);
            create_flow_proto_state(iflow);
        }
        if (rflow && rflow->flow_telemetry_create_flags) {
            allocate_flow_telemetry_hbm_stats_state(rflow, &session_state_p->
                                                            rflow_state);
            create_flow_proto_state(rflow);
        }
    }

    // Check if its a TCP flow with connection tracking enabled.
    // And connection tracking timer is not NULL. This means the session
    // is one of connection establishment or connection close phase. Disable
    // aging at that time as the timer would eventually fire and clean up the
    // session anyway.
    if (tcp_session && session->conn_track_en &&
        session->tcp_cxntrack_timer != NULL) {
        HAL_TRACE_DEBUG("Session {} connection tracking timer is on -- bailing aging",
                        session->hal_handle);
        return retval;
    }

    // check if iflow has expired now
    // If there is no timeout configured then we do not age the session
    session_timeout = session_aging_timeout(session, iflow, rflow);
    if (!session_timeout) {
#if SESSION_AGE_DEBUG
        HAL_TRACE_VERBOSE("Session timeout is not configured");
#endif
        return retval;
    }

    // Check initiator flow. Check for session state as we dont want to age half-closed
    // connections if half-closed timeout is disabled.
#if SESSION_AGE_DEBUG
    HAL_TRACE_VERBOSE("retval {} session handle: {}, session iflow state: {}, session rflow state: {}",
                    retval, session->hal_handle, session_state_p->iflow_state, session_state_p->rflow_state);
    HAL_TRACE_VERBOSE("session_age_cb: last pkt ts: {} ctime_ns: {} session_timeout: {}",
                    session_state_p->iflow_state.last_pkt_ts, ctime_ns, session_timeout);
#endif
    if ((TIME_DIFF(ctime_ns, session_state_p->iflow_state.last_pkt_ts) >= session_timeout) ||
        (tcp_session && session_state_p->iflow_state.state >= session::FLOW_TCP_STATE_BIDIR_FIN_RCVD &&
         session_state_p->iflow_state.state != session->iflow->state)) {
        // session hasn't aged yet, move on
        retval = SESSION_AGED_IFLOW;
    }

    if (rflow) {
        //check responder flow. Check for session state as we dont want to age half-closed
        //connections if half-closed timeout is disabled.
        if ((TIME_DIFF(ctime_ns, session_state_p->rflow_state.last_pkt_ts) >= session_timeout) ||
            (tcp_session && session_state_p->rflow_state.state >= session::FLOW_TCP_STATE_BIDIR_FIN_RCVD &&
             session_state_p->rflow_state.state != session->rflow->state)) {
            // responder flow seems to be active still
            if (retval == SESSION_AGED_IFLOW)
                retval = SESSION_AGED_BOTH;
            else
                retval = SESSION_AGED_RFLOW;
        }
    }

#if SESSION_AGE_DEBUG
   HAL_TRACE_VERBOSE("Session Aged: {}", retval);
#endif

   return retval;
}

void build_and_send_tcp_pkt(void *);

static void
tcp_tickle_timeout_cb (void *timer, uint32_t timer_id, void *timer_ctxt)
{
    tcptkle_timer_ctx_t *ctx = (tcptkle_timer_ctx_t *)timer_ctxt;
    timespec_t           ctime;
    uint64_t             ctime_ns;
    session_state_t      session_state;
    session_aged_ret_t   ret = SESSION_AGED_NONE;
    session_t           *session = NULL;

    session = hal::find_session_by_handle(ctx->session_handle);
    if (session == NULL) {
        HAL_TRACE_DEBUG("Cant find session with handle: {} -- bailing", ctx->session_handle);
        // Cant find session -- bail out
        goto cleanup;
    }

    session->tcp_cxntrack_timer = NULL;
    // get current time
    clock_gettime(CLOCK_MONOTONIC, &ctime);
    sdk::timestamp_to_nsecs(&ctime, &ctime_ns);
    hal_has_session_aged(session, ctime_ns, &session_state, false);

    /*
     * We cannot rely on the timestamp here as our tickle would have
     * also incremented the timestamp and we have no way of making sure
     * if we really got a response. Hence, look at the packet count per flow.
     *
     * If only IFLOW was aged, we would send tickle on the rflow to the initiator
     * of the IFLOW hence the count on the iflow packets would need to more than
     * what we saw before sending the tickle. If both IFLOW & RFLOW were aged, then
     * we would generate tickles on both direction, in that case we would need to make
     * sure the packet count is > (1+prev-packet-count) as tickle would also have
     * incremented the packet count and we should rely on that.
     */
    if ((ctx->aged_flow == SESSION_AGED_IFLOW &&
        (session_state.iflow_state.state <= session::FLOW_TCP_STATE_FIN_RCVD) &&
         session_state.iflow_state.packets < (ctx->session_state.iflow_state.packets + 1)) ||
        (ctx->aged_flow == SESSION_AGED_BOTH &&
        (session_state.iflow_state.state <= session::FLOW_TCP_STATE_FIN_RCVD) &&
         session_state.iflow_state.packets <= (ctx->session_state.iflow_state.packets + 1))) { 
        ret = SESSION_AGED_IFLOW;
    }

    if ((ctx->aged_flow == SESSION_AGED_RFLOW &&
        (session_state.rflow_state.state <= session::FLOW_TCP_STATE_FIN_RCVD) &&
         session_state.rflow_state.packets < (ctx->session_state.rflow_state.packets + 1)) ||
        (ctx->aged_flow == SESSION_AGED_BOTH &&
        (session_state.rflow_state.state <= session::FLOW_TCP_STATE_FIN_RCVD) &&
         session_state.rflow_state.packets <= (ctx->session_state.rflow_state.packets + 1))) {
        if (ret == SESSION_AGED_IFLOW)
            ret = SESSION_AGED_BOTH;
        else
            ret = SESSION_AGED_RFLOW;
    }

#if SESSION_AGE_DEBUG
    HAL_TRACE_VERBOSE("Iflow Packets: {} context iflow packets: {} rflow packets: {} context rflow packets: {}"
                    "ret: {}", session_state.iflow_state.packets, ctx->session_state.iflow_state.packets,
                    session_state.rflow_state.packets, ctx->session_state.rflow_state.packets, ret);
#endif

    ctx->session_state = session_state;
    if (ret == SESSION_AGED_NONE) {
        HAL_TRACE_DEBUG("Bailing session aging on session {}",
                                           session->iflow->config.key);
        // Session aging is stopped as we saw some packet
        goto cleanup;
    } else {
        ctx->aged_flow = ret;
        fte::fte_softq_enqueue(session->fte_id, /* queue it on the right FTE thread */
                               build_and_send_tcp_pkt, (void *)ctx);
    }

    return;

cleanup:
    HAL_FREE(HAL_MEM_ALLOC_SESS_TIMER_CTXT, ctx);
}

void
build_and_send_tcp_pkt (void *data)
{
    pd::cpu_to_p4plus_header_t   cpu_header;
    pd::p4plus_to_p4_header_t    p4plus_header;
    tcptkle_timer_ctx_t         *ctxt = (tcptkle_timer_ctx_t *)data;
    uint8_t                      pkt[TCP_IPV4_DOT1Q_PKT_SZ];
    hal_ret_t                    ret = HAL_RET_OK;
    session_t                   *session = NULL;
    uint32_t                     sz = 0;
    bool                         send_ts = 0;
    uint8_t                      flags = 0;

    session = hal::find_session_by_handle(ctxt->session_handle);
    if (session == NULL) {
        HAL_TRACE_ERR("Cant find session for handle: {}", ctxt->session_handle);
        // Cant find session -- bail out
        goto cleanup;
    }

    // Set the timestamp option if needed
    send_ts = ctxt->session_state.tcp_ts_option;
    if (send_ts)
        flags |= BUILD_TCP_SEND_TIMESTAMP;


    if (ctxt->num_tickles <= MAX_TCP_TICKLES) {
        /*
         * Send tickles to one or both flows. Send tickles on the opposite direction
         * so it reaches the initiator of the flow that is aged.
         */
        if ((ctxt->aged_flow == SESSION_AGED_IFLOW ||
             ctxt->aged_flow == SESSION_AGED_BOTH) && (session->rflow)) {
            sz = build_tcp_packet(session->rflow, session,
                             ctxt->session_state.rflow_state, &cpu_header, &p4plus_header,
                             pkt, flags);
            // There was some issue with packet construction
            // bail out
            if (!sz) goto triggerdelete;
            fte::fte_asq_send(&cpu_header, &p4plus_header, pkt, sz);
            SDK_ATOMIC_INC_UINT64(&session->rflow->stats.num_tcp_tickles_sent, 1);
        }
        if ((ctxt->aged_flow == SESSION_AGED_RFLOW ||
             ctxt->aged_flow == SESSION_AGED_BOTH)) {
            sz = build_tcp_packet(session->iflow, session,
                             ctxt->session_state.iflow_state, &cpu_header, &p4plus_header,
                             pkt, flags);
            // There was some issue with packet construction
            // bail out
            if (!sz) goto triggerdelete;
            fte::fte_asq_send(&cpu_header, &p4plus_header, pkt, sz);
            SDK_ATOMIC_INC_UINT64(&session->iflow->stats.num_tcp_tickles_sent, 1);
        }

#if SESSION_AGE_DEBUG
        HAL_TRACE_VERBOSE("Sending another tickle and starting timer {} iflow stats: {} rflow stats {}",
                         session->hal_handle, session->iflow->stats.num_tcp_tickles_sent,
                         (session->rflow)?session->rflow->stats.num_tcp_tickles_sent:0);
#endif

        /*
         * If Tickles were generated then we increment the tickle count
         * and start a timer waiting for the idle host to respond.
         */
         ctxt->num_tickles++;
         session->tcp_cxntrack_timer = sdk::lib::timer_schedule(
                                              HAL_TIMER_ID_TCP_TICKLE_WAIT,
                                              SESSION_DEFAULT_TCP_TICKLE_TIMEOUT,
                                              (void *)ctxt,
                                              tcp_tickle_timeout_cb, false);
         return;
    }
    // Send TCP RST to the flow if there is no response
    if (ctxt->aged_flow == SESSION_AGED_IFLOW ||
        ctxt->aged_flow == SESSION_AGED_BOTH) {
        flags |= BUILD_TCP_SEND_RST;
        sz = build_tcp_packet(session->iflow, session,
                              ctxt->session_state.iflow_state, &cpu_header, &p4plus_header,
                              pkt, flags);
        if (sz)  {
            fte::fte_asq_send(&cpu_header, &p4plus_header, pkt, sz);
            SDK_ATOMIC_INC_UINT64(&session->iflow->stats.num_tcp_rst_sent, 1);
        }
    }
    if (ctxt->aged_flow == SESSION_AGED_RFLOW ||
        ctxt->aged_flow == SESSION_AGED_BOTH) {
        flags |= BUILD_TCP_SEND_RST;
        sz = build_tcp_packet(session->rflow, session,
                              ctxt->session_state.rflow_state, &cpu_header, &p4plus_header,
                              pkt, flags);
        if (sz)  {
            fte::fte_asq_send(&cpu_header, &p4plus_header, pkt, sz);
            SDK_ATOMIC_INC_UINT64(&session->rflow->stats.num_tcp_rst_sent, 1);
        }
    }

triggerdelete:
    HAL_SESSION_STATS_PTR(session->fte_id)->aged_sessions += 1;
    // time to clean up the session
    ret = fte::session_delete_in_fte(session->hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete aged session {}",
                      session->iflow->config.key);
    }

cleanup:
    // Free the context
    HAL_FREE(HAL_MEM_ALLOC_SESS_TIMER_CTXT, ctxt);
}

void
process_hal_periodic_tkle (void *data)
{
    tcptkle_timer_ctx_t **tctx_list = (tcptkle_timer_ctx_t **)data;

    for (uint8_t i=0; i<HAL_MAX_SESSION_PER_ENQ; i++) {
        if (tctx_list[i])
            build_and_send_tcp_pkt((void *)tctx_list[i]);
    }
    HAL_FREE(HAL_MEM_ALLOC_SESS_TIMER_CTXT_PER_FTE, tctx_list);
}

void
process_hal_periodic_sess_delete (void *data)
{
    hal_handle_t  *session_list = (hal_handle_t *)data;

    for (uint8_t i=0; i<HAL_MAX_SESSION_PER_ENQ; i++) {
        if (session_list[i])
            fte::session_delete_in_fte(session_list[i]);
    }
    HAL_FREE(HAL_MEM_ALLOC_SESS_HANDLE_LIST_PER_FTE, session_list);
}

//------------------------------------------------------------------------------
// determine whether a given session should be aged or not
//------------------------------------------------------------------------------

typedef tcptkle_timer_ctx_t **timer_ctx_list;
typedef hal_handle_t        *timer_handle_list;

struct session_age_cb_args_t {
    uint32_t          *num_sessions;
    uint64_t           ctime_ns;
    uint8_t           *num_ctx;
    uint16_t          *num_del_sess;
    timer_ctx_list    *tctx_list;
    timer_handle_list *session_list;
};

bool
session_age_cb (void *entry, void *ctxt)
{
    session_t              *session = (session_t *)entry;
    session_age_cb_args_t  *args = (session_age_cb_args_t *)ctxt;
    session_state_t         session_state;
    tcptkle_timer_ctx_t    *tklectx = NULL;
    session_aged_ret_t      retval = SESSION_AGED_NONE;

    (*args->num_sessions)++;

    // In case session or handle is invalid, we dont want to look further
    if (session == NULL || session->hal_handle == HAL_HANDLE_INVALID) {
        return false;
    }

    retval = hal_has_session_aged(session, args->ctime_ns, &session_state,
                                  true);

    if (retval != SESSION_AGED_NONE) {
#if SESSION_AGE_DEBUG
	    HAL_TRACE_VERBOSE("Aged session: {}", session->iflow->config.key);
#endif
        /*
         *  Send out TCP tickle if it is a TCP session and start a timer for 2
         *  seconds. We send 3 tickles (keepalives) before we send out TCP RST
         *  and proceed to delete the session
         */
        if ((session->iflow->config.key.proto == IPPROTO_TCP) &&
            (session_state.iflow_state.state == session::FLOW_TCP_STATE_ESTABLISHED ||
             session_state.rflow_state.state == session::FLOW_TCP_STATE_ESTABLISHED)) {
            tklectx = (tcptkle_timer_ctx_t *)HAL_CALLOC(HAL_MEM_ALLOC_SESS_TIMER_CTXT,
                                                     sizeof(tcptkle_timer_ctx_t));
            SDK_ASSERT_RETURN((tklectx != NULL), false);

            tklectx->session_handle = session->hal_handle;
            tklectx->num_tickles = 1;
            tklectx->session_state = session_state;
            tklectx->aged_flow = retval;
            args->tctx_list[session->fte_id][args->num_ctx[session->fte_id]++] = tklectx;

            // Stop processing if we have reached the maximum limit per FTE
            // We will process the rest in the next round
            if (args->num_ctx[session->fte_id] == HAL_SESSIONS_TO_SCAN_PER_INTVL)
                return true;
        } else {
            /*
             * Dont delete UDP/ICMP sessions if one flow ages and other remains
             * For TCP, the only reason why we get here is because we want to
             * cleanup for cases where we could have missed TCP Close in FTE
             * so let it cleanup
             */
            if ((session->iflow->config.key.proto != IPPROTO_TCP) &&
                 session->rflow != NULL && retval != SESSION_AGED_BOTH) {
                return false;
            }
#if SESSION_AGE_DEBUG
            HAL_TRACE_VERBOSE("UDP Session: {} num_del_sess: {} session_list: {:p}",
                    session->hal_handle, args->num_del_sess[session->fte_id],
                    (void *)args->session_list[session->fte_id]);
#endif
            // time to clean up the session, add handle to session list
            args->session_list[session->fte_id][args->num_del_sess[session->fte_id]++] = \
                                                                 session->hal_handle;
            session->deleting = 1;

            // Stop processing if we have reached the maximum limit per FTE
            // We will process the rest in the next round
            if (args->num_del_sess[session->fte_id] == HAL_SESSIONS_TO_SCAN_PER_INTVL)
                return true;

            HAL_SESSION_STATS_PTR(session->fte_id)->aged_sessions += 1;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
// callback invoked by the HAL periodic thread for session aging
//------------------------------------------------------------------------------
static void
session_age_walk_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    uint64_t              bucket = (uint64_t)(ctxt);
    timespec_t            ctime;
    hal_ret_t             ret = HAL_RET_OK;
    uint8_t               fte_id = 0;
    uint32_t              num_sessions = 0, bucket_no = 0;
    flow_telemetry_state_t *flow_telemetry_state_p;

    session_age_cb_args_t args;
#if SESSION_AGE_DEBUG
    HAL_TRACE_VERBOSE("session age walk cb bucket {} context {:p}", bucket, ctxt);
#endif

    // Keep track of age_timer_ticks for pps / bw calculations
    g_age_timer_ticks++;

    //
    // Scan Flow-Proto-State in Age-list and free if applicable
    //
    // Flow-Proto-State will remain in the Age-list for up to 1-min since
    // the time concerned flow got deleted. That way, TMAGENT will have an
    // opportunity to expose the last-updated-data in that flow-context
    //
    flow_telemetry_state_p = g_flow_telemetry_state_age_head_p;
    while (flow_telemetry_state_p != NULL) {
        flow_telemetry_state_t *next_p;

        next_p = flow_telemetry_state_p->next_p;

        flow_telemetry_state_p->u2.delayed_age_ticks--;
        if (flow_telemetry_state_p->u2.delayed_age_ticks == 0)
            dequeue_flow_telemetry_state_from_age_list(flow_telemetry_state_p);

        flow_telemetry_state_p = next_p;
    }

    // We dont have any sessions yet - bail
    if (!g_hal_state->session_hal_handle_ht()->num_entries())
        return;
    args.num_sessions = &num_sessions;

    args.num_ctx = (uint8_t *)HAL_CALLOC(HAL_MEM_ALLOC_SESS_AGE_ARGS,
                                   (sizeof(uint8_t)*HAL_MAX_DATA_THREAD));
    SDK_ASSERT(args.num_ctx != NULL);

    args.num_del_sess = (uint16_t *)HAL_CALLOC(HAL_MEM_ALLOC_SESS_AGE_ARGS,
                                     sizeof(uint16_t)*HAL_MAX_DATA_THREAD);
    SDK_ASSERT(args.num_del_sess != NULL);

    args.tctx_list = (timer_ctx_list *)HAL_CALLOC(HAL_MEM_ALLOC_SESS_TIMER_CTXT,
                              sizeof(timer_ctx_list)*HAL_MAX_DATA_THREAD);
    SDK_ASSERT(args.tctx_list != NULL);

    args.session_list = (timer_handle_list *)HAL_CALLOC(HAL_MEM_ALLOC_SESS_HANDLE_LIST,
                                sizeof(timer_handle_list)*HAL_MAX_DATA_THREAD);
    SDK_ASSERT(args.session_list != NULL);

    for (fte_id=0; fte_id<HAL_MAX_DATA_THREAD; fte_id++) {
        args.tctx_list[fte_id] = (tcptkle_timer_ctx_t **)HAL_CALLOC(
                          HAL_MEM_ALLOC_SESS_TIMER_CTXT_PER_FTE,
                          sizeof(tcptkle_timer_ctx_t*)*HAL_SESSIONS_TO_SCAN_PER_INTVL);
        SDK_ASSERT(args.tctx_list[fte_id] != NULL);

        args.session_list[fte_id] = (hal_handle_t *)HAL_CALLOC(
                                 HAL_MEM_ALLOC_SESS_HANDLE_LIST_PER_FTE,
                                sizeof(hal_handle_t)*HAL_SESSIONS_TO_SCAN_PER_INTVL);
        SDK_ASSERT(args.session_list[fte_id] != NULL);
        args.num_ctx[fte_id] = 0;
        args.num_del_sess[fte_id] = 0;
    }

    // get current time
    clock_gettime(CLOCK_MONOTONIC, &ctime);
    sdk::timestamp_to_nsecs(&ctime, &args.ctime_ns);
#if SESSION_AGE_DEBUG
    HAL_TRACE_VERBOSE("Entering timer id {}, bucket: {} bucket_no: {}", timer_id,  bucket, bucket_no);
#endif
    while (num_sessions < HAL_SESSIONS_TO_SCAN_PER_INTVL &&
           bucket_no < g_hal_state->session_hal_handle_ht()->num_buckets()) {
        g_hal_state->session_hal_handle_ht()->walk_bucket_safe(bucket,
                                                     session_age_cb, &args);
        bucket_no++;
        bucket = (bucket + 1)%g_hal_state->session_hal_handle_ht()->num_buckets();
    }
#if SESSION_AGE_DEBUG
    HAL_TRACE_VERBOSE("Num sessions: {} bucket {} buckets {} max buckets {}", num_sessions, bucket, bucket_no, g_hal_state->session_hal_handle_ht()->num_buckets());
#endif

    //Check if there are pending requests that need to be queued to FTE threads
    for (fte_id=0; fte_id<HAL_MAX_DATA_THREAD; fte_id++) {
        uint16_t ctxt_num = 0, del_sess_num = 0;
        while (args.num_ctx[fte_id]) {
            // Enqueue HAL_MAX_SESSION_PER_ENQ tickles at a time!!
            timer_ctx_list tctx_list = (tcptkle_timer_ctx_t **)HAL_CALLOC(
                                          HAL_MEM_ALLOC_SESS_TIMER_CTXT_PER_FTE,
                                          sizeof(tcptkle_timer_ctx_t*)*HAL_MAX_SESSION_PER_ENQ);
            SDK_ASSERT(tctx_list != NULL);
            for (uint8_t idx=0; (idx < HAL_MAX_SESSION_PER_ENQ && idx < args.num_ctx[fte_id]);
                 idx++, ctxt_num++, args.num_ctx[fte_id]--) {
                tctx_list[idx] = args.tctx_list[fte_id][ctxt_num];
            }
            ret = fte::fte_softq_enqueue(fte_id,
                                    process_hal_periodic_tkle,
                                    (void *)tctx_list);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
        HAL_FREE(HAL_MEM_ALLOC_SESS_TIMER_CTXT_PER_FTE, args.tctx_list[fte_id]);

        while (args.num_del_sess[fte_id]) {
            timer_handle_list session_list = (hal_handle_t *)HAL_CALLOC(
                                             HAL_MEM_ALLOC_SESS_HANDLE_LIST_PER_FTE,
                                             sizeof(hal_handle_t)*HAL_MAX_SESSION_PER_ENQ);
            SDK_ASSERT(session_list != NULL);
            for (uint8_t idx=0; (idx < HAL_MAX_SESSION_PER_ENQ && idx < args.num_del_sess[fte_id]);
                 idx++, del_sess_num++, args.num_del_sess[fte_id]--) {
                session_list[idx] = args.session_list[fte_id][del_sess_num];
            }
            ret = fte::fte_softq_enqueue(fte_id,
                                  process_hal_periodic_sess_delete,
                                  (void *)session_list);
            SDK_ASSERT(ret == HAL_RET_OK);
        }
        HAL_FREE(HAL_MEM_ALLOC_SESS_HANDLE_LIST_PER_FTE, args.session_list[fte_id]);
    }

    HAL_FREE(HAL_MEM_ALLOC_SESS_AGE_ARGS, args.num_ctx);
    HAL_FREE(HAL_MEM_ALLOC_SESS_AGE_ARGS, args.num_del_sess);
    HAL_FREE(HAL_MEM_ALLOC_SESS_TIMER_CTXT, args.tctx_list);
    HAL_FREE(HAL_MEM_ALLOC_SESS_HANDLE_LIST, args.session_list);

    ctxt = (reinterpret_cast<void *>(bucket));

    // store the bucket id to resume on next invocation
    sdk::lib::timer_update_ctxt(timer, ctxt);
}

//------------------------------------------------------------------------------
// initialize the session management module
//------------------------------------------------------------------------------
hal_ret_t
session_init (hal_cfg_t *hal_cfg)
{
    if (hal::is_platform_type_sim()) {
        // SIM
        // Libmodel client doesnt have support to map hbm to shared memory today so
        // we cannot work with virtual address. Hence these stats will not be registered
        // with delphi until that is done
        g_session_stats = (session_stats_t *)HAL_CALLOC(HAL_MEM_ALLOC_SESSION_STATS,
                                   (sizeof(session_stats_t)*hal::g_hal_cfg.num_data_cores));
        SDK_ASSERT(g_session_stats != NULL);

    } else {
        sdk::types::mem_addr_t vaddr;
        sdk::types::mem_addr_t pal_addr = get_mem_addr(CAPRI_HBM_REG_SESSION_SUMMARY_STATS);
        HAL_TRACE_VERBOSE("Start addr: {:p}", pal_addr);
        SDK_ASSERT(pal_addr != INVALID_MEM_ADDRESS);

        for (uint32_t fte = 0; fte < hal::g_hal_cfg.num_data_cores; fte++) {
             //Register with Delphi
             auto session_global_stats =
                   delphi::objects::SessionSummaryMetrics::NewSessionSummaryMetrics(fte, pal_addr);
             SDK_ASSERT(session_global_stats != NULL);

             sdk::lib::pal_ret_t ret = sdk::lib::pal_physical_addr_to_virtual_addr(pal_addr, &vaddr);
             SDK_ASSERT(ret == sdk::lib::PAL_RET_OK);

             if (!fte) {
                 g_session_stats = (session_stats_t *)vaddr;
                 bzero(g_session_stats, sizeof(session_stats_t));
             }

             pal_addr += 1 << HAL_SESSION_STATS_SHIFT;
        }

        // Flow-Proto Stats Indexer Init
        g_flow_proto_state_indexer = sdk::lib::indexer::factory(
                                          HAL_FLOW_TELEMETRY_MAX_STATS_STATE, true);
        SDK_ASSERT(g_flow_proto_state_indexer != NULL);

        pal_addr = get_mem_addr(CAPRI_HBM_REG_FLOW_TELEMETRY_STATS);
        SDK_ASSERT(pal_addr != INVALID_MEM_ADDRESS);

        g_flow_telemetry_hbm_start = pal_addr;
    }

    g_hal_state->oper_db()->set_max_data_threads(hal_cfg->num_data_cores);

    // wait until the periodic thread is ready
    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }

    // Disable aging when FTE is disabled
    if (getenv("DISABLE_AGING")) {
        HAL_TRACE_DEBUG("Aging is disabled");
        return HAL_RET_OK;
    }
    t_session_timer =
        sdk::lib::timer_schedule(HAL_TIMER_ID_SESSION_AGEOUT,            // timer_id
                                      HAL_SESSION_AGE_SCAN_INTVL,
                                      (void *)0,    // ctxt
                                      session_age_walk_cb, true);
    if (!t_session_timer) {
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Started session aging periodic timer with {}ms invl",
                    HAL_SESSION_AGE_SCAN_INTVL);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// callback invoked by the Session TCP close timer to cleanup session state
//------------------------------------------------------------------------------
static void
tcp_close_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    hal_ret_t     ret;
    hal_handle_t  session_handle = (hal_handle_t)ctxt;
    session_t    *session = NULL;

    session = hal::find_session_by_handle(session_handle);
    if (session == NULL) {
        HAL_TRACE_VERBOSE("Cant find the session for handle {} -- bailing",
                                                         session_handle);
        return;
    }

    HAL_TRACE_DEBUG("TCP close timer callback -- deleting session with handle {}",
                    session_handle);

    // time to clean up the session
    // Delete asynchronously so we dont hold the periodic thread
    ret = fte::session_delete_async(session);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delte aged session {}",
                      session->hal_handle);
    }
}

typedef enum timeout_type_ {
    TCP_CXNSETUP_TIMEOUT = 1,
    TCP_HALF_CLOSED_TIMEOUT = 2,
    TCP_CLOSE_TIMEOUT = 3,
} timeout_type_t;
//------------------------------------------------------------------------------
// Get TCP timeout from nwsec profile
//------------------------------------------------------------------------------
static uint64_t
get_tcp_timeout (session_t *session, timeout_type_t timeout)
{
    vrf_t              *vrf = NULL;
    nwsec_profile_t    *nwsec_prof = NULL;

    vrf = vrf_lookup_by_handle(session->vrf_handle);
    if (vrf != NULL) {
        nwsec_prof = find_nwsec_profile_by_handle(vrf->nwsec_profile_handle);
    }

    switch (timeout) {
        case TCP_CXNSETUP_TIMEOUT:
        {
            if (nwsec_prof != NULL) {
                return ((uint64_t)(nwsec_prof->tcp_cnxn_setup_timeout * TIME_MSECS_PER_SEC));
            } else {
                return (SESSION_SW_DEFAULT_TCP_CXNSETUP_TIMEOUT);
            }
        }
        break;

        case TCP_HALF_CLOSED_TIMEOUT:
        {
            if (nwsec_prof != NULL) {
                return ((uint64_t)(nwsec_prof->tcp_half_closed_timeout * TIME_MSECS_PER_SEC));
            } else {
                return (SESSION_SW_DEFAULT_TCP_HALF_CLOSED_TIMEOUT);
            }
        }
        break;

        case TCP_CLOSE_TIMEOUT:
        {
            if (nwsec_prof != NULL) {
                return ((uint64_t)(nwsec_prof->tcp_close_timeout * TIME_MSECS_PER_SEC));
            } else {
                return (SESSION_SW_DEFAULT_TCP_CLOSE_TIMEOUT);
            }
        }
        break;

        default: break;
    }

    return 0;
}

//------------------------------------------------------------------------------
// API to start timer on TCP close when RST is received or bidirectional
// FIN is received
//------------------------------------------------------------------------------
hal_ret_t
schedule_tcp_close_timer (session_t *session)
{
    flow_key_t  key = {};
    uint32_t    timeout = 0;

    if (getenv("DISABLE_AGING")) {
        return HAL_RET_OK;
    }

    timeout = get_tcp_timeout(session, TCP_HALF_CLOSED_TIMEOUT);
    if (timeout == HAL_MAX_INACTIVTY_TIMEOUT) {
        // No timeout configured so we bail out
        return HAL_RET_OK;
    }

    // Delete the previous timers if any and start a new one
    if (session->tcp_cxntrack_timer != NULL) {
        // Let the library delete it
        //sdk::lib::timer_delete(session->tcp_cxntrack_timer);
        session->tcp_cxntrack_timer = NULL;
    }

    session->tcp_cxntrack_timer = sdk::lib::timer_schedule(
                                     HAL_TIMER_ID_TCP_CLOSE_WAIT,
                                     get_tcp_timeout(session, TCP_CLOSE_TIMEOUT),
                                     (void *)(session->hal_handle), tcp_close_cb, false);
    if (!session->tcp_cxntrack_timer) {
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("TCP Close timer started for session {}",
                    (session->iflow)?session->iflow->config.key:key);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Callback invoked when TCP half closed timer fires
//
// If the tcp close timeout is not configured then we get INT_MAX from agent
// and we dont start any timer. The session will be aged out when it reaches
// idle timeout
//------------------------------------------------------------------------------
void
tcp_half_close_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    hal_ret_t                 ret;
    hal_handle_t              session_handle = (hal_handle_t)ctxt;
    session_t                *session = NULL;
    pd::pd_session_get_args_t args;
    session_state_t           state = {0};
    pd::pd_func_args_t        pd_func_args = {0};

    session = hal::find_session_by_handle(session_handle);
    if (session == NULL) {
        HAL_TRACE_VERBOSE("Cant find the session for handle {} -- bailing",
                                                         session_handle);
        return;
    }

    args.session = session;
    args.session_state = &state;
    pd_func_args.pd_session_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to fetch iflow record of session {}",
                       session->hal_handle);
        return;
    }

    HAL_TRACE_DEBUG("Session handle: {} IFlow State: {}",
                     session_handle, state.iflow_state.state);

    session->tcp_cxntrack_timer = NULL;

    // If we havent received bidir FIN by now then we go ahead and cleanup
    // the session
    if (state.iflow_state.state != session::FLOW_TCP_STATE_BIDIR_FIN_RCVD &&
        state.rflow_state.state != session::FLOW_TCP_STATE_BIDIR_FIN_RCVD) {
        tcp_close_cb(timer, timer_id, (void *)(session->hal_handle));
    }
}

//------------------------------------------------------------------------------
// API to start timer when TCP FIN is seen for the first time on the session
// This is to give enough time for other side to send the FIN
//
// If the half_closed timeout is not configured then we get INT_MAX from agent
// and we dont start any timer. The session will be aged out when it reaches
// idle timeout
//------------------------------------------------------------------------------
hal_ret_t
schedule_tcp_half_closed_timer (session_t *session)
{
    uint32_t  timeout = 0;

    if (getenv("DISABLE_AGING")) {
        return HAL_RET_OK;
    }

    timeout = get_tcp_timeout(session, TCP_HALF_CLOSED_TIMEOUT);
    if (timeout == HAL_MAX_INACTIVTY_TIMEOUT) {
        // No timeout configured so we bail out
        return HAL_RET_OK;
    }

    // Delete the previous timers if any and start a new one
    if (session->tcp_cxntrack_timer != NULL) {
        // Let the library delete it
        //sdk::lib::timer_delete(session->tcp_cxntrack_timer);
        session->tcp_cxntrack_timer = NULL;
    }

    session->tcp_cxntrack_timer = sdk::lib::timer_schedule(
                                     HAL_TIMER_ID_TCP_HALF_CLOSED_WAIT,
                                     get_tcp_timeout(session, TCP_HALF_CLOSED_TIMEOUT),
                                     (void *)(session->hal_handle),
                                     tcp_half_close_cb, false);

    if (!session->tcp_cxntrack_timer) {
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("TCP Half Closed timer started for session {}",
                     session->hal_handle);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// callback invoked by the Session TCP CXN setup timer. If the session is not
// in established state then cleanup the session
//------------------------------------------------------------------------------
static void
tcp_cxnsetup_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    hal_ret_t                 ret;
    hal_handle_t              session_handle = (hal_handle_t)ctxt;
    pd::pd_session_get_args_t args;
    session_state_t           state;
    session_t                *session = NULL;
    pd::pd_func_args_t       pd_func_args = {0};

    session = hal::find_session_by_handle(session_handle);
    if (session == NULL) {
        HAL_TRACE_VERBOSE("Cant find the session for handle {} -- bailing", session_handle);
        return;
    }

    args.session = session;
    args.session_state = &state;
    pd_func_args.pd_session_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to fetch iflow record of session {}",
                       session->hal_handle);
        return;
    }

    //HAL_TRACE_DEBUG("Session {} IFlow State: {}", session_handle, state.iflow_state.state);

    session->tcp_cxntrack_timer = NULL;

    if (state.iflow_state.state < session::FLOW_TCP_STATE_ESTABLISHED ||
        state.rflow_state.state < session::FLOW_TCP_STATE_ESTABLISHED) {
        // session is not in established state yet.
        // Cleanup the session
        HAL_SESSION_STATS_PTR(session->fte_id)->num_cxnsetup_timeout += 1;
        ret = fte::session_delete_async(session);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete session {}",
                          session->hal_handle);
        }
    }

    // Only update the established state.
    // We dont want to update anything beyond that (RESET/FIN)
    if (session->iflow &&
        state.iflow_state.state == session::FLOW_TCP_STATE_ESTABLISHED)
        session->iflow->state = state.iflow_state.state;
    if (session->rflow &&
        state.rflow_state.state == session::FLOW_TCP_STATE_ESTABLISHED)
        session->rflow->state = state.rflow_state.state;
}

//------------------------------------------------------------------------------
// API to start timer when TCP SYN received to make sure the session
// goes to established state within the given timeout
//------------------------------------------------------------------------------
hal_ret_t
schedule_tcp_cxnsetup_timer (session_t *session)
{
    if (getenv("DISABLE_AGING")) {
        return HAL_RET_OK;
    }

    session->tcp_cxntrack_timer = sdk::lib::timer_schedule(
                                        HAL_TIMER_ID_TCP_CXNSETUP_WAIT,
                                        get_tcp_timeout(session, TCP_CXNSETUP_TIMEOUT),
                                        (void *)(session->hal_handle),
                                        tcp_cxnsetup_cb, false);

    if (!session->tcp_cxntrack_timer) {
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("TCP Cxn Setup timer started for session {}",
                     session->hal_handle);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// API to set the runtime TCP state when FTE sees TCP close packets
//------------------------------------------------------------------------------
void
session_set_tcp_state (session_t *session, hal::flow_role_t role,
                       FlowTCPState tcp_state)
{
    hal::flow_t *flow = NULL;

    if (role == hal::FLOW_ROLE_INITIATOR) {
        flow = session->iflow;
    } else {
        flow = session->rflow;
    }

    if (flow)
        flow->state = tcp_state;

    HAL_TRACE_DEBUG("Updated tcp state to {}", (uint32_t)tcp_state);
}

bool
check_session_match (session_match_t *match, hal::session_t *session)
{
    hal::flow_key_t *key = &session->iflow->config.key;
    if (match->match_fields & SESSION_MATCH_SVRF) {
        /** Match on vrf **/
        if (key->svrf_id != match->key.svrf_id) { return FALSE; }
    }
    if (match->match_fields & SESSION_MATCH_V4_FLOW) {
        if (key->flow_type !=  hal::FLOW_TYPE_V4) { return FALSE; }
    }

    // Extend this function to add more filter conditions
    return TRUE;
}
void
extract_acl_key_from_flow_key(hal::ipv4_tuple &acl_key, hal::flow_key_t *key)
{
    if (key->flow_type == hal::FLOW_TYPE_V4) {
        acl_key.ip_src = key->sip.v4_addr;
        acl_key.ip_dst = key->dip.v4_addr;
    }
    acl_key.proto = key->proto;
    switch(key->proto) {
    case types::IPPROTO_ICMP:
    case types::IPPROTO_ICMPV6:
        acl_key.port_src = key->icmp_id;
        acl_key.port_dst = ((key->icmp_type << 8) | key->icmp_code);
        break;
    case types::IPPROTO_ESP:
        acl_key.port_src = key->spi >> 16 & 0xFFFF;
        acl_key.port_dst = key->spi & 0xFFFF;
        break;
    case types::IPPROTO_TCP:
    case types::IPPROTO_UDP:
        acl_key.port_src = key->sport;
        acl_key.port_dst = key->dport;
        break;
    default:
        HAL_TRACE_DEBUG("proto not handled");
    }

}

hal_ret_t
session_eval_matching_session (session_match_t  *match)
{
    // Close the config db to avoid any deadlocks with FTE
    hal::hal_cfg_db_close();

    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t  *session = (session_t *)entry;
        session_match_t *match = (session_match_t *) ctxt;
        dllist_ctxt_t   *list_head = match->session_list;

        if (!session->skip_sfw_reval && check_session_match(match, session)) {
            hal::flow_key_t *key = &session->iflow->config.key;
            hal::ipv4_tuple acl_key = {};
            extract_acl_key_from_flow_key(acl_key, key);
            bool allow = securitypolicy_is_allow(key->svrf_id, &acl_key, (session::FlowAction)session->iflow->config.action);
            if (!allow) {
                HAL_TRACE_DEBUG("add the handle {}", session->hal_handle);
                hal_handle_id_list_entry_t *list_entry = (hal_handle_id_list_entry_t *)g_hal_state->
                    hal_handle_id_list_entry_slab()->alloc();
                if (list_entry == NULL) {
                    HAL_TRACE_ERR("Out of memory - skipping delete session {}", session->hal_handle);
                    return false;
                }

                list_entry->handle_id = session->hal_handle;
                dllist_add(list_head, &list_entry->dllist_ctxt);
            }
        }
        return false;
    };

    // build list of session_ids
    dllist_ctxt_t session_list;
    dllist_reset(&session_list);

    match->session_list = &session_list;
    HAL_TRACE_DEBUG("calling walk func");
    g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, match);

    HAL_TRACE_DEBUG("delete session");
    session_delete_list(&session_list, true /*async:true*/);
    return HAL_RET_OK;
}

hal_ret_t
system_fte_txrx_stats_get(SystemResponse *rsp)
{
    fte::fte_txrx_stats_t txrx_stats = {};

    PMDStats          *pmd_stats = NULL;


    HAL_TRACE_DEBUG("Gathering fte txrx stats ");
    pmd_stats = rsp->mutable_stats()->mutable_pmd_stats();

    for (uint32_t threads = 0; threads < hal::g_hal_cfg.num_data_cores; threads++) {
        txrx_stats = fte::fte_txrx_stats_get(threads, false);
        FTEInfo *fte_info = pmd_stats->add_fte_info();
        QGlobalInfo *global_info = fte_info->mutable_glbal();
        global_info->set_gc_pindex(txrx_stats.glinfo.gc_pindex);
        global_info->set_cpu_tx_page_pindex(txrx_stats.glinfo.cpu_tx_page_pindex);
        global_info->set_cpu_tx_page_cindex(txrx_stats.glinfo.cpu_tx_page_cindex);
        global_info->set_cpu_tx_descr_pindex(txrx_stats.glinfo.cpu_tx_descr_pindex);
        global_info->set_cpu_tx_descr_cindex(txrx_stats.glinfo.cpu_tx_descr_cindex);
        global_info->set_cpu_rx_dpr_cindex(txrx_stats.glinfo.cpu_rx_dpr_cindex);
        global_info->set_cpu_rx_dpr_sem_cindex(txrx_stats.glinfo.cpu_rx_dpr_sem_cindex);
        global_info->set_cpu_rx_dpr_descr_free_err(txrx_stats.glinfo.cpu_rx_dpr_descr_free_err);
        global_info->set_cpu_rx_dpr_sem_free_err(txrx_stats.glinfo.cpu_rx_dpr_sem_free_err);
        global_info->set_cpu_rx_dpr_descr_invalid_free_err(txrx_stats.glinfo.cpu_rx_dpr_descr_invalid_free_err);

        for (int i = 0; i < FTE_MAX_CPU_QUEUES; i++) {
            //for number of queues
            hal::pd::cpupkt_qinfo_stats_t fte_qinfo = txrx_stats.qinfo[i];


            QInfo *qinfo = fte_info->add_qinfo();
            qinfo->set_base_addr(fte_qinfo.inst.base_addr);
            qinfo->set_pc_index(fte_qinfo.inst.pc_index);
            qinfo->set_pc_index_addr(fte_qinfo.inst.pc_index_addr);
            //qinfo->set_valid_bit_value(fte_qinfo.inst.valid_bit_value);
            qinfo->set_queue_id(fte_qinfo.inst.queue_id);
            qinfo->set_queue_type(fte_qinfo.type);

            // Per Queue Ctrs
            QCtr *ctr = qinfo->mutable_ctr();
            ctr->set_send_pkts(fte_qinfo.inst.ctr.send_pkts);
            ctr->set_recv_pkts(fte_qinfo.inst.ctr.recv_pkts);
            ctr->set_rx_sem_wr_err(fte_qinfo.inst.ctr.rx_sem_wr_err);
            ctr->set_rx_slot_value_read_err(fte_qinfo.inst.ctr.rx_slot_value_read_err);
            ctr->set_rx_descr_read_err(fte_qinfo.inst.ctr.rx_descr_read_err);
            ctr->set_rx_descr_to_hdr_err(fte_qinfo.inst.ctr.rx_descr_to_hdr_err);
            ctr->set_rx_descr_free_err(fte_qinfo.inst.ctr.rx_descr_free_err);
            ctr->set_tx_descr_free_err(fte_qinfo.inst.ctr.tx_descr_free_err);
            ctr->set_tx_page_alloc_err(fte_qinfo.inst.ctr.tx_page_alloc_err);
            ctr->set_tx_page_copy_err(fte_qinfo.inst.ctr.tx_page_copy_err);
            ctr->set_tx_descr_pgm_err(fte_qinfo.inst.ctr.tx_descr_pgm_err);
            ctr->set_tx_send_err(fte_qinfo.inst.ctr.tx_send_err);
            ctr->set_rx_descr_addr_oob(fte_qinfo.inst.ctr.rx_descr_addr_oob);
            ctr->set_poll_count(fte_qinfo.inst.ctr.poll_count);
            ctr->set_tx_doorbell_err(fte_qinfo.inst.ctr.tx_db_err);
        }
    }
    return HAL_RET_OK;
}

hal_ret_t
system_fte_stats_get(SystemResponse *rsp)
{
    FTEStats          *fte_global_stats = NULL;
    FTEStatsInfo      *fte_stats = NULL;
    fte::fte_stats_t   per_fte_stats = {0};

    HAL_TRACE_DEBUG("Gathering fte stats ");
    fte_global_stats = rsp->mutable_stats()->mutable_fte_stats();
    for (uint32_t i = 0; i < hal::g_hal_cfg.num_data_cores; i++) {
        per_fte_stats = fte::fte_stats_get(i);
        fte_stats = fte_global_stats->add_fte_stats_info();

        if (per_fte_stats.fte_hbm_stats) {
            fte_stats->set_conn_per_second(per_fte_stats.fte_hbm_stats->cpsstats.cps);
            fte_stats->set_max_conn_per_second(per_fte_stats.fte_hbm_stats->cpsstats.cps_hwm);
            fte_stats->set_flow_miss_pkts(per_fte_stats.fte_hbm_stats->qstats.flow_miss_pkts);
            fte_stats->set_retransmit_pkts(per_fte_stats.fte_hbm_stats->qstats.flow_retransmit_pkts);
            fte_stats->set_redir_pkts(per_fte_stats.fte_hbm_stats->qstats.redirect_pkts);
            fte_stats->set_cflow_pkts(per_fte_stats.fte_hbm_stats->qstats.cflow_pkts);
            fte_stats->set_tcp_close_pkts(per_fte_stats.fte_hbm_stats->qstats.tcp_close_pkts);
            fte_stats->set_tls_proxy_pkts(per_fte_stats.fte_hbm_stats->qstats.tls_proxy_pkts);
            fte_stats->set_softq_reqs(per_fte_stats.fte_hbm_stats->qstats.softq_req);
            fte_stats->set_queued_tx_pkts(per_fte_stats.fte_hbm_stats->qstats.queued_tx_pkts);
            fte_stats->set_packets_per_second(per_fte_stats.fte_hbm_stats->cpsstats.pps);
            fte_stats->set_max_packets_per_second(per_fte_stats.fte_hbm_stats->cpsstats.pps_hwm);
            fte_stats->set_max_session_pkt_drops(per_fte_stats.fte_hbm_stats->qstats.max_session_drop_pkts);
        }
        int qlen = 0;
        fte::fte_softq_stats_get(i, qlen);
        fte_stats->set_softq_len(qlen);

        for (uint8_t idx=0; idx<HAL_RET_ERR; idx++) {
            if (std::strcmp(HAL_RET_ENTRIES_str((hal_ret_t)idx), "unknown")) {
                FTEError   *fte_err = fte_stats->add_fte_errors();

                fte_err->set_count(per_fte_stats.fte_errors[idx]);
                fte_err->set_fte_error(HAL_RET_ENTRIES_str((hal_ret_t)idx));
            }
        }

        for (uint8_t feature=0; feature<fte::get_num_features(); feature++) {
             FTEFeatureStats *feature_stat = fte_stats->add_feature_stats();
             std::string      feature_name = fte::feature_id_to_name(feature);

             feature_stat->set_feature_name(feature_name.substr(feature_name.find(":")+1));
             feature_stat->set_drop_pkts(per_fte_stats.feature_stats[feature].drop_pkts);
             for (uint8_t idx=0; idx<HAL_RET_ERR; idx++) {
                 if (std::strcmp(HAL_RET_ENTRIES_str((hal_ret_t)idx), "unknown")) {
                     FTEError   *fte_err = feature_stat->add_drop_reason();

                     fte_err->set_count(per_fte_stats.feature_stats[feature].drop_reason[idx]);
                     fte_err->set_fte_error(HAL_RET_ENTRIES_str((hal_ret_t)idx));
                }
             }
        }
    }
    return HAL_RET_OK;
}

hal_ret_t
system_session_summary_get(SystemResponse *rsp)
{
    session_stats_t      session_summary = {0};
    SessionSummaryStats *session_stats = NULL;

    for (uint32_t fte = 0; fte < hal::g_hal_cfg.num_data_cores; fte++) {
        session_summary.total_active_sessions += HAL_SESSION_STATS_PTR(fte)->total_active_sessions;
        session_summary.l2_sessions += HAL_SESSION_STATS_PTR(fte)->l2_sessions;
        session_summary.tcp_sessions += HAL_SESSION_STATS_PTR(fte)->tcp_sessions;
        session_summary.udp_sessions += HAL_SESSION_STATS_PTR(fte)->udp_sessions;
        session_summary.icmp_sessions += HAL_SESSION_STATS_PTR(fte)->icmp_sessions;
        session_summary.drop_sessions += HAL_SESSION_STATS_PTR(fte)->drop_sessions;
        session_summary.aged_sessions += HAL_SESSION_STATS_PTR(fte)->aged_sessions;
        session_summary.num_tcp_rst_sent += HAL_SESSION_STATS_PTR(fte)->num_tcp_rst_sent;
        session_summary.num_icmp_error_sent += HAL_SESSION_STATS_PTR(fte)->num_icmp_error_sent;
        session_summary.num_cxnsetup_timeout += HAL_SESSION_STATS_PTR(fte)->num_cxnsetup_timeout;
        session_summary.num_session_create_err += HAL_SESSION_STATS_PTR(fte)->num_session_create_err;
    }

    session_stats = rsp->mutable_stats()->mutable_session_stats();
    session_stats->set_total_active_sessions(session_summary.total_active_sessions);
    session_stats->set_l2_sessions(session_summary.l2_sessions);
    session_stats->set_tcp_sessions(session_summary.tcp_sessions);
    session_stats->set_udp_sessions(session_summary.udp_sessions);
    session_stats->set_icmp_sessions(session_summary.icmp_sessions);
    session_stats->set_drop_sessions(session_summary.drop_sessions);
    session_stats->set_aged_sessions(session_summary.aged_sessions);
    session_stats->set_num_tcp_reset_sent(session_summary.num_tcp_rst_sent);
    session_stats->set_num_icmp_error_sent(session_summary.num_icmp_error_sent);
    session_stats->set_num_connection_timeout_sessions(session_summary.num_cxnsetup_timeout);
    session_stats->set_num_session_create_errors(session_summary.num_session_create_err);

    return HAL_RET_OK;
}

/*
 * TCP FIN context used to send
 * out TCP FIN in case of Upgrade.
 */
typedef struct tcpfin_args_ {
    hal_handle_t        session_handle;
    session_state_t     session_state;
    bool                local_src;
    bool                local_dst;
    int                 fte_id;
    dllist_ctxt_t       dllist_ctxt;
} __PACK__ tcpfin_args_t;

/*
 * Send out FIN on IFLOW and/or RFLOW if SEP or DEP is local
 */
void
session_send_tcp_fin (void *data) {
    tcpfin_args_t               *tcpfin = (tcpfin_args_t *)data;
    bool                        local_src = tcpfin->local_src;
    bool                        local_dst = tcpfin->local_dst;
    pd::cpu_to_p4plus_header_t  cpu_header;
    pd::p4plus_to_p4_header_t   p4plus_header;
    uint8_t                     flags = BUILD_TCP_SEND_FIN;
    session_state_t             session_state = tcpfin->session_state;
    uint8_t                     pkt[TCP_IPV4_DOT1Q_PKT_SZ];
    uint32_t                    sz = 0;
    session_t                  *session = NULL;

    session = hal::find_session_by_handle(tcpfin->session_handle);
    if (session == NULL) {
        HAL_TRACE_VERBOSE("Cant find the session for handle {} -- bailing",
                      tcpfin->session_handle);
        return;
    }

    HAL_TRACE_DEBUG("Sending TCP FIN on: {}", session->hal_handle);
    // Set the timestamp option if needed
    if (session_state.tcp_ts_option)
        flags |= BUILD_TCP_SEND_TIMESTAMP;

    if (local_dst &&
        session_state.iflow_state.state == session::FLOW_TCP_STATE_ESTABLISHED) {
        sz = build_tcp_packet(session->iflow, session,
                              session_state.iflow_state, &cpu_header, &p4plus_header,
                              pkt, flags);
        if (sz)
            fte::fte_asq_send(&cpu_header, &p4plus_header, pkt, sz);
    }

    if (local_src &&
        session_state.rflow_state.state == session::FLOW_TCP_STATE_ESTABLISHED) {
        sz = build_tcp_packet(session->rflow, session,
                              session_state.rflow_state, &cpu_header, &p4plus_header,
                              pkt, flags);
        if (sz)
            fte::fte_asq_send(&cpu_header, &p4plus_header, pkt, sz);
    }

    HAL_FREE(HAL_MEM_ALLOC_SESS_UPGRADE_TCP_FIN, tcpfin);
}

hal_ret_t
session_send_fin_list(dllist_ctxt_t *fin_list, bool async) 
{
    hal_ret_t ret = HAL_RET_OK;
    dllist_ctxt_t *curr = NULL, *next = NULL;
    dllist_for_each_safe(curr, next, fin_list) {
        tcpfin_args_t *finargs = dllist_entry(curr, tcpfin_args_t, dllist_ctxt);
        ret = fte::fte_softq_enqueue(finargs->fte_id,
                                     session_send_tcp_fin,
                                     (void *)finargs);
        if (ret == HAL_RET_OK) {
            HAL_TRACE_DEBUG("Successfully enqueued TCP FIN for {}",
                                    finargs->session_handle);
        }
    }
    return ret;
}

/*
 * Upgrade Handling -- Send TCP FIN to sessions with local EPs
 */
hal_ret_t
session_handle_upgrade (void)
{
    
    struct session_upgrade_data_t {
        dllist_ctxt_t session_list;
        dllist_ctxt_t fin_list;
        session_upgrade_data_t () {
            dllist_reset(&session_list);
            dllist_reset(&fin_list);
        }
    } ctxt;
    auto walk_func = [](void *entry, void *ctxt) {
        hal::session_t             *session = (session_t *)entry;
        session_upgrade_data_t     *session_data = (session_upgrade_data_t *) ctxt; 
        dllist_ctxt_t              *list_head =     &session_data->session_list;
        dllist_ctxt_t              *fin_list_head = &session_data->fin_list;
        ep_t                       *sep = NULL, *dep = NULL;
        bool                        src_is_local = false, dst_is_local = false;
        pd::pd_session_get_args_t   args;
        hal_ret_t                   ret = HAL_RET_OK;
        pd::pd_func_args_t          pd_func_args = {0};
        tcpfin_args_t              *finargs = NULL;

        if (session->sep_handle != HAL_HANDLE_INVALID)
            sep = find_ep_by_handle(session->sep_handle);

        if (session->dep_handle != HAL_HANDLE_INVALID)
            dep = find_ep_by_handle(session->dep_handle);

        src_is_local = (sep && (sep->ep_flags & EP_FLAGS_LOCAL));
        dst_is_local = (dep && (dep->ep_flags & EP_FLAGS_LOCAL));

        if (src_is_local || dst_is_local) {
            finargs = (tcpfin_args_t *)HAL_CALLOC(
                          HAL_MEM_ALLOC_SESS_UPGRADE_TCP_FIN,
                          sizeof(tcpfin_args_t));
            SDK_ASSERT(finargs != NULL);

            args.session = session;
            args.session_state = &finargs->session_state;
            pd_func_args.pd_session_get = &args;
            ret = pd::hal_pd_call(pd::PD_FUNC_ID_SESSION_GET, &pd_func_args);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to fetch session state for session {}",
                              session->hal_handle);
                return false;
            }

            finargs->session_handle = session->hal_handle;
            finargs->local_src = src_is_local;
            finargs->local_dst = dst_is_local;
            finargs->fte_id = session->fte_id;
            dllist_add(fin_list_head, &finargs->dllist_ctxt);
        }

        // Add the sessions to the list to send a delete

        hal_handle_id_list_entry_t *list_entry = (hal_handle_id_list_entry_t *)g_hal_state->
                hal_handle_id_list_entry_slab()->alloc();

        if (list_entry == NULL) {
            HAL_TRACE_ERR("Out of memory - skipping delete session {}", session->hal_handle);
            return false;
        }

        list_entry->handle_id = session->hal_handle;
        dllist_add(list_head, &list_entry->dllist_ctxt);

        return false;
    };

    HAL_TRACE_DEBUG("calling walk func");
    g_hal_state->session_hal_handle_ht()->walk_safe(walk_func, &ctxt);
    session_send_fin_list(&ctxt.fin_list, true);
    session_delete_list(&ctxt.session_list);
    

    return HAL_RET_OK;
}

hal_ret_t session_flow_hash_get(FlowHashGetRequest& req,
                                FlowHashGetResponseMsg *rsp) {
    pd::pd_flow_hash_get_args_t   args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    hal_ret_t                   ret = HAL_RET_OK;

    extract_flow_key_from_spec(req.flow_key().src_vrf_id(), &args.key, req.flow_key());

    HAL_TRACE_DEBUG("Flow key: {}", args.key);
    args.key.dir = (req.flow_direction() == types::FLOW_DIRECTION_FROM_UPLINK)?FLOW_DIR_FROM_UPLINK:FLOW_DIR_FROM_DMA;
    args.hw_vrf_id = req.hardware_vrf_id();
    args.lkp_inst = req.flow_instance();
    args.rsp = rsp->add_response();
    pd_func_args.pd_flow_hash_get = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_FLOW_HASH_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to fetch flow hash for key {}",
                      args.key);
        return HAL_RET_ERR;
    }

    return ret;
}

}    // namespace hal
