// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/utils/thread/thread.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/hal/pd/asic_pd.hpp"

namespace hal {
namespace pd {

// check if this thread is the asic-rw thread for given chip
// Returns true if:
//    this thread's id matches with asic-rw thread's id
bool
is_asic_rw_thread(uint32_t chip, uint32_t thread_id)
{
    hal::utils::thread *asic_rw_thread = g_hal_threads[HAL_THREAD_ID_ASIC_RW];

    if (asic_rw_thread == NULL ||
        asic_rw_thread->is_running() == false) {
        assert(0);
    }

    if (thread_id == asic_rw_thread->thread_id()) {
        return true;
    }

    return false;
}

}    // namespace pd
}    // namespace hal

extern bool read_reg (uint64_t addr, uint32_t& data);
extern bool write_reg(uint64_t addr, uint32_t  data);

uint32_t
read_reg_base(uint32_t chip, uint64_t addr)
{
    uint32_t data = 0;
    hal_ret_t rc = HAL_RET_OK;

    hal::utils::thread *curr_thread = hal::utils::thread::current_thread();

    if (curr_thread && hal::pd::is_asic_rw_thread(
                        chip, curr_thread->thread_id()) == false) {
        rc = hal::pd::asic_reg_read(addr, &data);
        if (rc != HAL_RET_OK) {
            HAL_TRACE_ERR("Error reading chip: %d addr: 0x%llx",
                    chip, addr);
        }
    } else {
        read_reg(addr, data);
    }

    return data;
}

void
write_reg_base(uint32_t chip, uint64_t addr, uint32_t data)
{
    hal_ret_t rc = HAL_RET_OK;

    hal::utils::thread *curr_thread = hal::utils::thread::current_thread();

    if (curr_thread && hal::pd::is_asic_rw_thread(
                        chip, curr_thread->thread_id()) == false) {
        rc = hal::pd::asic_reg_write(addr, &data, true);
        if (rc != HAL_RET_OK) {
            HAL_TRACE_ERR("Error writing chip: %d addr: 0x%llx data: 0x%x",
                    chip, addr, data);
        }
    } else {
        write_reg(addr, data);
    }
}
