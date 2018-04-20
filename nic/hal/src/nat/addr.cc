//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles Config APIs for NAT addr
//-----------------------------------------------------------------------------

#include "nat.hpp"

namespace hal {

static inline nat_cfg_addr_t *
nat_cfg_addr_alloc (void)
{
    return ((nat_cfg_addr_t *)g_hal_state->nat_cfg_addr_slab()->alloc());
}

static inline void
nat_cfg_addr_init (nat_cfg_addr_t *addr)
{
    dllist_reset(&addr->lentry);
}

static hal_ret_t
nat_cfg_rule_match_dst_addr_spec_extract (nat::NatRuleSpec& spec,
                                          nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;
    nat_cfg_addr_t *addr;

    uint32_t sz = spec.dst_address_size();
    for (uint32_t i = 0; i < sz; i++) {
        if ((addr = nat_cfg_addr_alloc()) == NULL)
            return HAL_RET_OOM;
        nat_cfg_addr_init(addr);
        if (spec.dst_address(i).has_address()) {
            if (spec.dst_address(i).address().has_prefix()) {
                addr->range.af = IP_AF_IPV4;
                addr->range.ip_lo.v4_addr = addr->range.ip_hi.v4_addr =
                    spec.dst_address(i).address().prefix().
                        ipv4_subnet().address().v4_addr();
            }
        }
        dllist_add_tail(&match->dst_addr_list, &addr->lentry);
    }

    return ret;
}

static hal_ret_t
nat_cfg_rule_match_src_addr_spec_extract (nat::NatRuleSpec& spec,
                                          nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;
    nat_cfg_addr_t *addr;

    uint32_t sz = spec.src_address_size();
    for (uint32_t i = 0; i < sz; i++) {
        if ((addr = nat_cfg_addr_alloc()) == NULL)
            return HAL_RET_OOM;
        nat_cfg_addr_init(addr);
        if (spec.src_address(i).has_address()) {
            if (spec.src_address(i).address().has_prefix()) {
                addr->range.af = IP_AF_IPV4;
                addr->range.ip_lo.v4_addr = addr->range.ip_hi.v4_addr =
                    spec.src_address(i).address().prefix().
                        ipv4_subnet().address().v4_addr();
            }
        }
        dllist_add_tail(&match->src_addr_list, &addr->lentry);
    }

    return ret;
}

hal_ret_t
nat_cfg_rule_match_addr_spec_extract (nat::NatRuleSpec& spec,
                                      nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_rule_match_src_addr_spec_extract(
           spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_match_dst_addr_spec_extract(
           spec, match)) != HAL_RET_OK)
        return ret;

    return ret;
}

} // namespace hal
