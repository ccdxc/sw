#ifndef __PID_PIPE_H__
#define __PID_PIPE_H__

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "logger.h"

#define PID_PIPE_READ 0
#define PID_PIPE_WRITE 1

class PidPipe
{
  public:
    PidPipe()
    {
        int rc = pipe2(this->fds, O_CLOEXEC | O_NONBLOCK);
        if (rc < 0)
        {
            exit(-1);
        }
    }

    int raw_fd()
    {
        return this->fds[PID_PIPE_READ];
    }

    void write_pid(pid_t pid)
    {
        ssize_t n = write(this->fds[PID_PIPE_WRITE], &pid, sizeof(pid));
        if (n != sizeof(pid))
        {
            exit(-1);
        }
    }

    pid_t read_pid()
    {
        pid_t pid = -1;

        int n = read(this->fds[PID_PIPE_READ], &pid, sizeof(pid));
        if (n < (ssize_t)sizeof(pid))
        {
            return -1;
        }

        return pid;
    }

  private:
    int fds[2];
};

#endif
