// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;

class ExampleService :
   public delphi::Service,
   public sysmgr::ServiceStatusReactor,
   public enable_shared_from_this<ExampleService>
{
private:
   delphi::SdkPtr delphi;
   string name;
   sysmgr::ClientPtr sysmgr;

public:
   ExampleService(delphi::SdkPtr delphi, string name)
   {
      this->delphi = delphi;
      this->name = name;
      this->sysmgr = sysmgr::CreateClient(delphi, name);
   }

   void register_mounts()
   {
      this->sysmgr->register_service_reactor(
	 "TestCompleteService",
	 shared_from_this());
   }

   virtual void OnMountComplete()
   {
      this->sysmgr->init_done();
   }

   virtual void ServiceUp(std::string name)
   {
   }

   virtual void ServiceDown(std::string)
   {
   }
};

int main(int argc, char **argv) {
   delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
   
   shared_ptr<ExampleService> svc = make_shared<ExampleService>(sdk, "ExampleService");
   svc->register_mounts();
   sdk->RegisterService(svc);

   return sdk->MainLoop();
}
