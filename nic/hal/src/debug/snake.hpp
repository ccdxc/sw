//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __SNAKE_HPP__
#define __SNAKE_HPP__

#include "nic/include/base.hpp"
#include "gen/proto/debug.pb.h"
#include "nic/include/hal_lock.hpp"

using debug::SnakeTestRequestMsg;
using debug::SnakeTestResponseMsg;
using debug::SnakeTestRequest;
using debug::SnakeTestResponse;
using debug::SnakeTestDeleteRequest;
using debug::SnakeTestDeleteResponse;

#define SNAKE_TEST_LOOP_VLAN_DEF        4000
#define SNAKE_TEST_LOOP_ARM_TO_ARM_DEF  4001

namespace hal {

typedef struct snake_test_s {
    hal_spinlock_t       slock;
    hal_handle_t         hal_handle;
    types::SnakeTestType type;
    vlan_id_t            vlan;

    void *pd;
} snake_test_t;

hal_ret_t snake_test_create(SnakeTestRequest& req,
                            SnakeTestResponse *rsp);
hal_ret_t snake_test_delete(SnakeTestDeleteRequest& req,
                            SnakeTestDeleteResponse *rsp);
hal_ret_t snake_test_get(SnakeTestResponseMsg *rsp_msg);

}    // namespace hal

#endif    // __SNAKE_HPP__

