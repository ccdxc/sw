#include "gtest/gtest.h"

#include <set>

#include <sys/types.h>

#include "service.h"
#include "scheduler.h"
#include "spec.h"

static pid_t launch_srv(Scheduler &sched, shared_ptr<Service> srv)
{
    static pid_t pid = 0;

    pid++;
    sched.service_launched(srv, pid);

    return pid;
}
static void start_srv(Scheduler &sched, pid_t pid)
{
    sched.service_started(pid);
}

static void start_ready(Scheduler &sched)
{
    auto action = sched.next_action();
    list<pid_t> pids;

    for (auto s : action->launch_list)
    {
        pid_t pid = launch_srv(sched, s);
        pids.push_back(pid);
    }

    action = sched.next_action();
    ASSERT_EQ(action->launch_list.size(), 0);

    for (auto pid : pids)
    {
        start_srv(sched, pid);
    }
}

static void next_action_is(Scheduler &sched, action_type_t action_type, set<string> should)
{
    auto act = sched.next_action();
    ASSERT_EQ(act->type, action_type);
    ASSERT_EQ(act->launch_list.size(), should.size());
    for (auto r : act->launch_list)
    {
        ASSERT_EQ(should.count(r->get_name()), 1);
    }
}

TEST(Scheduler, ServiceCompare)
{
    auto s = ServiceSet();
    auto srv = make_shared<Service>("nicmgr", "/bin/ls -l");
    auto srv2 = srv;
    s.insert(srv);
    s.insert(srv2);
    ASSERT_EQ(s.size(), 1);
}

TEST(Scheduler, BasicFunctionality)
{
    const vector<Spec> specs = {
        Spec("delphi", NON_RESTARTABLE, "/bin/ls -l", {}),
        Spec("agent1", NON_RESTARTABLE, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
        Spec("hal", NON_RESTARTABLE, "/bin/ls -l", {"delphi"}),
        Spec("nicmgr", NON_RESTARTABLE, "/bin/ls -l", {"delphi", "hal"}),
        Spec("agent2", NON_RESTARTABLE, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
    };

    auto sched = Scheduler(specs);

    next_action_is(sched, LAUNCH, {"delphi"});
    start_ready(sched);

    next_action_is(sched, LAUNCH, {"hal"});
    start_ready(sched);

    next_action_is(sched, LAUNCH, {"nicmgr"});
    start_ready(sched);

    next_action_is(sched, LAUNCH, {"agent1", "agent2"});
    start_ready(sched);

    next_action_is(sched, WAIT, {});
}

TEST(Scheduler, CircularDependencies)
{
    const vector<Spec> specs = {
        Spec("hal", NON_RESTARTABLE, "/bin/ls -l", {}),
        Spec("nicmgr", NON_RESTARTABLE, "/bin/ls -l", {"hal", "agent"}),
        Spec("agent", NON_RESTARTABLE, "/bin/ls -l", {"nicmgr"}),
    };

    auto sched = Scheduler(specs);
    start_ready(sched);

    auto action = sched.next_action();
    ASSERT_EQ(action->type, REBOOT);
}

TEST(Specs, NoDuplicateNames)
{
    set<string> service_names;
    for (auto &sp : SPECS)
    {
        ASSERT_EQ(service_names.count(sp.name), 0);
        service_names.insert(sp.name);
    }
}

TEST(Specs, DependenciesExist)
{
    set<string> service_names;
    for (auto &sp : SPECS)
    {
        service_names.insert(sp.name);
    }

    for (auto &sp : SPECS)
    {
        for (auto dep : sp.dependencies)
        {
            ASSERT_EQ(service_names.count(dep), 1);
        }
    }
}

TEST(Specs, NoCircularDependencies)
{
    auto sched = Scheduler(SPECS);

    for (;;)
    {
        auto action = sched.next_action();
        ASSERT_NE(action->type, REBOOT);
        if (action->type == WAIT)
        {
            break;
        }
        start_ready(sched);
    }
}
