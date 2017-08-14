#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <types.pb.h>
#include <ip.h>

namespace hal {

hal_ret_t ip_addr_spec_to_ip_addr(ip_addr_t *out_ipaddr,
                                  const types::IPAddress& in_ipaddr);

hal_ret_t ip_pfx_spec_to_pfx_spec(ip_prefix_t *ip_pfx, 
                                  const types::IPPrefix& in_ippfx);
}    // namespace hal

#endif    // __UTILS_HPP__

