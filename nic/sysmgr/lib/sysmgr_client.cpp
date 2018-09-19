#include "sysmgr_client.hpp"

#include <sys/types.h>
#include <unistd.h>

using namespace std;

void sysmgr::Client::init_done()
{
    auto obj = make_shared<delphi::objects::SysmgrServiceStatus>();

    obj->set_key(this->name);
    obj->set_pid(getpid());
    this->delphi->SetObject(obj);
}

delphi::error sysmgr::Client::OnSysmgrShutdownReqCreate(delphi::objects::SysmgrShutdownReqPtr obj)
{
    return delphi::error::OK();
}