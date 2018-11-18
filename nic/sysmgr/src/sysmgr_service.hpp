#ifndef __SYSMGR_SERVICE_HPP__
#define __SYSMGR_SERVICE_HPP__

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"

#include "logger.hpp"
#include "pipe_t.hpp"

#include "delphi_client_status_reactor.hpp"
#include "sysmgr_service_status_reactor.hpp"

#include "scheduler.hpp"

using namespace std;

class SysmgrService : public delphi::Service,
                      public enable_shared_from_this<SysmgrService>,
                      public ServiceStatusWatcher
{
  private:
    delphi::SdkPtr sdk;
    string name;
    shared_ptr<SysmgrServiceStatusReactor> serviceStatusReactor;
    shared_ptr<DelphiClientStatusReactor> heartbeatReactor;
    shared_ptr<Pipe<int32_t> > delphi_message_pipe;
  public:
    SysmgrService(delphi::SdkPtr sdk, string name, shared_ptr<Pipe<pid_t>> started_pids_pipe, 
        shared_ptr<Pipe<int32_t> > delphi_message_pipe, shared_ptr<Pipe<pid_t> > heartbeat_pipe);
    void set_system_fault();
    virtual void OnMountComplete();
    virtual void service_status_changed(const string &name, pid_t pid,
        enum service_status status, string reason);
};

#endif
