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

    void pipe_write(T value)
    {
        ssize_t n = write(this->fds[PIPE_T_WRITE], &value, sizeof(value));
        if (n != sizeof(value))
        {
            exit(-1);
        }
    }

    T pipe_read()
    {
        T value = -1;

        int n = read(this->fds[PIPE_T_READ], &value, sizeof(value));
        if (n < (ssize_t)sizeof(value))
        {
            return -1;
        }

        return value;
    }

  private:
    int fds[2];
};

#endif
