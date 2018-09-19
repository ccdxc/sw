#ifndef __DELPHI_SERVICE_STATUS_REACTOR_HPP__
#define __DELPHI_SERVICE_STATUS_REACTOR_HPP__

#include <memory>
#include <string>

#include <sys/types.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/sdk/proto/client.delphi.hpp"

#include "pipe_t.hpp"
#include "logger.hpp"

using namespace std;

class DelphiClientStatusReactor : public delphi::objects::DelphiClientStatusReactor
{
    public:
        DelphiClientStatusReactor(shared_ptr<Pipe<pid_t> > heartbeat_pipe);

        virtual delphi::error OnDelphiClientStatusCreate(delphi::objects::DelphiClientStatusPtr obj);
    
        virtual delphi::error OnDelphiClientStatusDelete(delphi::objects::DelphiClientStatusPtr obj);
        
        virtual delphi::error OnDelphiClientStatusUpdate(delphi::objects::DelphiClientStatusPtr obj);
    private:
        shared_ptr<Pipe<pid_t> > heartbeat_pipe;
};

#endif
