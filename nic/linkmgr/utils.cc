#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/utils.hpp"
#include <cxxabi.h>
#include <execinfo.h>
#include <iostream>

#include "nic/include/hal_lock.hpp"

#include "sdk/ht.hpp"
#include "sdk/list.hpp"

#include "linkmgr_src.hpp"
#include "linkmgr_state.hpp"

using types::ApiStatus;

namespace hal {

namespace utils {
hal::utils::mem_mgr     g_hal_mem_mgr;
}

// thread local variables
thread_local cfg_db_ctxt_t t_cfg_db_ctxt;

bool gl_super_user = false;

void
hal_handle_cfg_db_lock(bool readlock, bool lock)
{
    if (readlock == true) {
        if (lock == true) {
            linkmgr::g_linkmgr_state->cfg_db_rlock(true);
        } else {
            linkmgr::g_linkmgr_state->cfg_db_rlock(false);
        }
    } else {
        if (lock == true) {
            linkmgr::g_linkmgr_state->cfg_db_wlock(true);
        } else {
            linkmgr::g_linkmgr_state->cfg_db_wlock(false);
        }
    }
}

slab *
hal_handle_slab()
{
    return linkmgr::g_linkmgr_state->hal_handle_slab();
}

slab *
hal_handle_ht_entry_slab()
{
    return linkmgr::g_linkmgr_state->hal_handle_ht_entry_slab();
}

ht *
hal_handle_id_ht()
{
    return linkmgr::g_linkmgr_state->hal_handle_id_ht();
}

//------------------------------------------------------------------------------
// free an element back to given slab specified by its id
//------------------------------------------------------------------------------
hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    switch (slab_id) {
    case HAL_SLAB_HANDLE:
        linkmgr::g_linkmgr_state->hal_handle_slab()->free(elem);
        break;

    case HAL_SLAB_HANDLE_HT_ENTRY:
        linkmgr::g_linkmgr_state->hal_handle_ht_entry_slab()->free(elem);
        break;

    case HAL_SLAB_HANDLE_ID_HT_ENTRY:
        linkmgr::g_linkmgr_state->hal_handle_id_ht_entry_slab()->free(elem);
        break;

    case HAL_SLAB_PORT:
        linkmgr::g_linkmgr_state->port_slab()->free(elem);
        break;

    default:
        HAL_TRACE_ERR("Unknown slab id {}", slab_id);
        HAL_ASSERT(FALSE);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

namespace pd {

hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    switch (slab_id) {
    case HAL_SLAB_PORT_PD:
        //TODO cleanup
        //linkmgr::pd::g_linkmgr_state_pd->port_slab()->free(elem);
        break;

    default:
        HAL_TRACE_ERR("Unknown slab id {}", slab_id);
        HAL_ASSERT(FALSE);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

} /* namespace pd */

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
    default:
        return types::API_STATUS_ERR;
        break;
    }
}

// ----------------------------------------------------------------------------
// Use this at the begin and end of a svc api
// ----------------------------------------------------------------------------
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
    HAL_TRACE_DEBUG("{}", buf.c_str());
}

port_speed_t
port_speed_spec_to_sdk_port_speed (::port::PortSpeed speed)
{
    switch (speed) {
    case ::port::PORT_SPEED_10G:
        return port_speed_t::PORT_SPEED_10G;
        break;
    case ::port::PORT_SPEED_25G:
        return port_speed_t::PORT_SPEED_25G;
        break;
    case ::port::PORT_SPEED_100G:
        return port_speed_t::PORT_SPEED_100G;
        break;
    default:
        return port_speed_t::PORT_SPEED_NONE;
    }

    return port_speed_t::PORT_SPEED_NONE;
}

::port::PortSpeed
sdk_port_speed_to_port_speed_spec (port_speed_t speed)
{
    switch (speed) {
    case port_speed_t::PORT_SPEED_10G:
        return ::port::PORT_SPEED_10G;
        break;
    case port_speed_t::PORT_SPEED_25G:
        return ::port::PORT_SPEED_25G;
        break;
    case port_speed_t::PORT_SPEED_100G:
        return ::port::PORT_SPEED_100G;
        break;
    default:
        return ::port::PORT_SPEED_NONE;
    }

    return ::port::PORT_SPEED_NONE;
}

port_type_t
port_type_spec_to_sdk_port_type (::port::PortType type)
{
    switch (type) {
    case ::port::PORT_TYPE_ETH:
        return port_type_t::PORT_TYPE_ETH;
    default:
        return port_type_t::PORT_TYPE_NONE;
    }

    return port_type_t::PORT_TYPE_NONE;
}

::port::PortType
sdk_port_type_to_port_type_spec (port_type_t type)
{
    switch (type) {
    case port_type_t::PORT_TYPE_ETH:
        return ::port::PORT_TYPE_ETH;
        break;
    default:
        return ::port::PORT_TYPE_NONE;
    }

    return ::port::PORT_TYPE_NONE;
}

::port::PortAdminState
sdk_port_admin_st_to_port_admin_st_spec (port_admin_state_t admin_st)
{
    switch(admin_st) {
    case port_admin_state_t::PORT_ADMIN_STATE_DOWN:
        return ::port::PORT_ADMIN_STATE_DOWN;

    case port_admin_state_t::PORT_ADMIN_STATE_UP:
        return ::port::PORT_ADMIN_STATE_UP;

    default:
        return ::port::PORT_ADMIN_STATE_NONE;
    }

    return ::port::PORT_ADMIN_STATE_NONE;
}

port_admin_state_t
port_admin_st_spec_to_sdk_port_admin_st (::port::PortAdminState admin_st)
{
    switch(admin_st) {
    case ::port::PORT_ADMIN_STATE_DOWN:
        return port_admin_state_t::PORT_ADMIN_STATE_DOWN;

    case ::port::PORT_ADMIN_STATE_UP:
        return port_admin_state_t::PORT_ADMIN_STATE_UP;

    default:
        return port_admin_state_t::PORT_ADMIN_STATE_NONE;
    }

    return port_admin_state_t::PORT_ADMIN_STATE_NONE;
}

::port::PortOperStatus
sdk_port_oper_st_to_port_oper_st_spec(port_oper_status_t oper_st)
{
    switch (oper_st) {
    case port_oper_status_t::PORT_OPER_STATUS_UP:
        return ::port::PORT_OPER_STATUS_UP;

    case port_oper_status_t::PORT_OPER_STATUS_DOWN:
        return ::port::PORT_OPER_STATUS_DOWN;

    default:
        return ::port::PORT_OPER_STATUS_NONE;
    }
    return ::port::PORT_OPER_STATUS_NONE;
}

}    // namespace hal
