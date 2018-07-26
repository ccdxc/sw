#include "nic/include/base.hpp"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/quiesce_svc.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/src/internal/quiesce.hpp"

Status QuiesceServiceImpl::QuiesceMsgSnd(ServerContext* context,
                     const Empty* request,
                     Empty* response)
{
    HAL_TRACE_DEBUG("QuiesceMsgSnd Request");
    hal::proxy::quiesce_message_tx();
    return Status::OK;
}
Status QuiesceServiceImpl::QuiesceStart(ServerContext* context,
                             const Empty* request,
                             Empty* response)

{
    hal::quiesce_start();
    return Status::OK;
}
Status QuiesceServiceImpl::QuiesceStop(ServerContext* context,
                             const Empty* request,
                             Empty* response)
{
    hal::quiesce_stop();
    return Status::OK;
}
