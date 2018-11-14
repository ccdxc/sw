#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/client.delphi.hpp"
#include "gen/proto/sysmgr.delphi.hpp"


#include "delphi_messages.hpp"
#include "sysmgr_service.hpp"

#include "delphi_client_status_reactor.hpp"
#include "sysmgr_service_status_reactor.hpp"

using namespace std;

SysmgrService::SysmgrService(delphi::SdkPtr sdk, string name, shared_ptr<Pipe<pid_t> > started_pids_pipe, 
    shared_ptr<Pipe<int32_t> > delphi_message_pipe, shared_ptr<Pipe<pid_t> > heartbeat_pipe)
{
    this->sdk = sdk;
    this->name = name;
    this->delphi_message_pipe = delphi_message_pipe;

    serviceStatusReactor = make_shared<SysmgrServiceStatusReactor>(started_pids_pipe);
    heartbeatReactor = make_shared<DelphiClientStatusReactor>(heartbeat_pipe);
    
    delphi::objects::SysmgrServiceStatus::Mount(this->sdk, delphi::ReadMode);
    delphi::objects::DelphiClientStatus::Mount(this->sdk, delphi::ReadMode);
    
    delphi::objects::SysmgrServiceStatus::Watch(this->sdk, serviceStatusReactor);
    delphi::objects::DelphiClientStatus::Watch(this->sdk, heartbeatReactor);
}

void SysmgrService::OnMountComplete()
{
    int32_t msg = DELPHI_UP;
    delphi_message_pipe->pipe_write(&msg);
}
