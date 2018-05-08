//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "nic/utils/nat/addr_db.hpp"

namespace hal {
namespace plugins {
namespace nat {

/*
 * update_flow
 *   Update flow from NAT policy
 */
static inline hal_ret_t
update_iflow_from_nat_rules (fte::ctx_t& ctx)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};
    hal::utils::nat::addr_entry_key_t  src_addr_key, dst_addr_key;
    hal::utils::nat::addr_entry_t     *src_addr_entry, *dst_addr_entry;
    const nat_cfg_rule_t              *nat_cfg;
    hal::rule_data_t                  *rule_data;
    hal::ipv4_tuple                    acl_key = {};
    const hal::ipv4_rule_t            *rule = NULL;
    hal_ret_t                          ret = HAL_RET_OK;
    const acl::acl_ctx_t              *acl_ctx = NULL;

    const char *ctx_name = nat_acl_ctx_name(ctx.key().svrf_id);
    acl_ctx = acl::acl_get(ctx_name);
    if (acl_ctx == NULL) {
        HAL_TRACE_DEBUG("nat::flow lookup failed to lookup acl_ctx {}", ctx_name);
        return ret;
    }
    // fte state to store the original vrf/ip/port for rflow
    nat_info_t *nat_info = (nat_info_t *)ctx.feature_state();
    if (ctx.key().flow_type == FLOW_TYPE_V4) {
        src_addr_key.ip_addr.af = IP_AF_IPV4;
        dst_addr_key.ip_addr.af = IP_AF_IPV4;
    } else if (ctx.key().flow_type == FLOW_TYPE_V6){
        src_addr_key.ip_addr.af = IP_AF_IPV6;
        dst_addr_key.ip_addr.af = IP_AF_IPV6;
    } else {
        return HAL_RET_OK;
    }

    // snat 
    src_addr_key.vrf_id = ctx.key().svrf_id;
    src_addr_key.ip_addr.addr = ctx.key().sip;
    src_addr_entry = hal::utils::nat::addr_entry_get(&src_addr_key);

    // dnat
    dst_addr_key.vrf_id = ctx.key().dvrf_id;
    dst_addr_key.ip_addr.addr = ctx.key().dip;
    dst_addr_entry = hal::utils::nat::addr_entry_get(&dst_addr_key);

    if (!src_addr_entry || !dst_addr_entry) {
        acl_key.proto = ctx.key().proto;
        acl_key.ip_src = ctx.key().sip.v4_addr;
        acl_key.ip_dst = ctx.key().dip.v4_addr;
        switch ( ctx.key().proto) {
        case types::IPPROTO_ICMP:
        case types::IPPROTO_ICMPV6:
            acl_key.port_src =  ctx.key().icmp_id;
            acl_key.port_dst = ((ctx.key().icmp_type << 8) |  ctx.key().icmp_code);
            break;
        case types::IPPROTO_ESP:
            acl_key.port_src = ctx.key().spi >> 16 & 0xFFFF;
            acl_key.port_dst = ctx.key().spi & 0xFFFF;
            break;
        case types::IPPROTO_TCP:
        case types::IPPROTO_UDP:
            acl_key.port_src = ctx.key().sport;
            acl_key.port_dst = ctx.key().dport;
            break;
        default:
            HAL_ASSERT(true);
            ret = HAL_RET_FTE_RULE_NO_MATCH;
            return ret;
        }
        ret = acl_classify(acl_ctx, (const uint8_t *)&acl_key, (const acl_rule_t **)&rule, 0x01);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("nat::rule lookup failed ret={}", ret);
            return ret;
        }
        if (rule) {
            rule_data = (hal::rule_data_t *) rule->data.userdata;
            nat_cfg = (const hal::nat_cfg_rule_t *)rule_data->userdata;
            // Handle for each NatAction: None, Static address, Dynamic address
            if (!src_addr_entry) {
                if (nat_cfg->action.src_nat_action != ::nat::NAT_TYPE_NONE) {
                    nat_pool_t *nat_pool = find_nat_pool_by_handle(nat_cfg->action.src_nat_pool);
                    if (nat_pool) {
                        ret = nat_pool_address_alloc(nat_pool, &src_addr_entry->tgt_ip_addr);
                        if (ret != HAL_RET_OK) {
                            HAL_TRACE_DEBUG("failed to allocate src nat ip");
                            return ret;
                        }
                    } else {
                        HAL_TRACE_DEBUG("unable to find src nat pool for the handle = {}", nat_cfg->action.src_nat_pool);
                        return HAL_RET_ERR;
                    }
                }
            }

            if (!dst_addr_entry) {
                if (nat_cfg->action.src_nat_action != ::nat::NAT_TYPE_NONE) {
                    nat_pool_t *nat_pool = find_nat_pool_by_handle(nat_cfg->action.dst_nat_pool);
                    if (nat_pool) {
                        ret = nat_pool_address_alloc(nat_pool, &dst_addr_entry->tgt_ip_addr);
                        if (ret != HAL_RET_OK) {
                            HAL_TRACE_DEBUG("failed to allocate dst nat ip");
                            return ret;
                        }
                    } else {
                        HAL_TRACE_DEBUG("unable to find dst nat pool for the handle = {}", nat_cfg->action.dst_nat_pool);
                        return HAL_RET_ERR;
                    }
                }
            }
        }
    }

    if (src_addr_entry) {
        if (ctx.key().flow_type == FLOW_TYPE_V4) {
            if (src_addr_entry->tgt_vrf_id != ctx.key().svrf_id) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, svrf_id, src_addr_entry->tgt_vrf_id);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dvrf_id, src_addr_entry->tgt_vrf_id);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv4, sip, src_addr_entry->tgt_ip_addr.addr.v4_addr);
        } else {
            if (src_addr_entry->tgt_vrf_id != ctx.key().svrf_id) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, svrf_id, src_addr_entry->tgt_vrf_id);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dvrf_id, src_addr_entry->tgt_vrf_id);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv6, sip,src_addr_entry->tgt_ip_addr.addr.v6_addr);
        }

        // Store the original info for rflow dnat
        nat_info->nat_dip.af = src_addr_key.ip_addr.af;
        nat_info->nat_dip.addr = ctx.key().sip;
        if (src_addr_entry->tgt_vrf_id != ctx.key().svrf_id) {
            nat_info->nat_dvrf = ctx.key().svrf_id;
            nat_info->nat_svrf = ctx.key().dvrf_id;
        }

        // TODO  port nat not supported yet
        // if (addr_entry->tgt_port) {
        //     nat_info->nat_dport = ctx.key().sport;
        //     if (ctx.key().proto == IPPROTO_TCP) {
        //         HEADER_SET_FLD(flowupd.header_rewrite, tcp, sport, addr_entry->tgt_port);
        //     } else (ctx.key().proto == IPPROTO_UDP) {
        //         HEADER_SET_FLD(flowupd.header_rewrite, udp, sport, addr_entry->tgt_port);
        //     }
        // }

    }

    if (dst_addr_entry) {
        if (ctx.key().flow_type == FLOW_TYPE_V4) {
            if (dst_addr_entry->tgt_vrf_id != ctx.key().dvrf_id) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dvrf_id, dst_addr_entry->tgt_vrf_id);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dip, dst_addr_entry->tgt_ip_addr.addr.v4_addr);
        } else {
            if (dst_addr_entry->tgt_vrf_id != ctx.key().dvrf_id) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dvrf_id, dst_addr_entry->tgt_vrf_id);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dip, dst_addr_entry->tgt_ip_addr.addr.v6_addr);
        }

        // Store the original info for rflow snat
        nat_info->nat_sip.af = dst_addr_key.ip_addr.af;
        nat_info->nat_sip.addr = ctx.key().dip;
        if (dst_addr_entry->tgt_vrf_id != ctx.key().svrf_id) {
            nat_info->nat_svrf = ctx.key().dvrf_id;
        }

        // TODO port nat not supported yet
        // if (addr_entry->tgt_port) {
        //     nat_info->nat_sport = ctx.key().dport;
        //     if (ctx.key().proto == IPPROTO_TCP) {
        //         HEADER_SET_FLD(flowupd.header_rewrite, tcp, dport, addr_entry->tgt_port);
        //     } else if (ctx.key().proto == IPPROTO_UDP) {
        //         HEADER_SET_FLD(flowupd.header_rewrite, udp, dport, addr_entry->tgt_port);
        //     }
        // }
    }

    return ctx.update_flow(flowupd);
}

/*
 * update_flow
 *   Update rflow from nat info
 */
static inline hal_ret_t
update_rflow_from_nat_info (fte::ctx_t& ctx)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};
    nat_info_t *nat_info = (nat_info_t *)ctx.feature_state();

    // snat
    if (!ip_addr_is_zero(&nat_info->nat_sip)) {
        if (nat_info->nat_sip.af == IP_AF_IPV4) {
            if (nat_info->nat_svrf) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, svrf_id, nat_info->nat_svrf);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dvrf_id, nat_info->nat_svrf);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv4, sip, nat_info->nat_sip.addr.v4_addr);
        } else {
            if (nat_info->nat_svrf) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, svrf_id, nat_info->nat_svrf);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dvrf_id, nat_info->nat_svrf);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv6, sip, nat_info->nat_sip.addr.v6_addr);
        }
    }

    if (nat_info->nat_sport) {
        if (ctx.key().proto == IPPROTO_TCP) {
            HEADER_SET_FLD(flowupd.header_rewrite, tcp, sport, nat_info->nat_sport);
        } else if (ctx.key().proto == IPPROTO_UDP) {
            HEADER_SET_FLD(flowupd.header_rewrite, udp, sport, nat_info->nat_sport);
        }
    }

    // dnat
    if (!ip_addr_is_zero(&nat_info->nat_dip)) {
        if (nat_info->nat_dip.af == IP_AF_IPV4) {
            if (nat_info->nat_dvrf) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dvrf_id, nat_info->nat_dvrf);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dip, nat_info->nat_dip.addr.v4_addr);
        } else {
            if (nat_info->nat_dvrf) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dvrf_id, nat_info->nat_dvrf);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dip, nat_info->nat_dip.addr.v6_addr);
        }
    }

    if (nat_info->nat_dport) {
        if (ctx.key().proto == IPPROTO_TCP) {
            HEADER_SET_FLD(flowupd.header_rewrite, tcp, dport, nat_info->nat_dport);
        } else if (ctx.key().proto == IPPROTO_UDP) {
            HEADER_SET_FLD(flowupd.header_rewrite, udp, dport, nat_info->nat_dport);
        }
    }

    return ctx.update_flow(flowupd);
}

/*
 * nat_exec
 *   Entry point into the NAT feature
 */
fte::pipeline_action_t
nat_exec (fte::ctx_t &ctx)
{
    hal_ret_t ret;

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        ret = update_iflow_from_nat_rules(ctx);
    } else {
        ret = update_rflow_from_nat_info(ctx);
    }

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace nat
} // namespace plugins
} // namespace hal
