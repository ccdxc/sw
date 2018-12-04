#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/utils/penlog/lib/null_logger.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"
#include "gen/proto/sysmgr.delphi.hpp"

#include "pipe_t.hpp"
#include "sysmgr_service_status_reactor.hpp"

using namespace std;

SysmgrServiceStatusReactor::SysmgrServiceStatusReactor(
    shared_ptr<Pipe<pid_t>> started_pids_pipe) :
    SysmgrServiceStatusReactor(started_pids_pipe,
        std::make_shared<penlog::NullLogger>())
{
}

SysmgrServiceStatusReactor::SysmgrServiceStatusReactor(
    shared_ptr<Pipe<pid_t>> started_pids_pipe, penlog::LoggerPtr logger) 
{
    this->logger = logger;
    this->started_pids_pipe = started_pids_pipe;
}

delphi::error SysmgrServiceStatusReactor::OnSysmgrServiceStatusCreate(
    delphi::objects::SysmgrServiceStatusPtr obj)
{
    this->logger->info("SysmgrServiceCreate {}({})", obj->key(), obj->pid());
    pid_t pid = obj->pid();
    started_pids_pipe->pipe_write(&pid);
    return delphi::error::OK();
}
delphi::error SysmgrServiceStatusReactor::OnSysmgrServiceStatusDelete(
    delphi::objects::SysmgrServiceStatusPtr obj)
{
    return delphi::error::OK();
}

delphi::error SysmgrServiceStatusReactor::OnSysmgrServiceStatusUpdate(
    delphi::objects::SysmgrServiceStatusPtr obj)
{
    this->logger->info("SysmgrServiceUpdate {}({})", obj->key(), obj->pid());
    pid_t pid = obj->pid();
    started_pids_pipe->pipe_write(&pid);
    return delphi::error::OK();
}
