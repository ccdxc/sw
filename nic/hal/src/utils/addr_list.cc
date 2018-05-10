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
            addr, &af)) != HAL_RET_OK) {
        HAL_TRACE_ERR("Spec validate failure");    
        return NULL;
    }

    if ((addr_lelem = addr_list_elem_alloc_init(af)) == NULL) {
        HAL_TRACE_ERR("Address list alloc failure");    
        return NULL;
    }

    if ((ret = addr_list_elem_address_spec_extract(
            addr, addr_lelem)) != HAL_RET_OK) {
        HAL_TRACE_ERR("Spec extract failure");    
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

hal_ret_t
addr_list_elem_spec_src_addr_build (dllist_ctxt_t *head,
                           types::RuleMatch *spec)
{
    dllist_ctxt_t       *entry;
    addr_list_elem_t    *addr_elem;

    dllist_for_each(entry, head) {
        auto addr = spec->add_src_address();
        addr_elem = dllist_entry(entry, addr_list_elem_t, list_ctxt);
        ip_range_to_spec(addr->mutable_address()->mutable_range(), &addr_elem->ip_range);
        addr->set_negate(addr_elem->negate);
    }

    return HAL_RET_OK;
}

hal_ret_t
addr_list_elem_spec_dst_addr_build (dllist_ctxt_t *head,
                           types::RuleMatch *spec)
{
    dllist_ctxt_t       *entry;
    addr_list_elem_t    *addr_elem;

    dllist_for_each(entry, head) {
        auto addr = spec->add_dst_address();
        addr_elem = dllist_entry(entry, addr_list_elem_t, list_ctxt);
        ip_range_to_spec(addr->mutable_address()->mutable_range(), &addr_elem->ip_range);
        addr->set_negate(addr_elem->negate);
    }

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

// Mac address list methods
static inline mac_addr_list_elem_t *
mac_addr_list_elem_alloc ()
{
    return ((mac_addr_list_elem_t *)g_hal_state->
             mac_addr_list_elem_slab()->alloc());
}

static inline void
mac_addr_list_elem_free (mac_addr_list_elem_t *addr)
{
    hal::delay_delete_to_slab(HAL_SLAB_MACADDR_LIST_ELEM, addr);
}

static inline void
mac_addr_list_elem_init (mac_addr_list_elem_t *addr)
{
    dllist_reset(&addr->list_ctxt);
}

static inline void
mac_addr_list_elem_uninit (mac_addr_list_elem_t *addr)
{
}

static inline mac_addr_list_elem_t *
mac_addr_list_elem_alloc_init ()
{
    mac_addr_list_elem_t *addr;

    if ((addr = mac_addr_list_elem_alloc()) ==  NULL)
        return NULL;

    mac_addr_list_elem_init(addr);
    return addr;
}

static inline void
mac_addr_list_elem_uninit_free (mac_addr_list_elem_t *addr)
{
    if (addr) {
        mac_addr_list_elem_uninit(addr);
        mac_addr_list_elem_free(addr);
    }
}

static inline void
mac_addr_list_elem_add (dllist_ctxt_t *head, mac_addr_list_elem_t *addr)
{
    dllist_add_tail(head, &addr->list_ctxt);
}

static inline void
mac_addr_list_elem_del (mac_addr_list_elem_t *addr)
{
    dllist_del(&addr->list_ctxt);
}

void
mac_addr_list_cleanup (dllist_ctxt_t *head)
{
    dllist_ctxt_t       *curr, *next;
    mac_addr_list_elem_t    *addr;

    dllist_for_each_safe(curr, next, head) {
        addr = dllist_entry(curr, mac_addr_list_elem_t, list_ctxt);
        mac_addr_list_elem_del(addr);
        mac_addr_list_elem_free(addr);
    }
}

hal_ret_t
mac_addr_elem_add (uint64_t mac_addr, dllist_ctxt_t *head)
{
    mac_addr_list_elem_t *elem;
    if ((elem = mac_addr_list_elem_alloc_init()) == NULL) {
        return HAL_RET_OOM;
    }
    
    elem->addr = mac_addr;
    mac_addr_list_elem_add(head, elem);
    
    return HAL_RET_OK;
}

hal_ret_t
mac_addr_elem_delete (uint64_t mac_addr, dllist_ctxt_t *head)
{
    dllist_ctxt_t        *curr, *next;
    mac_addr_list_elem_t *elem;
    bool                 found = false; 

    dllist_for_each_safe(curr, next, head) {
        elem = dllist_entry(curr, mac_addr_list_elem_t, list_ctxt);
        if (elem->addr == mac_addr) {
            mac_addr_list_elem_del(elem);
            mac_addr_list_elem_free(elem);
            found = true;
        }
    }
    if (!found)
        return HAL_RET_ENTRY_NOT_FOUND;

    return HAL_RET_OK;
}

} // namespace hal
