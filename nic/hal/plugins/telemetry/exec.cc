//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/telemetry/telemetry.hpp"

namespace hal {
namespace plugins {
namespace telemetry {

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
    
    memset(&mirror_flowupd, 0, sizeof(fte::flow_update_t));
    memset(&export_flowupd, 0, sizeof(fte::flow_update_t));
    mirror_flowupd.mirror_info.mirror_en = 0;
    export_flowupd.export_info.export_en = 0;
    const char *ctx_name = flowmon_acl_ctx_name(ctx.get_key().svrf_id, mirror_action);
    acl_ctx = acl::acl_get(ctx_name);
    HAL_TRACE_DEBUG("ctx_name: {} acl_ctx: {:#x}", ctx_name, (uint64_t) acl_ctx);
    if (acl_ctx == NULL) {
        HAL_TRACE_DEBUG("telemetry::No telemetry acl_ctx for vrf {} id {} mirror_action {}",
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
        HAL_TRACE_DEBUG("telemetry::Invalid flow type {}", ctx.get_key().flow_type);
        ret = HAL_RET_OK;
        goto end;
    }
    if (ctx.get_key().flow_type != FLOW_TYPE_L2){
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
            goto end;
        }
    }

    ret = acl_classify(acl_ctx, (const uint8_t *)&acl_key, (const acl_rule_t **)&rule, 0x01);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("telemetry::rule lookup failed ret={}", ret);
        goto end;
    }

    if (rule) {
        acl::ref_t *rc;
        rc = get_rule_data((acl_rule_t *)rule);
        frule = (const hal::flow_monitor_rule_t *) RULE_MATCH_USER_DATA(rc, hal::flow_monitor_rule_t, ref_count);
        if (frule->action.num_mirror_dest > 0) {
            mirror_flowupd.type = fte::FLOWUPD_MIRROR_INFO;
            mirror_flowupd.mirror_info.mirror_en = true;
            mirror_flowupd.mirror_info.ing_mirror_session = 0;
            for (int i = 0; i < frule->action.num_mirror_dest; i++) {
                mirror_flowupd.mirror_info.ing_mirror_session |= (1 << frule->action.mirror_destinations[i]);
            }
        }
        if (frule->action.num_collector > 0) {
            export_flowupd.type = fte::FLOWUPD_EXPORT_INFO;
            int n = frule->action.num_collector;
            export_flowupd.export_info.export_en = 0;
            if (n >= 1) {
                export_flowupd.export_info.export_en |= (1 << 0);
                export_flowupd.export_info.export_id1 = frule->action.collectors[0];
            }
            if (n >= 2) {
                export_flowupd.export_info.export_en |= (1 << 1);
                export_flowupd.export_info.export_id2 = frule->action.collectors[1];
            }
            if (n >= 3) {
                export_flowupd.export_info.export_en |= (1 << 2);
                export_flowupd.export_info.export_id3 = frule->action.collectors[2];
            }
            if (n == 4) {
                export_flowupd.export_info.export_en |= (1 << 3);
                export_flowupd.export_info.export_id4 = frule->action.collectors[3];
            }
        }
        if (mirror_flowupd.mirror_info.mirror_en) {
            ret = ctx.update_flow(mirror_flowupd);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error updating mirror info");
                return ret;
            }
        }
        if (export_flowupd.export_info.export_en) {
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
    HAL_TRACE_DEBUG("continue to process telemetry");
    return ret;
}

/*
 * telemetry_exec
 *   Entry point into the TELEMETRY feature
 */
fte::pipeline_action_t
telemetry_exec (fte::ctx_t &ctx)
{
    hal_ret_t ret;
    
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
