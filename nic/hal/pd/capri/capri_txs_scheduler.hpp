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

#define CAPRI_TXS_MAX_TABLE_ENTRIES 2048
// 2K * 8K scheduler
#define CAPRI_TXS_SCHEDULER_MAP_MAX_ENTRIES 2048
#define CAPRI_TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY 8192

/* (lif,queue,cos) mapping params in scheduler table */
typedef struct capri_txs_sched_lif_params_s_ {
    uint32_t             sched_table_offset;
    uint32_t             num_entries_per_cos; 
    uint32_t             total_qcount; 
    uint16_t             cos_bmp;
} __PACK__ capri_txs_sched_lif_params_t;

/* (lif,cos) mapping params in policer table */
typedef struct capri_txs_policer_lif_params_s {
    uint32_t             sched_table_start_offset;
    uint32_t             sched_table_end_offset;
} __PACK__ capri_txs_policer_lif_params_t;  

/** capri_txs_scheduler_init
 * API to init the txs scheduler module
 *
 * @return hal_ret_t: Status of the operation
 */

hal_ret_t capri_txs_scheduler_init(uint32_t admin_cos);

/** capri_txs_scheduler_lif_params_update
 * API to program txs scheduler table with lif,queue,cos mappings.
 *
 * @param  hw_lif_id[in]: hw lif id for this entry.
 * @param  txs_hw_params[in]. scheduler table params for this entry.
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t capri_txs_scheduler_lif_params_update(uint32_t hw_lif_id, 
                                                capri_txs_sched_lif_params_t *txs_hw_params);

hal_ret_t capri_txs_scheduler_tx_alloc(capri_txs_sched_lif_params_t *tx_params,
                                       uint32_t *alloc_offset,
                                       uint32_t *alloc_units);

hal_ret_t capri_txs_scheduler_tx_dealloc(uint32_t alloc_offset,
                                         uint32_t alloc_units);

/** capri_txs_policer_lif_params_update
 * API to program txs policer table with lif,cos scheduler-table mappings.
 *      
 * @param  hw_lif_id[in]: hw lif id for this entry.
 * @param  txs_hw_params[in]. scheduler table params for this entry.
 * @return hal_ret_t: Status of the operation
 */     
hal_ret_t capri_txs_policer_lif_params_update(uint32_t hw_lif_id,
                                 capri_txs_policer_lif_params_t *txs_hw_params);

#endif
