/* ============================================================================
 |
 | MET Library
 | --- ------
 |
 | - Helps in programming multi-destination replication tables in HBM
 |
 * ============================================================================
 */
#ifndef __MET_HPP__
#define __MET_HPP__

#include <string>
#include <map>
#include "nic/include/base.hpp"
#include "nic/include/hal_mem.hpp"
#include "lib/indexer/indexer.hpp"
#include "lib/table/common/table.hpp"

using sdk::lib::indexer;
using sdk::table::table_health_state_t;
using sdk::table::table_health_monitor_func_t;

namespace hal {
namespace pd {
namespace utils {

class Met;
class ReplList;

typedef std::map<uint32_t, ReplList*> ReplListMap;

typedef uint32_t (*met_repl_entry_to_str_func_t)(void *repl_entry_data,
                                                 char *buff, uint32_t buff_size);

typedef bool (*met_iterate_func_t)(uint32_t repl_list_idx, Met *met,
                                   const void *cb_data);

/** ---------------------------------------------------------------------------
  *
  *  class Met
  *
  *  - Multi-destination table management.
  *
  * ---------------------------------------------------------------------------
*/
class Met {
public:
    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_FAIL_NO_RES,
        STATS_INS_FAIL_HW,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_HW,
        STATS_MAX
    };

private:

    std::string                 table_name_;             // table name
    uint32_t                    table_id_;               // table id
    uint32_t                    repl_table_capacity_;    // size of repl table
    uint32_t                    max_num_repls_per_entry_;// repl entries per table entry
    uint32_t                    repl_entry_data_len_;    // repl entry data len
    indexer                     *repl_table_indexer_;    // repl table indexer
    table_health_state_t        health_state_;  // health state
    table_health_monitor_func_t health_monitor_func_;   // health mon. cb
    met_repl_entry_to_str_func_t repl_entry_str_func_;   // repl. entry to str func

    ReplListMap     repl_list_map_;

    uint64_t        *stats_;                // Statistics

    void stats_incr(stats stat);
    void stats_decr(stats stat);
    enum api {
        INSERT,
        UPDATE,
        REMOVE,
        RETRIEVE,
        RETRIEVE_FROM_HW,
        ITERATE
    };
    void stats_update(api ap, hal_ret_t rs);
    void trigger_health_monitor();

    Met(std::string table_name, uint32_t table_id,
        uint32_t repl_table_capacity, uint32_t num_repl_entries,
        uint32_t repl_entry_data_len,
        table_health_monitor_func_t health_monitor_func = NULL,
        met_repl_entry_to_str_func_t repl_entry_str_func = NULL);
    ~Met();

public:
    static Met *factory(std::string table_name, uint32_t table_id,
                        uint32_t repl_table_capacity, uint32_t num_repl_entries,
                        uint32_t repl_entry_data_len,
                        uint32_t mtrack_id = HAL_MEM_ALLOC_MET,
                        table_health_monitor_func_t health_monitor_func = NULL,
                        met_repl_entry_to_str_func_t repl_entry_str_func = NULL);
    static void destroy(Met *met,
                        uint32_t mtrack_id = HAL_MEM_ALLOC_MET);

    // Debug Info
    uint32_t table_id(void) { return table_id_; }
    const char *table_name(void) { return table_name_.c_str(); }
    uint32_t table_capacity(void) { return repl_table_capacity_; }
    uint32_t table_num_entries_in_use(void);
    uint32_t table_num_inserts(void);
    uint32_t table_num_insert_errors(void);
    uint32_t table_num_updates(void);
    uint32_t table_num_update_errors(void);
    uint32_t table_num_deletes(void);
    uint32_t table_num_delete_errors(void);



    // Getters & Setters
    uint32_t get_repl_entry_data_len() { return repl_entry_data_len_; }
    uint32_t get_max_num_repls_per_entry() { return max_num_repls_per_entry_; }
    met_repl_entry_to_str_func_t get_repl_entry_to_str_func() { return repl_entry_str_func_; }
    // Methods
    hal_ret_t create_repl_list(uint32_t *repl_list_idx);
    hal_ret_t create_repl_list_with_id(uint32_t repl_list_idx);
    hal_ret_t create_repl_list_block(uint32_t *repl_list_idx, uint32_t size);
    hal_ret_t add_replication(uint32_t repl_list_idx, void *data);
    hal_ret_t del_replication(uint32_t repl_list_idx, void *data);
    hal_ret_t delete_repl_list(uint32_t repl_list_idx);
    hal_ret_t delete_repl_list_block(uint32_t repl_list_idx, uint32_t size);
    hal_ret_t attach_repl_lists(uint32_t frm_list_idx, uint32_t to_list_idx);
    hal_ret_t detach_repl_lists(uint32_t frm_list_idx);
    hal_ret_t iterate(met_iterate_func_t cb, const void *cb_data);
    hal_ret_t repl_list_to_str(uint32_t repl_list_idx, char *buff, uint32_t buff_size);

    hal_ret_t alloc_repl_table_index(uint32_t *idx);
    hal_ret_t free_repl_table_index(uint32_t idx);
    hal_ret_t trace_met();
};

} // utils
} // pd
} // hal

#endif // __MET_HPP__

