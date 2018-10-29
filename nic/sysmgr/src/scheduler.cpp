#include <assert.h>
#include <memory>

#include "logger.hpp"
#include "scheduler.hpp"
#include "spec.hpp"

using namespace std;

shared_ptr<Service> Scheduler::get_for_pid(pid_t pid)
{
    auto s = this->pids.find(pid);
    if (s == this->pids.end())
    {
        DEBUG("PID {} not found", pid);
        return nullptr;
    }

    return s->second;
}

shared_ptr<Service> Scheduler::get_for_name(const string &name)
{
    auto s = this->services.find(name);
    if (s == this->services.end())
    {
       DEBUG("Name {} not found", name);
    }
    assert(s != this->services.end());

    return s->second;
}

Scheduler::Scheduler(vector<Spec> specs)
{
    // Populate the services map based on the spec
    for (auto &sp : specs)
    {
        auto service = make_shared<Service>(sp.name, sp.command, 
            (sp.flags & RESTARTABLE) == RESTARTABLE,
            (sp.flags & NO_WATCHDOG) == NO_WATCHDOG);
        this->services.insert(pair<const string &, shared_ptr<Service>>(service->name, service));
        INFO("Added service {}", service->name);
    }

    // Populate the dependencies of the services or add them to the read list if they have no dependencies
    for (auto &sp : specs)
    {
        shared_ptr<Service> &dependee = services.find(sp.name)->second;
        // If there are no dependencies, the service is ready to be launched
        if (sp.dependencies.size() == 0)
        {
            this->service_ready(dependee);
        }
        else
        {
            for (auto &dependency_name : sp.dependencies)
            {
                shared_ptr<Service> &dependency = services.find(dependency_name)->second;
                dependee->add_depenency(dependency);
                dependency->add_dependee(dependee);
                INFO("{} depends on {}", dependee->name, dependency->name);

                dependee->set_status(WAITING);
                this->waiting.insert(dependee);
            }
        }
    }
}

list<shared_ptr<Service> > Scheduler::next_launch()
{
    auto launch_list = list<shared_ptr<Service> >();

    if (this->dead.size() > 0)
    {
        for (auto s : this->dead)
        {
            if (s->is_restartable)
            {
                launch_list.push_back(s);
            }
        }
    }
    if (this->ready.size() > 0)
    {
        auto action = unique_ptr<Action>(new Action(LAUNCH));

        for (auto s : this->ready)
        {
            launch_list.push_back(s);
        }
    }
    return launch_list;
}

bool Scheduler::deadlocked()
{
    // we have processes waiting for other dependency resolution but no more 
    // processes to launch and no processes waiting to come up; deadlock
    return ((this->waiting.size() != 0) && (this->starting.size() == 0));
}

bool Scheduler::should_reboot()
{
    if (deadlocked())
    {
        INFO("Deadlock restart");
        return true;
    }
    for (auto s: this->dead)
    {
        if (s->is_restartable == false)
        {
            INFO("Non-restartable process({}) restart", s->name);
            return true;
        }
    }
    if (this->watchdog->expired().size() > 0)
    {
        INFO("Watchdog restart");
        for (auto ex: this->watchdog->expired()) {
            INFO("Expired watchdog process: {}", ex);
        }
        return true;
    }
    return false;
}

unique_ptr<Action> Scheduler::next_action()
{
    DEBUG("Waiting: {}, Starting: {}, Ready: {}, Running: {}, Dead: {}",
        this->waiting.size(), this->starting.size(), this->ready.size(),
        this->running.size(), this->dead.size());

    auto launch_list = next_launch();
    if (launch_list.size() > 0)
    {
        return unique_ptr<Action>(new Action(LAUNCH, 0, launch_list));
    }
    
    if (should_reboot())
    {
        return unique_ptr<Action>(new Action(REBOOT, 0));
    }

    return unique_ptr<Action>(new Action(WAIT, this->watchdog->next_tick()));
}

void Scheduler::service_ready(shared_ptr<Service> service)
{
    assert(service->get_status() == WAITING);

    service->set_status(READY);
    this->waiting.erase(service);
    this->ready.insert(service);
    INFO("{} -> ready", service->name);
}

void Scheduler::service_launched(shared_ptr<Service> service, pid_t pid)
{
    assert((service->get_status() == READY) || (service->get_status() == DIED));

    service->set_status(STARTING);
    service->pid = pid;
    this->ready.erase(service);
    this->dead.erase(service);
    this->starting.insert(service);

    if (service->is_watchdog_disabled == false)
    {
        this->watchdog->refresh(service->name);
    }

    this->pids.insert(pair<pid_t, shared_ptr<Service>>(pid, service));
    INFO("{} -> launched with pid({})", service->name, pid);
}

void Scheduler::service_started(const string &name)
{
    auto service = this->get_for_name(name);
    this->service_started(service->pid);
}

void Scheduler::service_started(pid_t pid)
{
    auto service = this->get_for_pid(pid);
    assert(service->get_status() == STARTING);

    service->set_status(RUNNING);
    this->starting.erase(service);
    this->running.insert(service);
    INFO("{} -> started", service->name);

    // Remove it from all dependencies. If a dependee has no more dependencies, move it to ready.
    for (auto dependee : service->get_dependees())
    {
        assert(dependee);
        dependee->remove_dependency(service);
        if (dependee->dependenies_count() == 0)
        {
            service_ready(dependee);
        }
    }
}

void Scheduler::service_died(const string &name)
{
    auto service = this->get_for_name(name);
    this->service_died(service->pid);
}

void Scheduler::service_died(pid_t pid)
{
    auto service = this->get_for_pid(pid);
    assert((service->get_status() == RUNNING) || (service->get_status() == STARTING));

    service->set_status(DIED);
    this->pids.erase(pid);
    this->running.erase(service);
    this->starting.erase(service);
    this->dead.insert(service);
    INFO("{} -> died", service->name);
}

void Scheduler::debug()
{
    INFO("- Debug start -");
    INFO("Ready list:");
    for (auto kv : this->services)
    {
        if (auto srv = kv.second)
        {
            INFO("{}: {}", srv->name.c_str(), srv->get_status());
        }
    }
    INFO("- Debug end -");
}

void Scheduler::heartbeat(pid_t pid)
{
    auto service = this->get_for_pid(pid);
    if (service == nullptr) {
        return;
    }
    DEBUG("Received heartbeat from {}", service->name);

    watchdog->refresh(service->name);
}
