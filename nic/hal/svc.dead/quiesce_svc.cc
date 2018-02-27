#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/quiesce_svc.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"

Status QuiesceServiceImpl::QuiesceMsgSnd(ServerContext* context,
                     const Empty* request,
                     Empty* response)
{
    HAL_TRACE_DEBUG("QuiesceMsgSnd Request");
    hal::proxy::quiesce_message_tx();
    return Status::OK;
}
