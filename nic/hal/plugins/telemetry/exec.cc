//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils/if_utils.hpp"

namespace hal {
namespace plugins {
namespace telemetry {

using namespace hal::plugins::sfw;
/*
 * update_flow
 *   Update flow from TELEMETRY policy
 */
static inline hal_ret_t
update_flow_from_telemetry_rules (fte::ctx_t& ctx, bool mirror_action)
{
    fte::flow_update_t                  mirror_flowupd;
    fte::flow_update_t                  export_flowupd;
    hal::ipv4_tuple                     acl_key = {};
    hal_ret_t                           ret = HAL_RET_OK;
    const hal::flow_monitor_rule_t      *frule = NULL;
    const hal::ipv4_rule_t              *rule = NULL;
    const acl::acl_ctx_t                *acl_ctx = NULL;

    mirror_flowupd.mirror_info.mirror_en = 0;
    export_flowupd.export_info.export_en = 0;
    const char *ctx_name = flowmon_acl_ctx_name(ctx.get_key().svrf_id, mirror_action);
    acl_ctx = acl::acl_get(ctx_name);
    HAL_TRACE_VERBOSE("ctx_name: {} acl_ctx: {:#x}", ctx_name, (uint64_t) acl_ctx);
    if (acl_ctx == NULL) {
        HAL_TRACE_DEBUG("No telemetry policy for vrf {} id {} mirror_action {}",
                         ctx_name, ctx.get_key().svrf_id, mirror_action);
        ret = HAL_RET_OK;
        goto end;
    }
    if (ctx.get_key().flow_type == FLOW_TYPE_V4) {
        acl_key.proto = ctx.get_key().proto;
        acl_key.ip_src = ctx.get_key().sip.v4_addr;
        acl_key.ip_dst = ctx.get_key().dip.v4_addr;
    } else if (ctx.get_key().flow_type == FLOW_TYPE_L2){
        acl_key.ethertype = ctx.get_key().ether_type;
        acl_key.mac_src = 0;
        acl_key.mac_dst = 0;
        for (int i = 0; i < ETH_ADDR_LEN; i++) {
            acl_key.mac_src |= ctx.get_key().smac[i] << (i  * 8);
            acl_key.mac_dst |= ctx.get_key().dmac[i] << (i * 8);
        }
    } else {
        HAL_TRACE_DEBUG("Invalid flow type {}", ctx.get_key().flow_type);
        ret = HAL_RET_OK;
        goto end;
    }
    if (ctx.get_key().flow_type != FLOW_TYPE_L2){
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
            goto end;
        }
    }

    ret = acl_classify(acl_ctx, (const uint8_t *)&acl_key, (const acl_rule_t **)&rule, 0x01);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("telemetry policy didn't match ret={}", ret);
        goto end;
    }

    if (rule) {
        acl::ref_t *rc;
        rc = get_rule_data((acl_rule_t *)rule);
        frule = (const hal::flow_monitor_rule_t *) RULE_MATCH_USER_DATA(rc, hal::flow_monitor_rule_t, ref_count);

        if (mirror_action) {
            memset(&mirror_flowupd, 0, sizeof(fte::flow_update_t));
            mirror_flowupd.type = fte::FLOWUPD_MIRROR_INFO;
            if (frule->action.num_mirror_dest > 0) {
                mirror_flowupd.mirror_info.mirror_en = true;
                mirror_flowupd.mirror_info.ing_mirror_session = 0;
                mirror_flowupd.mirror_info.egr_mirror_session = 0;
                for (int i = 0; i < frule->action.num_mirror_dest; i++) {
                    //
                    // We always want to send user-vlan while mirroring. so pick
                    // the mirroring as ingress or egress depending on the
                    // flow direction. There is a bit of a cost to do the recirc
                    // for egress in P4. TBD: Move to user configurable option
                    //
                    if (ctx.flow_direction() == hal::FLOW_DIR_FROM_DMA) { 
                        mirror_flowupd.mirror_info.egr_mirror_session |= 
                        (1 << frule->action.mirror_destinations[i]);
                    } else {
                        mirror_flowupd.mirror_info.ing_mirror_session |= 
                        (1 << frule->action.mirror_destinations[i]);
                    }
                }
            }

            ret = ctx.update_flow(mirror_flowupd);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error updating mirror info");
                return ret;
            }
        }
        else {
            memset(&export_flowupd, 0, sizeof(fte::flow_update_t));
            export_flowupd.type = fte::FLOWUPD_EXPORT_INFO;
            if (frule->action.num_collector > 0) {
                int n = frule->action.num_collector;
                export_flowupd.export_info.export_en = 0;
                if (n >= 1) {
                    export_flowupd.export_info.export_en |= (1 << 0);
                    export_flowupd.export_info.export_id1 = 
                    frule->action.collectors[0];
                }
                if (n >= 2) {
                    export_flowupd.export_info.export_en |= (1 << 1);
                    export_flowupd.export_info.export_id2 = 
                    frule->action.collectors[1];
                }
                if (n >= 3) {
                    export_flowupd.export_info.export_en |= (1 << 2);
                    export_flowupd.export_info.export_id3 = 
                    frule->action.collectors[2];
                }
                if (n == 4) {
                    export_flowupd.export_info.export_en |= (1 << 3);
                    export_flowupd.export_info.export_id4 = 
                    frule->action.collectors[3];
                }
            }

            ret = ctx.update_flow(export_flowupd);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error updating export info");
                return ret;
            }
        }
    } else {
        HAL_TRACE_DEBUG("Flow did not match any telemetry rules!");
    }

end:

    if (acl_ctx) {
        acl::acl_deref(acl_ctx);
    }
    return ret;
}

static hal_ret_t
telemetry_pick_dest_if (fte::ctx_t &ctx)
{
    hal_ret_t           ret = HAL_RET_OK;
    fte::flow_update_t  flowupd;
    if_t                *ndif = NULL;

    if_t *dif = ctx.dif();
    if (!dif) {
        return HAL_RET_OK;
    }
    // Pick an active uplink
    if ((dif->if_type == intf::IF_TYPE_UPLINK) &&
            (dif->if_op_status == intf::IF_STATUS_DOWN)) {
        HAL_TRACE_DEBUG("Pinned uplink id {} is down", dif->if_id);
        // Pinned uplink is down, pick a new active uplink
        if (ctx.pipeline_event() == fte::FTE_SESSION_UPDATE) {
            ndif = telemetry_get_active_uplink();
        } else {
            ndif = telemetry_get_active_bond_uplink();
        }
        if (!ndif) {
            return HAL_RET_OK;
        }
        if (dif != ndif) {
            HAL_TRACE_DEBUG("Picked new uplink id {}", ndif->if_id);
            memset(&flowupd, 0, sizeof(fte::flow_update_t));
            flowupd.type = fte::FLOWUPD_FWDING_INFO;
            flowupd.fwding.dif = ndif;
            ret = ctx.update_flow(flowupd);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error updating the dif");
                return ret;
            }
            // Update the EP if ptr
            ep_t *dep = ctx.dep();
            if (dep) {
                // Update EP's if handle
                dep->if_handle = ndif->hal_handle;
                // Update the if to ep backptr also
                if_del_ep(dif, dep);
                if_add_ep(ndif, dep);
            }
        }
    }

    return ret;
}

fte::pipeline_action_t
telemetry_exec_delete (fte::ctx_t &ctx)
{
    hal::session_t *session = ctx.session();
    
    HAL_TRACE_DEBUG("Delete for telemetry plugin, event {}",
                                        ctx.pipeline_event());
    if (session && session->is_ipfix_flow) {
        hal::session_t *ls = (session_t *)
            g_hal_state->session_hal_telemetry_ht()->lookup(&session->hal_handle);
        if (ls) {
            HAL_TRACE_DEBUG("Found IPFIX session {} Deleting from HT",
                                               session->hal_handle);
            g_hal_state->session_hal_telemetry_ht()->remove_entry(session,
                                               &session->hal_telemetry_ht_ctxt);
        }
        ctx.set_is_ipfix_flow(true);
    }

    return fte::PIPELINE_CONTINUE;
}

static void
telemetry_completion_hdlr (fte::ctx_t& ctx, bool status)
{
    hal::session_t *session = ctx.session();
    
    // Insert IPFIX sessions to telemetry db
    if (session && ctx.is_ipfix_flow()) {
        hal::session_t *ls = (session_t *)
            g_hal_state->session_hal_telemetry_ht()->lookup(&session->hal_handle);
        if (!ls) {
            session->hal_telemetry_ht_ctxt.reset();
            HAL_TRACE_DEBUG("Inserting IPFIX session {} into HT",
                                               session->hal_handle);
            g_hal_state->session_hal_telemetry_ht()->insert(session,
                                               &session->hal_telemetry_ht_ctxt);
            session->is_ipfix_flow = true;
        }
    }
}


/*
 * telemetry_exec
 *   Entry point into the TELEMETRY feature
 */
fte::pipeline_action_t
telemetry_exec (fte::ctx_t &ctx)
{
    hal_ret_t   ret = HAL_RET_OK;
    sfw_info_t  *sfw_info = NULL;
    fte::flow_update_t  ipfix_upd = {type: fte::FLOWUPD_ACTION};

    // For updates process only if we need to
    if (ctx.pipeline_event() == fte::FTE_SESSION_UPDATE &&
        !(ctx.featureid_bitmap() & (1 << ctx.get_feature_id())))
        return fte::PIPELINE_CONTINUE;  
    
    hal::session_t *session = ctx.session();
    HAL_TRACE_VERBOSE("telemetry plugin exec, event {}", ctx.pipeline_event());
    // IPFIX flows
    if ((session && session->is_ipfix_flow) ||
            (ctx.cpu_rxhdr() && (ctx.cpu_rxhdr()->src_lif == HAL_LIF_CPU) &&
            (ctx.cpu_rxhdr()->src_app_id == P4PLUS_APPTYPE_TELEMETRY))) {
        // If its a flow-miss for IPFIX we need to make sure the VLAN tag
        // on the packet is the same as the from cpu vlan id of the collector
        // With PS-1888 we noticed that the incoming vlan tag was not the same
        // as the collectors from-cpu vlan id. It is not conclusive but there is
        // a possibility that we did a delete-create on the l2seg/collector after
        // P4 queued the flow-miss. We could end up with memhash mismatch leading to
        // asserts hence the check.
        if (!session && ctx.cpu_rxhdr() &&
            (ctx.cpu_rxhdr()->flags&CPU_FLAGS_VLAN_VALID) && ctx.dl2seg()) {
            hal::pd::pd_func_args_t  pd_func_args = {0};
            hal::pd::pd_l2seg_get_fromcpu_vlanid_args_t args;
            uint16_t                 fromcpu_tag = 0;

            vlan_header_t *vlan = (vlan_header_t*)(ctx.pkt() + ctx.cpu_rxhdr()->l2_offset);

            args.l2seg = ctx.dl2seg();
            args.vid = &fromcpu_tag;

            pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
            if ((hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                 &pd_func_args) == HAL_RET_OK) && (fromcpu_tag != ntohs(vlan->vlan_tag))) {
                 fte::flow_update_t flowupd = {type: fte::FLOWUPD_ACTION};
                 flowupd.action = session::FLOW_ACTION_DROP;
                 HAL_TRACE_ERR("IPFIX from-cpu vlan mismatch fromcpu_tag: {} vlanhdr tag: {}", 
                               fromcpu_tag, vlan->vlan_tag);
                 ctx.update_flow(flowupd);
                 return fte::PIPELINE_CONTINUE;
            }
        }
        HAL_TRACE_DEBUG("Processing IPFIX flow");
        sfw_info = sfw::sfw_feature_state(ctx);
        // IPFIX flows are always allowed
        sfw_info->skip_sfw = true;
        ipfix_upd.action = session::FLOW_ACTION_ALLOW;

        /* Skip rflow for IPFIX pkts */
        ctx.set_valid_rflow(false);
        ctx.set_is_ipfix_flow(true);
        ctx.register_completion_handler(telemetry_completion_hdlr);
        telemetry_pick_dest_if(ctx);
        ret = ctx.update_flow(ipfix_upd);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error updating flow policy action!");
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END;
        }
        // No need to evaluate telemetry rules for IPFIX flows
        return fte::PIPELINE_CONTINUE;
    }

    /* Iflow and Rflow are independently evaluated */
    /* Update mirror rules */
    ret = update_flow_from_telemetry_rules(ctx, true);
    /* Update flowmon rules */
    ret = update_flow_from_telemetry_rules(ctx, false);
    
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace telemetry
} // namespace plugins
} // namespace hal
