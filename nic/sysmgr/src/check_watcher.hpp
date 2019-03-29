#ifndef __CHECK_HPP__
#define __CHECK_HPP__

#include <memory>

#include <ev++.h>

class CheckReactor
{
public:
    virtual void on_check() = 0;
};
typedef std::shared_ptr<CheckReactor> CheckReactorPtr;

class CheckWatcher
{
private:
    ev::check       check;
    CheckReactorPtr reactor;
    void check_callback(ev::check &watcher, int revents);
public:
    static std::shared_ptr<CheckWatcher> create(CheckReactorPtr reactor);
    CheckWatcher(CheckReactorPtr reactor);
    ~CheckWatcher();
    void stop();
};
typedef std::shared_ptr<CheckWatcher> CheckWatcherPtr;

#endif // __CHECK_HPP__
