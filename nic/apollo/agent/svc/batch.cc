//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/agent/svc/batch.hpp"
#include "nic/apollo/agent/hooks.hpp"

Status
BatchSvcImpl::BatchStart(ServerContext *context,
                         const pds::BatchSpec *proto_spec,
                         pds::BatchStatus *proto_status) {
    pds_batch_params_t api_batch_params = {0};
    api_batch_params.epoch = proto_spec->epoch();

    // TODO: Adding this here since there is no proto defs for
    // flows. This needs to be cleaned up
    if (hooks::batch_start(api_batch_params.epoch) != SDK_RET_OK) {
        return Status::CANCELLED;
    }

    if (api_batch_params.epoch != PDS_EPOCH_INVALID) {
        if (pds_batch_start(&api_batch_params) == sdk::SDK_RET_OK) {
            return Status::OK;
        }
    }
    return Status::OK;
}

Status
BatchSvcImpl::BatchCommit(ServerContext *context, const Empty *proto_spec,
                          Empty *proto_status) {
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
