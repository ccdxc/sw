#ifndef __PIPEDIO_HPP__
#define __PIPEDIO_HPP__

#include <memory>
#include <string>

#include <stdio.h>

#include "io_watcher.hpp"

class PipedIO: public std::enable_shared_from_this<PipedIO>,
               public IOReactor
{
private:
    int          in_fd;
    int          out_fd;
    int          size;
    std::string  filename;
    IOWatcherPtr watcher;
    void         rotate();
    void         stop();
public:
    static std::shared_ptr<PipedIO> create(int fd, std::string filename);
    ~PipedIO();
    virtual void on_io(int fd);
};
typedef std::shared_ptr<PipedIO> PipedIOPtr;

#endif
