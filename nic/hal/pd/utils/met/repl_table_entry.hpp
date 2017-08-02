#ifndef __REPL_TABLE_ENTRY_HPP__
#define __REPL_TABLE_ENTRY_HPP__

#include "base.h"


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

public:

    ReplTableEntry(uint32_t repl_table_index, ReplList *repl_list);
    ~ReplTableEntry();

    // Getters & Setters
    uint32_t get_num_repl_entries() { return num_repl_entries_; }
    ReplEntry *get_first_repl_entry() { return first_repl_entry_; }
    uint32_t get_repl_table_index() { return repl_table_index_; }
    ReplTableEntry *get_prev() { return prev_; }
    ReplTableEntry *get_next() { return next_; }
    void set_prev(ReplTableEntry *prev) { prev_ = prev; }
    void set_next(ReplTableEntry *next) { next_ = next; }
    


    // Methods
    hal_ret_t add_replication(ReplEntry *re);
    hal_ret_t del_replication(void *data);
    hal_ret_t program_table();
    hal_ret_t deprogram_table();
};

}   // utils
}   // pd
}   // hal

#endif // __REPL_TABLE_ENTRY_HPP__
