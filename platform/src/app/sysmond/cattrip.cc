/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmond.h"
#include "asic/pd/pd.hpp"

using namespace sdk::asic::pd;

static void
checkcattrip(void)
{
    bool iscattrip = false;
    static bool logcattrip = false;
    int chip_id = 0;
    int inst_id = 0;

    asic_pd_unravel_hbm_intrs(&iscattrip);
    if (iscattrip == true &&
        logcattrip == false) {
        TRACE_INFO(GetObflLogger(), "Cattrip::Reducing speed to half");
        TRACE_FLUSH(GetObflLogger());
        asic_pd_set_half_clock(chip_id, inst_id);
        logcattrip = true;
    }
}

MONFUNC(checkcattrip);
