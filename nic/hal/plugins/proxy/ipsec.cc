//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/src/ipsec/ipsec.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/nw/vrf.hpp"

namespace hal {
namespace proxy {

static bool
is_dst_local_ep(hal::if_t *intf) {
    return (intf != NULL && intf->if_type == intf::IF_TYPE_ENIC);
}

static inline fte::pipeline_action_t
update_host_flow_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    hal_ret_t ret = HAL_RET_OK;
    flow_key_t              flow_key = ctx.get_key();
    bool is_local_dest = is_dst_local_ep(ctx.dif());

    HAL_TRACE_DEBUG("IPSec Host flow forwarding role: {} direction: {} key {}", ctx.role(), ctx.direction(), ctx.get_key());
    if (
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == hal::FLOW_DIR_FROM_DMA)) ||
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == FLOW_DIR_FROM_UPLINK) && (flow_key.proto != IPPROTO_ESP) && !is_local_dest ) ||   //temporary - only
          ((ctx.role() ==  hal::FLOW_ROLE_RESPONDER) &&
           (ctx.direction() == FLOW_DIR_FROM_UPLINK) && (flow_key.proto == IPPROTO_ESP))
        ) {

        fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};
        HAL_TRACE_DEBUG("IPSec updating lport = {}", pfi->proxy->meta->lif_info[0].lport_id);
        HAL_TRACE_DEBUG("IPsec flow qid1: {} qid2: {}", pfi->qid1, pfi->qid2);

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
    }

    return  fte::PIPELINE_CONTINUE;
}

static inline fte::pipeline_action_t
update_esp_flow_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    hal_ret_t ret;
    HAL_TRACE_DEBUG("IPSec ESP flow forwarding role: {}  direction: {} key {}", ctx.role(), ctx.direction(), ctx.get_key());
    if (
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == hal::FLOW_DIR_FROM_UPLINK)) ||
          ((ctx.role() ==  hal::FLOW_ROLE_RESPONDER) &&
           (ctx.direction() == hal::FLOW_DIR_FROM_DMA))
        ) {

        fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};
        HAL_TRACE_DEBUG("IPSec updating lport = {} qid1 {} qid2 {}", pfi->proxy->meta->lif_info[0].lport_id, pfi->qid1, pfi->qid2);
        // update fwding info
        flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
        flowupd.fwding.qid_en = true;
        flowupd.fwding.qtype = 1;
        flowupd.fwding.qid = pfi->qid1;
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
    }
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

    HAL_TRACE_DEBUG("vrf_id {} key : {}", vrf_id, ctx.key());

    acl_key.proto = ctx.get_key().proto;
    acl_key.ip_src = ctx.get_key().sip.v4_addr;
    acl_key.ip_dst = ctx.get_key().dip.v4_addr;
    switch ( ctx.get_key().proto) {
    case types::IPPROTO_ICMP:
    case types::IPPROTO_ICMPV6:
        acl_key.port_src =  ctx.get_key().icmp_id;
        acl_key.port_dst = ((ctx.get_key().icmp_type << 8) |  ctx.get_key().icmp_code);
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
        HAL_ASSERT(true);
        ret = HAL_RET_FTE_RULE_NO_MATCH;
    } //switch
    ipsec_acl_ctx_name(ctx_name, vrf_id);
    acl_ctx = acl::acl_get(ctx_name);
    if (acl_ctx != NULL) {
        ret = acl_classify(acl_ctx, (const uint8_t *)&acl_key, (const acl_rule_t **)&rule, 0x01);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("ipsec::rule lookup failed ret={}", ret);
           return NULL;
        } else {
            HAL_TRACE_DEBUG("ipsec: rule lookup success for {}", ctx_name);
        }
    }
    if (!rule) {
        HAL_TRACE_DEBUG("Rule lookup failed for vrf {}", vrf_id);
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
    HAL_TRACE_DEBUG("ipsec: Got lport as {} for lif {}", args.lport_id, 1004);

    if (rule) {
        acl::ref_t *rc;
        rc = (acl::ref_t *) rule->data.userdata;
        rule_cfg = RULE_MATCH_USER_DATA(rc, ipsec_cfg_rule_t, ref_count);
        HAL_TRACE_DEBUG("ipsec: rule update fwding info: type {}, enc_handle {}, dec_handle{}", 
            rule_cfg->action.sa_action, rule_cfg->action.sa_action_enc_handle, 
            rule_cfg->action.sa_action_dec_handle);
        if (rule_cfg->action.sa_action == ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_ENCRYPT) {
            HAL_TRACE_DEBUG("Result type Encrypt with qid {}", rule_cfg->action.sa_action_enc_handle);
            flowupd.fwding.qid = rule_cfg->action.sa_action_enc_handle;
            flowupd.fwding.qtype = 0;
            flowupd.fwding.qid_en = true;
            flowupd.fwding.lport = args.lport_id; 
            HAL_TRACE_DEBUG("Updating encrypt result qid {}, lport {}", flowupd.fwding.qid, flowupd.fwding.lport);
        }
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        ipsec_info->vrf = ctx.key().dvrf_id; 
        ipsec_info->action = IPSEC_PLUGIN_RFLOW_ACTION_DO_NOTHING;
        HAL_TRACE_DEBUG("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);
        return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
    } else {
        HAL_TRACE_DEBUG("Rule lookup failed for vrf {}", ctx.key().dvrf_id);
    }
    return fte::PIPELINE_CONTINUE; 
}

static inline fte::pipeline_action_t
ipsec_process_post_encrypt_esp_flow(fte::ctx_t&ctx)
{
    hal::proxy::ipsec_info_t *ipsec_info = NULL;

    ipsec_info = (ipsec_info_t*) ctx.feature_state();
    ipsec_info->vrf = ctx.key().dvrf_id; 
    ipsec_info->action = IPSEC_PLUGIN_RFLOW_ACTION_DECRYPT;
   
    HAL_TRACE_DEBUG("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);
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
    HAL_TRACE_DEBUG("ipsec: Got lport as {} for lif {}", args.lport_id, IPSEC_SVC_LIF);

    if (rule) {
        acl::ref_t *rc;
        rc = (acl::ref_t *) rule->data.userdata;
        rule_cfg = RULE_MATCH_USER_DATA(rc, ipsec_cfg_rule_t, ref_count);
        HAL_TRACE_DEBUG("ipsec: rule update fwding info: type {}, enc_handle {}, dec_handle{}", 
            rule_cfg->action.sa_action, rule_cfg->action.sa_action_enc_handle, 
            rule_cfg->action.sa_action_dec_handle);
        if (rule_cfg->action.sa_action == ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_DECRYPT) {
            HAL_TRACE_DEBUG("Result type Decrypt with qid {}", rule_cfg->action.sa_action_dec_handle);
            flowupd.fwding.qid = rule_cfg->action.sa_action_dec_handle;
            flowupd.fwding.qtype = 1;
            flowupd.fwding.qid_en = true;
            flowupd.fwding.lport = args.lport_id; 
            HAL_TRACE_DEBUG("Updating decrypt result qid {}, lport {}", flowupd.fwding.qid, flowupd.fwding.lport);
            ret = ctx.update_flow(flowupd);
            ctx.set_feature_status(ret);
            ipsec_info->vrf = ctx.key().dvrf_id; 
            ipsec_info->action = IPSEC_PLUGIN_RFLOW_ACTION_DO_NOTHING;
            HAL_TRACE_DEBUG("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);
            return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
        }
    } else {
        HAL_TRACE_DEBUG("Rule lookup failed for vrf {}", ctx.key().dvrf_id);
    }
    return fte::PIPELINE_CONTINUE; 
}
  

static inline fte::pipeline_action_t
ipsec_process_post_decrypt_flow(fte::ctx_t&ctx)
{
    hal::proxy::ipsec_info_t *ipsec_info = NULL;

    ipsec_info = (ipsec_info_t*) ctx.feature_state();
    ipsec_info->vrf = ctx.get_key().dvrf_id; 
    ipsec_info->action = IPSEC_PLUGIN_RFLOW_ACTION_ENCRYPT;
    HAL_TRACE_DEBUG("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);

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
    HAL_TRACE_DEBUG("ipsec: Got lport as {} for lif {}", args.lport_id, IPSEC_SVC_LIF);

    hal::pd::pd_vrf_get_lookup_id_args_t vrf_args;
    ipsec_info = (ipsec_info_t*) ctx.feature_state();
    vrf_t *vrf = vrf_lookup_by_id(ipsec_info->vrf); 
    HAL_TRACE_DEBUG("Reverse Flow params: vrf {} action {}", ipsec_info->vrf, ipsec_info->action);

    if (ipsec_info->action == IPSEC_PLUGIN_RFLOW_ACTION_DO_NOTHING) {
        flowupd = {type: fte::FLOWUPD_LKP_INFO};
        vrf_args.vrf = vrf;
        pd_func_args.pd_vrf_get_lookup_id = &vrf_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_VRF_GET_FLOW_LKPID, &pd_func_args);
        flowupd.lkp_info.vrf_hwid = vrf_args.lkup_id;
        HAL_TRACE_DEBUG("Got vrf-hw-id as {}", flowupd.lkp_info.vrf_hwid);
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        return fte::PIPELINE_CONTINUE;
    } else {
        //rule = ipsec_lookup_rules(ipsec_info->vrf, ctx);
        rule = ipsec_lookup_rules(ctx.get_key().dvrf_id, ctx);
        if (rule) {
            acl::ref_t *rc;
            rc = (acl::ref_t *) rule->data.userdata;
            rule_cfg = RULE_MATCH_USER_DATA(rc, ipsec_cfg_rule_t, ref_count);
            flowupd = {type: fte::FLOWUPD_FWDING_INFO};
            HAL_TRACE_DEBUG("ipsec: rule update fwding info: type {}, enc_handle {}, dec_handle{}", 
                rule_cfg->action.sa_action, rule_cfg->action.sa_action_enc_handle, 
                rule_cfg->action.sa_action_dec_handle);
            if (rule_cfg->action.sa_action == ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_DECRYPT) {
                HAL_TRACE_DEBUG("Result type Decrypt with qid {}", rule_cfg->action.sa_action_dec_handle);
                flowupd.fwding.qid = rule_cfg->action.sa_action_dec_handle;
                flowupd.fwding.qtype = 1;
            } else if (rule_cfg->action.sa_action == ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_ENCRYPT) {
                HAL_TRACE_DEBUG("Result type Encrypt with qid {}", rule_cfg->action.sa_action_enc_handle);
                flowupd.fwding.qid = rule_cfg->action.sa_action_enc_handle;
                flowupd.fwding.qtype = 0;
            }
            flowupd.fwding.qid_en = true;
            flowupd.fwding.lport = args.lport_id; 
            ret = ctx.update_flow(flowupd);
            HAL_TRACE_DEBUG("fwding info: qid_en {} qid {} qtype {} lport {}", 
                            flowupd.fwding.qid_en, flowupd.fwding.qid, flowupd.fwding.qtype, flowupd.fwding.lport);
            ctx.set_feature_status(ret);
            return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
        } else {
            HAL_TRACE_DEBUG("Rule lookup failed for vrf {}", ipsec_info->vrf);
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

    HAL_TRACE_DEBUG("IPSec flow forwarding role: {} direction: {} key {} src_lif {}", 
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
        return fte::PIPELINE_CONTINUE;
    }

    // Ignore direction. Always set it to 0
    flow_key.dir = 0;

    // get the flow info for the tcp proxy service
    pfi = proxy_get_flow_info(types::PROXY_TYPE_IPSEC,
                              &flow_key);

    if(pfi) {
        if(flow_key.proto == IPPROTO_ESP) {
            HAL_TRACE_DEBUG("ipsec ESP flow");
            return update_esp_flow_fwding_info(ctx, pfi);
        } else {
            HAL_TRACE_DEBUG("ipsec HOST flow");
            // Get Tunnel PFI
            tpfi = proxy_get_flow_info(types::PROXY_TYPE_IPSEC,
                                       &(pfi->u.ipsec.u.host_flow.esp_flow_key));
            if(!tpfi) {
                HAL_TRACE_ERR("Failed to get tunnel info for host flow: {}", flow_key);
                //ctx.set_feature_status(HAL_RET_FLOW_NOT_FOUND);
                //return fte::PIPELINE_END;
            }
            // Update flow
            return update_host_flow_fwding_info(ctx, pfi);
        }
    } else {
        HAL_TRACE_DEBUG("ipsec is not enabled for the flow");
        return ipsec_exec_pkt(ctx);
    }
    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
