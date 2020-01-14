#include "service_watcher.hpp"

#include <list>
#include <map>
#include <memory>
#include <set>

#include <ev++.h>

#include "log.hpp"

ServiceEventPtr ServiceEvent::create(std::string name,
    enum service_event_kind kind)
{
    if (kind < 0 || kind >= SERVICE_EVENT_MAX)
    {
	throw std::runtime_error("Unknown event kind");
    }
    ServiceEventPtr ev = std::make_shared<ServiceEvent>();
    ev->name = name;
    ev->kind = kind;

    return ev;
}

ServiceLoopPtr ServiceLoop::instance = nullptr;

ServiceLoopPtr ServiceLoop::getInstance()
{
    if (instance == nullptr)
    {
	instance = std::make_shared<ServiceLoop>();
	custom_loop_reactor_register(instance);
    }

    return instance;
}

void ServiceLoop::do_work()
{
    while (this->event_queue.size() > 0)
    {
	auto ev = this->event_queue.front();
	this->event_queue.pop_front();
	auto &reactors = this->event_reactors[ev->kind][ev->name];
	g_log->debug("Notyfing service reactors of %s", ev->name.c_str());
	for (auto reactor: reactors)
	{
	    if (ev->kind == SERVICE_EVENT_START)
	    {
		reactor->on_service_start(ev->name);
	    }
	    else if (ev->kind == SERVICE_EVENT_STOP)
	    {
		reactor->on_service_stop(ev->name);
	    }
	    else if (ev->kind == SERVICE_EVENT_HEARTBEAT)
	    {
		reactor->on_service_heartbeat(ev->name);
	    }
	    else
	    {
		throw std::runtime_error("Unknown Event Kind");
	    }
	}
    }
}

bool ServiceLoop::has_work()
{
    return (this->event_queue.size() > 0);
}

void ServiceLoop::queue_event(ServiceEventPtr event)
{
    this->event_queue.push_back(event);
}

void ServiceLoop::register_event_reactor(enum service_event_kind kind,
    std::string name, ServiceReactorPtr reactor)
{
    if (kind < 0 || kind >= SERVICE_EVENT_MAX)
    {
	throw std::runtime_error("Unknown event kind");
    }

    auto &reactors = this->event_reactors[kind][name];

    reactors.insert(reactor);
}
