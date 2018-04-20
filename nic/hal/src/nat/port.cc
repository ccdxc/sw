//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles Config APIs for NAT port
//-----------------------------------------------------------------------------

#include "nat.hpp"

namespace hal {

static inline nat_cfg_port_t *
nat_cfg_port_alloc (void)
{
    return ((nat_cfg_port_t *)g_hal_state->nat_cfg_port_slab()->alloc());
}

static inline void
nat_cfg_port_init (nat_cfg_port_t *port)
{
    dllist_reset(&port->lentry);
}

static hal_ret_t
nat_cfg_rule_match_dst_port_spec_extract (nat::NatRuleSpec& spec,
                                          nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;
    nat_cfg_port_t *port;

    uint32_t sz = spec.dst_port_range_size();
    for (uint32_t i = 0; i < sz; i++) {
        if ((port = nat_cfg_port_alloc()) == NULL)
            return HAL_RET_OOM;
        nat_cfg_port_init(port);
        port->range.port_hi = spec.dst_port_range(i).port_high();
        port->range.port_lo = spec.dst_port_range(i).port_low();
        dllist_add_tail(&match->dst_port_list, &port->lentry);
    }

    return ret;
}

static hal_ret_t
nat_cfg_rule_match_src_port_spec_extract (nat::NatRuleSpec& spec,
                                          nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;
    nat_cfg_port_t *port;

    uint32_t sz = spec.src_port_range_size();
    for (uint32_t i = 0; i < sz; i++) {
        if ((port = nat_cfg_port_alloc()) == NULL)
            return HAL_RET_OOM;
        nat_cfg_port_init(port);
        port->range.port_hi = spec.src_port_range(i).port_high();
        port->range.port_lo = spec.src_port_range(i).port_low();
        dllist_add_tail(&match->src_port_list, &port->lentry);
    }

    return ret;
}

hal_ret_t
nat_cfg_rule_match_port_spec_extract (nat::NatRuleSpec& spec,
                                      nat_cfg_rule_match_t *match)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_rule_match_src_port_spec_extract(
           spec, match)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_rule_match_dst_port_spec_extract(
           spec, match)) != HAL_RET_OK)
        return ret;

    return ret;
}

} // namespace hal
