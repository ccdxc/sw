#ifndef __REPL_LIST_HPP__
#define __REPL_LIST_HPP__

#include "base.h"


namespace hal {
namespace pd {
namespace utils {

class ReplTableEntry;
class Met;

/** ---------------------------------------------------------------------------
  *
  * class ReplList
  *
  *  - Identifies a replication List
  *
  * ---------------------------------------------------------------------------
*/
class ReplList {

private:
    uint32_t        repl_tbl_index_;            // idx of head of replication list
    uint32_t        num_repl_tbl_entries_;      // number of repl tbl entries
    ReplTableEntry  *first_repl_tbl_entry_;     // head of linked list
    ReplTableEntry  *last_repl_tbl_entry_;      // tail of linked list

    Met             *met_;                      // Back Pointer to MET

public:
    ReplList(uint32_t repl_tbl_index, Met *met);
    ~ReplList();

    // Getters & Setters
    uint32_t get_repl_tbl_index() { return repl_tbl_index_; }
    ReplTableEntry *get_repl_tbl_entry() { return first_repl_tbl_entry_; }
    Met *get_met() { return met_; }

    // Methods
    hal_ret_t add_replication(void *data);
    hal_ret_t del_replication(void *data);
    hal_ret_t get_repl_table_entry(ReplTableEntry **rte);
};


} // utils
} // pd
} // hal

#endif // __REPL_LIST_HPP__
