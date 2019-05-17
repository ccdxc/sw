//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the batch test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_BATCH_HPP__
#define __TEST_UTILS_BATCH_HPP__

namespace api_test {

static inline void
batch_start(void) {
    static pds_epoch_t epoch = 1;
    pds_batch_params_t batch_params = { .epoch = ++epoch };

    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
}

static inline void
batch_commit(void) {
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

static inline void
batch_commit_fail(void) {
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
}

static inline void
batch_abort(void) {
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

}    // end namespace

#endif    // __TEST_UTILS_BATCH_HPP__
