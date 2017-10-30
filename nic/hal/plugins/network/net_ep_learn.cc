#include "ep_learn/arp/arp_learn.hpp"
#include "ep_learn/dhcp/dhcp_learn.hpp"
#include "net_plugin.hpp"

namespace hal {
namespace net {

fte::pipeline_action_t ep_learn_exec(fte::ctx_t &ctx) {
    hal_ret_t ret;
    fte::flow_update_t flowupd = {type : fte::FLOWUPD_ACTION};

    HAL_TRACE_DEBUG("Invoking EP learning feature");
    if (hal::network::is_dhcp_flow(&ctx.key())) {
        ret = hal::network::dhcp_process_packet(ctx);

        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in processing DHCP packet.");
            flowupd.action = session::FLOW_ACTION_DROP;
            ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
            return fte::PIPELINE_END;
        }
    }

    if (hal::network::is_arp_flow(&ctx.key())) {
        ret = hal::network::arp_process_packet(ctx);

        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in processing ARP packet.");
            flowupd.action = session::FLOW_ACTION_DROP;
            ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
            return fte::PIPELINE_END;
        }
    }
    return fte::PIPELINE_CONTINUE;
}

}
}
