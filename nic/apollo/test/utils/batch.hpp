//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_BATCH_HPP__
#define __TEST_UTILS_BATCH_HPP__

#include "nic/apollo/api/include/pds_batch.hpp"
#ifdef AGENT_MODE
#include "nic/apollo/agent/client/app.hpp"
#endif

namespace api_test {

static inline void
batch_start (void)
{
    static pds_epoch_t epoch = 0;
    pds_batch_params_t batch_params = { .epoch = ++epoch };

#ifdef AGENT_MODE
    if (!agent_mode()) {
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    } else {
        ASSERT_TRUE(batch_start_grpc(epoch) == sdk::SDK_RET_OK);
    }
#else
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
#endif
}

static inline void
batch_commit (void)
{
#ifdef AGENT_MODE
    if (!agent_mode()) {
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    } else {
        ASSERT_TRUE(batch_commit_grpc() == sdk::SDK_RET_OK);
    }
#else
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
#endif
}

static inline void
batch_commit_fail (void)
{
#ifdef AGENT_MODE
    if (!agent_mode()) {
        ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    } else {
        ASSERT_TRUE(batch_commit_grpc() != sdk::SDK_RET_OK);
    }
#else
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
#endif
}

static inline void
batch_abort (void)
{
#ifdef AGENT_MODE
    if (!agent_mode()) {
        ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
    } else {
        ASSERT_TRUE(batch_abort_grpc() == sdk::SDK_RET_OK);
    }
#else
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
#endif
}

}    // end namespace

#endif    // __TEST_UTILS_BATCH_HPP__
