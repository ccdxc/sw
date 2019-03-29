#ifndef __TIMER_WATCHER_HPP__
#define __TIMER_WATCHER_HPP__

#include <memory>

#include <ev++.h>

class TimerReactor
{
public:
    virtual void on_timer() = 0;
};
typedef std::shared_ptr<TimerReactor> TimerReactorPtr;

class TimerWatcher
{
private:
    ev::timer       timer;
    TimerReactorPtr reactor;
    void timer_callback();
public:
    static std::shared_ptr<TimerWatcher> create(ev_tstamp after,
	ev_tstamp repeat, TimerReactorPtr reactor);
    TimerWatcher(ev_tstamp after, ev_tstamp repeat, TimerReactorPtr reactor);
    ~TimerWatcher();
    void stop();
    void repeat();
};
typedef std::shared_ptr<TimerWatcher> TimerWatcherPtr;

#endif // __TIMER_WATCHER_HPP__
