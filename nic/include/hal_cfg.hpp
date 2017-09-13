// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __HAL_CFG_HPP__
#define __HAL_CFG_HPP__

#include <base.h>

namespace hal {

//------------------------------------------------------------------------------
// HAL config operations
//------------------------------------------------------------------------------
typedef enum cfg_op_e {
    CFG_OP_NONE,
    CFG_OP_READ,
    CFG_OP_WRITE,
} cfg_op_t;

//------------------------------------------------------------------------------
// Wrappers on HAL config db oeprations by APIs
//------------------------------------------------------------------------------
hal_ret_t hal_cfg_db_open(cfg_op_t op);
hal_ret_t hal_cfg_db_close(bool abort);

//------------------------------------------------------------------------------
// HAL config db APIs store some context in the cfg_db_ctxt_t
// NOTE: this context is per thread, not for the whole process
//------------------------------------------------------------------------------
typedef struct cfg_db_ctxt_s {
    bool                   cfg_db_open_;    // true if cfg db is opened
    cfg_op_t               cfg_op_;         // cfg operation for which db is opened
    cfg_version_t          ver_in_use_;     // version we are starting the operation with
    cfg_version_t          rsvd_ver_;       // version to commit db modifications with
    //cfg_db_dirty_objs_t    *dirty_objs;     // dirty object list to be committed
} cfg_db_ctxt_t;
extern thread_local cfg_db_ctxt_t t_cfg_db_ctxt;

}    // namespace hal

#endif    // __HAL_CFG_HPP__

