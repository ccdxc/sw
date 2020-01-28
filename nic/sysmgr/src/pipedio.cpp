#include "pipedio.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "io_watcher.hpp"
#include "log.hpp"
#include "utils.hpp"

#define MAX_LOG_SIZE (128 * 1024)

PipedIOPtr PipedIO::create(int fd, std::string filename, bool cap)
{
    PipedIOPtr io = std::make_shared<PipedIO>();
    io->cap = cap;
    io->capped = false;
    io->in_fd = fd;
    io->filename = filename;
    io->out_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_CLOEXEC,
        S_IRUSR | S_IWUSR);
    assert(io->out_fd != -1);
    io->watcher = IOWatcher::create(fd, io);
    io->size = 0;

    return io;
}

PipedIO::~PipedIO()
{
    this->stop();
}

void PipedIO::stop()
{
    g_log->debug("Closing %s", this->filename.c_str());
    if (this->watcher != nullptr)
    {
        this->watcher->stop();
        this->watcher = nullptr;
    }
    if (this->out_fd != -1)
    {
        close(this->out_fd);
        this->out_fd = -1;
    }
    if (this->in_fd != -1)
    {
        this->in_fd = -1;
        close(this->in_fd);
    }
}

void PipedIO::rotate()
{
    if (this->size >= MAX_LOG_SIZE)
    {
        if (this->cap) {
            this->capped = true;
            return;
        }
        std::string old = this->filename + ".1";
        g_log->debug("Rotating %s", this->filename.c_str());
        // remove the old "old" if it exist
        close(this->out_fd);
        unlink(old.c_str());
        rename(this->filename.c_str(), old.c_str());
        this->out_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_CLOEXEC,
            S_IRUSR | S_IWUSR);
        assert(this->out_fd != -1);
        this->size = 0;
    }
}

void PipedIO::on_io(int fd)
{
    char buf[1024];
    int  n;
    int  wn;

    while (true)
    {
        n = read(fd, buf, sizeof(buf));
        g_log->debug("read returned %i for %s", n, this->filename.c_str());
        if (n == 0) /* socket closed */
        {
            g_log->debug("EOF for %s", this->filename.c_str());
            this->stop();
            return;
        }
        if (n == -1) /* error */
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                /* we are non-blocking so this is normal */
            {
                return;
            }
            g_log->err("Errno: %s", strerror(errno));
            this->stop();
            return;
        }

        // We reached the maximum log size for this. Stop writing anymore.
        if (this->capped) {
            continue;
        }
        
        this->size += n;
        wn = write(this->out_fd, buf, n);
        if (wn != n) {
            g_log->err("Read/Write mismatch %i/%i", n, wn);
        }
        rotate();
    }
}
