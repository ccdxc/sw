#include "switchroot_watcher.hpp"

#include <list>
#include <memory>

#include "custom_loop.hpp"
#include "service_spec.hpp"

SwitchrootLoopPtr SwitchrootLoop::instance = nullptr;

std::shared_ptr<SwitchrootLoop> SwitchrootLoop::getInstance()
{
    if (instance == nullptr)
    {
        instance = std::make_shared<SwitchrootLoop>();
        custom_loop_reactor_register(instance);
    }
    
    return instance;
}

void SwitchrootLoop::do_work()
{
    for (auto reactor: this->switchroot_reactors)
    {
        reactor->on_switchroot();
    }
    this->switchroot_pending = false;
}

bool SwitchrootLoop::has_work()
{
    return this->switchroot_pending;
}

void SwitchrootLoop::set_switchroot()
{
   this->switchroot_pending = true;
}

void SwitchrootLoop::register_switchroot_reactor(SwitchrootReactorPtr reactor)
{
    this->switchroot_reactors.push_back(reactor);
}
