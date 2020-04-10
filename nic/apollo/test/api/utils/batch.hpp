//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_BATCH_HPP__
#define __TEST_API_UTILS_BATCH_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#ifdef AGENT_MODE
#include "nic/apollo/agent/test/client/app.hpp"
#endif
#include "base.hpp"

namespace test {
namespace api {

static inline pds_batch_ctxt_t
batch_start (void)
{
    pds_batch_ctxt_t bctxt;
    static pds_epoch_t epoch = 0;
    pds_batch_params_t batch_params = { .epoch = ++epoch,
                                        .async = false,
                                        .response_cb = nullptr,
                                        .cookie = nullptr
                                      };
#ifdef AGENT_MODE
    if (!agent_mode()) {
        bctxt = pds_batch_start(&batch_params);
        SDK_ASSERT(bctxt != PDS_BATCH_CTXT_INVALID);
    } else {
        bctxt = batch_start_grpc(epoch);
        SDK_ASSERT(bctxt != PDS_BATCH_CTXT_INVALID);
    }
#else
    bctxt = pds_batch_start(&batch_params);
    SDK_ASSERT(bctxt != PDS_BATCH_CTXT_INVALID);
#endif
    return bctxt;
}

static inline void
batch_commit (pds_batch_ctxt_t bctxt)
{
#ifdef AGENT_MODE
    if (!agent_mode()) {
        SDK_ASSERT(pds_batch_commit(bctxt) == SDK_RET_OK);
    } else {
        SDK_ASSERT(batch_commit_grpc(bctxt) == SDK_RET_OK);
    }
#else
    SDK_ASSERT(pds_batch_commit(bctxt) == SDK_RET_OK);
#endif
}

static inline void
batch_commit_fail (pds_batch_ctxt_t bctxt)
{
#ifdef AGENT_MODE
    if (!agent_mode()) {
        SDK_ASSERT(pds_batch_commit(bctxt) != SDK_RET_OK);
    } else {
        SDK_ASSERT(batch_commit_grpc(bctxt) != SDK_RET_OK);
    }
#else
    SDK_ASSERT(pds_batch_commit(bctxt) != SDK_RET_OK);
#endif
}

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_BATCH_HPP__
