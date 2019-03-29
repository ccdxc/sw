#include "io_watcher.hpp"

#include <memory>

#include <ev++.h>
#include <fcntl.h>

static void
set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

std::shared_ptr<IOWatcher> IOWatcher::create(int fd, IOReactorPtr reactor)
{
    return std::make_shared<IOWatcher>(fd, reactor);
}

IOWatcher::IOWatcher(int fd, IOReactorPtr reactor)
{
    this->reactor = reactor;
    set_non_blocking(fd);
    io.set<IOWatcher, &IOWatcher::io_callback>(this);
    io.start(fd, ev::READ);
}

void IOWatcher::io_callback(ev::io &watcher, int revents)
{
    this->reactor->on_io(watcher.fd);
}

void IOWatcher::stop()
{
    io.stop();
}

IOWatcher::~IOWatcher()
{
    this->stop();
}
