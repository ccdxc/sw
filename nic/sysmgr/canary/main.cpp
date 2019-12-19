// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;

class Canary : public delphi::Service
{
private:
    string name;
    delphi::SdkPtr delphi;
    sysmgr::ClientPtr sysmgr;

public:
    Canary(delphi::SdkPtr delphi, string name) {
        this->delphi = delphi;
        this->name = name;
        this->sysmgr = sysmgr::CreateClient(delphi, name);
    }

    virtual string Name() {
        return this->name;
    }

    virtual void OnMountComplete() override {
        this->sysmgr->init_done();
    }
};

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Please use: %s <SERICE_NAME>\n", argv[0]);
        return -1;
    }

    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    
    std::shared_ptr<Canary> svc = std::make_shared<Canary>(sdk, argv[1]);
    sdk->RegisterService(svc);
    
    return sdk->MainLoop();
}
