#include "config_watcher.hpp"

#include <list>
#include <memory>

#include "custom_loop.hpp"
#include "service_spec.hpp"

ConfigLoopPtr ConfigLoop::instance = nullptr;

std::shared_ptr<ConfigLoop> ConfigLoop::getInstance()
{
    if (instance == nullptr)
    {
	instance = std::make_shared<ConfigLoop>();
	custom_loop_reactor_register(instance);
    }
    
    return instance;
}

void ConfigLoop::do_work()
{
    while (this->config_queue.size() > 0)
    {
	auto spec = this->config_queue.front();
	this->config_queue.pop_front();
	for (auto reactor: this->config_reactors)
	{
	    reactor->on_config_add(spec);
	}
    }
}

bool ConfigLoop::has_work()
{
    return (this->config_queue.size() > 0);
}

void ConfigLoop::queue_config(ServiceSpecPtr config)
{
    this->config_queue.push_back(config);
}

void ConfigLoop::register_config_reactor(ConfigReactorPtr reactor)
{
    this->config_reactors.push_back(reactor);
}
