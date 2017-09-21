#include "fte_ctx.hpp"
#include "fte_flow.hpp"
#include <session.hpp>
#include <pd_api.hpp>
#include <defines.h>

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

// extract session key (iflow key) from spec
hal_ret_t
ctx_t::extract_flow_key_from_spec(hal::flow_key_t *key, const FlowKey&  flow_spec_key,
                                  hal::tenant_id_t tid)
{
    if (flow_spec_key.has_l2_key()) {
        key->flow_type = hal::FLOW_TYPE_L2;
        key->l2seg_id = flow_spec_key.l2_key().l2_segment_id();
        key->ether_type = flow_spec_key.l2_key().ether_type();
        MAC_UINT64_TO_ADDR(key->smac, flow_spec_key.l2_key().smac());
        MAC_UINT64_TO_ADDR(key->dmac, flow_spec_key.l2_key().dmac());
    } else if (flow_spec_key.has_v4_key()) {
        key->flow_type = hal::FLOW_TYPE_V4;
        key->tenant_id = tid;
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
        key->tenant_id = tid;
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
            if(key->icmp_type != 128 && key->icmp_type != 129) {
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

// build flow key fron phv
hal_ret_t
ctx_t::extract_flow_key_from_phv()
{
    HAL_ASSERT_RETURN(phv_ != NULL, HAL_RET_INVALID_ARG);

    switch (phv_->lkp_type) {
    case FLOW_KEY_LOOKUP_TYPE_MAC:
        key_.flow_type = hal::FLOW_TYPE_L2;
        break;
    case FLOW_KEY_LOOKUP_TYPE_IPV4:
        key_.flow_type = hal::FLOW_TYPE_V4;
        break;
    case FLOW_KEY_LOOKUP_TYPE_IPV6:
        key_.flow_type = hal::FLOW_TYPE_V6;
        break;
    default:
        HAL_TRACE_ERR("fte: unknown flow lkp_type {}", phv_->lkp_type);
        return HAL_RET_INVALID_ARG;
    }

    key_.dir = phv_->lkp_dir; 

    hal::l2seg_t *l2seg =  hal::pd::find_l2seg_by_hwid(phv_->lkp_vrf);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("fte: l2seg not found, hwid={}", phv_->lkp_vrf);
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    switch (key_.flow_type) {
    case hal::FLOW_TYPE_L2:
        // L2 flow
        key_.l2seg_id = l2seg->seg_id;
        memcpy(key_.smac, phv_->lkp_src, sizeof(key_.smac));
        memcpy(key_.dmac, phv_->lkp_dst, sizeof(key_.dmac));
        key_.ether_type = phv_->lkp_sport;
        break;
    case  hal::FLOW_TYPE_V4:
    case  hal::FLOW_TYPE_V6:
        // v4 or v6 flow
        key_.tenant_id = l2seg->tenant_id; 
        memcpy(key_.sip.v6_addr.addr8, phv_->lkp_src, sizeof(key_.sip.v6_addr.addr8));
        memcpy(key_.dip.v6_addr.addr8, phv_->lkp_dst, sizeof(key_.dip.v6_addr.addr8));
        key_.proto = phv_->lkp_proto;
        switch (key_.proto) {
        case IP_PROTO_TCP:
        case IP_PROTO_UDP:
            key_.sport = phv_->lkp_sport;
            key_.dport = phv_->lkp_dport;
            break;
        case IP_PROTO_ICMP:
        case IP_PROTO_ICMPV6:
            key_.icmp_type =  phv_->lkp_sport >> 8;
            key_.icmp_code = phv_->lkp_sport & 0x00FF;
            key_.icmp_id = phv_->lkp_dport;
            break;
        case IPPROTO_ESP:
            key_.spi = phv_->lkp_sport << 16 | phv_->lkp_dport;
        default:
            key_.sport = 0;
            key_.dport = 0;
        }
        break;
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::lookup_flow_objs()
{
    hal::tenant_id_t tid;

    if (key_.flow_type == hal::FLOW_TYPE_L2) {
        hal::l2seg_t *l2seg = hal::find_l2seg_by_id(key_.l2seg_id);
        if (l2seg == NULL) {
            HAL_TRACE_ERR("fte: l2seg not found, key={}", key_);
            return HAL_RET_L2SEG_NOT_FOUND;
        }
        tid = l2seg->tenant_id;
    } else {
        tid = key_.tenant_id;
    }

    tenant_ = hal::tenant_lookup_by_id(tid);
    if (tenant_ == NULL) {
        HAL_TRACE_ERR("fte: tenant {} not found, key={}", tid, key_);
        return HAL_RET_TENANT_NOT_FOUND;
    }

    //Lookup src and dest EPs
    hal::ep_get_from_flow_key(&key_, &sep_, &dep_);

    if (sep_ == NULL) {
        HAL_TRACE_ERR("fte: src ep unknown, key={}", key_);
        return HAL_RET_EP_NOT_FOUND;
    }


    if (dep_ == NULL) {
        HAL_TRACE_INFO("fte: dest ep unknown, key={}", key_);
        // TODO(goli) handle VIP
    }

    key_.dir = (sep_->ep_flags & EP_FLAGS_LOCAL)? FLOW_DIR_FROM_DMA :
        FLOW_DIR_FROM_UPLINK;

    sl2seg_ = hal::find_l2seg_by_handle(sep_->l2seg_handle);
    HAL_ASSERT_RETURN(sl2seg_, HAL_RET_L2SEG_NOT_FOUND);
    sif_ = hal::find_if_by_handle(sep_->if_handle);
    HAL_ASSERT_RETURN(sif_ , HAL_RET_IF_NOT_FOUND);

    if (dep_) {
        dl2seg_ = hal::find_l2seg_by_handle(dep_->l2seg_handle);
        HAL_ASSERT_RETURN(dl2seg_, HAL_RET_L2SEG_NOT_FOUND);
        dif_ = hal::find_if_by_handle(dep_->if_handle);
        HAL_ASSERT_RETURN(dif_, HAL_RET_IF_NOT_FOUND);
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::lookup_session()
{
    hal::flow_t *hflow;

    hflow = NULL; // TODO(goli) do hal::lookup_flow(ctx.key);
    if (!hflow) {
        return HAL_RET_SESSION_NOT_FOUND;
    }

    HAL_TRACE_DEBUG("fte: found existing session");
#if 0
    session_ = hflow->session;

    // TODO(goli) handle post svc flows
    if (hflow->config.role == hal::FLOW_ROLE_INITIATOR) {
        iflow_->from_config(hflow->config, hflow->pgm_attrs);
        if (hflow->reverse_flow) {
            rflow_->from_config(hflow->reverse_flow->config, hflow->reverse_flow->pgm_attrs);
        } else {
            rflow_ = NULL;
        }
    } else {
        rflow_->from_config(hflow->config, hflow->pgm_attrs);
        iflow_->from_config(hflow->reverse_flow->config, hflow->reverse_flow->pgm_attrs);
    }
#endif

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::create_session()
{
    hal::flow_key_t rkey = {};
    hal::flow_key_t ikey = {};
    hal_ret_t ret;

    HAL_TRACE_DEBUG("fte: create session");

    ikey = key_;
    for (int i = 0; i < MAX_STAGES; i++) {
        iflow_[i]->set_key(ikey);
    }

    // read rkey from spec
    if (protobuf_request()) {
        if (sess_spec_->has_responder_flow()) {
            ret = extract_flow_key_from_spec(&rkey, sess_spec_->responder_flow().flow_key(),
                                             sess_spec_->meta().tenant_id());
            if (ret != HAL_RET_OK) {
                return ret;
            }
            valid_rflow_ = true;
        }
    } else {
        valid_rflow_ = true;
        rkey.flow_type = key_.flow_type;
        rkey.tenant_id = key_.tenant_id;

        // TODO(goli) check valid ether types for rflow
        if (key_.flow_type == hal::FLOW_TYPE_L2) {
            memcpy(rkey.smac, key_.dmac, sizeof(rkey.smac));
            memcpy(rkey.dmac, key_.smac, sizeof(rkey.dmac));
            rkey.ether_type = key_.ether_type;
        } else {
            rkey.sip = key_.dip;
            rkey.dip = key_.sip;
            rkey.proto = key_.proto;
            switch (key_.proto) {
            case IP_PROTO_TCP:
            case IP_PROTO_UDP:
                rkey.sport = key_.dport;
                rkey.dport = key_.sport;
                break;
            case IP_PROTO_ICMP:
                rkey.icmp_type = key_.icmp_type ? 0 : 8; // flip echo to reply
                rkey.icmp_code = key_.icmp_code;
                rkey.icmp_id = key_.icmp_id;
                break;
            case IP_PROTO_ICMPV6:
                rkey.icmp_type = key_.icmp_type == 128 ? 129 : 128; // flip echo to reply
                rkey.icmp_code = key_.icmp_code;
                rkey.icmp_id = key_.icmp_id;
                break;
            case IPPROTO_ESP:
                rkey.spi = key_.spi;
            default:
                valid_rflow_ = false;
                break;
            }
        }
    }

    if (valid_rflow_) {
        rkey.dir = (dep_ && (dep_->ep_flags & EP_FLAGS_LOCAL)) ?
            FLOW_DIR_FROM_DMA : FLOW_DIR_FROM_UPLINK;
        for (int i = 0; i < MAX_STAGES; i++) {
            rflow_[i]->set_key(rkey);
        }
    } 

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::update_gft()
{
    hal_ret_t ret;
    hal_handle_t  session_handle;

    hal::session_args_fte_t session_args = {};
    hal::session_cfg_t session_cfg = {};
    hal::session_state_t session_state = {};

    hal::flow_cfg_t iflow_cfg_list[MAX_STAGES] = {};
    hal::flow_cfg_t rflow_cfg_list[MAX_STAGES] = {};
    hal::flow_pgm_attrs_t iflow_attrs_list[MAX_STAGES] = {};
    hal::flow_pgm_attrs_t rflow_attrs_list[MAX_STAGES] = {};

    session_args.session = &session_cfg;
    if (protobuf_request()) {
        session_cfg.session_id = sess_spec_->session_id();
        session_state.tcp_ts_option = sess_spec_->tcp_ts_option();
    }

    // TODO(goli) handle other stages

    // by this time dep should be known
    if (dep_ == NULL) {
        HAL_TRACE_ERR("fte::{} dep not found", __func__);
        return HAL_RET_EP_NOT_FOUND;
    }

    for (uint8_t stage = 0; stage <= istage_; stage++) {
        flow_t *iflow = iflow_[stage];
        hal::flow_cfg_t &iflow_cfg = iflow_cfg_list[stage];
        hal::flow_pgm_attrs_t& iflow_attrs = iflow_attrs_list[stage];

        iflow->to_config(iflow_cfg, iflow_attrs);
        iflow_cfg.role = iflow_attrs.role = hal::FLOW_ROLE_INITIATOR;

        // Set the lkp_inst for all stages except the first stage
        if (stage != 0) {
            iflow_attrs.lkp_inst = 1;
        }

        // TODO(goli) fix tnnl_rw_idx lookup
        iflow_attrs.tnnl_rw_idx =
            hal::pd::ep_pd_get_tnnl_rw_tbl_idx_from_pi_ep(dep_, iflow_attrs.tnnl_rw_act);

        session_args.iflow[stage] = &iflow_cfg;
        session_args.iflow_attrs[stage] = &iflow_attrs;

        if (iflow->valid_flow_state()) {
            session_cfg.conn_track_en = true;
            session_args.session_state = &session_state;
            session_state.iflow_state = iflow->flow_state();
        }

        HAL_TRACE_DEBUG("fte::update_gft: iflow.{} key={} lkp_inst={} action={} smac_rw={} dmac-rw={} "
                        "ttl_dec={} mcast={} lport={} qid_en={} qtype={} qid={} rw_act={} "
                        "rw_idx={} tnnl_rw_act={} tnnl_rw_idx={} tnnl_vnid={} nat_sip={} "
                        "nat_dip={} nat_sport={} nat_dport={} nat_type={}",
                        stage, iflow_cfg.key, iflow_attrs.lkp_inst, iflow_cfg.action,
                        iflow_attrs.mac_sa_rewrite,
                        iflow_attrs.mac_da_rewrite, iflow_attrs.ttl_dec, iflow_attrs.mcast_en,
                        iflow_attrs.lport, iflow_attrs.qid_en, iflow_attrs.qtype, iflow_attrs.qid,
                        iflow_attrs.rw_act, iflow_attrs.rw_idx, iflow_attrs.tnnl_rw_act,
                        iflow_attrs.tnnl_rw_idx, iflow_attrs.tnnl_vnid, iflow_attrs.nat_sip,
                        iflow_attrs.nat_dip, iflow_attrs.nat_sport, iflow_attrs.nat_dport,
                        iflow_attrs.nat_type);
    }


    for (uint8_t stage = 0; valid_rflow_ && stage <= rstage_; stage++) {
        flow_t *rflow = rflow_[stage];
        hal::flow_cfg_t &rflow_cfg = rflow_cfg_list[stage];
        hal::flow_pgm_attrs_t& rflow_attrs = rflow_attrs_list[stage];

        rflow->to_config(rflow_cfg, rflow_attrs);
        rflow_cfg.role = rflow_attrs.role = hal::FLOW_ROLE_RESPONDER;

        // Set the lkp_inst for all stages except the first stage
        if (stage != 0) {
            rflow_attrs.lkp_inst = 1;
        }

        // TODO(goli) fix tnnl w_idx lookup
        rflow_attrs.tnnl_rw_idx =
            hal::pd::ep_pd_get_tnnl_rw_tbl_idx_from_pi_ep(sep_, rflow_attrs.tnnl_rw_act);
            
        session_args.rflow[stage] = &rflow_cfg;
        session_args.rflow_attrs[stage] = &rflow_attrs;
            
        if (rflow->valid_flow_state()) {
            session_state.rflow_state = rflow->flow_state();
        }
            
        HAL_TRACE_DEBUG("fte::update_gft: rflow.{} key={} lkp_inst={} action={} smac_rw={} dmac-rw={} "
                        "ttl_dec={} mcast={} lport={} qid_en={} qtype={} qid={} rw_act={} "
                        "rw_idx={} tnnl_rw_act={} tnnl_rw_idx={} tnnl_vnid={} nat_sip={} "
                        "nat_dip={} nat_sport={} nat_dport={} nat_type={}",
                        stage, rflow_cfg.key, rflow_attrs.lkp_inst, rflow_cfg.action,
                        rflow_attrs.mac_sa_rewrite,
                        rflow_attrs.mac_da_rewrite, rflow_attrs.ttl_dec, rflow_attrs.mcast_en,
                        rflow_attrs.lport, rflow_attrs.qid_en, rflow_attrs.qtype, rflow_attrs.qid,
                        rflow_attrs.rw_act, rflow_attrs.rw_idx, rflow_attrs.tnnl_rw_act,
                        rflow_attrs.tnnl_rw_idx, rflow_attrs.tnnl_vnid, rflow_attrs.nat_sip,
                        rflow_attrs.nat_dip, rflow_attrs.nat_sport, rflow_attrs.nat_dport,
                        rflow_attrs.nat_type);
    }

    session_args.tenant = tenant_;
    session_args.sep = sep_;
    session_args.dep = dep_;
    session_args.sif = sif_;
    session_args.dif = dif_;
    session_args.sl2seg = sl2seg_;
    session_args.dl2seg = dl2seg_;
    session_args.spec = sess_spec_;
    session_args.rsp = sess_resp_;
    
    ret =  hal::session_create_fte(&session_args, &session_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: session create failure, err : {}", ret);
        return ret;
    }

    if (protobuf_request()) {
        sess_resp_->mutable_status()->set_session_handle(session_handle);
    }

    return ret;
}

hal_ret_t
ctx_t::update_for_dnat(hal::flow_role_t role, const header_rewrite_info_t& header)
{
    hal_ret_t ret;

    hal::ep_t *dep;
    hal::if_t *dif;
    hal::l2seg_t *dl2seg;

    if (!header.valid_flds.dip) {
        return HAL_RET_OK;
    }

    if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
        dep = hal::find_ep_by_v4_key(tenant_->tenant_id, header.ipv4.dip);
    } else {
        ip_addr_t addr;
        addr.af = IP_AF_IPV6;
        addr.addr.v6_addr = header.ipv6.dip;
        dep = hal::find_ep_by_v6_key(tenant_->tenant_id, &addr);
    }

    if (dep == NULL) {
        return HAL_RET_EP_NOT_FOUND;
    }

    // rewrite dest mac
    flow_update_t flowupd = {type: FLOWUPD_HEADER_REWRITE};
    HEADER_SET_FLD(flowupd.header_rewrite, ether, dmac, 
                   *(struct ether_addr *)hal::ep_get_mac_addr(dep));
    ret = update_flow(flowupd);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    dl2seg = hal::find_l2seg_by_handle(dep->l2seg_handle);
    HAL_ASSERT(dl2seg != NULL);

    dif = hal::find_if_by_handle(dep->if_handle);
    HAL_ASSERT(dif != NULL);

    if (role == hal::FLOW_ROLE_INITIATOR){
        dep_ = dep; 
        dif_ = dif;
        dl2seg_ = dl2seg;
    } else {
        sep_ = dep;
        sif_ = dif;
        sl2seg_ = dl2seg;
    }

    return  HAL_RET_OK;
}

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
    if (sess_spec_) {
        ret = extract_flow_key_from_spec(&key_, sess_spec_->initiator_flow().flow_key(),
                                         sess_spec_->meta().tenant_id());
    } else {
        ret = extract_flow_key_from_phv();
    }

    if (ret != HAL_RET_OK) {
        return ret;
    }

    HAL_TRACE_DEBUG("fte: extracted flow key {}", key_);


    // Lookup ep, intf, l2seg, tenant
    ret = lookup_flow_objs();
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // Lookup old session
    ret = lookup_session();
    if (ret == HAL_RET_SESSION_NOT_FOUND) {
        // Create new session
        // TODO(goli) - check syn flag for TCP (could be a FIN packet for non-exisiting flow)

        ret = create_session();
    }

    return ret;
}

hal_ret_t
ctx_t::init(phv_t *phv, uint8_t *pkt, size_t pkt_len, flow_t iflow[], flow_t rflow[])
{
    hal_ret_t ret;

    *this = {};

    phv_ = phv;
    pkt_ = pkt;
    pkt_len_ = pkt_len;
    arm_lifq_ = {phv->lif, phv->qtype, phv->qid};

    ret = init_flows(iflow, rflow);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init flows, err={}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::init(SessionSpec* spec, SessionResponse *rsp, flow_t iflow[], flow_t rflow[])
{
    hal_ret_t ret;

    *this = {};

    sess_spec_ = spec;
    sess_resp_ = rsp;
    arm_lifq_ = FLOW_MISS_LIFQ;

    ret = init_flows(iflow, rflow);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init flows, err={}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::update_flow(const flow_update_t& flowupd)
{
    hal_ret_t ret;

    flow_t *flow;

    if (role_ == hal::FLOW_ROLE_INITIATOR) {
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
        }
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} action={}",
                        role_, feature_name_, ret, flowupd.action);
        break;

    case FLOWUPD_HEADER_REWRITE:
        ret = flow->header_rewrite(flowupd.header_rewrite);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} header_rewrite={}",
                        role_, feature_name_, ret, flowupd.header_rewrite);
        if (ret == HAL_RET_OK) {
            // check if dep needs to be updated
            ret = update_for_dnat(role_, flowupd.header_rewrite);
        }
        break;

    case FLOWUPD_HEADER_PUSH:
        ret = flow->header_push(flowupd.header_push);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} header_push={}",
                        role_, feature_name_, ret, flowupd.header_push);
        break;

    case FLOWUPD_HEADER_POP:
        ret = flow->header_pop(flowupd.header_pop);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} header_pop={}",
                        role_, feature_name_, ret, flowupd.header_pop);
        break;

    case FLOWUPD_FLOW_STATE:
        ret = flow->set_flow_state(flowupd.flow_state);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} flow_state={}",
                        role_, feature_name_, ret, flowupd.flow_state);
        break;

    case FLOWUPD_FWDING_INFO:
        ret = flow->set_fwding(flowupd.fwding);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} fwding_info={}",
                        role_, feature_name_, ret, flowupd.fwding);
        break;
    }


    return ret;
}

hal_ret_t
ctx_t::advance_to_next_stage() {
    if (role_ == hal::FLOW_ROLE_INITIATOR && iflow_[istage_]->valid_fwding()) {
        HAL_ASSERT_RETURN(istage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
        istage_++;
        HAL_TRACE_DEBUG("fte: advancing to next iflow stage {}", istage_);
    } else if (rflow_[rstage_]->valid_fwding()){
        HAL_ASSERT_RETURN(rstage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
        rstage_++;
        HAL_TRACE_DEBUG("fte: advancing to next iflow stage {}", rstage_);
    }
    return HAL_RET_OK;
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
        HEADER_FORMAT_FLD(out, val, ipv4, ttl);
        HEADER_FORMAT_FLD(out, val, ipv4, dscp);
        break;
    case FTE_HEADER_ipv6:
        HEADER_FORMAT_FLD(out, val, ipv6, sip);
        HEADER_FORMAT_FLD(out, val, ipv6, dip);
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
        HEADER_FORMAT_FLD(out, val, vxlan, tenant_id);
        break;
    case FTE_HEADER_vxlan_gpe:
        HEADER_FORMAT_FLD(out, val, vxlan_gpe, tenant_id);
        break;
    case FTE_HEADER_nvgre:
        HEADER_FORMAT_FLD(out, val, nvgre, tenant_id);
        break;
    case FTE_HEADER_geneve:
        HEADER_FORMAT_FLD(out, val, geneve, tenant_id);
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
    return os << "}";
}

} // namespace fte
