//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"

#include "flow_miss/fmiss_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/ndp_learn.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_learn.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
namespace eplearn {



ip_move_check_handler ip_move_handlers[EP_LEARN_MAX];

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
is_fmiss_ep_learning_enabled(fte::ctx_t &ctx)
{
    hal::l2seg_t *src_l2seg = ctx.sl2seg();

    if (src_l2seg != nullptr && src_l2seg->eplearn_cfg.fmiss_cfg.enabled) {
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

/*
 * Check whether packet is host originated for learing based on ARP and flow miss
 */
static bool is_host_originated_packet(fte::ctx_t &ctx)
{
    const fte::cpu_rxhdr_t* cpu_hdr = ctx.cpu_rxhdr();
    hal::pd::pd_get_object_from_flow_lkupid_args_t args;
    hal::pd::pd_func_args_t          pd_func_args = {0};
    hal::hal_obj_id_t obj_id;
    void *obj;
    hal_ret_t ret = HAL_RET_OK;
    hal::l2seg_t *l2seg;
    ether_header_t * ethhdr;
    ep_t *sep = nullptr;
    if_t *sif;

    if (!is_arp_ep_learning_enabled(ctx)) {
        return false;
    }

    args.flow_lkupid = cpu_hdr->lkp_vrf;
    args.obj_id = &obj_id;
    args.pi_obj = &obj;
    pd_func_args.pd_get_object_from_flow_lkupid = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, &pd_func_args);
    if (ret != HAL_RET_OK && obj_id != hal::HAL_OBJ_ID_L2SEG) {
        HAL_TRACE_ERR("fte: Invalid obj id: {}, ret:{}", obj_id, ret);
        return false;
    }
    l2seg = (hal::l2seg_t *)obj;

    ethhdr = (ether_header_t *)(ctx.pkt() + cpu_hdr->l2_offset);
    sep = hal::find_ep_by_l2_key(l2seg->seg_id, ethhdr->smac);
    if (sep == nullptr) {
        /* Probably remote endpoint */
        HAL_TRACE_INFO("Source endpoint not found.");
        return false;
    }

    sif = hal::find_if_by_handle(sep->if_handle);
    if (!sif || sif->if_type != intf::IF_TYPE_ENIC) {
        HAL_TRACE_INFO("Source endpoint interface is not of type ENIC.");
        return false;
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
    } else if (is_arp_flow(&ctx.key())) {
        if (is_host_originated_packet(ctx) &&
                is_arp_ep_learning_enabled(ctx)) {
            HAL_TRACE_INFO("EP_LEARN : ARP packet processing...");
            ret = arp_process_packet(ctx);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error in processing ARP packet.");
                ctx.set_feature_status(ret);
                ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
            }
        }
        ctx.set_valid_rflow(false);
    } else if (is_neighbor_discovery_flow(&ctx.key()) &&
            is_arp_ep_learning_enabled(ctx)) {
        HAL_TRACE_INFO("EP_LEARN : NDP packet processing...");
        ret = neighbor_disc_process_packet(ctx);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error in processing NDP packet.");
            ctx.set_feature_status(ret);
            ctx.update_flow(flowupd, FLOW_ROLE_INITIATOR);
        }
    } else if (is_host_originated_packet(ctx) &&
            is_fmiss_ep_learning_enabled(ctx) &&
            flow_miss_learning_required(ctx)) {
            HAL_TRACE_INFO("EP_LEARN : Flow miss packet processing...");
        ret = flow_miss_process_packet(ctx);
        if (ret != HAL_RET_OK) {
            ctx.set_feature_status(ret);
            HAL_TRACE_ERR("Error in processing flow miss packet.")
        }
    }

    return fte::PIPELINE_CONTINUE;
}


void
register_ip_move_check_handler(ip_move_check_handler handler,
        ep_learn_type_t type) {
    ip_move_handlers[type] = handler;
}

hal_ret_t
eplearn_ip_move_process(hal_handle_t ep_handle, const ip_addr_t *ip_addr,
        ep_learn_type_t type) {

    hal_ret_t ret;

    for (int i = 0; i < EP_LEARN_MAX; i++) {
        if (i != type && ip_move_handlers[i] != nullptr) {
            ret = ip_move_handlers[i](ep_handle, ip_addr);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("IP move handler failed!");
                return ret;
            }
        }
    }

    return HAL_RET_OK;
}

}
}
