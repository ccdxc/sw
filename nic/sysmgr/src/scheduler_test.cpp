#include "gtest/gtest.h"

#include <set>

#include <sys/types.h>
#include <thread>

#include "scheduler.hpp"
#include "service.hpp"
#include "spec.hpp"

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

static void launch_ready(Scheduler &sched)
{
    auto action = sched.next_action();

    for (auto s : action->launch_list)
    {
        launch_srv(sched, s);
    }
}

static void next_action_is(Scheduler &sched, action_type_t action_type, set<string> should)
{
    auto act = sched.next_action();
    ASSERT_EQ(act->type, action_type);
    ASSERT_EQ(act->launch_list.size(), should.size());
    for (auto r : act->launch_list)
    {
        // make sure r is in the "should" set
        ASSERT_EQ(should.count(r->name), 1);
    }
}

static void start_all(Scheduler &sched)
{
    const int max_cycles = 99;
    for (int i = 0; i < max_cycles; i++)
    {
        auto act = sched.next_action();
        if (act->type == WAIT) {
            return;
        }
        start_ready(sched);
    }
    FAIL() << "max_cycles exceeded";
}

TEST(Scheduler, ServiceCompare)
{
    auto s = ServiceSet();
    auto srv = make_shared<Service>("nicmgr", "/bin/ls -l", true, false);
    auto srv2 = srv;
    s.insert(srv);
    s.insert(srv2);
    ASSERT_EQ(s.size(), 1);
}

TEST(Scheduler, BasicFunctionality)
{
    const vector<Spec> specs = {
        Spec("delphi", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {}),
        Spec("agent1", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
        Spec("hal", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi"}),
        Spec("nicmgr", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal"}),
        Spec("agent2", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
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
        Spec("hal", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {}),
        Spec("nicmgr", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"hal", "agent"}),
        Spec("agent", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"nicmgr"}),
    };

    auto sched = Scheduler(specs);
    start_ready(sched);

    auto action = sched.next_action();
    ASSERT_EQ(action->type, REBOOT);
}

TEST(Scheduler, RestartableProcessDeath)
{
    const vector<Spec> specs = {
        Spec("delphi", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {}),
        Spec("agent1", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
        Spec("hal", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi"}),
        Spec("nicmgr", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal"}),
        Spec("agent2", RESTARTABLE, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
    };

    auto sched = Scheduler(specs);
    start_all(sched);

    // Test 1: Process dies while its in "started" state
    sched.service_died("agent2");
    auto action = sched.next_action();
    next_action_is(sched, LAUNCH, {"agent2"});

    launch_ready(sched);
    next_action_is(sched, WAIT, {});

    // Test 2: Process dies while its in "launched" state
    sched.service_died("agent2");
    action = sched.next_action();
    next_action_is(sched, LAUNCH, {"agent2"});
}

TEST(Scheduler, NonRestartableStartedProcessDeath)
{
    const vector<Spec> specs = {
        Spec("delphi", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {}),
        Spec("agent1", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
        Spec("hal", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi"}),
        Spec("nicmgr", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal"}),
        Spec("agent2", RESTARTABLE, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
    };

    auto sched = Scheduler(specs);
    start_all(sched);

    sched.service_died("agent1");
    auto action = sched.next_action();
    next_action_is(sched, REBOOT, {});
}

TEST(Scheduler, NonRestartableStartingProcessDeath)
{
    const vector<Spec> specs = {
        Spec("delphi", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {}),
        Spec("agent1", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
        Spec("hal", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi"}),
        Spec("nicmgr", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {"delphi", "hal"}),
        Spec("agent2", RESTARTABLE, "/bin/ls -l", {"delphi", "hal", "nicmgr"}),
    };

    auto sched = Scheduler(specs);
    // starts delphi
    start_ready(sched);
    // starts hal
    start_ready(sched);
    // starts nicmgr
    start_ready(sched);
    // launches agent1 and agent 2
    launch_ready(sched);

    sched.service_died("agent1");
    auto action = sched.next_action();
    next_action_is(sched, REBOOT, {});
}

TEST(Scheduler, WatchdogTest)
{
   const vector<Spec> specs = {
      Spec("delphi", DEFAULT_SPEC_FLAGS, "/bin/ls -l", {}),
   };

   auto sched = Scheduler(specs);
   start_ready(sched);
   sched.heartbeat("delphi");
   std::this_thread::sleep_for(std::chrono::seconds(20));
   next_action_is(sched, REBOOT, {});   
}

TEST(Scheduler, NoWatchdogTest)
{
   const vector<Spec> specs = {
      Spec("delphi", NO_WATCHDOG, "/bin/ls -l", {}),
   };

   auto sched = Scheduler(specs);
   start_ready(sched);
   sched.heartbeat("delphi");
   std::this_thread::sleep_for(std::chrono::seconds(20));
   next_action_is(sched, WAIT, {});   
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
