// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __HAL_CFG_HPP__
#define __HAL_CFG_HPP__

#include <base.h>

namespace hal {

// HAL CONFIG OPERATIONS
typedef enum cfg_op_e {
    CFG_OP_NONE,
    CFG_OP_READ,
    CFG_OP_WRITE,
} cfg_op_t;

hal_ret_t hal_cfg_db_open(cfg_op_t op);
hal_ret_t hal_cfg_db_close(bool abort);

}    // namespace hal

#endif    // __HAL_CFG_HPP__

