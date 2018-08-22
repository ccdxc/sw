#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <list>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <sys/types.h>

using namespace std;

enum service_status
{
    WAITING,
    READY,
    STARTING,
    FAILED_TO_START,
    RUNNING,
    DIED,
};

class Service
{
  public:
    Service(string name, string command) : name(name), command(command)
    {
        status = WAITING;
    }

    int Compare(const Service &srv)
    {
        return this->name.compare(srv.name);
    }

    string get_name()
    {
        return this->name;
    }

    string get_command()
    {
        return this->command;
    }

    void set_status(enum service_status status)
    {
        this->status = status;
    }

    enum service_status get_status()
    {
        return this->status;
    }

    void add_depenency(weak_ptr<Service> service)
    {
        this->dependencies.insert(service);
    }

    void remove_dependency(weak_ptr<Service> service)
    {
        this->dependencies.erase(service);
    }

    void add_dependee(weak_ptr<Service> service)
    {
        this->dependees.push_back(service);
    }

    size_t dependenies_count()
    {
        return this->dependencies.size();
    }

    list<shared_ptr<Service> > get_dependees()
    {
        auto l = list<shared_ptr<Service> >();
        for (auto w: this->dependees)
        {
            l.push_back(w.lock());
        }
        return l;
    }

  private:
    const string name;
    const string command;
    set<weak_ptr<Service>, std::owner_less<std::weak_ptr<Service> > > dependencies;
    list<weak_ptr<Service> > dependees;
    enum service_status status;
    pid_t pid;
};

struct ServiceCompare
{
    int operator()(const shared_ptr<Service> &lhs,
                   const shared_ptr<Service> &rhs) const
    {
        return lhs->get_name().compare(rhs->get_name());
    }
};

typedef set<shared_ptr<Service>, std::owner_less<std::shared_ptr<Service> > > ServiceSet;

#endif
