#include "fte_ctx.hpp"
#include "fte_flow.hpp"
#include <session.hpp>
#include <pd_api.hpp>
#include <defines.h>


namespace fte {

hal_ret_t
ctx_t::build_flow_key()
{
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
        HAL_TRACE_ERR("fte - unknown flow lkp_type {}", phv_->lkp_type);
        return HAL_RET_INVALID_ARG;
    }

    key_.dir = phv_->lkp_dir; 

    hal::l2seg_t *l2seg =  hal::pd::find_l2seg_by_hwid(phv_->lkp_vrf);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("fte - l2seg not found, hwid={}", phv_->lkp_vrf);
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    switch (key_.flow_type) {
    case hal::FLOW_TYPE_L2:
        // L2 flow
        // TODO(goli) - check for L2 flow session_pd is not copying smac/dmac to flow hash
        key_.l2seg_id = l2seg->seg_id;
        memcpy(key_.smac, phv_->lkp_src, sizeof(key_.smac));
        memcpy(key_.dmac, phv_->lkp_dst, sizeof(key_.dmac));
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
            key_.icmp_type =  phv_->lkp_sport >> 8;
            key_.icmp_code = phv_->lkp_sport & 0x00FF;
            key_.icmp_id = phv_->lkp_dport;
            break;
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
            HAL_TRACE_ERR("fte - l2seg not found, key={}", key_);
            return HAL_RET_L2SEG_NOT_FOUND;
        }
        tid = l2seg->tenant_id;
    } else {
        tid = key_.tenant_id;
    }

    tenant_ = hal::find_tenant_by_id(tid);
    if (tenant_ == NULL) {
        HAL_TRACE_ERR("fte - tenant {} not found, key={}", tid, key_);
        return HAL_RET_TENANT_NOT_FOUND;
    }

    //Lookup src and dest EPs
    hal::ep_get_from_flow_key(&key_, &sep_, &dep_);

    //one of the src or dst EPs should be known (no transit traffic)
    if (sep_ == NULL && dep_ == NULL) {
        HAL_TRACE_ERR("fte - src and dest eps unknown, key={}", key_);
        return HAL_RET_EP_NOT_FOUND;
    }

    if (sep_) {
        sl2seg_ = hal::find_l2seg_by_handle(sep_->l2seg_handle);
        sif_ = hal::find_if_by_handle(sep_->if_handle);
    }

    if (dep_) {
        dl2seg_ = hal::find_l2seg_by_handle(dep_->l2seg_handle);
        dif_ = hal::find_if_by_handle(dep_->if_handle);
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

    session_ = hflow->session;

    // TODO(goli) handle post svc flows
    if (hflow->config.role == hal::FLOW_ROLE_INITIATOR) {
        flow_ = iflow_;
        iflow_->from_config(hflow->config);
        if (hflow->reverse_flow) {
            rflow_->from_config(hflow->reverse_flow->config);
        } else {
            rflow_ = NULL;
        }
    } else {
        rflow_->from_config(hflow->config);
        iflow_->from_config(hflow->reverse_flow->config);
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::create_session()
{
    flow_ = iflow_;  // iflow is cur flow

    iflow_->set_key(key_);
   
    // Init rflow for TCP and UDP
    if ((key_.flow_type == hal::FLOW_TYPE_V4 || key_.flow_type == hal::FLOW_TYPE_V6) &&
        (key_.proto == IP_PROTO_TCP || key_.proto == IP_PROTO_UDP)) {

        // TODO(goli) do it in the fwding feature, dep might change
        hal::flow_key_t rkey = {};

        rkey.flow_type = key_.flow_type;
        rkey.dir = (dep_ && (dep_->ep_flags & EP_FLAGS_LOCAL)) ?
            FLOW_DIR_FROM_DMA : FLOW_DIR_FROM_UPLINK;
        rkey.tenant_id = key_.tenant_id; // TODO(goli) - check
        rkey.sip = key_.dip;
        rkey.dip = key_.sip;
        rkey.proto = key_.proto;
        rkey.sport = key_.dport;
        rkey.dport = key_.sport;

        rflow_->set_key(rkey);
    } else {
        rflow_ = rflow_post_ = NULL;
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::init_flows()
{
    hal_ret_t ret;

    // Build the key and lookup flow 
    ret = build_flow_key();
    if (ret != HAL_RET_OK) {
        return ret;
    }

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
ctx_t::update_gft()
{
    hal::session_args_t session_args = {};
    hal::session_cfg_t session_cfg = {};
    hal::flow_cfg_t iflow_cfg = {};
    hal::flow_cfg_t rflow_cfg = {};

    // Create hal::session and update GFT
    if (iflow_->valid_conn_track()) {
        session_cfg.conn_track_en = iflow_->conn_track().enable;
        session_cfg.syn_ack_delta = iflow_->conn_track().syn_ack_delta;
    } else if (iflow_post_->valid_conn_track()) {
        session_cfg.conn_track_en = iflow_post_->conn_track().enable;
        session_cfg.syn_ack_delta = iflow_post_->conn_track().syn_ack_delta;
    }

    // TODO(goli)non merge case
    iflow_->merge_flow(*iflow_post_);

    iflow_->to_config(iflow_cfg);
    iflow_cfg.role = hal::FLOW_ROLE_INITIATOR;
    session_args.iflow = &iflow_cfg;

    if (rflow_) {
        rflow_->merge_flow(*rflow_post_);

        rflow_->to_config(rflow_cfg);
        rflow_cfg.role = hal::FLOW_ROLE_RESPONDER;
        session_args.rflow = &rflow_cfg;
    }

    session_args.session = &session_cfg;
    session_args.iflow = &iflow_cfg;
    session_args.rflow = &rflow_cfg;
    session_args.tenant = tenant_;
    // TODO(bharat): Please take care of this after the changes in flow design.
#if 0
    session_args.sep = sep_;
    session_args.dep = dep_;
    session_args.sif = sif_;
    session_args.dif = dif_;
    session_args.sl2seg = sl2seg_;
    session_args.dl2seg = dl2seg_;
#endif

    return hal::session_create(&session_args, NULL);
}

hal_ret_t
ctx_t::init(phv_t *phv, uint8_t *pkt, size_t pkt_len,
            flow_t *iflow, flow_t *rflow, flow_t *iflow_post, flow_t *rflow_post)
{
    hal_ret_t ret;

    *this = {};
    *iflow = *rflow = *iflow_post = *rflow_post = {};

    phv_ = phv;
    pkt_ = pkt;
    pkt_len_ = pkt_len;
    arm_lifq_ = {phv->lif, phv->qtype, phv->qid};

    iflow_ = iflow;
    rflow_ = rflow;
    iflow_post_ = iflow_post;
    rflow_post_ = rflow_post;

    ret = init_flows();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte - failed to init flows, err={}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::update_flow(flow_t *flow, const flow_update_t& flowupd)
{
    switch (flowupd.type) {
    case FLOWUPD_ACTION:
        return flow->set_action(flowupd.action);
    case FLOWUPD_HEADER_REWRITE:
        return flow->header_rewrite(flowupd.header_rewrite);
    case FLOWUPD_HEADER_PUSH:
        return flow->header_push(flowupd.header_push);
    case FLOWUPD_HEADER_POP:
        return flow->header_pop(flowupd.header_pop);
    case FLOWUPD_CONN_TRACK:
        return flow->set_conn_track(flowupd.conn_track);
    case FLOWUPD_FWDING_INFO:
        return flow->set_fwding(flowupd.fwding);
    }

    return HAL_RET_INVALID_ARG;
}

hal_ret_t
ctx_t::update_iflow(const flow_update_t& flowupd)
{
    return update_flow(post_svcs_ ? iflow_post_ : iflow_, flowupd);
}

hal_ret_t
ctx_t::update_rflow(const flow_update_t& flowupd)
{
    if (!rflow_)
        return HAL_RET_OK;

    return update_flow(post_svcs_ ? rflow_post_ : rflow_, flowupd);
}

} // namespace fte
