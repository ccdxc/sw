#ifndef __SERVICE_HPP__
#define __SERVICE_HPP__

#include <list>
#include <map>
#include <memory>

#include "child_watcher.hpp"
#include "pipedio.hpp"
#include "service_spec.hpp"
#include "service_watcher.hpp"
#include "timer_watcher.hpp"

enum service_dep_kind
{
    SERVICE_DEP_SERVICE,
    SERVICE_DEP_FILE,
};

class ServiceDep
{
public:
    static std::shared_ptr<ServiceDep> create(ServiceSpecDepPtr spec);
    enum service_dep_kind kind;
    std::string           service_name;
    std::string           file_name;
    bool                  isMet;
};
typedef std::shared_ptr<ServiceDep> ServiceDepPtr;

enum service_config_state
{
    SERVICE_CONFIG_STATE_ON,
    SERVICE_CONFIG_STATE_OFF,
};

enum service_running_state
{
    SERVICE_RUNNING_STATE_ON,
    SERVICE_RUNNING_STATE_OFF,
};

class Service : public std::enable_shared_from_this<Service>,
                public ServiceReactor,
                public ChildReactor,
                public TimerReactor
{
private:
    ServiceSpecPtr spec;
    std::list<ServiceDepPtr> dependencies;
    ChildWatcherPtr child_watcher;
    TimerWatcherPtr timer_watcher;
    pid_t pid;
    enum service_config_state config_state;
    enum service_running_state running_state;
    int restart_count;
    PipedIOPtr stdout_pipe;
    PipedIOPtr stderr_pipe;
    void check_dep_and_launch();
    void start_heartbeat();
    void launch();
    void fault(std::string reason);
public:
    static std::shared_ptr<Service> create(ServiceSpecPtr spec);
    virtual void on_service_start(std::string name);
    virtual void on_service_stop(std::string name);
    virtual void on_service_heartbeat(std::string name);
    virtual void on_child(pid_t pid);
    virtual void on_timer();
    void stop();
    void start();
    bool is_running();
    void reset_dependencies();
};
typedef std::shared_ptr<Service> ServicePtr;

#endif // __SERVICE_HPP__
