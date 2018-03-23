//{C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "nic/fte/fte.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/p4/iris/include/defines.h"
#include "nic/hal/src/security/nwsec_group.hpp"
#include "core.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/hal/plugins/alg_utils/alg_db.hpp"

using namespace hal::app_redir;
using namespace hal::plugins::alg_utils;

namespace hal {
namespace plugins {
namespace sfw {

hal_ret_t
net_sfw_match_rules(fte::ctx_t                  &ctx,
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
                            match_rslt->action = (session::FlowAction)nwsec_plcy_rules->action;
                            match_rslt->log    = nwsec_plcy_rules->log;
                            return HAL_RET_OK;
                        }
                    }

                }
            }
        } else {
            match_rslt->valid  = 1;
            match_rslt->alg = matched_svc->alg;
            match_rslt->action = (session::FlowAction)nwsec_plcy_rules->action;
            match_rslt->log    = nwsec_plcy_rules->log;
            return HAL_RET_OK;
        }
    }

    return HAL_RET_FTE_RULE_NO_MATCH;
}

hal_ret_t
net_sfw_check_policy_pair(fte::ctx_t                    &ctx,
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

    HAL_TRACE_DEBUG("fte::Lookup Policy for SG Pair: {} {}", src_sg, dst_sg);

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
net_sfw_check_security_policy(fte::ctx_t &ctx, net_sfw_match_result_t *match_rslt)
{
    hal_ret_t ret;
    hal::ipv4_tuple acl_key = {};

    const hal::nwsec_rule_t *nwsec_rule;
    const hal::ipv4_rule_t *rule = NULL;
    const acl::acl_ctx_t *acl_ctx = NULL;

    HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy acl rule lookup for key={}", ctx.key());

    acl_ctx = acl::acl_get(nwsec_acl_ctx_name(ctx.key().vrf_id));
    if (acl_ctx == NULL) {
        HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy failed to lookup acl_ctx");
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

    nwsec_rule = (const hal::nwsec_rule_t *)rule->data.userdata;

    HAL_TRACE_DEBUG("sfw::net_sfw_check_security_policy matched acl rule {}", nwsec_rule->rule_id);

    match_rslt->valid = 1;
    if (nwsec_rule->action == nwsec::SECURITY_RULE_ACTION_ALLOW) {
        match_rslt->action = session::FLOW_ACTION_ALLOW;
    } else {
        match_rslt->action = session::FLOW_ACTION_DROP;
    }
    match_rslt->alg = nwsec_rule->alg;

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
net_sfw_pol_check_sg_policy(fte::ctx_t                  &ctx,
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


fte::pipeline_action_t
sfw_exec(fte::ctx_t& ctx)
{
    hal_ret_t               ret;
    net_sfw_match_result_t  match_rslt = {};
    sfw_info_t              *sfw_info = (sfw_info_t*)ctx.feature_state();

    // security policy action
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_ACTION};

    // ALG Wild card entry table lookup. 
    expected_flow_t *expected_flow = lookup_expected_flow(ctx.key());
    if (expected_flow) {
        ret = expected_flow->handler(ctx, expected_flow);
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
                HAL_TRACE_DEBUG("Matching rule: {}", sfw_info->alg_proto);
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
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
