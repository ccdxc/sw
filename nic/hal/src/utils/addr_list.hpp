//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for types::Address & types::AddressObj object
//-----------------------------------------------------------------------------

#ifndef __ADDR_LIST_HPP__
#define __ADDR_LIST_HPP__

#include "sdk/list.hpp"
#include "nic/include/ip.h"

using sdk::lib::dllist_ctxt_t;

namespace hal {

//-----------------------------------------------------------------------------
// message Address {
//   oneof   Address {
//       IPSubnet      prefix     = 1;    // v4 or v6 address prefix
//       AddressRange  range      = 2;    // v4 or v6 address range
//    }
// }
//
// message IPAddressObj {
//   oneof Formats {
//     IPAddressType   type        = 1;
//     Address         address     = 2;
//   }
//   bool negate                   = 3;
// }
//
// IPAddressObj is a superset of Address object
//
// A common data structure is used to maintain both types of objects.
//
// The Address object is one of prefix or range. The prefix is normalized
// into a range and maintained.
//
// The default value of negate is false, and assigned appropriately only
// in the IPAddressObj case. The type is converted into appropriate prefix
// and passed through the same logic of Address object
//-----------------------------------------------------------------------------
typedef struct addr_list_elem_s {
    bool             negate;
    uint32_t         num_addrs;    // no. of addresses in this range
    dllist_ctxt_t    list_ctxt;    // list ctxt to link to other nodes
    ip_range_t       ip_range;     // IP address range
} __PACK__ addr_list_elem_t;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
addr_list_elem_t *addr_list_elem_address_spec_handle(
    const types::Address& addr, dllist_ctxt_t *head);
hal_ret_t addr_list_elem_ipaddressobj_spec_handle(
    const types::IPAddressObj& addr, dllist_ctxt_t *head);
void addr_list_cleanup(dllist_ctxt_t *head);
bool addr_in_addr_list_elem(ip_addr_t *addr, addr_list_elem_t *addr_list_elem);
hal_ret_t addr_offset(ip_addr_t *addr, addr_list_elem_t *addr_list_elem,
                      uint32_t *offset);
hal_ret_t addr_list_elem_spec_src_addr_build(dllist_ctxt_t *head,
                                             types::RuleMatch *spec);
hal_ret_t addr_list_elem_spec_dst_addr_build(dllist_ctxt_t *head,
                                             types::RuleMatch *spec);
} // namespace hal

#endif  // __ADDR_LIST_HPP__
