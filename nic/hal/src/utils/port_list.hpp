//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for types::L4PortRange
//-----------------------------------------------------------------------------

#ifndef __PORT_LIST_HPP__
#define __PORT_LIST_HPP__

#include "sdk/list.hpp"
#include "nic/include/l4.h"

using sdk::lib::dllist_ctxt_t;

namespace hal {

typedef struct port_list_elem_s {
    port_range_t     port_range;
    dllist_ctxt_t    list_ctxt;
} __PACK__ port_list_elem_t;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
hal_ret_t port_list_elem_l4portrange_spec_handle(
    const types::L4PortRange& port, dllist_ctxt_t *head);
void port_list_cleanup(dllist_ctxt_t *head);
hal_ret_t port_list_elem_l4portrange_spec_build(
    port_list_elem_t *port_lelem, types::L4PortRange *port_range);

} // namespace hal

#endif  // __PORT_LIST_HPP__
