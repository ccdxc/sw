#ifndef __SYSMGR_SERVICE_STATUS_REACTOR_HPP__
#define __SYSMGR_SERVICE_STATUS_REACTOR_HPP__

#include <memory>
#include <string>

#include <sys/types.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"

#include "pipe_t.hpp"
#include "logger.hpp"

using namespace std;

class SysmgrServiceStatusReactor : public delphi::objects::SysmgrServiceStatusReactor
{
    public:
        SysmgrServiceStatusReactor(shared_ptr<Pipe<pid_t> > died_pids_pipe);

        virtual delphi::error OnSysmgrServiceStatusCreate(delphi::objects::SysmgrServiceStatusPtr obj);
    
        virtual delphi::error OnSysmgrServiceStatusDelete(delphi::objects::SysmgrServiceStatusPtr obj);
        
        virtual delphi::error OnSysmgrServiceStatusUpdate(delphi::objects::SysmgrServiceStatusPtr obj);
    private:
        shared_ptr<Pipe<pid_t> > started_pids_pipe;
};

#endif
