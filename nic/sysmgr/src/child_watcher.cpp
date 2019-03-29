#include "child_watcher.hpp"

#include <memory>

#include <ev++.h>


std::shared_ptr<ChildWatcher> ChildWatcher::create(pid_t pid,
    ChildReactorPtr reactor)
{
    return std::make_shared<ChildWatcher>(pid, reactor);
}

ChildWatcher::ChildWatcher(pid_t pid, ChildReactorPtr reactor)
{
    this->reactor = reactor;
    child.set<ChildWatcher, &ChildWatcher::child_callback>(this);
    child.start(pid, ev::READ);
}

void ChildWatcher::child_callback()
{
    this->reactor->on_child(child.pid);
}

void ChildWatcher::stop()
{
    child.stop();
}

ChildWatcher::~ChildWatcher()
{
    this->stop();
}

int ChildWatcher::get_status()
{
    return this->child.rstatus;
}
