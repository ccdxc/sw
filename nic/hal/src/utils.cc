#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/utils.hpp"
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
// convert IP prefix spec in proto to ip_addr used in HAL
//----------------------------------------------------------------------------
hal_ret_t
ip_pfx_spec_to_pfx_spec (ip_prefix_t *ip_pfx, 
                         const types::IPPrefix& in_ippfx)
{
    hal_ret_t ret = HAL_RET_OK;

    ip_pfx->len = in_ippfx.prefix_len();
    ret = ip_addr_spec_to_ip_addr(&ip_pfx->addr, in_ippfx.address());
    return ret;
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
        return types::API_STATUS_VRF_NOT_FOUND;
        break;
    case HAL_RET_L2SEG_NOT_FOUND:
        return types::API_STATUS_L2_SEGMENT_NOT_FOUND;
        break;
    case HAL_RET_IF_NOT_FOUND:
        return types::API_STATUS_INTERFACE_NOT_FOUND;
        break;
    case HAL_RET_SECURITY_PROFILE_NOT_FOUND:
        return types::API_STATUS_NWSEC_PROFILE_NOT_FOUND;
        break;
    case HAL_RET_QOS_CLASS_NOT_FOUND:
        return types::API_STATUS_QOS_CLASS_NOT_FOUND;
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
    fmt::MemoryWriter   buf;

    if (!trace) return;

    for (int i = 0; i < NUM_DASHES; i++) {
        buf.write("{}", "-");
    }
    buf.write("{}", trace);
    for (int i = 0; i < NUM_DASHES; i++) {
        buf.write("{}", "-");
    }
    HAL_TRACE_DEBUG(buf.c_str());
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
        HAL_TRACE_ERR("{}:invalid args. bl:{:#x}, handle:{}",
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
        HAL_TRACE_ERR("{}:invalid args. bl:{:#x}, handle:{}",
                      (uint64_t)bl, handle);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ret = bl->insert(&handle);

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
        HAL_TRACE_DEBUG("{}: freeing list handle: {}", __FUNCTION__, entry->handle_id);
        sdk::lib::dllist_del(&entry->dllist_ctxt);
        g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
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
        HAL_TRACE_ERR("{}:invalid args. bl:{:#x}",
                      (uint64_t)bl);
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
        HAL_TRACE_ERR("{}:invalid args. bl:{:#x}",
                      (uint64_t)bl);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }
    

    block_list::destroy(*bl);
    *bl = NULL;

end:

    return ret;
}

//-----------------------------------------------------------------------------
// demangling symbols for custom backtrace
//-----------------------------------------------------------------------------
std::string
demangle (const char* const symbol)
{
    const std::unique_ptr< char, decltype( &std::free ) > demangled(
            abi::__cxa_demangle( symbol, 0, 0, 0 ), &std::free );
    if (demangled ) {
        return demangled.get();
    }
    else {
        return symbol;
    }
}

//-----------------------------------------------------------------------------
// prints the 2nd frame in the BT
//   x -> y -> custom_backtrace
//   prints the x frame
//-----------------------------------------------------------------------------
void
custom_backtrace (void)
{
    // TODO: replace hardcoded limit?               
    void* addresses[ 256 ];
    const int n = ::backtrace( addresses, std::extent< decltype( addresses ) >::value );
    const std::unique_ptr< char*, decltype( &std::free ) > symbols(
            ::backtrace_symbols( addresses, n ), &std::free );
    for( int i = 0; i < n; ++i ) {
        if (i != 2) {
            continue;
        }
        // we parse the symbols retrieved from backtrace_symbols() to
        // extract the "real" symbols that represent the mangled names.  
        char* const symbol = symbols.get()[ i ];
        char* end = symbol;
        while( *end ) {
            ++end;
        }
        // scanning is done backwards, since the module name
        // might contain both '+' or '(' characters.
        while( end != symbol && *end != '+' ) {
            --end;
        }
        char* begin = end;
        while( begin != symbol && *begin != '(' ) {
            --begin;
        }

        if( begin != symbol ) {
            // std::cout << std::string( symbol, ++begin - symbol );
            *end++ = '\0';
            std::cout << demangle( begin ) << '+' << end;
        }
        else {
            std::cout << symbol;
        }
        // Revisit: Line number not working. 
#if 0
        // For line number
        size_t p = 0;
        while(symbol[p] != '(' && symbol[p] != ' '
                && symbol[p] != 0)
            ++p;
        char syscom[256];
        sprintf(syscom,"addr2line %p -e %.*s", addresses[i], (int)p, symbol);
        //last parameter is the file name of the symbol
        system(syscom);
#endif
        std::cout << std::endl;
    }
}

}    // namespace hal
