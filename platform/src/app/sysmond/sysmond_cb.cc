//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include "sysmond_cb.hpp"
#include "platform/src/app/sysmond/logger.h"
#include "platform/src/app/sysmond/delphi/sysmond_delphi_cb.hpp"
#include "platform/src/app/sysmond/event_recorder/sysmond_eventrecorder_cb.hpp"

void
event_cb_init (void)
{
    delphi_event_cb_init();
}

void
frequency_change_event_cb (uint32_t frequency)
{
    delphi_frequency_change_event_cb(frequency);
}

void
cattrip_event_cb (void)
{
    delphi_cattrip_event_cb();
    eventrecorder_cattrip_event_cb();
}

void
power_event_cb (sdk::platform::sensor::system_power_t *power)
{
    delphi_power_event_cb(power);
}

void
temp_event_cb (sdk::platform::sensor::system_temperature_t *temperature,
               sysmond_hbm_threshold_event_t hbm_event)
{
    delphi_temp_event_cb(temperature);
    TRACE_INFO(GetObflLogger(), "temp_event_cb::hbm event is {}.", hbm_event);
    eventrecorder_temp_event_cb(temperature, hbm_event);
}

void
memory_event_cb (uint64_t total_mem, uint64_t available_mem,
                 uint64_t free_mem)
{
    delphi_memory_event_cb(total_mem, available_mem, free_mem);
}

void
intr_event_cb (const intr_reg_t *reg, const intr_field_t *field)
{
    switch (field->severity) {
    case INTR_SEV_TYPE_HW_RMA:
    case INTR_SEV_TYPE_FATAL:
    case INTR_SEV_TYPE_ERR:
        TRACE_INFO(GetObflLogger(), "name: {}_{}, count: {}, severity: {}, desc: {}",
                   reg->name, field->name, field->count,
                   get_severity_str(field->severity).c_str(), field->desc);
        break;
    case INTR_SEV_TYPE_INFO:
    default:
        break;
    }
}
