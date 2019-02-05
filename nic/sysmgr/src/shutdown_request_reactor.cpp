#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/utils/penlog/lib/null_logger.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"
#include "gen/proto/sysmgr.delphi.hpp"

#include "pipe_t.hpp"
#include "shutdown_request_reactor.hpp"

using namespace std;

ShutdownRequestReactor::ShutdownRequestReactor(
    shared_ptr<Pipe<pid_t>> shutdown_pipe) :
    ShutdownRequestReactor(shutdown_pipe,
        std::make_shared<penlog::NullLogger>())
{
}

ShutdownRequestReactor::ShutdownRequestReactor(
    shared_ptr<Pipe<int>> shutdown_pipe, penlog::LoggerPtr logger) 
{
    this->logger = logger;
    this->shutdown_pipe = shutdown_pipe;
}

delphi::error ShutdownRequestReactor::OnSysmgrShutdownReqCreate(
    delphi::objects::SysmgrShutdownReqPtr obj)
{
    this->logger->info("ShutdownRequest {}", obj->key());
    int restart = 1;
    shutdown_pipe->pipe_write(&restart);
    return delphi::error::OK();
}

delphi::error ShutdownRequestReactor::OnSysmgrShutdownReqDelete(
    delphi::objects::SysmgrShutdownReqPtr obj)
{
    return delphi::error::OK();
}

delphi::error ShutdownRequestReactor::OnSysmgrShutdownReqUpdate(
    delphi::objects::SysmgrShutdownReqPtr obj)
{
    return delphi::error::OK();
}

