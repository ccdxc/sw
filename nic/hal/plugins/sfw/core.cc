//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/plugins/sfw/cfg/nwsec_group.hpp"
#include "core.hpp"
#include "sfw_pkt_utils.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/hal/plugins/sfw/alg_utils/alg_db.hpp"
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

fte::pipeline_action_t 
sfw_session_delete_cb (fte::ctx_t &ctx) {
    if (ctx.role() != hal::FLOW_ROLE_INITIATOR)
        return fte::PIPELINE_CONTINUE;

    if (ctx.session()) {
        ctx.flow_log()->sfw_action = (nwsec::SecurityAction)ctx.session()->sfw_action;
        ctx.flow_log()->rule_id = ctx.session()->sfw_rule_id;
    }    

    return fte::PIPELINE_CONTINUE;
}

hal_ret_t
net_sfw_match_app_redir(ctx_t                   &ctx,
                        nwsec_rule_t            *rule,
                        net_sfw_match_result_t  *match_rslt)
{
    dllist_ctxt_t        *lnode2 = NULL;
    nwsec_policy_appid_t *appid_policy = NULL;
    app_redir_ctx_t *app_ctx = app_redir_ctx(ctx, false);

    dllist_for_each(lnode2, &rule->appid_list_head) {
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
                    match_rslt->alg = rule->fw_rule_action.alg;
                    if (rule->fw_rule_action.sec_action == nwsec::SECURITY_RULE_ACTION_ALLOW) {
                        match_rslt->action = session::FLOW_ACTION_ALLOW;
                    } else {
                        match_rslt->action = session::FLOW_ACTION_DROP;
                    }
                    match_rslt->log    = rule->fw_rule_action.log_action;
                    match_rslt->sfw_action = rule->fw_rule_action.sec_action;
                    return HAL_RET_OK;
                }
            }
        }
    }
    return HAL_RET_OK;
}

static hal_ret_t
net_sfw_check_security_policy(ctx_t &ctx, net_sfw_match_result_t *match_rslt)
{
    hal_ret_t ret;
    hal::ipv4_tuple acl_key = {};
    ep_t            *sep = NULL;
    ep_t            *dep = NULL;

    hal::nwsec_rule_t *nwsec_rule;
    const hal::ipv4_rule_t *rule = NULL;
    const acl::acl_ctx_t *acl_ctx = NULL;

    HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy acl rule lookup for key={} vrf={}",
                   ctx.key(), hal::g_hal_state->customer_default_vrf());

    const char *ctx_name = nwsec_acl_ctx_name(hal::g_hal_state->customer_default_vrf());
    acl_ctx = acl::acl_get(ctx_name);
    if (acl_ctx == NULL) {
        HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy failed to lookup acl_ctx {}", ctx_name);
        return HAL_RET_FTE_RULE_NO_MATCH;
    }

    // initialize the acl key for v4 rules only, v6 we use only src_sg, dst_sg
    if (ctx.key().flow_type == hal::FLOW_TYPE_V4) {
        acl_key.ip_src =  ctx.key().sip.v4_addr;
        acl_key.ip_dst =  ctx.key().dip.v4_addr;
    }

    acl_key.proto =  ctx.key().proto;
    switch ( ctx.key().proto) {
    case types::IPPROTO_ICMP:
    case types::IPPROTO_ICMPV6:
        acl_key.icmp_type =  ctx.key().icmp_type;
        acl_key.icmp_code = ctx.key().icmp_code;
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
        HAL_TRACE_DEBUG("Any proto:{}", ctx.key().proto);
    }

    sep = ctx.sep();
    dep = ctx.dep();

    if (((!sep) || (!dep)) || (sep && dep && sep->sgs.sg_id_cnt == 0 && dep->sgs.sg_id_cnt == 0)) {
        ret = acl_classify(acl_ctx, (const uint8_t *)&acl_key, (const acl_rule_t **)&rule, 0x01);
        if (ret != HAL_RET_OK) {
            goto end_match;
        }

        if (rule == NULL) {
            ret = HAL_RET_FTE_RULE_NO_MATCH;
        }
    } else {
        for (int i = 0; i < sep->sgs.sg_id_cnt; i++) {
            for (int j = 0; j < dep->sgs.sg_id_cnt; j++) {
                acl_key.src_sg = sep->sgs.arr_sg_id[i];
                acl_key.dst_sg = dep->sgs.arr_sg_id[i];
                ret = acl_classify(acl_ctx, (const uint8_t *)&acl_key, (const acl_rule_t **)&rule, 0x01);
                if (ret != HAL_RET_OK) {
                    goto end_match;
                }

                if (rule != NULL) {
                    goto end_match;
                }
            }
        }
        ret = HAL_RET_FTE_RULE_NO_MATCH;
    }


 end_match:
    if (rule != NULL) {
        acl::ref_t *user_ref = get_rule_data((acl_rule_t *)rule);
        nwsec_rule = (hal::nwsec_rule_t *) RULE_MATCH_USER_DATA(user_ref, nwsec_rule_t, ref_count);

        rule_ctr_t *rule_ctr = get_rule_ctr((acl_rule_t *)rule);
        rule_ctr->rule_stats->total_hits++;
        switch (acl_key.proto) {
        case types::IPPROTO_ICMP:
        case types::IPPROTO_ICMPV6:
            rule_ctr->rule_stats->icmp_hits++;
            break;
        case types::IPPROTO_ESP:
            rule_ctr->rule_stats->esp_hits++;
            break;
        case types::IPPROTO_TCP:
            rule_ctr->rule_stats->tcp_hits++;
            break;
        case types::IPPROTO_UDP:
            rule_ctr->rule_stats->udp_hits++;
            break;
        default:
            HAL_TRACE_DEBUG("Stats: Any proto:{}", ctx.key().proto);
            rule_ctr->rule_stats->other_hits++;
        }

        if (!dllist_empty(&nwsec_rule->appid_list_head)) {
            net_sfw_match_app_redir(ctx, nwsec_rule, match_rslt);
        } else {
            HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy matched acl rule {} action={} alg={}",
                    nwsec_rule->rule_id, nwsec_rule->fw_rule_action.sec_action, nwsec_rule->fw_rule_action.alg);
            match_rslt->valid = 1;
            if (nwsec_rule->fw_rule_action.sec_action == nwsec::SECURITY_RULE_ACTION_ALLOW) {
                match_rslt->action = session::FLOW_ACTION_ALLOW;
            } else {
                match_rslt->action = session::FLOW_ACTION_DROP;
            }
            match_rslt->alg = nwsec_rule->fw_rule_action.alg;
            memcpy(&match_rslt->alg_opts, &nwsec_rule->fw_rule_action.app_options, sizeof(alg_opts));
            match_rslt->log = nwsec_rule->fw_rule_action.log_action;
            match_rslt->sfw_action = nwsec_rule->fw_rule_action.sec_action;
            match_rslt->rule_id = nwsec_rule->rule_id;
            match_rslt->idle_timeout = nwsec_rule->fw_rule_action.idle_timeout;
        }
    } else {
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
    hal_ret_t    ret;


    if (ctx.protobuf_request() && !ctx.sync_session_request()) {
        match_rslt->valid  = 1;
        match_rslt->action = ctx.sess_spec()->initiator_flow().flow_data().flow_info().flow_action();
        return HAL_RET_OK;
    }

    ret = net_sfw_check_security_policy(ctx, match_rslt);
    if (ret != HAL_RET_OK) {
        if (ret == HAL_RET_FTE_RULE_NO_MATCH) {
            match_rslt->valid = 1;
            match_rslt->action = session::FLOW_ACTION_DROP;
            match_rslt->sfw_action = nwsec::SECURITY_RULE_ACTION_IMPLICIT_DENY;
            return HAL_RET_OK;
        }
    }
    return HAL_RET_OK;
}

static void
net_sfw_generate_reject_pkt(ctx_t& ctx, bool status)
{
    cpu_to_p4plus_header_t      cpu_hdr = {0};
    p4plus_to_p4_header_t       p4plus_hdr = {0};
    uint8_t                    *pkt = NULL;
    uint32_t                    pkt_len = 0;
    hal_ret_t                   ret = HAL_RET_OK;
    header_rewrite_info_t  rewrite_info;
    header_push_info_t     push_info;

    if (!g_cpu_bypass_flowid) {
        // Get Hw bypass flow idx to send out TCP RST
        // or ICMP error for Firewall rejects
        pd_get_cpu_bypass_flowid_args_t args;
        pd::pd_func_args_t          pd_func_args = {0};
        args.hw_flowid = 0;
        pd_func_args.pd_get_cpu_bypass_flowid = &args;
        ret = hal_pd_call(PD_FUNC_ID_BYPASS_FLOWID_GET, &pd_func_args);
        if (ret == HAL_RET_OK) {
            g_cpu_bypass_flowid = args.hw_flowid;
        }
    }

    cpu_hdr.src_lif = SERVICE_LIF_CPU_BYPASS;
    p4plus_hdr.flow_index_valid = 1;
    p4plus_hdr.flow_index = g_cpu_bypass_flowid;
    p4plus_hdr.dst_lport_valid = 1;
    p4plus_hdr.compute_l4_csum = 1;
    p4plus_hdr.compute_ip_csum = 1;

    p4plus_hdr.dst_lport = ctx.cpu_rxhdr()?ctx.cpu_rxhdr()->src_lport:0;

    if (ctx.key().proto == IP_PROTO_TCP) {
        pkt_len = net_sfw_build_tcp_rst(ctx, &pkt, rewrite_info, push_info);
    } else if (ctx.key().proto == IP_PROTO_UDP) {
        // Generate ICMP Error
        pkt_len = net_sfw_build_icmp_error(ctx, &pkt, rewrite_info, push_info);
    }

    if (pkt_len)
        ctx.queue_txpkt(pkt, pkt_len, &cpu_hdr, &p4plus_hdr, HAL_LIF_CPU,
                    CPU_ASQ_QTYPE, CPU_ASQ_QID, CPU_SCHED_RING_ASQ, types::WRING_TYPE_ASQ,
                    net_sfw_free_reject_pkt);

    if (ctx.key().proto == IP_PROTO_TCP) {
        hal::incr_global_session_tcp_rst_stats(fte::fte_id());
    } else if (ctx.key().proto == IP_PROTO_UDP) {
        hal::incr_global_session_icmp_error_stats(fte::fte_id());
    }
}

pipeline_action_t
sfw_exec(ctx_t& ctx)
{
    hal_ret_t               ret;
    net_sfw_match_result_t  match_rslt = {};
    sfw_info_t              *sfw_info = (sfw_info_t*)ctx.feature_state();

    // security policy action
    flow_update_t flowupd = {type: FLOWUPD_ACTION};
    match_rslt.idle_timeout = HAL_MAX_INACTIVTY_TIMEOUT;

    if (hal::g_hal_state->is_flow_aware()) {
        flowupd.action = session::FLOW_ACTION_ALLOW;
        ret = ctx.update_flow(flowupd);  
        return PIPELINE_CONTINUE;
    }

    if (ctx.drop_flow()) {
        flowupd.action = session::FLOW_ACTION_DROP;
        sfw_info->sfw_done = true;
        goto install_flow;
    }

    // only ipv4 is handled in data path.
    if (!ctx.protobuf_request()  &&
         (ctx.key().flow_type == hal::FLOW_TYPE_V6 || ctx.key().flow_type == hal::FLOW_TYPE_L2)) {
        sfw_info->sfw_done = true;
        goto install_flow;

    }

    if ((!ctx.protobuf_request() && ctx.existing_session()) ||
        (ctx.role() == hal::FLOW_ROLE_INITIATOR &&
         (sfw_info->skip_sfw || sfw_info->sfw_done))) {
        return PIPELINE_CONTINUE;
    }

    // ALG Wild card entry table lookup.
    if (ctx.role() == hal::FLOW_ROLE_INITIATOR && !ctx.existing_session()) {
        expected_flow_t *expected_flow = lookup_expected_flow(ctx.key());
        if (expected_flow) {
            ret = expected_flow->handler(ctx, expected_flow);
            flow_update_t flowupd = {type: FLOWUPD_AGING_INFO};
            flowupd.aging_info.idle_timeout = sfw_info->idle_timeout;
            ret = ctx.update_flow(flowupd);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to update aging action");
            }
            ctx.set_feature_name(FTE_FEATURE_SFW.c_str());
            sfw_info->sfw_done = true;
        }
    }

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR &&
        !sfw_info->skip_sfw && !sfw_info->sfw_done) {
        ret = net_sfw_pol_check_sg_policy(ctx, &match_rslt);
        if (ret == HAL_RET_OK) {
            if (match_rslt.valid) {
                flowupd.action  = match_rslt.action;
                sfw_info->idle_timeout = match_rslt.idle_timeout;
                sfw_info->sfw_done = true;
                if (match_rslt.sfw_action == nwsec::SECURITY_RULE_ACTION_REJECT &&
                    ctx.valid_rflow()) {
                    // Register completion handler to send a reject packet out
                    // We need the forwarding information to use the CPU bypass
                    // queue as the flow doesnt really exist.
                    ctx.register_completion_handler(net_sfw_generate_reject_pkt);
                    ctx.set_ignore_session_create(true);
                    ctx.set_ipc_logging_disable(false);
                    ctx.set_drop();
                } else {
                    sfw_info->alg_proto = match_rslt.alg;
                    memcpy(&sfw_info->alg_opts, &match_rslt.alg_opts, sizeof(alg_opts));
                    flow_update_t flowupd = {type: FLOWUPD_AGING_INFO};
                    flowupd.aging_info.idle_timeout = match_rslt.idle_timeout;
                    ret = ctx.update_flow(flowupd);
                    if (ret != HAL_RET_OK) {
                        HAL_TRACE_ERR("Failed to update aging action");
                    }
                }
            } else {
                // ToDo ret value was ok but match_rslt.valid is 0
                // to handle the case if it happens
            }
        }

        flow_update_t flowupd = {type: FLOWUPD_SFW_INFO};
        flowupd.sfw_info.skip_sfw_reval = 0;
        flowupd.sfw_info.sfw_rule_id = match_rslt.rule_id;
        flowupd.sfw_info.sfw_action = (uint8_t)match_rslt.sfw_action;
        ret = ctx.update_flow(flowupd);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to update sfw action");
        }

        ctx.flow_log()->sfw_action = match_rslt.sfw_action;
        ctx.flow_log()->alg = match_rslt.alg;
        ctx.flow_log()->rule_id = match_rslt.rule_id;
    } else {
        sfw_info->sfw_done = true;
        //Responder Role: Not checking explicitly
        if (ctx.drop_flow()) {
            flowupd.action = session::FLOW_ACTION_DROP;
        } else {
            flowupd.action = session::FLOW_ACTION_ALLOW;
        }
        *ctx.flow_log() = *ctx.flow_log(hal::FLOW_ROLE_INITIATOR);
    }

install_flow:
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
    os << " ,idle_timeout=" << val.idle_timeout;
    return os << "}";
}

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
