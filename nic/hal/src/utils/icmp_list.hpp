//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for sgrange
//-----------------------------------------------------------------------------

#ifndef __ICMP_LIST_HPP__
#define __ICMP_LIST_HPP__

#include "lib/list/list.hpp"

using sdk::lib::dllist_ctxt_t;

namespace hal {

typedef struct icmp_list_elem_s {
    uint32_t         icmp_type;
    uint32_t         icmp_code;
    dllist_ctxt_t    list_ctxt;
} __PACK__ icmp_list_elem_t;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
hal_ret_t icmp_list_elem_icmp_spec_handle(
    uint32_t icmp_type, uint32_t icmp_code, dllist_ctxt_t *head);
void icmp_list_cleanup(dllist_ctxt_t *head);
#if 0
hal_ret_t sg_list_elem_dst_sg_spec_build(dllist_ctxt_t *head,
                                             types::SGRange *sg_info);
hal_ret_t sg_list_elem_src_sg_spec_build(dllist_ctxt_t *head,types::SGRange *sg_info);
#endif
} // namespace hal

#endif  // __ICMP_LIST_HPP__
