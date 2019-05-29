#include "fault_watcher.hpp"

#include <list>
#include <memory>

#include "custom_loop.hpp"
#include "service_spec.hpp"

FaultLoopPtr FaultLoop::instance = nullptr;

std::shared_ptr<FaultLoop> FaultLoop::getInstance()
{
    if (instance == nullptr)
    {
        instance = std::make_shared<FaultLoop>();
        custom_loop_reactor_register(instance);
    }
    
    return instance;
}

void FaultLoop::do_work()
{
    for (auto reactor: this->fault_reactors)
    {
        reactor->on_fault(this->fault_reason);
    }
    this->fault_pending = false;
}

bool FaultLoop::has_work()
{
    return this->fault_pending;
}

void FaultLoop::set_fault(std::string reason)
{
    if (this->fault_set) {
        /* We are already in fault mode, ignore further notifications */
        return;
    }
    this->fault_reason = reason;
    this->fault_set = true;
    this->fault_pending = true;
}

void FaultLoop::register_fault_reactor(FaultReactorPtr reactor)
{
    this->fault_reactors.push_back(reactor);
}
