//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/gen/proto/hal/telemetry.pb.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/src/telemetry/telemetry.hpp"

using hal::pd::pd_mirror_session_create_args_t;
using hal::pd::pd_mirror_session_delete_args_t;
using hal::pd::pd_mirror_session_get_args_t;
using hal::mirror_session_t;
using hal::pd::pd_flow_monitor_rule_create_args_t;
using hal::pd::pd_flow_monitor_rule_delete_args_t;
using hal::pd::pd_flow_monitor_rule_get_args_t;
using hal::flow_monitor_rule_t;
using hal::pd::pd_drop_monitor_rule_create_args_t;
using hal::pd::pd_drop_monitor_rule_delete_args_t;
using hal::pd::pd_drop_monitor_rule_get_args_t;
using hal::drop_monitor_rule_t;

namespace hal {

// Global structs
const acl_ctx_t *flowmon_acl_ctx;
acl::acl_config_t flowmon_rule_config_glbl = { };
flow_monitor_rule_t *flow_mon_rules[MAX_FLOW_MONITOR_RULES];

hal_ret_t
mirror_session_update (MirrorSessionSpec &spec, MirrorSessionResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
mirror_session_create (MirrorSessionSpec &spec, MirrorSessionResponse *rsp)
{
    pd_mirror_session_create_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    mirror_session_t session;
    kh::InterfaceKeyHandle ifid;
    hal_ret_t ret;
    if_t *id;

    HAL_TRACE_DEBUG("Mirror session ID {}/{}",
                    spec.key_or_handle().mirrorsession_id(), spec.snaplen());
    // Eventually the CREATE API will differ from the Update API in the way it
    // is enabled. In a create invocation, the session is created only after all
    // the flows using a previous incarnation of the mirror session have been
    // cleanedup (i.e. mirror session removed by the periodic thread). Update is
    // treated as an incremental update.
    session.id = spec.key_or_handle().mirrorsession_id();
    session.truncate_len = spec.snaplen();
    if (spec.has_local_span_if()) {
    }
    if (spec.has_rspan_spec()) {
    }
    if (spec.has_erspan_spec()) {
    }
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_active_flows(0);
    switch (spec.destination_case()) {
    case MirrorSessionSpec::kLocalSpanIf: {
        HAL_TRACE_DEBUG("Local Span IF is true");
        ifid = spec.local_span_if();
        id = if_lookup_key_or_handle(ifid);
        if (id != NULL) {
            session.dest_if = id;
        } else {
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
        session.type = hal::MIRROR_DEST_LOCAL;
        break;
    }
    case MirrorSessionSpec::kRspanSpec: {
        HAL_TRACE_DEBUG("RSpan IF is true");
        auto rspan = spec.rspan_spec();
        ifid = rspan.intf();
        session.dest_if = if_lookup_key_or_handle(ifid);
        auto encap = rspan.rspan_encap();
        if (encap.encap_type() == types::ENCAP_TYPE_DOT1Q) {
            session.mirror_destination_u.r_span_dest.vlan = encap.encap_value();
        }
        session.type = hal::MIRROR_DEST_RSPAN;
        break;
    }
    case MirrorSessionSpec::kErspanSpec: {
        HAL_TRACE_DEBUG("ERSpan IF is true");
        auto erspan = spec.erspan_spec();
        ip_addr_t src_addr, dst_addr;
        ip_addr_spec_to_ip_addr(&src_addr, erspan.src_ip());
        ip_addr_spec_to_ip_addr(&dst_addr, erspan.dest_ip());
        ep_t *ep;
        switch (dst_addr.af) {
            case IP_AF_IPV4:
                ep = find_ep_by_v4_key(spec.meta().vrf_id(), dst_addr.addr.v4_addr);
                break;
            case IP_AF_IPV6:
                ep = find_ep_by_v6_key(spec.meta().vrf_id(), &dst_addr);
                break;
            default:
                HAL_TRACE_ERR("Unknown ERSPAN dest AF {}", dst_addr.af);
                return HAL_RET_INVALID_ARG;
        }
        if (ep == NULL) {
            HAL_TRACE_ERR("Unknown ERSPAN dest {}, vrfId {}",
                          ipaddr2str(&dst_addr), spec.meta().vrf_id());
            return HAL_RET_INVALID_ARG;
        }
        auto dest_if = find_if_by_handle(ep->if_handle);
        if (dest_if == NULL) {
            HAL_TRACE_ERR("Could not find if ERSPAN dest {}",
                          ipaddr2str(&dst_addr));
            return HAL_RET_INVALID_ARG;
        }
        session.dest_if = dest_if;
        auto ift = find_if_by_handle(ep->gre_if_handle);
        if (ift == NULL) {
            HAL_TRACE_ERR("Could not find ERSPAN tunnel dest if {}",
                          ipaddr2str(&dst_addr));
            return HAL_RET_INVALID_ARG;
        }
        if (ift->if_type != intf::IF_TYPE_TUNNEL) {
            HAL_TRACE_ERR("No tunnel to ERSPAN dest {}",
                          ipaddr2str(&dst_addr));
            return HAL_RET_INVALID_ARG;
        }
        if (!is_if_type_tunnel(ift)) {
            HAL_TRACE_ERR("Not GRE tunnel to ERSPAN dest {}",
                          ipaddr2str(&dst_addr));
            return HAL_RET_INVALID_ARG;
        }
        session.mirror_destination_u.er_span_dest.tunnel_if = ift;
        session.type = hal::MIRROR_DEST_ERSPAN;
        break;
    }
    default: {
        HAL_TRACE_ERR("Unknown session type{}", spec.destination_case());
        return HAL_RET_INVALID_ARG;
    }
    }
    args.session = &session;
    pd_func_args.pd_mirror_session_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Create failed {}", ret);
    } else {
        HAL_TRACE_DEBUG("Create Succeeded {}", session.id);
    }
    return ret;
}

static hal_ret_t
mirror_session_process_get (MirrorSessionGetRequest &req,
                            pd_mirror_session_get_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("{}: Mirror Session ID {}", __FUNCTION__,
            req.key_or_handle().mirrorsession_id());
    memset(args->session, 0, sizeof(mirror_session_t));
    pd_func_args.pd_mirror_session_get = args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: PD API failed {}", __FUNCTION__, ret);
        return ret;
    }
    return ret;
}

hal_ret_t
mirror_session_get(MirrorSessionGetRequest &req, MirrorSessionGetResponseMsg *rsp)
{
    pd_mirror_session_get_args_t args;
    mirror_session_t session;
    hal_ret_t ret;
    bool exists = false;

    args.session = &session;
    if (!req.has_key_or_handle()) {
        /* Iterate over all the sessions */
        for (int i = 0; i < MAX_MIRROR_SESSION_DEST; i++) {
            args.session->id = i;
            ret = mirror_session_process_get(req, &args);
            if (ret == HAL_RET_OK) {
                exists = true;
                auto response = rsp->add_response();
                response->set_api_status(types::API_STATUS_OK);
                response->mutable_spec()->mutable_key_or_handle()->set_mirrorsession_id(i);
                response->mutable_spec()->set_snaplen(session.truncate_len);
            }
        }
        if (!exists) {
            auto response = rsp->add_response();
            response->set_api_status(types::API_STATUS_NOT_FOUND);
        }
    } else {
        auto response = rsp->add_response();
        args.session->id = req.key_or_handle().mirrorsession_id();
        ret = mirror_session_process_get(req, &args);
        if (ret == HAL_RET_OK) {
            response->set_api_status(types::API_STATUS_OK);
            response->mutable_spec()->mutable_key_or_handle()->set_mirrorsession_id(args.session->id);
            response->mutable_spec()->set_snaplen(session.truncate_len);
        } else {
            response->set_api_status(types::API_STATUS_INVALID_ARG);
        }
    }

    /* TODO: Find the interface ID depending on interface type.
       verify against local cache of session.
       switch (session->type) {
       case hal::MIRROR_LOCAL_SPAN_ID:
       break
       case hal::MIRROR_DEST_RSPAN:
       break
       case hal::MIRROR_DEST_ERSPAN:
       break
       case hal::MIRROR_DEST_NONE:
       break
       } */
    return HAL_RET_OK;
}

hal_ret_t
mirror_session_delete (MirrorSessionDeleteRequest &req, MirrorSessionDeleteResponse *rsp)
{
    pd_mirror_session_delete_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    mirror_session_t session;
    hal_ret_t ret;

    HAL_TRACE_DEBUG("{}: Delete Mirror Session ID {}", __FUNCTION__,
            req.key_or_handle().mirrorsession_id());
    memset(&session, 0, sizeof(session));
    session.id = req.key_or_handle().mirrorsession_id();
    args.session = &session;
    pd_func_args.pd_mirror_session_delete = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_MIRROR_SESSION_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: PD API failed {}", __FUNCTION__, ret);
        rsp->set_api_status(types::API_STATUS_OK);
        rsp->mutable_key_or_handle()->set_mirrorsession_id(session.id);
    }
    return ret;
}

hal_ret_t
collector_create (CollectorSpec &spec, CollectorResponse *rsp)
{
    collector_config_t cfg;
    pd::pd_collector_create_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("{}: ExportID {}", __FUNCTION__,
            spec.key_or_handle().collector_id());

    cfg.exporter_id = spec.key_or_handle().collector_id();
    ip_addr_spec_to_ip_addr(&cfg.src_ip, spec.src_ip());
    ip_addr_spec_to_ip_addr(&cfg.dst_ip, spec.dest_ip());
    auto ep = find_ep_by_v4_key(spec.meta().vrf_id(), cfg.dst_ip.addr.v4_addr);
    if (ep == NULL) {
        HAL_TRACE_ERR("PI-Collector:{}: Unknown endpoint {} : {}", __FUNCTION__,
            spec.meta().vrf_id(), ipaddr2str(&cfg.dst_ip));
        return HAL_RET_INVALID_ARG;
    }
    memcpy(cfg.dest_mac, ep->l2_key.mac_addr, sizeof(cfg.dest_mac));
    cfg.template_id = spec.template_id();
    switch (spec.format()) {
        case telemetry::ExportFormat::IPFIX:
            cfg.format = EXPORT_FORMAT_IPFIX;
            break;
        case telemetry::ExportFormat::NETFLOWV9:
            cfg.format = EXPORT_FORMAT_NETFLOW9;
            break;
        default:
            HAL_TRACE_DEBUG("PI-Collector:{}: Unknown format type {}", __FUNCTION__, spec.template_id());
            return HAL_RET_INVALID_ARG;
    }
    cfg.protocol = spec.protocol();
    cfg.dport = spec.dest_port();
    auto encap = spec.encap();
    if (encap.encap_type() == types::ENCAP_TYPE_DOT1Q) {
        cfg.vlan = encap.encap_value();
    } else {
        HAL_TRACE_DEBUG("PI-Collector:{}: Unsupport Encap {}", __FUNCTION__, encap.encap_type());
        return HAL_RET_INVALID_ARG;
    }
    cfg.l2seg = l2seg_lookup_by_handle(spec.l2seg_key_handle().l2segment_handle());
    if (cfg.l2seg == NULL) {
        HAL_TRACE_DEBUG("PI-Collector:{}: Could not retrieve L2 segment", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }
    auto dmac = l2seg_get_rtr_mac(cfg.l2seg);
    if (dmac == NULL) {
        HAL_TRACE_DEBUG("PI-Collector:{}: Could not retrieve L2 segment source mac", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }
    memcpy(cfg.src_mac, *dmac, sizeof(cfg.src_mac));
    args.cfg = &cfg;
    pd_func_args.pd_collector_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_COLLECTOR_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Collector:{}: PD API failed {}", __FUNCTION__, ret);
    }
    HAL_TRACE_DEBUG("{}: SUCCESS: ExportID {}, dest {}, source {},  port {}", __FUNCTION__,
            spec.key_or_handle().collector_id(), ipaddr2str(&cfg.dst_ip),
            ipaddr2str(&cfg.src_ip), cfg.dport);
    return ret;
}

hal_ret_t
collector_update (CollectorSpec &spec, CollectorResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
collector_delete (CollectorDeleteRequest &req, CollectorDeleteResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
collector_get (CollectorGetRequest &req, CollectorGetResponseMsg *rsp)
{

    return HAL_RET_OK;
}

hal_ret_t
flow_monitor_acl_ctx_create()
{
    // Create lib acl ctx
    HAL_TRACE_DEBUG("Creating flow_mon acl ctx");
    flowmon_acl_ctx = hal::rule_lib_init("flowmon_rule", &flowmon_rule_config_glbl);
    if (!flowmon_acl_ctx) {
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

static hal_ret_t
populate_flow_monitor_rule (FlowMonitorRuleSpec &spec,
                            flow_monitor_rule_t *rule)
{
    hal_ret_t   ret = HAL_RET_OK;

    // Populate the rule_match structure
    ret = rule_match_spec_extract(spec.match(), &rule->rule_match);
    if (ret != HAL_RET_OK) {
        rule_match_cleanup(&rule->rule_match);
        HAL_TRACE_ERR("Failed to retrieve rule_match");
        return ret;
    }
    if (spec.has_action()) {
        int n = spec.action().ms_key_handle_size();
        for (int i = 0; i < n; i++) {
            rule->action.mirror_destinations[i] = spec.action().ms_key_handle(i).mirrorsession_id();
        }
        n = spec.action().action_size();
        if (n != 0) {
            // Only one action for mirroring
            rule->action.mirror_to_cpu = (spec.action().action(0) ==
                                       telemetry::MIRROR_TO_CPU) ? true : false;
        }
    }
    ret = rule_match_rule_add(&flowmon_acl_ctx, &rule->rule_match, 0, (void *)&rule->ref_count);
    return ret;
}

hal_ret_t
flow_monitor_rule_create (FlowMonitorRuleSpec &spec, FlowMonitorRuleResponse *rsp)
{
    flow_monitor_rule_t *rule;
    uint32_t rule_id;
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("PI-FlowMonitorRule create");
    rule_id = spec.key_or_handle().flowmonitorrule_id();
    if (spec.meta().vrf_id() == HAL_VRF_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        HAL_TRACE_ERR("vrf {}", spec.meta().vrf_id());
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    if (rule_id >= MAX_FLOW_MONITOR_RULES) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        HAL_TRACE_ERR("ruleid {}", spec.key_or_handle().flowmonitorrule_id());
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    rule = flow_monitor_rule_alloc_init();
    rule->vrf_id = spec.meta().vrf_id();
    flow_mon_rules[rule_id] = rule;
    ret = populate_flow_monitor_rule(spec, rule);
    if (ret != HAL_RET_OK) {
        goto end;
    }

end:
    return ret;
}

hal_ret_t
flow_monitor_rule_update (FlowMonitorRuleSpec &spec, FlowMonitorRuleResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

hal_ret_t
flow_monitor_rule_delete (FlowMonitorRuleDeleteRequest &req, FlowMonitorRuleDeleteResponse *rsp)
{
    pd_flow_monitor_rule_create_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};
    flow_monitor_rule_t rule = {0};
    hal_ret_t ret;

    if (req.meta().vrf_id() == HAL_VRF_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        HAL_TRACE_ERR("vrf {}", req.meta().vrf_id());
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    rule.vrf_id = req.meta().vrf_id();
    rule.rule_id = req.key_or_handle().flowmonitorrule_id();
    args.rule = &rule;
    pd_func_args.pd_flow_monitor_rule_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_FLOW_MONITOR_RULE_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-FlowMonitor delete failed {}", ret);
    } else {
        HAL_TRACE_DEBUG("PI-MirrorSession delete succeeded");
    }

end:
    return ret;
}

hal_ret_t
flow_monitor_rule_get (FlowMonitorRuleGetRequest &req, FlowMonitorRuleGetResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

static void
populate_drop_monitor_rule (DropMonitorRuleSpec &spec,
                            drop_monitor_rule_t *rule)
{
    rule->codes.drop_input_mapping = spec.reasons().drop_input_mapping();
    rule->codes.drop_input_mapping_dejavu = spec.reasons().drop_input_mapping_dejavu();
    rule->codes.drop_flow_hit = spec.reasons().drop_flow_hit();
    rule->codes.drop_flow_miss = spec.reasons().drop_flow_miss();
    rule->codes.drop_ipsg = spec.reasons().drop_ipsg();
    rule->codes.drop_nacl = spec.reasons().drop_nacl();
    rule->codes.drop_malformed_pkt = spec.reasons().drop_malformed_pkt();
    rule->codes.drop_ip_normalization = spec.reasons().drop_ip_normalization();
    rule->codes.drop_tcp_normalization = spec.reasons().drop_tcp_normalization();
    rule->codes.drop_tcp_non_syn_first_pkt = spec.reasons().drop_tcp_non_syn_first_pkt();
    rule->codes.drop_icmp_normalization = spec.reasons().drop_icmp_normalization();
    rule->codes.drop_input_properties_miss = spec.reasons().drop_input_properties_miss();
    rule->codes.drop_tcp_out_of_window = spec.reasons().drop_tcp_out_of_window();
    rule->codes.drop_tcp_split_handshake = spec.reasons().drop_tcp_split_handshake();
    rule->codes.drop_tcp_win_zero_drop = spec.reasons().drop_tcp_win_zero_drop();
    rule->codes.drop_tcp_data_after_fin = spec.reasons().drop_tcp_data_after_fin();
    rule->codes.drop_tcp_non_rst_pkt_after_rst = spec.reasons().drop_tcp_non_rst_pkt_after_rst();
    rule->codes.drop_tcp_invalid_responder_first_pkt = spec.reasons().drop_tcp_invalid_responder_first_pkt();
    rule->codes.drop_tcp_unexpected_pkt = spec.reasons().drop_tcp_unexpected_pkt();
    rule->codes.drop_src_lif_mismatch = spec.reasons().drop_src_lif_mismatch();
    rule->codes.drop_parser_icrc_error = spec.reasons().drop_parser_icrc_error();
    rule->codes.drop_parse_len_error = spec.reasons().drop_parse_len_error();
    rule->codes.drop_hardware_error = spec.reasons().drop_hardware_error();
    return;
}

hal_ret_t
drop_monitor_rule_create (DropMonitorRuleSpec &spec, DropMonitorRuleResponse *rsp)
{
    pd_drop_monitor_rule_create_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};
    drop_monitor_rule_t rule = {0};
    hal_ret_t ret = HAL_RET_OK;
    int sess_id;

    populate_drop_monitor_rule(spec, &rule);
    int n = spec.ms_key_handle_size();
    for (int i = 0; i < n; i++) {
        sess_id = spec.ms_key_handle(i).mirrorsession_id();
        if (sess_id >= MAX_MIRROR_SESSION_DEST) {
            ret = HAL_RET_INVALID_ARG;
            HAL_TRACE_ERR("PI-DropMonitor create failed {} mirror_dest_id: {}",
                           ret, sess_id);
            goto end;
        }
        rule.mirror_destinations[sess_id] = true;
    }
    args.rule = &rule;
    pd_func_args.pd_drop_monitor_rule_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DROP_MONITOR_RULE_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-DropMonitor create failed {}", ret);
        goto end;
    } else {
        HAL_TRACE_DEBUG("PI-MirrorSession create succeeded");
    }

end:
    return ret;
}

hal_ret_t
drop_monitor_rule_update (DropMonitorRuleSpec &spec, DropMonitorRuleResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
drop_monitor_rule_delete (DropMonitorRuleDeleteRequest &req, DropMonitorRuleDeleteResponse *rsp)
{
    pd_drop_monitor_rule_create_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};
    drop_monitor_rule_t rule = {0};
    hal_ret_t ret;

    rule.rule_id = req.key_or_handle().dropmonitorrule_id();
    args.rule = &rule;
    pd_func_args.pd_drop_monitor_rule_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DROP_MONITOR_RULE_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-DropMonitor delete failed {}", ret);
        goto end;
    } else {
        HAL_TRACE_DEBUG("PI-MirrorSession delete succeeded");
    }

end:
    return ret;
}

hal_ret_t
drop_monitor_rule_get (DropMonitorRuleGetRequest &req, DropMonitorRuleGetResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

hal_ret_t
export_control_create (ExportControlSpec &spec, ExportControlResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

hal_ret_t
export_control_update (ExportControlSpec &spec, ExportControlResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

hal_ret_t
export_control_delete (ExportControlDeleteRequest &req, ExportControlDeleteResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}


hal_ret_t
export_control_get(ExportControlGetRequest &req, ExportControlGetResponseMsg *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

hal_ret_t
hal_telemetry_init_cb (hal_cfg_t *hal_cfg)
{
    return HAL_RET_OK;
}

hal_ret_t
hal_telemetry_cleanup_cb (void)
{
    return HAL_RET_OK;
}

}    // namespace hal
