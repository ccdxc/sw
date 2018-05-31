//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

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
hal_ret_t ip_addr_to_spec(types::IPAddress *ip_addr_spec,
                          const ip_addr_t *ip_addr);
hal_ret_t ip_pfx_spec_to_pfx(ip_prefix_t *ip_pfx,
                             const types::IPPrefix& in_ippfx);
hal_ret_t ip_pfx_to_spec(types::IPPrefix *ip_pfx_spec,
                         const ip_prefix_t *ip_pfx);
hal_ret_t ip_range_spec_to_ip_range(ip_range_t *range,
                                    const types::AddressRange& spec);
hal_ret_t ip_range_to_spec(types::AddressRange *spec,
                           ip_range_t *range);
hal_ret_t ip_subnet_spec_to_ip_range(ip_range_t *range,
                                     const ::types::IPSubnet& spec);
bool ip_addr_check_equal(ip_addr_t *ipaddr1, ip_addr_t *ipaddr2);
bool ip_addr_in_ip_pfx(ip_addr_t *ipaddr, ip_prefix_t *ip_pfx);
ApiStatus hal_prepare_rsp(hal_ret_t ret);
void hal_api_trace(const char *trace);

// handle list specific APIs
void hal_print_handles_list(dllist_ctxt_t  *list);
bool hal_handle_in_list(dllist_ctxt_t *handle_list, hal_handle_t handle);
hal_ret_t hal_add_to_handle_list(dllist_ctxt_t *handle_list,
                                 hal_handle_t handle);
void hal_free_handles_list(dllist_ctxt_t *list);
hal_ret_t hal_cleanup_handle_list(dllist_ctxt_t **list);
hal_ret_t hal_remove_from_handle_list(dllist_ctxt_t *list_head,
                                      hal_handle_t handle);
hal_ret_t
hal_unlink_and_free_from_handle_list(hal_handle_id_list_entry_t *entry);

// handle block list specific APIs
void hal_print_handles_block_list(block_list *bl);
bool hal_handle_in_block_list(block_list *bl, hal_handle_t handle);
hal_ret_t hal_add_to_handle_block_list(block_list *bl, hal_handle_t handle);
hal_ret_t hal_del_from_handle_block_list(block_list *bl, hal_handle_t handle);
hal_ret_t hal_remove_all_handles_block_list(block_list *bl);
hal_ret_t hal_cleanup_handle_block_list(block_list **bl);
hal_ret_t hal_add_block_lists(block_list *dst, block_list *src);
hal_ret_t hal_del_block_lists(block_list *dst, block_list *src);
hal_ret_t hal_copy_block_lists(block_list *dst, block_list *src);

typedef uint32_t (*hal_changed_list_size_cb)(const void *spec);
typedef hal_handle_t (*hal_changed_list_get_handle)(const void *spec, uint32_t idx);
typedef std::set<hal_handle_t> hal_handle_set_t;
hal_ret_t
hal_find_changed_lists (block_list *exist_list,                         // _IN
                        const void *spec,                               // _IN
                        hal_changed_list_size_cb size_cb,               // _IN
                        hal_changed_list_get_handle hdl_cb,             // _IN
                        block_list **add_list,                          // _OUT
                        block_list **del_list,                          // _OUT
                        block_list **new_list,                          // _OUT
                        bool *has_changed);                              // _OUT


}    // namespace hal

#endif    // __UTILS_HPP__

