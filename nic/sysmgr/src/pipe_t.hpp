#ifndef __PIPE_H__
#define __PIPE_H__

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define PIPE_T_READ 0
#define PIPE_T_WRITE 1

template <class T>
class Pipe
{
  public:
    Pipe()
    {
        int rc = pipe2(this->fds, O_CLOEXEC | O_NONBLOCK);
        if (rc < 0)
        {
            exit(-1);
        }
    }

    int raw_fd()
    {
        return this->fds[PIPE_T_READ];
    }

    void pipe_write(T *value)
    {
        ssize_t n = write(this->fds[PIPE_T_WRITE], value, sizeof(T));
        if (n != sizeof(T))
        {
            exit(-1);
        }
    }

    int pipe_read(T *value)
    {
        ssize_t n = read(this->fds[PIPE_T_READ], value, sizeof(T));
        if (n < (ssize_t)sizeof(T))
        {
            return -1;
        }

        return 0;
    }

  private:
    int fds[2];
};

#endif
