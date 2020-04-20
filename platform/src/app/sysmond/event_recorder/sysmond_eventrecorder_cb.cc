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
                    sysmon_hbm_threshold_event_t hbm_event)
{
    if (hbm_event == SYSMON_HBM_TEMP_ABOVE_THRESHOLD) {
        EventLogger::getInstance()->LogOverTempAlarmEvent(
                "System temperature is above threshold - %dC",
                temperature->hbmtemp);
    } else if (hbm_event == SYSMON_HBM_TEMP_BELOW_THRESHOLD) {
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

void
eventrecorder_postdiag_event_cb (void)
{
    EventLogger::getInstance()->LogPostdiagEvent("Post diag test failed on this boot");
}

void
eventrecorder_pciehealth_event_cb(sysmon_pciehealth_severity_t sev, const char *reason)
{
    if (sev == SYSMON_PCIEHEALTH_INFO) {
        EventLogger::getInstance()->LogInfoPcieHealthEvent(reason);
    } else if (sev == SYSMON_PCIEHEALTH_WARN){
        EventLogger::getInstance()->LogWarnPcieHealthEvent(reason);
    } else if (sev == SYSMON_PCIEHEALTH_ERROR){
        EventLogger::getInstance()->LogErrorPcieHealthEvent(reason);
    }
}
