//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_batch.hpp"
#include "nic/apollo/agent/svc/batch.hpp"
#include "nic/apollo/test/flow_test/flow_test.hpp"
extern flow_test *g_flow_test_obj;

Status
BatchSvcImpl::BatchStart(ServerContext *context,
                         const tpc::BatchSpec *proto_spec,
                         tpc::BatchStatus *proto_status) {
    pds_batch_params_t api_batch_params = {0};

    api_batch_params.epoch = proto_spec->epoch();
    if (pds_batch_start(&api_batch_params) == sdk::SDK_RET_OK) {
        return Status::OK;
    }
    return Status::CANCELLED;
}

Status
BatchSvcImpl::BatchCommit(ServerContext *context, const Empty *proto_spec,
                          Empty *proto_status) {
#if 0
    sdk_ret_t ret = SDK_RET_OK;

    ret = g_flow_test_obj->create_flows(1024*1024, 17, 100, 100, false);
    if (ret != sdk::SDK_RET_OK) {
        return Status::CANCELLED;
    }

    ret = g_flow_test_obj->create_flows(1024*1024, 17, 100, 100, true);
    if (ret != sdk::SDK_RET_OK) {
        return Status::CANCELLED;
    }
#endif

    if (pds_batch_commit() == sdk::SDK_RET_OK) {
        return Status::OK;
    }
    return Status::CANCELLED;
}

Status
BatchSvcImpl::BatchAbort(ServerContext *context, const Empty *proto_spec,
                         Empty *proto_status) {
    if (pds_batch_abort() == sdk::SDK_RET_OK) {
        return Status::OK;
    }
    return Status::CANCELLED;
}
