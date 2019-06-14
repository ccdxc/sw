#include "child_watcher.hpp"

#include <memory>

#include <ev++.h>

#include "utils.hpp"

std::shared_ptr<ChildWatcher> ChildWatcher::create(pid_t pid,
    ChildReactorPtr reactor)
{
    return std::make_shared<ChildWatcher>(pid, reactor);
}

ChildWatcher::ChildWatcher(pid_t pid, ChildReactorPtr reactor)
{
    this->reactor = reactor;
    this->pid = pid;
    child.set<ChildWatcher, &ChildWatcher::child_callback>(this);
    child.start(pid, ev::READ);
    logger->info("Child listenting {}", pid);
}

void ChildWatcher::child_callback()
{
    this->reactor->on_child(child.pid);
}

void ChildWatcher::stop()
{
    logger->info("Child stop listenting {}", this->pid);
    child.stop();
}

ChildWatcher::~ChildWatcher()
{
    logger->info("Child listener destructor {}", this->pid);
    this->stop();
}

int ChildWatcher::get_status()
{
    return this->child.rstatus;
}
