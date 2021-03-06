#ifndef __REPL_LIST_HPP__
#define __REPL_LIST_HPP__

#include "nic/include/base.hpp"
#include "nic/include/hal_mem.hpp"


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
    uint32_t        attached_list_idx_;	        // Idx of the spliced list (if any)
    uint32_t        num_repl_tbl_entries_;      // number of repl tbl entries
    ReplTableEntry  *first_repl_tbl_entry_;     // head of linked list
    ReplTableEntry  *last_repl_tbl_entry_;      // tail of linked list

    Met             *met_;                      // Back Pointer to MET

    ReplList(uint32_t repl_tbl_index, Met *met);
    ~ReplList();
public:
    static ReplList *factory(uint32_t repl_tbl_index, Met *met,
                             uint32_t mtrack_id = HAL_MEM_ALLOC_MET_REPL_LIST);
    static void destroy(ReplList *repl_list,
                        uint32_t mtrack_id = HAL_MEM_ALLOC_MET_REPL_LIST);

    // Getters & Setters
    uint32_t get_repl_tbl_index() { return repl_tbl_index_; }
    ReplTableEntry *get_repl_tbl_entry() { return first_repl_tbl_entry_; }
    Met *get_met() { return met_; }
    uint32_t get_attached_list_index() { return attached_list_idx_; }
    void set_attached_list_index(uint32_t list_idx){attached_list_idx_ = list_idx;}

    // Methods
    hal_ret_t add_replication(void *data);
    hal_ret_t del_replication(void *data);
    hal_ret_t get_repl_table_entry(ReplTableEntry **rte);
    hal_ret_t process_del_repl_tbl_entry(ReplTableEntry *rte);
    hal_ret_t initialize_first_repl_table_entry(void);
    hal_ret_t cleanup_last_repl_table_entry(void);
    hal_ret_t attach_to_repl_list(ReplList *list);
    hal_ret_t detach_frm_repl_list(void);

    hal_ret_t entry_to_str(char *buff, uint32_t buff_size);
    hal_ret_t trace_repl_list();
};


} // utils
} // pd
} // hal

#endif // __REPL_LIST_HPP__
