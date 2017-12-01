#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/ndp_learn.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_learn.hpp"

namespace hal {
namespace net {

fte::pipeline_action_t ep_learn_exec(fte::ctx_t &ctx) {
    hal_ret_t ret;
    fte::flow_update_t flowupd = {type : fte::FLOWUPD_ACTION};

    if (ctx.pkt() == nullptr || ctx.role() == hal::FLOW_ROLE_RESPONDER) {
        return fte::PIPELINE_CONTINUE;
    }

    HAL_TRACE_DEBUG("Invoking EP learning feature");
    if (hal::network::is_dhcp_flow(&ctx.key())) {
        ret = hal::network::dhcp_process_packet(ctx);

        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in processing DHCP packet.");
            ctx.set_feature_status(ret);
            ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
        }
        return fte::PIPELINE_END;
    }

    if (hal::network::is_arp_flow(&ctx.key())) {
        HAL_TRACE_INFO("EP_LEARN : ARP packet processing...");
        ret = hal::network::arp_process_packet(ctx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in processing ARP packet.");
            ctx.set_feature_status(ret);
            ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
        }
        ctx.set_valid_rflow(false);
        if (hal::network::is_arp_broadcast(&ctx.key())) {
            ctx.set_ignore_session_create(true);
            return fte::PIPELINE_END;
        }
    }

    if (hal::network::is_neighbor_discovery_flow(&ctx.key())) {
        HAL_TRACE_INFO("EP_LEARN : NDP packet processing...");
        ret = hal::network::neighbor_disc_process_packet(ctx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in processing NDP packet.");
            ctx.set_feature_status(ret);
            ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
        }
        ctx.set_valid_rflow(false);
        //if (hal::network::is_arp_broadcast(&ctx.key())) {
            ctx.set_ignore_session_create(true);
            return fte::PIPELINE_END;
        //}
    }

    return fte::PIPELINE_CONTINUE;
}

}
}
