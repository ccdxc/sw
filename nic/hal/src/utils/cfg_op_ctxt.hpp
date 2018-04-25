//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// handlers for cfg_op_ctxt_t
//-----------------------------------------------------------------------------

#ifndef __CFG_OP_CTXT_HPP__
#define __CFG_OP_CTXT_HPP__

#include "nic/include/hal_cfg.hpp"

namespace hal {

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
hal_ret_t cfg_ctxt_op_create_handle (hal_obj_id_t obj_id, void *obj,
                                     hal_cfg_op_cb_t add_cb,
                                     hal_cfg_commit_cb_t commit_cb,
                                     hal_cfg_abort_cb_t abort_cb,
                                     hal_cfg_cleanup_cb_t cleanup_cb,
                                     hal_handle_t *hal_hdl);

} // namespace hal

#endif  // __CFG_OP_CTXT_HPP__
