// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"

using namespace std;

class ExampleService : public delphi::Service, public enable_shared_from_this<ExampleService>
{
  private:
    delphi::SdkPtr sdk;
    string name;

  public:
    ExampleService(delphi::SdkPtr sdk, string name)
    {
        this->sdk = sdk;
        this->name = name;

        delphi::objects::SysmgrServiceStatus::Mount(this->sdk, delphi::ReadWriteMode);
    }

    virtual void OnMountComplete()
    {
        auto obj = make_shared<delphi::objects::SysmgrServiceStatus>();

        obj->set_key("example");
        obj->set_pid(getpid());
        sdk->SetObject(obj);
    }
};

int main(int argc, char **argv) {
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());

    shared_ptr<ExampleService> svc = make_shared<ExampleService>(sdk, "ExampleService");
    sdk->RegisterService(svc);

    return sdk->MainLoop();
}
