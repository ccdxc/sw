#ifndef __SIGNAL_WATCHER_HPP__
#define __SIGNAL_WATCHER_HPP__

#include <memory>

#include <ev++.h>

class SignalReactor
{
public:
    virtual void on_signal(int sig) = 0;
};
typedef std::shared_ptr<SignalReactor> SignalReactorPtr;

class SignalWatcher
{
private:
    ev::sig          signal;
    SignalReactorPtr reactor;
    void signal_callback();
public:
    static std::shared_ptr<SignalWatcher> create(int signum,
	SignalReactorPtr reactor);
    SignalWatcher(int signum, SignalReactorPtr reactor);
    ~SignalWatcher();
    void stop();
};
typedef std::shared_ptr<SignalWatcher> SignalWatcherPtr;

#endif // __SIGNAL_WATCHER_HPP__
