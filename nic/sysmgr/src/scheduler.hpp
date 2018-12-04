#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <list>
#include <map>
#include <memory>
#include <set>

#include "nic/utils/penlog/lib/penlog.hpp"

#include "service.hpp"
#include "spec.hpp"
#include "watchdog.hpp"

enum action_type_t
{
    WAIT,
    LAUNCH,
    REBOOT,
};

class Action
{
public:
    Action(action_type_t type, int sleep = 0,
        list<shared_ptr<Service> > launch_list=list<shared_ptr<Service> >()):
        type(type), sleep(sleep), launch_list(launch_list) {};
    const action_type_t type;
    int sleep;
    list<shared_ptr<Service> > launch_list;
};

class ServiceStatusWatcher {
public:
    virtual void service_status_changed(const string &name, pid_t pid,
        enum service_status status, string reason) = 0;    
};
typedef shared_ptr<ServiceStatusWatcher> ServiceStatusWatcherPtr;

class Scheduler
{
public:
    Scheduler(vector<Spec> specs);
    Scheduler(vector<Spec> specs, penlog::LoggerPtr logger);
    void debug();
    unique_ptr<Action> next_action();
    void service_launched(shared_ptr<Service> srv, pid_t pid);
    void service_started(const string &name);
    void service_started(pid_t pid);
    void service_died(const string &name, int status);
    void service_died(pid_t pid, int status);
    void heartbeat(const string &name);
    void heartbeat(pid_t pid);
    void set_service_status_watcher(ServiceStatusWatcherPtr watcher);

private:
    penlog::LoggerPtr logger;
    void service_ready(shared_ptr<Service> srv);
    shared_ptr<Service> get_for_pid(pid_t pid);
    shared_ptr<Service> get_for_name(const string &name);
    map<pid_t, shared_ptr<Service> > pids;
    map<string, shared_ptr<Service> > services;
    ServiceSet ready;
    ServiceSet starting;
    ServiceSet waiting;
    ServiceSet running;
    ServiceSet dead;

    list<shared_ptr<Service> > next_launch();
    bool deadlocked();
    bool should_reboot();

    shared_ptr<Watchdog> watchdog = make_shared<Watchdog>();

    ServiceStatusWatcherPtr watcher;
};

#endif
