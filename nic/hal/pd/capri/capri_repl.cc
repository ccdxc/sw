// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/include/hal_pd_error.hpp"
#include "nic/hal/pd/capri/capri_repl.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/p4/nw/include/defines.h"

/* HBM base address in System memory map; Cached once at the init time */
static uint32_t repl_table_base_addr;

hal_ret_t
capri_repl_entry_read(uint32_t index, capri_repl_table_entry* entry)
{
    uint32_t entry_offset = index * CAPRI_REPL_ENTRY_WIDTH;
    uint32_t base_in_entry_units = repl_table_base_addr / CAPRI_REPL_ENTRY_WIDTH;

    HAL_ASSERT(index < CAPRI_REPL_TABLE_DEPTH);

    hal::pd::asic_mem_read(repl_table_base_addr + entry_offset,
                           (uint8_t *)entry,
                           CAPRI_REPL_ENTRY_WIDTH);

    if (entry->get_last_entry() == 0) {
        entry->set_next_ptr(entry->get_next_ptr() - base_in_entry_units);
    }

    return (HAL_RET_OK);
}

hal_ret_t
capri_repl_entry_write(uint32_t index, capri_repl_table_entry* entry)
{
    uint32_t entry_offset = index * CAPRI_REPL_ENTRY_WIDTH;
    uint32_t base_in_entry_units = repl_table_base_addr / CAPRI_REPL_ENTRY_WIDTH;

    HAL_ASSERT(index < CAPRI_REPL_TABLE_DEPTH);

    if (entry->get_last_entry() == 0) {
        entry->set_next_ptr(entry->get_next_ptr() + base_in_entry_units);
    }

    hal::pd::asic_mem_write(repl_table_base_addr + entry_offset,
                            (uint8_t *)entry,
                            CAPRI_REPL_ENTRY_WIDTH);

    if (entry->get_last_entry() == 0) {
        entry->set_next_ptr(entry->get_next_ptr() - base_in_entry_units);
    }

#ifdef HAL_LOG_TBL_UPDATES
    HAL_TRACE_DEBUG("{}", "REPL-TABLE Written");
#endif

    return (HAL_RET_OK);
}

hal_ret_t
capri_repl_init (void)
{
    repl_table_base_addr = get_start_offset(JP4_REPL);
    capri_tm_repl_table_base_addr_set(repl_table_base_addr / CAPRI_REPL_ENTRY_WIDTH);
    capri_tm_repl_table_token_size_set(P4_REPL_ENTRY_WIDTH * 8);
    return HAL_RET_OK;
}
