#ifndef __SERVICE_FACTORY_HPP__
#define __SERVICE_FACTORY_HPP__

#include <map>
#include <memory>
#include <string>

#include "config_watcher.hpp"
#include "service.hpp"
#include "signal_watcher.hpp"

class ServiceFactory : public ConfigReactor,
		       public SignalReactor
{
private:
    static std::shared_ptr<ServiceFactory> instance;
    std::map<std::string, ServicePtr>      services;
    SignalWatcherPtr                       term_watcher;
    SignalWatcherPtr                       int_watcher;
public:
    static std::shared_ptr<ServiceFactory> getInstance();
    virtual void on_config_add(ServiceSpecPtr spec);
    void load_config(std::string path);
    virtual void on_signal(int sig);
};
typedef std::shared_ptr<ServiceFactory> ServiceFactoryPtr;

#endif //__SERVICE_FACTORY_HPP__

