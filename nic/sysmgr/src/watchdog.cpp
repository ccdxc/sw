#include "watchdog.hpp"

#include <memory>
#include <string>

#include "nic/sdk/platform/pal/include/pal.h"

#include "fault_watcher.hpp"
#include "timer_watcher.hpp"

#include "nic/utils/penlog/lib/penlog.hpp"

extern penlog::LoggerPtr logger;

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
    logger->info("pal_watchdog_init() rc = {}", rc);
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
    logger->critical("Simulation Watchdog Expired");
    // Kill all children
    kill(-getpid(), SIGTERM);
    throw std::runtime_error("SimulationWatchdog expired");
}

WatchdogPtr Watchdog::create()
{
    WatchdogPtr wchdg = std::make_shared<Watchdog>();

    wchdg->kick_it = true;
    
    wchdg->timer_watcher = TimerWatcher::create(1.0, 1.0, wchdg);
    FaultLoop::getInstance()->register_fault_reactor(wchdg);
    SwitchrootLoop::getInstance()->register_switchroot_reactor(wchdg);

    logger->info("Creating watchdog");

    // If file "/data/no_watchdog" is present or NO_WATCHDOG is set then use
    // the simulation watchdog
    if ((access("/data/no_watchdog", F_OK) != -1) ||
        (std::getenv("NO_WATCHDOG"))) {
        wchdg->watchdog = SimulationWatchdog::create();
        logger->info("Using Simulation watchdog");
    } else {
        wchdg->watchdog = PALWatchdog::create();
        logger->info("Using PAL watchdog");
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
        logger->debug("Not kicking watchdog");
        return;
    }
    logger->debug("Kicking watchdog");
    this->watchdog->kick();
}

void Watchdog::on_switchroot()
{
    logger->debug("Watchdog on_switchroot()");
    this->stop();
}

void Watchdog::stop()
{
    logger->debug("Watchdgo stop()");
    this->timer_watcher->stop();
    pal_watchdog_stop();
}
