#ifndef __LINKMGR_UTILS_HPP__
#define __LINKMGR_UTILS_HPP__

#include "nic/gen/proto/hal/types.pb.h"
#include "nic/include/ip.h"
#include "nic/gen/proto/hal/port.pb.h"
#include "sdk/catalog.hpp"

using types::ApiStatus;

namespace hal {

#define NUM_DASHES 20
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

hal_ret_t ip_addr_spec_to_ip_addr(ip_addr_t *out_ipaddr,
                                  const types::IPAddress& in_ipaddr);
hal_ret_t ip_addr_to_spec (types::IPAddress *ip_addr_spec,
                           const ip_addr_t *ip_addr);
hal_ret_t ip_pfx_spec_to_pfx_spec(ip_prefix_t *ip_pfx, 
                                  const types::IPPrefix& in_ippfx);
bool ip_addr_in_ip_pfx(ip_addr_t *ipaddr, ip_prefix_t *ip_pfx);

ApiStatus hal_prepare_rsp (hal_ret_t ret);
void hal_api_trace (const char *trace);

// handle list specific APIs
void hal_print_handles_list(dllist_ctxt_t  *list);
bool hal_handle_in_list(dllist_ctxt_t *handle_list, hal_handle_t handle);
hal_ret_t hal_add_to_handle_list(dllist_ctxt_t *handle_list, 
                                 hal_handle_t handle);
void hal_free_handles_list(dllist_ctxt_t *list);
hal_ret_t hal_cleanup_handle_list(dllist_ctxt_t **list);

sdk::lib::port_speed_t port_speed_spec_to_sdk_port_speed(::port::PortSpeed speed);
::port::PortSpeed sdk_port_speed_to_port_speed_spec(sdk::lib::port_speed_t speed);
sdk::lib::port_type_t port_type_to_sdk_port_type_spec(::port::PortType type);
::port::PortType sdk_port_type_to_port_type_spec(sdk::lib::port_type_t type);

}    // namespace hal

#endif    // __LINKMGR_UTILS_HPP__

