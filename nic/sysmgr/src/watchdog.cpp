#include "watchdog.hpp"

#include <memory>
#include <string>

#include "nic/sdk/platform/pal/include/pal.h"

#include "fault_watcher.hpp"
#include "timer_watcher.hpp"

#include "nic/utils/penlog/lib/penlog.hpp"

extern penlog::LoggerPtr logger;

WatchdogPtr Watchdog::create()
{
    WatchdogPtr wchdg = std::make_shared<Watchdog>();

    wchdg->kick_it = true;
    
    wchdg->timer_watcher = TimerWatcher::create(1.0, 1.0, wchdg);
    FaultLoop::getInstance()->register_fault_reactor(wchdg);
    SwitchrootLoop::getInstance()->register_switchroot_reactor(wchdg);

    int rc = pal_watchdog_init(PANIC_WDT);
    logger->info("pal_watchdog_init() rc = {}", rc);
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
    pal_watchdog_kick();
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
