#ifndef __REPL_ENTRY_HPP__
#define __REPL_ENTRY_HPP__

#include "nic/include/base.hpp"
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

#define P4PD_REPL_DATA_SIZE            (60)
#define P4PD_REPL_ENTRY_WIDTH          (64)
#define P4PD_REPL_TABLE_DEPTH          (64*1024)

class ReplEntryHw {
	struct __attribute__((__packed__)) __repl_entry_hw_s {
		uint8_t  token_bytes[P4PD_REPL_DATA_SIZE];
		uint32_t last_entry:1;
		uint32_t num_tokens:4;
		uint32_t next_ptr  :27;
	} repl_entry_hw_;

public:
	ReplEntryHw()  {memset(&repl_entry_hw_, 0, sizeof(repl_entry_hw_));}
	uint32_t get_last_entry() {return repl_entry_hw_.last_entry;}
	uint32_t get_num_tokens() {return repl_entry_hw_.num_tokens;}
	uint32_t get_next_ptr()   {return repl_entry_hw_.next_ptr;}
	void get_token(void *token, uint32_t index, uint32_t size){memcpy(token, &repl_entry_hw_.token_bytes[index*size], size);}

	void set_last_entry(uint32_t las) {repl_entry_hw_.last_entry = las;}
	void set_num_tokens(uint32_t num) {repl_entry_hw_.num_tokens = num;}
	void set_next_ptr(uint32_t ptr) {repl_entry_hw_.next_ptr = ptr;}
	void set_token(void *token, uint32_t index, uint32_t size){
		for (uint32_t i=0; i<size; i++) {
			repl_entry_hw_.token_bytes[(index * size) + i] = ((uint8_t*)token)[i];
		}
	}
    hal_ret_t read(uint32_t index);
    hal_ret_t write(uint32_t index);
};

} // utils
} // pd
} // hal

#endif // __REPL_ENTRY_HPP__
