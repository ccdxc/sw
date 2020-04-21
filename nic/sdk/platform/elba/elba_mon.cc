// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

/*
 * elba_mon.cc
 */
#include "platform/elba/elba_mon.hpp"
#include "asic/pd/pd.hpp"

namespace sdk {
namespace platform {
namespace elba {

static const hbmerrcause_t hbmerrcause_table[] = {
    /* TBD-ELBA-REBASE: not applicable for ELBA */
};

static const uint32_t slave_err_reg[] = {
    /* TBD-ELBA-REBASE: not applicable for ELBA */
};

static const uint32_t master_err_reg[] = {
    /* TBD-ELBA-REBASE: not applicable for ELBA */
};

static bool __attribute__ ((unused))
read_hbmerrcause_table (const hbmerrcause_t *entry, uint64_t nwl_base_addr,
                        uint8_t channel, bool logging)
{
    return false; // TBD-ELBA-REBASE: Missing function in elba
}

static bool __attribute__ ((unused))
read_cattrip_reg (uint64_t nwl_base_addr, uint8_t channel)
{
    return false; // TBD-ELBA-REBASE: Missing function in elba
}

static bool __attribute__ ((unused))
print_mch_sta_data (uint64_t mc_base_addr, uint8_t channel)
{
    return false; // TBD-ELBA-REBASE: Missing function in elba
}

static bool __attribute__ ((unused))
print_slave_err_regs (void)
{
    return false; // TBD-ELBA-REBASE: Missing function in elba
}

static bool __attribute__ ((unused))
print_master_err_regs (void)
{
    return false; // TBD-ELBA-REBASE: Missing function in elba
}

sdk_ret_t
elba_unravel_hbm_intrs (bool *iscattrip, bool *iseccerr, bool logging)
{
    return SDK_RET_INVALID_OP; // TBD-ELBA-REBASE: Missing function in elba
}

pen_adjust_perf_status_t
elba_adjust_perf (int chip_id, int inst_id, pen_adjust_index_t &idx,
                  pen_adjust_perf_type_t perf_type)
{
    return PEN_PERF_SUCCESS;    // TBD-ELBA-REBASE: Missing function in elba
}

void
elba_set_half_clock (int chip_id, int inst_id)
{
    return ;
}

} // namespace elba
} // namespace platform
} // namespace sdk
