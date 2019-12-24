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
    pds_batch_ctxt_t bctxt;
    pds_batch_params_t batch_params = { 0 };

    batch_params.epoch = proto_spec->epoch();
    batch_params.async = false;

    // TODO: adding this here since there is no proto defs for
    // flows, this needs to be cleaned up
    if (hooks::batch_start(batch_params.epoch) != SDK_RET_OK) {
        return Status::CANCELLED;
    }

    if (batch_params.epoch != PDS_EPOCH_INVALID) {
        bctxt = pds_batch_start(&batch_params);
        if (bctxt != PDS_BATCH_CTXT_INVALID) {
            proto_status->set_apistatus(types::ApiStatus::API_STATUS_OK);
            proto_status->mutable_batchcontext()->set_batchcookie(bctxt);
            return Status::OK;
        }
        proto_status->set_apistatus(types::ApiStatus::API_STATUS_ERR);
        proto_status->mutable_batchcontext()->set_batchcookie(PDS_BATCH_CTXT_INVALID);
        return Status::CANCELLED;
    }
    // TODO: return OK until hooks is cleaned up
    proto_status->set_apistatus(types::ApiStatus::API_STATUS_OK);
    return Status::OK;
}

Status
BatchSvcImpl::BatchCommit(ServerContext *context,
                          const types::BatchCtxt *ctxt,
                          Empty *proto_status) {
    if (ctxt->batchcookie() != PDS_BATCH_CTXT_INVALID) {
        if (pds_batch_commit(ctxt->batchcookie()) == SDK_RET_OK) {
            return Status::OK;
        }
    }
    return Status::CANCELLED;
}

Status
BatchSvcImpl::BatchAbort(ServerContext *context,
                         const types::BatchCtxt *ctxt,
                         Empty *proto_status) {
    if (ctxt->batchcookie() != PDS_BATCH_CTXT_INVALID) {
        if (pds_batch_destroy(ctxt->batchcookie()) == SDK_RET_OK) {
            return Status::OK;
        }
    }
    return Status::CANCELLED;
}
