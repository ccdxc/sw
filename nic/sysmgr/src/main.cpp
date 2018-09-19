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
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

#include "delphi_messages.hpp"
#include "logger.hpp"
#include "pipe_t.hpp"
#include "scheduler.hpp"
#include "sysmgr_service.hpp"

using namespace std;

#define MAX_EVENTS 10

static auto died_pids = make_shared<Pipe<pid_t>>();
static auto started_pids = make_shared<Pipe<pid_t>>();
static auto delphi_messages = make_shared<Pipe<int32_t>>();
static auto heartbeats = make_shared<Pipe<pid_t>>();

void redirect(const string &filename, int fd)
{
    int file_fd = open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    close(fd);

    dup2(file_fd, fd);

    close(file_fd);
}

// Redirect stdout and stderr
void redirect_stds(const string &name, pid_t pid)
{
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
    fprintf(stderr, "exec(%s) failed: %s", c_strings[0], strerror(errno));
    exit(-1); // the only way to get here is if exec failed, exit
}

pid_t launch(const string &name, const string &command)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        ERR("Fork failed: {}", strerror(errno));
        exit(1);
    }
    else if (pid == 0)
    {
        redirect_stds(name, getpid());
        exec_command(command);
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
        died_pids->pipe_write(pid);
    }
    errno = saved_errno;
}

void sigquit_handler(__attribute__((unused)) int sig)
{
}

void install_sigchld_handler()
{
    struct sigaction sa;
    sa.sa_handler = &sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1)
    {
        ERR("sigaction(): {}", strerror(errno));
        exit(1);
    }
}

void wait_for_events(Scheduler &scheduler, int epollfd, int sleep)
{
    struct epoll_event events[MAX_EVENTS];

    int fds = epoll_wait(epollfd, events, MAX_EVENTS, sleep * 1000);
    if (fds == -1) {
        ERR("epoll_wait() error: {}", strerror(errno));
    }
    INFO("Number of events: {}", fds);
    for (int i = 0; i < fds; i++)
    {
        if (events[i].data.fd == died_pids->raw_fd())
        {
            INFO("died_pids event");
            pid_t pid;
            while ((pid = died_pids->pipe_read()) > 0)
            {
                INFO("died_pids: pipe_read(): {}", pid);
                scheduler.service_died(pid);
            }
        }
        else if (events[i].data.fd == started_pids->raw_fd())
        {
            INFO("started_pids event");
            pid_t pid;
            while ((pid = started_pids->pipe_read()) > 0)
            {
                INFO("started_pids: pipe_read(): {}", pid);
                scheduler.service_started(pid);
            }
        }
        else if (events[i].data.fd == delphi_messages->raw_fd())
        {
            INFO("delphi_messages event");
            int32_t msg;
            while ((msg = delphi_messages->pipe_read()) > 0)
            {
                INFO("delphi_messages: pipe_read(): {}", msg);
                if (msg == DELPHI_UP)
                {
                    scheduler.service_started("delphi");
                }
                else
                {
                    INFO("UKNOWN delphi_messages message");
                }
            }
        }
        else if (events[i].data.fd == heartbeats->raw_fd())
        {
            INFO("heartbeat event");
            pid_t pid;
            while((pid = heartbeats->pipe_read()) > 0)
            {
                INFO("heartbeat: pipe_read(): {}", pid);
                scheduler.heartbeat(pid);
            }
        }
        else
        {
            INFO("UNKOWN EVENT");
        }
    }
}

void epollfd_register(int epollfd, int fd)
{
    struct epoll_event ev = {0};
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        ERR("epoll_ctl: {}", strerror(errno));
        exit(1);
    }
}

void loop(Scheduler &scheduler)
{
    int epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        ERR("epoll_create1: {}", strerror(errno));
        exit(1);
    }

    epollfd_register(epollfd, died_pids->raw_fd());
    epollfd_register(epollfd, started_pids->raw_fd());
    epollfd_register(epollfd, delphi_messages->raw_fd());
    epollfd_register(epollfd, heartbeats->raw_fd());

    for (;;)
    {
        auto action = scheduler.next_action();
        INFO("Next action: {}", action->type);
        if (action->type == LAUNCH)
        {
            INFO("Launching services");
            for (auto service : action->launch_list)
            {
                pid_t pid = launch(service->name, service->command);
                scheduler.service_launched(service, pid);
            }
        }
        else if (action->type == WAIT)
        {
            INFO("Waiting for events");
            wait_for_events(scheduler, epollfd, action->sleep);
        }
        else if (action->type == REBOOT)
        {
            INFO("Rebooting");
            // kills all children "-pid" means all process in this process group
            kill(-getpid(), SIGQUIT);
            exit(0);
        }
    }
}


void *delphi_thread_run(void *ctx)
{
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());

    shared_ptr<SysmgrService> svc = make_shared<SysmgrService>(sdk, "SystemManager", started_pids, 
        delphi_messages, heartbeats);
    sdk->RegisterService(svc);

    INFO("Starting Delphi MainLoop()");
    sdk->MainLoop();
    INFO("Delphi main loop exited");

    return NULL;
}

int main()
{
    int rc = setpgid(0, 0);
    if (rc == -1) 
    {
        perror("setpgid");
        exit(-1);
    }
    redirect_stds("sysmgr", getpid());

    Scheduler scheduler(SPECS);

    install_sigchld_handler();
    signal(SIGQUIT, sigquit_handler);

    pthread_t delphi_thread;
    pthread_create(&delphi_thread, NULL, delphi_thread_run, NULL);

    loop(scheduler);

    return 0;
}
