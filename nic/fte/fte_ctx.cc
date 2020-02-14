#include "nic/hal/third-party/packet_parser/packet_parser.h"
#include "nic/hal/third-party/packet_parser/packet_to_string.h"
#include "fte.hpp"
#include "fte_ctx.hpp"
#include "fte_impl.hpp"
#include "fte_flow.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "lib/list/list.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/cpupkt_headers.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/include/pkt_hdrs.hpp"

using namespace hal::app_redir;

std::ostream& operator<<(std::ostream& os, const ether_addr& val)
{
    return os << macaddr2str(val.ether_addr_octet);
}
namespace session {
std::ostream& operator<<(std::ostream& os, session::FlowAction val)
{
    switch(val) {
    case session::FLOW_ACTION_DROP: return os << "drop";
    case session::FLOW_ACTION_ALLOW: return os << "allow";
    case session::FLOW_ACTION_NONE: return os << "none";
    default: return os;
    }
}
}

namespace fte {


//------------------------------------------------------------------------------
// Returns flow key of the specified flow
//------------------------------------------------------------------------------
const hal::flow_key_t&
ctx_t::get_key(hal::flow_role_t role) const
{
    if (role == hal::FLOW_ROLE_NONE) {
        role = role_;
    }

    return (role == hal::FLOW_ROLE_INITIATOR) ? key_ : rkey_;
}

//------------------------------------------------------------------------------
// Returns true if proxy mirror session(s) is configured for the current flow
//------------------------------------------------------------------------------
bool
ctx_t::get_proxy_mirror_flow(hal::flow_role_t role)
{
    flow_t *flow = NULL;

    if (role == hal::FLOW_ROLE_INITIATOR) {
        flow = iflow_[istage_];
    } else {
        flow = rflow_[rstage_];
    }

    if (flow) {
        if (flow->mirror_info().proxy_ing_mirror_session ||
            flow->mirror_info().proxy_egr_mirror_session) {
            return true;
        }

        if (flow->mcast_info().mcast_en && flow->mcast_info().proxy_mcast_ptr) {
            return true;
        }
    }

    return false;
}


void ctx_t::swap_flow_key(const hal::flow_key_t &key, hal::flow_key_t *rkey)
{
    rkey->lkpvrf = key.lkpvrf;
    rkey->flow_type = key.flow_type;
    rkey->svrf_id = key.dvrf_id;
    rkey->dvrf_id = key.svrf_id;

    if (key.flow_type == hal::FLOW_TYPE_L2) {
        memcpy(rkey->smac, key.dmac, sizeof(rkey->smac));
        memcpy(rkey->dmac, key.smac, sizeof(rkey->dmac));
        rkey->ether_type = key.ether_type;
    } else {
        rkey->sip = key.dip;
        rkey->dip = key.sip;
        rkey->proto = key.proto;
        switch (key.proto) {
        case IP_PROTO_TCP:
        case IP_PROTO_UDP:
            rkey->sport = key.dport;
            rkey->dport = key.sport;
            break;
        case IP_PROTO_ICMP:
            if (key.icmp_type == ICMP_TYPE_ECHO_RESPONSE) {
                rkey->icmp_type = ICMP_TYPE_ECHO_REQUEST;
            } else if (key.icmp_type == ICMP_TYPE_ECHO_REQUEST) {
                rkey->icmp_type = ICMP_TYPE_ECHO_RESPONSE;
            }
            rkey->icmp_code = key.icmp_code;
            rkey->icmp_id = key.icmp_id;
            break;
        case IP_PROTO_ICMPV6:
            if (key.icmp_type == ICMPV6_TYPE_ECHO_RESPONSE) {
                rkey->icmp_type = ICMPV6_TYPE_ECHO_REQUEST;
            } else if (key.icmp_type == ICMPV6_TYPE_ECHO_REQUEST) {
                rkey->icmp_type = ICMPV6_TYPE_ECHO_RESPONSE;
            }
            rkey->icmp_code = key.icmp_code;
            rkey->icmp_id = key.icmp_id;
            break;
        case IPPROTO_ESP:
            rkey->spi = key.spi;
            break;
        default:
            break;
        }
    }
}


//------------------------------------------------------------------------------
// Initialize the flow entries in the context
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::init_flows(flow_t iflow[], flow_t rflow[])
{
    hal_ret_t ret;

    for (uint8_t i = 0; i < MAX_STAGES; i++) {
        iflow_[i] = &iflow[i];
        rflow_[i] = &rflow[i];
        iflow_[i]->init(this);
        rflow_[i]->init(this);
    }

    // Build the key and lookup flow
    ret = extract_flow_key();

    if (ret != HAL_RET_OK) {
        return ret;
    }

    HAL_TRACE_DEBUG("fte: extracted flow key {}", key_);

    if (hal::g_hal_state->is_microseg_enabled()) { 
        // Lookup ep, intf, l2seg, vrf
        ret = lookup_flow_objs();
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    // Lookup old session
    ret = lookup_session();

    if (ret == HAL_RET_SESSION_NOT_FOUND) {
        // Create new session only in the case of flow miss pkt
        if (flow_miss()) {
            // Create new session
            ret = create_session();
        } else {
            ret = HAL_RET_OK;
        }
    } else if (flow_miss()) {
        // Flow miss + existing session
        incr_fte_retransmit_packets();
    }

    return ret;
}

//------------------------------------------------------------------------------
// Initialize the context
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::init(const lifqid_t &lifq, feature_state_t feature_state[], uint16_t num_features)
{
    bzero(this, sizeof(*this));

    arm_lifq_ = lifq;
    update_session_ = false;
    ipc_logging_disable_ = false;

    num_features_ = num_features;
    feature_state_ = feature_state;
    if (num_features) {
        feature_state_init(feature_state_, num_features_);
    }

    return HAL_RET_OK;
}

void
ctx_t::process_tcp_queues(void *tcp_ctx)
{
    /*
     *  arm_lifq_ is used to determine if the downcall from lkl to tcp plugin
     *  for tcp_transmit_pkt is for a packet from tcp_proxy_pipeline or cpu
     *  pipeline. Since process_tcp_queues processes rx pkts from
     *  tcp_proxy_pipeline, we need to set arm_lifq_ so as to prepare for the
     *  tx pkt downcall from lkl
     */
    arm_lifq_ = TCP_PROXY_LIFQ;

    fte::impl::process_tcp_queues(tcp_ctx);
}

//------------------------------------------------------------------------------
// Initialize the context from incoming pkt
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len, bool copied_pkt,
            flow_t iflow[], flow_t rflow[],
            feature_state_t feature_state[], uint16_t num_features)
{
    hal_ret_t ret;

    HAL_TRACE_DEBUG("fte: rxpkt{} cpu_rxhdr={}",
                    copied_pkt ? "(copy)" : "", hex_str((uint8_t*)cpu_rxhdr, sizeof(*cpu_rxhdr)));

    HAL_TRACE_DEBUG("fte: rxpkt len={} pkt={}", pkt_len, hex_str(pkt, (pkt_len >=128)?128:pkt_len));

    HAL_TRACE_DEBUG("fte: rxpkt slif={} lif={} qtype={} qid={} vrf={} "
                    "src_app_id={} lkp_dir={} lkp_inst={} lkp_type={} flags={} "
                    "l2={} l3={} l4={} payload={}",
                    cpu_rxhdr->src_lif, cpu_rxhdr->lif, cpu_rxhdr->qtype,
                    cpu_rxhdr->qid, cpu_rxhdr->lkp_vrf, cpu_rxhdr->src_app_id,
                    cpu_rxhdr->lkp_dir, cpu_rxhdr->lkp_inst, cpu_rxhdr->lkp_type,
                    cpu_rxhdr->flags, cpu_rxhdr->l2_offset, cpu_rxhdr->l3_offset,
                    cpu_rxhdr->l4_offset, cpu_rxhdr->payload_offset);

    lifqid_t lifq =  {cpu_rxhdr->lif, cpu_rxhdr->qtype, cpu_rxhdr->qid};

    ret = init(lifq, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init ctx, err={}", ret);
        return ret;
    }

    cpu_rxhdr_ = cpu_rxhdr;
    pkt_ = pkt;
    pkt_len_ = pkt_len;
    copied_pkt_ = copied_pkt;
    enq_or_free_rx_pkt_ = false;

     if (fte_span()) {
         // TODO: Print packet and exit
         struct packet *packet = packet_new(pkt_len);
         char *error = NULL;
         memcpy(packet->buffer, pkt, pkt_len);
         enum packet_parse_result_t result =
             (packet_parse_result_t)parse_packet(packet, pkt_len, PACKET_LAYER_2_ETHERNET,
                                                 &error);
         if (result != PACKET_OK || error != NULL) {
             HAL_TRACE_ERR("Unable to parse packet. result: {}, error: {}",
                           result, error);
             packet_free(packet);
             return HAL_RET_FTE_SPAN;
         }
         // SDK_ASSERT(result == PACKET_OK);
         // SDK_ASSERT(error == NULL);

         char *dump = NULL;
         // int status = packet_to_string(packet, DUMP_SHORT, &dump, &error);
         int status = packet_to_string(packet, DUMP_VERBOSE, &dump, &error);
         SDK_ASSERT(status == STATUS_OK);
         SDK_ASSERT(error == NULL);

         HAL_TRACE_VERBOSE("fte: fte-span packet: {}", dump);
         free(dump);
         packet_free(packet);
         return HAL_RET_FTE_SPAN;
     }

    if ((cpu_rxhdr->lif == HAL_LIF_CPU) || app_redir_pkt_rx_raw(*this) ||
        (tcp_close())) {
        ret = init_flows(iflow, rflow);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to init flows, err={}", ret);
            return ret;
        }
    }

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// Initialize the context from GRPC protobuf
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::init(SessionSpec* spec, SessionStatus *status, SessionStats *stats,
            SessionResponse *rsp, flow_t iflow[], flow_t rflow[],
            feature_state_t feature_state[], uint16_t num_features)
{
    hal_ret_t ret;

    ret = init(FLOW_MISS_LIFQ, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init ctx, err={}", ret);
        return ret;
    }

    sess_spec_   = spec;
    sess_status_ = status;
    sess_stats_  = stats;
    sess_resp_   = rsp;

    ret = init_flows(iflow, rflow);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init flows, err={}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------------
// Initialize the context from session -- needed when called from hal for cleanup
//------------------------------------------------------------------------------------
hal_ret_t
ctx_t::init(hal::session_t *session, flow_t iflow[], flow_t rflow[],
            feature_state_t feature_state[], uint16_t num_features)
{
    hal_ret_t ret;

    ret = init(FLOW_MISS_LIFQ, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init ctx, err={}", ret);
        return ret;
    }

    for (uint8_t i = 0; i < MAX_STAGES; i++) {
        iflow_[i] = &iflow[i];
        rflow_[i] = &rflow[i];
        iflow_[i]->init(this);
        rflow_[i]->init(this);
    }

    init_ctxt_from_session(session);

    ret = lookup_flow_objs();

    return HAL_RET_OK;
}

#define LOG_FLOW_UPDATE(__updinfo)                                         \
    HAL_TRACE_VERBOSE("{}.{} feature={} ret={} {}={}",                       \
                    role,                                                  \
                    (role == hal::FLOW_ROLE_INITIATOR)? istage_ : rstage_, \
                    feature_name_, ret, #__updinfo, flowupd.__updinfo)

//------------------------------------------------------------------------------
// Updates the current flow with the sepcified flowupd info
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_flow(const flow_update_t& flowupd)
{
    return update_flow(flowupd, role());
}

//------------------------------------------------------------------------------
// Updates the specified flow with the sepcified flowupd info
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_flow(const flow_update_t& flowupd,
                   const hal::flow_role_t role)
{
    hal_ret_t ret = HAL_RET_OK;

    flow_t *flow;

    if (role == hal::FLOW_ROLE_INITIATOR) {
        flow = iflow_[istage_];
    } else {
        flow = rflow_[rstage_];
    }

    if (!flow) {
        return HAL_RET_OK;
    }

    switch (flowupd.type) {
    case FLOWUPD_ACTION:
        ret = flow->set_action(flowupd.action);
        if (flowupd.action == session::FLOW_ACTION_DROP) {
            drop_ = true;
            drop_flow_ = true;
            // TODO(goli) - need to invalidate rflow - but dol is failing
            // if (role == hal::FLOW_ROLE_INITIATOR) {
            //    valid_rflow_ = false;
            //}
            // Increment the drop counter for the feature
            if (role == hal::FLOW_ROLE_INITIATOR)
                incr_inst_feature_stats(feature_id_);
        } else if (flowupd.action == session::FLOW_ACTION_ALLOW) {
            drop_ = false;
            drop_flow_ = false;
        }
        if (ret == HAL_RET_OK)
            LOG_FLOW_UPDATE(action);
        break;

    case FLOWUPD_HEADER_REWRITE:
        ret = flow->header_rewrite(flowupd.header_rewrite);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(header_rewrite);
            // check if dep needs to be updated
            ret = update_for_dnat(role, flowupd.header_rewrite);
            if (ret == HAL_RET_OK){
                ret = update_for_snat(role, flowupd.header_rewrite);
            }
        }
        break;

    case FLOWUPD_HEADER_PUSH:
        ret = flow->header_push(flowupd.header_push);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(header_push);
        }
        break;

    case FLOWUPD_HEADER_POP:
        ret = flow->header_pop(flowupd.header_pop);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(header_pop);
        }
        break;

    case FLOWUPD_FLOW_STATE:
        ret = flow->set_flow_state(flowupd.flow_state);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(flow_state);
        }
        break;

    case FLOWUPD_FWDING_INFO:
        ret = flow->set_fwding(flowupd.fwding);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(fwding);
            if (flowupd.fwding.dep) {
                dep_ = flowupd.fwding.dep;
            }
            if (flowupd.fwding.dif) {
                dif_ = flowupd.fwding.dif;
            }
            if (flowupd.fwding.dl2seg) {
                dl2seg_ =  flowupd.fwding.dl2seg;
            }
        }
        break;

    case FLOWUPD_MCAST_COPY:
        ret = flow->merge_mcast_info(flowupd.mcast_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(mcast_info);
        }
        break;

    case FLOWUPD_INGRESS_INFO:
        ret = flow->set_ingress_info(flowupd.ingress_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(ingress_info);
        }
        break;

    case FLOWUPD_MIRROR_INFO:
        ret = flow->merge_mirror_info(flowupd.mirror_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(mirror_info);
        }
        break;

    case FLOWUPD_EXPORT_INFO:
        ret = flow->merge_export_info(flowupd.export_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(export_info);
        }
        break;

    case FLOWUPD_QOS_INFO:
        ret = flow->set_qos_info(flowupd.qos_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(qos_info);
        }
        break;

    case FLOWUPD_LKP_INFO:
        ret = flow->set_lkp_info(flowupd.lkp_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(lkp_info);
        }
        break;

    case FLOWUPD_LKP_KEY:
        ret = flow->set_key(flowupd.key);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(key);
        }
        break;
    case FLOWUPD_AGING_INFO:
        ret = flow->set_aging_info(flowupd.aging_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(aging_info);
        }
        break;
    case FLOWUPD_SFW_INFO:
        ret = flow->set_sfw_info(flowupd.sfw_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(sfw_info);
        }
        break;
    }

    if (ret == HAL_RET_OK)  {
        update_session_ = true;
    }

    if (ret == HAL_RET_ENTRY_EXISTS)
        ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Advance the pipeline to next stage if the fwding info is valid
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::advance_to_next_stage() {

    if (is_ipfix_flow()) return HAL_RET_OK; 
    
    if (existing_session()) {
        if (role_ == hal::FLOW_ROLE_INITIATOR &&
            session()->iflow && session()->iflow->assoc_flow) {
            SDK_ASSERT_RETURN(istage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
            istage_++;
            HAL_TRACE_DEBUG("fte: advancing to next iflow stage {}", istage_);
        } else if (role_ == hal::FLOW_ROLE_RESPONDER &&
                   session()->rflow && session()->rflow->assoc_flow) {
            SDK_ASSERT_RETURN(rstage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
            rstage_++;
            HAL_TRACE_DEBUG("fte: advancing to next rflow stage {}", rstage_);
        }
    } else {

        if (role_ == hal::FLOW_ROLE_INITIATOR && iflow_[istage_]->valid_fwding()) {
            SDK_ASSERT_RETURN(istage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
            istage_++;
            HAL_TRACE_DEBUG("fte: advancing to next iflow stage {}", istage_);
        } else if (rflow_[rstage_]->valid_fwding()){
            SDK_ASSERT_RETURN(rstage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
            rstage_++;
            HAL_TRACE_DEBUG("fte: advancing to next rflow stage {}", rstage_);
        }
    }
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// Invoke all the queued completion handlers
//------------------------------------------------------------------------------
void
ctx_t::invoke_completion_handlers(bool fail)
{
    HAL_TRACE_VERBOSE("fte: invoking completion handlers.");
    for (int i = 0; i < num_features_; i++) {
        if (feature_state_[i].completion_handler != nullptr) {
            HAL_TRACE_VERBOSE("fte: invoking completion handler {}",
                              feature_state_[i].name);
            set_feature_name(feature_state_[i].name);
            (*feature_state_[i].completion_handler)(*this, fail);
        }
    }
}

//------------------------------------------------------------------------------
// Process the packet and update the flow table
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::process()
{
    hal_ret_t ret;

    // We are not in FTE thread for GET
    // and we do not want to log the get
    if (!ipc_logging_disable() &&
        pipeline_event() != FTE_SESSION_GET) {
        HAL_TRACE_VERBOSE("get ipc logger");
        logger_  = get_current_ipc_logger_inst();
    }

    // execute the pipeline
    ret = execute_pipeline(*this);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to execute pipeline, ret={}", ret);
        goto end;
    }

    // update flow table
    ret = update_flow_table();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to updated gft, ret={}", ret);
        goto end;
    }

 end:
    // Increment FTE error counter
    incr_inst_fte_error(ret);

    // Invoke completion handlers
    invoke_completion_handlers((ret==HAL_RET_OK));

    return ret;
}

//-------------------------------------------------------------------------
// Swap the derived flow objects for reverse flow processing.
//-------------------------------------------------------------------------
void
ctx_t::swap_flow_objs()
{
    hal::if_t    *dif = sif_;
    hal::ep_t    *dep = sep_;
    hal_handle_t  dep_handle = sep_handle_;
    hal::l2seg_t *dl2seg = sl2seg_;

    sif_ = dif_;
    dif_ = dif;
    sep_ = dep_;
    dep_ = dep;
    sep_handle_ = dep_handle_;
    dep_handle_ = dep_handle;
    sl2seg_ = dl2seg_;
    dl2seg_ = dl2seg;
}

bool
ctx_t::is_proxy_enabled()
{
    if (is_ipfix_flow()) return false;

    flow_t **flow = (role_ == hal::FLOW_ROLE_INITIATOR) ? iflow_ : rflow_;

    // For existing sessions, fwding will be set to TRUE even if it is
    // not proxy. So, get it from the pgm attrs
    if (existing_session()) {
        if (stage() == 0 && flow[stage()]->is_proxy_enabled())
            return true;
        else
            return false;
    }

    // For proxy flow we need to be either in stage 1 or
    // in stage 0 with forwarding info set by the proxy.
    if (stage() == 0 && flow[stage()]->valid_fwding() == false) {
        return false;
    }
    return true;
}

bool
ctx_t::is_proxy_flow()
{
    flow_t **flow = (role_ == hal::FLOW_ROLE_INITIATOR) ? iflow_ : rflow_;

    if (!is_proxy_enabled()) {
        return false;
    }

    // In the case of pkts from uplink, stage 0 flow is proxy flow and
    // in the case of pkts from host, stage 1 flow is the proxy flow
    return (flow[stage()]->key().dir == hal::FLOW_DIR_FROM_UPLINK) ?
                                stage() == 0 : stage() != 0;
}

bool
ctx_t::vlan_valid()
{
    return cpu_rxhdr_->flags&CPU_FLAGS_VLAN_VALID;
}

bool
ctx_t::tunnel_terminated()
{
    return cpu_rxhdr_->flags&CPU_FLAGS_TUNNEL_TERMINATE;
}

std::ostream& operator<<(std::ostream& os, const mpls_label_t& val)
{
    os << "{label=" << val.label;
    os << ", exp=" << val.exp;
    os << ", bos=" << val.bos;
    os << ", ttl=" <<val.ttl;
    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const header_rewrite_info_t& val)
{
    char buf[400];
    fmt::ArrayWriter out(buf, 400);
    out.write("{{");
    switch (val.valid_hdrs&FTE_L2_HEADERS) {
    case FTE_HEADER_ether:
        HEADER_FORMAT_FLD(out, val, ether, smac);
        HEADER_FORMAT_FLD(out, val, ether, dmac);
        HEADER_FORMAT_FLD(out, val, ether, vlan_id);
        HEADER_FORMAT_FLD(out, val, ether, dot1p);
        break;
    }
    switch(val.valid_hdrs&FTE_L3_HEADERS){
    case FTE_HEADER_ipv4:
        HEADER_FORMAT_IPV4_FLD(out, val, ipv4, sip);
        HEADER_FORMAT_IPV4_FLD(out, val, ipv4, dip);
        HEADER_FORMAT_FLD(out, val, ipv4, svrf_id);
        HEADER_FORMAT_FLD(out, val, ipv4, dvrf_id);
        HEADER_FORMAT_FLD(out, val, ipv4, ttl);
        HEADER_FORMAT_FLD(out, val, ipv4, dscp);
        break;
    case FTE_HEADER_ipv6:
        HEADER_FORMAT_FLD(out, val, ipv6, sip);
        HEADER_FORMAT_FLD(out, val, ipv6, dip);
        HEADER_FORMAT_FLD(out, val, ipv6, svrf_id);
        HEADER_FORMAT_FLD(out, val, ipv6, dvrf_id);
        HEADER_FORMAT_FLD(out, val, ipv6, ttl);
        HEADER_FORMAT_FLD(out, val, ipv6, dscp);
        break;
    }
    switch(val.valid_hdrs&FTE_L4_HEADERS){
    case FTE_HEADER_tcp:
        HEADER_FORMAT_FLD(out, val, tcp, sport);
        HEADER_FORMAT_FLD(out, val, tcp, dport);
        break;
    case FTE_HEADER_udp:
        HEADER_FORMAT_FLD(out, val, udp, sport);
        HEADER_FORMAT_FLD(out, val, udp, dport);
        break;
    }
    out.write("dec_ttl={}}}", val.flags.dec_ttl);
    return os << out.c_str();
}

std::ostream& operator<<(std::ostream& os, const header_push_info_t& val)
{
    char buf[400];
    fmt::ArrayWriter out(buf, 400);
    out.write("{{");
    switch (val.valid_hdrs&FTE_L2_HEADERS) {
    case FTE_HEADER_ether:
        HEADER_FORMAT_FLD(out, val, ether, smac);
        HEADER_FORMAT_FLD(out, val, ether, dmac);
        HEADER_FORMAT_FLD(out, val, ether, vlan_id);
        break;
    }
    switch(val.valid_hdrs&FTE_L3_HEADERS){
    case FTE_HEADER_ipv4:
        HEADER_FORMAT_IPV4_FLD(out, val, ipv4, sip);
        HEADER_FORMAT_IPV4_FLD(out, val, ipv4, dip);
        break;
    case FTE_HEADER_ipv6:
        HEADER_FORMAT_FLD(out, val, ipv6, sip);
        HEADER_FORMAT_FLD(out, val, ipv6, dip);
        break;
    }
    switch(val.valid_hdrs&FTE_ENCAP_HEADERS){
    case FTE_HEADER_vxlan:
        HEADER_FORMAT_FLD(out, val, vxlan, vrf_id);
        break;
    case FTE_HEADER_vxlan_gpe:
        HEADER_FORMAT_FLD(out, val, vxlan_gpe, vrf_id);
        break;
    case FTE_HEADER_nvgre:
        HEADER_FORMAT_FLD(out, val, nvgre, vrf_id);
        break;
    case FTE_HEADER_geneve:
        HEADER_FORMAT_FLD(out, val, geneve, vrf_id);
        break;
    case FTE_HEADER_gre:
        HEADER_FORMAT_FLD(out, val, gre, dummy);
        break;
    case FTE_HEADER_erspan:
        HEADER_FORMAT_FLD(out, val, erspan, dummy);
        break;
    case FTE_HEADER_ip_in_ip:
        HEADER_FORMAT_FLD(out, val, ip_in_ip, dummy);
        break;
    case FTE_HEADER_ipsec_esp:
        HEADER_FORMAT_FLD(out, val, ipsec_esp, dummy);
        break;
    case FTE_HEADER_mpls:
        HEADER_FORMAT_FLD(out, val, mpls, eompls);
        HEADER_FORMAT_FLD(out, val, mpls, label0);
        HEADER_FORMAT_FLD(out, val, mpls, label1);
        HEADER_FORMAT_FLD(out, val, mpls, label2);
        break;
    }
    out.write("}}");
    return os << out.c_str();
}
std::ostream& operator<<(std::ostream& os, const header_pop_info_t& val)
{
    return os << "{}";
}

std::ostream& operator<<(std::ostream& os, const fwding_info_t& val)
{
    os << "{lport=" << val.lport;
    if (val.qid_en) {
        os << " ,qtype=" << val.qtype;
        os << ", qid=" << val.qid;
    }

    if (val.dep) {
        os << " ,dep=" << val.dep->hal_handle;
    }

    if (val.dif) {
        os << " ,dif=" << val.dif->if_id;
    }

    if (val.dl2seg) {
        os << " ,dl2seg=" << val.dl2seg->seg_id;
    }

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const ingress_info_t& val)
{
    os << "{expected_sif=" << val.expected_sif->if_id;
    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const qos_info_t& val)
{
    os << "{qos_class_en=" << (bool)val.qos_class_en;
    if (val.qos_class_en) {
        os << " ,qos_class_id=" << val.qos_class_id;
    }
    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const lkp_info_t& val)
{
    os << "{vrf_hwid=" << val.vrf_hwid;
    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const mcast_info_t& val)
{
    os << "{mcast_en=" << (bool)val.mcast_en;
    if (val.mcast_en) {
        os << " ,mcast_ptr=" << val.mcast_ptr;
        os << " ,proxy_mcast_ptr=" << val.proxy_mcast_ptr;
    }
    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const mirror_info_t& val)
{
    os << "{ mirror_en=" << (bool)val.mirror_en;
    os << " ,ing_mirror_session=" << (uint32_t)val.ing_mirror_session;
    os << " ,egr_mirror_session=" << (uint32_t)val.egr_mirror_session;
    os << " ,proxy_ing_mirror_session=" << (uint32_t)val.proxy_ing_mirror_session;
    os << " ,proxy_egr_mirror_session=" << (uint32_t)val.proxy_egr_mirror_session;
    return os << " }";
}

std::ostream& operator<<(std::ostream& os, const export_info_t& val)
{
    os << "{ export_en=" << (uint32_t)val.export_en;
    os << " ,export_id1=" << (uint32_t)val.export_id1;
    os << " ,export_id2=" << (uint32_t)val.export_id2;
    os << " ,export_id3=" << (uint32_t)val.export_id3;
    os << " ,export_id4=" << (uint32_t)val.export_id4;
    return os << " }";
}

std::ostream& operator<<(std::ostream& os, const aging_info_t& val)
{
    os << "{ idle_timeout=" << val.idle_timeout;
    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const sfw_flow_info_t& val)
{
    os << "{ sfw_rule_id=" << (uint64_t)val.sfw_rule_id;
    os << " ,sfw_action="  << val.sfw_action;
    os << " ,skip_sfw_reval=" << val.skip_sfw_reval;
    return os << "}";
}

} // namespace fte
