//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include "sysmond_cb.hpp"
#include "platform/src/app/sysmond/logger.h"
#include "platform/src/app/sysmond/delphi/sysmond_delphi_cb.hpp"
#include "platform/src/app/sysmond/event_recorder/sysmond_eventrecorder_cb.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

extern int g_print_fd;

void
event_cb_init (void)
{
    delphi_event_cb_init();
}

void
intr_dump_cb (intr_reg_t *reg, intr_field_t *field)
{
    if (field->count == 0) {
        return;
    }

    std::string name = string(reg->name) + "_" + string(field->name);

    if (g_print_fd == -1) {
        SDK_TRACE_ERR("name %s, count %lu, severity %s, desc %s",
                  name.c_str(), field->count,
                  get_severity_str(field->severity).c_str(), field->desc);
    } else {
        dprintf(g_print_fd, "%-50s %-10lu %-9s %-s\n",
                  name.c_str(), field->count,
                  get_severity_str(field->severity).c_str(), field->desc);
    }
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
    eventrecorder_temp_event_cb(temperature, hbm_event);
}

void
memory_event_cb (uint64_t total_mem, uint64_t available_mem,
                 uint64_t free_mem)
{
    delphi_memory_event_cb(total_mem, available_mem, free_mem);
}

void
panic_event_cb ()
{
    eventrecorder_panic_event_cb();
}

void
intr_event_cb (const intr_reg_t *reg, const intr_field_t *field)
{
    bool iscattrip = false;
    std::string desc;

    switch (field->severity) {
    case INTR_SEV_TYPE_HW_RMA:
        desc = string(reg->name) + "_" + string(field->name) + ":" +
               string(field->desc);
        switch (reg->id) {
        // TODO use enums
        case 477 ... 482:
            // skip over MX and BX interrupts
            break;
        default:
            eventrecorder_fatal_interrupt_event_cb(desc.c_str());
            break;
        }

    case INTR_SEV_TYPE_FATAL:
    case INTR_SEV_TYPE_ERR:
        if (field->count == 1) {
            TRACE_INFO(GetAsicErrObflOnetimeLogger(),
                       "name: {}_{}, count: {}, severity: {}, desc: {}",
                       reg->name, field->name, field->count,
                       get_severity_str(field->severity).c_str(), field->desc);
        }
        TRACE_INFO(GetAsicErrObflLogger(),
                   "name: {}_{}, count: {}, severity: {}, desc: {}",
                   reg->name, field->name, field->count,
                   get_severity_str(field->severity).c_str(), field->desc);
        break;
    case INTR_SEV_TYPE_INFO:
    default:
        break;
    }

    // TODO use enums
    // invoke unravel interrupts for mc[0-7]_mch_int_mc registers
    switch (reg->id) {
    case 396:
    case 400:
    case 404:
    case 408:
    case 412:
    case 416:
    case 420:
    case 424:
        sdk::asic::pd::asic_pd_unravel_hbm_intrs(&iscattrip, true);
        if (iscattrip == false) {
            TRACE_ERR(GetAsicErrObflLogger(), "ECCERR observed on the system.");
        }
        break;
    default:
        break;
    }

    // notify
    interrupt_notify(reg->id, field->id);
}
