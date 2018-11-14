// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include <boost/program_options.hpp>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;
using namespace boost::program_options;

class ExampleService :
    public delphi::Service,
    public sysmgr::ServiceStatusReactor,
    public enable_shared_from_this<ExampleService>
{
private:
    delphi::SdkPtr delphi;
    string name;
    bool no_heartbeat;
    sysmgr::ClientPtr sysmgr;

public:
    ExampleService(delphi::SdkPtr delphi, string name, bool no_heartbeat) {
	this->delphi = delphi;
	this->name = name;
	this->no_heartbeat = no_heartbeat;
	this->sysmgr = sysmgr::CreateClient(delphi, name);
    }

    virtual string Name() {
	return this->name;
    }

    void register_mounts() {
	this->sysmgr->register_service_reactor(
	    "TestCompleteService",
	    shared_from_this());
    }

    virtual void OnMountComplete() {
	this->sysmgr->init_done();
    }

    virtual bool SkipHeartbeat() {
	return this->no_heartbeat;
    }

    virtual void ServiceUp(std::string name) {
    }

    virtual void ServiceDown(std::string) {
    }
};

int main(int argc, char *argv[]) {
    variables_map vm;
    try
    {
    	options_description desc{"Options"};
    	desc.add_options()
    	    ("name", value<string>()->default_value("example"), "Name")
    	    ("exit", value<int>(), "Exit code")
	    ("no-heartbeat", "No heartbeat");
    	store(parse_command_line(argc, argv, desc), vm);
    }
    catch (const error &ex)
    {
    	std::cerr << ex.what() << '\n';
    }

    if (vm.count("exit") > 0) {
    	std::exit(vm["exit"].as<int>());
    }
    
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    
    shared_ptr<ExampleService> svc = make_shared<ExampleService>(sdk,
	vm["name"].as<string>(), vm.count("no-heartbeat") > 0);
    svc->register_mounts();
    sdk->RegisterService(svc);
    
    return sdk->MainLoop();
}
