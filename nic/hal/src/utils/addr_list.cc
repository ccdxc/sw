//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for types::Address & types::IPAddressObj object
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "utils.hpp"
#include "addr_list.hpp"

namespace hal {

static inline addr_list_elem_t *
addr_list_elem_alloc (uint8_t af)
{
    if (af == IP_AF_IPV4)
        return ((addr_list_elem_t *)g_hal_state->
                   v4addr_list_elem_slab()->alloc());
    else
        return ((addr_list_elem_t *)g_hal_state->
                   v6addr_list_elem_slab()->alloc());
}

static inline void
addr_list_elem_free (addr_list_elem_t *addr)
{
    if (addr->ip_range.af == IP_AF_IPV4)
        hal::delay_delete_to_slab(HAL_SLAB_V4ADDR_LIST_ELEM, addr);
    else
        hal::delay_delete_to_slab(HAL_SLAB_V6ADDR_LIST_ELEM, addr);
}

static inline void
addr_list_elem_init (addr_list_elem_t *addr)
{
    dllist_reset(&addr->list_ctxt);
}

static inline void
addr_list_elem_uninit (addr_list_elem_t *addr)
{
}

static inline addr_list_elem_t *
addr_list_elem_alloc_init (uint8_t af)
{
    addr_list_elem_t *addr;

    if ((addr = addr_list_elem_alloc(af)) ==  NULL)
        return NULL;

    addr_list_elem_init(addr);
    return addr;
}

static inline void
addr_list_elem_uninit_free (addr_list_elem_t *addr)
{
    if (addr) {
        addr_list_elem_uninit(addr);
        addr_list_elem_free(addr);
    }
}

static inline void
addr_list_elem_db_add (dllist_ctxt_t *head, addr_list_elem_t *addr)
{
    dllist_add_tail(head, &addr->list_ctxt);
}

static inline void
addr_list_elem_db_del (addr_list_elem_t *addr)
{
    dllist_del(&addr->list_ctxt);
}

static inline hal_ret_t
addr_list_elem_prefix_spec_extract (const types::Address& addr,
                                    addr_list_elem_t *addr_lelem)
{
    auto prefix = addr.prefix();
    // accepting only v4 now, others are rejected in validation phase
    if (prefix.has_ipv4_subnet()) {
        ip_subnet_spec_to_ip_range(&addr_lelem->ip_range, prefix);
        return HAL_RET_OK;
    }

    return HAL_RET_INVALID_ARG;
}

static inline hal_ret_t
addr_list_elem_range_spec_extract (const types::Address& addr,
                                   addr_list_elem_t *addr_lelem)
{
    auto range = addr.range();
    // accepting only v4 now, others are rejected in validation phase
    if (range.has_ipv4_range()) {
        ip_range_spec_to_ip_range(&addr_lelem->ip_range, range);
        return HAL_RET_OK;
    }

    return HAL_RET_INVALID_ARG;
}

static inline hal_ret_t
addr_list_elem_address_spec_extract (const types::Address& addr,
                                     addr_list_elem_t *addr_lelem)
{
    hal_ret_t ret;

    if (addr.has_range()) {
        if ((ret = addr_list_elem_range_spec_extract(
                addr, addr_lelem)) != HAL_RET_OK)
            return ret;
    } else if (addr.has_prefix()) {
        if ((ret = addr_list_elem_prefix_spec_extract(
                addr, addr_lelem)) != HAL_RET_OK)
            return ret;
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
addr_list_elem_address_prefix_spec_validate (const types::Address& addr,
                                             uint8_t *af)
{
    auto prefix = addr.prefix();

    *af = IP_AF_IPV4;
    if (prefix.has_ipv4_subnet())
        return HAL_RET_OK;
    else if (prefix.has_ipv6_subnet())
        return HAL_RET_NOT_SUPPORTED;

    return HAL_RET_INVALID_ARG;
}

static inline hal_ret_t
addr_list_elem_address_range_spec_validate (const types::Address& addr,
                                            uint8_t *af)
{
    auto range = addr.range();

    *af = IP_AF_IPV4;
    if (range.has_ipv4_range())
        return HAL_RET_OK;
    else if (range.has_ipv6_range())
        return HAL_RET_NOT_SUPPORTED;

    return HAL_RET_INVALID_ARG;
}

static inline hal_ret_t
addr_list_elem_address_spec_validate (const types::Address& addr, uint8_t *af)
{
    if (addr.has_range())
        return addr_list_elem_address_range_spec_validate(addr, af);
    else if (addr.has_prefix())
        return addr_list_elem_address_prefix_spec_validate(addr, af);

    return HAL_RET_INVALID_ARG;
}

addr_list_elem_t *
addr_list_elem_address_spec_handle (const types::Address& addr,
                                    dllist_ctxt_t *head)
{
    uint8_t af;
    hal_ret_t ret;
    addr_list_elem_t *addr_lelem;

    if ((ret = addr_list_elem_address_spec_validate(
            addr, &af)) != HAL_RET_OK)
        return NULL;

    if ((addr_lelem = addr_list_elem_alloc_init(af)) == NULL)
        return NULL;

    if ((ret = addr_list_elem_address_spec_extract(
            addr, addr_lelem)) != HAL_RET_OK) {
        addr_list_elem_uninit_free(addr_lelem);
        return NULL;
    }

    addr_list_elem_db_add(head, addr_lelem);
    return addr_lelem;
}

static inline hal_ret_t
addr_list_elem_ipaddressobj_spec_extract (const types::IPAddressObj& addr,
                                          addr_list_elem_t *addr_lelem)
{
    hal_ret_t ret = HAL_RET_OK;

    //todo: deal with type
    if (addr.has_address()) {
        if ((ret = addr_list_elem_address_spec_extract(
                addr.address(), addr_lelem)) != HAL_RET_OK)
            return ret;
    }

    addr_lelem->negate = addr.negate();
    return HAL_RET_OK;
}

static inline hal_ret_t
addr_list_elem_ipaddressobj_spec_validate (const types::IPAddressObj& addr,
                                           uint8_t *af)
{
    return addr_list_elem_address_spec_validate(addr.address(), af);
}

hal_ret_t
addr_list_elem_ipaddressobj_spec_handle (const types::IPAddressObj& addr,
                                         dllist_ctxt_t *head)
{
    uint8_t af;
    hal_ret_t ret;
    addr_list_elem_t *addr_lelem;

    if ((ret = addr_list_elem_ipaddressobj_spec_validate(
            addr, &af)) != HAL_RET_OK)
        return ret;

    if ((addr_lelem = addr_list_elem_alloc_init(af)) == NULL)
        return HAL_RET_OOM;

    if ((ret = addr_list_elem_ipaddressobj_spec_extract(
            addr, addr_lelem)) != HAL_RET_OK) {
        addr_list_elem_uninit_free(addr_lelem);
        return ret;
    }

    addr_list_elem_db_add(head, addr_lelem);
    return HAL_RET_OK;
}

void
addr_list_cleanup (dllist_ctxt_t *head)
{
    dllist_ctxt_t       *curr, *next;
    addr_list_elem_t    *addr;

    dllist_for_each_safe(curr, next, head) {
        addr = dllist_entry(curr, addr_list_elem_t, list_ctxt);
        addr_list_elem_db_del(addr);
        addr_list_elem_free(addr);
    }
}

bool
addr_in_addr_list_elem (ip_addr_t *addr, addr_list_elem_t *addr_list_elem)
{
    if (addr->af != addr_list_elem->ip_range.af) {
        return false;
    }
    switch (addr->af) {
    case IP_AF_IPV4:
        if ((addr->addr.v4_addr >=
                 addr_list_elem->ip_range.vx_range[0].v4_range.ip_lo) &&
            (addr->addr.v4_addr <=
                 addr_list_elem->ip_range.vx_range[0].v4_range.ip_hi)) {
            return true;
        }
        return false;

    case IP_AF_IPV6:
    default:
        return false;
        break;
    }
}

hal_ret_t
addr_offset (ip_addr_t *addr, addr_list_elem_t *addr_list_elem,
             uint32_t *offset)
{
    switch (addr->af) {
    case IP_AF_IPV4:
        *offset =
            addr->addr.v4_addr -
                addr_list_elem->ip_range.vx_range[0].v4_range.ip_lo;
        return HAL_RET_OK;
        break;

    case IP_AF_IPV6:
    default:
        break;
    }
    return HAL_RET_NOT_SUPPORTED;
}

} // namespace hal
