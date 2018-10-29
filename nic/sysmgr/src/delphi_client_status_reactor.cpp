#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/client.delphi.hpp"

#include "logger.hpp"
#include "pipe_t.hpp"
#include "delphi_client_status_reactor.hpp"

using namespace std;

DelphiClientStatusReactor::DelphiClientStatusReactor(shared_ptr<Pipe<pid_t> > heartbeat_pipe) 
{
    this->heartbeat_pipe = heartbeat_pipe;
}

delphi::error DelphiClientStatusReactor::OnDelphiClientStatusCreate(delphi::objects::DelphiClientStatusPtr obj)
{
    DEBUG("DelphiServiceCreate {}({})", obj->key(), obj->pid());
    this->heartbeat_pipe->pipe_write(obj->pid());
    return delphi::error::OK();
}
delphi::error DelphiClientStatusReactor::OnDelphiClientStatusDelete(delphi::objects::DelphiClientStatusPtr obj)
{
    return delphi::error::OK();
}

delphi::error DelphiClientStatusReactor::OnDelphiClientStatusUpdate(delphi::objects::DelphiClientStatusPtr obj)
{
    DEBUG("DelphiServiceUpdate {}({})", obj->key(), obj->pid());
    heartbeat_pipe->pipe_write(obj->pid());
    return delphi::error::OK();
}
