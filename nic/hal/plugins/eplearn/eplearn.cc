//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"

#include "data_pkt/dpkt_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/ndp_learn.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_learn.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"

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
is_dpkt_ep_learning_enabled(fte::ctx_t &ctx)
{
    hal::l2seg_t *src_l2seg = ctx.sl2seg();

    if (src_l2seg != nullptr && src_l2seg->eplearn_cfg.dpkt_cfg.enabled) {
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
 * Check whether packet is host originated.
 */
static bool is_host_originated_packet(fte::ctx_t &ctx) {

    if (ctx.direction() == hal::FLOW_DIR_FROM_DMA) {
        return true;
    }

    return false;
}


static hal_ret_t
do_learning_ep_lif_update(fte::ctx_t &ctx) {
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
    lif_t *lif;
    if_t *new_hal_if;
    uint32_t cur_lif_id = 0;
    hal_handle_t ep_handle;

    /* Lif update only for host originated packet */
    if (!is_host_originated_packet(ctx)) {
        return ret;
    }

   if ((cpu_hdr->src_app_id == P4PLUS_APPTYPE_IPSEC) &&
       (cpu_hdr->src_lif == HAL_LIF_CPU)) {
       HAL_TRACE_DEBUG("No EP lif update necessary for pkts coming from CPU lif");
       return ret;
    }

    HAL_TRACE_VERBOSE("Doing EP lif update if necessary");

    args.flow_lkupid = cpu_hdr->lkp_vrf;
    args.obj_id = &obj_id;
    args.pi_obj = &obj;
    pd_func_args.pd_get_object_from_flow_lkupid = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, &pd_func_args);
    if (ret != HAL_RET_OK && obj_id != hal::HAL_OBJ_ID_L2SEG) {
        HAL_TRACE_ERR("fte: Invalid obj id: {}, ret:{}", obj_id, ret);
        return ret;
    }
    l2seg = (hal::l2seg_t *)obj;

    ethhdr = GET_L2_HEADER(ctx.pkt(), cpu_hdr);
    sep = hal::find_ep_by_l2_key(l2seg->seg_id, ethhdr->smac);
    if (sep == nullptr) {
        HAL_TRACE_ERR("Source endpoint not found.");
        return HAL_RET_EP_NOT_FOUND;
    }

    sif = hal::find_if_by_handle(sep->if_handle);
    if (sif == NULL) {
        HAL_TRACE_ERR("Source interface not found.");
        return HAL_RET_IF_NOT_FOUND;
    }

    lif = find_lif_by_handle(sif->lif_handle);
    if (lif != NULL) {
        /* get the current lif ID if found */
        cur_lif_id = hal::lif_hw_lif_id_get(lif);
    }

    /* Update lif nothing set or LIF id changed */
    if (cur_lif_id == 0 || cur_lif_id != cpu_hdr->src_lif) {

        lif = hal::find_lif_by_hw_lif_id(cpu_hdr->src_lif);

        if (lif == NULL) {
            HAL_TRACE_INFO("Could not find lif object for hw_lif{}.", cpu_hdr->src_lif);
            return HAL_RET_LIF_NOT_FOUND;
        }

       ret = hal::enic_update_lif(sif, lif, &new_hal_if);

       if (ret != HAL_RET_OK) {
           HAL_TRACE_ERR("Enic update failed!");
           return ret;
       }

       //Remember the ep handle as we are updating.
      ep_handle = sep->hal_handle;
      ret = hal::endpoint_update_if(sep, new_hal_if);
      if (ret != HAL_RET_OK) {
          HAL_TRACE_ERR("Endpoint interface update failed!");
          goto end;
      }

      // Open the cfg db as it could be closed
      // in update if
      hal::hal_cfg_db_open(hal::CFG_OP_READ);

      sep = find_ep_by_handle(ep_handle);
      if (sep == nullptr) {
          HAL_TRACE_ERR("Endpoint look up failed after update");
          return HAL_RET_EP_NOT_FOUND;
      }

      /* Set Source EP so that plugins can process correctly */
      ctx.set_sep(sep);

      /* Set Interface so that plugins can process correctly */
      ctx.set_sif(new_hal_if);

    }

    return ret;

end:
    // Come here only if EP update failed
    // Open the cfg db as it could be closed
    // in update if 
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    return ret;
}

fte::pipeline_action_t ep_learn_exec(fte::ctx_t &ctx) {
    hal_ret_t ret;
    fte::flow_update_t flowupd = {type : fte::FLOWUPD_ACTION};

    if (ctx.pkt() == nullptr || ctx.role() == hal::FLOW_ROLE_RESPONDER ||
        (ctx.existing_session() && ctx.is_flow_swapped())) {
        return fte::PIPELINE_CONTINUE;
    }

    ret = do_learning_ep_lif_update(ctx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Ep learning lif update failed!");
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

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
            is_dpkt_ep_learning_enabled(ctx) &&
            dpkt_learn_required(ctx)) {
            HAL_TRACE_INFO("EP_LEARN : Flow miss packet processing...");
        ret = dpkt_learn_process_packet(ctx);
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
