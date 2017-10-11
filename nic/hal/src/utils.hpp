#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "nic/proto/types.pb.h"
#include "nic/include/ip.h"

using types::ApiStatus;

namespace hal {

#define NUM_DASHES 20

hal_ret_t ip_addr_spec_to_ip_addr(ip_addr_t *out_ipaddr,
                                  const types::IPAddress& in_ipaddr);
hal_ret_t ip_addr_to_spec (types::IPAddress *ip_addr_spec,
                           const ip_addr_t *ip_addr);
hal_ret_t ip_pfx_spec_to_pfx_spec(ip_prefix_t *ip_pfx, 
                                  const types::IPPrefix& in_ippfx);
bool ip_addr_in_ip_pfx(ip_addr_t *ipaddr, ip_prefix_t *ip_pfx);

ApiStatus hal_prepare_rsp (hal_ret_t ret);
void hal_api_trace (const char *trace);
void hal_print_handles_list(dllist_ctxt_t  *list);
void hal_free_handles_list(dllist_ctxt_t *list);
void custom_backtrace();
}    // namespace hal

#endif    // __UTILS_HPP__

