//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include "sysmon_internal.hpp"
#include "asic/pd/pd.hpp"
#include "third-party/asic/capri/verif/apis/cap_freq_api.h"

using namespace sdk::asic::pd;

sysmond_db_t db;

static void
sysmond_event (sysmond_event_t event)
{
    switch (event) {
    case SYSMOND_FREQUENCY_CHANGE:
        break;

    default:
        break;
    }
}

void
checkfrequency(void)
{
    int chip_id = 0;
    int inst_id = 0;

    uint32_t frequency = cap_top_sbus_get_core_freq(chip_id, inst_id);
    if (frequency != db.frequency) {
        db.frequency = frequency;
        if (g_sysmon_cfg.frequency_change_event_cb) {
            g_sysmon_cfg.frequency_change_event_cb(frequency);
        }
    }
    return;
}

void
checkcattrip(void)
{
    bool iscattrip = false;
    bool iseccerr = false;
    int chip_id = 0;
    int inst_id = 0;
    static bool logcattrip = false;

    asic_pd_unravel_hbm_intrs(&iscattrip, &iseccerr, false);
    if (iscattrip == true && logcattrip == false) {
        // asic_pd_set_half_clock(chip_id, inst_id);
        if (g_sysmon_cfg.cattrip_event_cb) {
            g_sysmon_cfg.cattrip_event_cb();
        }
        logcattrip = true;
        sync();
        // resetting the system.
        pal_power_cycle();
    }
}

// MONFUNC(checkfrequency);
// MONFUNC(checkcattrip);
