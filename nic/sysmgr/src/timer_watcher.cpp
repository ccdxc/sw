#include "timer_watcher.hpp"

#include <memory>

#include <ev++.h>


std::shared_ptr<TimerWatcher> TimerWatcher::create(ev_tstamp after,
    ev_tstamp repeat, TimerReactorPtr reactor)
{
    return std::make_shared<TimerWatcher>(after, repeat, reactor);
}

TimerWatcher::TimerWatcher(ev_tstamp after, ev_tstamp repeat,
    TimerReactorPtr reactor)
{
    this->reactor = reactor;
    timer.set<TimerWatcher, &TimerWatcher::timer_callback>(this);
    timer.start(after, repeat);
}

void TimerWatcher::timer_callback()
{
    this->reactor->on_timer();
}

void TimerWatcher::stop()
{
    timer.stop();
}

TimerWatcher::~TimerWatcher()
{
    this->stop();
}


void TimerWatcher::repeat()
{
    timer.again();
}
