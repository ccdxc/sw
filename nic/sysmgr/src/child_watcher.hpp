#ifndef __CHILD_WATCHER_HPP__
#define __CHILD_WATCHER_HPP__

#include <memory>

#include <ev++.h>

class ChildReactor
{
public:
    virtual void on_child(pid_t fd) = 0;
};
typedef std::shared_ptr<ChildReactor> ChildReactorPtr;

class ChildWatcher
{
private:
    ev::child       child;
    ChildReactorPtr reactor;
    void child_callback();
public:
    static std::shared_ptr<ChildWatcher> create(pid_t pid,
	ChildReactorPtr reactor);
    ChildWatcher(pid_t pid, ChildReactorPtr reactor);
    ~ChildWatcher();
    void stop();
    int get_status();
};
typedef std::shared_ptr<ChildWatcher> ChildWatcherPtr;

#endif // __CHILD_WATCHER_HPP__
