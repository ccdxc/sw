#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "nic/gen/proto/hal/types.pb.h"
#include "sdk/list.hpp"
#include "nic/utils/block_list/block_list.hpp"
#include "nic/include/ip.h"

using types::ApiStatus;
using hal::utils::block_list;

namespace hal {

#define NUM_DASHES 20
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)


hal_ret_t ip_addr_spec_to_ip_addr(ip_addr_t *out_ipaddr,
                                  const types::IPAddress& in_ipaddr);
hal_ret_t ip_addr_to_spec (types::IPAddress *ip_addr_spec,
                           const ip_addr_t *ip_addr);
hal_ret_t ip_pfx_spec_to_pfx_spec(ip_prefix_t *ip_pfx, 
                                  const types::IPPrefix& in_ippfx);
hal_ret_t ip_pfx_to_spec (types::IPPrefix *ip_pfx_spec,
                          const ip_prefix_t *ip_pfx);
bool ip_addr_check_equal(ip_addr_t *ipaddr1, ip_addr_t *ipaddr2);
bool ip_addr_in_ip_pfx(ip_addr_t *ipaddr, ip_prefix_t *ip_pfx);

ApiStatus hal_prepare_rsp (hal_ret_t ret);
void hal_api_trace (const char *trace);
void custom_backtrace();

// handle list specific APIs
void hal_print_handles_list(dllist_ctxt_t  *list);
bool hal_handle_in_list(dllist_ctxt_t *handle_list, hal_handle_t handle);
hal_ret_t hal_add_to_handle_list(dllist_ctxt_t *handle_list, 
                                 hal_handle_t handle);
void hal_free_handles_list(dllist_ctxt_t *list);
hal_ret_t hal_cleanup_handle_list(dllist_ctxt_t **list);

// handle block list specific APIs
void hal_print_handles_block_list(block_list *bl);
bool hal_handle_in_block_list(block_list *bl, hal_handle_t handle);
hal_ret_t hal_add_to_handle_block_list(block_list *bl, hal_handle_t handle);
hal_ret_t hal_remove_all_handles_block_list(block_list *bl);
hal_ret_t hal_cleanup_handle_block_list(block_list **bl);

}    // namespace hal

#endif    // __UTILS_HPP__

