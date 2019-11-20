//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------
#include "sysmond_eventrecorder.hpp"
#include "sysmond_eventrecorder_cb.hpp"

void
eventrecorder_cattrip_event_cb (void)
{
    EventLogger::getInstance()->LogCriticalTempEvent(
                                    "Cattrip occurred, resetting the system");
}

void
eventrecorder_temp_event_cb (
                    sdk::platform::sensor::system_temperature_t *temperature,
                    sysmond_hbm_threshold_event_t hbm_event)
{
    if (hbm_event == SYSMOND_HBM_TEMP_ABOVE_THRESHOLD) {
        EventLogger::getInstance()->LogOverTempAlarmEvent(
                "System temperature is above threshold - %dC",
                temperature->hbmtemp);
    } else if (hbm_event == SYSMOND_HBM_TEMP_BELOW_THRESHOLD) {
        EventLogger::getInstance()->LogOverTempExitAlarmEvent(
                "System temperature is normal - %dC", temperature->hbmtemp);
    }
    return;
}

void
eventrecorder_fatal_interrupt_event_cb (const char *desc)
{
    EventLogger::getInstance()->LogFatalInterruptEvent(desc);
}

void
eventrecorder_panic_event_cb (void)
{
    EventLogger::getInstance()->LogPanicEvent("Panic occurred on the previous boot");
}
