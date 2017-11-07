#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/quiesce_svc.hpp"

Status QuiesceServiceImpl::QuiesceMsgSnd(ServerContext* context,
                     const Empty* request,
                     Empty* response)
{
    HAL_TRACE_DEBUG("QuiesceMsgSnd Request");
    //hal::quiesce_message_tx();
    return Status::OK;
}
