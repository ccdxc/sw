//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/plugins/cfg/ipsec/ipsec.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/nw/vrf_api.hpp"

namespace hal {
namespace proxy {

static bool
is_dst_local_ep(hal::if_t *intf) {
    return (intf != NULL && intf->if_type == intf::IF_TYPE_ENIC);
}

static inline void
update_flow_qos (fte::flow_update_t *flowupd)
{
    // For uplink-uplink, QOS needs to be updated based on ipsec flow
    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_SWITCH) {
        flowupd->fwding.update_qos = true;
    }
}

static inline fte::pipeline_action_t
update_host_flow_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    hal_ret_t ret = HAL_RET_OK;
    flow_key_t              flow_key = ctx.get_key();
    bool is_local_dest = is_dst_local_ep(ctx.dif());
    vrf_t  *vrf = vrf_get_infra_vrf();
    hal::pd::pd_vrf_get_lookup_id_args_t vrf_args;
    pd::pd_func_args_t                pd_func_args = {0};

    if (!vrf) {
        return fte::PIPELINE_END;  // Fwding to IPSEC proxy, no other fte featrures needed
    }

    HAL_TRACE_VERBOSE("IPSec Host flow forwarding role: {} direction: {} key {}", ctx.role(), ctx.direction(), ctx.get_key());
    if (
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == hal::FLOW_DIR_FROM_DMA)) ||
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == FLOW_DIR_FROM_UPLINK) && (flow_key.proto != IPPROTO_ESP) && !is_local_dest  && !ctx.cpu_rxhdr()) ||   //temporary - only
          ((ctx.role() ==  hal::FLOW_ROLE_RESPONDER) &&
           (ctx.direction() == FLOW_DIR_FROM_UPLINK) && (flow_key.proto == IPPROTO_ESP))
        ) {

        fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};
        HAL_TRACE_VERBOSE("IPSec updating lport = {}", pfi->proxy->meta->lif_info[0].lport_id);
        HAL_TRACE_VERBOSE("IPsec flow qid1: {} qid2: {}", pfi->qid1, pfi->qid2);

        // update fwding info
        flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
        flowupd.fwding.qid_en = true;
        flowupd.fwding.qtype = 0;
        flowupd.fwding.qid = pfi->qid1;
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
    } else {
        // Set the lookup vrfid
        //fte::flow_update_t flowupd = {type: fte::FLOWUPD_LKP_INFO};
        //flowupd.lkp_info.vrf_hwid = hal::pd::pd_vrf_get_lookup_id(ctx.vrf());
        //ret = ctx.update_flow(flowupd);
        //if (ret != HAL_RET_OK) {
        //    ctx.set_feature_status(ret);
        //    return fte::PIPELINE_END;
        //}
        fte::flow_update_t flowupd = {type: fte::FLOWUPD_LKP_KEY};
        vrf_args.vrf = vrf;
        pd_func_args.pd_vrf_get_lookup_id = &vrf_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FLOW_LKPID, &pd_func_args);
        ctx.set_flow_lkupid(vrf_args.lkup_id);
        flow_key.lkpvrf = vrf_args.lkup_id;
        ctx.set_key(flow_key);
        flowupd.key = flow_key;
        ctx.update_flow(flowupd);
        HAL_TRACE_VERBOSE("IPsec flow_key {}", ctx.get_key());
        HAL_TRACE_VERBOSE("IPsec set_flow_lkup_id {}", vrf_args.lkup_id);
        HAL_TRACE_VERBOSE("IPsec set_use_vrf {}", vrf->vrf_id);
        ctx.set_use_vrf(vrf);
    }
    return  fte::PIPELINE_CONTINUE;
}

static inline fte::pipeline_action_t
update_esp_flow_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    hal_ret_t ret;
    flow_key_t              flow_key = ctx.get_key();
    const char *ipv6_tunnel_sip = "1000:0000:0000:0000:0000:0000:0000:0002";
    vrf_t  *vrf = vrf_get_infra_vrf();

    hal::pd::pd_vrf_get_lookup_id_args_t vrf_args;
    pd::pd_func_args_t                pd_func_args = {0};
    if (!vrf) {
        return fte::PIPELINE_END;  // Fwding to IPSEC proxy, no other fte featrures needed
    }
    HAL_TRACE_VERBOSE("IPSec ESP flow forwarding role: {}  direction: {} key {}", ctx.role(), ctx.direction(), ctx.get_key());
    HAL_TRACE_VERBOSE("IPv6 SIP: {}", ipv6addr2str(flow_key.sip.v6_addr));
    if (flow_key.sip.v4_addr == (0x0a010002) ||
        (strcmp(ipv6_tunnel_sip, ipv6addr2str(flow_key.sip.v6_addr)) == 0)) {
        fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};
        HAL_TRACE_VERBOSE("IPSec updating lport = {}", pfi->proxy->meta->lif_info[0].lport_id);
        HAL_TRACE_VERBOSE("IPsec flow qid1: {} qid2: {}", pfi->qid1, pfi->qid2);

        // update fwding info
        flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
        flowupd.fwding.qid_en = true;
        flowupd.fwding.qtype = 1;
        flowupd.fwding.qid = pfi->qid1;
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
    } else {
        fte::flow_update_t flowupd = {type: fte::FLOWUPD_LKP_KEY};
        vrf_args.vrf = vrf;
        pd_func_args.pd_vrf_get_lookup_id = &vrf_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FLOW_LKPID, &pd_func_args);
        ctx.set_flow_lkupid(vrf_args.lkup_id);
        flow_key.lkpvrf = vrf_args.lkup_id;
        ctx.set_key(flow_key);
        flowupd.key = flow_key;
        ctx.update_flow(flowupd);
        HAL_TRACE_VERBOSE("IPsec flow_key {}", ctx.get_key()); 
        HAL_TRACE_VERBOSE("IPsec set_flow_lkup_id {}", vrf_args.lkup_id);
        HAL_TRACE_VERBOSE("IPsec set_use_vrf {}", vrf->vrf_id);
        ctx.set_use_vrf(vrf);
    }
#if 0
    if (
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == hal::FLOW_DIR_FROM_UPLINK)) ||
          ((ctx.role() ==  hal::FLOW_ROLE_RESPONDER) &&
           (ctx.direction() == hal::FLOW_DIR_FROM_DMA))
        ) {

        fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};
        HAL_TRACE_VERBOSE("IPSec updating lport = {} qid1 {} qid2 {}", pfi->proxy->meta->lif_info[0].lport_id, pfi->qid1, pfi->qid2);
        // update fwding info
        flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
        flowupd.fwding.qid_en = true;
        flowupd.fwding.qtype = 1;
        flowupd.fwding.qid = pfi->qid1;
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
    } else {
        HAL_TRACE_VERBOSE("IPsec set_use_vrf {}", vrf->vrf_id);
        vrf_args.vrf = vrf;
        pd_func_args.pd_vrf_get_lookup_id = &vrf_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FLOW_LKPID, &pd_func_args);
        ctx.set_flow_lkupid(vrf_args.lkup_id);
        HAL_TRACE_VERBOSE("IPsec set_flow_lkup_id {}", vrf_args.lkup_id);
        ctx.set_use_vrf(vrf);
        ctx.set_use_vrf(vrf);
    }
#endif
    return fte::PIPELINE_CONTINUE;
}


static inline
const hal::ipv4_rule_t* ipsec_lookup_rules(vrf_id_t vrf_id, fte::ctx_t&ctx)
{
    const hal::ipv4_rule_t            *rule = NULL;
    const acl::acl_ctx_t              *acl_ctx = NULL;
    hal::ipv4_tuple                    acl_key = {};
    hal_ret_t ret = HAL_RET_OK;
    char ctx_name[ACL_NAMESIZE];

    HAL_TRACE_VERBOSE("vrf_id {} key : {}", vrf_id, ctx.key());

    acl_key.proto = ctx.get_key().proto;
    acl_key.ip_src = ctx.get_key().sip.v4_addr;
    acl_key.ip_dst = ctx.get_key().dip.v4_addr;
    switch ( ctx.get_key().proto) {
    case types::IPPROTO_ICMP:
    case types::IPPROTO_ICMPV6:
        acl_key.icmp_type =  ctx.get_key().icmp_type;
        acl_key.icmp_code =  ctx.get_key().icmp_code;
        break;
    case types::IPPROTO_ESP:
        acl_key.port_src = ctx.get_key().spi >> 16 & 0xFFFF;
        acl_key.port_dst = ctx.get_key().spi & 0xFFFF;
        break;
    case types::IPPROTO_TCP:
    case types::IPPROTO_UDP:
        acl_key.port_src = ctx.get_key().sport;
        acl_key.port_dst = ctx.get_key().dport;
        break;
    default:
        SDK_ASSERT(true);
        ret = HAL_RET_FTE_RULE_NO_MATCH;
    } //switch
    ipsec_acl_ctx_name(ctx_name, vrf_id);
    acl_ctx = acl::acl_get(ctx_name);
    if (acl_ctx != NULL) {
        ret = acl_classify(acl_ctx, (const uint8_t *)&acl_key, (const acl_rule_t **)&rule, 0x01);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_VERBOSE("ipsec::rule lookup failed ret={}", ret);
           return NULL;
        } else {
            HAL_TRACE_VERBOSE("ipsec: rule lookup success for {}", ctx_name);
        }
    }
    if (!rule) {
        //HAL_TRACE_VERBOSE("Rule lookup failed for vrf {}", vrf_id);
    }
    return rule;
}


static inline fte::pipeline_action_t
ipsec_process_initiator_plain_flow(fte::ctx_t&ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    ipsec_cfg_rule_t                  *rule_cfg;
    const hal::ipv4_rule_t            *rule = NULL;
    pd::pd_lif_get_lport_id_args_t    args = { 0 };
    pd::pd_func_args_t                pd_func_args = {0};
    hal::proxy::ipsec_info_t *ipsec_info = NULL;
    lif_t* lif = find_lif_by_id(IPSEC_SVC_LIF);
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    ipsec_info = (ipsec_info_t*) ctx.feature_state();
    rule = ipsec_lookup_rules(ctx.key().dvrf_id, ctx);

    args.pi_lif = lif;
    pd_func_args.pd_lif_get_lport_id = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_GET_LPORTID, &pd_func_args);
    HAL_TRACE_VERBOSE("ipsec: Got lport as {} for lif {}", args.lport_id, IPSEC_ARM_LIF);

    if (rule) {
        acl::ref_t *user_ref = get_rule_data((acl_rule_t *) rule);
        rule_cfg = (ipsec_cfg_rule_t *)RULE_MATCH_USER_DATA(user_ref, ipsec_cfg_rule_t, ref_count);

        rule_ctr_t *rule_ctr = get_rule_ctr((acl_rule_t *)rule);
        rule_ctr->rule_stats->total_hits++;
        switch ( ctx.get_key().proto) {
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
                HAL_TRACE_VERBOSE("Stats: Any proto:{}", ctx.key().proto);
                rule_ctr->rule_stats->other_hits++;

        }
        HAL_TRACE_VERBOSE("ipsec: rule update fwding info: type {}, enc_handle {}, dec_handle{}",
            rule_cfg->action.sa_action, rule_cfg->action.sa_action_enc_handle,
            rule_cfg->action.sa_action_dec_handle);
        if (rule_cfg->action.sa_action == ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_ENCRYPT) {
            HAL_TRACE_VERBOSE("Result type Encrypt with qid {}", rule_cfg->action.sa_action_enc_handle);
            flowupd.fwding.qid = rule_cfg->action.sa_action_enc_handle;
            flowupd.fwding.qtype = 0;
            flowupd.fwding.qid_en = true;
            flowupd.fwding.lport = args.lport_id;
            update_flow_qos(&flowupd);
            HAL_TRACE_VERBOSE("Updating encrypt result qid {}, lport {}", flowupd.fwding.qid, flowupd.fwding.lport);
        }
        ctx.set_l3_tunnel_flow(TRUE);
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        ipsec_info->vrf = ctx.key().dvrf_id;
        ipsec_info->action = IPSEC_PLUGIN_RFLOW_ACTION_DO_NOTHING;
        HAL_TRACE_VERBOSE("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);
        return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
    }
    return fte::PIPELINE_CONTINUE;
}

static inline fte::pipeline_action_t
ipsec_process_post_encrypt_esp_flow(fte::ctx_t&ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    hal::proxy::ipsec_info_t *ipsec_info = NULL;
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_LKP_INFO};
    hal::pd::pd_vrf_get_lookup_id_args_t vrf_args;
    pd::pd_func_args_t                pd_func_args = {0};

    ipsec_info = (ipsec_info_t*) ctx.feature_state();
    ipsec_info->vrf = ctx.key().dvrf_id;
    ipsec_info->action = IPSEC_PLUGIN_RFLOW_ACTION_DECRYPT;

    vrf_t *vrf = vrf_lookup_by_id(ipsec_info->vrf);
    vrf_args.vrf = vrf;
    pd_func_args.pd_vrf_get_lookup_id = &vrf_args;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FLOW_LKPID, &pd_func_args);
    flowupd.lkp_info.vrf_hwid = vrf_args.lkup_id;
    update_flow_qos(&flowupd);
    HAL_TRACE_VERBOSE("Got vrf-hw-id as {:#x}", flowupd.lkp_info.vrf_hwid);
    ret = ctx.update_flow(flowupd);
    ctx.set_feature_status(ret);
    HAL_TRACE_VERBOSE("Return Value {} Reverse Flow params: vrf {} action {}", ret, ipsec_info->vrf, ipsec_info->action);
    return fte::PIPELINE_CONTINUE;
}

static inline fte::pipeline_action_t
ipsec_process_uplink_esp_flow(fte::ctx_t&ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    ipsec_cfg_rule_t                  *rule_cfg;
    const hal::ipv4_rule_t            *rule = NULL;
    pd::pd_lif_get_lport_id_args_t    args = { 0 };
    pd::pd_func_args_t                pd_func_args = {0};
    hal::proxy::ipsec_info_t *ipsec_info = NULL;
    lif_t* lif = find_lif_by_id(IPSEC_SVC_LIF);
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    ipsec_info = (ipsec_info_t*) ctx.feature_state();
    rule = ipsec_lookup_rules(ctx.key().dvrf_id, ctx);

    args.pi_lif = lif;
    pd_func_args.pd_lif_get_lport_id = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_GET_LPORTID, &pd_func_args);
    HAL_TRACE_VERBOSE("ipsec: Got lport as {} for lif {}", args.lport_id, IPSEC_SVC_LIF);

    if (rule) {
        acl::ref_t *user_ref = get_rule_data((acl_rule_t *) rule);
        rule_cfg = (ipsec_cfg_rule_t *)RULE_MATCH_USER_DATA(user_ref, ipsec_cfg_rule_t, ref_count);

        rule_ctr_t *rule_ctr = get_rule_ctr((acl_rule_t *)rule);
        rule_ctr->rule_stats->total_hits++;
        switch ( ctx.get_key().proto) {
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
                HAL_TRACE_VERBOSE("Stats: Any proto:{}", ctx.key().proto);
                rule_ctr->rule_stats->other_hits++;

        }
        HAL_TRACE_VERBOSE("ipsec: rule update fwding info: type {}, enc_handle {}, dec_handle{}",
            rule_cfg->action.sa_action, rule_cfg->action.sa_action_enc_handle,
            rule_cfg->action.sa_action_dec_handle);
        ctx.set_l3_tunnel_flow(TRUE);
        if (rule_cfg->action.sa_action == ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_DECRYPT) {
            HAL_TRACE_VERBOSE("Result type Decrypt with qid {}", rule_cfg->action.sa_action_dec_handle);
            flowupd.fwding.qid = rule_cfg->action.sa_action_dec_handle;
            flowupd.fwding.qtype = 1;
            flowupd.fwding.qid_en = true;
            flowupd.fwding.lport = args.lport_id;
            update_flow_qos(&flowupd);
            HAL_TRACE_VERBOSE("Updating decrypt result qid {}, lport {}", flowupd.fwding.qid, flowupd.fwding.lport);
            ret = ctx.update_flow(flowupd);
            ctx.set_feature_status(ret);
            ipsec_info->vrf = ctx.key().dvrf_id;
            ipsec_info->action = IPSEC_PLUGIN_RFLOW_ACTION_DO_NOTHING;
            HAL_TRACE_VERBOSE("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);
            return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
        }
    }
    return fte::PIPELINE_CONTINUE;
}

static inline fte::pipeline_action_t
ipsec_process_post_decrypt_flow(fte::ctx_t&ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    hal::proxy::ipsec_info_t *ipsec_info = NULL;
    hal::pd::pd_vrf_get_lookup_id_args_t vrf_args;
    pd::pd_func_args_t                pd_func_args = {0};
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_LKP_KEY};
    flow_key_t              flow_key = ctx.get_key();

    ipsec_info = (ipsec_info_t*) ctx.feature_state();
    ipsec_info->vrf = ctx.get_key().dvrf_id;
    ipsec_info->action = IPSEC_PLUGIN_RFLOW_ACTION_ENCRYPT;
    vrf_t *vrf = vrf_lookup_by_id(ipsec_info->vrf);
    vrf_args.vrf = vrf;
    pd_func_args.pd_vrf_get_lookup_id = &vrf_args;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FLOW_LKPID, &pd_func_args);
    flowupd.lkp_info.vrf_hwid = vrf_args.lkup_id;
    update_flow_qos(&flowupd);
    HAL_TRACE_VERBOSE("Got vrf-hw-id as {:#x}", flowupd.lkp_info.vrf_hwid);
    flow_key.lkpvrf = vrf_args.lkup_id;
    ctx.set_key(flow_key);
    flowupd.key = flow_key;
    ctx.set_l3_tunnel_flow(TRUE);
    ret = ctx.update_flow(flowupd);
    ctx.set_feature_status(ret);
    HAL_TRACE_VERBOSE("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);

    return fte::PIPELINE_CONTINUE;
}

static inline fte::pipeline_action_t
ipsec_process_rflow(fte::ctx_t&ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    ipsec_cfg_rule_t                  *rule_cfg;
    const hal::ipv4_rule_t            *rule = NULL;
    pd::pd_lif_get_lport_id_args_t    args = { 0 };
    pd::pd_func_args_t                pd_func_args = {0};
    hal::proxy::ipsec_info_t *ipsec_info = NULL;
    lif_t* lif = find_lif_by_id(IPSEC_SVC_LIF);
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    ipsec_info = (ipsec_info_t*) ctx.feature_state();

    args.pi_lif = lif;
    pd_func_args.pd_lif_get_lport_id = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_GET_LPORTID, &pd_func_args);
    HAL_TRACE_VERBOSE("ipsec: Got lport as {} for lif {}", args.lport_id, IPSEC_SVC_LIF);

    hal::pd::pd_vrf_get_lookup_id_args_t vrf_args;
    ipsec_info = (ipsec_info_t*) ctx.feature_state();
    vrf_t *vrf = vrf_lookup_by_id(ipsec_info->vrf);
    HAL_TRACE_VERBOSE("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);

    if (ipsec_info->action == IPSEC_PLUGIN_RFLOW_ACTION_DO_NOTHING) {
        flowupd = {type: fte::FLOWUPD_LKP_INFO};
        vrf_args.vrf = vrf;
        pd_func_args.pd_vrf_get_lookup_id = &vrf_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FLOW_LKPID, &pd_func_args);
        flowupd.lkp_info.vrf_hwid = vrf_args.lkup_id;
        update_flow_qos(&flowupd);
        HAL_TRACE_VERBOSE("Got vrf-hw-id as {:#x}", flowupd.lkp_info.vrf_hwid);
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        return fte::PIPELINE_CONTINUE;
    } else {
        //rule = ipsec_lookup_rules(ipsec_info->vrf, ctx);
        rule = ipsec_lookup_rules(ctx.get_key().dvrf_id, ctx);
        if (rule) {
            acl::ref_t *user_ref = get_rule_data((acl_rule_t *) rule);
            rule_cfg = (ipsec_cfg_rule_t *)RULE_MATCH_USER_DATA(user_ref, ipsec_cfg_rule_t, ref_count);

            rule_ctr_t *rule_ctr = get_rule_ctr((acl_rule_t *)rule);
            rule_ctr->rule_stats->total_hits++;
            switch ( ctx.get_key().proto) {
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
                HAL_TRACE_VERBOSE("Stats: Any proto:{}", ctx.key().proto);
                rule_ctr->rule_stats->other_hits++;

            }
            flowupd = {type: fte::FLOWUPD_FWDING_INFO};
            HAL_TRACE_VERBOSE("ipsec: rule update fwding info: type {}, enc_handle {}, dec_handle{}",
                rule_cfg->action.sa_action, rule_cfg->action.sa_action_enc_handle,
                rule_cfg->action.sa_action_dec_handle);
            if (rule_cfg->action.sa_action == ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_DECRYPT) {
                HAL_TRACE_VERBOSE("Result type Decrypt with qid {}", rule_cfg->action.sa_action_dec_handle);
                flowupd.fwding.qid = rule_cfg->action.sa_action_dec_handle;
                flowupd.fwding.qtype = 1;
            } else if (rule_cfg->action.sa_action == ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_ENCRYPT) {
                HAL_TRACE_VERBOSE("Result type Encrypt with qid {}", rule_cfg->action.sa_action_enc_handle);
                flowupd.fwding.qid = rule_cfg->action.sa_action_enc_handle;
                flowupd.fwding.qtype = 0;
            }
            ctx.set_l3_tunnel_flow(TRUE);
            flowupd.fwding.qid_en = true;
            flowupd.fwding.lport = args.lport_id;
            update_flow_qos(&flowupd);
            ret = ctx.update_flow(flowupd);
            HAL_TRACE_VERBOSE("fwding info: qid_en {} qid {} qtype {} lport {}",
                            flowupd.fwding.qid_en, flowupd.fwding.qid, flowupd.fwding.qtype, flowupd.fwding.lport);
            ctx.set_feature_status(ret);
            return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
        }
    }
    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
ipsec_exec_pkt(fte::ctx_t&ctx)
{
    if (!ctx.cpu_rxhdr()) {
        return fte::PIPELINE_CONTINUE;
    }
    if ((ctx.cpu_rxhdr()->flags & CPU_FLAGS_IPV6_VALID) == CPU_FLAGS_IPV6_VALID) {
        return fte::PIPELINE_CONTINUE;
    }

    HAL_TRACE_VERBOSE("IPSec flow forwarding role: {} direction: {} key {} src_lif {}",
        ctx.role(), ctx.direction(), ctx.get_key(), ctx.cpu_rxhdr()->src_lif);

    if (ctx.role() ==  hal::FLOW_ROLE_INITIATOR) {
        // Un-Encrypted traffic from uplink or ENIC
        if ((ctx.get_key().proto != IP_PROTO_IPSEC_ESP) &&
            (ctx.cpu_rxhdr()->src_lif != IPSEC_ARM_LIF)) {
            return (ipsec_process_initiator_plain_flow(ctx));
        } // Un-Encrypted traffic from uplink or ENIC
        // Post Encrypted ESP packet from Barco
        if ((ctx.get_key().proto == IP_PROTO_IPSEC_ESP) &&
            (ctx.cpu_rxhdr()->src_lif == IPSEC_ARM_LIF)) {
            return (ipsec_process_post_encrypt_esp_flow(ctx));
        } // Post Encrypted ESP packet from Barco
        // Encrypted ESP packet from Uplink
        if ((ctx.get_key().proto == IP_PROTO_IPSEC_ESP) &&
            (ctx.cpu_rxhdr()->src_lif != IPSEC_ARM_LIF)) {
            return (ipsec_process_uplink_esp_flow(ctx));
        } // Encrypted ESP packet from Uplink
        // Decrypted packeted from Barco
        if ((ctx.get_key().proto != IP_PROTO_IPSEC_ESP) &&
            (ctx.cpu_rxhdr()->src_app_id ==  P4PLUS_APPTYPE_IPSEC) &&
            (ctx.cpu_rxhdr()->src_lif == IPSEC_ARM_LIF)) {
            return ipsec_process_post_decrypt_flow(ctx);
        } // Post Encrypted ESP packet from Barco
    } else if (ctx.role() ==  hal::FLOW_ROLE_RESPONDER) {
        return (ipsec_process_rflow(ctx));
    }
    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
ipsec_exec(fte::ctx_t& ctx)
{
    proxy_flow_info_t       *pfi = NULL, *tpfi = NULL;
    flow_key_t              flow_key = ctx.get_key();

    // TODO(goli) - this is to workaround ipsec issue, currently the encrupted pkt
    // is sent in the reverse direction for decryption without swapping the sip to dip.
    // For this pkt lport shouldn't be the ipsec proxy-lif.
    if (ctx.existing_session()) {
        HAL_TRACE_VERBOSE("Existing flow came in");
        return ipsec_exec_pkt(ctx);
    }

    // Ignore direction. Always set it to 0
    flow_key.dir = 0;

    // get the flow info for the tcp proxy service
    pfi = proxy_get_flow_info(types::PROXY_TYPE_IPSEC,
                              flow_key);

    if(pfi) {
        if(flow_key.proto == IPPROTO_ESP) {
            HAL_TRACE_VERBOSE("ipsec ESP flow");
            return update_esp_flow_fwding_info(ctx, pfi);
        } else {
            HAL_TRACE_VERBOSE("ipsec HOST flow");
            // Get Tunnel PFI
            tpfi = proxy_get_flow_info(types::PROXY_TYPE_IPSEC,
                                       (pfi->u.ipsec.u.host_flow.esp_flow_key));
            if(!tpfi) {
                //HAL_TRACE_ERR("Failed to get tunnel info for host flow: {}", flow_key);
                //ctx.set_feature_status(HAL_RET_FLOW_NOT_FOUND);
                //return fte::PIPELINE_END;
            }
            // Update flow
            return update_host_flow_fwding_info(ctx, pfi);
        }
    } else {
        return ipsec_exec_pkt(ctx);
    }
    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
