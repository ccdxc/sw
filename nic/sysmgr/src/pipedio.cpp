#include "pipedio.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "io_watcher.hpp"
#include "utils.hpp"

#define MAX_LOG_SIZE (128 * 1024)

PipedIOPtr PipedIO::create(int fd, std::string filename)
{
    PipedIOPtr io = std::make_shared<PipedIO>();
    io->in_fd = fd;
    io->filename = filename;
    io->out_fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    io->watcher = IOWatcher::create(fd, io);
    io->size = 0;

    return io;
}

PipedIO::~PipedIO()
{
    this->watcher->stop();
    close(this->out_fd);
    close(this->in_fd);
}

void PipedIO::rotate()
{
    if (this->size >= MAX_LOG_SIZE)
    {
        std::string old = this->filename + ".1";
        // remove the old "old" if it exist
        unlink(old.c_str());
        close(this->out_fd);
        rename(this->filename.c_str(), old.c_str());
        this->out_fd = open(filename.c_str(), O_RDWR | O_CREAT,
            S_IRUSR | S_IWUSR);
        this->size = 0;
    }
}

void PipedIO::on_io(int fd)
{
    char buf[1024];
    int  n;

    while (true)
    {
        n = read(fd, buf, sizeof(buf));
        if (n <= 0)
        {
            return;
        }
        this->size += n;
        write(this->out_fd, buf, n);
        rotate();
    }
}
