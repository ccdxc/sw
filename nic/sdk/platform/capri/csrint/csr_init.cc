//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for csr init func
///
//===----------------------------------------------------------------------===//

#include "platform/capri/csr/asicrw_if.hpp"
#include "third-party/asic/capri/model/utils/cap_csr_py_if.h"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"

namespace sdk {
namespace platform {
namespace capri {

void
csr_init ()
{
    static bool csr_init_done = false;

    if (csr_init_done) {
        return;
    }

    // Register for hal cpu interface
    auto cpu_if = new cpu_hal_if("cpu", "all");
    cpu::access()->add_if("cpu_if", cpu_if);
    cpu::access()->set_cur_if_name("cpu_if");

    // Register at top level for all MRL classes.
    cap_top_csr_t *cap0_ptr = new cap_top_csr_t("cap0");

    cap0_ptr->init(0);
    CAP_BLK_REG_MODEL_REGISTER(cap_top_csr_t, 0, 0, cap0_ptr);
    register_chip_inst("cap0", 0, 0);
    csr_init_done = true;
    return;
}

} // end namespace capri
} // end namespace platform
} // end namespace sdk
