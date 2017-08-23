#include "fte_flow.hpp"
#include "fte_ctx.hpp"
#include <session.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>
#include <defines.h>

namespace fte {

static inline hal_ret_t
_build_flow_key_from_phv(const phv_t *phv, hal::flow_key_t *key)
{
    switch (phv->lkp_type) {
    case FLOW_KEY_LOOKUP_TYPE_MAC:
        key->flow_type = hal::FLOW_TYPE_L2;
        break;
    case FLOW_KEY_LOOKUP_TYPE_IPV4:
        key->flow_type = hal::FLOW_TYPE_V4;
        break;
    case FLOW_KEY_LOOKUP_TYPE_IPV6:
        key->flow_type = hal::FLOW_TYPE_V6;
        break;
    default:
        HAL_TRACE_ERR("fte - unknown flow lkp_type {}", phv->lkp_type);
        return HAL_RET_INVALID_ARG;
    }

    key->dir = phv->lkp_dir; 

    hal::l2seg_t *l2seg =  hal::pd::find_l2seg_by_hwid(phv->lkp_vrf);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("fte - l2seg not found, hwid={}", phv->lkp_vrf);
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    switch (key->flow_type) {
    case hal::FLOW_TYPE_L2:
        // L2 flow
        // TODO(goli) - check for L2 flow session_pd is not copying smac/dmac to flow hash
        key->l2seg_id = l2seg->seg_id;
        memcpy(key->smac, phv->lkp_src, sizeof(key->smac));
        memcpy(key->dmac, phv->lkp_dst, sizeof(key->dmac));
        break;
    case  hal::FLOW_TYPE_V4:
    case  hal::FLOW_TYPE_V6:
        // v4 or v6 flow
        key->tenant_id = l2seg->tenant_id; 
        memcpy(key->sip.v6_addr.addr8, phv->lkp_src, sizeof(key->sip.v6_addr.addr8));
        memcpy(key->dip.v6_addr.addr8, phv->lkp_dst, sizeof(key->dip.v6_addr.addr8));
        key->proto = phv->lkp_proto;
        switch (key->proto) {
        case IP_PROTO_TCP:
        case IP_PROTO_UDP:
            key->sport = phv->lkp_sport;
            key->dport = phv->lkp_dport;
            break;
        case IP_PROTO_ICMP:
            key->icmp_type =  phv->lkp_sport >> 8;
            key->icmp_code = phv->lkp_sport & 0x00FF;
            key->icmp_id = phv->lkp_dport;
            break;
        default:
            key->sport = 0;
            key->dport = 0;
        }
        break;
    }
    

    return HAL_RET_OK;
}


static inline hal_ret_t
_lookup_flow_objs(ctx_t &ctx, const hal::flow_key_t *key)
{
    hal::tenant_id_t tid;
    if (key->flow_type == hal::FLOW_TYPE_L2) {
        hal::l2seg_t *l2seg = hal::find_l2seg_by_id(key->l2seg_id);
        if (l2seg == NULL) {
            HAL_TRACE_ERR("fte - l2seg not found, key={}", *key);
            return HAL_RET_L2SEG_NOT_FOUND;
        }
        tid = l2seg->tenant_id;
    } else {
        tid = key->tenant_id;
    }

    ctx.tenant = hal::find_tenant_by_id(tid);
    if (ctx.tenant == NULL) {
        HAL_TRACE_ERR("fte - tenant {} not found, key={}", tid, *key);
        return HAL_RET_TENANT_NOT_FOUND;
    }

    //Lookup src and dest EPs
    hal::ep_get_from_flow_key(key, &ctx.sep, &ctx.dep);

    //one of the src or dst EPs should be known (no transit traffic)
    if (ctx.sep == NULL && ctx.dep == NULL) {
        HAL_TRACE_ERR("fte - src and dest eps unknown, key={}", *key);
        return HAL_RET_EP_NOT_FOUND;
    }

    if (ctx.sep) {
        ctx.sl2seg = hal::find_l2seg_by_handle(ctx.sep->l2seg_handle);
        ctx.sif = hal::find_if_by_handle(ctx.sep->if_handle);
    }

    if (ctx.dep) {
        ctx.dl2seg = hal::find_l2seg_by_handle(ctx.dep->l2seg_handle);
        ctx.dif = hal::find_if_by_handle(ctx.dep->if_handle);
    }

    return HAL_RET_OK;
}

hal_ret_t
_lookup_session(ctx_t &ctx, hal::flow_key_t *key)
{
    hal::flow_t *hflow;

    hflow = NULL; // TODO(goli) do hal::lookup_flow(key);
    if (!hflow) {
        return HAL_RET_SESSION_NOT_FOUND;
    }

    ctx.session = hflow->session;
    ctx.session_cfg = hflow->session->config;

    if (hflow->config.role == hal::FLOW_ROLE_INITIATOR) {
        ctx.flow = &ctx.iflow;
        ctx.iflow = hflow->config;
        if (hflow->reverse_flow) {
            ctx.rflow = hflow->reverse_flow->config;
            ctx.rflow_valid = true;
        }
    } else {
        ctx.flow = &ctx.rflow;
        ctx.rflow = hflow->config;
        ctx.iflow = hflow->reverse_flow->config;
        ctx.rflow_valid = true;
    }

    ctx.drop = ctx.flow->action == session::FLOW_ACTION_DROP;

    return HAL_RET_OK;
}

hal_ret_t
_create_session(ctx_t &ctx, hal::flow_key_t *key)
{
    ctx.flow = &ctx.iflow;
    ctx.iflow.key = *key;
    ctx.iflow.role = hal::FLOW_ROLE_INITIATOR;


    // TODO(goli) - check if this can be populated from the phv or src_lif
    ctx.iflow.key.dir = (ctx.sep && (ctx.sep->ep_flags & EP_FLAGS_LOCAL)) ?
        FLOW_DIR_FROM_DMA : FLOW_DIR_FROM_UPLINK;

    // Init rflow for TCP and UDP
    if ((key->flow_type == hal::FLOW_TYPE_V4 || key->flow_type == hal::FLOW_TYPE_V6) &&
        (key->proto == IP_PROTO_TCP || key->proto == IP_PROTO_UDP)) {
        ctx.rflow_valid = true;
        ctx.rflow.key.flow_type = key->flow_type;
        ctx.iflow.key.dir = (ctx.dep && (ctx.dep->ep_flags & EP_FLAGS_LOCAL)) ?
            FLOW_DIR_FROM_DMA : FLOW_DIR_FROM_UPLINK;
        ctx.rflow.key.tenant_id = key->tenant_id; // TODO(goli) - check
        ctx.rflow.key.sip = key->dip;
        ctx.rflow.key.dip = key->sip;
        ctx.rflow.key.proto = key->proto;
        ctx.rflow.key.sport = key->dport;
        ctx.rflow.key.dport = key->sport;

        ctx.rflow.role = hal::FLOW_ROLE_RESPONDER;
    }

    return HAL_RET_OK;
}

hal_ret_t
flow_init_flows(ctx_t &ctx)
{
    hal_ret_t ret;
    hal::flow_key_t key;

    // Build the key and lookup flow 
    ret = _build_flow_key_from_phv(ctx.phv, &key);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // Lookup ep, intf, l2seg, tenant
    ret = _lookup_flow_objs(ctx, &key);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // Lookup old session
    ret = _lookup_session(ctx, &key);
    if (ret == HAL_RET_SESSION_NOT_FOUND) {
        // Create new session
        // TODO(goli) - check syn flag for TCP (could be a FIN packet for non-exisiting flow)
        ctx.flow_miss = true;
        ret = _create_session(ctx, &key);
    }

    return ret;
}

hal_ret_t
flow_update_gft(ctx_t &ctx)
{
    hal::session_args_t session_args = {};

    // Create hal::session and update GFT
    session_args.session = &ctx.session_cfg;
    session_args.iflow = &ctx.iflow;
    if (ctx.rflow_valid) {
        session_args.rflow = &ctx.rflow;
    }
    session_args.tenant = ctx.tenant;
    session_args.sep = ctx.sep;
    session_args.dep = ctx.dep;
    session_args.sif = ctx.sif;
    session_args.dif = ctx.dif;
    session_args.sl2seg = ctx.sl2seg;
    session_args.dl2seg = ctx.dl2seg;

    return hal::session_create(&session_args, NULL);
}

} // namespace fte
