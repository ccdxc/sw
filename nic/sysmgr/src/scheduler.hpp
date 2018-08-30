#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <list>
#include <map>
#include <memory>
#include <set>

#include "service.hpp"
#include "spec.hpp"

enum action_type_t
{
  WAIT,
  LAUNCH,
  REBOOT,
};

class Action
{
public:
  Action(action_type_t type, 
    list<shared_ptr<Service> > launch_list=list<shared_ptr<Service> >()):
    type(type), launch_list(launch_list) {};
  const action_type_t type;
  list<shared_ptr<Service> > launch_list;
};

class Scheduler
{
public:
  Scheduler(vector<Spec> specs);
  void debug();
  unique_ptr<Action> next_action();
  void service_launched(shared_ptr<Service> srv, pid_t pid);
  void service_started(const string &name);
  void service_started(pid_t pid);
  void service_died(const string &name);
  void service_died(pid_t pid);

private:
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
};

#endif
