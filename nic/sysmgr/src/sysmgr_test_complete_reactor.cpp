#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"

#include "delphi_messages.hpp"
#include "logger.hpp"
#include "pipe_t.hpp"
#include "sysmgr_test_complete_req_reactor.hpp"

using namespace std;

SysmgrTestCompleteReqReactor::SysmgrTestCompleteReqReactor(shared_ptr<Pipe<int32_t> > delphi_message_pipe) 
{
    this->delphi_message_pipe = delphi_message_pipe;
}

delphi::error SysmgrTestCompleteReqReactor::OnSysmgrTestCompleteReqCreate(delphi::objects::SysmgrTestCompleteReqPtr obj)
{
    INFO("SysmgrTestCompleteReq {}({})", obj->key(), obj->pid());
    delphi_message_pipe->pipe_write(TEST_COMPLETE);
    return delphi::error::OK();
}

delphi::error SysmgrTestCompleteReqReactor::OnSysmgrTestCompleteReqDelete(delphi::objects::SysmgrTestCompleteReqPtr obj)
{
    return delphi::error::OK();
}

delphi::error SysmgrTestCompleteReqReactor::OnSysmgrTestCompleteReqUpdate(delphi::objects::SysmgrTestCompleteReqPtr obj)
{
    INFO("SysmgrTestCompleteReq {}({})", obj->key(), obj->pid());
    delphi_message_pipe->pipe_write(TEST_COMPLETE);
    return delphi::error::OK();
}