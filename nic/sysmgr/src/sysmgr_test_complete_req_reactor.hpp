#ifndef __SYSMGR_TEST_COMPLETE_REACTOR_HPP__
#define __SYSMGR_TEST_COMPLETE_REACTOR_HPP__

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"

#include "logger.hpp"
#include "pipe_t.hpp"

using namespace std;

class SysmgrTestCompleteReqReactor : public delphi::objects::SysmgrTestCompleteReqReactor
{
    public:
        SysmgrTestCompleteReqReactor(shared_ptr<Pipe<int32_t> > delphi_message_pipe);

        virtual delphi::error OnSysmgrTestCompleteReqCreate(delphi::objects::SysmgrTestCompleteReqPtr obj);

        virtual delphi::error OnSysmgrTestCompleteReqDelete(delphi::objects::SysmgrTestCompleteReqPtr obj);

        virtual delphi::error OnSysmgrTestCompleteReqUpdate(delphi::objects::SysmgrTestCompleteReqPtr obj);
    private:
        shared_ptr<Pipe<int32_t> > delphi_message_pipe;
};

#endif
