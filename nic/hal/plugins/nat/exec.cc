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
    hal::utils::nat::addr_entry_key_t addr_key;
    hal::utils::nat::addr_entry_t *addr_entry;

    // fte state to store the original vrf/ip/port for rflow
    nat_info_t *nat_info = (nat_info_t *)ctx.feature_state();

    if (ctx.key().flow_type == FLOW_TYPE_V4) {
        addr_key.ip_addr.af = IP_AF_IPV4;
    } else if (ctx.key().flow_type == FLOW_TYPE_V6){
        addr_key.ip_addr.af = IP_AF_IPV6;
    } else {
        return HAL_RET_OK; 
    }

    // snat 
    addr_key.vrf_id = ctx.key().svrf_id;
    addr_key.ip_addr.addr = ctx.key().sip;
    addr_entry = hal::utils::nat::addr_entry_get(&addr_key);

    if (!addr_entry) {
        // TODO lookup nat policy and allocate NAT entry
    }

    if (addr_entry) {
        if (ctx.key().flow_type == FLOW_TYPE_V4) {
            if (addr_entry->tgt_vrf_id != ctx.key().svrf_id) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, svrf_id, addr_entry->tgt_vrf_id);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dvrf_id, addr_entry->tgt_vrf_id);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv4, sip, addr_entry->tgt_ip_addr.addr.v4_addr);
        } else {
            if (addr_entry->tgt_vrf_id != ctx.key().svrf_id) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, svrf_id, addr_entry->tgt_vrf_id);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dvrf_id, addr_entry->tgt_vrf_id);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv6, sip, addr_entry->tgt_ip_addr.addr.v6_addr);
        }

        // Store the original info for rflow dnat
        nat_info->nat_dip.af = addr_key.ip_addr.af;
        nat_info->nat_dip.addr = ctx.key().sip;
        if (addr_entry->tgt_vrf_id != ctx.key().svrf_id) {
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

    // dnat
    addr_key.vrf_id = ctx.key().dvrf_id;
    addr_key.ip_addr.addr = ctx.key().dip;
    addr_entry = hal::utils::nat::addr_entry_get(&addr_key);

    if (!addr_entry) {
        // TODO lookup nat policy and allocate NAT entry
    }

    if (addr_entry) {
        if (ctx.key().flow_type == FLOW_TYPE_V4) {
            if (addr_entry->tgt_vrf_id != ctx.key().dvrf_id) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dvrf_id, addr_entry->tgt_vrf_id);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dip, addr_entry->tgt_ip_addr.addr.v4_addr);
        } else {
            if (addr_entry->tgt_vrf_id != ctx.key().dvrf_id) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dvrf_id, addr_entry->tgt_vrf_id);
            }
            HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dip, addr_entry->tgt_ip_addr.addr.v6_addr);
        }

        // Store the original info for rflow snat
        nat_info->nat_sip.af = addr_key.ip_addr.af;
        nat_info->nat_sip.addr = ctx.key().dip;
        if (addr_entry->tgt_vrf_id != ctx.key().svrf_id) {
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
