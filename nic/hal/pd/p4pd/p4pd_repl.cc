// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/include/asic_pd.hpp"
#include "nic/hal/pd/p4pd/p4pd_repl.hpp"

#define HAL_LOG_TBL_UPDATES

/* HBM base address in System memory map; Cached once at the init time */
extern uint64_t capri_hbm_base;
extern uint64_t hbm_repl_table_offset;

hal_ret_t
p4pd_repl_entry_read(uint32_t index, p4pd_repl_table_entry* entry)
{
    uint64_t entry_offset = index * P4PD_REPL_ENTRY_WIDTH;
    uint64_t base_in_entry_units = hbm_repl_table_offset / P4PD_REPL_ENTRY_WIDTH;

    HAL_ASSERT(index < P4PD_REPL_TABLE_DEPTH);

    hal::pd::asic_mem_read(capri_hbm_base + hbm_repl_table_offset + entry_offset,
                           (uint8_t *)entry,
                           P4PD_REPL_ENTRY_WIDTH);

    if (entry->get_last_entry() == 0) {
        entry->set_next_ptr(entry->get_next_ptr() - base_in_entry_units);
    }

    return (HAL_RET_OK);
}

hal_ret_t
p4pd_repl_entry_write(uint32_t index, p4pd_repl_table_entry* entry)
{
    uint64_t entry_offset = index * P4PD_REPL_ENTRY_WIDTH;
    uint64_t base_in_entry_units = hbm_repl_table_offset / P4PD_REPL_ENTRY_WIDTH;

    HAL_ASSERT(index < P4PD_REPL_TABLE_DEPTH);

    if (entry->get_last_entry() == 0) {
        entry->set_next_ptr(entry->get_next_ptr() + base_in_entry_units);
    }

    hal::pd::asic_mem_write(capri_hbm_base + hbm_repl_table_offset + entry_offset,
                            (uint8_t *)entry,
                            P4PD_REPL_ENTRY_WIDTH);

    if (entry->get_last_entry() == 0) {
        entry->set_next_ptr(entry->get_next_ptr() - base_in_entry_units);
    }

#ifdef HAL_LOG_TBL_UPDATES
    HAL_TRACE_DEBUG("{}", "REPL-TABLE Written");
#endif

    return (HAL_RET_OK);
}
