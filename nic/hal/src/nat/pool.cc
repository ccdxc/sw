//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// NAT pool address and/or port management related APIs
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/src/nat/nat.hpp"
#include "nic/hal/src/utils/addr_list.hpp"

using hal::utils::nat::addr_entry_key_t;
using hal::utils::nat::addr_entry_t;

namespace hal {

//-----------------------------------------------------------------------------
// given a NAT pool, allocate a free address from the pool
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_address_alloc (nat_pool_t *pool, ip_addr_t *nat_addr)
{
    hal_ret_t           ret;
    dllist_ctxt_t       *curr;
    addr_list_elem_t    *addr_range;
    uint32_t            free_idx;

    // if all address in this NAT pool are in use, fail allocation
    if (pool->num_free == 0) {
        HAL_TRACE_ERR("NAT addr allocation failed, capacity reached");
        return HAL_RET_NO_RESOURCE;
    }

    HAL_SPINLOCK_LOCK(&pool->slock);
    ret = pool->addr_bmap->first_free(&free_idx);
    if (ret != HAL_RET_OK) {
        // we are supposed to find free NAT address, but didn't
        HAL_TRACE_ERR("NAT address allocation failure, unexpected err : {}",
                      ret);
        goto error;
    }
    if (free_idx < pool->num_addrs) {
        HAL_TRACE_ERR("NAT address allocation failure, invalid free idx {}",
                      free_idx);
        ret = HAL_RET_ERR;
        goto error;
    }

    // skip all the ranges to get to the range with free NAT address
    dllist_for_each(curr, &pool->addr_ranges) {
        addr_range = dllist_entry(curr, addr_list_elem_t, list_ctxt);
        if (free_idx < addr_range->num_addrs) {
            break;
        } else {
            free_idx -= addr_range->num_addrs;
        }
    }
    if (addr_range == NULL) {
        HAL_TRACE_ERR("NAT address allocation failure, addr range not found, "
                      "free idx {}", free_idx);
        ret = HAL_RET_ERR;
        goto error;
    }

    // allocate the NAT address now
    switch (addr_range->ip_range.af) {
    case IP_AF_IPV4:
        nat_addr->af = IP_AF_IPV4;
        nat_addr->addr.v4_addr =
            addr_range->ip_range.vx_range[0].v4_range.ip_lo + free_idx;
        pool->addr_bmap->set(free_idx);
        HAL_TRACE_DEBUG("Allocated NAT IP {} from pool ({}, {})",
                        ipaddr2str(nat_addr),
                        pool->key.vrf_id, pool->key.pool_id);
        break;

    case IP_AF_IPV6:
    default:
        HAL_TRACE_ERR("NAT address allocation failure, address family unknown");
        ret = HAL_RET_NO_RESOURCE;
        goto error;
    }

error:

    HAL_SPINLOCK_UNLOCK(&pool->slock);
    return ret;
}

//-----------------------------------------------------------------------------
// given a NAT pool and NAT address, free the NAT address back to the pool
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_address_free (nat_pool_t *pool, ip_addr_t *nat_addr)
{
    hal_ret_t           ret;
    dllist_ctxt_t       *curr;
    addr_list_elem_t    *addr_range;
    uint32_t            num_addrs, offset;

    dllist_for_each(curr, &pool->addr_ranges) {
        addr_range = dllist_entry(curr, addr_list_elem_t, list_ctxt);
        if (addr_in_addr_list_elem(nat_addr, addr_range)) {
            break;
        }
        num_addrs += addr_range->num_addrs;
    }
    if (addr_range == NULL) {
        HAL_TRACE_DEBUG("Failed to free address {} to NAT pool ({}, {})",
                        ipaddr2str(nat_addr),
                        pool->key.vrf_id, pool->key.pool_id);
        return HAL_RET_ERR;
    }
    ret = addr_offset(nat_addr, addr_range, &offset);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to compute offset of addr {} in NAT pool "
                      "({}, {}), err : {}", ipaddr2str(nat_addr),
                      pool->key.vrf_id, pool->key.pool_id, ret);
        return ret;
    }

    // reset the bit corresponding to this address in the NAT pool
    ret = pool->addr_bmap->clear(num_addrs + offset);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to reset the bit corresponding to the addr {} in "
                      "NAT pool ({}, {}), err : {}", ipaddr2str(nat_addr),
                      pool->key.vrf_id, pool->key.pool_id, ret);
    }
    return ret;
}

}    // namespace hal
