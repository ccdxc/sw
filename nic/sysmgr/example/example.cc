// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <memory>
#include <iostream>
#include <string>

#include <stdio.h>

#include <boost/program_options.hpp>

#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"

using namespace std;
using namespace boost::program_options;

static penlog::LoggerPtr logger;

class ExampleService :
    public delphi::Service,
    public sysmgr::ServiceStatusReactor,
    public delphi::objects::SysmgrSystemStatusReactor,
    public delphi::objects::SysmgrProcessStatusReactor,
    public enable_shared_from_this<ExampleService>
{
private:
    delphi::SdkPtr delphi;
    string name;
    bool no_heartbeat;
    bool reboot;
    bool respawn;
    sysmgr::ClientPtr sysmgr;

public:
    ExampleService(delphi::SdkPtr delphi, string name, bool no_heartbeat,
        bool reboot, bool respawn) {
        this->delphi = delphi;
        this->name = name;
        this->no_heartbeat = no_heartbeat;
        this->reboot = reboot;
        this->respawn = respawn;
        this->sysmgr = sysmgr::CreateClient(delphi, name);
    }

    virtual string Name() {
        return this->name;
    }

    void register_mounts() {
        this->sysmgr->register_service_reactor(
            "TestCompleteService",
            shared_from_this());
        
        delphi::objects::SysmgrProcessStatus::Mount(this->delphi,
            delphi::ReadMode);
        delphi::objects::SysmgrSystemStatus::Mount(this->delphi,
            delphi::ReadMode);

        delphi::objects::SysmgrProcessStatus::Watch(this->delphi,
            shared_from_this());
        delphi::objects::SysmgrSystemStatus::Watch(this->delphi,
            shared_from_this());
    }

    virtual void OnMountComplete() {
        this->sysmgr->init_done();
        if (this->reboot)
        {
            this->sysmgr->restart_system();
        }
        else if (this->respawn)
        {
            logger->info("Requesting respawn\n");
            this->sysmgr->respawn_processes();
        }
    }

    virtual bool SkipHeartbeat() {
        return this->no_heartbeat;
    }

    virtual void ServiceUp(std::string name) {
    }

    virtual void ServiceDown(std::string) {
    }

    virtual delphi::error OnSysmgrSystemStatusCreate(
        delphi::objects::SysmgrSystemStatusPtr obj) {
        logger->info("SystemStatus System Halted\n");
        return delphi::error::OK();
    }

    virtual delphi::error OnSysmgrSystemStatusUpdate(
        delphi::objects::SysmgrSystemStatusPtr obj) {
        return delphi::error::OK();
    }

    virtual delphi::error OnSysmgrProcessStatusCreate(
        delphi::objects::SysmgrProcessStatusPtr obj) {
        logger->info("ProcessStatus {}, {}, {}, {}", obj->key(),
            obj->pid(), obj->state(), obj->exitreason());
        return delphi::error::OK();
    }

    virtual delphi::error OnSysmgrProcessStatusUpdate(
        delphi::objects::SysmgrProcessStatusPtr obj) {
        logger->info("ProcessStatus {}, {}, {}, {}", obj->key(),
            obj->pid(), obj->state(), obj->exitreason());
        return delphi::error::OK();
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
            ("reboot", "Reboot")
            ("respawn", "Respawn")
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
    logger = penlog::logger_init(vm["name"].as<string>());
    
    shared_ptr<ExampleService> svc = make_shared<ExampleService>(sdk,
        vm["name"].as<string>(), vm.count("no-heartbeat") > 0,
        vm.count("reboot") > 0, vm.count("respawn") > 0);
    svc->register_mounts();
    sdk->RegisterService(svc);
    
    return sdk->MainLoop();
}
