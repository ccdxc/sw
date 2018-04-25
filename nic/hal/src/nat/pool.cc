//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// NAT pool address and/or port management related APIs
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/src/nat/nat.hpp"
//#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/utils/addr_list.hpp"

using hal::utils::nat::addr_entry_key_t;
using hal::utils::nat::addr_entry_t;

namespace hal {

//-----------------------------------------------------------------------------
// given a NAT pool, allocate a free address from the pool
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_address_alloc (addr_entry_key_t *key, nat_pool_t *pool,
                        addr_entry_key_t *mapping)
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
    mapping->vrf_id = pool->key.vrf_id;
    switch (addr_range->ip_range.af) {
    case IP_AF_IPV4:
        mapping->ip_addr.af = IP_AF_IPV4;
        mapping->ip_addr.addr.v4_addr =
            addr_range->ip_range.vx_range[0].v4_range.ip_lo + free_idx;
        break;
    case IP_AF_IPV6:
    default:
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
nat_pool_address_free (addr_entry_key_t *key, nat_pool_t *pool)
{
    return HAL_RET_OK;
}

}    // namespace hal
