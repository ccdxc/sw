#include "nic/utils/agent_api/fte_sim/fwlogsim_svc.hpp"

Status
FWLogSimServiceImpl::LogFlow(ServerContext* context, const LogFlowRequestMsg* request, LogFlowResponseMsg* response)
{
    const FWEvent& fwe = request->request(0);
    log_firew_event(fwe);
    return Status::OK;
}
