#ifndef __SHUTDOWN_REQUEST_REACTOR_HPP__
#define __SHUTDOWN_REQUEST_REACTOR_HPP__

#include <memory>
#include <string>

#include <sys/types.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"
#include "gen/proto/sysmgr.delphi.hpp"

#include "pipe_t.hpp"

using namespace std;

class ShutdownRequestReactor : public delphi::objects::SysmgrShutdownReqReactor
{
public:
    ShutdownRequestReactor(shared_ptr<Pipe<pid_t> > died_pids_pipe);
    ShutdownRequestReactor(shared_ptr<Pipe<pid_t> > died_pids_pipe,
        penlog::LoggerPtr logger);

    virtual delphi::error OnSysmgrShutdownReqCreate(
	delphi::objects::SysmgrShutdownReqPtr obj);
    virtual delphi::error OnSysmgrShutdownReqDelete(
	delphi::objects::SysmgrShutdownReqPtr obj);
    virtual delphi::error OnSysmgrShutdownReqUpdate(
	delphi::objects::SysmgrShutdownReqPtr obj);
private:
    shared_ptr<Pipe<int> > shutdown_pipe;
    penlog::LoggerPtr logger;
};

#endif
