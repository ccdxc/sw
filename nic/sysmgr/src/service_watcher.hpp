#ifndef __SERVICE_WATCHER_HPP__
#define __SERVICE_WATCHER_HPP__

#include <list>
#include <map>
#include <memory>
#include <set>

#include <ev++.h>

#include "custom_loop.hpp"

enum service_event_kind {
    SERVICE_EVENT_START = 0,
    SERVICE_EVENT_STOP,
    SERVICE_EVENT_HEARTBEAT,
    SERVICE_EVENT_MAX,
};

class ServiceEvent {
public:
    static std::shared_ptr<ServiceEvent> create(std::string name,
	enum service_event_kind kind);
    std::string name;
    enum service_event_kind kind;
};
typedef std::shared_ptr<ServiceEvent> ServiceEventPtr;

class ServiceReactor
{
public:
    virtual void on_service_start(std::string name) = 0;
    virtual void on_service_stop(std::string name) = 0;
    virtual void on_service_heartbeat(std::string name) = 0;
};
typedef std::shared_ptr<ServiceReactor> ServiceReactorPtr;

typedef std::set<ServiceReactorPtr> ServiceReactorSet;

class ServiceLoop : public CustomLoopReactor
{
public:
    std::list<ServiceEventPtr> event_queue;
    std::map<std::string, ServiceReactorSet> event_reactors[SERVICE_EVENT_MAX];
    static std::shared_ptr<ServiceLoop> instance;
public:
    static std::shared_ptr<ServiceLoop> getInstance();
    void queue_event(ServiceEventPtr event);
    void register_event_reactor(enum service_event_kind kind,
	std::string name, ServiceReactorPtr reactor);
    virtual void do_work();
    virtual bool has_work();
};
typedef std::shared_ptr<ServiceLoop> ServiceLoopPtr;

#endif // __SERVICE_WATCHER_HPP__
