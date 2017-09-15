// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <base.h>
#include <p4pd.h>
#include <capri_hbm.hpp>
#include <hal_pd_error.hpp>
#include <capri_repl.hpp>
#include <capri_tm_rw.hpp>
#include <lib_model_client.h>

/* HBM base address in System memory map; Cached once at the init time */
static uint64_t repl_table_base_addr;

hal_ret_t
capri_repl_entry_read(uint32_t index, capri_repl_table_entry* capri_repl_entry)
{
    uint64_t entry_start_addr = index * CAPRI_REPL_ENTRY_WIDTH;

    HAL_ASSERT(index < CAPRI_REPL_TABLE_DEPTH);

    read_mem(repl_table_base_addr + entry_start_addr, (uint8_t *)capri_repl_entry, CAPRI_REPL_ENTRY_WIDTH);

    return (HAL_RET_OK);
}

hal_ret_t
capri_repl_entry_write(uint32_t index, capri_repl_table_entry* capri_repl_entry)
{
    uint64_t entry_start_addr = index * CAPRI_REPL_ENTRY_WIDTH;

    HAL_ASSERT(index < CAPRI_REPL_TABLE_DEPTH);

    write_mem(repl_table_base_addr + entry_start_addr, (uint8_t *)capri_repl_entry, CAPRI_REPL_ENTRY_WIDTH);

#ifdef HAL_LOG_TBL_UPDATES
    HAL_TRACE_DEBUG("{}", "REPL-TABLE Written");
#endif

    return (HAL_RET_OK);
}

hal_ret_t
capri_repl_init (void)
{
    repl_table_base_addr = (uint64_t)get_start_offset(JP4_REPL);
    capri_tm_repl_table_base_addr_set(repl_table_base_addr >> 6);
    capri_tm_repl_table_num_tokens_set(CAPRI_REPL_NUM_P4_ENTRIES_PER_NODE);
    return HAL_RET_OK;
}
