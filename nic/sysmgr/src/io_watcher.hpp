#ifndef __IO_WATCHER_HPP__
#define __IO_WATCHER_HPP__

#include <memory>

#include <ev++.h>

class IOReactor
{
public:
    virtual void on_io(int fd) = 0;
};
typedef std::shared_ptr<IOReactor> IOReactorPtr;

class IOWatcher
{
private:
    ev::io       io;
    IOReactorPtr reactor;
    void io_callback(ev::io &watcher, int revents);
public:
    static std::shared_ptr<IOWatcher> create(int fd, IOReactorPtr reactor);
    IOWatcher(int fd, IOReactorPtr reactor);
    ~IOWatcher();
    void stop();
};
typedef std::shared_ptr<IOWatcher> IOWatcherPtr;

#endif // __IO_WATCHER_HPP__
