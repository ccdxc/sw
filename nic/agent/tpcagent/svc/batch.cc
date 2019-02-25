//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "batch.hpp"

Status
BatchSvcImpl::BatchStart(ServerContext *context,
                         const tpc::BatchSpec *spec,
                         tpc::BatchStatus *status)
{
    return Status::OK;
}

Status
BatchSvcImpl::BatchCommit(ServerContext *context,
                          const Empty *spec,
                          Empty *status)
{
    return Status::OK;
}

Status
BatchSvcImpl::BatchAbort(ServerContext *context,
                         const Empty *spec,
                         Empty *status)
{
    return Status::OK;
}
