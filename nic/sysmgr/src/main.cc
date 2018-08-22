// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#include "pid_pipe.h"
#include "scheduler.h"

using namespace std;

#define MAX_EVENTS 10

static PidPipe died_pids;
static PidPipe started_pids;

void redirect(const string &filename, int fd)
{
    int file_fd = open(filename.c_str(), O_APPEND | O_CREAT);
    int rc;

    rc = close(fd);
    if (rc != 0)
    {
        close(file_fd);
        return;
    }
}

// Redirect stdout and stderr
void redirect_stds(const string &name, pid_t pid)
{
    close(0);

    redirect(name + "." + to_string(pid) + ".out" + ".log", 1);
    redirect(name + "." + to_string(pid) + ".err" + ".log", 2);
}

void exec_command(const string &command)
{
     // split the command to an array of string(tokens)
    istringstream iss(command);
    vector<string> tokens;
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter(tokens));

    // convert them to c_strings
    vector<const char *> c_strings;
    for (auto &&s : tokens)
    {
        c_strings.push_back(s.c_str());
    }
    c_strings.push_back(NULL);

    execvp(c_strings[0], (char *const *)&c_strings[0]);
}

pid_t launch(const string &name, const string &command)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        ERROR("Fork failed: {}", strerror(errno));
        exit(1);
    }
    else if (pid == 0)
    {
        redirect_stds(name, getpid());
        exec_command(command);
        exit(-1); // exec failed, exit
    }

    INFO("Fork success. Child pid: {}", pid);

    return pid;
}

void sigchld_handler(__attribute__((unused)) int sig)
{
    int saved_errno = errno;
    pid_t pid;
    while ((pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0)
    {
        died_pids.write_pid(pid);
    }
    errno = saved_errno;
}

void install_sigchld_handler()
{
    struct sigaction sa;
    sa.sa_handler = &sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1)
    {
        ERROR("sigaction(): {}", strerror(errno));
        exit(1);
    }
}

void wait_for_events(Scheduler &scheduler, int epollfd)
{
    struct epoll_event events[MAX_EVENTS];

    int fds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < fds; i++)
    {
        if (events[i].data.fd == died_pids.raw_fd())
        {
            pid_t pid;
            while ((pid = died_pids.read_pid()) > 0)
            {
                INFO("died_pids: read_pid(): {}", pid);
                scheduler.service_died(pid);
            }
        }
        if (events[i].data.fd == started_pids.raw_fd())
        {
            pid_t pid;
            while ((pid = started_pids.read_pid()) > 0)
            {
                INFO("started_pids: read_pid(): {}", pid);
                scheduler.service_started(pid);
            }
        }
    }
}

void loop(Scheduler &scheduler, int epollfd)
{
    for (;;)
    {
        auto action = scheduler.next_action();
        INFO("Next action: {}", action->type);
        if (action->type == LAUNCH)
        {
            INFO("Launching...");
            for (auto service: action->launch_list)
            {
                pid_t pid = launch(service->get_name(), service->get_command());
                scheduler.service_launched(service, pid);

                // temporary
                started_pids.write_pid(pid);
            }
        }
        else if (action->type == WAIT)
        {
            INFO("Waiting on pipes...");
            wait_for_events(scheduler, epollfd);
        }
        else if (action->type == REBOOT)
        {
            INFO("Rebooting...");
            break;
        }
    }
}

int main()
{
    Scheduler scheduler(SPECS);

    install_sigchld_handler();

    int epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        ERROR("epoll_create1: {}", strerror(errno));
        exit(1);
    }

    struct epoll_event ev = {0};
    fcntl(died_pids.raw_fd(), F_SETFL, fcntl(died_pids.raw_fd(), F_GETFL) | O_NONBLOCK);
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = died_pids.raw_fd();
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, died_pids.raw_fd(), &ev) == -1)
    {
        ERROR("epoll_ctl: {}", strerror(errno));
        exit(1);
    }

    fcntl(started_pids.raw_fd(), F_SETFL, fcntl(started_pids.raw_fd(), F_GETFL) | O_NONBLOCK);
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = started_pids.raw_fd();
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, started_pids.raw_fd(), &ev) == -1)
    {
        ERROR("epoll_ctl: {}", strerror(errno));
        exit(1);
    }

    loop(scheduler, epollfd);

    return 0;
}
