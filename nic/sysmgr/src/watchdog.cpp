#include "watchdog.hpp"

#include <memory>
#include <string>

#include "nic/sdk/platform/pal/include/pal.h"

#include "fault_watcher.hpp"
#include "log.hpp"
#include "timer_watcher.hpp"

class PALWatchdog : public AbstractWatchdog {
public:
    static std::shared_ptr<PALWatchdog> create();
    PALWatchdog();
    virtual void kick();
};
typedef std::shared_ptr<PALWatchdog> PALWatchdogPtr;

class SimulationWatchdog : public AbstractWatchdog,
                           public TimerReactor
{
private:
    TimerWatcherPtr timer_watcher;
public:
    static std::shared_ptr<SimulationWatchdog> create();
    virtual void kick();
    virtual void on_timer();
};
typedef std::shared_ptr<SimulationWatchdog> SimulationWatchdogPtr;

PALWatchdogPtr PALWatchdog::create()
{
    PALWatchdogPtr watchdog = std::make_shared<PALWatchdog>();
    return watchdog;
}

PALWatchdog::PALWatchdog()
{
    int rc = pal_watchdog_init(PANIC_WDT);
    g_log->info("pal_watchdog_init() rc = %i", rc);
}

void PALWatchdog::kick()
{
    pal_watchdog_kick();
}

SimulationWatchdogPtr SimulationWatchdog::create()
{
    SimulationWatchdogPtr watchdog = std::make_shared<SimulationWatchdog>();
    watchdog->timer_watcher = TimerWatcher::create(60.0, 60.0, watchdog);

    return watchdog;
}

void SimulationWatchdog::kick()
{
    this->timer_watcher->repeat();
}

void SimulationWatchdog::on_timer()
{
    ev::default_loop loop;

    g_log->err("Simulation Watchdog Expired");
    // Kill all children
    kill(-getpid(), SIGTERM);
    loop.break_loop(ev::ALL);
}

WatchdogPtr Watchdog::create()
{
    WatchdogPtr wchdg = std::make_shared<Watchdog>();

    wchdg->kick_it = true;
    
    wchdg->timer_watcher = TimerWatcher::create(1.0, 1.0, wchdg);
    FaultLoop::getInstance()->register_fault_reactor(wchdg);
    SwitchrootLoop::getInstance()->register_switchroot_reactor(wchdg);

    g_log->debug("Creating watchdog");

    // If file "/data/no_watchdog" is present or NO_WATCHDOG is set then use
    // the simulation watchdog
    if ((access("/data/no_watchdog", F_OK) != -1) ||
        (std::getenv("NO_WATCHDOG"))) {
        wchdg->watchdog = SimulationWatchdog::create();
        g_log->info("Using Simulation watchdog");
    } else {
        wchdg->watchdog = PALWatchdog::create();
        g_log->info("Using PAL watchdog");
    }
    
    wchdg->timer_watcher->repeat();
    
    return wchdg;
}

void Watchdog::on_fault(std::string reason)
{
    this->kick_it = false;
}

void Watchdog::on_timer()
{
    if (this->kick_it == false)
    {
        g_log->trace("Not kicking watchdog");
        return;
    }
    g_log->trace("Kicking watchdog");
    this->watchdog->kick();
}

void Watchdog::on_switchroot()
{
    g_log->debug("Watchdog on_switchroot()");
    this->stop();
}

void Watchdog::stop()
{
    g_log->debug("Watchdog stop()");
    this->timer_watcher->stop();
    pal_watchdog_stop();
}
