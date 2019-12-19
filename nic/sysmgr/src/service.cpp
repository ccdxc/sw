#include "service.hpp"

#include <memory>

#include "cgroups.hpp"
#include "events_api.hpp"
#include "fault_watcher.hpp"
#include "pipedio.hpp"
#include "log.hpp"
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

    glog->info("Created service dependency {}, {}, {}",
        dep->kind, dep->service_name, dep->isMet);

    return dep;
}

void Service::launch()
{
    process_t new_process;

    ::launch(this->spec->name, this->spec->command, this->spec->cpu_affinity,
             this->spec->mem_limit, &new_process);
    this->pid = new_process.pid;
    this->stdout_pipe = PipedIO::create(new_process.stdout,
        get_logname_for_process(this->spec->name, this->pid, "out"));
    this->stderr_pipe = PipedIO::create(new_process.stderr,
        get_logname_for_process(this->spec->name, this->pid, "err"));
    glog->info("Launched {}({}) using {} with affinity 0x{:x}",
               this->spec->name, this->pid, this->spec->command,
               this->spec->cpu_affinity);
    
    if (this->child_watcher != nullptr)
    {
        this->child_watcher->stop();
        this->child_watcher = nullptr;
    }
    this->child_watcher = ChildWatcher::create(this->pid, shared_from_this());
    this->start_heartbeat();
    this->running_state = SERVICE_RUNNING_STATE_ON;
}

void Service::check_dep_and_launch()
{
    glog->info("Checking dependencies for {}",
        this->spec->name);
    for (auto dep: this->dependencies)
    {
        glog->info("Dependency {} is{} met",
            dep->service_name, dep->isMet?"":" not");
        if (!dep->isMet)
        {
            return;
        }
    }
    glog->info("All dependencies are go for {}",
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

    glog->info("Created service {}", spec->name);

    // If it has memory limitation, create a cgroup for it
    if (spec->mem_limit > 0.0) {
        glog->info("Creating cgroup with size %d", spec->mem_limit);
        cg_create(spec->name.c_str(), (size_t)spec->mem_limit * (1024 * 1024));
    }
    
    for (auto spec_dep: spec->dependencies)
    {
        auto dep = ServiceDep::create(spec_dep);
        svc->dependencies.push_back(dep);
        ServiceLoop::getInstance()->register_event_reactor(SERVICE_EVENT_START,
            dep->service_name, svc);
    }

    svc->spec = spec;
    svc->child_watcher = nullptr;
    svc->timer_watcher = nullptr;
    svc->restart_count = 0;
    svc->stdout_pipe = nullptr;
    svc->stderr_pipe = nullptr;
    svc->config_state = SERVICE_CONFIG_STATE_ON;
    svc->running_state = SERVICE_RUNNING_STATE_OFF;
    svc->check_dep_and_launch();
    ServiceLoop::getInstance()->register_event_reactor(SERVICE_EVENT_START,
        spec->name, svc);
    ServiceLoop::getInstance()->register_event_reactor(SERVICE_EVENT_HEARTBEAT,
        spec->name, svc);

    return svc;
}

void Service::on_service_start(std::string name)
{
    if (name == this->spec->name)
    {
        glog->info("Service {} started", name);
        g_events->ServiceStartedEvent(spec->name);
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
    glog->debug("{} got hearbeat!", this->spec->name);
    if (this->timer_watcher)
    {
        this->timer_watcher->repeat();
    }
}

void Service::fault(std::string reason)
{
    reason = reason + " - " + this->spec->name;

    if (this->spec->kind == SERVICE_ONESHOT) {
        glog->info("Service {} is oneshot, not setting fault",
                   this->spec->name);
        return;
    }
    if (this->config_state == SERVICE_CONFIG_STATE_OFF) {
        glog->info("Service {} shutdown on purpose, not setting fault",
            this->spec->name);
        return;
    }
    if (this->spec->flags & PANIC_ON_FAILURE) {
        glog->info("{} is critical. Triggering watchdog", this->spec->name);
        FaultLoop::getInstance()->set_fault(reason);
    }
    
    glog->info("System in fault mode ({})", reason);
    g_bus->SystemFault(reason);
}

void Service::on_child(pid_t pid)
{
    this->running_state = SERVICE_RUNNING_STATE_OFF;

    std::string reason = parse_status(this->child_watcher->get_status());

    glog->info("Service {} {}", this->spec->name, reason);
    g_events->ServiceStoppedEvent(this->spec->name);

    run_debug(this->pid);

    // SERVICE_CONFIG_STATE_OFF means we killed the process, don't worry about
    // copying stdout
    if (this->config_state == SERVICE_CONFIG_STATE_ON &&
        this->spec->flags & COPY_STDOUT_ON_CRASH) {

        save_stdout_stderr(this->spec->name, pid);
    }

    // SERVICE_CONFIG_STATE_OFF means we killed the process, don't try to
    // restart it
    if (this->config_state == SERVICE_CONFIG_STATE_ON &&
        this->spec->flags & RESTARTABLE && this->restart_count < 5) {

        this->restart_count += 1;
        this->launch();
        return;
    }

    g_bus->ProcessDied(this->spec->name, pid, reason);

    ServiceLoop::getInstance()->queue_event(
        ServiceEvent::create(this->spec->name, SERVICE_EVENT_STOP));

    this->fault("Process died");
}

void Service::on_timer()
{
    glog->warn("Service {} timed out. Killing it", this->spec->name);
    this->timer_watcher->stop();

    // Killing the process will end `on_child` getting called where we
    // will run debugs for the process and set fault
    kill(this->pid, SIGQUIT);
}

void Service::reset_dependencies()
{
    for (auto dep: this->dependencies)
    {
        dep->isMet = false;
    }
}

void Service::stop()
{
    this->config_state = SERVICE_CONFIG_STATE_OFF;
    if (this->timer_watcher != nullptr) {
        this->timer_watcher->stop();
        this->timer_watcher = nullptr;
    }
    this->restart_count = 0;
    this->reset_dependencies();
    glog->info("Killing {}({})", this->spec->name, this->pid);
    kill(this->pid, SIGKILL);
}

void Service::start()
{
    this->config_state = SERVICE_CONFIG_STATE_ON;
    this->check_dep_and_launch();
}

bool Service::is_running()
{
    return (this->running_state == SERVICE_RUNNING_STATE_ON);
}
