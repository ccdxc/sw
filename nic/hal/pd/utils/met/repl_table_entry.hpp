#ifndef __REPL_TABLE_ENTRY_HPP__
#define __REPL_TABLE_ENTRY_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"


namespace hal {
namespace pd {
namespace utils {

// Forward Declarations
class ReplEntry;
class ReplList;

/** ---------------------------------------------------------------------------
   *
   * class RepTableEntry
   *
   *  - Identifies an entry in replication table
   *
   * ---------------------------------------------------------------------------
 */
class ReplTableEntry {

private:

    uint32_t        repl_table_index_;		// idx in repl. table	
    ReplList        *repl_list_;			// back ptr to repl. list

    uint32_t        num_repl_entries_;		// number of repl entries
    ReplEntry       *first_repl_entry_;		// head of linked list
    ReplEntry       *last_repl_entry_;		// tail of linked list

    ReplTableEntry  *prev_, *next_;			// linked list ptrs

    ReplTableEntry(uint32_t repl_table_index, ReplList *repl_list);
    ~ReplTableEntry();
public:
    static ReplTableEntry *factory(uint32_t repl_table_index, 
                                   ReplList *repl_list, 
                                   uint32_t mtrack_id = HAL_MEM_ALLOC_MET_REPL_TABLE_ENTRY);
    static void destroy(ReplTableEntry *rte, 
                        uint32_t mtrack_id = HAL_MEM_ALLOC_MET_REPL_TABLE_ENTRY);

    // Getters & Setters
    uint32_t get_num_repl_entries() { return num_repl_entries_; }
    ReplEntry *get_first_repl_entry() { return first_repl_entry_; }
    uint32_t get_repl_table_index() { return repl_table_index_; }
    ReplTableEntry *get_prev() { return prev_; }
    ReplTableEntry *get_next() { return next_; }
    void set_prev(ReplTableEntry *prev) { prev_ = prev; }
    void set_next(ReplTableEntry *next) { next_ = next; }
    void set_repl_table_index(uint32_t rte_idx) { repl_table_index_ = rte_idx; }
    


    // Methods
    hal_ret_t add_replication(ReplEntry *re);
    hal_ret_t del_replication(void *data);
    hal_ret_t program_table();
    hal_ret_t deprogram_table();
    hal_ret_t trace_repl_tbl_entry();
};

}   // utils
}   // pd
}   // hal

#endif // __REPL_TABLE_ENTRY_HPP__
