#include <memory>
#include <string>

#include "gen/proto/client.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"
#include "nic/utils/penlog/lib/null_logger.hpp"

#include "pipe_t.hpp"
#include "delphi_client_status_reactor.hpp"

using namespace std;

DelphiClientStatusReactor::DelphiClientStatusReactor(
    shared_ptr<Pipe<pid_t> > heartbeat_pipe) :
    DelphiClientStatusReactor(heartbeat_pipe, make_shared<penlog::NullLogger>())
{
}

DelphiClientStatusReactor::DelphiClientStatusReactor(
    shared_ptr<Pipe<pid_t> > heartbeat_pipe, penlog::LoggerPtr logger) 
{
    this->logger = logger;
    this->heartbeat_pipe = heartbeat_pipe;
}

delphi::error DelphiClientStatusReactor::OnDelphiClientStatusCreate(delphi::objects::DelphiClientStatusPtr obj)
{
    logger->debug("DelphiServiceCreate {}({})", obj->key(), obj->pid());
    pid_t pid = obj->pid();
    this->heartbeat_pipe->pipe_write(&pid);
    return delphi::error::OK();
}
delphi::error DelphiClientStatusReactor::OnDelphiClientStatusDelete(delphi::objects::DelphiClientStatusPtr obj)
{
    return delphi::error::OK();
}

delphi::error DelphiClientStatusReactor::OnDelphiClientStatusUpdate(delphi::objects::DelphiClientStatusPtr obj)
{
    logger->debug("DelphiServiceUpdate {}({})", obj->key(), obj->pid());
    pid_t pid = obj->pid();
    heartbeat_pipe->pipe_write(&pid);
    return delphi::error::OK();
}
