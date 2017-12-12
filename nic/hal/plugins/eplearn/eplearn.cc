#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/ndp_learn.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_learn.hpp"

namespace hal {
namespace eplearn {

static bool
is_broadcast(fte::ctx_t &ctx) {
    const fte::cpu_rxhdr_t* cpu_hdr = ctx.cpu_rxhdr();
    ether_header_t *eth_hdr = (ether_header_t*)(ctx.pkt() + cpu_hdr->l2_offset);
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        if (eth_hdr->dmac[i] != 0xff) {
            return false;
        }
    }
    return true;
}

fte::pipeline_action_t ep_learn_exec(fte::ctx_t &ctx) {
    hal_ret_t ret;
    fte::flow_update_t flowupd = {type : fte::FLOWUPD_ACTION};

    if (ctx.pkt() == nullptr || ctx.role() == hal::FLOW_ROLE_RESPONDER) {
        return fte::PIPELINE_CONTINUE;
    }

    HAL_TRACE_DEBUG("Invoking EP learning feature");
    if (is_dhcp_flow(&ctx.key())) {
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
    } else if (is_arp_flow(&ctx.key())) {
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
    } else if (is_neighbor_discovery_flow(&ctx.key())) {
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
