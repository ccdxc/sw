//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/ndp_learn.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_learn.hpp"

namespace hal {
namespace eplearn {

static bool
is_arp_ep_learning_enabled(fte::ctx_t &ctx)
{
    hal::l2seg_t *src_l2seg = ctx.sl2seg();

    if (src_l2seg != nullptr && src_l2seg->eplearn_cfg.arp_cfg.enabled) {
        return true;
    }
    return false;
}

static bool
is_dhcp_ep_learning_enabled(fte::ctx_t &ctx)
{
    hal::l2seg_t *src_l2seg = ctx.sl2seg();

    if (src_l2seg != nullptr && src_l2seg->eplearn_cfg.dhcp_cfg.enabled) {
        return true;
    }

    return false;
}

fte::pipeline_action_t ep_learn_exec(fte::ctx_t &ctx) {
    hal_ret_t ret;
    fte::flow_update_t flowupd = {type : fte::FLOWUPD_ACTION};

    if (ctx.pkt() == nullptr || ctx.role() == hal::FLOW_ROLE_RESPONDER) {
        return fte::PIPELINE_CONTINUE;
    }

    HAL_TRACE_DEBUG("Invoking EP learning feature");

    if (is_dhcp_flow(&ctx.key()) && is_dhcp_ep_learning_enabled(ctx)) {
            HAL_TRACE_INFO("EP_LEARN : DHCP packet processing...");
            ret = dhcp_process_packet(ctx);

            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error in processing DHCP packet.");
                ctx.set_feature_status(ret);
                ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
            }
            ctx.set_valid_rflow(false);
            if (is_broadcast(ctx)) {
                HAL_TRACE_INFO("EP_LEARN: Ignore Session create for DHCP flood.");
                ctx.set_ignore_session_create(true);
                /*
                 * TODO : Have to clean up the unicast flows.
                 */
                return fte::PIPELINE_END;
            }
    } else if (is_arp_flow(&ctx.key()) && is_arp_ep_learning_enabled(ctx)) {
        HAL_TRACE_INFO("EP_LEARN : ARP packet processing...");
        ret = arp_process_packet(ctx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in processing ARP packet.");
            ctx.set_feature_status(ret);
            ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
        }
        ctx.set_valid_rflow(false);
        if (is_broadcast(ctx)) {
            ctx.set_ignore_session_create(true);
            return fte::PIPELINE_END;
        }
    } else if (is_neighbor_discovery_flow(&ctx.key()) &&
                is_arp_ep_learning_enabled(ctx)) {
        HAL_TRACE_INFO("EP_LEARN : NDP packet processing...");
        ret = neighbor_disc_process_packet(ctx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in processing NDP packet.");
            ctx.set_feature_status(ret);
            ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
        }
        ctx.set_valid_rflow(false);
        if (is_broadcast(ctx)) {
            ctx.set_ignore_session_create(true);
            return fte::PIPELINE_END;
        }
    }

    return fte::PIPELINE_CONTINUE;
}

}
}
