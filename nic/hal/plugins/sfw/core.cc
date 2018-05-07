//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/fte/fte.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/p4/iris/include/defines.h"
#include "nic/hal/src/firewall/nwsec_group.hpp"
#include "core.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/hal/plugins/alg_utils/alg_db.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/fte/fte_flow.hpp"

using namespace hal::app_redir;
using namespace hal::plugins::alg_utils;
using namespace hal::pd;
using namespace fte;

static uint32_t g_cpu_bypass_flowid;

namespace nwsec {
std::ostream& operator<<(std::ostream& os, const ALGName val) {
    switch (val) {
    case APP_SVC_TFTP:     return os << "TFTP";
    case APP_SVC_FTP:      return os << "FTP";
    case APP_SVC_DNS:      return os << "DNS";
    case APP_SVC_SIP:      return os << "SIP";
    case APP_SVC_SUN_RPC:  return os << "SUNRPC";
    case APP_SVC_MSFT_RPC: return os << "MSFTRPC";
    case APP_SVC_RTSP:     return os << "RTSP";
    default:               return os << "NONE";
    }
}
}

namespace hal {
namespace plugins {
namespace sfw {

hal_ret_t
net_sfw_match_rules(ctx_t                  &ctx,
                    nwsec_policy_rules_t        *nwsec_plcy_rules,
                    net_sfw_match_result_t *match_rslt)
{
    flow_key_t          flow_key;
    nwsec_policy_svc_t  *nwsec_plcy_svc = NULL, *matched_svc = NULL;
    dllist_ctxt_t       *lnode = NULL;

    flow_key = ctx.key();
    HAL_TRACE_DEBUG(" proto {} dport {}", flow_key.proto, flow_key.dport);
    dllist_for_each(lnode, &nwsec_plcy_rules->fw_svc_list_head) {
        nwsec_plcy_svc = dllist_entry(lnode, nwsec_policy_svc_t, lentry);
        if (nwsec_plcy_svc != NULL) {
            //ToDo (lseshan) - identify the wildcard better way
            if (nwsec_plcy_svc->ipproto == 0) {
                // Match all
                matched_svc = nwsec_plcy_svc;
                HAL_TRACE_DEBUG("Wild card Rule match {} {}", match_rslt->action, match_rslt->log);
                break;

            } else if (nwsec_plcy_svc->ipproto == flow_key.proto) {        //Compare proto
                if (nwsec_plcy_svc->ipproto == IPPROTO_ICMP ||
                    nwsec_plcy_svc->ipproto == IPPROTO_ICMPV6) {
                    //Check the icmp message type
                    if (nwsec_plcy_svc->icmp_msg_type == flow_key.icmp_type)
                    {
                        matched_svc = nwsec_plcy_svc;
                        break;
                    }
                } else {
                    //Check the port match
                    if (nwsec_plcy_svc->dst_port == flow_key.dport) {
                        matched_svc = nwsec_plcy_svc;
                        break;
                    }
                }
            }
        }
    }

    // TODO: Can nwsec_plcy_svc be NULL?
    if(matched_svc) { // svc is wildcard or matched a specific service
        dllist_ctxt_t *lnode2 = NULL;
        nwsec_policy_appid_t *appid_policy = NULL;
        app_redir_ctx_t *app_ctx = app_redir_ctx(ctx, false);
        if(!dllist_empty(&nwsec_plcy_rules->appid_list_head)) {
            dllist_for_each(lnode2, &nwsec_plcy_rules->appid_list_head) {
                appid_policy = dllist_entry(lnode2, nwsec_policy_appid_t, lentry);
                if(appid_policy) {
                    if(!app_ctx->appid_started()) {
                        app_ctx->set_appid_needed(ctx);
                        match_rslt->valid  = 1;
                        match_rslt->action = session::FLOW_ACTION_ALLOW;
                        return HAL_RET_OK;
                    }

                    // Phase II invocation of dfw in flow miss pipeline or phase I invocation of dfw in l7 flow-hit pipeline
                    appid_info_t* appid_info = app_ctx->appid_info();
                    for(int i = 0; i < appid_info->id_count_; i++) {
                        if(appid_policy->appid == appid_info->ids_[i]) {
                            match_rslt->valid  = 1;
                            if(matched_svc) match_rslt->alg = matched_svc->alg;
                            if (nwsec_plcy_rules->action == nwsec::FIREWALL_ACTION_ALLOW) {
                                match_rslt->action = session::FLOW_ACTION_ALLOW;
                            } else {
                                match_rslt->action = session::FLOW_ACTION_DROP;
                            }
                            match_rslt->log    = nwsec_plcy_rules->log;
                            match_rslt->sfw_action = nwsec_plcy_rules->action;
                            return HAL_RET_OK;
                        }
                    }

                }
            }
        } else {
            match_rslt->valid  = 1;
            match_rslt->alg = matched_svc->alg;
            if (nwsec_plcy_rules->action == nwsec::FIREWALL_ACTION_ALLOW) {
                match_rslt->action = session::FLOW_ACTION_ALLOW;
            } else {
                match_rslt->action = session::FLOW_ACTION_DROP;
            }
            match_rslt->log    = nwsec_plcy_rules->log;
            match_rslt->sfw_action = nwsec_plcy_rules->action;
            return HAL_RET_OK;
        }
    }

    return HAL_RET_FTE_RULE_NO_MATCH;
}

hal_ret_t
net_sfw_check_policy_pair(ctx_t                    &ctx,
                          uint32_t                      src_sg,
                          uint32_t                      dst_sg,
                          net_sfw_match_result_t   *match_rslt)
{
    nwsec_policy_cfg_t    *nwsec_plcy_cfg = NULL;
    dllist_ctxt_t         *lnode = NULL;
    nwsec_policy_rules_t  *nwsec_plcy_rules = NULL;
    hal_ret_t             ret = HAL_RET_OK;

    nwsec_policy_key_t plcy_key;

    plcy_key.sg_id = src_sg;
    plcy_key.peer_sg_id = dst_sg;

    HAL_TRACE_DEBUG("Lookup Policy for SG Pair: {} {}", src_sg, dst_sg);

    nwsec_plcy_cfg = nwsec_policy_cfg_lookup_by_key(plcy_key);
    if (nwsec_plcy_cfg == NULL) {
        HAL_TRACE_ERR("Failed to get the src_sg: {} peer_sg {}", src_sg, dst_sg);
        return HAL_RET_SG_NOT_FOUND;
    }

    dllist_for_each(lnode, &nwsec_plcy_cfg->rules_head) {
        nwsec_plcy_rules = dllist_entry(lnode, nwsec_policy_rules_t, lentry);
        ret = net_sfw_match_rules(ctx, nwsec_plcy_rules, match_rslt);
        if (ret == HAL_RET_OK && match_rslt->valid) {
            return ret;
        }
    }
    return ret;
}

static hal_ret_t
net_sfw_check_security_policy(ctx_t &ctx, net_sfw_match_result_t *match_rslt)
{
    hal_ret_t ret;
    rule_data_t *rule_data;
    hal::ipv4_tuple acl_key = {};

    const hal::nwsec_rule_t *nwsec_rule;
    const hal::ipv4_rule_t *rule = NULL;
    const acl::acl_ctx_t *acl_ctx = NULL;

    HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy acl rule lookup for key={}", ctx.key());

    const char *ctx_name = nwsec_acl_ctx_name(ctx.key().svrf_id);
    acl_ctx = acl::acl_get(ctx_name);
    if (acl_ctx == NULL) {
        HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy failed to lookup acl_ctx {}", ctx_name);
        return HAL_RET_FTE_RULE_NO_MATCH;
    }

    // initialize the acl key
    if (ctx.key().flow_type != hal::FLOW_TYPE_V4) {
        // TODO(goli) only v4 rules for now
        ret = HAL_RET_FTE_RULE_NO_MATCH;
        goto end;
    }

    acl_key.proto =  ctx.key().proto;
    acl_key.ip_src =  ctx.key().sip.v4_addr;
    acl_key.ip_dst =  ctx.key().dip.v4_addr;
    switch ( ctx.key().proto) {
    case types::IPPROTO_ICMP:
    case types::IPPROTO_ICMPV6:
        acl_key.port_src =  ctx.key().icmp_id;
        acl_key.port_dst = ((ctx.key().icmp_type << 8) |  ctx.key().icmp_code);
        break;
    case types::IPPROTO_ESP:
        acl_key.port_src = ctx.key().spi >> 16 & 0xFFFF;
        acl_key.port_dst = ctx.key().spi & 0xFFFF;
        break;
    case types::IPPROTO_TCP:
    case types::IPPROTO_UDP:
        acl_key.port_src = ctx.key().sport;
        acl_key.port_dst = ctx.key().dport;
        break;
    default:
        HAL_ASSERT(true);
        ret = HAL_RET_FTE_RULE_NO_MATCH;
        goto end;
    }

    ret = acl_classify(acl_ctx, (const uint8_t *)&acl_key, (const acl_rule_t **)&rule, 0x01);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    if (rule == NULL) {
        ret = HAL_RET_FTE_RULE_NO_MATCH;
        goto end;
    }


    rule_data = ( hal::rule_data_t *)rule->data.userdata;
    nwsec_rule = (const hal::nwsec_rule_t *)rule_data->userdata;

    HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy matched acl rule {} action={} alg={}",
                    nwsec_rule->rule_id, nwsec_rule->fw_rule_action.sec_action, nwsec_rule->fw_rule_action.alg);

    match_rslt->valid = 1;
    if (nwsec_rule->fw_rule_action.sec_action == nwsec::SECURITY_RULE_ACTION_ALLOW) {
        match_rslt->action = session::FLOW_ACTION_ALLOW;
    } else {
        match_rslt->action = session::FLOW_ACTION_DROP;
    }
    match_rslt->alg = nwsec_rule->fw_rule_action.alg;

 end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy rule lookup failed ret={}", ret);
    }

    if (acl_ctx) {
        acl_deref(acl_ctx);
    }
    return ret;
}


hal_ret_t
net_sfw_pol_check_sg_policy(ctx_t                  &ctx,
                            net_sfw_match_result_t *match_rslt)
{
    ep_t        *sep = NULL;
    ep_t        *dep = NULL;
    hal_ret_t    ret;


    if (ctx.protobuf_request()) {
        match_rslt->valid  = 1;
        match_rslt->action = ctx.sess_spec()->initiator_flow().flow_data().flow_info().flow_action();
        return HAL_RET_OK;
    }

    if (ctx.drop_flow()) {
        match_rslt->valid = 1;
        match_rslt->action = session::FLOW_ACTION_DROP;
        return HAL_RET_OK;
    }

    ret = net_sfw_check_security_policy(ctx, match_rslt);
    if (ret == HAL_RET_OK) {
        return ret;
    }

    // TODO(goli) - folloing code is kept for DOLs using the old policy model

    //ToDo (lseshan) - For now if a sep or dep is not found allow
    // Eventually use prefix to find the sg
    if (ctx.sep() == NULL ||  (ctx.dep() == NULL)) {
        match_rslt->valid = 1;
        match_rslt->action = session::FLOW_ACTION_ALLOW;
        return HAL_RET_OK;
    }

    sep  = ctx.sep();
    dep  = ctx.dep();
    for (int i = 0; i < sep->sgs.sg_id_cnt; i++) {
        for (int j = 0; j < dep->sgs.sg_id_cnt; j++) {
            ret = net_sfw_check_policy_pair(ctx,
                                            sep->sgs.arr_sg_id[i],
                                            dep->sgs.arr_sg_id[j],
                                            match_rslt);
            if (ret == HAL_RET_OK) {
                if (match_rslt->valid) {
                    return ret;
                }
            } else {
                // Handle error case
            }
        }
    }

    // ToDo (lseshan) Handle SP miss condition
    // For now hardcoding to ALLOW but we have read default action and act
    // accordingly
    match_rslt->valid = 1;
    match_rslt->action = session::FLOW_ACTION_ALLOW;

    return HAL_RET_OK;
}

static void
net_sfw_generate_reject_pkt(ctx_t& ctx, bool status)
{
    cpu_to_p4plus_header_t      cpu_hdr = {0};
    p4plus_to_p4_header_t       p4plus_hdr = {0};
    uint8_t                    *pkt = NULL;
    uint32_t                    pkt_len = 0;
    fte::flow_t                *rflow = NULL;
    hal_ret_t                   ret = HAL_RET_OK;
    fwding_info_t          fwding;
    header_rewrite_info_t  rewrite_info;
    header_push_info_t     push_info;

    if (!g_cpu_bypass_flowid) {
        // Get Hw bypass flow idx to send out TCP RST
        // or ICMP error for Firewall rejects
        pd_get_cpu_bypass_flowid_args_t args;
        args.hw_flowid = 0;
        ret = hal_pd_call(PD_FUNC_ID_BYPASS_FLOWID_GET, &args);
        if (ret == HAL_RET_OK) {
            g_cpu_bypass_flowid = args.hw_flowid;
        }
    }

    cpu_hdr.src_lif = SERVICE_LIF_CPU_BYPASS;
    p4plus_hdr.flow_index_valid = 1;
    p4plus_hdr.flow_index = g_cpu_bypass_flowid;
    p4plus_hdr.dst_lport_valid = 1;

    // Get the reverse flow forwarding info
    rflow = ctx.flow(hal::FLOW_ROLE_RESPONDER);
    fwding = rflow->fwding();
    rewrite_info = rflow->header_rewrite_info();
    push_info = rflow->header_push_info();

    p4plus_hdr.dst_lport = fwding.lport;

    if (ctx.key().proto == IP_PROTO_TCP) {
        pkt_len = net_sfw_build_tcp_rst(ctx, &pkt, rewrite_info, push_info);
    } else if (ctx.key().proto == IP_PROTO_UDP) {
        // Generate ICMP Error
        pkt_len = net_sfw_build_icmp_error(ctx, &pkt, rewrite_info, push_info);
    }

    if (pkt_len)
        ctx.queue_txpkt(pkt, pkt_len, &cpu_hdr, &p4plus_hdr, hal::SERVICE_LIF_CPU,
                    CPU_ASQ_QTYPE, CPU_ASQ_QID, CPU_SCHED_RING_ASQ, types::WRING_TYPE_ASQ,
                    net_sfw_free_reject_pkt);
}

pipeline_action_t
sfw_exec(ctx_t& ctx)
{
    hal_ret_t               ret;
    net_sfw_match_result_t  match_rslt = {};
    sfw_info_t              *sfw_info = (sfw_info_t*)ctx.feature_state();

    // security policy action
    flow_update_t flowupd = {type: FLOWUPD_ACTION};

    HAL_TRACE_DEBUG("In sfw_exec....");

    // ALG Wild card entry table lookup.
    if (!ctx.existing_session() && ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        HAL_TRACE_DEBUG("Looking up expected flow...");
        expected_flow_t *expected_flow = lookup_expected_flow(ctx.key());
        if (expected_flow) {
            ret = expected_flow->handler(ctx, expected_flow);
        }
    }
    // reset the feature name back to sfw
    // (expected_flow_handler might have changed the name)
    // TODO(goli) instead set the feature name to the feature that installed
    //            the expected flow prior to calling handler, so that handler
    //            doen't need to do it
    ctx.set_feature_name(FTE_FEATURE_SFW.c_str());

    // ToDo (lseshan) - for now handling only ingress rules
    // Need to select SPs based on the flow direction
    HAL_TRACE_DEBUG("Firewall lookup {}", (sfw_info->skip_sfw)?"skipped":"begin");
    HAL_TRACE_DEBUG("Firewall lookup {}", (sfw_info->sfw_done)?"skipped":"begin");
    if (ctx.role() == hal::FLOW_ROLE_INITIATOR && !sfw_info->skip_sfw && !sfw_info->sfw_done) {
        ret = net_sfw_pol_check_sg_policy(ctx, &match_rslt);
        if (ret == HAL_RET_OK) {
            if (match_rslt.valid) {
                flowupd.action  = match_rslt.action;
                sfw_info->alg_proto = match_rslt.alg;
                sfw_info->sfw_done = true;
                //ctx.log         = match_rslt.log;
                HAL_TRACE_DEBUG("Match result: {}", match_rslt);
                if (match_rslt.sfw_action == nwsec::FIREWALL_ACTION_REJECT &&
                    ctx.valid_rflow()) {
                    // Register completion handler to send a reject packet out
                    // We need the forwarding information to use the CPU bypass
                    // queue as the flow doesnt really exist.
                    ctx.register_completion_handler(net_sfw_generate_reject_pkt);
                    ctx.set_ignore_session_create(true);
                }
            } else {
                // ToDo ret value was ok but match_rslt.valid is 0
                // to handle the case if it happens
            }
        }
    } else {
        //Responder Role: Not checking explicitly
        if (ctx.drop_flow()) {
            flowupd.action = session::FLOW_ACTION_DROP;
        } else {
            flowupd.action = session::FLOW_ACTION_ALLOW;
        }
    }

    ret = ctx.update_flow(flowupd);
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return PIPELINE_END;
    }

    return PIPELINE_CONTINUE;
}

std::ostream& operator<<(std::ostream& os, const sfw_info_t& val) {
    os << "{alg_proto="<< val.alg_proto;
    os << " ,skip_sfw=" << val.skip_sfw;
    os << " ,sfw_done=" << val.sfw_done;
    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const net_sfw_match_result_t& val) {
    os << "{valid="<< val.valid;
    os << " ,action=" << val.action;
    os << " ,alg=" << val.alg;
    os << " ,log=" << val.log;
    os << " ,sfw_action=" << val.sfw_action;
    return os << "}";
}

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
