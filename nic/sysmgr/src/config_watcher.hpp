#ifndef __CONFIG_WATCHER_HPP__
#define __CONFIG_WATCHER_HPP__

#include <list>
#include <memory>

#include <ev++.h>

#include "custom_loop.hpp"
#include "service_spec.hpp"

class ConfigReactor
{
public:
    virtual void on_config_add(ServiceSpecPtr spec) = 0;
};
typedef std::shared_ptr<ConfigReactor> ConfigReactorPtr;

class ConfigLoop : public CustomLoopReactor
{
private:
    std::list<ServiceSpecPtr> config_queue;
    std::list<ConfigReactorPtr> config_reactors;
    static std::shared_ptr<ConfigLoop> instance;
public:
    static std::shared_ptr<ConfigLoop> getInstance();
    void queue_config(ServiceSpecPtr config);
    void register_config_reactor(ConfigReactorPtr watcher);
    virtual void do_work();
    virtual bool has_work();
};
typedef std::shared_ptr<ConfigLoop> ConfigLoopPtr;

#endif // __CONFIG_WATCHER_HPP__
