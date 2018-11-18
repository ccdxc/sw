#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/client.delphi.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "gen/proto/sysmgr.pb.h"

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

    delphi::objects::SysmgrProcessStatus::Mount(this->sdk, delphi::ReadWriteMode);
    delphi::objects::SysmgrSystemStatus::Mount(this->sdk, delphi::ReadWriteMode);
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


void SysmgrService::service_status_changed(const string &name, pid_t pid,
    enum service_status status, string reason)
{
    auto obj = make_shared<delphi::objects::SysmgrProcessStatus>();
    obj->set_key(name);
    obj->set_pid(pid);
    switch (status) {
    case WAITING:
        obj->set_state(::sysmgr::Waiting);
        break;
    case READY:
        obj->set_state(::sysmgr::Ready);
        break;
    case STARTING:
        obj->set_state(::sysmgr::Launched);
        break;
    case RUNNING:
        obj->set_state(::sysmgr::Running);
        break;
    case DIED:
        obj->set_state(::sysmgr::Died);
        break;
    case FAILED_TO_START:
        obj->set_state(::sysmgr::Died);
        break;
    }
    obj->set_exitreason(reason);

    this->sdk->QueueUpdate(obj);
}

void SysmgrService::set_system_fault()
{
    auto obj = make_shared<delphi::objects::SysmgrSystemStatus>();

    obj->set_state(::sysmgr::Fault);

    this->sdk->QueueUpdate(obj);
}
