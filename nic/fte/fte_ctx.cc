#include "fte.hpp"
#include "fte_ctx.hpp"
#include "fte_flow.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "sdk/list.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/cpupkt_headers.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"

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
// extract flow key from packet
//------------------------------------------------------------------------------
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
    hal::pd::pd_get_object_from_flow_lkupid_args_t args;
    hal::l2seg_t *l2seg = NULL;
    hal::hal_obj_id_t obj_id;
    void *obj;
    hal_ret_t ret;

    HAL_ASSERT_RETURN(cpu_rxhdr_ != NULL && pkt_ != NULL, HAL_RET_INVALID_ARG);

    key_.dir = cpu_rxhdr_->lkp_dir;

    args.flow_lkupid = cpu_rxhdr_->lkp_vrf;
    args.obj_id = &obj_id;
    args.pi_obj = &obj;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, (void *)&args);
    if (ret != HAL_RET_OK || obj_id != hal::HAL_OBJ_ID_L2SEG) {
        HAL_TRACE_ERR("fte: Invalid obj id: {}, ret: {}", obj_id, ret);
        return HAL_RET_L2SEG_NOT_FOUND;
    }
    l2seg = (hal::l2seg_t *)obj;

    key_.svrf_id = key_.dvrf_id = hal::vrf_lookup_by_handle(l2seg->vrf_handle)->vrf_id;

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
        key_.sip.v4_addr = ntohl(iphdr->saddr);
        key_.dip.v4_addr = ntohl(iphdr->daddr);
        key_.proto = (types::IPProtocol) iphdr->protocol;
        break;

    case FLOW_KEY_LOOKUP_TYPE_IPV6:
        iphdr6 = (ipv6_header_t *)(pkt_ + cpu_rxhdr_->l3_offset);
        key_.flow_type = hal::FLOW_TYPE_V6;
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

//------------------------------------------------------------------------------
// Lookup teannt/ep/if/l2seg from flow lookup key
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::lookup_flow_objs()
{
    ether_header_t *ethhdr;
    hal::pd::pd_get_object_from_flow_lkupid_args_t args;
    hal::hal_obj_id_t obj_id;
    void *obj;
    hal_ret_t ret;

    svrf_ = hal::vrf_lookup_by_id(key_.svrf_id);
    if (svrf_ == NULL) {
        HAL_TRACE_ERR("fte: vrf not found, key {}", key_);
        return HAL_RET_VRF_NOT_FOUND;
    }

    if (key_.svrf_id == key_.dvrf_id) {
        dvrf_ = svrf_;
    } else {
        dvrf_ = hal::vrf_lookup_by_id(key_.dvrf_id);
        if (dvrf_ == NULL) {
            HAL_TRACE_ERR("fte: dvrf not found, key {}", key_);
            return HAL_RET_VRF_NOT_FOUND;
        }
    }

    //Lookup src and dest EPs
    hal::ep_get_from_flow_key(&key_, &sep_, &dep_);

    if (sep_) {
        if (protobuf_request()) {
            key_.dir = (sep_->ep_flags & EP_FLAGS_LOCAL)? FLOW_DIR_FROM_DMA :
                FLOW_DIR_FROM_UPLINK;
        }
        sl2seg_ = hal::l2seg_lookup_by_handle(sep_->l2seg_handle);
        HAL_ASSERT_RETURN(sl2seg_, HAL_RET_L2SEG_NOT_FOUND);
        sif_ = hal::find_if_by_handle(sep_->if_handle);
        HAL_ASSERT_RETURN(sif_ , HAL_RET_IF_NOT_FOUND);
    } else {
        HAL_TRACE_ERR("fte: src ep unknown, key={}", key_);
        if (!cpu_rxhdr_) {
            return HAL_RET_EP_NOT_FOUND;
        }

        args.flow_lkupid = cpu_rxhdr_->lkp_vrf;
        args.obj_id = &obj_id;
        args.pi_obj = &obj;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, (void *)&args);
        if (ret != HAL_RET_OK || obj_id != hal::HAL_OBJ_ID_L2SEG) {
            HAL_TRACE_ERR("fte: Invalid obj id: {}, ret:{}", obj_id, ret);
            return HAL_RET_L2SEG_NOT_FOUND;
        }
        sl2seg_ = (hal::l2seg_t *)obj;

        // Try to find sep by looking at L2.
         ethhdr = (ether_header_t *)(pkt_ + cpu_rxhdr_->l2_offset);
         sep_ = hal::find_ep_by_l2_key(sl2seg_->seg_id, ethhdr->smac);
         if (sep_) {
             HAL_TRACE_INFO("fte: src ep found by L2 lookup, key={}", key_);
             sif_ = hal::find_if_by_handle(sep_->if_handle);
             HAL_ASSERT_RETURN(sif_ , HAL_RET_IF_NOT_FOUND);
         }
    }

    if (sep_) {
        sep_handle_ = sep_->hal_handle;
    }

    if (dep_) {
        dl2seg_ = hal::l2seg_lookup_by_handle(dep_->l2seg_handle);
        HAL_ASSERT_RETURN(dl2seg_, HAL_RET_L2SEG_NOT_FOUND);
        dif_ = hal::find_if_by_handle(dep_->if_handle);
        HAL_ASSERT_RETURN(dif_, HAL_RET_IF_NOT_FOUND);
    } else {
        HAL_TRACE_INFO("fte: dest ep unknown, key={}", key_);
    }

    if ((key_.sip.v4_addr == (0x0a010001)) && (key_.proto == IPPROTO_ESP)) {
        HAL_TRACE_ERR("Ramesh : overwriting direction 1");
        key_.dir = FLOW_DIR_FROM_DMA;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Returns flow key of the specified flow
//------------------------------------------------------------------------------
const hal::flow_key_t&
ctx_t::get_key(hal::flow_role_t role)
{
    flow_t *flow = NULL;

    if (role == hal::FLOW_ROLE_NONE) {
        role = role_;
    }

    if (role == hal::FLOW_ROLE_INITIATOR) {
        flow = iflow_[istage_];
    } else {
        flow = rflow_[rstage_];
    }

    HAL_ASSERT(flow != NULL);

    return flow->key();
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

//-----------------------------------------------------------------------------
// Initialize context from the existing session
//-----------------------------------------------------------------------------
void
ctx_t::init_ctxt_from_session(hal::session_t *sess)
{
    hal::flow_t *hflow = NULL;
    int          stage = 0;

    session_ = sess;

    hflow = sess->iflow;
    if (role_ != hal::FLOW_ROLE_INITIATOR) {
        role_ = hal::FLOW_ROLE_INITIATOR;
        key_ = hflow->config.key;
        swap_flow_objs();
    }

    // Init feature sepcific session state
    sdk::lib::dllist_ctxt_t   *entry;
    dllist_for_each(entry, &sess->feature_list_head) {
        feature_session_state_t *state =
            dllist_entry(entry, feature_session_state_t, session_feature_lentry);
        uint16_t id = feature_id(state->feature_name);
        if (id <= num_features_) {
            feature_state_[id].session_state = state;
            HAL_TRACE_DEBUG("fte: feature={} restored session state {:p}",
                            state->feature_name, (void*)state);
        }
    }

    if(role_ == hal::FLOW_ROLE_INITIATOR)
        valid_iflow_ = true;

    // TODO(goli) handle post svc flows
    if (hflow->config.role == hal::FLOW_ROLE_INITIATOR) {
        iflow_[stage]->from_config(hflow->config, hflow->pgm_attrs);
            if (sess->rflow) {
                rflow_[stage]->from_config(sess->rflow->config,
                                           sess->rflow->pgm_attrs);
                valid_rflow_ = true;
                if (sess->rflow->assoc_flow) {
                    rflow_[++stage]->from_config(
                                       sess->rflow->assoc_flow->config,
                                       sess->rflow->assoc_flow->pgm_attrs);
                }
            }
            if (hflow->assoc_flow) {
                iflow_[++stage]->from_config(hflow->assoc_flow->config,
                                           hflow->assoc_flow->pgm_attrs);
            }
    } else {
        rflow_[stage]->from_config(hflow->config, hflow->pgm_attrs);
        if (hflow->assoc_flow) {
            rflow_[++stage]->from_config(hflow->assoc_flow->config,
                                         hflow->assoc_flow->pgm_attrs);
        }
        iflow_[stage]->from_config(hflow->reverse_flow->config,
                                   hflow->reverse_flow->pgm_attrs);
        if (sess->iflow->assoc_flow) {
            iflow_[++stage]->from_config(
                                       sess->iflow->assoc_flow->config,
                                       sess->iflow->assoc_flow->pgm_attrs);
        }

        valid_rflow_ = true;
    }
}

//------------------------------------------------------------------------------
// Lookup existing session for the flow
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::lookup_session()
{
    if (protobuf_request()) {
        return HAL_RET_SESSION_NOT_FOUND;
    }

    session_ = hal::session_lookup(key_, &role_);
    if (!session_) {
        return HAL_RET_SESSION_NOT_FOUND;
    }

    HAL_TRACE_DEBUG("fte: found existing session");

    init_ctxt_from_session(session_);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Creates new seesion for the pkt's flow
//------------------------------------------------------------------------------
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

    valid_iflow_ = true;
    // read rkey from spec
    if (protobuf_request()) {
        if (sess_spec_->has_responder_flow()) {
            ret = extract_flow_key_from_spec(sess_spec_->meta().vrf_id(),
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
        rkey.svrf_id = key_.dvrf_id;
        rkey.dvrf_id = key_.svrf_id;

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

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Create/update session and flow table entries in hardware
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_flow_table()
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    session_handle;
    hal::session_t *session = NULL;
    hal::pd::pd_l2seg_get_flow_lkupid_args_t args;
    hal::pd::pd_tunnelif_get_rw_idx_args_t t_args;

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
        session_state.tcp_sack_perm_option = sess_spec_->tcp_sack_perm_option();
    }

    if (ignore_session_create()) {
        return HAL_RET_OK;
    }

    app_redir_ctx_t* app_ctx = app_redir_ctx(*this, false);
    if (!(flow_miss() ||
        (app_redir_pipeline() && app_ctx && app_ctx->appid_completed()))) {
        return HAL_RET_OK;
    }

    for (uint8_t stage = 0; valid_iflow_ && !hal_cleanup() && stage <= istage_; stage++) {
        flow_t *iflow = iflow_[stage];
        hal::flow_cfg_t &iflow_cfg = iflow_cfg_list[stage];
        hal::flow_pgm_attrs_t& iflow_attrs = iflow_attrs_list[stage];

        // For existing sessions initialize with the configs the session
        // came with and update anything else
        if (existing_session() && stage == 0 && session_->iflow) {
            iflow_cfg = session_->iflow->config;
            iflow_attrs = session_->iflow->pgm_attrs;
        }

        iflow->to_config(iflow_cfg, iflow_attrs);
        iflow_cfg.role = iflow_attrs.role = hal::FLOW_ROLE_INITIATOR;

        // Set the lkp_inst for all stages except the first stage
        if (stage != 0) {
            iflow_attrs.lkp_inst = 1;
        }

        args.l2seg = sl2seg_;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, (void *)&args);
        iflow_attrs.vrf_hwid = args.hwid;
        // iflow_attrs.vrf_hwid = hal::pd::pd_l2seg_get_flow_lkupid(sl2seg_);

        // TODO(goli) fix tnnl_rw_idx lookup
        if (iflow_attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_NOP_ID) {
            iflow_attrs.tnnl_rw_idx = 0;
        } else if (iflow_attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_ENCAP_VLAN_ID) {
            iflow_attrs.tnnl_rw_idx = 1;
        } else if (dif_ && dif_->if_type == intf::IF_TYPE_TUNNEL) {
            t_args.hal_if = dif_;
            ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TNNL_IF_GET_RW_IDX,
                                       (void *)&t_args);
            iflow_attrs.tnnl_rw_idx = t_args.tnnl_rw_idx;
        }

        session_args.iflow[stage] = &iflow_cfg;
        session_args.iflow_attrs[stage] = &iflow_attrs;

        if (iflow->valid_flow_state()) {
            session_cfg.conn_track_en = true;
            session_args.session_state = &session_state;
            session_state.iflow_state = iflow->flow_state();
        }

        if (is_proxy_enabled()) {
            iflow_attrs.is_proxy_en = 1;
        }

        HAL_TRACE_DEBUG("fte::update_flow_table: iflow.{} key={} lkp_inst={} action={} smac_rw={} dmac-rw={} "
                        "ttl_dec={} mcast={} lport={} qid_en={} qtype={} qid={} rw_act={} "
                        "rw_idx={} tnnl_rw_act={} tnnl_rw_idx={} tnnl_vnid={} nat_sip={} "
                        "nat_dip={} nat_sport={} nat_dport={} nat_type={} is_ing_proxy_mirror={} "
                        "is_eg_proxy_mirror={} slif_en={} slif={} qos_class_en={} "
                        "qos_class_id={} is_proxy_en={} is_proxy_mcast={}",
                        stage, iflow_cfg.key, iflow_attrs.lkp_inst, iflow_cfg.action,
                        iflow_attrs.mac_sa_rewrite,
                        iflow_attrs.mac_da_rewrite, iflow_attrs.ttl_dec, iflow_attrs.mcast_en,
                        iflow_attrs.lport, iflow_attrs.qid_en, iflow_attrs.qtype, iflow_attrs.qid,
                        iflow_attrs.rw_act, iflow_attrs.rw_idx, iflow_attrs.tnnl_rw_act,
                        iflow_attrs.tnnl_rw_idx, iflow_attrs.tnnl_vnid, iflow_cfg.nat_sip,
                        iflow_cfg.nat_dip, iflow_cfg.nat_sport, iflow_cfg.nat_dport,
                        iflow_cfg.nat_type, iflow_cfg.is_ing_proxy_mirror, iflow_cfg.is_eg_proxy_mirror,
                        iflow_attrs.expected_src_lif_en, iflow_attrs.expected_src_lif,
                        iflow_attrs.qos_class_en, iflow_attrs.qos_class_id, iflow_attrs.is_proxy_en,
                        iflow_attrs.is_proxy_mcast);
    }

    for (uint8_t stage = 0; valid_rflow_ && !hal_cleanup() && stage <= rstage_; stage++) {
        flow_t *rflow = rflow_[stage];
        hal::flow_cfg_t &rflow_cfg = rflow_cfg_list[stage];
        hal::flow_pgm_attrs_t& rflow_attrs = rflow_attrs_list[stage];

        // For existing sessions initialize with the configs the session
        // came with and update anything else
        if (existing_session() && stage == 0 && session_->rflow) {
            rflow_cfg = session_->rflow->config;
            rflow_attrs = session_->rflow->pgm_attrs;
        }

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

		args.l2seg = dl2seg_;
		hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, (void *)&args);
		rflow_attrs.vrf_hwid = args.hwid;
        // rflow_attrs.vrf_hwid = hal::pd::pd_l2seg_get_flow_lkupid(dl2seg_);

        // TODO(goli) fix tnnl w_idx lookup
        if (rflow_attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_NOP_ID) {
            rflow_attrs.tnnl_rw_idx = 0;
        } else if (rflow_attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_ENCAP_VLAN_ID) {
            rflow_attrs.tnnl_rw_idx = 1;
        } else if (sif_ && sif_->if_type == intf::IF_TYPE_TUNNEL) {
			t_args.hal_if = sif_;
			ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TNNL_IF_GET_RW_IDX,
									   (void *)&t_args);
			rflow_attrs.tnnl_rw_idx = t_args.tnnl_rw_idx;
        }

        session_args.rflow[stage] = &rflow_cfg;
        session_args.rflow_attrs[stage] = &rflow_attrs;

        if (rflow->valid_flow_state()) {
            session_state.rflow_state = rflow->flow_state();
        }

        HAL_TRACE_DEBUG("fte::update_flow_table: rflow.{} key={} lkp_inst={} action={} smac_rw={} dmac-rw={} "
                        "ttl_dec={} mcast={} lport={} qid_en={} qtype={} qid={} rw_act={} "
                        "rw_idx={} tnnl_rw_act={} tnnl_rw_idx={} tnnl_vnid={} nat_sip={} "
                        "nat_dip={} nat_sport={} nat_dport={} nat_type={} slif_en={} slif={} "
                        "qos_class_en={} qos_class_id={}",
                        stage, rflow_cfg.key, rflow_attrs.lkp_inst, rflow_cfg.action,
                        rflow_attrs.mac_sa_rewrite,
                        rflow_attrs.mac_da_rewrite, rflow_attrs.ttl_dec, rflow_attrs.mcast_en,
                        rflow_attrs.lport, rflow_attrs.qid_en, rflow_attrs.qtype, rflow_attrs.qid,
                        rflow_attrs.rw_act, rflow_attrs.rw_idx, rflow_attrs.tnnl_rw_act,
                        rflow_attrs.tnnl_rw_idx, rflow_attrs.tnnl_vnid, rflow_cfg.nat_sip,
                        rflow_cfg.nat_dip, rflow_cfg.nat_sport, rflow_cfg.nat_dport,
                        rflow_cfg.nat_type, rflow_attrs.expected_src_lif_en, rflow_attrs.expected_src_lif,
                        rflow_attrs.qos_class_en, rflow_attrs.qos_class_id);
    }

    session_args.vrf         = svrf_;
    session_args.sep         = sep_;
    session_args.dep         = dep_;
    session_args.sif         = sif_;
    session_args.dif         = dif_;
    session_args.sl2seg      = sl2seg_;
    session_args.dl2seg      = dl2seg_;
    session_args.spec        = sess_spec_;
    session_args.rsp         = sess_resp_;
    session_args.valid_rflow = valid_rflow_;

    if (hal_cleanup() == true) {
        // Cleanup session if hal_cleanup is set
        if (session_) {
            ret = hal::session_delete(&session_args, session_);
        }
    } else if (session_) {
        if (update_session_) {
            HAL_TRACE_DEBUG("Updating Session");
            // Update session if it already exists
            ret = hal::session_update(&session_args, session_);
        }
    } else {
        // Create a new HAL session
        ret = hal::session_create(&session_args, &session_handle, &session);
        if (ret == HAL_RET_OK) {
            session_ = session;
            // Insert session sepcific feature state
            for (uint16_t id = 0; id < num_features_; id++) {
                feature_session_state_t *state = feature_state_[id].session_state;
                if (state) {
                    sdk::lib::dllist_add(&session_->feature_list_head,
                                           &state->session_feature_lentry);
                }
            }
        }
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

//------------------------------------------------------------------------------
// Update the dest ep/if/l2seg on dnat change
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_for_dnat(hal::flow_role_t role, const header_rewrite_info_t& header)
{
    ipvx_addr_t dip;

    if (header.valid_flds.dvrf_id) {
        if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            key_.dvrf_id = header.ipv4.dvrf_id;
        } else {
            key_.dvrf_id = header.ipv6.dvrf_id;
        }

        dvrf_ =  hal::vrf_lookup_by_id(key_.dvrf_id);

        if (dvrf_ == NULL) {
            HAL_TRACE_ERR("DNAT vrf not found vrf={}", key_.dvrf_id);
            return HAL_RET_VRF_NOT_FOUND;
        }
    }

    if (header.valid_flds.dip) {
        if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            dep_ = hal::find_ep_by_v4_key(dvrf_->vrf_id, header.ipv4.dip);
            dip.v4_addr = header.ipv4.dip;
        } else {
            ip_addr_t addr;
            addr.af = IP_AF_IPV6;
            addr.addr.v6_addr = header.ipv6.dip;
            dep_ = hal::find_ep_by_v6_key(dvrf_->vrf_id, &addr);
            dip.v6_addr = header.ipv6.dip;
        }

        if (!this->protobuf_request()) {
            key_.dip = dip;
        }

    } else if (dep_ == NULL && header.valid_flds.dmac && sl2seg_ != NULL) {
        /* L2 DSR - lookup EP using mac */
        dep_ = hal::find_ep_by_l2_key(sl2seg_->seg_id,
                                      header.ether.dmac.ether_addr_octet);
    } else {
        return HAL_RET_OK;
    }

    if (dep_) {
        dep_handle_ = dep_->hal_handle;
        dl2seg_ = hal::l2seg_lookup_by_handle(dep_->l2seg_handle);
        dif_ = hal::find_if_by_handle(dep_->if_handle);
        HAL_ASSERT(dif_ != NULL);
    }

    // If we are doing dnat on iflow, update the rflow's key
    if (role == hal::FLOW_ROLE_INITIATOR && valid_rflow_ &&  header.valid_flds.dip) {
        hal::flow_key_t rkey = {};
        for (int i = 0; i < MAX_STAGES; i++) {
            rkey = rflow_[i]->key();
            if (!this->protobuf_request()) {
                rkey.sip = dip;
                rkey.svrf_id = dvrf_->vrf_id;
            }
            rkey.dir = (dep_ && dep_->ep_flags & EP_FLAGS_LOCAL) ?
                FLOW_DIR_FROM_DMA : FLOW_DIR_FROM_UPLINK;
            rflow_[i]->set_key(rkey);
        }
    }

    return  HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Update the src key  on snat change
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_for_snat(hal::flow_role_t role, const header_rewrite_info_t& header)
{
    if (this->protobuf_request()) {
        return HAL_RET_OK;
    }

    if (header.valid_flds.svrf_id) {
        if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            key_.svrf_id = header.ipv4.svrf_id;
        } else {
            key_.svrf_id = header.ipv6.svrf_id;
        }

        svrf_ =  hal::vrf_lookup_by_id(key_.svrf_id);

        if (svrf_ == NULL) {
            HAL_TRACE_ERR("SNAT vrf not found vrf={}", key_.svrf_id);
            return HAL_RET_VRF_NOT_FOUND;
        }
    }

    if (header.valid_flds.sip) {
        if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            key_.sip.v4_addr = header.ipv4.sip;
        } else {
            key_.sip.v6_addr = header.ipv6.sip;
        }
    }

    // If we are doing snat on iflow, update the rflow's key
    if (role == hal::FLOW_ROLE_INITIATOR && valid_rflow_) {
        hal::flow_key_t rkey = {};
        for (int i = 0; i < MAX_STAGES; i++) {
            rkey = rflow_[i]->key();
            rkey.dip = key_.sip;
            rkey.dvrf_id = svrf_->vrf_id;
            rflow_[i]->set_key(rkey);
        }
    }

    return  HAL_RET_OK;
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
    if (sess_spec_) {
        ret = extract_flow_key_from_spec(sess_spec_->meta().vrf_id(),
                                         &key_,
                                         sess_spec_->initiator_flow().flow_key());
    } else {
        ret = extract_flow_key();
    }

    if (ret != HAL_RET_OK) {
        return ret;
    }

    HAL_TRACE_DEBUG("fte: extracted flow key {}", key_);


    // Lookup ep, intf, l2seg, vrf
    ret = lookup_flow_objs();
    if (ret != HAL_RET_OK) {
        return ret;
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
    }

    return ret;
}

//------------------------------------------------------------------------------
// Initialize the context
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::init(const lifqid_t &lifq, feature_state_t feature_state[], uint16_t num_features)
{
    *this = {};

    arm_lifq_ = lifq;
    update_session_ = false;

    num_features_ = num_features;
    feature_state_ = feature_state;
    if (num_features) {
        feature_state_init(feature_state_, num_features_);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Initialize the context from incoming pkt
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
            flow_t iflow[], flow_t rflow[],
            feature_state_t feature_state[], uint16_t num_features)
{
    hal_ret_t ret;

    HAL_TRACE_DEBUG("fte: rxpkt cpu_rxhdr={}",
                    hex_str((uint8_t*)cpu_rxhdr, sizeof(*cpu_rxhdr)));

    HAL_TRACE_DEBUG("fte: rxpkt len={} pkt={}", pkt_len, hex_str(pkt, pkt_len));

    HAL_TRACE_DEBUG("fte: rxpkt slif={} lif={} qtype={} qid={} vrf={} "
                    "pad={} lkp_dir={} lkp_inst={} lkp_type={} flags={} "
                    "l2={} l3={} l4={} payload={}",
                    cpu_rxhdr->src_lif, cpu_rxhdr->lif, cpu_rxhdr->qtype,
                    cpu_rxhdr->qid, cpu_rxhdr->lkp_vrf, cpu_rxhdr->pad,
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

    if ((cpu_rxhdr->lif == hal::SERVICE_LIF_CPU) || app_redir_pkt_rx_raw(*this) ||
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
ctx_t::init(SessionSpec* spec, SessionResponse *rsp, flow_t iflow[], flow_t rflow[],
            feature_state_t feature_state[], uint16_t num_features)
{
    hal_ret_t ret;

    ret = init(FLOW_MISS_LIFQ, feature_state, num_features);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to init ctx, err={}", ret);
        return ret;
    }

    sess_spec_ = spec;
    sess_resp_ = rsp;

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

#define LOG_FLOW_UPDATE(__updinfo)                                      \
    HAL_TRACE_DEBUG("fte::update_flow {}.{} feature={} ret={} {}={}",   \
                    role,                                               \
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

    case FLOWUPD_KEY:
        ret = flow->set_key(flowupd.key);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(key);
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

    case FLOWUPD_QOS_INFO:
        ret = flow->set_qos_info(flowupd.qos_info);
        if (ret == HAL_RET_OK) {
            LOG_FLOW_UPDATE(qos_info);
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

    if (existing_session()) {
        if (role_ == hal::FLOW_ROLE_INITIATOR &&
            session()->iflow && session()->iflow->assoc_flow) {
            HAL_ASSERT_RETURN(istage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
            istage_++;
            HAL_TRACE_DEBUG("fte: advancing to next iflow stage {}", istage_);
        } else if (role_ == hal::FLOW_ROLE_RESPONDER &&
                   session()->rflow && session()->rflow->assoc_flow) {
            HAL_ASSERT_RETURN(rstage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
            rstage_++;
            HAL_TRACE_DEBUG("fte: advancing to next rflow stage {}", rstage_);
        }
    } else {

        if (role_ == hal::FLOW_ROLE_INITIATOR && iflow_[istage_]->valid_fwding()) {
            HAL_ASSERT_RETURN(istage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
            istage_++;
            HAL_TRACE_DEBUG("fte: advancing to next iflow stage {}", istage_);
        } else if (rflow_[rstage_]->valid_fwding()){
            HAL_ASSERT_RETURN(rstage_ + 1 < MAX_STAGES, HAL_RET_INVALID_OP);
            rstage_++;
            HAL_TRACE_DEBUG("fte: advancing to next rflow stage {}", rstage_);
        }
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Queues pkt for transmission on ASQ at the end of pipeline processing,
// after updating the flow table
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::queue_txpkt(uint8_t *pkt, size_t pkt_len,
                   hal::pd::cpu_to_p4plus_header_t *cpu_header,
                   hal::pd::p4plus_to_p4_header_t  *p4plus_header,
                   uint16_t dest_lif, uint8_t  qtype, uint32_t qid,
                   uint8_t  ring_number, types::WRingType wring_type,
                   post_xmit_cb_t cb) 
{
    txpkt_info_t *pkt_info;
    hal::pd::pd_l2seg_get_fromcpu_vlanid_args_t args;

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
            args.l2seg = sl2seg_;
            args.vid = &pkt_info->cpu_header.hw_vlan_id;

            if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                                     (void*)&args) == HAL_RET_OK) {

#if 0
            if (hal::pd::pd_l2seg_get_fromcpu_vlanid(sl2seg_,
                                                     &pkt_info->cpu_header.hw_vlan_id) == HAL_RET_OK) {
#endif

                pkt_info->cpu_header.flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
            }
        }
    }

    pkt_info->cpu_header.tm_oq = cpu_rxhdr_->src_tm_iq;

    if (p4plus_header) {
        pkt_info->p4plus_header = *p4plus_header;
    }

    pkt_info->pkt = pkt;
    pkt_info->pkt_len = pkt_len;
    pkt_info->lifq.lif = dest_lif;
    pkt_info->lifq.qtype = qtype;
    pkt_info->lifq.qid = qid;
    pkt_info->ring_number = ring_number;
    pkt_info->wring_type = wring_type;
    pkt_info->cb = cb;

    HAL_TRACE_DEBUG("fte: feature={} queued txpkt lkp_inst={} src_lif={} vlan={} "
                    "dest_lifq={} ring={} wring={} pkt={:p} len={}",
                    feature_name_,
                    pkt_info->p4plus_header.flags & P4PLUS_TO_P4_FLAGS_LKP_INST,
                    pkt_info->cpu_header.src_lif,
                    pkt_info->cpu_header.hw_vlan_id,
                    pkt_info->lifq, pkt_info->ring_number, pkt_info->wring_type,
                    pkt_info->pkt, pkt_info->pkt_len);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Sends all the queued packets
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::send_queued_pkts(hal::pd::cpupkt_ctxt_t* arm_ctx)
{
    hal_ret_t ret;

    // queue rx pkt if tx_queue is empty, it is a flow miss and firwall action is not drop
    if(pkt_ != NULL && txpkt_cnt_ == 0 && flow_miss() && !drop() &&
       !app_redir_pkt_tx_ownership(*this)) {
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

        hal::pd::pd_cpupkt_send_args_t args;
        args.ctxt = arm_ctx;
        args.type = pkt_info->wring_type;
        args.queue_id = pkt_info->wring_type == types::WRING_TYPE_ASQ ? fte_id() : pkt_info->lifq.qid;
        args.cpu_header = &pkt_info->cpu_header;
        args.p4_header = &pkt_info->p4plus_header;
        args.data = pkt_info->pkt;
        args.data_len = pkt_info->pkt_len;
        args.dest_lif = pkt_info->lifq.lif;
        args.qtype = pkt_info->lifq.qtype;
        args.qid = pkt_info->wring_type == types::WRING_TYPE_ASQ ? fte_id() : pkt_info->lifq.qid;
        args.ring_number = pkt_info->ring_number;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND, (void *)&args);
#if 0
        ret = hal::pd::cpupkt_send(arm_ctx,
                                   pkt_info->wring_type,
                                   pkt_info->wring_type == types::WRING_TYPE_ASQ ?
                                   fte_id() : pkt_info->lifq.qid,
                                   &pkt_info->cpu_header,
                                   &pkt_info->p4plus_header,
                                   pkt_info->pkt, pkt_info->pkt_len,
                                   pkt_info->lifq.lif, pkt_info->lifq.qtype,
                                   pkt_info->wring_type == types::WRING_TYPE_ASQ ?
                                   fte_id() : pkt_info->lifq.qid,  pkt_info->ring_number);
#endif

        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to transmit pkt, ret={}", ret);
        }
        // Issue a callback to free the packet
        if (pkt_info->cb) 
            pkt_info->cb(pkt_info->pkt);
    }

    txpkt_cnt_ = 0;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Invoke all the queued completion handlers
//------------------------------------------------------------------------------
void
ctx_t::invoke_completion_handlers(bool fail)
{
    HAL_TRACE_DEBUG("fte: invoking completion handlers.");
    for (int i = 0; i < num_features_; i++) {
        if (feature_state_[i].completion_handler != nullptr) {
            HAL_TRACE_DEBUG("fte: invoking completion handler {}",
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
    return (flow[stage()]->key().dir == FLOW_DIR_FROM_UPLINK) ?
                                stage() == 0 : stage() != 0;
}

bool
ctx_t::vlan_valid()
{
    return cpu_rxhdr_->flags&CPU_FLAGS_VLAN_VALID;
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
    os << "{mirror_en=" << val.mirror_en;
    os << " ,ing_mirror_session=" << val.ing_mirror_session;
    os << " ,egr_mirror_session=" << val.egr_mirror_session;
    os << " ,proxy_ing_mirror_session=" << val.proxy_ing_mirror_session;
    os << " ,proxy_egr_mirror_session=" << val.proxy_egr_mirror_session;
    return os << "}";
}

} // namespace fte
