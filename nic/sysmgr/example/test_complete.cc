// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"

using namespace std;

class TestCompleteService : public delphi::Service, public enable_shared_from_this<TestCompleteService>
{
  private:
    delphi::SdkPtr sdk;
    string name;

  public:
    TestCompleteService(delphi::SdkPtr sdk, string name)
    {
        this->sdk = sdk;
        this->name = name;

        delphi::objects::SysmgrTestCompleteReq::Mount(this->sdk, delphi::ReadWriteMode);
    }

    virtual void OnMountComplete()
    {
        auto obj = make_shared<delphi::objects::SysmgrTestCompleteReq>();
        obj->set_key("test_complete");
        obj->set_pid(getpid());
        sdk->SetObject(obj);

        printf("Send message %d\n", getpid());
    }
};

int main(int argc, char **argv) {
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    
    printf("Started test_complete\n");

    shared_ptr<TestCompleteService> svc = make_shared<TestCompleteService>(sdk, "TestCompleteService");
    sdk->RegisterService(svc);

    return sdk->MainLoop();
}
