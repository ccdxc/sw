#include "service.hpp"

#include <memory>

#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"

#include "eventlogger.hpp"
#include "service_watcher.hpp"
#include "timer_watcher.hpp"
#include "utils.hpp"

ServiceDepPtr ServiceDep::create(ServiceSpecDepPtr spec)
{
    ServiceDepPtr dep = std::make_shared<ServiceDep>();

    if (spec->kind == SERVICE_SPEC_DEP_SERVICE)
    {
        dep->kind = SERVICE_DEP_SERVICE;
    }
    else if (spec->kind == SERVICE_SPEC_DEP_FILE)
    {
        dep->kind = SERVICE_DEP_FILE;
    }
    else
    {
        throw new std::runtime_error("Unknown dependency kind");
    }
    dep->service_name = spec->service_name;
    dep->file_name = spec->file_name;
    dep->isMet = false;

    logger->info("Created service dependency {}, {}, {}",
        dep->kind, dep->service_name, dep->isMet);

    return dep;
}

void Service::launch()
{
    this->pid = ::launch(this->spec->name, this->spec->command);
    logger->info("Launched {}({}) using {}", this->spec->name, this->pid,
        this->spec->command);

    this->child_watcher = ChildWatcher::create(this->pid, shared_from_this());
    this->start_heartbeat();
}

void Service::check_dep_and_launch()
{
    logger->info("Checking dependencies for {}",
        this->spec->name);
    for (auto dep: this->dependencies)
    {
        logger->info("Dependency {} is{} met",
            dep->service_name, dep->isMet?"":" not");
        if (!dep->isMet)
        {
            return;
        }
    }
    logger->info("All dependencies are go for {}",
        this->spec->name);
    launch();
}

void Service::start_heartbeat()
{
    if (this->spec->timeout == 0.)
    {
        return;
    }
    if (this->timer_watcher == nullptr)
    {
        this->timer_watcher = TimerWatcher::create(this->spec->timeout,
            this->spec->timeout, shared_from_this());
    }
    this->timer_watcher->repeat();
}

ServicePtr Service::create(ServiceSpecPtr spec)
{
    ServicePtr svc = std::make_shared<Service>();

    logger->info("Created service {}", spec->name);

    for (auto spec_dep: spec->dependencies)
    {
        auto dep = ServiceDep::create(spec_dep);
        svc->dependencies.push_back(dep);
        ServiceLoop::getInstance()->register_event_reactor(SERVICE_EVENT_START,
            dep->service_name, svc);
    }
    
    svc->spec = spec;
    svc->check_dep_and_launch();
    svc->child_watcher = nullptr;
    svc->timer_watcher = nullptr;
    svc->restart_count = 0;
    ServiceLoop::getInstance()->register_event_reactor(SERVICE_EVENT_START,
        spec->name, svc);
    ServiceLoop::getInstance()->register_event_reactor(SERVICE_EVENT_HEARTBEAT,
        spec->name, svc);

    EventLogger::getInstance()->LogServiceEvent(
        sysmgr_events::SERVICE_PENDING, "Service %s pending", spec->name);
    
    return svc;
}

void Service::on_service_start(std::string name)
{
    if (name == this->spec->name)
    {
        logger->info("Service {} started", name);
        EventLogger::getInstance()->LogServiceEvent(
            sysmgr_events::SERVICE_UP, "Service %s started", spec->name);
        return;
    }

    for (auto dep: this->dependencies)
    {
        // Todo: Fixme: Linear complexity
        if (dep->service_name == name)
        {
            dep->isMet = true;
        }
    }
    this->check_dep_and_launch();
}

void Service::on_service_stop(std::string name)
{
}

void Service::on_service_heartbeat(std::string name)
{
    logger->debug("{} got hearbeat!", this->spec->name);
    this->timer_watcher->repeat();
}

void Service::on_child(pid_t pid)
{
    std::string reason = parse_status(this->child_watcher->get_status());

    logger->info("Service {} {}", this->spec->name, reason);
    EventLogger::getInstance()->LogServiceEvent(sysmgr_events::SERVICE_DOWN,
        "Service %s stopped", this->spec->name);

    if (this->spec->flags & COPY_STDOUT_ON_CRASH) {
        save_stdout_stderr(this->spec->name, pid);
    }

    if (this->spec->flags & RESTARTABLE && this->restart_count < 5) {
        this->restart_count += 1;
        this->launch();
        return;
    }

    auto obj = std::make_shared<delphi::objects::SysmgrProcessStatus>();

    obj->set_key(this->spec->name);
    obj->set_pid(pid);
    obj->set_state(::sysmgr::Died);
    obj->set_exitreason(reason);

    delphi_sdk->QueueUpdate(obj);
     
}

void Service::on_timer()
{
    logger->info("Service {} timed out", this->spec->name);
    this->timer_watcher->stop();

    logger->info("System in fault mode");
    auto obj = std::make_shared<delphi::objects::SysmgrSystemStatus>();
    obj->set_state(::sysmgr::Fault);
    delphi_sdk->QueueUpdate(obj);
}
