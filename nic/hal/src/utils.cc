#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/utils.hpp"

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
    if (ip_addr->af == types::IP_AF_INET) {
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
// Converts hal_ret_t to API status
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
        case HAL_RET_TENANT_NOT_FOUND:
            return types::API_STATUS_TENANT_NOT_FOUND;
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
        case HAL_RET_POLICER_NOT_FOUND:
            return types::API_STATUS_POLICER_NOT_FOUND;
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
        default:
            return types::API_STATUS_ERR;
            break;
    }
}




}    // namespace hal
