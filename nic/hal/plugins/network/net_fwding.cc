#include "net_plugin.hpp"
#include <interface_api.hpp>
#include <pd_api.hpp>
#include <net/ethernet.h>
#include <common_defines.h>

namespace hal {
namespace net {

static inline hal_ret_t
update_tunnel_info(fte::ctx_t&ctx, hal::flow_role_t role, hal::if_t *dif, hal::l2seg_t *dl2seg)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_PUSH};

    if (dif->if_type != intf::IF_TYPE_TUNNEL) {
        return HAL_RET_OK;
    }

    // TODO(goli) set appropriate header fields
    HEADER_SET_FLD(flowupd.header_push, ether, dmac, ether_addr{});
    HEADER_SET_FLD(flowupd.header_push, ether, smac, ether_addr{});
    HEADER_SET_FLD(flowupd.header_push, ipv4, sip, ipv4_addr_t{});
    HEADER_SET_FLD(flowupd.header_push, ipv4, dip, ipv4_addr_t{});
    
    switch (dl2seg->fabric_encap.type) {
    case types::encapType::ENCAP_TYPE_VXLAN:
        HEADER_SET_FLD(flowupd.header_push, vxlan, tenant_id, dl2seg->fabric_encap.val);
        break;
    default:
        return HAL_RET_INVALID_ARG;
    }

    return ctx.update_flow(role, flowupd);
}

static inline hal_ret_t
update_rewrite_info(fte::ctx_t&ctx, hal::flow_role_t role,
                    hal::ep_t *dep, hal::if_t *dif,
                    hal::l2seg_t *sl2seg, hal::l2seg_t *dl2seg)
{
    uint8_t vlan_tags;
    uint16_t vlan_id;
    mac_addr_t *smac, *dmac;

    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};

    // smac rewrite for routed pkts
    if (sl2seg != dl2seg) {
        dmac = hal::ep_get_mac_addr(dep);
        smac = hal::ep_get_rmac(dep, dl2seg);

        flowupd.header_rewrite.flags.dec_ttl = true;
        HEADER_SET_FLD(flowupd.header_rewrite, ether, dmac, *(ether_addr *)dmac);
        HEADER_SET_FLD(flowupd.header_rewrite, ether, smac, *(ether_addr *)smac);
    }

    // VLAN rewrite
    if (dif->if_type != intf::IF_TYPE_TUNNEL) {
        if_l2seg_get_encap(dif, dl2seg, &vlan_tags, &vlan_id);
        if (vlan_tags == 1) {
            HEADER_SET_FLD(flowupd.header_rewrite, ether, vlan_id, vlan_id);
        }
    }

    return ctx.update_flow(role, flowupd);
}

static inline hal_ret_t
update_fwding_info(fte::ctx_t&ctx, hal::flow_role_t role, hal::if_t *dif)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    // update fwding info
    flowupd.fwding.lport = hal::pd::if_get_lport_id(dif);

    // TODO(goli) Update qid for enic
    if (dif->if_type == intf::IF_TYPE_ENIC) {
        flowupd.fwding.qtype = Q_TYPE_RXQ;
        flowupd.fwding.qid_en = 1;
        flowupd.fwding.qid = 0;
    }

    return ctx.update_flow(role, flowupd);
}

static inline hal_ret_t
update_flow(fte::ctx_t&ctx, hal::flow_role_t role,
            hal::ep_t *dep, hal::if_t *dif,
            hal::l2seg_t *sl2seg, hal::l2seg_t *dl2seg)
{
    hal_ret_t ret;

    if (dif == NULL) {
        // TODO(goli) may be lookup dif using src ep info (host pinning)
        return HAL_RET_IF_NOT_FOUND;
    }

    ret = update_fwding_info(ctx, role, dif);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = update_rewrite_info(ctx, role, dep, dif, sl2seg, dl2seg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = update_tunnel_info(ctx, role, dif, dl2seg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

fte::pipeline_action_t
fwding_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret;

    // Update iflow
    ret = update_flow(ctx, hal::FLOW_ROLE_INITIATOR, ctx.dep(), ctx.dif(), ctx.sl2seg(), ctx.dl2seg());
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }

    //Update rflow
    if (ctx.valid_rflow()) {
        ret = update_flow(ctx, hal::FLOW_ROLE_RESPONDER, ctx.sep(), ctx.sif(), ctx.dl2seg(), ctx.sl2seg());
        if (ret != HAL_RET_OK) {
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END; 
        }
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
