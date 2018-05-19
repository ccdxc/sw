//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// configuration handlers for sgrange
//-----------------------------------------------------------------------------

#ifndef __SG_LIST_HPP__
#define __SG_LIST_HPP__

#include "sdk/list.hpp"

using sdk::lib::dllist_ctxt_t;

namespace hal {

typedef struct sg_list_elem_s {
    sg_id_t        sg_id;
    dllist_ctxt_t  list_ctxt;
} __PACK__ sg_list_elem_t;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
hal_ret_t sg_list_elem_sg_spec_handle(
    uint32_t sg_id, dllist_ctxt_t *head);
void port_list_cleanup(dllist_ctxt_t *head);
#if 0
hal_ret_t sg_list_elem_dst_sg_spec_build(dllist_ctxt_t *head,
                                             types::SGRange *sg_info);
hal_ret_t sg_list_elem_src_sg_spec_build(dllist_ctxt_t *head,types::SGRange *sg_info);
#endif
} // namespace hal

#endif  // __PORT_LIST_HPP__
