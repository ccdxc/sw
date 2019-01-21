//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for csr init func
///
//===----------------------------------------------------------------------===//

#include "nic/sdk/platform/capri/csr/asicrw_if.hpp"
#include "nic/asic/capri/model/utils/cap_csr_py_if.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"

void
csr_init ()
{
    // Register for hal cpu interface
    auto cpu_if = new cpu_hal_if("cpu", "all");
    cpu::access()->add_if("cpu_if", cpu_if);
    cpu::access()->set_cur_if_name("cpu_if");

    // Register at top level for all MRL classes.
    cap_top_csr_t *cap0_ptr = new cap_top_csr_t("cap0");

    cap0_ptr->init(0);
    CAP_BLK_REG_MODEL_REGISTER(cap_top_csr_t, 0, 0, cap0_ptr);
    register_chip_inst("cap0", 0, 0);
}
