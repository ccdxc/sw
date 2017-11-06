#include "fte.hpp"
#include "fte_ctx.hpp"
#include "fte_flow.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/tenant.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/pd/common/cpupkt_headers.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"

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

// extract flow key from packet
hal_ret_t
ctx_t::extract_flow_key()
{
    ether_header_t *ethhdr;
    ipv4_header_t *iphdr;
    ipv6_header_t *iphdr6;
    tcp_header_t *tcphdr;
    udp_header_t *udphdr;
    icmp_header_t *icmphdr;
    ipsec_esp_header_t *esphdr;
    
    HAL_ASSERT_RETURN(cpu_rxhdr_ != NULL && pkt_ != NULL, HAL_RET_INVALID_ARG);

    key_.dir = cpu_rxhdr_->lkp_dir;

    // Lookup l2seg using vrf id
    hal::l2seg_t *l2seg =  hal::pd::find_l2seg_by_hwid(cpu_rxhdr_->lkp_vrf);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("fte: l2seg not found, hwid={}", cpu_rxhdr_->lkp_vrf);
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    // extract src/dst/proto
    switch (cpu_rxhdr_->lkp_type) {
    case FLOW_KEY_LOOKUP_TYPE_MAC:
        ethhdr = (ether_header_t *)(pkt_ + cpu_rxhdr_->l2_offset);
        key_.flow_type = hal::FLOW_TYPE_L2;
        key_.l2seg_id = l2seg->seg_id;
        memcpy(key_.smac, ethhdr->smac, sizeof(key_.smac));
        memcpy(key_.dmac, ethhdr->dmac, sizeof(key_.dmac));
        key_.ether_type = (cpu_rxhdr_->flags&CPU_FLAGS_VLAN_VALID) ?
            ntohs(((vlan_header_t*)ethhdr)->etype): ntohs(ethhdr->etype);
        break;

    case FLOW_KEY_LOOKUP_TYPE_IPV4: 
        iphdr = (ipv4_header_t*)(pkt_ + cpu_rxhdr_->l3_offset);
        key_.flow_type = hal::FLOW_TYPE_V4;
        key_.tenant_id = hal::tenant_lookup_by_handle(l2seg->tenant_handle)->tenant_id; 
        key_.sip.v4_addr = ntohl(iphdr->saddr);
        key_.dip.v4_addr = ntohl(iphdr->daddr);
        key_.proto = (types::IPProtocol) iphdr->protocol;
        break;

    case FLOW_KEY_LOOKUP_TYPE_IPV6: 
        iphdr6 = (ipv6_header_t *)(pkt_ + cpu_rxhdr_->l3_offset);
        key_.flow_type = hal::FLOW_TYPE_V6;
        key_.tenant_id = hal::tenant_lookup_by_handle(l2seg->tenant_handle)->tenant_id;
        memcpy(key_.sip.v6_addr.addr8, iphdr6->saddr, sizeof(key_.sip.v6_addr.addr8));
        memcpy(key_.dip.v6_addr.addr8, iphdr6->daddr, sizeof(key_.dip.v6_addr.addr8));
        key_.proto = (types::IPProtocol) iphdr6->nexthdr;
        break;

    default:
        HAL_TRACE_ERR("Unsupported lkp_type {}", cpu_rxhdr_->lkp_type);
        return HAL_RET_INVALID_ARG;
    }

    // extract l4 info
    if (cpu_rxhdr_->l4_offset > 0) {
        switch (key_.proto) {
        case IPPROTO_TCP: 
            tcphdr = (tcp_header_t*)(pkt_ + cpu_rxhdr_->l4_offset);
            key_.sport = ntohs(tcphdr->sport);
            key_.dport = ntohs(tcphdr->dport);
            break;
            
        case IPPROTO_UDP: 
            udphdr = (udp_header_t*)(pkt_ + cpu_rxhdr_->l4_offset);
            key_.sport = ntohs(udphdr->sport);
            key_.dport = ntohs(udphdr->dport);
            break;
            
        case IPPROTO_ICMP:
        case IPPROTO_ICMPV6:
            icmphdr = (icmp_header_t*)(pkt_ + cpu_rxhdr_->l4_offset);
            key_.icmp_type =  icmphdr->type;
            key_.icmp_code = icmphdr->code;
            key_.icmp_id = ntohs(icmphdr->echo.id);
            break;
            
        case IPPROTO_ESP:
            esphdr = (ipsec_esp_header_t*)(pkt_ + cpu_rxhdr_->l4_offset);
            key_.spi = ntohl(esphdr->spi);
            break;
            
        default:
            key_.sport = key_.dport = 0;
        }
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
        tid = hal::tenant_lookup_by_handle(l2seg->tenant_handle)->tenant_id;
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
    if (sep_ == NULL && dep_ == NULL) {
        HAL_TRACE_ERR("fte: both src and dst ep unknown, key={}", key_);
        return HAL_RET_EP_NOT_FOUND;
    }

    if (sep_) {
        if (protobuf_request()) {
            key_.dir = (sep_->ep_flags & EP_FLAGS_LOCAL)? FLOW_DIR_FROM_DMA :
                FLOW_DIR_FROM_UPLINK;
        }
        sl2seg_ = hal::find_l2seg_by_handle(sep_->l2seg_handle);
        HAL_ASSERT_RETURN(sl2seg_, HAL_RET_L2SEG_NOT_FOUND);
        sif_ = hal::find_if_by_handle(sep_->if_handle);
        HAL_ASSERT_RETURN(sif_ , HAL_RET_IF_NOT_FOUND);
    } else {
        HAL_TRACE_ERR("fte: src ep unknown, key={}", key_);
        if (!cpu_rxhdr_) {
            return HAL_RET_EP_NOT_FOUND;            
        }

        // lookup l2seg from cpuheader lkp_vrf
        sl2seg_ =  hal::pd::find_l2seg_by_hwid(cpu_rxhdr_->lkp_vrf);
        HAL_ASSERT_RETURN(sl2seg_, HAL_RET_L2SEG_NOT_FOUND);

        //sif_ = hal::find_if_by_hwid(cpu_rxhdr_->src_lif);
        //HAL_ASSERT_RETURN(sif_ , HAL_RET_IF_NOT_FOUND);
    }

    if (dep_) {
        dl2seg_ = hal::find_l2seg_by_handle(dep_->l2seg_handle);
        HAL_ASSERT_RETURN(dl2seg_, HAL_RET_L2SEG_NOT_FOUND);
        dif_ = hal::find_if_by_handle(dep_->if_handle);
        HAL_ASSERT_RETURN(dif_, HAL_RET_IF_NOT_FOUND);
    } else {
        HAL_TRACE_INFO("fte: dest ep unknown, key={}", key_);
    }

    return HAL_RET_OK;
}

const hal::flow_key_t& 
ctx_t::get_key(hal::flow_role_t role)
{
    flow_t *flow = NULL;

    if (role == hal::FLOW_ROLE_INITIATOR) {
        flow = iflow_[istage_];
    } else {
        flow = rflow_[rstage_];
    }

    HAL_ASSERT(flow != NULL);

    return flow->key();
}

hal_ret_t
ctx_t::lookup_session()
{
    hal::flow_t *hflow = NULL;
    int stage = 0;
    alg_entry_t  *entry = NULL;

    // TODO(pavithra) handle protobuf requests
    if (protobuf_request()) {
        return HAL_RET_SESSION_NOT_FOUND; 
    }

    entry = (alg_entry_t *)lookup_alg_db(this);
    if (entry) {
        // ALG Entry found
        session_ = entry->session;
        set_alg_proto_state(entry->alg_proto_state);
         
        // This is an RFlow if we found its key and role
        // set as responder. Initialize the rflow_ stage
        if (entry->role == hal::FLOW_ROLE_RESPONDER) {
            valid_rflow_ = true;
            rflow_[stage]->set_key(key());
        }

        set_role(entry->role);  
        set_flow_miss((arm_lifq_==FLOW_MISS_LIFQ));
        set_alg_entry(*entry);
        if (!flow_miss()) key_ = entry->key;
    }

    if (!session_) {
        session_ = hal::session_lookup(key_, std::addressof(role_));
    }

    if (!session_) {
        existing_session_ = false;
        return HAL_RET_SESSION_NOT_FOUND;
    }

    HAL_TRACE_DEBUG("fte: found existing session");
    existing_session_ = true;
    hflow = session_->iflow;
    // TODO(goli) handle post svc flows
    if (hflow->config.role == hal::FLOW_ROLE_INITIATOR) {
        iflow_[stage]->from_config(hflow->config, hflow->pgm_attrs);
            if (hflow->reverse_flow) {
                rflow_[stage]->from_config(hflow->reverse_flow->config, 
                                               hflow->reverse_flow->pgm_attrs);
                valid_rflow_ = true;
            }
    } else {
        rflow_[stage]->from_config(hflow->config, hflow->pgm_attrs);
        iflow_[stage]->from_config(hflow->reverse_flow->config, hflow->reverse_flow->pgm_attrs);
        valid_rflow_ = true;
    }

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
 
    cleanup_hal_ = false;
    num_handlers_ = 0;
   
    // read rkey from spec
    if (protobuf_request()) {
        if (sess_spec_->has_responder_flow()) {
            ret = extract_flow_key_from_spec(sess_spec_->meta().tenant_id(),
                                             &rkey,
                                             sess_spec_->responder_flow().flow_key());
            
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

    set_role(hal::FLOW_ROLE_INITIATOR);
    if (arm_lifq_.lif == hal::SERVICE_LIF_CPU) { 
        set_flow_miss(true);
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::update_flow_table()
{
    hal_ret_t       ret;
    hal_handle_t    session_handle;
    hal::session_t *session = NULL;

    hal::session_args_t session_args = {};
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

    if (!flow_miss()) {
      return HAL_RET_OK;
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

        iflow_attrs.tenant_hwid = hal::pd::pd_l2seg_get_ten_hwid(sl2seg_);

        // TODO(goli) fix tnnl_rw_idx lookup
        if (iflow_attrs.tnnl_rw_act == TUNNEL_REWRITE_NOP_ID) {
            iflow_attrs.tnnl_rw_idx = 0;
        } else if (iflow_attrs.tnnl_rw_act == TUNNEL_REWRITE_ENCAP_VLAN_ID) {
            iflow_attrs.tnnl_rw_idx = 1;
        } else if (dif_ && dif_->if_type == intf::IF_TYPE_TUNNEL) {
            iflow_attrs.tnnl_rw_idx = hal::pd::pd_tunnelif_get_rw_idx((hal::pd::pd_tunnelif_t *)dif_->pd_if);
        }

        session_args.iflow[stage] = &iflow_cfg;
        session_args.iflow_attrs[stage] = &iflow_attrs;

        if (iflow->valid_flow_state()) {
            session_cfg.conn_track_en = true;
            session_args.session_state = &session_state;
            session_state.iflow_state = iflow->flow_state();
        }

        HAL_TRACE_DEBUG("fte::update_flow_table: iflow.{} key={} lkp_inst={} action={} smac_rw={} dmac-rw={} "
                        "ttl_dec={} mcast={} lport={} qid_en={} qtype={} qid={} rw_act={} "
                        "rw_idx={} tnnl_rw_act={} tnnl_rw_idx={} tnnl_vnid={} nat_sip={} "
                        "nat_dip={} nat_sport={} nat_dport={} nat_type={} slif_en={} slif={}",
                        stage, iflow_cfg.key, iflow_attrs.lkp_inst, iflow_cfg.action,
                        iflow_attrs.mac_sa_rewrite,
                        iflow_attrs.mac_da_rewrite, iflow_attrs.ttl_dec, iflow_attrs.mcast_en,
                        iflow_attrs.lport, iflow_attrs.qid_en, iflow_attrs.qtype, iflow_attrs.qid,
                        iflow_attrs.rw_act, iflow_attrs.rw_idx, iflow_attrs.tnnl_rw_act,
                        iflow_attrs.tnnl_rw_idx, iflow_attrs.tnnl_vnid, iflow_cfg.nat_sip,
                        iflow_cfg.nat_dip, iflow_cfg.nat_sport, iflow_cfg.nat_dport,
                        iflow_cfg.nat_type, iflow_attrs.expected_src_lif_en, iflow_attrs.expected_src_lif);
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

        if (dl2seg_ == NULL) {
            HAL_TRACE_DEBUG("fte: dest l2seg not known");
            return HAL_RET_L2SEG_NOT_FOUND;
        }

        rflow_attrs.tenant_hwid = hal::pd::pd_l2seg_get_ten_hwid(dl2seg_);

        // TODO(goli) fix tnnl w_idx lookup
        if (rflow_attrs.tnnl_rw_act == TUNNEL_REWRITE_NOP_ID) {
            rflow_attrs.tnnl_rw_idx = 0;
        } else if (rflow_attrs.tnnl_rw_act == TUNNEL_REWRITE_ENCAP_VLAN_ID) {
            rflow_attrs.tnnl_rw_idx = 1;
        } else if (sif_ && sif_->if_type == intf::IF_TYPE_TUNNEL) {
            rflow_attrs.tnnl_rw_idx = hal::pd::pd_tunnelif_get_rw_idx((hal::pd::pd_tunnelif_t *)sif_->pd_if);
        }

        session_args.rflow[stage] = &rflow_cfg;
        session_args.rflow_attrs[stage] = &rflow_attrs;
            
        if (rflow->valid_flow_state()) {
            session_state.rflow_state = rflow->flow_state();
        }
            
        HAL_TRACE_DEBUG("fte::update_flow_table: rflow.{} key={} lkp_inst={} action={} smac_rw={} dmac-rw={} "
                        "ttl_dec={} mcast={} lport={} qid_en={} qtype={} qid={} rw_act={} "
                        "rw_idx={} tnnl_rw_act={} tnnl_rw_idx={} tnnl_vnid={} nat_sip={} "
                        "nat_dip={} nat_sport={} nat_dport={} nat_type={} slif_en={} slif={}",
                        stage, rflow_cfg.key, rflow_attrs.lkp_inst, rflow_cfg.action,
                        rflow_attrs.mac_sa_rewrite,
                        rflow_attrs.mac_da_rewrite, rflow_attrs.ttl_dec, rflow_attrs.mcast_en,
                        rflow_attrs.lport, rflow_attrs.qid_en, rflow_attrs.qtype, rflow_attrs.qid,
                        rflow_attrs.rw_act, rflow_attrs.rw_idx, rflow_attrs.tnnl_rw_act,
                        rflow_attrs.tnnl_rw_idx, rflow_attrs.tnnl_vnid, rflow_cfg.nat_sip,
                        rflow_cfg.nat_dip, rflow_cfg.nat_sport, rflow_cfg.nat_dport,
                        rflow_cfg.nat_type, rflow_attrs.expected_src_lif_en, rflow_attrs.expected_src_lif);
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
    session_args.valid_rflow = valid_rflow_;

    if (hal_cleanup() == true) {
        // Cleanup session if hal_cleanup is set
        if (session_) {
            ret = hal::session_delete(&session_args, session_);
        }
    } else if (session_) { 
        // Update session if it already exists
        ret = hal::session_update(&session_args, session_);
    } else {
        // Create a new HAL session
        ret = hal::session_create(&session_args, &session_handle, &session);
        session_ = session;
    }

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
    ipvx_addr_t dip;

    if (!header.valid_flds.dip) {
        return HAL_RET_OK;
    }

    if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
        dep_ = hal::find_ep_by_v4_key(tenant_->tenant_id, header.ipv4.dip);
        dip.v4_addr = header.ipv4.dip;
    } else {
        ip_addr_t addr;
        addr.af = IP_AF_IPV6;
        addr.addr.v6_addr = header.ipv6.dip;
        dep_ = hal::find_ep_by_v6_key(tenant_->tenant_id, &addr);
        dip.v6_addr = header.ipv6.dip;
    }

    if (dep_ == NULL) {
        return HAL_RET_EP_NOT_FOUND;
    }

    dl2seg_ = hal::find_l2seg_by_handle(dep_->l2seg_handle);
    dif_ = hal::find_if_by_handle(dep_->if_handle);
    HAL_ASSERT(dif_ != NULL);

    // If we are doing dnat on iflow, update the rflow's key
    if (role == hal::FLOW_ROLE_INITIATOR && valid_rflow_ ) {
        hal::flow_key_t rkey = {};
        for (int i = 0; i < MAX_STAGES; i++) {
            rkey = rflow_[i]->key();
            if (!this->protobuf_request()) {
                rkey.sip = dip;
            }
            rkey.dir = (dep_->ep_flags & EP_FLAGS_LOCAL) ?
                FLOW_DIR_FROM_DMA : FLOW_DIR_FROM_UPLINK;
            rflow_[i]->set_key(rkey);
        }  
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
        ret = extract_flow_key_from_spec(sess_spec_->meta().tenant_id(),
                                         &key_,
                                         sess_spec_->initiator_flow().flow_key());
    } else {
        ret = extract_flow_key();
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
ctx_t::init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len, flow_t iflow[], flow_t rflow[])
{
    hal_ret_t ret;

    *this = {};
    app_redir().init();

    cpu_rxhdr_ = cpu_rxhdr;
    pkt_ = pkt;
    pkt_len_ = pkt_len;
    arm_lifq_ = {cpu_rxhdr->lif, cpu_rxhdr->qtype, cpu_rxhdr->qid};


    if (cpu_rxhdr->lif == hal::SERVICE_LIF_CPU) {
        ret = init_flows(iflow, rflow);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to init flows, err={}", ret);
            return ret;
        }
    }

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::init(SessionSpec* spec, SessionResponse *rsp, flow_t iflow[], flow_t rflow[])
{
    hal_ret_t ret;

    *this = {};
    app_redir().init();

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
    return update_flow(flowupd, role());
}

hal_ret_t
ctx_t::update_flow(const flow_update_t& flowupd, 
                   const hal::flow_role_t role)
{
    hal_ret_t ret;

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
        }
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} action={}",
                        role, feature_name_, ret, flowupd.action);
        break;

    case FLOWUPD_HEADER_REWRITE:
        ret = flow->header_rewrite(flowupd.header_rewrite);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} header_rewrite={}",
                        role, feature_name_, ret, flowupd.header_rewrite);
        if (ret == HAL_RET_OK) {
            // check if dep needs to be updated
            ret = update_for_dnat(role, flowupd.header_rewrite);
        }
        break;

    case FLOWUPD_HEADER_PUSH:
        ret = flow->header_push(flowupd.header_push);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} header_push={}",
                        role, feature_name_, ret, flowupd.header_push);
        break;

    case FLOWUPD_HEADER_POP:
        ret = flow->header_pop(flowupd.header_pop);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} header_pop={}",
                        role, feature_name_, ret, flowupd.header_pop);
        break;

    case FLOWUPD_FLOW_STATE:
        ret = flow->set_flow_state(flowupd.flow_state);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} flow_state={}",
                        role, feature_name_, ret, flowupd.flow_state);
        break;

    case FLOWUPD_FWDING_INFO:
        ret = flow->set_fwding(flowupd.fwding);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} fwding_info={}",
                        role, feature_name_, ret, flowupd.fwding);
        if (flowupd.fwding.dif) {
            dif_ = flowupd.fwding.dif;
        }
        if (flowupd.fwding.dl2seg) {
            dl2seg_ =  flowupd.fwding.dl2seg;
        }
        break;

    case FLOWUPD_KEY:
        ret = flow->set_key(flowupd.key);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} key={}",
                        role, feature_name_, ret, flowupd.key);
        break;

    case FLOWUPD_MCAST_COPY:
        ret = flow->set_mcast_info(flowupd.mcast_info);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} mcast_info={}",
                        role, feature_name_, ret, flowupd.mcast_info);
        break;
    case FLOWUPD_INGRESS_INFO:
        ret = flow->set_ingress_info(flowupd.ingress_info);
        HAL_TRACE_DEBUG("fte::update_flow {} feature={} ret={} ingress_info={}",
                        role, feature_name_, ret, flowupd.ingress_info);
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

hal_ret_t
ctx_t::queue_txpkt(uint8_t *pkt, size_t pkt_len,
                   hal::pd::cpu_to_p4plus_header_t *cpu_header,
                   hal::pd::p4plus_to_p4_header_t  *p4plus_header,
                   uint16_t dest_lif, uint8_t  qtype, uint32_t qid,
                   uint8_t  ring_number)
{
    txpkt_info_t *pkt_info;
    
    if (txpkt_cnt_ >= MAX_QUEUED_PKTS) {
        HAL_TRACE_ERR("fte: queued tx pkts exceeded {}", txpkt_cnt_);
        return HAL_RET_ERR;
    }
    
    pkt_info = &txpkts_[txpkt_cnt_++];
    HAL_TRACE_DEBUG("fte: txpkt for dir={}", key_.dir);
    if (cpu_header) {
        pkt_info->cpu_header = *cpu_header;
    } else {
        pkt_info->cpu_header.src_lif = cpu_rxhdr_->src_lif;
        // change lif/vlan for uplink pkts
        if (key_.dir == FLOW_DIR_FROM_UPLINK) {
     	    HAL_TRACE_DEBUG("fte: setting defaults for uplink -> host direction");
            pkt_info->cpu_header.src_lif = hal::SERVICE_LIF_CPU;
            if (hal::pd::pd_l2seg_get_fromcpu_id(sl2seg_, &pkt_info->cpu_header.hw_vlan_id)) {
                pkt_info->cpu_header.flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
            }
        }
    }
    
    if (p4plus_header) {
        pkt_info->p4plus_header = *p4plus_header;
    }

    pkt_info->pkt = pkt;
    pkt_info->pkt_len = pkt_len;
    pkt_info->lifq.lif = dest_lif;
    pkt_info->lifq.qtype = qtype;
    pkt_info->lifq.qid = qid;
    pkt_info->ring_number = ring_number;

    HAL_TRACE_DEBUG("fte: feature={} queued txpkt lkp_inst={} src_lif={} vlan={} "
                    "dest_lifq={} ring={} pkt={:p} len={}",
                    feature_name_,
                    pkt_info->p4plus_header.flags & P4PLUS_TO_P4_FLAGS_LKP_INST,
                    pkt_info->cpu_header.src_lif,
                    pkt_info->cpu_header.hw_vlan_id,
                    pkt_info->lifq, pkt_info->ring_number,
                    pkt_info->pkt, pkt_info->pkt_len);

    return HAL_RET_OK;
}

hal_ret_t
ctx_t::send_queued_pkts(hal::pd::cpupkt_ctxt_t* arm_ctx)
{
    hal_ret_t ret;

    // queue rx pkt if tx_queue is empty, it is a flow miss and firwall action is not drop
    if(pkt_ != NULL && txpkt_cnt_ == 0 && flow_miss() && !drop()) {
        queue_txpkt(pkt_, pkt_len_);
    }

    for (int i = 0; i < txpkt_cnt_; i++) {
        txpkt_info_t *pkt_info = &txpkts_[i];
        HAL_TRACE_DEBUG("fte:: txpkt slif={} pkt={:p} len={}",
                        pkt_info->cpu_header.src_lif,
                        pkt_info->pkt, pkt_info->pkt_len);

        if ( istage_ > 0 ){
            pkt_info->p4plus_header.flags |=  P4PLUS_TO_P4_FLAGS_LKP_INST;
        }


        pkt_info->p4plus_header.p4plus_app_id = P4PLUS_APPTYPE_CPU;

        ret = hal::pd::cpupkt_send(arm_ctx,
                                   types::WRING_TYPE_ASQ,
                                   0,
                                   &pkt_info->cpu_header,
                                   &pkt_info->p4plus_header,
                                   pkt_info->pkt, pkt_info->pkt_len,
                                   pkt_info->lifq.lif, pkt_info->lifq.qtype,
                                   pkt_info->lifq.qid,  pkt_info->ring_number);

        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failied to transmit pkt, ret={}", ret);
        }
    }

    txpkt_cnt_ = 0;

    return HAL_RET_OK;
}

void
ctx_t::invoke_completion_handlers(bool fail)
{
    HAL_TRACE_DEBUG("fte: invoking completion handlers count={}", num_handlers_);
    for (int i = 0; i < num_handlers_; i++) {
        HAL_TRACE_DEBUG("fte: invoking completion handler {:p}", (void*)(completion_handlers_[i]));
        (*completion_handlers_[i])(*this, fail);
    }
}

//
// Process the packet and update the flow table
hal_ret_t
ctx_t::process()
{
    hal_ret_t ret;

    // execute the pipeline
    ret = execute_pipeline(*this);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failied to execute pipeline, ret={}", ret);
        goto end;
    }

    // update flow table
    ret = update_flow_table();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failied to updated gft, ret={}", ret);
        goto end;
    }

 end:
    // Invoke completion handlers
    invoke_completion_handlers((ret==HAL_RET_OK));

    return ret;
}

/*-----------------------------------------------------------------------
  - Swap the derived flow objects for reverse flow processing.
  -------------------------------------------------------------------------*/
void
ctx_t::swap_flow_objs()
{
    hal::if_t    *dif = sif_;
    hal::ep_t    *dep = sep_;
    hal::l2seg_t *dl2seg = sl2seg_;

    sif_ = dif_;
    dif_ = dif;
    sep_ = dep_;
    dep_ = dep;
    sl2seg_ = dl2seg_;
    dl2seg_ = dl2seg;
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

std::ostream& operator<<(std::ostream& os, const mcast_info_t& val)
{
    os << "{mcast_en=" << (bool)val.mcast_en;
    if (val.mcast_en) {
        os << " ,mcast_ptr=" << val.mcast_ptr;
    }
    return os << "}";
}

} // namespace fte
