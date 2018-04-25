//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include <cxxabi.h>
#include <execinfo.h>
#include <iostream>

using types::ApiStatus;

namespace hal {

//----------------------------------------------------------------------------
// convert IP address spec in proto to ip_addr used in HAL
//----------------------------------------------------------------------------
hal_ret_t
ip_addr_spec_to_ip_addr (ip_addr_t *out_ipaddr,
                         const types::IPAddress& in_ipaddr)
{
    memset(out_ipaddr, 0, sizeof(ip_addr_t));
    if (in_ipaddr.ip_af() == types::IP_AF_INET) {
        out_ipaddr->af = IP_AF_IPV4;
        out_ipaddr->addr.v4_addr = in_ipaddr.v4_addr();
    } else if (in_ipaddr.ip_af() == types::IP_AF_INET6) {
        out_ipaddr->af = IP_AF_IPV6;
        memcpy(out_ipaddr->addr.v6_addr.addr8,
               in_ipaddr.v6_addr().c_str(),
               IP6_ADDR8_LEN);
    } else {
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// convert HAL IP address to spec
//----------------------------------------------------------------------------
hal_ret_t
ip_addr_to_spec (types::IPAddress *ip_addr_spec,
                 const ip_addr_t *ip_addr)
{
    if (ip_addr->af == IP_AF_IPV4) {
        ip_addr_spec->set_ip_af(types::IP_AF_INET);
        ip_addr_spec->set_v4_addr(ip_addr->addr.v4_addr);
    } else {
        ip_addr_spec->set_ip_af(types::IP_AF_INET6);
        ip_addr_spec->set_v6_addr(ip_addr->addr.v6_addr.addr8, IP6_ADDR8_LEN);
    }

    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// convert IP prefix to IPPrefix proto spec
//----------------------------------------------------------------------------
hal_ret_t
ip_pfx_to_spec (types::IPPrefix *ip_pfx_spec,
                const ip_prefix_t *ip_pfx)
{
    hal_ret_t ret = HAL_RET_OK;

    ip_pfx_spec->set_prefix_len(ip_pfx->len);
    ret = ip_addr_to_spec(ip_pfx_spec->mutable_address(), &ip_pfx->addr);
    return ret;
}

//----------------------------------------------------------------------------
// convert IP prefix spec in proto to ip_addr used in HAL
//----------------------------------------------------------------------------
hal_ret_t
ip_pfx_spec_to_pfx (ip_prefix_t *ip_pfx, const types::IPPrefix& in_ippfx)
{
    hal_ret_t ret = HAL_RET_OK;

    ip_pfx->len = in_ippfx.prefix_len();
    if (((in_ippfx.address().ip_af() == types::IP_AF_INET) &&
             (ip_pfx->len > 32)) ||
        ((in_ippfx.address().ip_af() == types::IP_AF_INET6) &&
             (ip_pfx->len > 128))) {
        ret = HAL_RET_INVALID_ARG;
    } else {
        ret = ip_addr_spec_to_ip_addr(&ip_pfx->addr, in_ippfx.address());
    }
    return ret;
}

//----------------------------------------------------------------------------
// convert IP range spec in proto to IP range used in HAL
//----------------------------------------------------------------------------
hal_ret_t
ip_range_spec_to_ip_range (ip_range_t *range, const types::AddressRange& spec)
{
    if (spec.has_ipv4_range()) {
        range->af = IP_AF_IPV4;
        range->vx_range[0].v4_range.ip_lo =
            spec.ipv4_range().low_ipaddr().v4_addr();
        range->vx_range[0].v4_range.ip_hi =
            spec.ipv4_range().high_ipaddr().v4_addr();
    } else if (spec.has_ipv6_range()) {
        return HAL_RET_NOT_SUPPORTED;
    } else {
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// convert IP prefix spec in proto to IP address range used in HAL
//----------------------------------------------------------------------------
hal_ret_t
ip_subnet_spec_to_ip_range (ip_range_t *range, const ::types::IPSubnet& spec)
{
    if (spec.has_ipv4_subnet()) {
        range->af = IP_AF_IPV4;
        range->vx_range[0].v4_range.ip_lo =
            spec.ipv4_subnet().address().v4_addr() &
                ~((1 << (32 - spec.ipv4_subnet().prefix_len())) - 1);
        range->vx_range[0].v4_range.ip_hi =
            range->vx_range[0].v4_range.ip_lo +
                (1 << (32 - spec.ipv4_subnet().prefix_len())) - 1;
    } else if (spec.has_ipv6_subnet()) {
        return HAL_RET_NOT_SUPPORTED;
    } else {
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// convert HAL IP range to IP range proto spec
//----------------------------------------------------------------------------
hal_ret_t
ip_range_to_spec (types::AddressRange *spec, ip_range_t *range)
{
    if (range->af == IP_AF_IPV4) {
        auto v4_range = spec->mutable_ipv4_range();
        v4_range->mutable_low_ipaddr()->set_v4_addr(range->vx_range[0].v4_range.ip_lo);
        v4_range->mutable_high_ipaddr()->set_v4_addr(range->vx_range[0].v4_range.ip_hi);
    } else if (range->af == IP_AF_IPV6) {
        return HAL_RET_NOT_SUPPORTED;
    } else {
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// check if 2 IP addresses are equal
//----------------------------------------------------------------------------
bool
ip_addr_check_equal (ip_addr_t *ipaddr1, ip_addr_t *ipaddr2)
{
    if (!ipaddr1 || !ipaddr2) {
        return false;
    }

    if (ipaddr1->af != ipaddr2->af) {
        return false;
    }

    if (ipaddr1->af == IP_AF_IPV4) {
        if (ipaddr1->addr.v4_addr != ipaddr2->addr.v4_addr) {
            return false;
        }
    } else {
        if (memcmp(&(ipaddr1->addr.v6_addr), &(ipaddr2->addr.v6_addr), sizeof(uint8_t) * IP6_ADDR8_LEN)) {
            return false;
        }
    }

    return true;
}

//----------------------------------------------------------------------------
// check if IP address is in IP prefix
//----------------------------------------------------------------------------
bool
ip_addr_in_ip_pfx (ip_addr_t *ipaddr, ip_prefix_t *ip_pfx)
{
    int              num_bytes = 0, last_byte = 0;
    int              num_bits_in_last_byte = 0;
    uint8_t          *pos1 = NULL, *pos2 = NULL;
    unsigned char    mask = 0;

    if (!ipaddr || !ip_pfx) {
        return false;
    }

    if (ipaddr->af != ip_pfx->addr.af) {
        return false;
    }

    num_bytes = ip_pfx->len >> 3;
    last_byte = (ip_pfx->len & 0x7) ? num_bytes + 1 : -1;

    if (ipaddr->af == IP_AF_IPV4) {
        pos1 = ipaddr->addr.v6_addr.addr8 + 4;
        pos2 = ip_pfx->addr.addr.v6_addr.addr8 + 4;
        while (num_bytes) {
            if (*pos1 != *pos2) {
                return false;
            }
            num_bytes--;
            pos1--;
            pos2--;
        }

        // compare last byte
        if (last_byte != -1) {
            num_bits_in_last_byte = ip_pfx->len & 0x7;
            mask = ~((1 << (8 - num_bits_in_last_byte)) - 1);
            if ((*pos1 & mask) != (*pos2 & mask)) {
                return false;
            }
        }
    } else {
        // compare bytes
        if (memcmp(ipaddr->addr.v6_addr.addr8,
                    ip_pfx->addr.addr.v6_addr.addr8, num_bytes)) {
            return false;
        }

        // compare last byte
        if (last_byte != -1) {
            num_bits_in_last_byte = ip_pfx->len & 0x7;
            unsigned char mask = ~((1 << (8 - num_bits_in_last_byte)) - 1);
            if ((ipaddr->addr.v6_addr.addr8[last_byte] & mask) !=
                    (ip_pfx->addr.addr.v6_addr.addr8[last_byte] & mask)) {
                return false;
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------
// converts hal_ret_t to API status
//------------------------------------------------------------------------------
ApiStatus
hal_prepare_rsp (hal_ret_t ret)
{
    switch (ret) {
    case HAL_RET_OK:
        return types::API_STATUS_OK;
        break;
    case HAL_RET_HW_PROG_ERR:
        return types::API_STATUS_HW_PROG_ERR;
        break;
    case HAL_RET_TABLE_FULL:
    case HAL_RET_OTCAM_FULL:
        return types::API_STATUS_OUT_OF_RESOURCE;
        break;
    case HAL_RET_OOM:
        return types::API_STATUS_OUT_OF_MEM;
        break;
    case HAL_RET_INVALID_ARG:
        return types::API_STATUS_INVALID_ARG;
        break;
    case HAL_RET_VRF_NOT_FOUND:
        return types::API_STATUS_NOT_FOUND;
        break;
    case HAL_RET_L2SEG_NOT_FOUND:
        return types::API_STATUS_NOT_FOUND;
        break;
    case HAL_RET_IF_NOT_FOUND:
        return types::API_STATUS_NOT_FOUND;
        break;
    case HAL_RET_SECURITY_PROFILE_NOT_FOUND:
        return types::API_STATUS_NOT_FOUND;
        break;
    case HAL_RET_QOS_CLASS_NOT_FOUND:
        return types::API_STATUS_NOT_FOUND;
        break;
    case HAL_RET_HANDLE_INVALID:
        return types::API_STATUS_HANDLE_INVALID;
        break;
    case HAL_RET_IF_ENIC_TYPE_INVALID:
        return types::API_STATUS_IF_ENIC_TYPE_INVALID;
        break;
    case HAL_RET_IF_ENIC_INFO_INVALID:
        return types::API_STATUS_IF_ENIC_INFO_INVALID;
        break;
    case HAL_RET_IF_INFO_INVALID:
        return types::API_STATUS_IF_INFO_INVALID;
        break;
    case HAL_RET_VRF_ID_INVALID:
        return types::API_STATUS_VRF_ID_INVALID;
        break;
    case HAL_RET_L2SEG_ID_INVALID:
        return types::API_STATUS_L2_SEGMENT_ID_INVALID;
        break;
    case HAL_RET_NWSEC_ID_INVALID:
        return types::API_STATUS_NWSEC_PROFILE_ID_INVALID;
        break;
    case HAL_RET_ENTRY_EXISTS:
        return types::API_STATUS_EXISTS_ALREADY;
        break;
    case HAL_RET_OBJECT_IN_USE:
        return types::API_STATUS_OBJECT_IN_USE;
        break;
    case HAL_RET_ACL_NOT_FOUND:
        return types:: API_STATUS_NOT_FOUND;
        break;
    case HAL_RET_COPP_NOT_FOUND:
        return types:: API_STATUS_NOT_FOUND;
        break;
    case HAL_RET_SECURITY_POLICY_NOT_FOUND:
        return types:: API_STATUS_NOT_FOUND;
        break;
    default:
        return types::API_STATUS_ERR;
        break;
    }
}

//-----------------------------------------------------------------------------
// use this at the begin and end of a svc api
//-----------------------------------------------------------------------------
void
hal_api_trace (const char *trace)
{
    // To add as prefix and suffix to the actual trace statement
    static const std::string prefix_str("--------------------");

    if (!trace) return;

    HAL_TRACE_DEBUG("{}{}{}", prefix_str, trace, prefix_str);
}

//-----------------------------------------------------------------------------
// prints handles from the list
//-----------------------------------------------------------------------------
void
hal_print_handles_list (dllist_ctxt_t  *list)
{
    dllist_ctxt_t                   *lnode = NULL;
    hal_handle_id_list_entry_t      *entry = NULL;

    dllist_for_each(lnode, list) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        HAL_TRACE_DEBUG("handle: {}", entry->handle_id);
    }
}

//-----------------------------------------------------------------------------
// prints handles from the block list
//-----------------------------------------------------------------------------
void
hal_print_handles_block_list (block_list *bl)
{
    hal_handle_t    *p_hdl_id = NULL;

    for (const void *ptr : *bl) {
        p_hdl_id = (hal_handle_t *)ptr;
        HAL_TRACE_DEBUG("handle: {}", *p_hdl_id);
    }
}

//-----------------------------------------------------------------------------
// check if handle is present in handle list
//-----------------------------------------------------------------------------
bool
hal_handle_in_list (dllist_ctxt_t *handle_list, hal_handle_t handle)
{
    dllist_ctxt_t                   *curr, *next;
    hal_handle_id_list_entry_t      *entry = NULL;

    dllist_for_each_safe(curr, next, handle_list) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == handle) {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
// check if handle is present in handle block list
//-----------------------------------------------------------------------------
bool
hal_handle_in_block_list (block_list *bl, hal_handle_t handle)
{
    bool            is_present = false;

    if (!bl || handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("invalid args. bl:{:#x}, handle:{}",
                      (uint64_t)bl, handle);
        goto end;
    }

    is_present = bl->is_present(&handle);

end:

    return is_present;
}

//-----------------------------------------------------------------------------
// adds handle to the handles list
//-----------------------------------------------------------------------------
hal_ret_t
hal_add_to_handle_list (dllist_ctxt_t *handle_list, hal_handle_t handle)
{
    hal_ret_t                       ret = HAL_RET_OK;
    hal_handle_id_list_entry_t      *entry = NULL;

    entry = (hal_handle_id_list_entry_t *)g_hal_state->
            hal_handle_id_list_entry_slab()->alloc();
    if (entry == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    entry->handle_id = handle;
    sdk::lib::dllist_add(handle_list, &entry->dllist_ctxt);

end:

    return ret;
}

//-----------------------------------------------------------------------------
// adds handle to the handles block list
//-----------------------------------------------------------------------------
hal_ret_t
hal_add_to_handle_block_list (block_list *bl, hal_handle_t handle)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!bl || handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("invalid args bl:{:#x}, handle:{}", (uint64_t)bl, handle);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = bl->insert(&handle);

end:

    return ret;
}

//-----------------------------------------------------------------------------
// Unlinks and frees a handle entry from the handles list
//-----------------------------------------------------------------------------
hal_ret_t
hal_unlink_and_free_from_handle_list (hal_handle_id_list_entry_t *entry)
{
    HAL_ASSERT(entry);
    HAL_TRACE_DEBUG("freeing list handle: {}", entry->handle_id);
    sdk::lib::dllist_del(&entry->dllist_ctxt);
    return hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY, entry);
}

//-----------------------------------------------------------------------------
// searches and removes a handle from the handles list
//-----------------------------------------------------------------------------
hal_ret_t
hal_remove_from_handle_list (dllist_ctxt_t *list_head, hal_handle_t handle)
{
    hal_ret_t                       ret = HAL_RET_ENTRY_NOT_FOUND;
    hal_handle_id_list_entry_t      *entry = NULL;
    dllist_ctxt_t                   *curr, *next;

    dllist_for_each_safe(curr, next, list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == handle){
            ret = hal_unlink_and_free_from_handle_list(entry);
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
// dels handle to the handles block list
//-----------------------------------------------------------------------------
hal_ret_t
hal_del_from_handle_block_list (block_list *bl, hal_handle_t handle)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!bl || handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("invalid args bl:{:#x}, handle:{}", (uint64_t)bl, handle);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = bl->remove(&handle);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// free handle entries in a list.
// please take locks if necessary outside this call.
//-----------------------------------------------------------------------------
void
hal_free_handles_list (dllist_ctxt_t *list)
{
    dllist_ctxt_t                   *curr, *next;
    hal_handle_id_list_entry_t      *entry = NULL;

    dllist_for_each_safe(curr, next, list) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        hal_unlink_and_free_from_handle_list(entry);
    }
}

//-----------------------------------------------------------------------------
// remove all elements from block list
//-----------------------------------------------------------------------------
hal_ret_t
hal_remove_all_handles_block_list (block_list *bl)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (!bl) {
        HAL_TRACE_ERR("invalid args bl:{:#x}", (uint64_t)bl);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    bl->remove_all();

end:
    return ret;
}

//-----------------------------------------------------------------------------
// clean up list
//-----------------------------------------------------------------------------
hal_ret_t
hal_cleanup_handle_list (dllist_ctxt_t **list)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (*list == NULL) {
        return ret;
    }
    hal_free_handles_list(*list);
    HAL_FREE(HAL_MEM_ALLOC_DLLIST, *list);
    *list = NULL;

    return ret;
}

//-----------------------------------------------------------------------------
// free handle entries in a block list.
// please take locks if necessary outside this call.
//-----------------------------------------------------------------------------
hal_ret_t
hal_cleanup_handle_block_list (block_list **bl)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (*bl == NULL) {
        HAL_TRACE_ERR("invalid args. bl:{:#x}", (uint64_t)bl);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }


    block_list::destroy(*bl);
    *bl = NULL;

end:

    return ret;
}

//-----------------------------------------------------------------------------
// dst = dst + src
//-----------------------------------------------------------------------------
hal_ret_t
hal_add_block_lists (block_list *dst, block_list *src)
{
    hal_handle_t    *p_hdl_id = NULL;

    for (const void *ptr : *src) {
        p_hdl_id = (hal_handle_t *)ptr;
        dst->insert(p_hdl_id);
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dst = dst - src
//-----------------------------------------------------------------------------
hal_ret_t
hal_del_block_lists (block_list *dst, block_list *src)
{
    hal_handle_t    *p_hdl_id = NULL;

    for (const void *ptr : *src) {
        p_hdl_id = (hal_handle_t *)ptr;
        dst->remove(p_hdl_id);
    }
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Copy src to dst
//-----------------------------------------------------------------------------
hal_ret_t
hal_copy_block_lists (block_list *dst, block_list *src)
{
    // Clean up dst
    hal_remove_all_handles_block_list(dst);

    // Add src to empty dst
    hal_add_block_lists(dst, src);

    return HAL_RET_OK;
}

}    // namespace hal
