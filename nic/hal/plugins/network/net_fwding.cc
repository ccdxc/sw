#include <net/ethernet.h>
#include "nic/p4/include/common_defines.h"
#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
namespace net {


static inline hal_ret_t
update_rewrite_info(fte::ctx_t&ctx)
{
    uint8_t vlan_valid;
    uint16_t vlan_id;
    mac_addr_t *smac, *dmac;

    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};

    // smac rewrite for routed pkts
    if (ctx.sl2seg() != ctx.dl2seg()) {
        dmac = hal::ep_get_mac_addr(ctx.dep());
        smac = hal::ep_get_rmac(ctx.dep(), ctx.dl2seg());

        flowupd.header_rewrite.flags.dec_ttl = true;
        HEADER_SET_FLD(flowupd.header_rewrite, ether, dmac, *(ether_addr *)dmac);
        HEADER_SET_FLD(flowupd.header_rewrite, ether, smac, *(ether_addr *)smac);
    }

    // VLAN rewrite
    if (ctx.dif()->if_type != intf::IF_TYPE_TUNNEL) {
        if_l2seg_get_encap(ctx.dif(), ctx.dl2seg(), &vlan_valid, &vlan_id);
        if (vlan_valid == 1) {
            HEADER_SET_FLD(flowupd.header_rewrite, ether, vlan_id, vlan_id);
        }
    }

    return ctx.update_flow(flowupd);
}

static inline hal_ret_t
update_fwding_info(fte::ctx_t&ctx)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};
    hal::if_t *dif = ctx.dif();

    if (dif && dif->if_type == intf::IF_TYPE_ENIC) {
        hal::lif_t *lif = if_get_lif(dif);
        if (lif == NULL){
            return HAL_RET_LIF_NOT_FOUND;
        }
        flowupd.fwding.qtype = lif_get_qtype(lif, intf::LIF_QUEUE_PURPOSE_RX);
        flowupd.fwding.qid_en = 1;
        flowupd.fwding.qid = 0;
    } else if (ctx.sep() && ctx.sep()->pinned_if_handle != HAL_HANDLE_INVALID) {
        dif = hal::find_if_by_handle(ctx.sep()->pinned_if_handle);
        HAL_ASSERT_RETURN(dif, HAL_RET_IF_NOT_FOUND);

        if (ctx.dif() == NULL) {
            flowupd.fwding.dif = dif;
            flowupd.fwding.dl2seg = ctx.sl2seg();
        }
    }

    if (dif == NULL) {
        return HAL_RET_IF_NOT_FOUND;
    }

    // update fwding info
    flowupd.fwding.lport = hal::pd::if_get_lport_id(dif);
    return ctx.update_flow(flowupd);
}

static inline hal_ret_t
update_flow(fte::ctx_t&ctx)
{
    hal_ret_t ret;

    ret = update_fwding_info(ctx);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = update_rewrite_info(ctx);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

fte::pipeline_action_t
fwding_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret;

    ret = update_flow(ctx);

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }

    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
fwding_pre_stage_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    /*
     * When proxy mirror is enabled on the current flow, 2 flow instances will
     * be required: instance 0 with normal network forwarding plus mirror, and
     * instance 1 with normal forwarding only (as needed for CPU flow-miss Tx).
     *
     * Hence, prior to stage_exec(), if the current flow has been updated
     * to mirror to a proxy, issue network forwarding update for the flow.
     */
    if (ctx.proxy_mirror_flow()) {
        ret = update_flow(ctx);

        if (ret != HAL_RET_OK) {
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END; 
        }
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
