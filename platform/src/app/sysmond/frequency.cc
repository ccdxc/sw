/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "third-party/asic/capri/verif/apis/cap_freq_api.h"

static delphi::objects::asicfrequencymetrics_t    asicfrequency;

static void
checkfrequency(void)
{
    uint64_t key = 0;
    int ret;
    unsigned int frequency;
    int chip_id = 0;
    int inst_id = 0;

    frequency = cap_top_sbus_get_core_freq(chip_id, inst_id);
    if (frequency != asicfrequency.frequency) {
        TRACE_INFO(GetLogger(), "Core frequency of the system {}MHz", frequency);
    }
    asicfrequency.frequency = frequency;
    delphi::objects::AsicFrequencyMetrics::Publish(key, &asicfrequency);
    return;
}

MONFUNC(checkfrequency);
