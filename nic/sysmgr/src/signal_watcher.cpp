#include "signal_watcher.hpp"

#include <memory>

#include <ev++.h>


std::shared_ptr<SignalWatcher> SignalWatcher::create(int signum,
    SignalReactorPtr reactor)
{
    return std::make_shared<SignalWatcher>(signum, reactor);
}

SignalWatcher::SignalWatcher(int signum, SignalReactorPtr reactor)
{
    this->reactor = reactor;
    signal.signum = signum;
    signal.set<SignalWatcher, &SignalWatcher::signal_callback>(this);
    signal.start();
}

void SignalWatcher::signal_callback()
{
    this->reactor->on_signal(signal.signum);
}

void SignalWatcher::stop()
{
    signal.stop();
}

SignalWatcher::~SignalWatcher()
{
    this->stop();
}
