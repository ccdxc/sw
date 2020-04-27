//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines elba soft initialization
///
//----------------------------------------------------------------------------

#include "asic/cmn/asic_init.hpp"
#include "platform/elba/elba_cfg.hpp"
#include "platform/elba/elba_tm_rw.hpp"
#include "platform/elba/elba_tbl_rw.hpp"
#include "platform/elba/elba_state.hpp"

namespace sdk {
namespace platform {
namespace elba {

// elba soft initialization for modules which they need to read the elba
// registers
sdk_ret_t
elba_soft_init (asic_cfg_t *cfg)
{
    sdk_ret_t    ret;
    bool         asm_write_to_mem = false;

    SDK_ASSERT_TRACE_RETURN((cfg != NULL), SDK_RET_INVALID_ARG, "Invalid cfg");
    SDK_TRACE_DEBUG("Initializing Elba");

    ret = sdk::platform::elba::elba_state_pd_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "elba_state_pd_init failure, err : %d", ret);

    ret = elba_table_rw_soft_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "elba_tbl_rw_init failure, err : %d", ret);

    // just populate the program info. don't write to the memory
    ret = sdk::asic::asic_asm_init(cfg, asm_write_to_mem);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Elba ASM init failure, err : %d", ret);
    // initialize the profiles for elba register accesses by
    // other modules (link manager).
    ret = elba_tm_soft_init(cfg->catalog,
                             &cfg->device_profile->qos_profile);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "Elba TM Slave init failure, err : %d", ret);

    if (cfg->completion_func) {
        cfg->completion_func(sdk::SDK_STATUS_ASIC_INIT_DONE);
    }

    return ret;
}

}    // namespace elba
}    // namespace platform
}    // namespace sdk
