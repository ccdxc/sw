/*
 * capri_txs_scheduler.hpp
 * Vishwas Danivas (Pensando Systems)
 */

#ifndef __CAPRI_TXS_SCHEDULER_HPP__
#define __CAPRI_TXS_SCHEDULER_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "nic/include/base.h"

#define TXS_MAX_TABLE_ENTRIES 2048

/* (lif,queue,cos) mapping params in scheduler table */
typedef struct txs_sched_lif_params_s {
    uint32_t             sched_table_offset;
    uint32_t             num_entries_per_cos; 
    uint16_t             cos_bmp;
} txs_sched_lif_params_t;

/** capri_txs_scheduler_init
 * API to init the txs scheduler module
 *
 * @return hal_ret_t: Status of the operation
 */

hal_ret_t capri_txs_scheduler_init();

/** capri_txs_scheduler_lif_params_update
 * API to program txs scheduler table with lif,queue,cos mappings.
 *
 * @param  hw_lif_id[in]: hw lif id for this entry.
 * @param  txs_hw_params[in]. scheduler table params for this entry.
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t capri_txs_scheduler_lif_params_update(uint32_t hw_lif_id, 
                                                txs_sched_lif_params_t *txs_hw_params);

#endif
