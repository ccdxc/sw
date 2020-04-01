#include "nic/fte/fte.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/fte/fte_flow.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "lib/list/list.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/cpupkt_headers.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"

#define LOG_SIZE(ev) ev.ByteSizeLong()
#define TYPE_TO_LG_SZ(type, sz_) {                                    \
    if (type == IPC_LOG_TYPE_FW) {                                    \
       fwlog::FWEvent ev; sz_ = (LOG_SIZE(ev) + IPC_HDR_SIZE);        \
    } else {                                                          \
       sz_ = IPC_BUF_SIZE;                                            \
    }                                                                 \
}

//-------------------------------------------------------
// Thread local protospec to update FTE Flow logging info
// ------------------------------------------------------
thread_local fwlog::FWEvent t_fwlg;

namespace hal {
extern hal::session_stats_t  *g_session_stats;
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
    hal::pd::pd_func_args_t pd_func_args = {0};
    hal::hal_obj_id_t obj_id;
    void *obj;
    hal_ret_t ret;

    // if it is a grpc req extract key from spec
    if (sess_spec_) {
        return extract_flow_key_from_spec(sess_spec_->vrf_key_handle().vrf_id(),
                                          &key_,
                                          sess_spec_->initiator_flow().flow_key());
    }

    SDK_ASSERT_RETURN(cpu_rxhdr_ != NULL && pkt_ != NULL, HAL_RET_INVALID_ARG);

    key_.dir = cpu_rxhdr_->lkp_dir;
    key_.lkpvrf = cpu_rxhdr_->lkp_vrf;
    HAL_TRACE_DEBUG("Lkp vrf: {}", key_.lkpvrf);
    args.flow_lkupid = cpu_rxhdr_->lkp_vrf;
    args.obj_id = &obj_id;
    args.pi_obj = &obj;
    pd_func_args.pd_get_object_from_flow_lkupid = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: Invalid obj id: {}, ret: {}", obj_id, ret);
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    if (obj_id == hal::HAL_OBJ_ID_L2SEG) {
        sl2seg_ = (hal::l2seg_t *)obj;
    } else if (obj_id == hal::HAL_OBJ_ID_VRF)  {
        SDK_ASSERT_RETURN(cpu_rxhdr_->lkp_type != hal::FLOW_KEY_LOOKUP_TYPE_MAC, HAL_RET_ERR);
        use_vrf_ = svrf_ = (hal::vrf_t *)obj;
    } else {
        HAL_TRACE_ERR("fte: Invalid obj id: {}", obj_id);
        return HAL_RET_ERR;
    }

    // extract src/dst/proto
    switch (cpu_rxhdr_->lkp_type) {
    case hal::FLOW_KEY_LOOKUP_TYPE_MAC:
        ethhdr = (ether_header_t *)(pkt_ + cpu_rxhdr_->l2_offset);
        key_.flow_type = hal::FLOW_TYPE_L2;
        key_.l2seg_id = sl2seg_->seg_id;
        memcpy(key_.smac, ethhdr->smac, sizeof(key_.smac));
        memcpy(key_.dmac, ethhdr->dmac, sizeof(key_.dmac));
        key_.ether_type = (cpu_rxhdr_->flags&CPU_FLAGS_VLAN_VALID) ?
            ntohs(((vlan_header_t*)ethhdr)->etype): ntohs(ethhdr->etype);
        break;

    case hal::FLOW_KEY_LOOKUP_TYPE_IPV4:
        iphdr = (ipv4_header_t*)(pkt_ + cpu_rxhdr_->l3_offset);
        key_.flow_type = hal::FLOW_TYPE_V4;
        key_.sip.v4_addr = ntohl(iphdr->saddr);
        key_.dip.v4_addr = ntohl(iphdr->daddr);
        key_.proto = (types::IPProtocol) iphdr->protocol;
        payload_len_ = (ntohs(iphdr->tot_len) + cpu_rxhdr_->l3_offset)- cpu_rxhdr_->payload_offset;
        break;

    case hal::FLOW_KEY_LOOKUP_TYPE_IPV6:
        iphdr6 = (ipv6_header_t *)(pkt_ + cpu_rxhdr_->l3_offset);
        key_.flow_type = hal::FLOW_TYPE_V6;
        memcpy(key_.sip.v6_addr.addr8, iphdr6->saddr, sizeof(key_.sip.v6_addr.addr8));
        memcpy(key_.dip.v6_addr.addr8, iphdr6->daddr, sizeof(key_.dip.v6_addr.addr8));
        key_.proto = (types::IPProtocol) iphdr6->nexthdr;
        payload_len_ = (ntohs(iphdr6->payload_len) + cpu_rxhdr_->l3_offset) - cpu_rxhdr_->payload_offset;
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
            key_.dport = ntohs(udphdr->dport);
            //force sport to 0 if dport is UDP_PORT_ROCE_V2
            if (key_.dport == UDP_PORT_ROCE_V2) {
                key_.sport = 0;
            } else {
                key_.sport = ntohs(udphdr->sport);
            }
            break;

        case IPPROTO_ICMP:
        case IPPROTO_ICMPV6:
            icmphdr = (icmp_header_t*)(pkt_ + cpu_rxhdr_->l4_offset);
            key_.icmp_type =  icmphdr->type;
            key_.icmp_code = icmphdr->code;
            if (icmphdr->type == ICMP_TYPE_ECHO_REQUEST ||
                icmphdr->code == ICMP_TYPE_ECHO_RESPONSE)
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
ctx_t::lookup_flow_objs (void)
{
    ether_header_t *ethhdr = NULL;
    hal::pd::pd_l2seg_get_flow_lkupid_args_t l2seg_args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    hal::ep_t *sep = NULL, *dep = NULL;

    rkey_.lkpvrf = key_.lkpvrf;
    dl2seg_ = sl2seg_;
    if (sl2seg_ != NULL && cpu_rxhdr_ != NULL) {
        // Try to find sep by looking at L2.
        ethhdr = (ether_header_t *)(pkt_ + cpu_rxhdr_->l2_offset);
        sep_ = hal::find_ep_by_l2_key(sl2seg_->seg_id, ethhdr->smac);
        ethhdr = (ether_header_t *)(pkt_ + cpu_rxhdr_->l2_offset);
        dep_ = hal::find_ep_by_l2_key(sl2seg_->seg_id, ethhdr->dmac);
    } else if (existing_session()) {
            sep_ = hal::find_ep_by_l2_key(session_->iflow->config.l2_info.l2seg_id,
                                      session_->iflow->config.l2_info.smac);
            dep_ = hal::find_ep_by_l2_key(session_->iflow->config.l2_info.l2seg_id,
                                      session_->iflow->config.l2_info.dmac);
    }

    if (unlikely(protobuf_request() && sep_ == NULL)) {
        hal::ep_get_from_flow_key(&key_, &sep, &dep);
        if (sep) {
            sep_ = sep;
        }

        if (dep) {
            dep_ = dep;
            dl2seg_ = hal::l2seg_lookup_by_handle(dep_->l2seg_handle);
            SDK_ASSERT_RETURN(dl2seg_, HAL_RET_L2SEG_NOT_FOUND);
        }

        HAL_TRACE_DEBUG("VRF:{} l2seg_id:{}, sl2seg:{:p} sep:{:p} dep:{:p}", key_.lkpvrf,
                      (sl2seg_)?sl2seg_->seg_id:0, (void *)sl2seg_, (void *)sep_, (void *)dep_);

        if (sep_) {
            sep_handle_ = sep_->hal_handle;
            sl2seg_ = hal::l2seg_lookup_by_handle(sep_->l2seg_handle);
            SDK_ASSERT_RETURN(sl2seg_, HAL_RET_L2SEG_NOT_FOUND);

            l2seg_args.l2seg = sl2seg_;
            pd_func_args.pd_l2seg_get_flow_lkupid = &l2seg_args;
            hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, &pd_func_args);
            rkey_.lkpvrf = key_.lkpvrf = l2seg_args.hwid; 
        }

        dep_handle_ = (dep_)?dep_->hal_handle:0;
    }

    sep_handle_ = (sep_)?sep_->hal_handle:0;
    dep_handle_ = (dep_)?dep_->hal_handle:0;

    if (unlikely(sl2seg_ != NULL)) {
        svrf_ = dvrf_ = hal::vrf_lookup_by_handle(sl2seg_->vrf_handle);
        key_.svrf_id = key_.dvrf_id = svrf_->vrf_id;
    }

    return HAL_RET_OK;
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
        swap_flow_objs();
        is_flow_swapped_ = true;
    }
    key_ = hflow->config.key;

    // Init feature sepcific session state
    sdk::lib::dllist_ctxt_t   *entry = NULL;
    dllist_for_each(entry, &sess->feature_list_head) {
        feature_session_state_t *state =
            dllist_entry(entry, feature_session_state_t, session_feature_lentry);
        uint16_t id = feature_id(state->feature_name);
        if (id <= num_features_) {
            feature_state_[id].session_state = state;
            HAL_TRACE_VERBOSE("fte: feature={} restored session state {:p}",
                            state->feature_name, (void*)state);
        }
    }

    if(role_ == hal::FLOW_ROLE_INITIATOR)
        valid_iflow_ = true;

    // TODO(goli) handle post svc flows
    if (hflow->config.role == hal::FLOW_ROLE_INITIATOR) {
        iflow_[stage]->from_config(hflow->config, hflow->pgm_attrs, session_);
            if (sess->rflow) {
                rflow_[stage]->from_config(sess->rflow->config,
                                           sess->rflow->pgm_attrs, session_);
                valid_rflow_ = true;
                if (sess->rflow->assoc_flow) {
                    rflow_[++stage]->from_config(
                                       sess->rflow->assoc_flow->config,
                                       sess->rflow->assoc_flow->pgm_attrs, session_);
                }
            }
            if (hflow->assoc_flow) {
                iflow_[++stage]->from_config(hflow->assoc_flow->config,
                                           hflow->assoc_flow->pgm_attrs, session_);
            }
    } else {
        rflow_[stage]->from_config(hflow->config, hflow->pgm_attrs, session_);
        if (hflow->assoc_flow) {
            rflow_[++stage]->from_config(hflow->assoc_flow->config,
                                         hflow->assoc_flow->pgm_attrs, session_);
        }
        iflow_[stage]->from_config(hflow->reverse_flow->config,
                                   hflow->reverse_flow->pgm_attrs, session_);
        if (sess->iflow->assoc_flow) {
            iflow_[++stage]->from_config(
                                       sess->iflow->assoc_flow->config,
                                       sess->iflow->assoc_flow->pgm_attrs, session_);
        }

        valid_rflow_ = true;
    }

    if (valid_rflow_) {
        rkey_ = rflow_[0]->key();
    }

    // Set drop from the existing session
    if (sess->iflow->pgm_attrs.drop)
        set_drop();
}

//------------------------------------------------------------------------------
// Lookup existing session for the flow
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::lookup_session()
{
    session_ = hal::session_lookup(key_, &role_);

    if ((sync_session_request()) && (!session_)) {
        // In case of Session Sync for vMotions, direction bit will be reverse in the old
        // host. Flip the direction bit and lookup the session 
        key_.dir = !key_.dir;
        session_ = hal::session_lookup(key_, &role_);
    }

    if (!session_) {
        HAL_TRACE_DEBUG("fte: session not found role:{}", role_);
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
    hal_ret_t ret;
    hal::flow_key_t l2_info = {0};
    ether_header_t *ethhdr = NULL;

    bzero(&l2_info, sizeof(hal::flow_key_t));

    if (sl2seg_) {
        l2_info.l2seg_id = sl2seg_->seg_id;
    } else if (dl2seg_) {
        l2_info.l2seg_id = dl2seg_->seg_id;
    }
    if (cpu_rxhdr_ != NULL) {
        ethhdr = (ether_header_t *)(pkt_ + cpu_rxhdr_->l2_offset);
        memcpy(l2_info.smac, ethhdr->smac, sizeof(l2_info.smac));
        memcpy(l2_info.dmac, ethhdr->dmac, sizeof(l2_info.dmac));
    }

    HAL_TRACE_DEBUG("Key: {}", key_);
    for (int i = 0; i < MAX_STAGES; i++) {
        iflow_[i]->set_key(key_);
        iflow_[i]->set_l2_info(l2_info);
    }

    cleanup_hal_ = false;

    valid_iflow_ = true;

    // read rkey from spec
    if (protobuf_request()) {
        if (sess_spec_->has_responder_flow()) {
            ret = extract_flow_key_from_spec(sess_spec_->vrf_key_handle().vrf_id(),
                                             &rkey_,
                                             sess_spec_->responder_flow().flow_key());

            if (ret != HAL_RET_OK) {
                return ret;
            }
            valid_rflow_ = true;
        }
    } else {
        if (key_.flow_type == hal::FLOW_TYPE_L2 ||
            key_.proto == IP_PROTO_TCP || key_.proto == IP_PROTO_UDP ||
            key_.proto == IP_PROTO_ICMP || key_.proto == IP_PROTO_ICMPV6) {
            swap_flow_key(key_, &rkey_);
            valid_rflow_ = true;
        } else {
            valid_rflow_ = false;
        }
    }

    if (valid_rflow_) {
        rkey_.dir = (dep_ && (dep_->ep_flags & EP_FLAGS_LOCAL)) ?
            hal::FLOW_DIR_FROM_DMA : hal::FLOW_DIR_FROM_UPLINK;
        if (cpu_rxhdr_ != NULL) {
            ethhdr = (ether_header_t *)(pkt_ + cpu_rxhdr_->l2_offset);
            memcpy(l2_info.smac, ethhdr->dmac, sizeof(l2_info.smac));
            memcpy(l2_info.dmac, ethhdr->smac, sizeof(l2_info.dmac));
        }
        for (int i = 0; i < MAX_STAGES; i++) {
            rflow_[i]->set_key(rkey_);
            rflow_[i]->set_l2_info(l2_info);
        }
    }

    role_ = hal::FLOW_ROLE_INITIATOR;

    return HAL_RET_OK;
}

// fw_log reports a firewall event to the agent
static inline void fw_log(ipc_logger *logger, fwlog::FWEvent ev)
{
    uint8_t *buf = logger->get_buffer(LOG_SIZE(ev));
    if (buf == NULL) {
        return;
    }

    if (!ev.SerializeToArray(buf, LOG_SIZE(ev))) {
        HAL_TRACE_ERR("Unable to serialize");
        return;
    }

    int size = ev.ByteSizeLong();
    logger->write_buffer(buf, size);
}

//------------------------------------------------------------------------------
// Add FTE Flow logging information in logging infra
//------------------------------------------------------------------------------
inline void
ctx_t::add_flow_logging (hal::flow_key_t key, hal_handle_t sess_hdl,
                  fte_flow_log_info_t *log) {
    timespec_t      ctime;
    int64_t         ctime_ns;

    t_fwlg.Clear();

    t_fwlg.set_source_vrf(key.svrf_id);
    t_fwlg.set_dest_vrf(key.dvrf_id);
    t_fwlg.set_sipv4(key.sip.v4_addr);
    t_fwlg.set_dipv4(key.dip.v4_addr);

    t_fwlg.set_ipprot(key.proto);
    if (key.proto == IP_PROTO_TCP || key.proto == IP_PROTO_UDP) {
        t_fwlg.set_sport(key.sport);
        t_fwlg.set_dport(key.dport);
    } else if (key.proto == IP_PROTO_ICMP) {
        t_fwlg.set_icmptype(key.icmp_type);
        t_fwlg.set_icmpcode(key.icmp_code);
        t_fwlg.set_icmpid(key.icmp_id);
    }

    t_fwlg.set_direction((key.dir == hal::FLOW_DIR_FROM_UPLINK) ?
                         types::FLOW_DIRECTION_FROM_UPLINK :\
                         types::FLOW_DIRECTION_FROM_HOST);
    clock_gettime(CLOCK_REALTIME, &ctime);
    if (pipeline_event() == FTE_SESSION_DELETE) {
        t_fwlg.set_flowaction(fwlog::FLOW_LOG_EVENT_TYPE_DELETE);
        t_fwlg.set_iflow_packets(session_state.iflow_state.packets);
        t_fwlg.set_rflow_packets(session_state.rflow_state.packets);
    }
    sdk::timestamp_to_nsecs(&ctime, &ctime_ns);
    t_fwlg.set_timestamp(ctime_ns);
    t_fwlg.set_session_id(sess_hdl);
    t_fwlg.set_alg(log->alg);
    t_fwlg.set_fwaction(log->sfw_action);
    t_fwlg.set_parent_session_id(log->parent_session_id);
    t_fwlg.set_rule_id(log->rule_id);

    if (logger_ != NULL) {
        fw_log(logger_, t_fwlg);
    }
}

//------------------------------------------------------------------------------
// Update qos_class_id for flows if applicable
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_flow_qos_class_id(flow_t *flow,
                                hal::flow_pgm_attrs_t *flow_attrs)
{
    // TODO specific for ipsec smart-switch mode
    if (flow->fwding().update_qos == true) {
        // Packet entering P4 from uplink:
        //      initiator: use DEFAULT UPLINK_IQ
        //      responder: use DEFAULT QUEUE
        // Packet entering P4 from DMA:
        //      initiator: use DEFAULT QUEUE
        //      responder: use DEFAULT UPLINK_IQ
        if (direction() == hal::FLOW_DIR_FROM_UPLINK) {
            if (flow_attrs->role == hal::FLOW_ROLE_INITIATOR) {
                flow_attrs->qos_class_id = CAPRI_TM_P4_UPLINK_IQ_OFFSET;
            } else {
                flow_attrs->qos_class_id = QOS_QUEUE_DEFAULT;
            }
        } else {
            if (flow_attrs->role == hal::FLOW_ROLE_INITIATOR) {
                flow_attrs->qos_class_id = QOS_QUEUE_DEFAULT;
            } else {
                flow_attrs->qos_class_id = CAPRI_TM_P4_UPLINK_IQ_OFFSET;
            }
        }
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Create/update session and flow table entries in hardware
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_flow_table()
{
    hal_ret_t       ret = HAL_RET_OK;
    hal_handle_t    session_handle = (session())?session()->hal_handle:HAL_HANDLE_INVALID;
    hal::session_t *session = NULL;
    hal::pd::pd_tunnelif_get_rw_idx_args_t t_args;
    hal::pd::pd_func_args_t          pd_func_args = {0};
    std::string      update_type = "none";
    hal::app_redir::app_redir_ctx_t* app_ctx = hal::app_redir::app_redir_ctx(*this, false);
    bool             session_exists = existing_session();

    session_args.session = &session_cfg;
    session_cfg.idle_timeout = HAL_MAX_INACTIVTY_TIMEOUT;
    if (unlikely(protobuf_request())) {
        session_cfg.session_id = sess_spec_->session_id();
        session_state.tcp_ts_option = sess_spec_->tcp_ts_option();
        session_state.tcp_sack_perm_option = sess_spec_->tcp_sack_perm_option();
        if (sync_session_request()) {
            session_args.session_state  = &session_state;
            session_cfg.syncing_session = session_exists ? FALSE : TRUE;
        }
    }

    if (ignore_session_create()) {
        HAL_TRACE_DEBUG("Sesssion create ignored");
        goto end;
    }

    if (!(flow_miss() ||
        (app_redir_pipeline() && app_ctx && app_ctx->appid_completed()))) {
        return HAL_RET_OK;
    }

    for (uint8_t stage = 0; valid_iflow_ && !hal_cleanup() && stage <= istage_; stage++) {
        flow_t *iflow = iflow_[stage];
        hal::flow_cfg_t& iflow_cfg = iflow_cfg_list[stage];
        hal::flow_pgm_attrs_t& iflow_attrs = iflow_attrs_list[stage];

        // For existing sessions initialize with the configs the session
        // came with and update anything else
        if (existing_session() && stage == 0 && session_->iflow) {
            iflow_cfg = session_->iflow->config;
            iflow_attrs = session_->iflow->pgm_attrs;
        }

        iflow->to_config(iflow_cfg, iflow_attrs);
        iflow_cfg.role = iflow_attrs.role = hal::FLOW_ROLE_INITIATOR;

        iflow_attrs.use_vrf = (use_vrf_)?1:0;

        // Set the lkp_inst for all stages except the first stage
        if (stage != 0) {
            iflow_attrs.lkp_inst = 1;
        }

        if (unlikely((hal::is_platform_type_sim()))) {
            // TODO(goli) fix tnnl_rw_idx lookup
            if (iflow_attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_NOP_ID) {
                iflow_attrs.tnnl_rw_idx = 0;
            } else if (iflow_attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_ENCAP_VLAN_ID) {
                iflow_attrs.tnnl_rw_idx = 1;
            } else if (dif_ && dif_->if_type == intf::IF_TYPE_TUNNEL) {
                t_args.hal_if = dif_;
                pd_func_args.pd_tunnelif_get_rw_idx = &t_args;
                ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TNNL_IF_GET_RW_IDX,
                                       &pd_func_args);
                iflow_attrs.tnnl_rw_idx = t_args.tnnl_rw_idx;
            }
        }

        session_args.iflow[stage] = &iflow_cfg;
        session_args.iflow_attrs[stage] = &iflow_attrs;

        if (iflow->valid_flow_state()) {
            session_cfg.conn_track_en = 1;
            session_args.session_state = &session_state;
            session_state.iflow_state = iflow->flow_state();
        }

        if (iflow->valid_aging_info()) {
            session_cfg.idle_timeout = iflow->aging_info().idle_timeout;
        }

        if (is_proxy_enabled()) {
            iflow_attrs.is_proxy_en = 1;
        }
        if (iflow->valid_sfw_info()) {
            session_cfg.skip_sfw_reval = (iflow->sfw_info().skip_sfw_reval&0x1);
            session_cfg.sfw_rule_id = iflow->sfw_info().sfw_rule_id;
            session_cfg.sfw_action = (iflow->sfw_info().sfw_action&0x7);
        }
        if (existing_session() && iflow->valid_export_info()) {
            session_args.update_iflow = true;
        } else {
            session_args.update_iflow = false;
        }

        if (unlikely(hal::utils::hal_trace_level() >= ::utils::trace_debug)) { 
            HAL_TRACE_DEBUG("fte::update_flow_table: iflow.{} key={} lkp_inst={} "
                        "lkp_vrf={} action={} smac_rw={} dmac-rw={} "
                        "ttl_dec={} mcast={} lport={} qid_en={} qtype={} qid={} rw_act={} "
                        "rw_idx={} tnnl_rw_act={} tnnl_rw_idx={} tnnl_vnid={} nat_sip={} "
                        "nat_dip={} nat_sport={} nat_dport={} nat_type={} is_ing_proxy_mirror={} "
                        "is_eg_proxy_mirror={} ing_mirror_session={} eg_mirror_session={} "
                        "slif_en={} slif={} qos_class_en={} qos_class_id={} "
                        "is_proxy_en={} is_proxy_mcast={} export_en={} export_id1={} "
                        "export_id2={} export_id3={} export_id4={} conn_track_en={} "
                        "session_idle_timeout={} smac={} dmac={} l2seg_id={}, skip_sfw_reval={} "
                        "sfw_rule_id={}, sfw_action={}, sync={}",
                        stage, iflow_cfg.key, iflow_attrs.lkp_inst, key_.lkpvrf,
                        iflow_cfg.action, iflow_attrs.mac_sa_rewrite,
                        iflow_attrs.mac_da_rewrite, iflow_attrs.ttl_dec, iflow_attrs.mcast_en,
                        iflow_attrs.lport, iflow_attrs.qid_en, iflow_attrs.qtype, iflow_attrs.qid,
                        iflow_attrs.rw_act, iflow_attrs.rw_idx, iflow_attrs.tnnl_rw_act,
                        iflow_attrs.tnnl_rw_idx, iflow_attrs.tnnl_vnid, iflow_cfg.nat_sip,
                        iflow_cfg.nat_dip, iflow_cfg.nat_sport, iflow_cfg.nat_dport,
                        iflow_cfg.nat_type, iflow_cfg.is_ing_proxy_mirror, iflow_cfg.is_eg_proxy_mirror,
                        iflow_cfg.ing_mirror_session, iflow_cfg.eg_mirror_session,
                        iflow_attrs.expected_src_lif_en, iflow_attrs.expected_src_lif,
                        iflow_attrs.qos_class_en, iflow_attrs.qos_class_id, iflow_attrs.is_proxy_en,
                        iflow_attrs.is_proxy_mcast, iflow_attrs.export_en, iflow_attrs.export_id1,
                        iflow_attrs.export_id2, iflow_attrs.export_id3, iflow_attrs.export_id4,
                        session_cfg.conn_track_en, session_cfg.idle_timeout,
                        ether_ntoa((struct ether_addr*)&iflow_cfg.l2_info.smac),
                        ether_ntoa((struct ether_addr*)&iflow_cfg.l2_info.dmac),
                        iflow_cfg.l2_info.l2seg_id, session_cfg.skip_sfw_reval, session_cfg.sfw_rule_id,
                        session_cfg.sfw_action, session_cfg.syncing_session);
        }
    }

    for (uint8_t stage = 0; valid_rflow_ && !hal_cleanup() && stage <= rstage_; stage++) {
        flow_t *rflow = rflow_[stage];
        hal::flow_cfg_t& rflow_cfg = rflow_cfg_list[stage];
        hal::flow_pgm_attrs_t& rflow_attrs = rflow_attrs_list[stage];

        // For existing sessions initialize with the configs the session
        // came with and update anything else
        if (existing_session() && stage == 0 && session_->rflow) {
            rflow_cfg = session_->rflow->config;
            rflow_attrs = session_->rflow->pgm_attrs;
        }

        rflow->to_config(rflow_cfg, rflow_attrs);
        rflow_cfg.role = rflow_attrs.role = hal::FLOW_ROLE_RESPONDER;

        rflow_attrs.use_vrf = (use_vrf_)?1:0;

        // Set the lkp_inst for all stages except the first stage
        if (stage != 0) {
            rflow_attrs.lkp_inst = 1;
        }

        if (unlikely(hal::is_platform_type_sim())) {
            // TODO(goli) fix tnnl w_idx lookup
            if (rflow_attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_NOP_ID) {
                rflow_attrs.tnnl_rw_idx = 0;
            } else if (rflow_attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_ENCAP_VLAN_ID) {
                rflow_attrs.tnnl_rw_idx = 1;
            } else if (sif_ && sif_->if_type == intf::IF_TYPE_TUNNEL) {
                t_args.hal_if = sif_;
                pd_func_args.pd_tunnelif_get_rw_idx = &t_args;
                ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TNNL_IF_GET_RW_IDX,
                                       &pd_func_args);
                rflow_attrs.tnnl_rw_idx = t_args.tnnl_rw_idx;
            }
        }

        session_args.rflow[stage] = &rflow_cfg;
        session_args.rflow_attrs[stage] = &rflow_attrs;

        if (rflow->valid_flow_state()) {
            session_state.rflow_state = rflow->flow_state();
        }

        if (existing_session() && rflow->valid_export_info()) {
            session_args.update_rflow = true;
        } else {
            session_args.update_rflow = false;
        }

        if (unlikely(dl2seg_ != sl2seg_ && dl2seg_ != NULL)) {
            hal::pd::pd_func_args_t  pd_func_args = {0};
            hal::pd::pd_l2seg_get_flow_lkupid_args_t args;

            args.l2seg = dl2seg_;    
            pd_func_args.pd_l2seg_get_flow_lkupid = &args;    
            hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, (hal::pd::pd_func_args_t*)&pd_func_args);    
            rflow_cfg.key.lkpvrf = rkey_.lkpvrf = args.hwid;    
        }

        if (unlikely(hal::utils::hal_trace_level() >= ::utils::trace_debug)) {
            HAL_TRACE_DEBUG("fte::update_flow_table: rflow.{} key={} lkp_inst={} "
                        "lkp_vrf={} action={} smac_rw={} dmac-rw={} "
                        "ttl_dec={} mcast={} lport={} qid_en={} qtype={} qid={} rw_act={} "
                        "rw_idx={} tnnl_rw_act={} tnnl_rw_idx={} tnnl_vnid={} nat_sip={} "
                        "nat_dip={} nat_sport={} nat_dport={} nat_type={} slif_en={} slif={} "
                        "ing_mirror_session={} eg_mirror_session={} "
                        "qos_class_en={} qos_class_id={} export_en={} export_id1={} "
                        "export_id2={} export_id3={} export_id4={} smac={} dmac={} l2_segid={}",
                        stage, rflow_cfg.key, rflow_attrs.lkp_inst,
                        rkey_.lkpvrf, rflow_cfg.action,
                        rflow_attrs.mac_sa_rewrite,
                        rflow_attrs.mac_da_rewrite, rflow_attrs.ttl_dec, rflow_attrs.mcast_en,
                        rflow_attrs.lport, rflow_attrs.qid_en, rflow_attrs.qtype, rflow_attrs.qid,
                        rflow_attrs.rw_act, rflow_attrs.rw_idx, rflow_attrs.tnnl_rw_act,
                        rflow_attrs.tnnl_rw_idx, rflow_attrs.tnnl_vnid, rflow_cfg.nat_sip,
                        rflow_cfg.nat_dip, rflow_cfg.nat_sport, rflow_cfg.nat_dport,
                        rflow_cfg.nat_type, rflow_attrs.expected_src_lif_en, rflow_attrs.expected_src_lif,
                        rflow_cfg.ing_mirror_session, rflow_cfg.eg_mirror_session,
                        rflow_attrs.qos_class_en, rflow_attrs.qos_class_id,
                        rflow_attrs.export_en, rflow_attrs.export_id1,
                        rflow_attrs.export_id2, rflow_attrs.export_id3, rflow_attrs.export_id4,
                        ether_ntoa((struct ether_addr*)&rflow_cfg.l2_info.smac),
                        ether_ntoa((struct ether_addr*)&rflow_cfg.l2_info.dmac),
                        rflow_cfg.l2_info.l2seg_id);
        }
    }

    if (cpu_rxhdr_) {
        session_args.flow_hash   = cpu_rxhdr_->flow_hash;
    } else {
        session_args.flow_hash   = 0;
    }
    session_args.vrf_handle  = (svrf_)?svrf_->hal_handle:HAL_HANDLE_INVALID;
    session_args.sep  = sep_;
    session_args.dep = dep_;
    session_args.sl2seg_handle = sl2seg_?sl2seg_->hal_handle:HAL_HANDLE_INVALID;
    session_args.dl2seg_handle = dl2seg_?dl2seg_->hal_handle:HAL_HANDLE_INVALID;
    session_args.spec        = sess_spec_;
    session_args.status      = sess_status_;
    session_args.stats       = sess_stats_;
    session_args.rsp         = sess_resp_;
    session_args.valid_rflow = valid_rflow_;

    if (hal_cleanup() == true) {
        session_args.session_state = &session_state;
        // Cleanup session if hal_cleanup is set
        if (session_) {
            update_type = "delete";
            ret = hal::session_delete(&session_args, session_);
        }
    } else if (session_) {
        if (update_session_) {
            //HAL_TRACE_DEBUG("Updating Session");
            update_type = "update";
            // Update session if it already exists
            ret = hal::session_update(&session_args, session_);
        }
    } else {
        // Create a new HAL session
        update_type = "create";
        ret = hal::session_create(&session_args, &session_handle, &session);
        if (ret == HAL_RET_OK) {
            session_ = session;
            // Insert session specific feature state
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
        HAL_TRACE_ERR("Session {} failed, ret = {}", update_type, ret);
    }

    if (sess_resp()) {
        sess_resp_->mutable_status()->set_session_handle(session_handle);
    }

end:
    // Dont log when we hit an error
    if ((key_.flow_type == hal::FLOW_TYPE_V4) && 
        (ret == HAL_RET_OK) && ((session_exists == false) || (update_type == "delete"))) {
      
        // Compute CPS
        if (session_exists == false) {
            fte_inst_compute_cps();
        }

        if (!ipc_logging_disable()) {
 
            /* Add flow logging only for initiator flows */
            uint8_t istage = 0;
            add_flow_logging(key_, session_handle, &iflow_log_[istage]);
            if (++istage <= istage_)
                add_flow_logging(key_, session_handle, &iflow_log_[istage]);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// Update the dest ep/if/l2seg on dnat change
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_for_dnat(hal::flow_role_t role, const header_rewrite_info_t& header)
{
    ipvx_addr_t dip = {};
    hal::flow_key_t *key = (role == hal::FLOW_ROLE_INITIATOR) ? &key_ : &rkey_;

    if (!header.valid_flds.dvrf_id && !header.valid_flds.dip && !header.valid_flds.dmac) {
        return HAL_RET_OK;
    }

    dvrf_ =  hal::vrf_lookup_by_id(key->dvrf_id);

    if (dvrf_ == NULL) {
        HAL_TRACE_ERR("DNAT vrf not found vrf={}", key->dvrf_id);
        return HAL_RET_VRF_NOT_FOUND;
    }

    if (header.valid_flds.dvrf_id) {
        if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            key->dvrf_id = header.ipv4.dvrf_id;
        } else {
            key->dvrf_id = header.ipv6.dvrf_id;
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
            key->dip = dip;
        }

    } else if (dep_ == NULL && header.valid_flds.dmac && sl2seg_ != NULL) {
        /* L2 DSR - lookup EP using mac */
        dep_ = hal::find_ep_by_l2_key(sl2seg_->seg_id,
                                      header.ether.dmac.ether_addr_octet);
    }

    if (dep_) {
        dep_handle_ = dep_->hal_handle;
        dl2seg_ = hal::l2seg_lookup_by_handle(dep_->l2seg_handle);
        dif_ = hal::find_if_by_handle(dep_->if_handle);
        SDK_ASSERT(dif_ != NULL);
    }

    // If we are doing dnat on iflow, update the rflow's key
    if (role == hal::FLOW_ROLE_INITIATOR && valid_rflow_) {
        // update ctx rkey
        if (!this->protobuf_request()) {
            if (header.valid_flds.dip)
                rkey_.sip = key_.dip;
            if (header.valid_flds.dvrf_id)
                rkey_.svrf_id = key_.dvrf_id;
        }
        rkey_.dir = (dep_ && dep_->ep_flags & EP_FLAGS_LOCAL) ?
            hal::FLOW_DIR_FROM_DMA : hal::FLOW_DIR_FROM_UPLINK;

        // update rflow key
        for (int i = 0; i < MAX_STAGES; i++) {
            rflow_[i]->set_key(rkey_);
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
    hal::flow_key_t *key = (role == hal::FLOW_ROLE_INITIATOR) ? &key_ : &rkey_;

    if (this->protobuf_request()) {
        return HAL_RET_OK;
    }

    if (!header.valid_flds.svrf_id && !header.valid_flds.sip) {
        return HAL_RET_OK;
    }

    if (header.valid_flds.svrf_id) {
        if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            key->svrf_id = header.ipv4.svrf_id;
        } else {
            key->svrf_id = header.ipv6.svrf_id;
        }

#if TBD_WHAT_TODO
        svrf_ =  hal::vrf_lookup_by_id(key->svrf_id);

        if (svrf_ == NULL) {
            HAL_TRACE_ERR("SNAT vrf not found vrf={}", key->svrf_id);
            return HAL_RET_VRF_NOT_FOUND;
        }
#endif

    }

    if (header.valid_flds.sip) {
        if ((header.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            key->sip.v4_addr = header.ipv4.sip;
        } else {
            key->sip.v6_addr = header.ipv6.sip;
        }
    }

    // If we are doing snat on iflow, update the rflow's key
    if (role == hal::FLOW_ROLE_INITIATOR && valid_rflow_) {
        // update ctx rkey
        if (header.valid_flds.sip)
            rkey_.dip = key_.sip;
        if (header.valid_flds.svrf_id)
            rkey_.dvrf_id = key_.svrf_id;

        // update rflow key
        for (int i = 0; i < MAX_STAGES; i++) {
            rflow_[i]->set_key(rkey_);
        }
    }

    return  HAL_RET_OK;
}

void free_flow_miss_pkt(uint8_t * pkt)
{
    HAL_TRACE_DEBUG("free flow miss packet");
    hal::free_to_slab(hal::HAL_SLAB_CPU_PKT, (pkt-sizeof(cpu_rxhdr_t)));
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
    hal::pd::pd_func_args_t pd_func_args = {0};

    if (unlikely(hal::utils::hal_trace_level() >= ::utils::trace_verbose)) {
        HAL_TRACE_VERBOSE("fte: txpkt len={} pkt={}", pkt_len, hex_str(pkt, (pkt_len >=128)?128:pkt_len));
    }

    if (txpkt_cnt_ >= MAX_QUEUED_PKTS) {
        HAL_TRACE_ERR("fte: queued tx pkts exceeded {}", txpkt_cnt_);
        return HAL_RET_ERR;
    }

    pkt_info = &txpkts_[txpkt_cnt_++];
    if (cpu_header) {
        pkt_info->cpu_header = *cpu_header;
    } else {
        pkt_info->cpu_header.src_lif = cpu_rxhdr_->src_lif;
        // change lif/vlan for uplink pkts
        // - Vxlan: P4 terminates vxlan and doesn't send outer headers to FTE. 
        //          So FTE can inject the same packet.
        // - IPsec Decrypt: FTE injected packets are coming back to FTE because
        //                  of flow miss. Even for this packets we re-inject
        //                  with cpu vlan.
        if ((cpu_rxhdr_->lkp_dir == hal::FLOW_DIR_FROM_UPLINK) && 
            (use_vrf_ || is_proxy_enabled() || tunnel_terminated() ||
             pkt_info->cpu_header.src_lif == HAL_LIF_CPU)) {
            pkt_info->cpu_header.src_lif = HAL_LIF_CPU;
            if (use_vrf_) {
                hal::pd::pd_vrf_get_fromcpu_vlanid_args_t args;
                args.vrf = use_vrf_;
                args.vid = &pkt_info->cpu_header.hw_vlan_id;

                pd_func_args.pd_vrf_get_fromcpu_vlanid = &args;
                if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FRCPU_VLANID,
                                         &pd_func_args) == HAL_RET_OK) {
                    pkt_info->cpu_header.flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
                }
            } else {
                hal::pd::pd_l2seg_get_fromcpu_vlanid_args_t args;
                args.l2seg = sl2seg_;
                args.vid = &pkt_info->cpu_header.hw_vlan_id;

                pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
                if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                                         &pd_func_args) == HAL_RET_OK) {
                    pkt_info->cpu_header.flags |= CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
                }
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

    if (unlikely(hal::utils::hal_trace_level() >= ::utils::trace_verbose)) {
        HAL_TRACE_VERBOSE("fte: feature={} queued txpkt lkp_inst={} src_lif={} vlan={} "
                      "dest_lifq={} ring={} wring={} pkt={:p} len={}",
                      feature_name_,
                      pkt_info->p4plus_header.lkp_inst,
                      pkt_info->cpu_header.src_lif,
                      pkt_info->cpu_header.hw_vlan_id,
                      pkt_info->lifq, pkt_info->ring_number, pkt_info->wring_type,
                      pkt_info->pkt, pkt_info->pkt_len);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Sends all the queued packets
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::send_queued_pkts(hal::pd::cpupkt_ctxt_t* arm_ctx)
{
    hal_ret_t ret;

    if (pkt_ != NULL && enq_or_free_rx_pkt_ == false) {

        if (flow_miss() && !drop() &&

	    // queue rx pkt if tx_queue is empty, it is a flow miss and firwall action is not drop
	    // Note: app_redir_pkt_tx_ownership() below is an inline
            !hal::app_redir::app_redir_pkt_tx_ownership(*this)) {
	    queue_txpkt(pkt_, pkt_len_, NULL, NULL, HAL_LIF_CPU,
                        CPU_ASQ_QTYPE, CPU_ASQ_QID, CPU_SCHED_RING_ASQ,
                        types::WRING_TYPE_ASQ, copied_pkt_ ? free_flow_miss_pkt : NULL);
	} else {

             /*
              * If the 'copied_pkt' is not set, then this is not a packet buffer
              * that we've allocated from slab, so no need to free it.
              */
	     if (copied_pkt_) {
                 free_flow_miss_pkt(pkt_);
	     } else {

	         /*
		  * Hand-over the packet to cpupkt library, to free any resources allocated
		  * from the data-path for this packet.
		  */
                 hal::pd::pd_cpupkt_free_pkt_resources_args_t args;
                 hal::pd::pd_func_args_t pd_func_args = {0};
                 args.ctxt = arm_ctx;
                 args.pkt = (pkt_ - sizeof(cpu_rxhdr_t));
                 pd_func_args.pd_cpupkt_free_pkt_resources = &args;

                 hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_FREE_PKT_RES, &pd_func_args);
	     }
        }
        enq_or_free_rx_pkt_ = true;
    }

    for (int i = 0; i < txpkt_cnt_; i++) {
        txpkt_info_t *pkt_info = &txpkts_[i];
        if ( istage_ > 0 ){
            pkt_info->p4plus_header.lkp_inst = 1;
        }

        pkt_info->p4plus_header.p4plus_app_id = P4PLUS_APPTYPE_CPU;

        hal::pd::pd_cpupkt_send_args_t args;
        hal::pd::pd_func_args_t pd_func_args = {0};
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
        pd_func_args.pd_cpupkt_send = &args;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to transmit pkt, ret={}", ret);
        }
        incr_inst_fte_tx_stats(pkt_info->pkt_len);
        // Issue a callback to free the packet
        if (pkt_info->cb) {
            pkt_info->cb(pkt_info->pkt);
        }
    }
    txpkt_cnt_ = 0;

    return HAL_RET_OK;
}

/*
 * Send batched packets in the Tx-queue to the PMD.
 */
hal_ret_t
ctx_t::send_queued_pkts_new (hal::pd::cpupkt_ctxt_t* arm_ctx)
{
    hal_ret_t ret;

    hal::pd::pd_cpupkt_send_new_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};

    for (int i = 0; i < txpkt_cnt_; i++) {
        txpkt_info_t *pkt_info = &txpkts_[i];
        HAL_TRACE_DEBUG("fte:: txpkt slif={} pkt={:p} len={}",
                        pkt_info->cpu_header.src_lif,
                        pkt_info->pkt, pkt_info->pkt_len);

        if ( istage_ > 0 ){
            pkt_info->p4plus_header.lkp_inst = 1;
        }

        pkt_info->p4plus_header.p4plus_app_id = P4PLUS_APPTYPE_CPU;

        args.pkt_batch.pkts[i].ctxt = arm_ctx;
        args.pkt_batch.pkts[i].type = pkt_info->wring_type;
        args.pkt_batch.pkts[i].queue_id = pkt_info->wring_type == types::WRING_TYPE_ASQ ? fte_id() : pkt_info->lifq.qid;
        args.pkt_batch.pkts[i].cpu_header = &pkt_info->cpu_header;
        args.pkt_batch.pkts[i].p4_header = &pkt_info->p4plus_header;
        args.pkt_batch.pkts[i].data = pkt_info->pkt;
        args.pkt_batch.pkts[i].data_len = pkt_info->pkt_len;
        args.pkt_batch.pkts[i].dest_lif = pkt_info->lifq.lif;
        args.pkt_batch.pkts[i].qtype = pkt_info->lifq.qtype;
        args.pkt_batch.pkts[i].qid = pkt_info->wring_type == types::WRING_TYPE_ASQ ? fte_id() : pkt_info->lifq.qid;
        args.pkt_batch.pkts[i].ring_number = pkt_info->ring_number;

    }
    args.pkt_batch.pktcount = txpkt_cnt_;

    pd_func_args.pd_cpupkt_send_new = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND_NEW, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte: failed to transmit pkt, ret={}", ret);
    }

    incr_inst_fte_tx_stats_batch(txpkt_cnt_);

    for (int i = 0; i < txpkt_cnt_; i++) {
        txpkt_info_t *pkt_info = &txpkts_[i];

	// Issue a callback to free the packet
	if (pkt_info->cb) {
	    HAL_TRACE_DEBUG(" packet buffer/cpu_rx header {:#x} {:#x}", (long)cpu_rxhdr_, (long)pkt_);
	    pkt_info->cb(pkt_info->pkt);
	}
    }

    txpkt_cnt_ = 0;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Apply session limits for various flow types
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::apply_session_limit(void)
{
    hal_ret_t                    ret = HAL_RET_OK;
    uint8_t                      id = fte_id();
    int8_t                       tcp_flags;
    const fte::cpu_rxhdr_t      *cpurxhdr = cpu_rxhdr();

    // check for flood protection limits
    // fetch the security profile, if any
    nwsec_prof_ =
        hal::find_nwsec_profile_by_handle(
                       hal::g_hal_state->customer_default_security_profile_hdl());
    if (nwsec_prof_ == NULL) {
        goto end;
    }

    HAL_TRACE_DEBUG("Security profile handle: {}", 
                     hal::g_hal_state->customer_default_security_profile_hdl());
    // check for flood protection limits
    switch (key_.flow_type) {
    case hal::FLOW_TYPE_V4: //intentional fall-through
    case hal::FLOW_TYPE_V6:
        if (key_.proto == types::IPPROTO_TCP) {
            //check if it is tcp half open session and then apply limit
            tcp_flags = cpurxhdr->tcp_flags;
            if (!(tcp_flags & TCP_FLAG_SYN)) {
                // if not a SYN packet then skip half-open session limit validation
                goto end;
            }
            if ((nwsec_prof_->tcp_half_open_session_limit) &&
                    (hal::g_session_stats[id].tcp_half_open_sessions >=
                     nwsec_prof_->tcp_half_open_session_limit)) {
                ret = HAL_RET_FLOW_LIMT_REACHED;
                hal::g_session_stats[id].tcp_session_drop_count++;
            }
        } else if (key_.proto == types::IPPROTO_UDP) {
            if ((nwsec_prof_->udp_active_session_limit) &&
                    (hal::g_session_stats[id].udp_sessions >=
                     nwsec_prof_->udp_active_session_limit)) {
                ret = HAL_RET_FLOW_LIMT_REACHED;
                hal::g_session_stats[id].udp_session_drop_count++;
            }
        } else if (key_.proto == types::IPPROTO_ICMP) {
            if ((nwsec_prof_->icmp_active_session_limit) &&
                    (hal::g_session_stats[id].icmp_sessions >=
                     nwsec_prof_->icmp_active_session_limit)) {
                ret = HAL_RET_FLOW_LIMT_REACHED;
                hal::g_session_stats[id].icmp_session_drop_count++;
            }
        }
        break;
    case hal::FLOW_TYPE_L2: //intentional fall-through
    default:
        if ((nwsec_prof_->other_active_session_limit) &&
                (hal::g_session_stats[id].other_active_sessions >=
                 nwsec_prof_->other_active_session_limit)) {
            ret = HAL_RET_FLOW_LIMT_REACHED;
            hal::g_session_stats[id].other_session_drop_count++;
        }
        break;
    }
end:
    if (ret != HAL_RET_OK) {
        hal::g_session_stats[id].drop_sessions++;
    }
    return ret;
}

}
