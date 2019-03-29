#include "check_watcher.hpp"

#include <memory>

#include <ev++.h>

std::shared_ptr<CheckWatcher> CheckWatcher::create(CheckReactorPtr reactor)
{
    return std::make_shared<CheckWatcher>(reactor);
}

CheckWatcher::CheckWatcher(CheckReactorPtr reactor)
{
    this->reactor = reactor;
    check.set<CheckWatcher, &CheckWatcher::check_callback>(this);
    check.start();
}

void CheckWatcher::check_callback(ev::check &watcher, int revents)
{
    this->reactor->on_check();
}

void CheckWatcher::stop()
{
    check.stop();
}

CheckWatcher::~CheckWatcher()
{
    this->stop();
}


