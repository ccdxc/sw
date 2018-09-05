// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;

class TestCompleteService : public delphi::Service, public enable_shared_from_this<TestCompleteService>
{
  private:
    delphi::SdkPtr sdk;
    string name;
    sysmgr::Client sysmgr;

  public:
    TestCompleteService(delphi::SdkPtr sdk, string name): sysmgr(sdk, name)
    {
        this->sdk = sdk;
        this->name = name;
    }

    virtual void OnMountComplete()
    {
        this->sysmgr.test_done();
    }
};

int main(int argc, char **argv) {
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    
    printf("Started test_complete\n");

    shared_ptr<TestCompleteService> svc = make_shared<TestCompleteService>(sdk, "TestCompleteService");
    sdk->RegisterService(svc);

    return sdk->MainLoop();
}
