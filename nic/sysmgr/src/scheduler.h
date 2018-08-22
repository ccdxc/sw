#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <list>
#include <map>
#include <memory>
#include <set>

#include "service.h"
#include "spec.h"

enum action_type_t
{
  WAIT,
  LAUNCH,
  REBOOT,
};

class Action
{
public:
  Action(action_type_t type) : type(type){};
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
  void service_started(pid_t pid);
  void service_died(pid_t pid);

private:
  void service_ready(shared_ptr<Service> srv);
  shared_ptr<Service> get_for_pid(pid_t pid);
  map<pid_t, shared_ptr<Service> > pids;
  map<string, shared_ptr<Service> > services;
  ServiceSet ready;
  ServiceSet starting;
  ServiceSet waiting;
  ServiceSet running;
  ServiceSet dead;
};

#endif