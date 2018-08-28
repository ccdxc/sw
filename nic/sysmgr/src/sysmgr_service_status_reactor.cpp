#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"

#include "logger.hpp"
#include "pipe_t.hpp"
#include "sysmgr_service_status_reactor.hpp"

using namespace std;

SysmgrServiceStatusReactor::SysmgrServiceStatusReactor(shared_ptr<Pipe<pid_t> > started_pids_pipe) 
{
    this->started_pids_pipe = started_pids_pipe;
}


delphi::error SysmgrServiceStatusReactor::OnSysmgrServiceStatusCreate(delphi::objects::SysmgrServiceStatusPtr obj)
{
    INFO("SysmgrServiceCreate {}({})", obj->key(), obj->pid());
    started_pids_pipe->pipe_write(obj->pid());
    return delphi::error::OK();
}
delphi::error SysmgrServiceStatusReactor::OnSysmgrServiceStatusDelete(delphi::objects::SysmgrServiceStatusPtr obj)
{
    return delphi::error::OK();
}

delphi::error SysmgrServiceStatusReactor::OnSysmgrServiceStatusUpdate(delphi::objects::SysmgrServiceStatusPtr obj)
{
    INFO("SysmgrServiceUpdate {}({})", obj->key(), obj->pid());
    started_pids_pipe->pipe_write(obj->pid());
    return delphi::error::OK();
}