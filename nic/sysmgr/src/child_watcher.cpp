#include "child_watcher.hpp"

#include <memory>

#include <ev++.h>

#include "log.hpp"
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
    g_log->debug("Child listenting %i", pid);
}

void ChildWatcher::child_callback()
{
    this->reactor->on_child(child.pid);
}

void ChildWatcher::stop()
{
    g_log->debug("Child stop listenting %i", this->pid);
    child.stop();
}

ChildWatcher::~ChildWatcher()
{
    g_log->debug("Child listener destructor %i", this->pid);
    this->stop();
}

int ChildWatcher::get_status()
{
    return this->child.rstatus;
}
