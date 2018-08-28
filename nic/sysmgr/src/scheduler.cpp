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
        ERR("PID {} not found", pid);
    }
    assert(s != this->pids.end());

    return s->second;
}

shared_ptr<Service> Scheduler::get_for_name(const string &name)
{
    auto s = this->services.find(name);
    if (s == this->services.end())
    {
        ERR("Name {} not found", name);
    }
    assert(s != this->services.end());

    return s->second;
}

Scheduler::Scheduler(vector<Spec> specs)
{
    // Populate the services map based on the spec
    for (auto &sp : specs)
    {
        auto service = make_shared<Service>(sp.name, sp.command);
        this->services.insert(pair<const string &, shared_ptr<Service>>(service->get_name(), service));
        INFO("Added service {}", service->get_name());
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
                INFO("{} depends on {}", dependee->get_name(), dependency->get_name());

                dependee->set_status(WAITING);
                this->waiting.insert(dependee);
            }
        }
    }
}

unique_ptr<Action> Scheduler::next_action()
{
    INFO("Waiting: {}, Starting: {}, Ready: {}, Running: {}, Dead: {}",
        this->waiting.size(), this->starting.size(), this->ready.size(),
        this->running.size(), this->dead.size());
    if (this->ready.size() > 0)
    {
        auto action = unique_ptr<Action>(new Action(LAUNCH));

        for (auto s : this->ready)
        {
            action->launch_list.push_back(s);
        }

        return action;
    }
    else // no service in ready list
    {
        // The waiting list is not empty, but we are not waiting on any more
        // services to run and the ready list is empty. This means we have a 
        // loop in dependencies. This should be caught by unit-tests run during
        // build time.
        if ((this->starting.size() == 0) && (this->waiting.size() > 0))
        {
            return unique_ptr<Action>(new Action(REBOOT));
        }
    }
    return unique_ptr<Action>(new Action(WAIT));
}

void Scheduler::service_ready(shared_ptr<Service> service)
{
    assert(service->get_status() == WAITING);

    service->set_status(READY);
    this->waiting.erase(service);
    this->ready.insert(service);
    INFO("{} -> ready", service->get_name());
}

void Scheduler::service_launched(shared_ptr<Service> service, pid_t pid)
{
    assert(service->get_status() == READY);

    service->set_status(STARTING);
    service->pid = pid;
    this->ready.erase(service);
    this->starting.insert(service);

    this->pids.insert(pair<pid_t, shared_ptr<Service>>(pid, service));
    INFO("{} -> launched with pid({})", service->get_name(), pid);
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
    INFO("{} -> started", service->get_name());

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

void Scheduler::service_died(pid_t pid)
{
    auto service = this->get_for_pid(pid);
    assert(service->get_status() == RUNNING);

    service->set_status(DIED);
    this->pids.erase(pid);
    this->running.erase(service);
    this->starting.erase(service);
    this->dead.insert(service);
    INFO("{} -> died", service->get_name());
}

void Scheduler::debug()
{
    INFO("- Debug -");
    INFO("Ready list:");
    for (auto kv : this->services)
    {
        if (auto srv = kv.second)
        {
            INFO("{}: {}", srv->get_name().c_str(), srv->get_status());
        }
    }
    INFO("- Debug end -");
}
