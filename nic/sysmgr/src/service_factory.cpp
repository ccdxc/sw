#include "service_factory.hpp"

#include <memory>
#include <string>

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ev++.h>

#include "config_watcher.hpp"
#include "log.hpp"
#include "service.hpp"
#include "service_spec.hpp"
#include "utils.hpp"

namespace pt = boost::property_tree;

static enum service_kind
service_kind_from_obj(pt::ptree obj)
{

    if (obj.count("kind") == 0) {
        return SERVICE_DAEMON;
    } else if (obj.get<std::string>("kind") == "daemon") {
        return SERVICE_DAEMON;
    } else if (obj.get<std::string>("kind") == "oneshot") {
        return SERVICE_ONESHOT;
    } else {
        throw std::runtime_error("Unknown kind: " + 
                                 obj.get<std::string>("kind"));
    }
}

static int flags_from_obj(pt::ptree obj)
{
   int flags = DEFAULT_SPEC_FLAGS;
   
   for (auto flag: obj.get_child("flags")) {
       if (boost::iequals(flag.second.data(), "restartable")) {
           flags |= RESTARTABLE;
       } else if (boost::iequals(flag.second.data(), "save_stdout_on_crash")) {
           flags |= COPY_STDOUT_ON_CRASH;
       } else if (boost::iequals(flag.second.data(), "critical")) {
           flags |= PANIC_ON_FAILURE;
       } else if (boost::iequals(flag.second.data(), "cap_stdout_stderr")) {
           flags |= CAP_STDOUT_STDERR;
       } else {
           throw std::runtime_error("Unknown flag: " + flag.second.data());
       }
   }
   return flags;
}

static ServiceSpecDepPtr dependency_from_obj(pt::ptree obj)
{
    ServiceSpecDepPtr dependency = ServiceSpecDep::create();

    if (obj.get<std::string>("kind") == "service")
    {
        dependency->kind = SERVICE_SPEC_DEP_SERVICE;
        dependency->service_name = obj.get<std::string>("service-name");
    }
    else
    {
        throw std::runtime_error("Unknown dependency kind: " + obj.data());
    }

    return dependency;
}

static std::vector<ServiceSpecDepPtr> dependencies_from_obj(pt::ptree obj)
{
   std::vector<ServiceSpecDepPtr> dependencies;

   for (auto dep: obj.get_child("dependencies")) {
       dependencies.push_back(dependency_from_obj(dep.second));
   }

   return dependencies;
}

static unsigned long cpu_affinity_from_obj(pt::ptree obj)
{
    std::string val = obj.get<std::string>("cpu-affinity", "0xffffffff");
    return std::stoul(val, nullptr, 16);
}

static ServiceSpecPtr spec_from_obj(pt::ptree obj)
{
    ServiceSpecPtr spec = ServiceSpec::create();
    spec->name = obj.get<std::string>("name");
    spec->command =  obj.get<std::string>("command");
    spec->kind = service_kind_from_obj(obj);
    spec->flags = flags_from_obj(obj);
    spec->timeout = obj.get<double>("timeout");
    if (obj.count("memory-limit") == 0) {
        spec->mem_limit = 0.0;
    } else {
        spec->mem_limit = obj.get<double>("memory-limit");
    }
    spec->dependencies = dependencies_from_obj(obj);
    spec->cpu_affinity = cpu_affinity_from_obj(obj);

    return spec;
}

ServiceFactoryPtr ServiceFactory::instance = nullptr;

ServiceFactoryPtr ServiceFactory::getInstance()
{
    if (instance != nullptr)
    {
        return instance;
    }

    instance = std::make_shared<ServiceFactory>();
    ConfigLoop::getInstance()->register_config_reactor(instance);
    instance->int_watcher = SignalWatcher::create(SIGINT, instance);
    instance->term_watcher = SignalWatcher::create(SIGTERM, instance);
    instance->respawn_in_progress = false;
    return instance;
}

void ServiceFactory::on_config_add(ServiceSpecPtr spec)
{
    g_log->info("Config for %s added", spec->name.c_str());
    this->services[spec->name] = Service::create(spec);
    ServiceLoop::getInstance()->register_event_reactor(SERVICE_EVENT_STOP,
        spec->name, instance);
}

void ServiceFactory::load_config(std::string path)
{
    pt::ptree root;

    g_log->info("Loading config from %s", path.c_str());

    if (access(path.c_str(), F_OK) == -1)
    {
        g_log->err("Config file '%s' not found", path.c_str());
        return;
    }
    
    try
    {
        read_json(path, root);
    }
    catch (...)
    {
        g_log->err("Failed to parse config file %s", path.c_str());
    }

    for (auto obj: root)
    {
        ConfigLoop::getInstance()->queue_config(spec_from_obj(obj.second));
    }
}

void ServiceFactory::on_signal(int sig)
{
    ev::default_loop loop;

    g_log->info("Got signal %i. Exiting", sig);
    kill(-mypid, SIGTERM);
    loop.break_loop(ev::ALL);
}

void ServiceFactory::respawn_all()
{
    if (this->respawn_in_progress) {
        return;
    }
    this->shutdown_pending.clear();
    for (auto s: services)
    {
        if (!s.second->is_running()) {
            g_log->info("Service %s is not running, skiping", s.first.c_str());
            s.second->reset_dependencies();
            continue;
        }
        s.second->stop();
        g_log->info("Adding %s to shutdown_pending", s.first.c_str());
        this->shutdown_pending.insert(s.first);
    }
    this->respawn_in_progress = true;
}

void ServiceFactory::on_service_start(std::string name)
{
}

void ServiceFactory::on_service_stop(std::string name)
{
    if (!this->respawn_in_progress) {
        return;
    }
    g_log->info("Service %s stopped", name.c_str());
    this->shutdown_pending.erase(name);

    g_log->info("Services in shutdown_pending: %lu",
        this->shutdown_pending.size());

    if (this->shutdown_pending.size() == 0) {
        for (auto s: services)
        {
            s.second->start();
        }
        respawn_in_progress = false;
    }
}

void ServiceFactory::on_service_heartbeat(std::string name)
{
}
