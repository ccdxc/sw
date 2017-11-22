#ifndef __REPL_ENTRY_HPP__
#define __REPL_ENTRY_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace pd {
namespace utils {

/** ---------------------------------------------------------------------------
   *
   * class ReplEntry
   *
   *  - Identifies a replication
   *
   * ---------------------------------------------------------------------------
 */
class ReplEntry {

private:
    void *data_;                // opaque data
    uint32_t data_len_;         // opaque data len

    ReplEntry *prev_, *next_;   // linked List

    ReplEntry(void *data, uint32_t data_len);
    ~ReplEntry();
public:
    static ReplEntry *factory(void *data, uint32_t data_len,
                              uint32_t mtrack_id = HAL_MEM_ALLOC_MET_REPL_ENTRY);
    static void destroy(ReplEntry *re, 
                        uint32_t mtrack_id = HAL_MEM_ALLOC_MET_REPL_ENTRY);

    // Getters & Setters
    void *get_data() { return data_; }
    uint32_t get_data_len() { return data_len_; }
    ReplEntry *get_prev() { return prev_; }
    ReplEntry *get_next() { return next_; }

    void set_data(void *data) { data_ = data; }
    void set_data_len(uint32_t data_len) { data_len_ = data_len; }
    void set_prev(ReplEntry *prev) { prev_ = prev; }
    void set_next(ReplEntry *next) { next_ = next; }

    // Methods:
    hal_ret_t trace_repl_entry();

};

} // utils
} // pd
} // hal

#endif // __REPL_ENTRY_HPP__
