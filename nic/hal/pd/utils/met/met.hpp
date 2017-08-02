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
#include "base.h"
#include "indexer.hpp"


using hal::utils::indexer;

namespace hal {
namespace pd {
namespace utils {


class ReplList;

typedef std::map<uint32_t, ReplList*> ReplListMap;

/** ---------------------------------------------------------------------------
  *  
  *  class Met
  *
  *  - Multi-destination table management.
  *
  * ---------------------------------------------------------------------------
*/
class Met {

private:

    std::string     table_name_;             // table name
    uint32_t        table_id_;               // table id
    uint32_t        repl_table_capacity_;    // size of repl table
    uint32_t        max_num_repls_per_entry_;// repl entries per table entry
    uint32_t        repl_entry_data_len_;    // repl entry data len
    indexer         *repl_table_indexer_;    // repl table indexer
    
    ReplListMap     repl_list_map_;

public:

    Met(std::string table_name, uint32_t table_id, 
        uint32_t repl_table_capacity, uint32_t num_repl_entries,
        uint32_t repl_entry_data_len);
    ~Met();

    // Getters & Setters
    uint32_t get_repl_entry_data_len() { return repl_entry_data_len_; }
    uint32_t get_max_num_repls_per_entry() { return max_num_repls_per_entry_; }
    // Methods
    hal_ret_t create_repl_list(uint32_t *repl_list_idx);
    hal_ret_t add_replication(uint32_t repl_list_idx, void *data);
    hal_ret_t del_replication(uint32_t repl_list_idx, void *data);
    hal_ret_t delete_repl_list(uint32_t repl_list_idx);

    hal_ret_t alloc_repl_table_index(uint32_t *idx);
    hal_ret_t free_repl_table_index(uint32_t idx);
};

} // utils
} // pd
} // hal

#endif // __MET_HPP__
