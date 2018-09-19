#ifndef __SYSMGR_SERVICE_HPP__
#define __SYSMGR_SERVICE_HPP__

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"

#include "logger.hpp"
#include "pipe_t.hpp"

#include "delphi_client_status_reactor.hpp"
#include "sysmgr_service_status_reactor.hpp"

using namespace std;

class SysmgrService : public delphi::Service, public enable_shared_from_this<SysmgrService>
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

    virtual void OnMountComplete();
};

#endif
